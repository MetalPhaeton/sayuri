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
 * @file fen.cpp
 * @author Hironori Ishibashi
 * @brief FEN文字列のパーサの実装。
 */

#include "fen.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <set>
#include <cstdlib>
#include <sstream>
#include <map>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  FEN::FEN(const std::string fen_str) :
  to_move_(WHITE),
  castling_rights_(ALL_CASTLING),
  en_passant_square_(0),
  clock_(0),
  ply_(0) {
    // 駒の配置を初期化。
    INIT_ARRAY(position_);

    try {
      // 構文木にパース。
      std::map<std::string, std::string> tree = Util::ParseFEN(fen_str);

      // 配置を解析。
      std::string::iterator itr = tree["fen position"].begin();
      FOR_SQUARES(square) {
        Bitboard bb = Util::SQUARE[square][R0];

        switch (*itr) {
          case 'P':
            position_[WHITE][PAWN] |= bb;
            break;
          case 'N':
            position_[WHITE][KNIGHT] |= bb;
            break;
          case 'B':
            position_[WHITE][BISHOP] |= bb;
            break;
          case 'R':
            position_[WHITE][ROOK] |= bb;
            break;
          case 'Q':
            position_[WHITE][QUEEN] |= bb;
            break;
          case 'K':
            position_[WHITE][KING] |= bb;
            break;
          case 'p':
            position_[BLACK][PAWN] |= bb;
            break;
          case 'n':
            position_[BLACK][KNIGHT] |= bb;
            break;
          case 'b':
            position_[BLACK][BISHOP] |= bb;
            break;
          case 'r':
            position_[BLACK][ROOK] |= bb;
            break;
          case 'q':
            position_[BLACK][QUEEN] |= bb;
            break;
          case 'k':
            position_[BLACK][KING] |= bb;
            break;
        }

        ++itr;
      }

      // 手番を解析。
      if (tree["fen to_move"] == "w") {
        to_move_ = WHITE;
      } else {
        to_move_ = BLACK;
      }

      // キャスリングの権利を解析。
      castling_rights_ = 0;
      if (tree["fen castling"][0] != '-') {
        castling_rights_ |= WHITE_SHORT_CASTLING;
      }
      if (tree["fen castling"][1] != '-') {
        castling_rights_ |= WHITE_LONG_CASTLING;
      }
      if (tree["fen castling"][2] != '-') {
        castling_rights_ |= BLACK_SHORT_CASTLING;
      }
      if (tree["fen castling"][3] != '-') {
        castling_rights_ |= BLACK_LONG_CASTLING;
      }

      // アンパッサンのマスを解析。
      std::string temp = tree["fen en_passant"];
      if (temp != "-") {
        en_passant_square_ =
        Util::CoordToSquare(temp[0] - 'a', temp[1] - '1');
      }

      // 50手ルールの手数を解析。 あれば。
      if (tree.find("fen clock") != tree.end()) {
        clock_ = std::stoul(tree["fen clock"]);
      }

      // 手数を解析。 あれば。
      if (tree.find("fen ply") != tree.end()) {
        ply_ = std::stoul(tree["fen ply"]);
      }
    } catch (...) {
      SetStartPosition();
    }
  }

  // デフォルトコンストラクタ。
  FEN::FEN() {
    SetStartPosition();
  }

  // コピーコンストラクタ。
  FEN::FEN(const FEN& fen) :
  to_move_(fen.to_move_),
  castling_rights_(fen.castling_rights_),
  en_passant_square_(fen.en_passant_square_),
  clock_(fen.clock_),
  ply_(fen.ply_) {
    // 駒の配置をコピー。
    COPY_ARRAY(position_, fen.position_);
  }

  // ムーブコンストラクタ。
  FEN::FEN(FEN&& fen) :
  to_move_(fen.to_move_),
  castling_rights_(fen.castling_rights_),
  en_passant_square_(fen.en_passant_square_),
  clock_(fen.clock_),
  ply_(fen.ply_) {
    // 駒の配置をコピー。
    COPY_ARRAY(position_, fen.position_);
  }

  // コピー代入演算子。
  FEN& FEN::operator=(const FEN& fen) {
    // メンバをコピー。
    to_move_ = fen.to_move_;
    castling_rights_ = fen.castling_rights_;
    en_passant_square_ = fen.en_passant_square_;
    clock_ = fen.clock_;
    ply_ = fen.ply_;
    COPY_ARRAY(position_, fen.position_);

    return *this;
  }

  // ムーブ代入演算子。
  FEN& FEN::operator=(FEN&& fen) {
    // メンバをコピー。
    to_move_ = fen.to_move_;
    castling_rights_ = fen.castling_rights_;
    en_passant_square_ = fen.en_passant_square_;
    clock_ = fen.clock_;
    ply_ = fen.ply_;
    COPY_ARRAY(position_, fen.position_);

    return *this;
  }

  // ================ //
  // プライベート関数 //
  // ================ //
  // スタートポジションにセット。
  void FEN::SetStartPosition() {
    to_move_ = WHITE;
    castling_rights_ = ALL_CASTLING;
    en_passant_square_ = 0;
    clock_ = 0;
    ply_ = 0;

    // 駒を初期配置にする。
    // ポーン。
    position_[WHITE][PAWN] = Util::RANK[RANK_2];
    position_[BLACK][PAWN] = Util::RANK[RANK_7];
    // ナイト。
    position_[WHITE][KNIGHT] = Util::SQUARE[B1][R0] | Util::SQUARE[G1][R0];
    position_[BLACK][KNIGHT] = Util::SQUARE[B8][R0] | Util::SQUARE[G8][R0];
    // ビショップ。
    position_[WHITE][BISHOP] = Util::SQUARE[C1][R0] | Util::SQUARE[F1][R0];
    position_[BLACK][BISHOP] = Util::SQUARE[C8][R0] | Util::SQUARE[F8][R0];
    // ルーク。
    position_[WHITE][ROOK] = Util::SQUARE[A1][R0] | Util::SQUARE[H1][R0];
    position_[BLACK][ROOK] = Util::SQUARE[A8][R0] | Util::SQUARE[H8][R0];
    // クイーン。
    position_[WHITE][QUEEN] = Util::SQUARE[D1][R0];
    position_[BLACK][QUEEN] = Util::SQUARE[D8][R0];
    // キング。
    position_[WHITE][KING] = Util::SQUARE[E1][R0];
    position_[BLACK][KING] = Util::SQUARE[E8][R0];
  }
}  // namespace Sayuri
