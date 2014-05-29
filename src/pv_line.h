/*
   pv_line.h: チェスの定数の定義。

   The MIT License (MIT)

   Copyright (c) 2014 Hironori Ishibashi

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
#include "common.h"

namespace Sayuri {
  // PVライン。
  class PVLine {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      PVLine();
      PVLine(const PVLine& pv_line);
      PVLine(PVLine&& pv_line);
      PVLine& operator=(const PVLine& pv_line);
      PVLine& operator=(PVLine&& pv_line);
      virtual ~PVLine() {}

      /********************/
      /* パブリック関数。 */
      /********************/
      // 手を最初に挿入する。
      void SetMove(Move move) {
        line_[0] = move;
        last_ = 1;
      }
      // PVLineを2番目以降の要素にコピーする。
      // [引数]
      // pv_line: コピーするライン。
      void Insert(const PVLine& pv_line);
      // ラインをリセットする。
      void ResetLine() {
        last_ = 0;
        score_ = 0;
        mate_in_ = -1;
      }

      /**************/
      /* アクセサ。 */
      /**************/
      // ライン。
      const Move& operator[](int index) const {return line_[index];}
      // 長さ。
      std::size_t length() const {return last_;}
      // 評価値。
      int score() const {return score_;}
      // メイトまでのプライ。-1はメイトなし。
      int mate_in() const {return mate_in_;}

      /******************/
      /* ミューテータ。 */
      /******************/
      // 評価値。
      void score(int score) {score_ = score;}
      // メイトまでのプライ。
      void mate_in(int mate_in) {mate_in_ = mate_in;}

    private:
      /****************/
      /* メンバ変数。 */
      /****************/
      // PVライン。
      Move line_[MAX_PLYS + 1 + 1];
      // PVラインのポインタ。
      std::size_t last_;
      // 評価値。
      int score_;
      // メイトまでのプライ。-1はメイトなし。
      int mate_in_;
  };
}  // namespace_Sayuri

#endif
