/* chess_def.cpp: チェスの定数の定義。
   Copyright (c) 2011 Ishibashi Hironori

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

#include "chess_def.h"

namespace Sayuri {
  // キャスリングのフラグ。
  const castling_t WHITE_SHORT_CASTLING = 1;  // 白のショートキャスリング。
  const castling_t WHITE_LONG_CASTLING = 1 << 1;  // 白のロングキャスリング。
  const castling_t BLACK_SHORT_CASTLING = 1 << 2;  // 黒のショートキャスリング。
  const castling_t BLACK_LONG_CASTLING = 1 << 3;  // 黒のロングキャスリング。
  const castling_t WHITE_CASTLING =
  WHITE_SHORT_CASTLING | WHITE_LONG_CASTLING;  // 白の全てのキャスリング。
  const castling_t BLACK_CASTLING =
  BLACK_SHORT_CASTLING | BLACK_LONG_CASTLING;  // 黒の全てのキャスリング。
  const castling_t ALL_CASTLING =
  WHITE_CASTLING | BLACK_CASTLING;  // 両方の全てのキャスリング。
}  // namespace Sayuri
