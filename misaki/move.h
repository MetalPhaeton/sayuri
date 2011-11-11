/* move.h: 手を表す。
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

#ifndef MOVE_H
#define MOVE_H

#include <iostream>
#include <vector>
#include "chess_def.h"

namespace Misaki {
  class Move;
  class MoveList;

  /****************
   * 手のクラス。 *
   ****************/
  std::ostream& operator<<(std::ostream& stream, const Move& move);
  class Move {
    public:
      /****************************************
       * コンストラクタとデストラクタと代入。 *
       ****************************************/
      // コンストラクタ。
      Move() : piece_square_(A1), goal_square_(A1), promotion_(EMPTY) {}
      // コンストラクタ。
      // [引数]
      // piece_square: 移動する駒の位置。
      // goal_square: 移動先の位置。
      // promotion: 昇格する駒の位置。デフォルトは昇格しない。
      Move(square_t piece_square, square_t goal_square,
      piece_t promotion=EMPTY);
      // コピーコンストラクタ。
      // [引数]
      // move: コピー元のオブジェクト。
      Move(const Move& move);
      virtual ~Move() {}
      // 代入。
      Move& operator=(const Move& move);

      /************
       * 演算子。 *
       ************/
      // 同じ演算子。
      bool operator==(const Move& move) const {
        if (piece_square_ != move.piece_square_) return false;
        if (goal_square_ != move.goal_square_) return false;
        if (promotion_ != move.promotion_) return false;

        return true;
      }
      // 違う演算子。
      bool operator!=(const Move& move) const {
        if (piece_square_ != move.piece_square_) return true;
        if (goal_square_ != move.goal_square_) return true;
        if (promotion_ != move.promotion_) return true;

        return false;
      }

      /**************
       * アクセサ。 *
       **************/
      // 動かす駒の位置。
      square_t piece_square() const {return piece_square_;}
      // 移動先の位置。
      square_t goal_square() const {return goal_square_;}
      // 昇格する駒の種類。
      piece_t promotion() const {return promotion_;}

    private:

      /****************
       * 出力演算子。 *
       ****************/
      friend std::ostream& operator<<(std::ostream& stream, const Move& move);

      /****************
       * メンバ変数。 *
       ****************/
      // 動かす駒の位置。
      square_t piece_square_;
      // 移動先の位置。
      square_t goal_square_;
      // 昇格する駒の種類。
      piece_t promotion_;
  };

  /************************
   * 手のリストのクラス。 *
   ************************/
  std::ostream& operator<<(std::ostream& stream, const MoveList& move_list);
  class MoveList {
    public:
      /************************************
       * デストラクタとインスタンス生成。 *
       ************************************/
      // インスタンスの生成。
      // [戻り値]
      // 手のリストのインスタンス。
      static MoveList* New() {
        return new MoveList();
      }
      // デストラクタ。
      virtual ~MoveList() {}

      /****************
       * 出力演算子。 *
       ****************/
      friend std::ostream& operator<<(std::ostream& stream,
      const MoveList& move_list);

      /************
       * 演算子。 *
       ************/
      // 手の追加演算子。
      MoveList& operator+=(const Move& move) {
        move_vector_.push_back(move);
        return *this;
      }
      // 手のリストの追加の演算子。
      MoveList& operator+=(const MoveList& move_list);
      // インデックス演算子。
      Move operator[](int index) const {return move_vector_[index];}

      /**********
       * 関数。 *
       **********/
      // 手を追加する。
      // [引数]
      // piece_square: 移動する駒の位置。
      // goal_square: 移動先の位置。
      // promotion: 昇格する駒の種類。
      void Add(square_t piece_square, square_t goal_square,
      piece_t promotion=EMPTY) {
        move_vector_.push_back(Move(piece_square, goal_square, promotion));
      }
      // リストのサイズを得る。
      // [戻り値]
      // リストのサイズ。
      int GetSize() const {return move_vector_.size();}

    protected:
      /********************
       * コンストラクタ。 *
       ********************/
      MoveList() {}

    private:
      /****************
       * コピー禁止。 *
       ****************/
      MoveList(const MoveList&);  // 削除。
      MoveList& operator=(const MoveList&);  // 削除。

      /****************
       * メンバ変数。 *
       ****************/
      // 手のベクトル。
      std::vector<Move> move_vector_;
  };
}  // Misaki

#endif
