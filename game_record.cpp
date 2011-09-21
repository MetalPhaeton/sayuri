/* game_record.cpp: ゲームの記録のデータ。
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

#include "chess_board.h"

#include <iostream>
#include <sstream>
#include "chess_def.h"
#include "chess_util.h"

namespace Misaki {
  /****************
   * 出力演算子。 *
   ****************/
  std::ostream& operator<<(std::ostream& stream, const GameRecord& record) {
    // ボードの文字列を入れる配列。
    char string_board[NUM_SQUARES][4];

    //ボードの文字列に文字を入れる。
    side_t side;
    piece_t piece_type;
    for (int square = 0; square < NUM_SQUARES; square++) {
      side = record.GetSide(static_cast<square_t>(square));
      piece_type = record.GetPieceType(static_cast<square_t>(square));
      string_board[square][3] = '\0';
      // 駒のサイドの文字を入れる。
      if (side == WHITE) {
        string_board[square][0] = '-';
        string_board[square][2] = '-';
      } else if (side == BLACK) {
        string_board[square][0] = '<';
        string_board[square][2] = '>';
      } else {
        string_board[square][0] = ' ';
        string_board[square][2] = ' ';
      }
      // 駒の種類の文字を入れる。
      switch (piece_type) {
        case PAWN:
          string_board[square][1] = 'P';
          break;
        case KNIGHT:
          string_board[square][1] = 'N';
          break;
        case BISHOP:
          string_board[square][1] = 'B';
          break;
        case ROOK:
          string_board[square][1] = 'R';
          break;
        case QUEEN:
          string_board[square][1] = 'Q';
          break;
        case KING:
          string_board[square][1] = 'K';
          break;
        default:
          string_board[square][1] = ' ';
          break;
      }
    }

    // ファイルとランクの文字の配列。
    static const char fyle_array[NUM_FYLES] = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
    };
    static const char rank_array[NUM_RANKS] = {
      '1', '2', '3', '4', '5', '6', '7', '8'
    };

    // ファイルとランク。
    fyle_t fyle;
    rank_t rank;

    // プライを文字列ストリームに入れる。
    std::ostringstream ply_stream;
    ply_stream << "Ply: " << record.ply_;

    // 手番を文字列ストリームに入れる。
    std::ostringstream to_move_stream;
    to_move_stream << "To Move: ";
    if (record.to_move_ == WHITE) to_move_stream << "White";
    else if (record.to_move_ == BLACK) to_move_stream << "Black";

    // 白のキャスリングの権利を文字列ストリームに入れる。
    std::ostringstream white_castling_stream;
    white_castling_stream << "White Castling: ";
    if (record.castling_rights_ & WHITE_SHORT_CASTLING)
      white_castling_stream << "Short ";
    if (record.castling_rights_ & WHITE_LONG_CASTLING)
      white_castling_stream << "Long ";

    // 黒のキャスリングの権利を文字列ストリームに入れる。
    std::ostringstream black_castling_stream;
    black_castling_stream << "Black Castling: ";
    if (record.castling_rights_ & BLACK_SHORT_CASTLING)
      black_castling_stream << "Short ";
    if (record.castling_rights_ & BLACK_LONG_CASTLING)
      black_castling_stream << "Long ";

    // アンパッサンのターゲットを文字列ストリームに入れる。
    std::ostringstream en_passant_stream;
    en_passant_stream << "En Passant Target: ";
    if (record.can_en_passant_) {
      fyle = ChessUtil::GetFyle(record.en_passant_target_);
      rank = ChessUtil::GetRank(record.en_passant_target_);
      en_passant_stream << fyle_array[fyle] << rank_array[rank];
    }

    // 50手ルールの手数を文字列ストリームに入れる。
    std::ostringstream ply_100_stream;
    ply_100_stream << "Ply 100: " << record.ply_100_;

    // 繰り返しの回数を文字列ストリームに入れる。
    std::ostringstream repetition_stream;
    repetition_stream << "Repetition: " << record.repetition_;

    // 最後の手が有効かどうか。
    bool is_last_move = (record.last_move_.piece_square_
    != record.last_move_.goal_square_ ? true : false);

    // 最後の手の1行目を文字列ストリームに入れる。
    std::ostringstream last_move_stream1;
    last_move_stream1 << "<Last Move>";

    // 最後の手の2行目を文字列ストリームに入れる。
    std::ostringstream last_move_stream2;
    last_move_stream2 << "  Piece Square: ";
    if (is_last_move) {
      fyle = ChessUtil::GetFyle(record.last_move_.piece_square_);
      rank = ChessUtil::GetRank(record.last_move_.piece_square_);
      last_move_stream2 << fyle_array[fyle] << rank_array[rank];
    }

    // 最後の手の3行目を文字列ストリームに入れる。
    std::ostringstream last_move_stream3;
    last_move_stream3 << "  Goal Square: ";
    if (is_last_move) {
      fyle = ChessUtil::GetFyle(record.last_move_.goal_square_);
      rank = ChessUtil::GetRank(record.last_move_.goal_square_);
      last_move_stream3 << fyle_array[fyle] << rank_array[rank];
    }

    // 最後の手の4行目を文字列ストリームに入れる。
    std::ostringstream last_move_stream4;
    last_move_stream4 << "  Promotion: ";
    switch (record.last_move_.promotion_) {
      case KNIGHT:
        last_move_stream4 << "Knight";
        break;
      case BISHOP:
        last_move_stream4 << "Bishop";
        break;
      case ROOK:
        last_move_stream4 << "Rook";
        break;
      case QUEEN:
        last_move_stream4 << "Queen";
        break;
    }

    // ボードの境界線。
    static const char* border = " +---+---+---+---+---+---+---+---+";

    // ボードのファイルの表記。
    static const char* fyle_row = "   a   b   c   d   e   f   g   h";

    // 1行目を出力。
    stream << border << "  " << ply_stream.str() << std::endl;

    // 2行目を出力。
    stream << "8|";
    for (int index = 56; index <= 63; index++) {
      stream << string_board[index] << "|";
    }
    stream << "  " << to_move_stream.str() << std::endl;

    // 3行目を出力。
    stream << border << "  " << white_castling_stream.str() << std::endl;

    // 4行目を出力。
    stream << "7|";
    for (int index = 48; index <= 55; index++) {
      stream << string_board[index] << "|";
    }
    stream << "  " << black_castling_stream.str() << std::endl;

    // 5行目を出力。
    stream << border << "  " << en_passant_stream.str() << std::endl;

    // 6行目を出力。
    stream << "6|";
    for (int index = 40; index <= 47; index++) {
      stream << string_board[index] << "|";
    }
    stream << "  " << ply_100_stream.str() << std::endl;

    // 7行目を出力。
    stream << border << "  " << repetition_stream.str() << std::endl;

    // 8行目を出力。
    stream << "5|";
    for (int index = 32; index <= 39; index++) {
      stream << string_board[index] << "|";
    }
    stream << "  " << last_move_stream1.str() << std::endl;

    // 9行目を出力。
    stream << border << "  " << last_move_stream2.str() << std::endl;

    // 10行目を出力。
    stream << "4|";
    for (int index = 24; index <= 31; index++) {
      stream << string_board[index] << "|";
    }
    stream << "  " << last_move_stream3.str() << std::endl;

    // 11行目を出力。
    stream << border << "  " << last_move_stream4.str() << std::endl;

    // 12行目を出力。
    stream << "3|";
    for (int index = 16; index <= 23; index++) {
      stream << string_board[index] << "|";
    }
    stream << std::endl;

    // 13行目を出力。
    stream << border << std::endl;

    // 14行目を出力。
    stream << "2|";
    for (int index = 8; index <= 15; index++) {
      stream << string_board[index] << "|";
    }
    stream << std::endl;

    // 15行目を出力。
    stream << border << std::endl;

    // 16行目を出力。
    stream << "1|";
    for (int index = 0; index <= 7; index++) {
      stream << string_board[index] << "|";
    }
    stream << std::endl;

    // 17行目を出力。
    stream << border << std::endl;

    // 18行目を出力。
    stream << fyle_row << std::endl;

    return stream;
  }

  /**************************
   * コンストラクタと代入。 *
   **************************/
  // コンストラクタ。
  GameRecord::GameRecord(const ChessBoard& board,
  int ply, int ply_100, int repetition, move_t last_move, hash_key_t key) :
  to_move_(board.to_move_),
  ply_(ply),
  castling_rights_(board.castling_rights_),
  en_passant_target_(board.en_passant_target_),
  can_en_passant_(board.can_en_passant_),
  ply_100_(ply_100),
  repetition_(repetition),
  last_move_(last_move),
  key_(key) {
    // 駒の配置をコピーする。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = board.position_[side][piece_type];
      }
    }
  }
  // コピーコンストラクタ。
  GameRecord::GameRecord(const GameRecord& record) :
  to_move_(record.to_move_),
  ply_(record.ply_),
  castling_rights_(record.castling_rights_),
  en_passant_target_(record.en_passant_target_),
  can_en_passant_(record.can_en_passant_),
  ply_100_(record.ply_100_),
  repetition_(record.repetition_),
  last_move_(record.last_move_) {
    // 駒の配置をコピーする。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = record.position_[side][piece_type];
      }
    }
  }
  // 代入。
  GameRecord& GameRecord::operator=(const GameRecord& record) {
    // 駒の配置をコピーする。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
        position_[side][piece_type] = record.position_[side][piece_type];
      }
    }
    // その他のメンバをコピー。
    to_move_ = record.to_move_;
    ply_ = record.ply_;
    castling_rights_ = record.castling_rights_;
    en_passant_target_ = record.en_passant_target_;
    can_en_passant_ = record.can_en_passant_;
    ply_100_ = record.ply_100_;
    repetition_ = record.repetition_;
    last_move_ = record.last_move_;

    return *this;
  }

  /**********
   * 関数。 *
   **********/
  // 駒の種類を得る。
  piece_t GameRecord::GetPieceType(square_t piece_square) const {
    // 駒の種類。
    piece_t piece_type = EMPTY;

    // 駒の種類を探して入れる。
    for (int index1 = 0; index1 < NUM_SIDES; index1++) {
      for (int index2 = 0; index2 < NUM_PIECE_TYPES; index2++) {
        if (position_[index1][index2] & ChessUtil::BIT[piece_square]) {
          piece_type = static_cast<piece_t>(index2);
          break;
        }
      }
    }

    return piece_type;
  }
  // 駒のサイドを得る。
  side_t GameRecord::GetSide(square_t piece_square) const {
    // 駒のサイド。
    side_t side = NO_SIDE;

    // 駒のサイドを探して入れる。
    for (int index1 = 0; index1 < NUM_SIDES; index1++) {
      for (int index2 = 0; index2 < NUM_PIECE_TYPES; index2++) {
        if (position_[index1][index2] & ChessUtil::BIT[piece_square]) {
          side = static_cast<side_t>(index1);
          break;
        }
      }
    }

    return side;
  }

  /**********************
   * プライベート関数。 *
   **********************/
  // ボードと同じ駒の配置かどうか調べる。
  bool GameRecord::EqualsPosition(const ChessBoard& board) const {
    for (int side = WHITE; side < NUM_SIDES; side++) {
      for (int piece_type = PAWN; piece_type < NUM_PIECE_TYPES; piece_type++) {
        if (position_[side][piece_type] != board.position_[side][piece_type])
          return false;
      }
    }

    return true;
  }
}  // Misaki
