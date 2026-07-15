#!/usr/bin/env python3
"""
adjctl.py — IM-110 本体 調整機能A シリアル駆動 (依存なし: 標準ライブラリ termios のみ)

本体 USART1 (CN2/BLE 経由の PC 直結) は 115200 8N1、行終端 LF。
調整機能A のコマンド (AMV/ALD/ALDA/AZR/ATC/ATCF/AMC/AMD/AMCF/AMCP/AKZ/AWC/AMR/AST) を送り、
応答行を読む。既存コマンド (VR/AD/MIGV 等) もそのまま送れる。

使い方:
  # 単発
  python3 adjctl.py -p /dev/cu.usbserial-XXXX AMV
  python3 adjctl.py -p /dev/cu.usbserial-XXXX "AMC,100"
  # 複数 (順に送信、各応答を表示)
  python3 adjctl.py -p /dev/cu.usbserial-XXXX AMR "AMC,100" "AMC,500" "AMC,8000" "AMCF,1" AMD
  # ポート自動検出 (cu.usb* が1つなら)
  python3 adjctl.py AMV
  # 対話 (1行1コマンド、空行/quit で終了)
  python3 adjctl.py -i
"""
import os, sys, termios, time, glob, argparse


def find_port():
    cands = sorted(glob.glob("/dev/cu.usb*") + glob.glob("/dev/tty.usb*"))
    if len(cands) == 1:
        return cands[0]
    if not cands:
        sys.exit("USB シリアルが見つかりません。-p /dev/cu.usbserial-XXXX を指定してください。")
    sys.exit("複数のポートが見つかりました。-p で指定:\n  " + "\n  ".join(cands))


def open_port(dev, baud=115200):
    fd = os.open(dev, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    a = termios.tcgetattr(fd)          # [iflag, oflag, cflag, lflag, ispeed, ospeed, cc]
    a[0] = 0                            # iflag: raw (パリティ/変換なし)
    a[1] = 0                            # oflag: raw
    a[3] = 0                            # lflag: 非カノニカル・エコーなし
    a[2] = termios.CS8 | termios.CREAD | termios.CLOCAL   # 8bit, 受信有効, モデム制御無視
    spd = termios.B115200 if baud == 115200 else getattr(termios, "B%d" % baud)
    a[4] = spd
    a[5] = spd
    termios.tcsetattr(fd, termios.TCSANOW, a)
    termios.tcflush(fd, termios.TCIOFLUSH)
    return fd


def send_cmd(fd, cmd, wait=1.5):
    """コマンド送信 (LF 終端) → タイムアウトまで応答収集。data が来たら小延長。"""
    os.write(fd, (cmd.rstrip("\r\n") + "\n").encode())
    buf = b""
    deadline = time.time() + wait
    while time.time() < deadline:
        try:
            chunk = os.read(fd, 512)
        except BlockingIOError:
            chunk = b""
        if chunk:
            buf += chunk
            deadline = time.time() + 0.35     # データ継続中は延長
        else:
            time.sleep(0.02)
    return buf.decode(errors="replace")


def main():
    ap = argparse.ArgumentParser(description="IM-110 調整機能A シリアル駆動")
    ap.add_argument("-p", "--port", help="/dev/cu.usbserial-XXXX (省略時は自動検出)")
    ap.add_argument("-b", "--baud", type=int, default=115200)
    ap.add_argument("-w", "--wait", type=float, default=1.5, help="応答待ち秒 (ALDA 等は長め)")
    ap.add_argument("-i", "--interactive", action="store_true")
    ap.add_argument("cmds", nargs="*", help="送信コマンド (複数可)")
    args = ap.parse_args()

    dev = args.port or find_port()
    fd = open_port(dev, args.baud)
    print("# port=%s baud=%d" % (dev, args.baud))
    try:
        if args.interactive:
            print("# 対話モード: 1行1コマンド、空行 or quit で終了")
            while True:
                try:
                    line = input("adj> ").strip()
                except EOFError:
                    break
                if line == "" or line.lower() in ("quit", "exit", "q"):
                    break
                w = 30.0 if line.upper().startswith("ALDA") else args.wait
                sys.stdout.write(send_cmd(fd, line, w))
                sys.stdout.flush()
        else:
            if not args.cmds:
                ap.error("コマンドを指定するか -i を使ってください")
            for c in args.cmds:
                w = 30.0 if c.upper().startswith("ALDA") else args.wait
                print("=> %s" % c)
                sys.stdout.write(send_cmd(fd, c, w))
                sys.stdout.flush()
    finally:
        os.close(fd)


if __name__ == "__main__":
    main()
