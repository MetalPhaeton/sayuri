**English version of this file is in the second half of this file.**  
[Jump to English version](#sayuri-as-static-library)

Sayuriを静的ライブラリとして使う
================================

目次
----

* [概要](#概要)
* [予備知識](#予備知識)
* [使い方](#使い方)
* [探索アルゴリズムのカスタマイズ](#探索アルゴリズムのカスタマイズ)
* [評価関数のカスタマイズ](#評価関数のカスタマイズ)



概要
----

Sayuriのソースコードを静的ライブラリとして使うことにより、
あなたのプログラムにSayuriを埋め込むことができます。

Sayuriは「MIT License」ソフトウェアです。  
Sayuriの「`LICENSE`」ファイルの内容をあなたのソフトウェアのREADMEファイル、
あるいはLICENSEファイルにコピペする等でSayuriを自由に使用することができます。

Sayuriの特徴は探索アルゴリズムや評価関数を
「 **Sayuriのソースコードを変更することなく** 」カスタマイズすることができる
ことです。



予備知識
--------

* Sayuriは「C++11」で書かれています。なので、「C++11」をサポートした
  コンパイラを使う必要があります。

* Sayuriはマルチスレッド機能を使ったソフトウェアです。  
  コンパイルの際は「`pthread`」など、マルチスレッドライブラリを
  リンクする必要があります。

* 静的ライブラリとして利用するので、「`src`」内の「`main.cpp`」は使いません。  
  コンパイルの際の邪魔になるので、削除するか別の場所に移動させておいて下さい。

* 全てのソースファイル、ヘッダファイルは「`src`」内にあります。
  それ以外は使用しません。  
  ヘッダファイルは「`sayuri.h`」にまとめているので、
  このファイルをインクルードすれば全ての機能を使用することができます。

* Sayuriのネームスペースは「`Sayuri`」です。

* Sayuriは静的ライブラリの場合でもUCIコマンドで操作します。  
  Sayuriからの出力もUCIコマンドですので、UCIコマンドを解析する必要があります。

* SayuriをCMakeでコンパイルした場合、作成された「`libsayurilib.a`」や
  「`include`」ディレクトリ内のヘッダファイルを使うことで同様のことができます。



使い方
------

### 準備 ###

Sayuriを使用したいソースコード内で「`sayuri.h`」をインクルードします。

``` cpp
// Sayuriをインクルード。
#include "sayuri.h"
```

ライブラリを使用する前にライブラリを初期化します。  
(注) ライブラリを使用する前に「 **必ず** 」初期化して下さい。
初期化をしないとおかしな動作をしてしまいます。

``` cpp
// Sayuriを初期化。
Sayuri::Init();
```

以下の2つのパラメータ用オブジェクトを作成します。

| オブジェクト   | 意味                               |
|----------------|------------------------------------|
| `SearchParams` | 探索アルゴリズムを操作する設定値。 |
| `EvalParams`   | 局面評価に関する設定値。           |

``` cpp
// SearchParamsを作成。
std::unique_ptr<Sayuri::SearchParams>
search_params_ptr(new Sayuri::SearchParams());

// EvalParamsを作成。
std::unique_ptr<Sayuri::EvalParams>
eval_params_ptr(new Sayuri::EvalParams());
```

「`ChessEngine`」オブジェクトを作り、上記2つのオブジェクトを関連付けます。

| オブジェクト  | 意味                                              |
|---------------|---------------------------------------------------|
| `ChessEngine` | 次の一手を思考するオブジェクト。 チェスエンジン。 |

``` cpp
// ChessEngineを作り、パラメータを関連付ける。
std::unique_ptr<Sayuri::ChessEngine>
engine_ptr(new Sayuri::ChessEngine(*search_params_ptr, *eval_params_ptr));
```

「`UCIShell`」オブジェクトを作り、エンジンを関連付けます。

| オブジェクト | 意味                                                |
|--------------|-----------------------------------------------------|
| `UCIShell`   | UCIコマンドでチェスエンジンを操作するオブジェクト。 |

``` cpp
// UCIShellを作り、エンジンを関連付ける。
std::unique_ptr<Sayuri::UCIShell>
shell_ptr(new Sayuri::UCIShell(*engine_ptr));
```

チェスエンジンからの出力を受け取るコールバック関数を作成し、
「`UCIShell`」に登録します。

* コールバック関数を登録する関数。  
  `void
  UCIShell::AddOutputListener(std::function<void(const std::string&)> func)`

* コールバック関数の型。  
  `void MyFunction(const std::string& output)`

``` cpp
// コールバック関数を登録する。
shell_ptr->AddOutputListener(MyFunction);
```

以上で準備は完了です。

### ライブラリの操作 ###

Sayuriは「`UCIShell`」にUCIコマンドを送って操作します。

* 「`UCIShell`」にコマンドを送るための関数。  
  `void UCIShell::InputCommand(const string& command)`

``` cpp
// コマンドを作成。
std::string cmd = "uci";

// コマンドを実行。
shell_ptr->InputCommand(cmd);

// 登録したMyFunction()が呼び出され、以下の出力が得られる。
//
// id name Sayuri ****.**.**
// id author Hironori Ishibashi
// option name Hash type spin default 32 min 8 max 8192
// option name Clear Hash type button
// option name Ponder type check default true
// option name Threads type spin default 1 min 1 max 64
// option name UCI_AnalyseMode type check default false
// uciok
```

以上のように、UCIコマンドを送受信してライブラリを操作します。

### 具体例 ###

``` cpp
#include <iostream>
#include <string>
#include <memory>

#include "sayuri.h"  // Sayuriライブラリのヘッダファイル。

// チェスエンジンからの出力を受け取るコールバック関数。
// この例では出力をそのまま標準出力に流している。
void Print(const std::string& message) {
    std::cout << message << std::endl;
}

// メイン。
int main(int argc, char* argv[]) {
    /********/
    /* 準備 */
    /********/
    // ライブラリの初期化。必ずやる。
    Sayuri::Init();

    // SearchParamsの作成。
    std::unique_ptr<Sayuri::SearchParams>
    search_params_ptr(new Sayuri::SearchParams());

    // EvalParamsの作成。
    std::unique_ptr<Sayuri::EvalParams>
    eval_params_ptr(new Sayuri::EvalParams());

    // ChessEngineの作成とパラメータの関連付け。
    std::unique_ptr<Sayuri::ChessEngine>
    engine_ptr(new Sayuri::ChessEngine(search_params_ptr, eval_params_ptr));

    // UCIShellの作成とチェスエンジンの関連付け。
    std::unique_ptr<Sayuri::UCIShell>
    shell_ptr(new Sayuri::UCIShell(*engine_ptr));

    // チェスエンジンからの出力を受け取るコールバック関数を登録。
    shell_ptr->AddOutputListener(Print);

    /********************/
    /* ライブラリの操作 */
    /********************/
    // InputCommand()でライブラリに指示する。

    shell_ptr->InputCommand("uci");
    // ID、オプション、"uciok"がPrint()に送られる。

    shell_ptr->InputCommand("isready");
    // "readyok"がPrint()に送られる。

    shell_ptr->InputCommand("ucinewgame");
    shell_ptr->InputCommand("isready");
    // 新しいゲームを始めるための準備をする。
    // "readyok"がPrint()に送られる。

    shell_ptr->InputCommand("position startpos");
    shell_ptr->InputCommand("isready");
    // 次の手を考えるための局面をセットする。
    // この例では"startpos"なので、初期配置。
    // "readyok"がPrint()に送られる。

    shell_ptr->InputCommand("go depth 10");
    // 深さ 10 プライまで探索する。
    // 別スレッドで探索するので、制御がすぐに戻ってくる。
    // 探索中のプロセスがPrint()に送られる。
    // 探索終了すれば"bestmove"コマンドと共に最善手がPrint()に送られる。

    // 標準入力からのコマンドを受け付ける。
    while (true) {
        std::string input;
        std::getline(std::cin, input);
        if (input == "quit") break;
        else shell_ptr->InputCommand(input);
    }

    return 0;
}
```



探索アルゴリズムのカスタマイズ
------------------------------

「`SearchParams`」は探索アルゴリズムを設定します。

アクセサ、ミューテータで設定値を取得、変更します。

デフォルトでは、作者の勘でそれなりの強さに設定されています。

以下では分かりやすいようにアクセサを例にとって説明します。  
ミューテータを使用したい場合は以下の表を参考に書き換えて下さい。

| 種類             | 形                              |
|------------------|---------------------------------|
| アクセサの型     | `型 パラメータ名()`             |
| ミューテータの型 | `void パラメータ名(型　変数名)` |

配列のインデックスで使う「`駒の種類`」、「`位置`」に入れる値は以下の通りです。

| 種類       | 入れる値                                                     |
|------------|--------------------------------------------------------------|
| `駒の種類` | `EMPTY`、`PAWN`、`KNIGHT`、`BISHOP`、`ROOK`、`QUEEN`、`KING` |
| `位置`     | `A1`、`B1`、`C1` ...... `F8`、`G8`、`H8`                     |

(注) 綴りを間違えてしまっているかもしれません。  
上手く機能しない場合は、「`params.h`」を見て確認して下さい。

###### マテリアル ######

* `int (& material())[駒の種類]`
    * 駒の価値。  
      チェスエンジンにおいて最も重要な数値。  
      探索アルゴリズムだけでなく、評価関数でも極めて重要な要素。

###### YBWC ######

* `int ybwc_limit_depth()`
    * 「`現在のdepth >= 設定値`」で YBWC を実行する。

* `int ybwc_after()`
    * 現在の局面の[設定値]番目以降の候補手から別スレッドに助けを求める。

###### Aspiration Windows ######

* `bool enable_aspiration_windows()`
    * Aspiration Windowsが有効かどうかの設定値。

* `std::uint32_t aspiration_windows_limit_depth()`
    * 「`現在のdepth >= 設定値`」で Aspiration Windows を実行する。

* `int aspiration_windows_delta()`
    * この設定値をベースにウィンドウのサイズを広げていく。

###### SEE ######

* `bool enable_see()`
    * Move OrderingでSEEが有効かどうかの設定値。

###### History Heuristic ######

* `bool enable_history()`
    * History Heuristicが有効かどうかの設定値。

###### Killer Move Heuristic ######

* `bool enable_killer()`
    * Killer Move Heuristicが有効かどうかの設定値。

* `bool enable_killer_2()`
    * 2プライ先のキラームーブが有効かどうかの設定値。  
      「`enable_killer`」が有効でないと有効にならない。

###### Transposition Table ######

* `bool enable_ttable()`
    * Transposition Tableが有効かどうかの設定値。

###### Internal Iterative Deepening ######

* `bool enable_iid()`
    * IIDが有効かどうかの設定値。

* `int iid_limit_depth()`
    * 「`現在のdepth >= 設定値`」で IID を実行する。

* `int iid_search_depth()`
    * IIDで探索する深さ。

###### Null Move Reduction ######

* `bool enable_nmr()`
    * NMRが有効かどうかの設定値。

* `int nmr_limit_depth()`
    * 「`現在のdepth >= 設定値`」で NMR を実行する。

* `int nmr_search_reduction()`
    * 現在のノードの深さより、どれだけ浅く探索するかの設定値。

* `int nmr_reduction()`
    * NMRでベータ値を超えた際、現在の深さを減らす量。

###### ProbCut ######

* `bool enable_probcut()`
    * ProbCutが有効かどうかの設定値。

* `int probcut_limit_depth()`
    * 「`現在のdepth >= 設定値`」で ProbCut を実行する。

* `int probcut_margin()`
    * ProbCutをする際のベータ値増量分。

* `int probcut_search_reduction()`
    * 現在のノードの深さより、どれだけ浅く探索するかの設定値。

###### History Pruning ######

* `bool enable_history_pruning()`
    * History Pruningが有効かどうかの設定値。  
      History Heuristicが有効でないと有効にならない。

* `int history_pruning_limit_depth()`
    * 「`現在のdepth >= 設定値`」で History Pruning を実行する。

* `double history_pruning_move_threshold()`
    * 候補手に対して、History Pruningするかしないかの閾値。  
      「`X = 設定値 * 全候補手の数`」で、X 番目以降の候補手に対して実行する。  
      0.0から1.0の間で設定する。

* `int history_pruning_after()`
    * 「`history_pruning_move_threshold`」がどんなに小さくても、
      [設定値]番目の候補手まではHistory Pruningしない。

* `double history_pruning_threshold()`
    * ヒストリー値に対して、History Pruningするかしないかの閾値。  
      「`X = 設定値 * 最大ヒストリー`」で、ヒストリー値が X 以下の手なら
      探索する深さを減らす。  
      0.0から1.0の間で設定する。

* `int history_pruning_reduction()`
    * 閾値以下のヒストリー値の手の、深さを減らす量。

###### Late Move Reduction ######

* `bool enable_lmr()`
    * LMRが有効かどうかの設定値。  

* `int lmr_limit_depth()`
    * 「`現在のdepth >= 設定値`」で LMR を実行する。

* `int lmr_threshold()`
    * 全候補手の数に対して、LMRするかしないかの閾値。  
      「`X = 設定値 * 全候補手の数`」で、X 番目以降の候補手に対して実行する。  
      0.0から1.0の間で設定する。

* `int lmr_after()`
    * 「`lmr_threshold`」がどんなに小さくても、
      [設定値]番目の候補手まではLMRしない。

* `int lmr_search_reduction()`
    * 現在のノードの深さより、どれだけ浅く探索するかの設定値。

###### Futility Pruning ######

* `bool enable_futility_pruning()`
    * 「`現在のdepth <= 設定値`」で Futility Pruning を実行する。

* `int futility_pruning_depth()`
    * 設定値以下の残り深さでFutility Pruningを実行する。

* `int futility_pruning_margin()`
    * Futility Pruningの1プライあたりのマージン。  
      「`X = 設定値 * 残り深さ`」がそのノードでのマージンとなる。



評価関数のカスタマイズ
----------------------

「`EvalParams`」は葉ノードにおける局面の、各戦略要素の評価の重みを設定します。

評価値は次の計算式で計算されます。  
"`the_score = (weight_1 * value_1) + (weight_2 * value_2) + ...`".

アクセサ、ミューテータで設定値を取得、変更します。

デフォルトでは、作者のチェス経験を元に設定されています。

以下では分かりやすいようにアクセサを例にとって説明します。  
ミューテータを使用したい場合は以下の表を参考に書き換えて下さい。

| 種類             | 形                              |
|------------------|---------------------------------|
| アクセサの型     | `型　パラメータ名()`            |
| ミューテータの型 | `void パラメータ名(型　変数名)` |

配列のインデックスで使う「`駒の種類`」、「`位置`」に入れる値は以下の通りです。

| 種類       | 入れる値                                                     |
|------------|--------------------------------------------------------------|
| `駒の種類` | `EMPTY`、`PAWN`、`KNIGHT`、`BISHOP`、`ROOK`、`QUEEN`、`KING` |
| `位置`     | `A1`、`B1`、`C1` ...... `F8`、`G8`、`H8`                     |

(注) 綴りを間違えてしまっているかもしれません。  
上手く機能しない場合は、「`params.h`」を見て確認して下さい。

### 値テーブル ###

値テーブルは駒の位置の値、攻撃する駒種類の値を定義した配列です。  
「`値 * ウェイト`」が実際の評価値となります。

* `double (& opening_position_value_table())[駒の種類][位置]`
    * オープニング時における「`駒の種類`」が「`位置`」にいる時の値です。

* `double (& ending_position_value_table())[駒の種類][位置]`
    * エンディング時における「`駒の種類`」が「`位置`」にいる時の値です。

* `double (& attack_value_table())[攻撃側の駒の種類][対象の駒の種類]`
    * 「`攻撃側の駒の種類`」が「`対象の駒の種類`」を
      攻撃している時の値です。

* `double (& pawn_shield_value_table())[位置]`
    * キングを守るべきポーンが「`位置`」にいる時の値です。  

キングを守るべきポーンの定義

| キングの位置                      | 対象となるポーン         |
|-----------------------------------|--------------------------|
| 第1、第2ランクの A、B、C ファイル | A、B、C ファイルのポーン |
| 第1、第2ランクの F、G、H ファイル | F、G、H ファイルのポーン |

### Weightオブジェクト ###

「`Weight`」は評価関数の値に掛け合わせる「重み」を計算するオブジェクトです。

オープニング時(駒が32個の状態)の設定値と、
エンディング時(駒がキングのみの状態)の設定値から、現在の駒の数を変数とした
一次関数で計算します。

* `double opening_weight()`
    * オープニング時(駒が32個の状態)の設定値。

* `double ending_weight()`
    * エンディング時(駒がキングのみの状態)の設定値。

### 各種ウェイト ###

「`EvalParams`」内の、戦略要素ごとの「重み」です。  
全て「`Weight`」オブジェクトです。

* `Weight (& weight_opening_position())[駒の種類]`
    * 「`double (& opening_position_value_table())[駒の種類][位置]`」
      にかけ合わせる重みです。  
      「`駒の種類`」ごとに設定します。

* `Weight (& weight_ending_position())[駒の種類]`
    * 「`double (& ending_position_value_table())[駒の種類][位置]`」
      にかけ合わせる重みです。  
      「`駒の種類`」ごとに設定します。

* `Weight& weight_mobility()`
    * 駒の機動力に対する重みです。

* `Weight& weight_center_control()`
    * c3、f3、c6、f6 を四隅とする矩形領域のコントロールに対する重みです。

* `Weight& weight_sweet_center_control()`
    * d4、e4、d5、e5 のコントロールに対する重みです。

* `Weight& weight_development()`
    * ナイト、ビショップの展開(初期位置にいない状態)に対する重みです。

* `Weight& (& weight_attack())[攻撃側の駒の種類]`
    * 「`double (& attack_value_table())[攻撃側の駒の種類][対象の駒の種類]`」
      に対する重みです。  
      攻撃側の駒の種類ごとに設定します。

* `Weight& weight_attack_around_king()`
    * 相手のキング周辺への攻撃に対する重みです。

* `Weight& weight_pass_pawn()`
    * パスポーンに対する重みです。

* `Weight& weight_protected_pass_pawn()`
    * 守られたパスポーンに対する重みです。  
      「`weight_pass_pawn`」に上乗せする形になります。

* `Weight& weight_double_pawn()`
    * ダブルポーンに対する重みです。  
      戦略的にペナルティになるので、マイナス値を設定するのがいいと思います。

* `Weight& weight_iso_pawn()`
    * 孤立ポーンに対する重みです。  
      戦略的にペナルティになるので、マイナス値を設定するのがいいと思います。

* `Weight& weight_pawn_shield()`
    * 「`double (& pawn_shield_value_table())[位置]`」に対する重みです。

* `Weight& weight_bishop_pair()`
    * ビショップが2つ以上存在していた場合に対する重みです。  
      3つ以上あっても2つの時と同じ評価値になります。

* `Weight& weight_bad_bishop()`
    * バッドビショップに対する重みです。  
      戦略的にペナルティになるので、マイナス値を設定するのがいいと思います。

* `Weight& weight_pin_knight()`
    * ビショップが相手のナイトを相手のルーク、クイーン、キングに
      ピンした時に対する重みです。

* `Weight& weight_rook_pair()`
    * ルークが2つ以上存在していた場合に対する重みです。  
      3つ以上あっても2つの時と同じ評価値になります。

* `Weight& weight_rook_semiopen_fyle()`
    * ルークがセミオープンファイルにいた場合に対する重みです。  
      作者の都合上、綴りが「`fyle`」になっています。 ミスではありません。

* `Weight& weight_rook_open_fyle()`
    * ルークがオープンファイルにいた場合に対する重みです。  
      作者の都合上、綴りが「`fyle`」になっています。 ミスではありません。

* `Weight& weight_early_queen_launched()`
    * クイーンの展開が早すぎた場合に対する重みです。  
      ナイトやビショップが展開された数で早すぎるかどうかを判断します。  
      戦略的にペナルティになるので、マイナス値を設定するのがいいと思います。

* `Weight& weight_weak_square()`
    * キング周りの弱いマスに対する重みです。  
      相手のビショップがいるマスの色と、キング周りのポーンの配置で
      弱さを計算します。  
      戦略的にペナルティになるので、マイナス値を設定するのがいいと思います。

* `Weight& weight_castling()`
    * キャスリングに対する重みです。  
      ゲーム中でキャスリングしたかどうかで判断します。  
      すでにキャスリングされた状態からゲームが始まった場合は、
      キャスリングを放棄したものとしてみなします。

* `Weight& weight_abandoned_castling()`
    * キャスリングの放棄に対する重みです。  
      ゲーム中でキャスリングしたかどうかで判断します。  
      すでにキャスリングされた状態からゲームが始まった場合は、
      キャスリングを放棄したものとしてみなします。  
      戦略的にペナルティになるので、マイナス値を設定するのがいいと思います。

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -



Sayuri as Static Library
========================

Index
-----

* [Summary](#summary)
* [Background](#background)
* [Usage](#usage)
* [To Customize of Search Algorithm](#to-customize-of-search-algorithm)
* [To Customize of Evaluation Function](#to-customize-of-evaluation-function)



Summary
-------

To use Sayuri's source code as static library, you can embed it into your
program.

Sayuri is "MIT License" software.  
To copy and paste the content of Sayuri's "`LICENSE`" file to your
README or LICENSE file, you can freely use Sayuri.

Sayuri's special feature is that you can customize the search algorithm or
the evaluation function **without changing Sayuri's source code** .



Background
----------

* Sayuri is written in C++11. So you have to use a compiler supporting C++11
  language.

* Sayuri uses a function of multi-threading of OS.  
  When you compile Sayuri, you have to link multi-threading library to it.

* Using it as static library, you don't need "`main.cpp`" in
 "`src`" diractory.  
 Because it gets in the way of compiling your program,
 you should delete the file or move it to elsewhere.

* All source files and header file are placed in "`src`" directory.  
  It doesn't need the other files.  
  You can use all the functions of Sayuri if you include "`sayuri.h`"

* The namespace of Sayuri is "`Sayuri`".

* When you use Sayuri as static library, you need to use UCI commands to
  operate Sayuri.

* If you have compiled Sayuri with CMake, you can do the same to use
  "`libsayurilib.a`" or "`include`".



Usage
-----

### Preparation ###

Include "`sayuri.h`".

``` cpp
// Include Sayuri.
#include "sayuri.h"
```

Initialize library before using it.  
Note: You " **must** " initialize Sayuri before using it.
If you don't do so, it won't work well.

``` cpp
// Initialize Sayuri.
Sayuri::Init();
```

Create the following 2 objects for parameters.

| Object name    | Meaning                                    |
|----------------|--------------------------------------------|
| `SearchParams` | The parameters for the search algorithm.   |
| `EvalParams`   | The parameters of the evaluation function. |

``` cpp
// Create SearchParams.
std::unique_ptr<Sayuri::SearchParams>
search_params_ptr(new Sayuri::SearchParams());

// Create EvalParams.
std::unique_ptr<Sayuri::EvalParams>
eval_params_ptr(new Sayuri::EvalParams());
```

Create "`ChessEngine`" object and relate the above 2 objects to it.

| Object name   | Meaning                                             |
|---------------|-----------------------------------------------------|
| `ChessEngine` | A object to thinking the next move. A chess engine. |

``` cpp
// Create ChessEngine and relate the parameter objects to it.
std::unique_ptr<Sayuri::ChessEngine>
engine_ptr(new Sayuri::ChessEngine(*search_params_ptr, *eval_params_ptr));
```

Create "`UCIShell`" object and relate the engine to it.

| Object name | Meaning                                           |
|-------------|---------------------------------------------------|
| `UCIShell`  | A object to operate the engine with UCI Commands. |

``` cpp
// Create UCIShell and relate the engine to it.
std::unique_ptr<Sayuri::UCIShell>
shell_ptr(new Sayuri::UCIShell(*engine_ptr));
```

Create a call-back function and register it to the "`UCIShell`".

* A function to register a call-back function.  
  `void
  UCIShell::AddOutputListener(std::function<void(const std::string&)> func)`

* Type of the call-back function.  
  `void MyFunction(const std::string& output)`

``` cpp
// Register the call-back function.
shell_ptr->AddOutputListener(MyFunction);
```

### To operate the library ###

To operate Sayuri, you send UCI commands to "`UCIShell`".

* A function to send the commands to "`UCIShell`".  
  `void UCIShell::InputCommand(const string& command)`

``` cpp
// Create a command.
std::string cmd = "uci";

// Run the command.
shell_ptr->InputCommand(cmd);

// Sayuri calls the registered functions and send it the following outputs.
//
// id name Sayuri ****.**.**
// id author Hironori Ishibashi
// option name Hash type spin default 32 min 8 max 8192
// option name Clear Hash type button
// option name Ponder type check default true
// option name Threads type spin default 1 min 1 max 64
// option name UCI_AnalyseMode type check default false
// uciok
```

### Example ###

``` cpp
#include <iostream>
#include <string>
#include <memory>

#include "sayuri.h"  // The header file for the library.

// The call-back function to recive outputs from the engine.
// In this example, the function send the outputs to stdout.
void Print(const std::string& message) {
    std::cout << message << std::endl;
}

int main(int argc, char* argv[]) {
    /***************/
    /* Preparation */
    /***************/
    // Initialize the library. you must do it.
    Sayuri::Init();

    // Create SearchParams.
    std::unique_ptr<Sayuri::SearchParams>
    search_params_ptr(new Sayuri::SearchParams());

    // Create EvalParams.
    std::unique_ptr<Sayuri::EvalParams>
    eval_params_ptr(new Sayuri::EvalParams());

    // Create ChessEngine and relate the 2 parameters object to it.
    std::unique_ptr<Sayuri::ChessEngine>
    engine_ptr(new Sayuri::ChessEngine(search_params_ptr, eval_params_ptr));

    // Create UCIShell and relate the engine to it.
    std::unique_ptr<Sayuri::UCIShell>
    shell_ptr(new Sayuri::UCIShell(*engine_ptr));

    // Register the call-back function to the shell.
    shell_ptr->AddOutputListener(Print);

    /**********************/
    /* Operate the engine */
    /**********************/
    // Command it to the library with InputCommand().

    shell_ptr->InputCommand("uci");
    // The engine sends ID, the options, and "uciok" to Print().

    shell_ptr->InputCommand("isready");
    // The engine sends "readyok" message to Print().

    shell_ptr->InputCommand("ucinewgame");
    shell_ptr->InputCommand("isready");
    // Prepare to start the new game.
    // The engine sends "readyok" message to Print().

    shell_ptr->InputCommand("position startpos");
    shell_ptr->InputCommand("isready");
    // Set the position to thinking the next move.
    // In this example, the position is "startpos", so it's starting position.
    // The engine sends "readyok" message to Print().

    shell_ptr->InputCommand("go depth 10");
    // Search until the depth of 10 plies.
    // Return the control soon because the engine is thinking
    // on the other thread.
    // The process log is sent to Print().
    // After finishing searching, the engine sends the best move to Print()
    // with "bestmove" command.

    // Recive commands from stdin.
    while (true) {
        std::string input;
        std::getline(std::cin, input);
        if (input == "quit") break;
        else shell_ptr->InputCommand(input);
    }

    return 0;
}
```



To Customize of Search Algorithm
--------------------------------

"`SearchParams`" sets parameters to the search algorithm.

To use an accessor or a mutator, you can get or change the parameters.

As default, it's set by the author's intuition.

I explain with an accessor.  
If you want to use the mutator, translate it with the following table.

| Type of functions   | Skeleton                       |
|---------------------|--------------------------------|
| Type of an accessor | `Type ParameterName()`         |
| Type of a mutator   | `void ParameterName(Type arg)` |

The following table is the index value of the array for `[piece type]` or
`[square]`.

| Name         | Index value                                                  |
|--------------|--------------------------------------------------------------|
| `piece type` | `EMPTY`, `PAWN`, `KNIGHT`, `BISHOP`, `ROOK`, `QUEEN`, `KING` |
| `square`     | `A1`, `B1`, `C1` ...... `F8`, `G8`, `H8`                     |

Note: I may mistake spells.  
If it doesn't work, please confirm "`param.h`".

###### Material ######

* `int (& material())[piece type]`
    * The values of a piece.  
      This is the most important numbers of chess engines.  
      Not only for search algorithm, but also evaluation function.

###### YBWC ######

* `int ybwc_limit_depth()`
    * Execute YBWC when "`the_current_depth >= the_parameter`".

* `int ybwc_after()`
    * Call for help to other thread after [the parameter]th candidate move.

###### Aspiration Windows ######

* `bool enable_aspiration_windows()`
    * A parameter whether it enables Aspiration Windows or not.

* `std::uint32_t aspiration_windows_limit_depth()`
    * Execute Aspiration Windows when "`the_current_depth >= the_parameter`".

* `int aspiration_windows_delta()`
    * It expands the window size based on the parameter.

###### SEE ######

* `bool enable_see()`
    * A parameter whether it enables SEE or not.

###### History Heuristic ######

* `bool enable_history()`
    * A parameter whether it enables History Heuristic or not.

###### Killer Move Heuristic ######

* `bool enable_killer()`
    * A parameter whether it enables Killer Move Heuristic or not.

* `bool enable_killer_2()`
    * A parameter whether it enables Killer Move Heuristic
      after 2 plies or not.  
      If "`enable_killer`" is not enabled, it won't be enabled.

###### Transposition Table ######

* `bool enable_ttable()`
    * A parameter whether it enables Transposition Table or not.

###### Internal Iterative Deepening ######

* `bool enable_iid()`
    * A parameter whether it enables IID or not.

* `int iid_limit_depth()`
    * Execute IID when "`the_current_depth >= the_parameter`".

* `int iid_search_depth()`
    * The searching depth in IID.

###### Null Move Reduction ######

* `bool enable_nmr()`
    * A parameter whether it enables NMR or not.

* `int nmr_limit_depth()`
    * Execute NMR when "`the_current_depth >= the_parameter`".

* `int nmr_search_reduction()`
    * A parameter how many reduce the current depth when NMR searches.

* `int nmr_reduction()`
    * The reduction of depth by NMR.

###### ProbCut ######

* `bool enable_probcut()`
    * A parameter whether it enables ProbCut or not.

* `int probcut_limit_depth()`
    * Execute ProbCut when "`the_current_depth >= the_parameter`".

* `int probcut_margin()`
    * A incremental value of the beta value when ProbCut searches.

* `int probcut_search_reduction()`
    * A parameter how many reduce the current depth when ProbCut searches.

###### History Pruning ######

* `bool enable_history_pruning()`
    * A parameter whether it enables History Pruning or not.  
      If History Heuristic is not enabled, this parameter won't be enable.

* `int history_pruning_limit_depth()`
    * Execute History Pruning when "`the_current_depth >= the_parameter`".

* `double history_pruning_move_threshold()`
    * The threshold whether it executes History Pruning or not.  
      "`X = the_parameter * number_of_all_candidate_moves`", it executes
      History Pruning after [X]th candidate move.  
      This parameter is set from 0.0 to 1.0.

* `int history_pruning_after()`
    * Even if "`history_pruning_move_threshold`" is too few, it doesn't do
      History Pruning until [the parameter]th candidate move.

* `double history_pruning_threshold()`
    * The threshold whether it executes History Pruning or not.  
      "`X = the_parameter * the_max_history_value`", it executes
      History Pruning to a move which has history value less than X.  
      This parameter is set from 0.0 to 1.0.

* `int history_pruning_reduction()`
    * The reduction of depth by History Pruning.

###### Late Move Reduction ######

* `bool enable_lmr()`
    * A parameter whether it enables LMR or not.  

* `int lmr_limit_depth()`
    * Execute LMR when "`the_current_depth >= the_parameter`".

* `int lmr_threshold()`
    * The threshold whether it executes LMR or not.  
      "`X = the_parameter * the_number_of_candidate_moves`", it executes
      LMR after [X]th candidate move.  
      This parameter is set from 0.0 to 1.0.

* `int lmr_after()`
    * Even if "`lmr_threshold`" is too few, it doesn't do
      LMR until [the parameter]th candidate move.

* `int lmr_search_reduction()`
    * A parameter how many reduce the current depth when LMR searches.

###### Futility Pruning ######

* `bool enable_futility_pruning()`
    * A parameter whether it enables Futility Pruning or not.  

* `int futility_pruning_depth()`
    * Execute Futility Pruning by "`the_current_depth <= the_parameter`".

* `int futility_pruning_margin()`
    * The margin per 1 ply of depth.  
      "`the_margin = the_parameter * the_current_depth`"



To Customize of Evaluation Function
-----------------------------------

"`EvalParams`" sets parameters for the evaluation function.

The evaluated score is defined by
"`the_score = (weight_1 * value_1) + (weight_2 * value_2) + ...`".

To use an accessor or a mutator, you can get or change the parameters.

As default, it's set by the author's experience as a chess player.

I explain with an accessor.  
If you want to use the mutator, translate it with the following table.

| Type of functions   | Skeleton                       |
|---------------------|--------------------------------|
| Type of an accessor | `Type ParameterName()`         |
| Type of a mutator   | `void ParameterName(Type arg)` |

The following table is the index value of the array for `[piece type]` or
`[square]`.

| Name         | Index value                                                  |
|--------------|--------------------------------------------------------------|
| `piece type` | `EMPTY`, `PAWN`, `KNIGHT`, `BISHOP`, `ROOK`, `QUEEN`, `KING` |
| `square`     | `A1`, `B1`, `C1` ...... `F8`, `G8`, `H8`                     |

Note: I may mistake spells.  
If it doesn't work, please confirm "`param.h`".

### Value tables ###

The value tables are array that defines the value of a place for the piece
or the value of the attacker.  

* `double (& opening_position_value_table())[piece type][square]`
    * The values that a piece of [piece type] is at [square] when the opening.

* `double (& ending_position_value_table())[piece type][square]`
    * The values that a piece of [piece type] is at [square] when the ending.

* `double (& attack_value_table())[attacker type][target type]`
    * The values when a piece of [attacker type] attacks the another piece of
      [target type].

* `double (& pawn_shield_value_table())[square]`
    * The values that a Pawn which has to guard its King is at [square].

The definition of Pawns which have to guard its King.

| The place of the King                | The Pawns                  |
|--------------------------------------|----------------------------|
| The 1st or 2nd rank in A, B, C file. | The Pawns on A, B, C file. |
| The 1st or 2nd rank in F, G, H file. | The Pawns on F, G, H file. |

### Weight object ###

"`Weight`" is a object that calculates the weight of the score.

It calculates the weight by linear function using the 2 parameters
when the opening (there are 32 pieces on the board) and when the ending
(there are 2 Kings only on the board).

* `double opening_weight()`
    * The weight when the opening (there are 32 pieces on the board).

* `double ending_weight()`
    * The weight when the ending (there are 2 Kings only on the board).

### Each weight ###

The followings are the weights for the elements of the chess strategies.  
All the weights are "`Weight`" object.

* `Weight (& weight_opening_position())[piece type]`
    * The weight for
      "`double (& opening_position_value_table())[piece type][square]`".  
      You can set the weight for each piece type.

* `Weight (& weight_ending_position())[piece type]`
    * The weight for
      "`double (& ending_position_value_table())[piece type][square]`".  
      You can set the weight for each piece type.

* `Weight& weight_mobility()`
    * The weight for the mobility of each piece.

* `Weight& weight_center_control()`
    * The weight for the control of the rectangle to have the corners of
      c3, f3, c6, f6.

* `Weight& weight_sweet_center_control()`
    * The weight for the control of d4, e4, d5, e5.

* `Weight& weight_development()`
    * The weight for the Development (not exist on the starting position)
      of Knights and Bishops.

* `Weight& (& weight_attack())[attacker type]`
    * The weight for
      "`double (& attack_value_table())[attacker type][target type]`".  
      You can set the weight for each piece type as attacker.

* `Weight& weight_attack_around_king()`
    * The weight for the attacks around the opponent's King.

* `Weight& weight_pass_pawn()`
    * The weight for the Pass Pawns.

* `Weight& weight_protected_pass_pawn()`
    * The weight for the Protected Pass Pawns.  
      It adds "`weight_pass_pawn`".

* `Weight& weight_double_pawn()`
    * The weight for the Double Pawns.  
      Because it is defined as the penalty on the chess strategies,
      I think it should be a negative number.

* `Weight& weight_iso_pawn()`
    * The weight for the Isolated Pawns.  
      Because it is defined as the penalty on the chess strategies,
      I think it should be a negative number.

* `Weight& weight_pawn_shield()`
    * The weight for "`double (& pawn_shield_value_table())[square]`".  

* `Weight& weight_bishop_pair()`
    * The weight for more than 2 Bishops existing on the board.  
      Even if more than 3 Bishops exist,
      the score is same as 2 Bishops existing.

* `Weight& weight_bad_bishop()`
    * The weight for the Bad Bishops.  
      Because it is defined as the penalty on the chess strategies,
      I think it should be a negative number.

* `Weight& weight_pin_knight()`
    * The weight for that a Bishop pins the opponent's Knight to his Rook,
      Queen, King.

* `Weight& weight_rook_pair()`
    * The weight for more than 2 Rooks existing on the board.  
      Even if more than 3 Rooks exist, the score is same as 2 Rooks existing.

* `Weight& weight_rook_semiopen_fyle()`
    * The weight for the Rooks on the semi-open file.  
      For some reasons, I write "fyle" instead of "file". It's not mistake. 

* `Weight& weight_rook_open_fyle()`
    * The weight for the Rooks on the open file.  
      For some reasons, I write "fyle" instead of "file". It's not mistake. 

* `Weight& weight_early_queen_launched()`
    * The weight for too early to develop the Queen.  
      It judges by how many the minor pieces have been developed.  
      Because it is defined as the penalty on the chess strategies,
      I think it should be a negative number.

* `Weight& weight_weak_square()`
    * The weight for the weak squares around the King.  
      It is calculated by the pawn structure around the King and
      the color of squares to be the opponent's Bishop.  
      Because it is defined as the penalty on the chess strategies,
      I think it should be a negative number.

* `Weight& weight_castling()`
    * The weight for the castling.  
      It judges whether the King has castled in the game or not.  
      If the game starts with the position that the King already has castled,
      it judges as the King abandoned its castling rights.

* `Weight& weight_abandoned_castling()`
    * The weight for the King abandoned its castling rights.  
      It judges whether the King has castled in the game or not.  
      If the game starts with the position that the King already has castled,
      it judges as the King abandoned its castling rights.  
      Because it is defined as the penalty on the chess strategies,
      I think it should be a negative number.
