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
 * @file position_record.cpp
 * @author Hironori Ishibashi
 * @brief エンジンのボードの状態を記録するクラスの実装。
 */

#include "position_record.h"

#include <iostream>
#include <cstdint>
#include "common.h"
#include "chess_engine.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  PositionRecord::PositionRecord() :
  to_move_(NO_SIDE),
  castling_rights_(0),
  en_passant_square_(0),
  clock_(0),
  ply_(0),
  pos_hash_(0) {
    INIT_ARRAY(position_);
    INIT_ARRAY(piece_board_);
    INIT_ARRAY(side_board_);
    INIT_ARRAY(side_pieces_);
    INIT_ARRAY(blocker_);
    INIT_ARRAY(king_);
    INIT_ARRAY(has_castled_);
    INIT_ARRAY(position_memo_);
  }

  // ========== //
  // 比較演算子 //
  // ========== //
  bool PositionRecord::operator==(const ChessEngine& engine) const {
    if (pos_hash_ != engine.GetCurrentHash()) return false;
    if (to_move_ != engine.basic_st_.to_move_) return false;
    if (castling_rights_ != engine.basic_st_.castling_rights_) return false;
    if (en_passant_square_ != engine.basic_st_.en_passant_square_) {
      return false;
    }
    for (Side side = WHITE; side <= BLACK; ++side) {
      for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
        if (position_[side][piece_type]
        != engine.basic_st_.position_[side][piece_type]) {
          return false;
        }
      }
    }

    return true;
  }

  bool PositionRecord::operator!=(const ChessEngine& engine) const {
    if (pos_hash_ != engine.GetCurrentHash()) return true;
    if (to_move_ != engine.basic_st_.to_move_) return true;
    if (castling_rights_ != engine.basic_st_.castling_rights_) return true;
    if (en_passant_square_ != engine.basic_st_.en_passant_square_) return true;
    for (Side side = WHITE; side <= BLACK; ++side) {
      for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
        if (position_[side][piece_type]
        != engine.basic_st_.position_[side][piece_type]) {
          return true;
        }
      }
    }

    return false;
  }

  bool PositionRecord::operator==(const PositionRecord& record) const {
    if (pos_hash_ != record.pos_hash_) return false;
    if (to_move_ != record.to_move_) return false;
    if (castling_rights_ != record.castling_rights_) return false;
    if (en_passant_square_ != record.en_passant_square_) return false;
    for (Side side = WHITE; side <= BLACK; ++side) {
      for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
        if (position_[side][piece_type]
        != record.position_[side][piece_type]) {
          return false;
        }
      }
    }

    return true;
  }

  bool PositionRecord::operator!=(const PositionRecord& record) const {
    if (pos_hash_ != record.pos_hash_) return true;
    if (to_move_ != record.to_move_) return true;
    if (castling_rights_ != record.castling_rights_) return true;
    if (en_passant_square_ != record.en_passant_square_) return true;
    for (Side side = WHITE; side <= BLACK; ++side) {
      for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
        if (position_[side][piece_type]
        != record.position_[side][piece_type]) {
          return true;
        }
      }
    }

    return true;
  }

  // ================ //
  // プライベート関数 //
  // ================ //
  // メンバをエンジンからコピーする。
  void PositionRecord::ScanMember(const ChessEngine& engine) {
    COPY_ARRAY(position_, engine.basic_st_.position_);
    COPY_ARRAY(piece_board_, engine.basic_st_.piece_board_);
    COPY_ARRAY(side_board_, engine.basic_st_.side_board_);
    COPY_ARRAY(side_pieces_, engine.basic_st_.side_pieces_);
    COPY_ARRAY(blocker_, engine.basic_st_.blocker_);
    COPY_ARRAY(king_, engine.basic_st_.king_);
    to_move_ = engine.basic_st_.to_move_;
    castling_rights_ = engine.basic_st_.castling_rights_;
    en_passant_square_ = engine.basic_st_.en_passant_square_;
    clock_ = engine.basic_st_.clock_;
    ply_ = engine.basic_st_.ply_;
    COPY_ARRAY(has_castled_, engine.basic_st_.has_castled_);
    COPY_ARRAY(position_memo_, engine.basic_st_.position_memo_);
    pos_hash_ = engine.GetCurrentHash();
  }
}  // namespace Sayuri
