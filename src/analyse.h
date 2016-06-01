/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Hironori Ishibashi
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
 * @file analyser.h
 * @author Hironori Ishibashi
 * @brief 局面分析器。
 */

#ifndef ANALYSER_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define ANALYSER_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>
#include <memory>
#include "common.h"
#include "board.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /** 配置の分析結果の構造体。 */
  struct ResultPositionAnalysis {
    /** 駒全体の数。 */
    int num_all_pieces_;
    /** 全ての駒の配置。 */
    std::vector<Square> pos_all_pieces_;

    /** 各サイドの駒の数。 */
    std::array<int, NUM_SIDES> num_side_pieces_;
    /** 各サイドの駒の配置。 */
    std::array<std::vector<Square>, NUM_SIDES> pos_side_pieces_;

    /** 各駒の数。 */
    std::array<std::array<int, NUM_PIECE_TYPES>, NUM_SIDES> num_each_pieces_;
    /** 各サイドの配置。 */
    std::array<std::array<std::vector<Square>, NUM_PIECE_TYPES>, NUM_SIDES>
    pos_each_pieces_;

    /** 相手のキングをチェックしている駒の数。 */
    std::array<int, NUM_SIDES> num_checking_pieces_;
    /** 相手のキングをチェックしている駒の配置。 */
    std::array<std::vector<Square>, NUM_SIDES> pos_checking_pieces_;
  };
  /** 配置の分析結果の構造体のポインタ。 */
  using ResultPositionAnalysisPtr = std::shared_ptr<ResultPositionAnalysis>;

  /** 駒の分析結果の構造体。 */
  struct ResultPieceAnalysis {
    // --- 駒の情報 --- //
    /** 位置。 */
    Square square_;
    /** サイド。 */
    Side side_;
    /** 駒の種類。 */
    PieceType piece_type_;
  };
  /** 駒の分析結果の構造体のポインタ。 */
  using ResultPieceAnalysisPtr = std::shared_ptr<ResultPieceAnalysis>;

  /**
   * 駒の配置を分析し、結果の構造体を返す。
   * @param position 駒の配置のビットボード。
   * @return 配置の分析結果の構造体。
   */
  ResultPositionAnalysisPtr AnalysePosition
  (const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]);

}  // namespace Sayuri

#endif
