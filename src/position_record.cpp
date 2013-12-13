/* 
   position_record.cpp: ボードの状態を記録するクラスの実装。

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

#include "position_record.h"

#include <iostream>
#include "chess_def.h"
#include "chess_engine.h"

namespace Sayuri {
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  PositionRecord::PositionRecord(const ChessEngine& engine) {
    // 駒の配置をコピー。ついでにhas_castled_もコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      has_castled_[i] = engine.has_castled()[i];
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = engine.position()[i][j];
      }
    }

    // それ以外をコピー。
    to_move_ = engine.to_move();
    castling_rights_ = engine.castling_rights();
    en_passant_square_ = engine.en_passant_square();
    can_en_passant_ = engine.can_en_passant();
    ply_100_ = engine.ply_100();
    ply_ = engine.ply();
    pos_hash_ = engine.GetCurrentHash();
  }

  // デフォルトコンストラクタ。
  PositionRecord::PositionRecord() :
  to_move_(NO_SIDE),
  castling_rights_(0),
  en_passant_square_(0),
  can_en_passant_(false),
  ply_100_(0),
  ply_(0),
  pos_hash_(0ULL) {
    for (int i = 0; i < NUM_SIDES; i++) {
      has_castled_[i] = false;
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = 0ULL;
      }
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

  // コピー代入。
  PositionRecord& PositionRecord::operator=(const PositionRecord& record) {
    ScanMember(record);
    return *this;
  }

  // ムーブ代入。
  PositionRecord& PositionRecord::operator=(PositionRecord&& record) {
    ScanMember(record);
    return *this;
  }

  /****************/
  /* 比較演算子。 */
  /****************/
  bool PositionRecord::operator==(const ChessEngine& engine) const {
    if (pos_hash_ != engine.GetCurrentHash()) return false;
    if (to_move_ != engine.to_move()) return false;
    if (castling_rights_ != engine.castling_rights()) return false;
    if (can_en_passant_ != engine.can_en_passant()) return false;
    if (can_en_passant_) {
      if (en_passant_square_ != engine.en_passant_square()) return false;
    }
    for (int i = WHITE; i <= BLACK; i++) {
      for (int j = PAWN; j <= KING; j++) {
        if (position_[i][j] != engine.position()[i][j]) return false;
      }
    }

    return true;
  }

  bool PositionRecord::operator!=(const ChessEngine& engine) const {
    if (pos_hash_ != engine.GetCurrentHash()) return true;
    if (to_move_ != engine.to_move()) return true;
    if (castling_rights_ != engine.castling_rights()) return true;
    if (can_en_passant_ != engine.can_en_passant()) return true;
    if (can_en_passant_) {
      if (en_passant_square_ != engine.en_passant_square()) return true;
    }
    for (int i = WHITE; i <= BLACK; i++) {
      for (int j = PAWN; j <= KING; j++) {
        if (position_[i][j] != engine.position()[i][j]) return true;
      }
    }

    return false;
  }

  bool PositionRecord::operator==(const PositionRecord& record) const {
    if (pos_hash_ != record.pos_hash_) return false;
    if (to_move_ != record.to_move_) return false;
    if (castling_rights_ != record.castling_rights_) return false;
    if (can_en_passant_ != record.can_en_passant_) return false;
    if (can_en_passant_) {
      if (en_passant_square_ != record.en_passant_square_) return false;
    }
    for (int i = WHITE; i <= BLACK; i++) {
      for (int j = PAWN; j <= KING; j++) {
        if (position_[i][j] != record.position_[i][j]) return false;
      }
    }

    return true;
  }

  bool PositionRecord::operator!=(const PositionRecord& record) const {
    if (pos_hash_ != record.pos_hash_) return true;
    if (to_move_ != record.to_move_) return true;
    if (castling_rights_ != record.castling_rights_) return true;
    if (can_en_passant_ != record.can_en_passant_) return true;
    if (can_en_passant_) {
      if (en_passant_square_ != record.en_passant_square_) return true;
    }
    for (int i = WHITE; i <= BLACK; i++) {
      for (int j = PAWN; j <= KING; j++) {
        if (position_[i][j] != record.position_[i][j]) return true;
      }
    }

    return true;
  }

  /**********************/
  /* プライベート関数。 */
  /**********************/
  // メンバをコピーする。
  void PositionRecord::ScanMember(const PositionRecord& record) {
    // 駒の配置をコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      has_castled_[i] = record.has_castled_[i];
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = record.position_[i][j];
      }
    }

    // その他をコピー。
    to_move_ = record.to_move_;
    castling_rights_ = record.castling_rights_;
    en_passant_square_ = record.en_passant_square_;
    can_en_passant_ = record.can_en_passant_;
    ply_100_ = record.ply_100_;
    ply_ = record.ply_;
    pos_hash_ = record.pos_hash_;
  }
}  // namespace Sayuri
