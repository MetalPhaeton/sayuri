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
  blocker_0_(0),
  blocker_45_(0),
  blocker_90_(0),
  blocker_135_(0),
  to_move_(NO_SIDE),
  castling_rights_(0),
  en_passant_square_(0),
  ply_100_(0),
  ply_(0),
  pos_hash_(0) {
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = 0;
      }
      side_pieces_[side] = 0;
      king_[side] = 0;
      has_castled_[side] = false;
    }
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      position_memo_[i] = 0;
    }
  }

  // コピーコンストラクタ。
  PositionRecord::PositionRecord(const PositionRecord& record) {
    ScanMember(record);
  }

  // ムーブコンストラクタ。
  PositionRecord::PositionRecord(PositionRecord&& record) {
    ScanMember(record);
  }

  // コピー代入演算子。
  PositionRecord& PositionRecord::operator=(const PositionRecord& record) {
    ScanMember(record);
    return *this;
  }

  // ムーブ代入演算子。
  PositionRecord& PositionRecord::operator=(PositionRecord&& record) {
    ScanMember(record);
    return *this;
  }

  // ========== //
  // 比較演算子 //
  // ========== //
  bool PositionRecord::operator==(const ChessEngine& engine) const {
    if (pos_hash_ != engine.GetCurrentHash()) return false;
    if (to_move_ != engine.to_move()) return false;
    if (castling_rights_ != engine.castling_rights()) return false;
    if (en_passant_square_ != engine.en_passant_square()) return false;
    for (Side side = WHITE; side <= BLACK; side++) {
      for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
        if (position_[side][piece_type]
        != engine.position()[side][piece_type]) {
          return false;
        }
      }
    }

    return true;
  }

  bool PositionRecord::operator!=(const ChessEngine& engine) const {
    if (pos_hash_ != engine.GetCurrentHash()) return true;
    if (to_move_ != engine.to_move()) return true;
    if (castling_rights_ != engine.castling_rights()) return true;
    if (en_passant_square_ != engine.en_passant_square()) return true;
    for (Side side = WHITE; side <= BLACK; side++) {
      for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
        if (position_[side][piece_type]
        != engine.position()[side][piece_type]) {
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
    for (Side side = WHITE; side <= BLACK; side++) {
      for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
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
    for (Side side = WHITE; side <= BLACK; side++) {
      for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
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
  // メンバをコピーする。
  void PositionRecord::ScanMember(const PositionRecord& record) {
    // サイド毎のコピー。
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = record.position_[side][piece_type];
      }
      side_pieces_[side] = record.side_pieces_[side];
      king_[side] = record.king_[side];
      has_castled_[side] = record.has_castled_[side];
    }

    // マス毎のコピー。
    for (Square square = 0; square < NUM_SQUARES; square++) {
      piece_board_[square] = record.piece_board_[square];
      side_board_[square] = record.side_board_[square];
    }

    // 全駒のコピー。
    blocker_0_ = record.blocker_0_;
    blocker_45_ = record.blocker_45_;
    blocker_90_ = record.blocker_90_;
    blocker_135_ = record.blocker_135_;

    // その他をコピー。
    to_move_ = record.to_move_;
    castling_rights_ = record.castling_rights_;
    en_passant_square_ = record.en_passant_square_;
    ply_100_ = record.ply_100_;
    ply_ = record.ply_;
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      position_memo_[i] = position_memo_[i];
    }
    pos_hash_ = record.pos_hash_;
  }

  // メンバをエンジンからコピーする。
  void PositionRecord::ScanMember(const ChessEngine& engine, Hash pos_hash) {
    // サイド毎のコピー。
    for (Side side = WHITE; side <= BLACK; side++) {
      for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
        position_[side][piece_type] = engine.position()[side][piece_type];
      }
      side_pieces_[side] = engine.side_pieces()[side];
      king_[side] = engine.king()[side];
      has_castled_[side] = engine.has_castled()[side];
    }

    // マス毎のコピー。
    for (Square square = A1; square <= H8; square++) {
      piece_board_[square] = engine.piece_board()[square];
      side_board_[square] = engine.side_board()[square];
    }

    // 全駒のコピー。
    blocker_0_ = engine.blocker_0();
    blocker_45_ = engine.blocker_45();
    blocker_90_ = engine.blocker_90();
    blocker_135_ = engine.blocker_135();

    // その他のコピー。
    to_move_ = engine.to_move();
    castling_rights_ = engine.castling_rights();
    en_passant_square_ = engine.en_passant_square();
    ply_100_ = engine.ply_100();
    ply_ = engine.ply();
    for (std::uint32_t i = 0; i < (MAX_PLYS + 1); i++) {
      position_memo_[i] = engine.position_memo()[i];
    }
    pos_hash_ = pos_hash;
  }
}  // namespace Sayuri
