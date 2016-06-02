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
  /** 分析結果のマスのベクトル。 */
  using ResultSquares = std::vector<Square>;

  /**
   * 駒の違いを分析する。
   * @param board 分析したいボードの構造体。
   * @param piece_type 分析したい駒の種類。
   * @return 駒の違いの配列。 プラスなら白が多い。 マイナスなら黒が多い。
   */
  int AnalyseDiff(const Board& board, PieceType piece_type);

  /**
   * 機動力を分析する。
   * @param board 分析したいボードの構造体。
   * @param piece_square 分析したい駒の位置。
   * @return その駒の動ける場所。
   */
  ResultSquares AnalyseMobility(const Board& board, Square piece_square);

  /**
   * どの駒を攻撃しているかを分析する。
   * @param board 分析したいボードの構造体。
   * @param piece_square 攻撃している駒の位置。
   * @return 攻撃ターゲットの位置。
   */
  ResultSquares AnalyseAttacking(const Board& board, Square piece_square);

  /**
   * どの駒に攻撃されているかを分析する。
   * @param board 分析したいボードの構造体。
   * @param piece_square 攻撃されている駒の位置。
   * @return 自分を攻撃している駒の位置。
   */
  ResultSquares AnalyseAttacked(const Board& board, Square piece_square);

  /**
   * 展開を分析する。
   * @param board 分析したいボード。
   * @param piece_side 分析したい駒のサイド。
   * @param piece_type 分析したい駒の種類。
   * @return 展開済みの駒の位置。
   */
  ResultSquares AnalyseDevelopment(const Board& board, Side piece_side,
  PieceType piece_type);
}  // namespace Sayuri

#endif
