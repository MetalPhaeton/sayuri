/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file pv_line.h
 * @author Hironori Ishibashi
 * @brief PVラインを記録するクラス。
 */

#ifndef PV_LINE_H
#define PV_LINE_H

#include <iostream>
#include <cstddef>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /** PVラインを記録するクラス。 */
  class PVLine {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      PVLine();
      /**
       * コピーコンストラクタ。
       * @param pv_line コピー元。
       */
      PVLine(const PVLine& pv_line);
      /**
       * ムーブコンストラクタ。
       * @param pv_line ムーブ元。
       */
      PVLine(PVLine&& pv_line);
      /**
       * コピー代入演算子。
       * @param pv_line コピー元。
       */
      PVLine& operator=(const PVLine& pv_line);
      /**
       * ムーブ代入演算子。
       * @param pv_line ムーブ元。
       */
      PVLine& operator=(PVLine&& pv_line);
      /** デストラクタ。 */
      virtual ~PVLine() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 初手を挿入する。
       * @param move 初手。
       */
      void SetMove(Move move) {
        line_[0] = move;
        last_ = 1;
      }

      /**
       * 2手目以降を挿入する。
       * @param pv_line 2手目以降のPVライン。
       */
      void Insert(const PVLine& pv_line);

      /** ラインをリセットする。 */
      void ResetLine() {
        last_ = 0;
        score_ = 0;
        mate_in_ = -1;
      }

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - ライン。
       * @return ライン。
       */
      const Move& operator[](int index) const {return line_[index];}
      /**
       * アクセサ - ラインの長さ。
       * @return ラインの長さ。
       */
      std::size_t length() const {return last_;}
      /**
       * アクセサ - 評価値。
       * @return 評価値。
       */
      int score() const {return score_;}
      /**
       * アクセサ - メイトまでの手数。 -1はメイトなし。
       * @return メイトまでの手数。 -1はメイトなし。
       */
      int mate_in() const {return mate_in_;}

      // ============ //
      // ミューテータ //
      // ============ //
      /**
       * ミューテータ - 評価値。
       * @param score 評価値。
       */
      void score(int score) {score_ = score;}
      /**
       * ミューテータ - メイトまでの手数。 -1はメイトなし。
       * @param mate_in メイトまでの手数。 -1はメイトなし。
       */
      void mate_in(int mate_in) {mate_in_ = mate_in;}

    private:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** PVライン。 */
      Move line_[MAX_PLYS + 1 + 1];
      /** PVラインのインデックス。 */
      std::size_t last_;
      /** 評価値。 */
      int score_;
      /** メイトまでの手数。 -1はメイトなし。 */
      int mate_in_;
  };
}  // namespace_Sayuri

#endif
