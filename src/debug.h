/*
   debug.h: Misakiをデバッグする。

   The MIT License (MIT)

   Copyright (c) 2013 Ishibashi Hironori

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

#ifndef SAYURI_DEBUG_H
#define SAYURI_DEBUG_H

#include <iostream>
#include "chess_def.h"
#include "chess_util.h"
#include "error.h"

namespace Sayuri {
  // デバッグ用メイン関数。
  int DebugMain(int argc, char* argv[]);

  // 擬似ハッシュキー生成。
  // [戻り値]
  // ランダムに生成されたハッシュキー。
  HashKey GenPseudoHashKey();

  // ビットボードを以下のように出力する。
  // @はビットが立っている場所。
  //  +-----------------+
  // 8| . . . . @ . . @ |
  // 7| . . . . . . . . |
  // 6| . . . . @ . @ . |
  // 5| . . . . . . . . |
  // 4| . @ . . . . . . |
  // 3| . . . . . . . . |
  // 2| . . . . . . @ . |
  // 1| @ . . . . . . . |
  //  +-----------------+
  //    a b c d e f g h 
  // [引数]
  // bitboard: 出力するビットボード。
  void PrintBitboard(Bitboard bitboard);

  // 手を出力する。
  // [引数]
  // move: 出力する手。
  void PrintMove(Move move);

  // 駒の配置を出力する。
  // 以下のような感じ。
  // 大文字は白、小文字は黒。
  //  +-----------------+
  // 8| r . b q k b n r |
  // 7| p p p p . p p p |
  // 6| . . n . . . . . |
  // 5| . . . . p . . . |
  // 4| . . . . P . . . |
  // 3| . . . . . . N . |
  // 2| P P P P . P P P |
  // 1| R N B Q K B . R |
  //  +-----------------+
  //    a b c d e f g h 
  // [引数]
  // 駒の配置のビットボード。
  void PrintPosition(const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]);

  /**********************
   * ストップウォッチ。 *
   **********************/
  // ストップウォッチをスタートする。
  void Start();
  // ストップウォッチをストップする。
  void Stop();
  // ストップウォッチで計測した秒数を得る。
  // [戻り値]
  // 計測した秒数。
  int GetTime();
}  // namespace Sayuri

#endif
