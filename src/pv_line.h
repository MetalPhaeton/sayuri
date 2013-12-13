/*
   pv_line.h: チェスの定数の定義。

   The MIT License (MIT)

   Copyright (c) 2013 Hironori Ishibashi

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
  class PVLine;

  // PVLineのスロット。
  class PVSlot {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      PVSlot();
      PVSlot(const PVSlot& slot);
      PVSlot(PVSlot&& slot);
      PVSlot& operator=(const PVSlot& slot);
      PVSlot& operator=(PVSlot&& slot);
      virtual ~PVSlot() {}

      /**************/
      /* アクセサ。 */
      /**************/
      // 手。
      Move move() const {return move_;}
      // チェックメイトされたか。
      bool has_checkmated() const {return has_checkmated_;}

      /******************/
      /* ミューテータ。 */
      /******************/
      // 手。
      void move(Move& move) {move_ = move;}
      // チェックメイトされたか。
      void has_checkmated(bool has_checkmated) {
        has_checkmated_ = has_checkmated;
      }

    private:
      /****************/
      /* メンバ変数。 */
      /****************/
      // 手。
      Move move_;
      // チェックメイトされたか。
      bool has_checkmated_;
  };

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
      // 最初の要素に手を登録する。
      // [引数]
      // move: セットする手。
      void SetMove(Move move);
      // PVLineを2番目以降の要素にコピーする。
      // [引数]
      // pv_line: コピーするライン。
      void Insert(const PVLine& pv_line);
      // 最初の要素にチェックメイトされたことを記録する。
      void MarkCheckmated();

      /**************/
      /* アクセサ。 */
      /**************/
      // 長さ。
      std::size_t length() const {return length_;}
      // ライン。
      const PVSlot (& line() const)[MAX_PLYS + 1] {return line_;}
      // 評価値。
      int score() const {return score_;}

      /******************/
      /* ミューテータ。 */
      /******************/
      // 評価値。
      void score(int score) {score_ = score;}

      /**********************/
      /* ソート用比較関数。 */
      /**********************/
      static bool Compare(const PVLine& first, const PVLine& second);

    private:
      /****************/
      /* メンバ変数。 */
      /****************/
      // 長さ。
      std::size_t length_;
      // ライン。
      PVSlot line_[MAX_PLYS + 1];
      // 評価値。
      int score_;
  };
}  // namespace_Sayuri

#endif
