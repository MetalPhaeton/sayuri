/*
   sayuri_debug.h: Misakiをデバッグする。
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
#include <stdexcept>
#include <string>

#include "chess_def.h"

namespace Sayuri {
  // デバッグ用メイン関数。
  int DebugMain(int argc, char* argv[]);

  // Sayuriのエラークラス。
  class SayuriError : public std::logic_error {
    public:
      SayuriError(const char* message) : std::logic_error(message) {}
      SayuriError(const std::string message) : std::logic_error(message) {}
  };

  // 論理テスト。
  // [引数]
  // expr: 条件式。
  // [例外]
  // exprがfalseなら例外発生。
  inline void Assert(bool expr) {
    if(expr) return;
    throw SayuriError("アサート失敗。");
  }

  // ビットボードを以下のように出力する。
  // (+)はビットが立っている場所。
  //  +---+---+---+---+---+---+---+---+
  // 8|   |   |   |   |   |   |   |   |
  //  +---+---+---+---+---+---+---+---+
  // 7|   |   |(+)|   |   |   |   |   |
  //  +---+---+---+---+---+---+---+---+
  // 6|   |   |   |   |   |   |   |   |
  //  +---+---+---+---+---+---+---+---+
  // 5|   |   |   |   |   |   |   |   |
  //  +---+---+---+---+---+---+---+---+
  // 4|   |   |   |   |(+)|   |   |   |
  //  +---+---+---+---+---+---+---+---+
  // 3|   |   |   |   |   |   |   |   |
  //  +---+---+---+---+---+---+---+---+
  // 2|   |   |(+)|   |   |   |   |   |
  //  +---+---+---+---+---+---+---+---+
  // 1|   |   |   |   |   |   |   |   |
  //  +---+---+---+---+---+---+---+---+
  //    a   b   c   d   e   f   g   h
  // [引数]
  // bitboard: 出力するビットボード。
  void PrintBitboard(Bitboard bitboard);

  // 手を出力する。
  // [引数]
  // move: 出力する手。
  void PrintMove(Move move);

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
  double GetTime();
}  // namespace Sayuri

#endif
