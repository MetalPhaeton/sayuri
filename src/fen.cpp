/* The MIT License (MIT)
 *
 * Copyright (c) 2015 Hironori Ishibashi
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
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  FEN::FEN(const std::string fen_str) :
  en_passant_square_(0),
  ply_100_(0),
  ply_(1) {
    // fen_strを分解。
    std::vector<std::string> fen_tokens =
    Util::Split<char>(fen_str, {' '}, std::set<char> {});

    try {
      // FENを解析。
      EvalPosition(fen_tokens[0]);
      EvalToMove(fen_tokens[1]);
      EvalCastlingRights(fen_tokens[2]);
      if (fen_tokens.size() >= 4) EvalEnPassant(fen_tokens[3]);
      if (fen_tokens.size() >= 5) EvalPly100(fen_tokens[4]);
      if (fen_tokens.size() >= 6) EvalPly(fen_tokens[5]);
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
  ply_100_(fen.ply_100_),
  ply_(fen.ply_) {
    // 駒の配置をコピー。
    for (Side side = 0; side < NUM_SIDES; ++side) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; ++piece_type) {
        position_[side][piece_type] = fen.position_[side][piece_type];
      }
    }
  }

  // ムーブコンストラクタ。
  FEN::FEN(FEN&& fen) :
  to_move_(fen.to_move_),
  castling_rights_(fen.castling_rights_),
  en_passant_square_(fen.en_passant_square_),
  ply_100_(fen.ply_100_),
  ply_(fen.ply_) {
    // 駒の配置をコピー。
    for (Side side = 0; side < NUM_SIDES; ++side) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; ++piece_type) {
        position_[side][piece_type] = fen.position_[side][piece_type];
      }
    }
  }

  // コピー代入演算子。
  FEN& FEN::operator=(const FEN& fen) {
    // メンバをコピー。
    to_move_ = fen.to_move_;
    castling_rights_ = fen.castling_rights_;
    en_passant_square_ = fen.en_passant_square_;
    ply_100_ = fen.ply_100_;
    ply_ = fen.ply_;
    for (Side side = 0; side < NUM_SIDES; ++side) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; ++piece_type) {
        position_[side][piece_type] = fen.position_[side][piece_type];
      }
    }

    return *this;
  }

  // ムーブ代入演算子。
  FEN& FEN::operator=(FEN&& fen) {
    // メンバをコピー。
    to_move_ = fen.to_move_;
    castling_rights_ = fen.castling_rights_;
    en_passant_square_ = fen.en_passant_square_;
    ply_100_ = fen.ply_100_;
    ply_ = fen.ply_;
    for (Side side = 0; side < NUM_SIDES; ++side) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; ++piece_type) {
        position_[side][piece_type] = fen.position_[side][piece_type];
      }
    }

    return *this;
  }

  // =========== //
  // FEN評価関数 //
  // =========== //
  // 駒の配置文字列を評価する。
  void FEN::EvalPosition(const std::string& position_str) {
    // 駒の配置を初期化。
    for (Side side = 0; side < NUM_SIDES; ++side) {
      for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; ++piece_type) {
        position_[side][piece_type] = 0;
      }
    }

    // ランク毎に区切って、逆転。
    // fenは8ランクから始まるので、1ランクからに逆転する必要がある。
    std::vector<std::string> position_vec =
    Util::Split<char>(position_str, {'/'}, std::set<char> {});
    std::reverse(position_vec.begin(), position_vec.end());

    // 値を格納していく。
    int square = 0;  // マスの位置。
    for (auto& a : position_vec) {
      for (auto& b : a) {

        // 文字を解析。
        switch (b) {
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
            square += (b - '0') - 1;
            break;
          case 'P':
            position_[WHITE][PAWN] |= Util::SQUARE[square];
            break;
          case 'N':
            position_[WHITE][KNIGHT] |= Util::SQUARE[square];
            break;
          case 'B':
            position_[WHITE][BISHOP] |= Util::SQUARE[square];
            break;
          case 'R':
            position_[WHITE][ROOK] |= Util::SQUARE[square];
            break;
          case 'Q':
            position_[WHITE][QUEEN] |= Util::SQUARE[square];
            break;
          case 'K':
            position_[WHITE][KING] |= Util::SQUARE[square];
            break;
          case 'p':
            position_[BLACK][PAWN] |= Util::SQUARE[square];
            break;
          case 'n':
            position_[BLACK][KNIGHT] |= Util::SQUARE[square];
            break;
          case 'b':
            position_[BLACK][BISHOP] |= Util::SQUARE[square];
            break;
          case 'r':
            position_[BLACK][ROOK] |= Util::SQUARE[square];
            break;
          case 'q':
            position_[BLACK][QUEEN] |= Util::SQUARE[square];
            break;
          case 'k':
            position_[BLACK][KING] |= Util::SQUARE[square];
            break;
          default:
            throw SayuriError("FENを解析できません。 in FEN::EvalPosition()");
            break;
        }
        ++square;
        if (square > 64) {
          throw SayuriError("FENを解析できません。 in FEN::EvalPosition()");
        }
      }
    }
  }

  // 手番文字列を評価する。
  void FEN::EvalToMove(const std::string& to_move_str) {
    switch (to_move_str[0]) {
      case 'w':
        to_move_ = WHITE;
        break;
      case 'b':
        to_move_ = BLACK;
        break;
      default:
        throw SayuriError("FENを解析できません。 in FEN::EvalToMove()");
        break;
    }
  }

  // キャスリングの権利文字列を評価する。
  void FEN::EvalCastlingRights(const std::string& castling_rights_str) {
    castling_rights_ = 0;

    if (castling_rights_str[0] == '-') return;

    for (auto& a : castling_rights_str) {
      switch (a) {
        case 'K':
          castling_rights_ |= WHITE_SHORT_CASTLING;
          break;
        case 'Q':
          castling_rights_ |= WHITE_LONG_CASTLING;
          break;
        case 'k':
          castling_rights_ |= BLACK_SHORT_CASTLING;
          break;
        case 'q':
          castling_rights_ |= BLACK_LONG_CASTLING;
          break;
        default:
          throw SayuriError
          ("FENを解析できません。 in FEN::EvalCastlingRights()");
          break;
      }
    }
  }

  // アンパッサン文字列を評価する。
  void FEN::EvalEnPassant(const std::string& en_passant_square_str) {
    // アンパッサンがない。
    if (en_passant_square_str[0] == '-') {
      en_passant_square_ = 0;
      return;
    }

    // ファイルから評価。
    int index = en_passant_square_str[0] - 'a';
    if ((index < 0) || (index > 7)) {
      throw SayuriError("FENを解析できません。 in FEN::EvalEnPassant()");
    }
    Bitboard fyle = Util::FYLE[index];

    // ランクを評価。
    if (en_passant_square_str[1] == '3') {
      index = RANK_3;
    } else if (en_passant_square_str[1] == '6') {
      index = RANK_6;
    } else {
      throw SayuriError("FENを解析できません。 in FEN::EvalEnPassant()");
    }
    Bitboard rank = Util::RANK[index];

    en_passant_square_ = Util::GetSquare(fyle & rank);
  }

  // 50手ルールの手数文字列を評価する。
  void FEN::EvalPly100(const std::string& ply_100_str) {
    try {
      ply_100_ = std::stoi(ply_100_str);
    } catch (...) {
      throw SayuriError("FENを解析できません。 in FEN::EvalPly100()");
    }
  }

  // 手数文字列を評価する。
  void FEN::EvalPly(const std::string& ply_str) {
    try {
      ply_ = std::stoi(ply_str) * 2;
      if (to_move_ == WHITE) {
        ply_ -= 1;
      }
    } catch (...) {
      throw SayuriError("FENを解析できません。 in FEN::EvalPly()");
    }
  }

  // ================ //
  // プライベート関数 //
  // ================ //
  // スタートポジションにセット。
  void FEN::SetStartPosition() {
    to_move_ = WHITE;
    castling_rights_ = ALL_CASTLING;
    en_passant_square_ = 0;
    ply_100_ = 0;
    ply_ = 1;

    // 駒を初期配置にする。
    // ポーン。
    position_[WHITE][PAWN] = Util::RANK[RANK_2];
    position_[BLACK][PAWN] = Util::RANK[RANK_7];
    // ナイト。
    position_[WHITE][KNIGHT] = Util::SQUARE[B1] | Util::SQUARE[G1];
    position_[BLACK][KNIGHT] = Util::SQUARE[B8] | Util::SQUARE[G8];
    // ビショップ。
    position_[WHITE][BISHOP] = Util::SQUARE[C1] | Util::SQUARE[F1];
    position_[BLACK][BISHOP] = Util::SQUARE[C8] | Util::SQUARE[F8];
    // ルーク。
    position_[WHITE][ROOK] = Util::SQUARE[A1] | Util::SQUARE[H1];
    position_[BLACK][ROOK] = Util::SQUARE[A8] | Util::SQUARE[H8];
    // クイーン。
    position_[WHITE][QUEEN] = Util::SQUARE[D1];
    position_[BLACK][QUEEN] = Util::SQUARE[D8];
    // キング。
    position_[WHITE][KING] = Util::SQUARE[E1];
    position_[BLACK][KING] = Util::SQUARE[E8];
  }
}  // namespace Sayuri
