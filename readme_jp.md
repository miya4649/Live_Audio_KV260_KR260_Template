# Live Audio Example for AMD (Xilinx) Kria KV260 and KR260

AMD (Xilinx) Kria KV260, KR260用Live Audio サンプル・プロジェクト

## このスクリプトについて

これはAMD (Xilinx) Kria KV260, KR260でLive AudioとLive Video機能（HDMIからの映像、音声出力をFPGA側からコントロールする機能）を使用するサンプル・プロジェクトです。
実行はスタンド・アローン（ベアメタル）モードで行います。
開発環境はVitis / Vivado 2023.2 以降に対応しています。

https://github.com/miya4649/Live_Audio_KV260_KR260_Template/assets/11752987/6a8df6ab-39c3-4977-8055-d6f61f38b995

## 使用方法

デフォルトではKV260用のプロジェクトが生成されます。KR260の場合は、vivado.tclの「set board_type kv260」を「set board_type kr260」に書き換えます。

### makeを使った方法

Linuxのターミナルで

$ source Vitisのインストールパス/settings64.sh

(例: $ source /opt/Xilinx/Vitis/2023.2/settings64.sh )

$ cd このディレクトリのパス

$ make

次に、Vitis Unified IDEを起動し、

File: Open Workspaceで

(このディレクトリ)/vitis_workspace を選択してOK

View: Flow を選択して表示、

FLOW: Component で Project_1_appを選択、

FLOW: Run で実行します。

### IDEを使った方法

Vivadoを起動し、メニュー: Window: Tcl Consoleで

pwd

(カレントディレクトリを確認)

cd このディレクトリのパス

source vivado.tcl

(Vivadoプロジェクト生成スクリプトを実行)

source vivado-run.tcl

(Synthesis, Implement, Export HW実行)

これでVivadoのプロジェクトがproject_1ディレクトリ以下に生成され、ビットストリーム生成、ビットストリーム付きのXSAファイルのエクスポートが行われます。

次にVitis Unified IDEを起動し、そのTerminalで、

$ pwd

(カレントディレクトリを確認)

$ cd このディレクトリのパス

(このディレクトリに移動)

$ vitis -s vitisnew.py

(Vitisプロジェクト生成スクリプトを実行。ビルドも自動実行される)

File: Open Workspaceで

(このディレクトリ)/vitis_workspace を選択してOK

View: Flow を選択して表示、

FLOW: Component で Project_1_appを選択、

FLOW: Run で実行します。

## 補足事項

### RTLのサブモジュールの更新について

rtl_top.v の下位モジュールのソースコードのみを修正した時、Vivado上で更新情報が正常に取得されない場合があります。その場合、rtl_top.v にコメントの追加等でダミーの修正を行うとVivadoで捕捉されて「Refresh Changed Modules」が表示されるのでそれをクリックするとソースツリーの再読込とリセットが行われます。

例: // rev. 1 (2, 3, ...)
