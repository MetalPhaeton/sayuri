**English version of this file is in the second half of this file.**  
[Jump to English version](#sayuri---uci-chess-engine)

Sayuri - UCI用チェスエンジン
============================

目次
----

* [特徴](#特徴)
* [ファイル構成](#ファイル構成)
* [CMakeを使ったビルド](#cmakeを使ったビルド)
* [配布用パッケージの作り方](#配布用パッケージの作り方)
* [UCIオプション](#uciオプション)



特徴
----

* MIT License。
* UCI用チェスエンジン。
* C++11と、その標準ライブラリのみで作成。
* CMakeを使用して簡単にビルドが出来る。  
* 静的ライブラリとして使用可能。
* 「`SearchParams`」クラス、「`EvalParams`」クラスの値を変更することで、
  探索アルゴリズム、評価関数を自由にカスタマイズすることが出来る。



ファイル構成
------------

###### ディレクトリ ######

* `src` : ソースファイルのディレクトリ。
* `SayuriCompiled` : コンパイル済みバイナリファイルのディレクトリ。
    * `Linux` : リナックス用バイナリファイルのディレクトリ。
        * `For64Bit` : 64ビットCPU向けのバイナリファイルのディレクトリ。
* `SayuriLogo` : ロゴ画像ファイルのディレクトリ。
* `SampleGames` : Sayuriとその他のエンジンによるサンプルゲームの
  ファイルのディレクトリ。

###### ファイル ######

* `README.md` : このファイル。
* `CMakeLists.txt` : CMakeのビルド設定ファイル。
* `LICENSE` : 使用許諾書。
* `as_static_library.md` : Sayuriを静的ライブラリとして使う方法。



CMakeを使ったビルド
-------------------

CMakeを使って簡単にビルドすることができます。

使用するコンパイラはデフォルトで「`clang++`」となります。  
(コンパイラを変更したいときは、「`CMakeLists.txt`」の26行目、
27行目のコンパイラ設定を変更してください。)

###### 通常のビルド手順 ######

1. 「`CMakeLists.txt`」のあるディレクトリに移動します。
2. 以下のコマンドを実行します。
    1. `$ mkdir build`
    2. `$ cd build`
    3. `$ cmake ..`
    4. `$ make`
3. 「`build`」ディレクト内に以下のファイル、ディレクトリが完成します。
    * `sayuri` : UCI用チェスエンジン。
    * `libsayurilib.a` : 静的ライブラリ。
    * `include` : 静的ライブラリ用ヘッダファイルの入ったディレクトリ。

###### デバッグモードでのビルド手順 ######

デバッグモードでビルドすと、「最適化なし」、「デバッグ情報の埋め込み」で
ビルドすることができます。

以下の手順2-3で「`-DCMAKE_BUILD_TYPE=Debug`」を挿入するだけです。

1. 「`CMakeLists.txt`」のあるディレクトリに移動します。
2. 以下のコマンドを実行します。
    1. `$ mkdir build`
    2. `$ cd build`
    3. `$ cmake -DCMAKE_BUILD_TYPE=Debug ..`
    4. `$ make`
3. 「`build`」ディレクト内に以下のファイル、ディレクトリが完成します。
    * `sayuri` : UCI用チェスエンジン。
    * `libsayurilib.a` : 静的ライブラリ。
    * `include` : 静的ライブラリ用ヘッダファイルの入ったディレクトリ。

###### CMakeで作成した各ファイルについて ######

「`sayuri`」はUCI用チェスエンジンです。  
このエンジンを使用する場合は、「`libsayurilib.a`」と
「`include`」ディレクトリは必要ありません。

「`libsayurilib.a`」と「`include`」はSayuriを静的ライブラリとして
使用する場合に使います。  
その場合は「`sayuri`」は必要ありません。  
詳しくは「`as_static_library.md`」を読んで下さい。



配布用パッケージの作り方
------------------------

CMakeで「`build`」内で「`Makefile`」を作成後、「`$ make dist`」を実行します。

これで以下の3つの配布用パッケージが出来上がります。

* `sayuri-xxxx.xx.xx.tar.Z`
* `sayuri-xxxx.xx.xx.tar.bz2`
* `sayuri-xxxx.xx.xx.tar.gz`

((注) 「`xxxx.xx.xx`」にはバージョン番号が入ります。)



UCIオプション
-------------

以下のコマンドでエンジンの設定を変更できます。

* ハッシュテーブルのサイズを変更。 (デフォルトは 32。最大は 8192。最小は 8。)  
  `setoption name Hash value <サイズ、メガバイト>`

* ハッシュテーブルを初期化。  
  `setoption name Clear Hash`

* Ponderの有効化、無効化。 (デフォルトは true。)  
  `setoption name Ponder value <true、又はfalse>`

* 探索用スレッドの数を変更。 (デフォルトは 1。最大は 64。最小は 1。)  
  `setoption name Threads value <スレッドの数>`

* アナライズモードの有効化、無効化。 (デフォルトは false。)  
  `setoption name UCI_AnalyseMode value <true、又はfalse>`

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -



Sayuri - UCI Chess Engine
=========================

Index
-----

* [Feature](#feature)
* [Files and Directories](#files-and-directories)
* [How To Build with CMake](#how-to-build-with-cmake)
* [To Make Distributable Packages](#to-make-distributable-packages)
* [UCI Options](#uci-options)



Feature
-------

* MIT License.
* A chess engine for UCI.
* Written in C++11 and the standard libraries.
* Easy to build with CMake.
* You can use it as a static library.
* To change "`SearchParams`" object or "`EvalParams`" object,
  you can freely customize the search algorithm or the static evaluation
  function.



Files and Directories
---------------------

###### Directories ######


* `src` : For Sayuri's source files.
* `SayuriCompiled` : For Sayuri's binary file.
    * `Linux` : For Linux OS.
        * `For64Bit` : For 64 bit machines.
* `SayuriLogo` : For the logo image files.
* `SampleGames` : For sample game files of Sayuri vs other chess engines.

###### Files ######

* `README.md` : This file.
* `CMakeLists.txt` : A configuration file for CMake.
* `LICENSE` : The software license.
* `as_static_library.md` : How to use Sayuri as a static library.



How To Build with CMake
-----------------------

To use CMake, you can build Sayuri easily.

It's default compiler is "`clang++`".  
(If you want to change a compiler, please change 26th line and 27th line
in "`CMakeLists.txt`".)

###### To build in normal mode ######

1. Change directory to where "`CMakeLists.txt`" is placed in.
2. Run the following commands.
    1. `$ mkdir build`
    2. `$ cd build`
    3. `$ cmake ..`
    4. `$ make`
3. The following files or directories have created in "`build`" directory.
    * `sayuri` : A chess engine for UCI.
    * `libsayurilib.a` : A static library.
    * `include` : A directory for header files for "`libsayurilib.a`".

###### To build in debug mode ######

To build in debug mode, you can build Sayuri with "No optimized" and
"Embedded the infomation for debugger".

In the following 2-3, insert "`-DCMAKE_BUILD_TYPE=Debug`" only.

1. Change directory to where "`CMakeLists.txt`" is placed in.
2. Run the following commands.
    1. `$ mkdir build`
    2. `$ cd build`
    3. `$ cmake -DCMAKE_BUILD_TYPE=Debug ..`
    4. `$ make`
3. The following files or directories have created in "`build`" directory.
    * `sayuri` : A chess engine for UCI.
    * `libsayurilib.a` : A static library.
    * `include` : A directory for header files for "`libsayurilib.a`".

###### About each file ######

"`sayuri`" is a chess engine for UCI.  
When you use it, you don't need "`libsayurilib.a`" and "`include`".

"`libsayurilib.a`" and "`include`" are files for using Sayuri as
a static library.
When you use them, you don't need "`sayuri`".
Please read "`as_static_library.md`" for details.



To Make Distributable Packages
------------------------------

After to make "`Makefile`" in "`build`" directory with CMake,
run "`$ make dist`" command.

If you do so, the following packages have created.

* `sayuri-xxxx.xx.xx.tar.Z`
* `sayuri-xxxx.xx.xx.tar.bz2`
* `sayuri-xxxx.xx.xx.tar.gz`

(Note: "`xxxx.xx.xx`" is the version number.)



UCI Options
-----------

You can change Sayuri's settings with the following commands.

* To change size of the hash table. (Default: 32 MB, Max: 8192 MB, Min: 8 MB)  
  `setoption name Hash value <Size(MB)>`

* To initialize the hash table.
  `setoption name Clear Hash`

* To enable Ponder. (Default: true)
  `setoption name Ponder value <true or false>`

* To change the number of threads. (Default: 1, Max: 64, Min: 1)
  `setoption name Threads value <Number of threads>`

* To enable analyse mode. (Default: false)
  `setoption name UCI_AnalyseMode value <true or false>`
