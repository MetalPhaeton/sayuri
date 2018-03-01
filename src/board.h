/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Hironori Ishibashi
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
 * @file board.h
 * @author Hironori Ishibashi
 * @brief チェスボードの構造体。
 */

#ifndef BOARD_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define BOARD_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <string>

#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /** チェスボード構造体。 */
  struct Board {
    /** 駒の配置のビットボード。 [サイド][駒の種類] */
    Bitboard position_[NUM_SIDES][NUM_PIECE_TYPES];
    /** 駒の種類の配置。 [マス] */
    PieceType piece_board_[NUM_SQUARES];
    /** サイドの配置。 [マス] */
    Side side_board_[NUM_SQUARES];
    /** 各サイドの駒の配置のビットボード。 [サイド] */
    Bitboard side_pieces_[NUM_SIDES];
    /** 全駒の配置のビットボード。 [角度]。 */
    Bitboard blocker_[NUM_ROTS];
    /** 各サイドのキングの位置。 [サイド] */
    Square king_[NUM_SIDES];
    /** 手番。 */
    Side to_move_;
    /** キャスリングの権利。 */
    Castling castling_rights_;
    /** アンパッサンの位置。アンパッサンできなければ0。 */
    Square en_passant_square_;
    /** 50手ルールの手数。 */
    int clock_;
    /** 現在の手数。 */
    int ply_;
    /** 各サイドのキャスリングしたかどうかのフラグ。 [サイド] */
    bool has_castled_[NUM_SIDES];

    static std::string ToString(const Board& board);
  };
}  // namespace Sayuri

#endif
