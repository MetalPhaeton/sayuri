/* 
   fen.cpp: fenパーサの実装ファイル。
   Copyright (c) 2013 Ishibashi Hironori

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
#include "sayuri_error.h"

#include "mylib.h"

namespace Sayuri {
  /********************/
  /* コンストラクタ。 */
  /********************/
  // コンストラクタ。
  Fen::Fen(const std::string fen_str) {
    // fen_strを分解。
    std::vector<std::string> fen_tokens = MyLib::Split(fen_str, " ", "");

    try {
      // パース。
      constexpr int index_position = 0;
      constexpr int index_to_move = 1;
      constexpr int index_castling_rights = 2;
      constexpr int index_en_passant = 3;
      constexpr int index_ply_100 = 4;
      constexpr int index_ply = 5;
      ParsePosition(fen_tokens[index_position]);
      ParseToMove(fen_tokens[index_to_move]);
      ParseCastlingRights(fen_tokens[index_castling_rights]);
      if (fen_tokens.size() > static_cast<size_t>(index_en_passant)) {
        ParseEnPassant(fen_tokens[index_en_passant]);
        if (fen_tokens.size() > static_cast<size_t>(index_ply_100)) {
          ParsePly100(fen_tokens[index_ply_100]);
          if (fen_tokens.size() > static_cast<size_t>(index_ply)) {
            ParsePly(fen_tokens[index_ply], fen_tokens[index_to_move]);
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
      throw SayuriError("FENをパース出来ません。");
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
    position_[WHITE][KNIGHT] = Util::BIT[B1] | Util::BIT[G1];
    position_[BLACK][KNIGHT] = Util::BIT[B8] | Util::BIT[G8];
    // ビショップ。
    position_[WHITE][BISHOP] = Util::BIT[C1] | Util::BIT[F1];
    position_[BLACK][BISHOP] = Util::BIT[C8] | Util::BIT[F8];
    // ルーク。
    position_[WHITE][ROOK] = Util::BIT[A1] | Util::BIT[H1];
    position_[BLACK][ROOK] = Util::BIT[A8] | Util::BIT[H8];
    // クイーン。
    position_[WHITE][QUEEN] = Util::BIT[D1];
    position_[BLACK][QUEEN] = Util::BIT[D8];
    // キング。
    position_[WHITE][KING] = Util::BIT[E1];
    position_[BLACK][KING] = Util::BIT[E8];
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
  /* パーサ。 */
  /************/
  // 駒の配置をパースする。
  void Fen::ParsePosition(const std::string& position_str) {
    // 駒の配置を初期化。
    for (int i = 0; i < NUM_SIDES; i++) {
      for (int j = 0; j < NUM_PIECE_TYPES; j++) {
        position_[i][j] = 0ULL;
      }
    }

    // ランク毎に区切って、逆転。
    // fenは8ランクから始まるので、1ランクからに逆転する必要がある。
    std::vector<std::string> position_vec = MyLib::Split(position_str, "/", "");
    std::reverse(position_vec.begin(), position_vec.end());

    // 値を格納していく。
    int square = 0;  // マスの位置。
    for (auto& a : position_vec) {
      for (auto& b : a) {

        // 文字をパース。
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
            position_[WHITE][PAWN] |= Util::BIT[square];
            break;
          case 'N':
            position_[WHITE][KNIGHT] |= Util::BIT[square];
            break;
          case 'B':
            position_[WHITE][BISHOP] |= Util::BIT[square];
            break;
          case 'R':
            position_[WHITE][ROOK] |= Util::BIT[square];
            break;
          case 'Q':
            position_[WHITE][QUEEN] |= Util::BIT[square];
            break;
          case 'K':
            position_[WHITE][KING] |= Util::BIT[square];
            break;
          case 'p':
            position_[BLACK][PAWN] |= Util::BIT[square];
            break;
          case 'n':
            position_[BLACK][KNIGHT] |= Util::BIT[square];
            break;
          case 'b':
            position_[BLACK][BISHOP] |= Util::BIT[square];
            break;
          case 'r':
            position_[BLACK][ROOK] |= Util::BIT[square];
            break;
          case 'q':
            position_[BLACK][QUEEN] |= Util::BIT[square];
            break;
          case 'k':
            position_[BLACK][KING] |= Util::BIT[square];
            break;
          default:
            throw SayuriError("FENをパース出来ません。");
            break;
        }
        square++;
        if (square > 64) {
          throw SayuriError("FENをパース出来ません。");
        }
      }
    }
  }
  // 手番をパースする。
  void Fen::ParseToMove(const std::string& to_move_str) {
    switch (to_move_str[0]) {
      case 'w':
        to_move_ = WHITE;
        break;
      case 'b':
        to_move_ = BLACK;
        break;
      default:
        throw SayuriError("FENをパース出来ません。");
        break;
    }
  }
  // キャスリングの権利をパースする。
  void Fen::ParseCastlingRights(const std::string& castling_rights_str) {
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
          throw SayuriError("FENをパース出来ません。");
          break;
      }
    }
  }
  // アンパッサンをパースする。
  void Fen::ParseEnPassant(const std::string& en_passant_str) {
    // アンパッサンがない。
    if (en_passant_str[0] == '-') {
      can_en_passant_ = false;
      en_passant_square_ = 0;
      return;
    }

    // アンパッサンがある。
    can_en_passant_ = true;
    // ファイルからパース。
    Bitboard fyle = 0ULL;
    int index = en_passant_str[0] - 'a';
    if ((index < 0) || (index > 7)) {
      throw SayuriError("FENをパース出来ません。");
    }
    fyle = Util::FYLE[index];
    // ランクをパース。
    Bitboard rank = 0ULL;
    index = (en_passant_str[1] - '0') - 1;
    if ((index < 0) || (index > 7)) {
      throw SayuriError("FENをパース出来ません。");
    }
    rank |= Util::RANK[index];

    en_passant_square_ = Util::GetSquare(fyle & rank);
  }
  // 50手ルールをパースする。
  void Fen::ParsePly100(const std::string& ply_100_str) {
    try {
      ply_100_ = std::stoi(ply_100_str);
    } catch (...) {
      throw SayuriError("FENをパース出来ません。");
    }
  }
  // 手数をバースする。
  void Fen::ParsePly(const std::string& ply_str,
  const std::string& to_move_str) {
    try {
      ply_ = std::stoi(ply_str) * 2;
      if (to_move_str[0] == 'w') {
        ply_ -= 1;
      }
    } catch (...) {
      throw SayuriError("FENをパース出来ません。");
    }
  }
}  // namespace Sayuri
