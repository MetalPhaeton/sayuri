/* 
   fen.cpp: fenパーサの実装ファイル。
   Copyright (c) 2014 Hironori Ishibashi

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

#include "fen.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>
#include "chess_def.h"
#include "chess_util.h"
#include "error.h"

namespace Sayuri {
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  Fen::Fen(const std::string fen_str) {
    // fen_strを分解。
    std::vector<std::string> fen_tokens = Util::Split(fen_str, " ", "");

    try {
      // FENを解析。
      constexpr unsigned int index_position = 0;
      constexpr unsigned int index_to_move = 1;
      constexpr unsigned int index_castling_rights = 2;
      constexpr unsigned int index_en_passant = 3;
      constexpr unsigned int index_ply_100 = 4;
      constexpr unsigned int index_ply = 5;
      EvalPosition(fen_tokens[index_position]);
      EvalToMove(fen_tokens[index_to_move]);
      EvalCastlingRights(fen_tokens[index_castling_rights]);
      if (fen_tokens.size() > index_en_passant) {
        EvalEnPassant(fen_tokens[index_en_passant]);
        if (fen_tokens.size() > index_ply_100) {
          EvalPly100(fen_tokens[index_ply_100]);
          if (fen_tokens.size() > index_ply) {
            EvalPly(fen_tokens[index_ply], fen_tokens[index_to_move]);
          } else {
            ply_ = 1;
          }
        } else {
          ply_100_ = 0;
          ply_ = 1;
        }
      } else {
        en_passant_square_ = 0;
        can_en_passant_ = false;
        ply_100_ = 0;
        ply_ = 1;
      }
    } catch (SayuriError error) {
      throw error;
    } catch (...) {
      throw SayuriError("FENを解析できません。");
    }

  }

  // デフォルトコンストラクタ。
  Fen::Fen() :
  to_move_(WHITE),
  castling_rights_(ALL_CASTLING),
  en_passant_square_(0),
  can_en_passant_(false),
  ply_100_(0),
  ply_(1) {
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
  // コピーコンストラクタ。
  Fen::Fen(const Fen& fen) :
  to_move_(fen.to_move_),
  castling_rights_(fen.castling_rights_),
  en_passant_square_(fen.en_passant_square_),
  can_en_passant_(fen.can_en_passant_),
  ply_100_(fen.ply_100_),
  ply_(fen.ply_) {
    // 駒の配置をコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = fen.position_[i][j];
      }
    }
  }
  // ムーブコンストラクタ。
  Fen::Fen(Fen&& fen) :
  to_move_(fen.to_move_),
  castling_rights_(fen.castling_rights_),
  en_passant_square_(fen.en_passant_square_),
  can_en_passant_(fen.can_en_passant_),
  ply_100_(fen.ply_100_),
  ply_(fen.ply_) {
    // 駒の配置をコピー。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = fen.position_[i][j];
      }
    }
  }
  // コピー代入。
  Fen& Fen::operator=(const Fen& fen) {
    // メンバをコピー。
    to_move_ = fen.to_move_;
    castling_rights_ = fen.castling_rights_;
    en_passant_square_ = fen.en_passant_square_;
    can_en_passant_ = fen.can_en_passant_;
    ply_100_ = fen.ply_100_;
    ply_ = fen.ply_;
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = fen.position_[i][j];
      }
    }

    return *this;
  }
  // ムーブ代入。
  Fen& Fen::operator=(Fen&& fen) {
    // メンバをコピー。
    to_move_ = fen.to_move_;
    castling_rights_ = fen.castling_rights_;
    en_passant_square_ = fen.en_passant_square_;
    can_en_passant_ = fen.can_en_passant_;
    ply_100_ = fen.ply_100_;
    ply_ = fen.ply_;
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = fen.position_[i][j];
      }
    }

    return *this;
  }

  /************/
  /* 評価器。 */
  /************/
  // 駒の配置トークンを評価する。
  void Fen::EvalPosition(const std::string& position_str) {
    // 駒の配置を初期化。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = 0ULL;
      }
    }

    // ランク毎に区切って、逆転。
    // fenは8ランクから始まるので、1ランクからに逆転する必要がある。
    std::vector<std::string> position_vec = Util::Split(position_str, "/", "");
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
            throw SayuriError("FENを解析できません。");
            break;
        }
        square++;
        if (square > 64) {
          throw SayuriError("FENを解析できません。");
        }
      }
    }
  }

  // 手番トークンを評価する。
  void Fen::EvalToMove(const std::string& to_move_str) {
    switch (to_move_str[0]) {
      case 'w':
        to_move_ = WHITE;
        break;
      case 'b':
        to_move_ = BLACK;
        break;
      default:
        throw SayuriError("FENを解析できません。");
        break;
    }
  }

  // キャスリングの権利トークンを評価する。
  void Fen::EvalCastlingRights(const std::string& castling_rights_str) {
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
          throw SayuriError("FENを解析できません。");
          break;
      }
    }
  }

  // アンパッサントークンを評価する。
  void Fen::EvalEnPassant(const std::string& en_passant_str) {
    // アンパッサンがない。
    if (en_passant_str[0] == '-') {
      can_en_passant_ = false;
      en_passant_square_ = 0;
      return;
    }

    // アンパッサンがある。
    can_en_passant_ = true;

    // ファイルから評価。
    int index = en_passant_str[0] - 'a';
    if ((index < 0) || (index > 7)) {
      throw SayuriError("FENを解析できません。");
    }
    Bitboard fyle = Util::FYLE[index];

    // ランクを評価。
    index = (en_passant_str[1] - '0') - 1;
    if ((index < 0) || (index > 7)) {
      throw SayuriError("FENを解析できません。");
    }
    Bitboard rank = Util::RANK[index];

    en_passant_square_ = Util::GetSquare(fyle & rank);
  }

  // 50手ルールトークンを評価する。
  void Fen::EvalPly100(const std::string& ply_100_str) {
    try {
      ply_100_ = std::stoi(ply_100_str);
    } catch (...) {
      throw SayuriError("FENを解析できません。");
    }
  }
  // 手数をバースする。
  void Fen::EvalPly(const std::string& ply_str,
  const std::string& to_move_str) {
    try {
      ply_ = std::stoi(ply_str) * 2;
      if (to_move_str[0] == 'w') {
        ply_ -= 1;
      }
    } catch (...) {
      throw SayuriError("FENを解析できません。");
    }
  }
}  // namespace Sayuri
