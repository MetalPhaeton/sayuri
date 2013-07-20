/*
   pv_line.h: チェスの定数の定義。

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

#ifndef PV_LINE_H
#define PV_LINE_H

#include <iostream>
#include <cstddef>
#include "chess_def.h"

namespace Sayuri {
  class PVLine {
    public:
      /********************/
      /* コンストラクタ。 */
      /********************/
      PVLine();
      PVLine(const PVLine& pv_line);
      PVLine(PVLine&& pv_line);
      PVLine& operator=(const PVLine& pv_line);
      PVLine& operator=(PVLine&& pv_line);
      virtual ~PVLine() {}

      /********************/
      /* パブリック関数。 */
      /********************/
      // 最初の要素に手を登録する。
      // [引数]
      // move: セットする手。
      void SetFirst(Move move);
      // PVLineを2番目以降の要素にコピーする。
      // [引数]
      // pv_line: コピーするライン。
      void Insert(const PVLine& pv_line);

      /**************/
      /* アクセサ。 */
      /**************/
      // 長さ。
      std::size_t length() const {return length_;}
      // ライン。
      const Move (& line() const)[MAX_PLY + 1] {return line_;}
    private:
      /****************/
      /* メンバ変数。 */
      /****************/
      std::size_t length_;
      Move line_[MAX_PLY + 1];
  };
}  // namespace_Sayuri

#endif
