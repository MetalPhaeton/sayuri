/* opening_book.cpp: オープニングのブック。
   Copyright (c) 2011 Ishibashi Hironori

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

#include "opening_book.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <boost/tokenizer.hpp>
#include "chess_def.h"
#include "chess_util.h"
#include "move.h"
#include "game_record.h"

namespace Sayuri {
  /**************************
   * オープニングのクラス。 *
   **************************/
  // コンストラクタ。
  Opening::Opening(const bitboard_t (& position)[NUM_SIDES][NUM_PIECE_TYPES],
  castling_t castling_rights, square_t en_passant_target,
  bool can_en_passant, side_t to_move, Move next_move) :
  castling_rights_(castling_rights),
  en_passant_target_(en_passant_target),
  can_en_passant_(can_en_passant),
  to_move_(to_move),
  next_move_(next_move) {
    // 駒の配置をコピー。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = position[side][piece_type];
      }
    }
  }
  Opening::Opening(const GameRecord& record, Move next_move) :
  castling_rights_(record.castling_rights()),
  en_passant_target_(record.en_passant_target()),
  can_en_passant_(record.can_en_passant()),
  to_move_(record.to_move()),
  next_move_(next_move) {
    // 駒の配置をコピー。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = record.position()[side][piece_type];
      }
    }
  }
  // コンストラクタ。
  Opening::Opening(std::string csv_record) throw (bool) :
  castling_rights_(0),
  en_passant_target_(A1),
  can_en_passant_(false),
  to_move_(NO_SIDE),
  next_move_(A1, A1) {
    // position_を0で初期化。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = 0;
      }
    }

    // トークナイザーを作る。
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    typedef boost::char_separator<char> separator;
    separator sep(",", "", boost::keep_empty_tokens);
    tokenizer tok(csv_record, sep);
    tokenizer::iterator itr = tok.begin();

    // パースする。
    ParsePosition(*itr);
    ++itr;
    ParseCastlingRights(*itr);
    ++itr;
    ParseEnPassantTarget(*itr);
    ++itr;
    ParseToMove(*itr);
    ++itr;
    ParseNextMove(*itr);
  }
  // コピーコンストラクタ。
  Opening::Opening(const Opening& opening) :
  castling_rights_(opening.castling_rights_),
  en_passant_target_(opening.en_passant_target_),
  can_en_passant_(opening.can_en_passant_),
  to_move_(opening.to_move_),
  next_move_(opening.next_move_) {
    // position_をコピー。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = opening.position_[side][piece_type];
      }
    }
  }
  // 代入。
  Opening& Opening::operator=(const Opening& opening) {
    // position_をコピー。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = opening.position_[side][piece_type];
      }
    }

    // その他をコピー。
    castling_rights_ = opening.castling_rights_;
    en_passant_target_ = opening.en_passant_target_;
    can_en_passant_ = opening.can_en_passant_;
    to_move_ = opening.to_move_;
    next_move_ = opening.next_move_;

    return *this;
  }
  // 同じ演算子。
  bool Opening::operator==(const Opening& opening) const {
    // メンバを比較。
    if (castling_rights_ != opening.castling_rights_) return false;
    if (can_en_passant_ != opening.can_en_passant_) return false;
    if (can_en_passant_) {
      if (en_passant_target_ != opening.en_passant_target_) return false;
    }
    if (to_move_ != opening.to_move_) return false;
    if (next_move_ != opening.next_move_) return false;

    // 駒の配置を比較。
    for (int side = WHITE; side < NUM_SIDES; side++) {
      for (int piece_type = PAWN; piece_type < NUM_PIECE_TYPES; piece_type++) {
        if (position_[side][piece_type]
        != opening.position_[side][piece_type]) {
          return false;
        }
      }
    }

    return true;
  }
  // 違う演算子。
  bool Opening::operator!=(const Opening& opening) const {
    // メンバを比較。
    if (castling_rights_ != opening.castling_rights_) return true;
    if (can_en_passant_ != opening.can_en_passant_) return true;
    if (can_en_passant_) {
      if (en_passant_target_ != opening.en_passant_target_) return true;
    }
    if (to_move_ != opening.to_move_) return true;
    if (next_move_ != opening.next_move_) return true;

    // 駒の配置を比較。
    for (int side = WHITE; side < NUM_SIDES; side++) {
      for (int piece_type = PAWN; piece_type < NUM_PIECE_TYPES; piece_type++) {
        if (position_[side][piece_type]
        != opening.position_[side][piece_type]) {
          return true;
        }
      }
    }

    return false;
  }
  // オープニングのCSVレコードを得る。
  std::string Opening::GetCSVRecord() const {
    // ファイルとランクと駒の種類の配列。
    static const char fyle_array[NUM_FYLES] = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
    };
    static const char rank_array[NUM_RANKS] = {
      '1', '2', '3', '4', '5', '6', '7', '8'
    };
    static const char piece_type_array[NUM_PIECE_TYPES] = {
      '\0', 'P', 'N', 'B', 'R', 'Q', 'K'
    };

    // 駒の配置を文字ストリームに入れる。
    std::ostringstream position_stream;
    for (bitboard_t point = 1ULL; point; point <<= 1) {
      if (position_[WHITE][PAWN] & point) {
        position_stream << "P";
      } else if (position_[WHITE][KNIGHT] & point) {
        position_stream << "N";
      } else if (position_[WHITE][BISHOP] & point) {
        position_stream << "B";
      } else if (position_[WHITE][ROOK] & point) {
        position_stream << "R";
      } else if (position_[WHITE][QUEEN] & point) {
        position_stream << "Q";
      } else if (position_[WHITE][KING] & point) {
        position_stream << "K";
      } else if (position_[BLACK][PAWN] & point) {
        position_stream << "p";
      } else if (position_[BLACK][KNIGHT] & point) {
        position_stream << "n";
      } else if (position_[BLACK][BISHOP] & point) {
        position_stream << "b";
      } else if (position_[BLACK][ROOK] & point) {
        position_stream << "r";
      } else if (position_[BLACK][QUEEN] & point) {
        position_stream << "q";
      } else if (position_[BLACK][KING] & point) {
        position_stream << "k";
      } else {
        position_stream << "-";
      }
    }

    // キャスリングの権利を文字列ストリームに入れる。
    std::ostringstream castling_stream;
    if (castling_rights_ & WHITE_SHORT_CASTLING) {
      castling_stream << 'w';
    }
    if (castling_rights_ & WHITE_LONG_CASTLING) {
      castling_stream << 'W';
    }
    if (castling_rights_ & BLACK_SHORT_CASTLING) {
      castling_stream << 'b';
    }
    if (castling_rights_ & BLACK_LONG_CASTLING) {
      castling_stream << 'B';
    }

    // アンパッサンのターゲットを文字列ストリームに入れる。
    std::ostringstream en_passant_stream;
    if (can_en_passant_) {
      en_passant_stream << fyle_array[ChessUtil::GetFyle(en_passant_target_)];
      en_passant_stream << rank_array[ChessUtil::GetRank(en_passant_target_)];
    }

    // 手番を文字列ストリームに入れる。
    std::ostringstream to_move_stream;
    if (to_move_ == WHITE) to_move_stream << 'w';
    else to_move_stream << 'b';

    // 次の手を文字列ストリームに入れる。
    std::ostringstream next_move_stream;
    next_move_stream <<
    fyle_array[ChessUtil::GetFyle(next_move_.piece_square())];
    next_move_stream <<
    rank_array[ChessUtil::GetRank(next_move_.piece_square())];
    next_move_stream <<
    fyle_array[ChessUtil::GetFyle(next_move_.goal_square())];
    next_move_stream <<
    rank_array[ChessUtil::GetRank(next_move_.goal_square())];
    if (next_move_.promotion()) {
      next_move_stream << piece_type_array[next_move_.promotion()];
    }

    // 全てのフィールををつなぐ。
    std::ostringstream record_stream;
    record_stream << position_stream.str() << ',';
    record_stream << castling_stream.str() << ',';
    record_stream << en_passant_stream.str() << ',';
    record_stream << to_move_stream.str() << ',';
    record_stream << next_move_stream.str();

    // レコードを返す。
    return record_stream.str();
  }
  // 同じ演算子。
  bool Opening::operator==(const GameRecord& record) const {
    // 同じ駒の配置かどうかチェックする。
    for (int side = WHITE; side < NUM_SIDES; side++) {
      for (int piece_type = PAWN; piece_type < NUM_PIECE_TYPES; piece_type++) {
        if (position_[side][piece_type]
        != record.position()[side][piece_type]) {
          return false;
        }
      }
    }

    // それ以外が同じかどうかチェックする。
    if (castling_rights_ != record.castling_rights()) return false;
    if (can_en_passant_ != record.can_en_passant()) return false;
    if (can_en_passant_) {
      if (en_passant_target_ != record.en_passant_target()) return false;
    }
    if (to_move_ != record.to_move()) return false;

    // 全部チェックしたのでtrue。
    return true;
  }
  // 違う演算子。
  bool Opening::operator!=(const GameRecord& record) const {
    // 違う駒の配置かどうかチェックする。
    for (int side = WHITE; side < NUM_SIDES; side++) {
      for (int piece_type = PAWN; piece_type < NUM_PIECE_TYPES; piece_type++) {
        if (position_[side][piece_type]
        != record.position()[side][piece_type]) {
          return true;
        }
      }
    }

    // それ以外が違うかどうかチェックする。
    if (castling_rights_ != record.castling_rights()) return true;
    if (can_en_passant_ != record.can_en_passant()) return true;
    if (can_en_passant_) {
      if (en_passant_target_ != record.en_passant_target()) return true;
    }
    if (to_move_ != record.to_move()) return true;

    // 全部チェックしたのでfalse。
    return false;
  }
  // 駒の配置をパース。
  void Opening::ParsePosition(std::string position_str)
  throw (bool) {
    // 文字数を確認。
    if (position_str.length() != NUM_SQUARES) throw false;

    // パースする。
    bitboard_t point = 1ULL;
    for (int index = 0; index < NUM_SQUARES; index++) {
      // 各駒をposition_にセット。
      if (position_str[index] == 'P') {  // 白ポーン。
        position_[WHITE][PAWN] |= point;
      } else if (position_str[index] == 'N') {  // 白ナイト。
        position_[WHITE][KNIGHT] |= point;
      } else if (position_str[index] == 'B') {  // 白ビショップ。
        position_[WHITE][BISHOP] |= point;
      } else if (position_str[index] == 'R') {  // 白ルーク。
        position_[WHITE][ROOK] |= point;
      } else if (position_str[index] == 'Q') {  // 白クイーン。
        position_[WHITE][QUEEN] |= point;
      } else if (position_str[index] == 'K') {  // 白キング。
        position_[WHITE][KING] |= point;
      } else if (position_str[index] == 'p') {  // 黒ポーン。
        position_[BLACK][PAWN] |= point;
      } else if (position_str[index] == 'n') {  // 黒ナイト。
        position_[BLACK][KNIGHT] |= point;
      } else if (position_str[index] == 'b') {  // 黒ビショップ。
        position_[BLACK][BISHOP] |= point;
      } else if (position_str[index] == 'r') {  // 黒ルーク。
        position_[BLACK][ROOK] |= point;
      } else if (position_str[index] == 'q') {  // 黒クイーン。
        position_[BLACK][QUEEN] |= point;
      } else if (position_str[index] == 'k') {  // 黒キング。
        position_[BLACK][KING] |= point;
      } else if (position_str[index] == '-') {  // 空のマス。
        // 何もしない。
      } else {  // それ以外は例外。
        throw false;
      }

      point <<= 1;
    }
  }
  // キャスリングの権利をパース。
  void Opening::ParseCastlingRights(std::string castling_rights_str)
  throw (bool) {
    // パースする。
    for (int index = 0; index < castling_rights_str.length(); index++) {
      if (castling_rights_str[index] == 'w') {
        castling_rights_ |= WHITE_SHORT_CASTLING;
      } else if (castling_rights_str[index] == 'W') {
        castling_rights_ |= WHITE_LONG_CASTLING;
      } else if (castling_rights_str[index] == 'b') {
        castling_rights_ |= BLACK_SHORT_CASTLING;
      } else if (castling_rights_str[index] == 'B') {
        castling_rights_ |= BLACK_LONG_CASTLING;
      } else {
        throw false;
      }
    }
  }
  // アンパッサンのターゲットをパース。
  void Opening::ParseEnPassantTarget(
  std::string en_passant_target_str) throw (bool) {
    if (en_passant_target_str == "") {
      can_en_passant_ = false;
      return;
    } else {
      can_en_passant_ = true;
      en_passant_target_ = ParseSquare(en_passant_target_str);
    }
  }
  // 手番をパース。
  void Opening::ParseToMove(std::string to_move_str) throw (bool) {
    // パースする。
    if (to_move_str == "w") to_move_ = WHITE;
    else if (to_move_str == "b") to_move_ = BLACK;
    else throw false;
  }
  // 次の手をパース。
  void Opening::ParseNextMove(std::string next_move_str)
  throw (bool) {
    // 文字の長さをチェック。
    if ((next_move_str.length() != 4) && (next_move_str.length() != 5))
      throw false;

    // 駒の位置をパース。
    square_t piece_square = ParseSquare(next_move_str.substr(0, 2));

    // 移動先の位置をパース。
    square_t goal_square = ParseSquare(next_move_str.substr(2, 2));

    // 昇格する駒の種類をパース。
    piece_t promotion = EMPTY;
    if (next_move_str.length() == 5) {
      if (next_move_str[4] == 'N') {
        promotion = KNIGHT;
      } else if (next_move_str[4] == 'B') {
        promotion = BISHOP;
      } else if (next_move_str[4] == 'R') {
        promotion = ROOK;
      } else if (next_move_str[4] == 'Q') {
        promotion = QUEEN;
      } else {  // パースできないので例外。
        throw false;
      }
    }

    next_move_ = Move(piece_square, goal_square, promotion);
  }
  // 位置をパース。
  square_t Opening::ParseSquare(std::string square_str)
  throw (bool) {
    // ファイルとランクの文字を得る。
    char fyle_c = square_str[0];
    char rank_c = square_str[1];
    if ((fyle_c < 'a') || (fyle_c > 'h')) throw false;
    if ((rank_c < '1') || (rank_c > '8')) throw false;

    // ファイルとランクを得る。
    fyle_t fyle = static_cast<fyle_t>(fyle_c - 'a');
    rank_t rank = static_cast<rank_t>(rank_c - '1');

    // 位置を返す。
    return static_cast<square_t>((rank << 3) | fyle);
  }

  /********************************
   * オープニングブックのクラス。 *
   ********************************/
  // 削除演算子。
  OpeningBook& OpeningBook::operator-=(const Opening& opening) {
    std::vector<Opening>::iterator itr = opening_vector_.begin();
    while (itr != opening_vector_.end()) {
      if (*itr == opening) {
        opening_vector_.erase(itr);
        break;
      }
      ++itr;
    }
    return *this;
  }
  MoveList* OpeningBook::CreateNextMoveList(const GameRecord& record) const {
    // 手のリスト。
    MoveList* move_list = MoveList::New();

    // 手を探して追加していく。
    std::vector<Opening>::const_iterator itr = opening_vector_.begin();
    while (itr != opening_vector_.end()) {
      if (*itr == record) *move_list += itr->next_move();
      ++itr;
    }

    return move_list;
  }
}  // namespace Sayuri
