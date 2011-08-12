/* move.cpp: 手を表す。
   copyright (c) 2011 石橋宏之利
 */

#include "move.h"

#include <iostream>
#include "chess_def.h"
#include "chess_util.h"

namespace Misaki {
  /****************
   * 出力演算子。 *
   ****************/
  // Moveの出力演算子。
  std::ostream& operator<<(std::ostream& stream, const Move& move) {
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

    // 移動する駒の位置を出力。
    fyle = ChessUtil::GetFyle(move.piece_square_);
    rank = ChessUtil::GetRank(move.piece_square_);
    stream << "Piece Square: " <<
    fyle_array[fyle] << rank_array[rank] << std::endl;

    // 移動先の位置を出力。
    fyle = ChessUtil::GetFyle(move.goal_square_);
    rank = ChessUtil::GetRank(move.goal_square_);
    stream << "Goal Square: " <<
    fyle_array[fyle] << rank_array[rank] << std::endl;

    // 昇格する駒の種類を出力。
    stream << "Promotion: ";
    switch (move.promotion_) {
      case KNIGHT:
        stream << "Knight";
        break;
      case BISHOP:
        stream << "Bishop";
        break;
      case ROOK:
        stream << "Rook";
        break;
      case QUEEN:
        stream << "Queen";
        break;
    }
    stream << std::endl;

    return stream;
  }
  // MoveListの出力演算子。
  std::ostream& operator<<(std::ostream& stream, const MoveList& move_list) {
    for (int index = 0; index < move_list.GetSize(); index++) {
      stream << "<Move[" << index << "]>" << std::endl;
      stream << move_list[index];
    }
  }

  /**************************
   * コンストラクタと代入。 *
   **************************/
  // Moveのコンストラクタ。
  Move::Move(square_t piece_square, square_t goal_square, piece_t promotion) :
  piece_square_(piece_square),
  goal_square_(goal_square),
  promotion_(promotion) {
    // 昇格の駒を整理する。
    if ((promotion_ != EMPTY) && (promotion_ != KNIGHT)
    && (promotion_ != BISHOP) && (promotion_ != ROOK)
    && (promotion_ != QUEEN))
      promotion_ = EMPTY;
  }
  // Moveのコピーコンストラクタ。
  Move::Move(const Move& move) :
  piece_square_(move.piece_square_),
  goal_square_(move.goal_square_),
  promotion_(move.promotion_) {}
  // Moveの代入。
  Move& Move::operator=(const Move& move) {
    piece_square_ = move.piece_square_;
    goal_square_ = move.goal_square_;
    promotion_ = move.promotion_;
    return *this;
  }

  /********************
   * MoveListの関数。 *
   ********************/
  MoveList& MoveList::operator+=(const MoveList& move_list) {
    for (int index = 0; index < move_list.move_vector_.size(); index++) {
      move_vector_.push_back(move_list[index]);
    }
    return *this;
  }
}  // Misaki
