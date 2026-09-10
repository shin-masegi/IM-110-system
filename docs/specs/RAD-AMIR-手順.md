# IM-110 プローブ係数 全読出し手順（`AMIR` / `RAD`）

対象 FW: 本体 Ver.0.44 以降（`RAD` は 2026-08-06 追加）。出典: `docs/specs/protocol-adjust-uart.md` §4.2、実装 `IM-110/Core/Src/IM_110.c` `Probe_Store_DebugDumpAll` / `Probe_Store_DebugStatus`。

## 1. 接続

- 本体 **CN2**（USART1）に USB シリアルを接続。**9600bps / 8N1**。
- コマンドは大文字、末尾 LF（CR LF でも可）。
- 本体の**電源を入れ直してから**実施する（起動時にプローブ flash の係数を本体 RAM へ読み込む）。起動後に校正・基板調整（ADBOAD）を操作しないこと。

## 2. 手順

```
AMIR      → 1 行目 AMIR,BOOT:0,... を確認（0 = プローブ flash を読めた）
RAD       → 全係数を約 30 行で出力
```

`BOOT` が 2 のときはプローブ flash を読めていない（本体 EEPROM の控え値で動作中）。電源を入れ直して再確認する。

## 3. `AMIR` 出力

| 行 | 内容 |
|---|---|
| `AMIR,BOOT:b,L2:l,CONN:c` | b: 0=プローブ flash 採用 / 2=読取失敗 / 3=実行中 / 255=未実行。l: 本体 EEPROM 控えを適用したか。c: 接続状態 |
| `AMIR,EEP:e,ID:<hex8>,UPD:<YYYYMMDD>` | 本体 EEPROM 控えの有効(1)/無効(0)、その ID と最終更新日 |
| `AMIR,LIVE,ID:<hex8>,UPD:…,MLSS_ZR:…,C1:…,SPB:…,SV1:…` | 現在の RAM 値の代表項目 |
| `AMIR,DIAG,ORE:n,UERR:n,DISC:n` | UART 診断カウンタ（オーバーラン／エラー／受信破棄） |
| `AMIR,RPG,R:…,N:…,BAD:…,JUNK:…,TO:…,CHK:…,TRY:…,FAIL:…` | プローブ flash 読出し（RPG）の診断 |
| `AMIR,RPGLINE:…` | 読出し失敗時のみ、壊れた行の現物 |

## 4. `RAD` 出力（読み取り専用。状態は変わらない）

数値はすべて 10 進テキスト。`<M|S|T>` は MLSS／SS／透視度。

| 行 | 内容 |
|---|---|
| `RAD,ID:<hex8>,UPD:<YYYYMMDD>,NAME:<名称>` | プローブ ID（新品は FFFFFFFF）、最終更新日（0=未設定）、製品名 |
| `RAD,SPAN:s1,s2,s3,s4,s5` | ADC span 正規化基準 mV（ch1〜5。既定 1450） |
| `RAD,LED:d0,d1,d2,d3,d4` | LED PWM duty（[0]=MLSS 系、[1]=SS／透視度系、[2..4]=予備。既定 0.36） |
| `RAD,MISC,KDEP:k,SADA:n,SADC:n,EQ:n,MODE:n` | 水深係数 [m/hPa]（既定 0.01）、移動平均件数、ADC ch マスク、選択相関式 No.、測定モード |
| `RAD,MLSS,ZRS:z1,ZRF:z2,RSV:r,b,b2` | MLSS ゼロ mV: 出荷時 z1／現場 z2（既定 1750）。RSV 以降は旧温度補正の予約（現行 0） |
| `RAD,MLSS,C:c0,c1,c2,SP:a,b,c,SV:sv1,sv2` | MLSS ベース式 2 次係数、スパン校正 2 次（恒等 = 0,1,0）、校正設定値（既定 8000／4000） |
| `RAD,MEQ,21:q0,q1,q2` 〜 `RAD,MEQ,30:…` | 相関式 No.21〜30 の 2 次係数（10 行） |
| `RAD,SS,ZRS:z1,ZRF:z2,RSV:r,b,b2` | SS ゼロ mV（MLSS と同形） |
| `RAD,SS,C:c0,c1,c2,SP:a,b,c,SV:sv1,sv2` | SS ベース式・スパン校正・校正設定値（既定 1000／500） |
| `RAD,SEQ,21:q0,q1,q2` | SS No.21 の 2 次係数 |
| `RAD,TR,ZRS:z1,ZRF:z2,Q:Q0,Q1,Q2,Q3` | 透視度ゼロ mV（出荷時／現場）と 4 点 3 次式係数 cm = Q0 + Q1·ABS + Q2·ABS² + Q3·ABS³（捕捉点から再計算した値） |
| `RAD,TR,SP:a,b,c,SV:sv1,sv2` | 透視度スパン校正（k 1 点方式では SP_B = k）、校正設定値 |
| `RAD,TR,P90,X:mV,Y:cm` | 透視度 出荷時 90cm 点（受光 mV、設定値 cm） |
| `RAD,SHIP,<M\|S\|T>,X:x0,x1,x2,Y:y0,y1,y2` | 出荷時 3 点調整。x=受光 mV、y=設定値。[0]=ゼロ（MLSS／SS は未使用、透視度は清水）、[1]=スパン、[2]=中間 |
| `RAD,CAL,<M\|S\|T>,V:vv,X:x0,x1,Y:y0,y1` | 現場スパン校正の捕捉点。x=生 mV、y=基準値。vv: bit0=フル点あり、bit1=中間点あり |
| `RAD,TC,<M\|S>,V:vv,REF:r5,r20,r35,JUKO:j5,j20,j35` | 温度補正の捕捉点（5／20／35℃ の Ref mV と受光 mV）。vv: bit0〜2 = 各温度の捕捉済み |
| `RAD,END` | 終端 |

## 5. 補足

- `RAD` はプローブ flash の統合ストア（25 ページ）に保存される全項目を出力する。相関式 No.1〜20 は FW 固定定数のためプローブには無く、出力されない。
- 出力は本体 RAM の現在値。起動直後（`BOOT:0`）で、かつ校正・調整を操作していなければプローブ flash の内容と同一。
- プローブを新品状態へ戻すコマンドは `P_RPF`（全係数を既定値へ。破壊的）。
