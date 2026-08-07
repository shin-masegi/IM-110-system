"""IM-110 LCD 描画エミュレータ: ヘッダのアイコン配列を読み 400x240 モノクロ画面を再現する。
   実装準拠: lcd_write_box (Lcd.c) / LS027_disp_icon, LS027_disp_number (IIJIMA_Templete.c)。
   1ユニット=8px、アイコン先頭4B=幅/高さ(LE16)、1bit/px、0=黒。"""
import re, struct

W, H = 400, 240
UNI = 8
INC = '/Users/masegi/workspace/IM-110/Core/Inc/'

def _parse(path, icons, tables):
    s = open(path, encoding='utf-8', errors='replace').read()
    # 単体配列  static const uint8_t name [] = { ... };
    for m in re.finditer(r'static const uint8_t (\w+)\s*\[\]\s*=\s*\{(.*?)\};', s, re.S):
        icons[m.group(1)] = [int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]{2})', m.group(2))]
    # 2次元配列 static const uint8_t name[N][M] = { {..},{..} };
    for m in re.finditer(r'static const uint8_t (\w+)\[(\d+)\]\[(\d+)\]\s*=\s*\{(.*?)\n\};', s, re.S):
        name, n, per = m.group(1), int(m.group(2)), int(m.group(3))
        vals = [int(x, 16) for x in re.findall(r'0x([0-9A-Fa-f]{2})', m.group(4))]
        tables[name] = [vals[i*per:(i+1)*per] for i in range(n)]
ICON, TABLE = {}, {}
_parse(INC + 'Disp_Data_Common.h', ICON, TABLE)
_parse(INC + 'DisplayData.h', ICON, TABLE)

class Screen:
    def __init__(self):
        self.buf = [[0]*W for _ in range(H)]   # 0=白, 1=黒
    def icon(self, uni_x, uni_y, data):
        if isinstance(data, str):
            data = ICON[data]
        w = data[0] | (data[1] << 8); h = data[2] | (data[3] << 8)
        body = data[4:]; nb = w // 8
        x0, y0 = uni_x * UNI, uni_y * UNI
        for r in range(h):
            for c in range(nb):
                byte = body[r*nb + c]
                for b in range(8):
                    if not (byte >> (7-b)) & 1:            # 0=黒
                        x, y = x0 + c*8 + b, y0 + r
                        if 0 <= x < W and 0 <= y < H:
                            self.buf[y][x] = 1
    def fill_box(self, uni_x, uni_y, sx, sy, white):
        for y in range(uni_y*UNI, min((uni_y+sy)*UNI, H)):
            for x in range(uni_x*UNI, min((uni_x+sx)*UNI, W)):
                self.buf[y][x] = 0 if white else 1
    def number(self, uni_x, uni_y, size, fval, digit):
        tbl = {0: 'l_num', 1: 'm_num', 2: 's_num'}[size]
        pre = {0: 'l_', 1: 'm_', 2: 's_'}[size]
        num = TABLE[tbl]; dot = ICON[pre+'dot']; minus = ICON[pre+'minus']
        q = 10**digit
        fval = (int(abs(fval)*q + 0.5) / q) * (1 if fval >= 0 else -1)   # Round_off 相当
        xs = num[0][0] | (num[0][1] << 8); xs //= 8
        x = uni_x; a = abs(fval)
        if digit:
            if digit == 2:
                self.icon(x, uni_y, num[int(a*100) % 10]); x -= xs
            self.icon(x, uni_y, num[int(a*10) % 10])
            x -= (dot[0] | (dot[1] << 8)) // 8
            self.icon(x, uni_y, dot); x -= xs
        self.icon(x, uni_y, num[int(a) % 10])
        for d in (10, 100, 1000, 10000, 100000):
            if a >= d:
                x -= xs; self.icon(x, uni_y, num[int(a/d) % 10])
        if fval < 0:
            x -= xs; self.icon(x, uni_y, minus)
    def _framed(self, bw=2):
        """外周に bw px の黒枠を付けたバッファを返す (既存 assets と同じ 404x244 化)。"""
        fw, fh = W + bw*2, H + bw*2
        buf = [[1]*fw for _ in range(fh)]          # 枠=黒(1)
        for y in range(H):
            row = buf[y+bw]
            for x in range(W):
                row[x+bw] = self.buf[y][x]
        return buf, fw, fh

    def save_png(self, path, bw=2):
        import zlib
        buf, fw, fh = self._framed(bw)
        raw = b''
        for y in range(fh):
            row = bytearray([0])
            acc = 0; n = 0
            for x in range(fw):
                acc = (acc << 1) | (0 if buf[y][x] else 1)     # PNG 1bit: 1=白
                n += 1
                if n == 8:
                    row.append(acc); acc = 0; n = 0
            if n:
                row.append(acc << (8-n) | ((1 << (8-n)) - 1))  # 端数は白で埋める
            raw += bytes(row)
        def chunk(t, d):
            c = t + d
            return struct.pack('>I', len(d)) + c + struct.pack('>I', zlib.crc32(c) & 0xffffffff)
        png = (b'\x89PNG\r\n\x1a\n'
               + chunk(b'IHDR', struct.pack('>IIBBBBB', fw, fh, 1, 0, 0, 0, 0))
               + chunk(b'IDAT', zlib.compress(raw)) + chunk(b'IEND', b''))
        open(path, 'wb').write(png)

    def save_bmp(self, path, bw=2):
        """1bit BMP (ボトムアップ、4B境界パディング)。既存 assets と同じ枠付きサイズ。"""
        import struct as _st
        buf, fw, fh = self._framed(bw)
        rowsz = ((fw + 31) // 32) * 4
        px = bytearray()
        for y in range(fh - 1, -1, -1):
            row = bytearray(rowsz)
            for x in range(fw):
                if not buf[y][x]:                       # 1=白
                    row[x >> 3] |= (0x80 >> (x & 7))
            px += row
        off = 14 + 40 + 8
        hdr = b'BM' + _st.pack('<IHHI', off + len(px), 0, 0, off)
        info = _st.pack('<IiiHHIIiiII', 40, fw, fh, 1, 1, 0, len(px), 3780, 3780, 2, 2)
        pal = _st.pack('<BBBBBBBB', 0,0,0,0, 255,255,255,0)
        open(path, 'wb').write(hdr + info + pal + bytes(px))
