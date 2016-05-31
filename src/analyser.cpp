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
 * @file analyser.cpp
 * @author Hironori Ishibashi
 * @brief 局面分析器。
 */

#include "analyser.h"

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
  /** 無名名前空間。 */
  namespace {
    // 駒の配置のベクトルを作る。
    std::vector<Square> GenPosVector(Bitboard bitboard) {
      std::vector<Square> ret(Util::CountBits(bitboard));
      for (int i = 0; bitboard; NEXT_BITBOARD(bitboard), ++i) {
        ret[i] = Util::GetSquare(bitboard);
      }
      return ret;
    }

    // 駒の数と配置を計算する。
    void CalNumAndPos(const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES],
    ResultPositionAnalysis& result) {
      // 初期化。
      result.num_all_pieces_ = 0;
      result.num_side_pieces_[NO_SIDE] = 0;
      result.num_side_pieces_[WHITE] = 0;
      result.num_side_pieces_[BLACK] = 0;

      // 計算。
      FOR_SIDES(side) {
        FOR_PIECE_TYPES(piece_type) {
          if (side && piece_type) {
            // 実在する駒の時。
            int num = Util::CountBits(position[side][piece_type]);

            // 数を足す。
            result.num_all_pieces_ += num;
            result.num_side_pieces_[side] += num;
            result.num_each_pieces_[side][piece_type] = num;

            // 駒の配置を追加する。
            std::vector<Square> vec = GenPosVector(position[side][piece_type]);

            result.pos_all_pieces_.insert
            (result.pos_all_pieces_.end(), vec.begin(), vec.end());

            result.pos_side_pieces_[side].insert
            (result.pos_side_pieces_[side].end(), vec.begin(), vec.end());

            result.pos_each_pieces_[side][piece_type].insert
            (result.pos_each_pieces_[side][piece_type].end(),
            vec.begin(), vec.end());
          } else {
            result.num_each_pieces_[side][piece_type] = 0;
          }
        }
      }
    }
  }

  // 配置の結果を返す。
  ResultPositionAnalysisPtr AnalysePosition
  (const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]) {
    ResultPositionAnalysisPtr ret_ptr(new ResultPositionAnalysis());

    // 数と配置を計算する。
    CalNumAndPos(position, *ret_ptr);

    return ret_ptr;
  }
}  // namespace Sayuri
