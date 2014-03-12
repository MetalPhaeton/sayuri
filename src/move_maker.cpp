/* 
   move_maker.cpp: 候補手を展開するクラスの実装。

   The MIT License (MIT)

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

#include "move_maker.h"

#include <iostream>
#include <mutex>
#include <cstddef>
#include <utility>
#include "chess_def.h"
#include "chess_engine.h"

namespace Sayuri {
  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  //コンストラクタ。
  MoveMaker::MoveMaker(const ChessEngine& engine) :
  engine_ptr_(&engine) {
    // スタックのポインターをセット。
    begin_ = last_ = move_stack_;
    end_ = &(move_stack_[MAX_CANDIDATES]);
  }

  // コピーコンストラクタ。
  MoveMaker::MoveMaker(const MoveMaker& maker) :
  engine_ptr_(maker.engine_ptr_) {
    for (std::size_t i = 0; i <= MAX_CANDIDATES; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.begin_ == &(maker.move_stack_[i])) {
        begin_ = &(move_stack_[i]);
      }
      if (maker.last_ == &(maker.move_stack_[i])) {
        last_ = &(move_stack_[i]);
      }
    }
  }

  // ムーブコンストラクタ。
  MoveMaker::MoveMaker(MoveMaker&& maker) :
  engine_ptr_(maker.engine_ptr_) {
    for (std::size_t i = 0; i <= MAX_CANDIDATES; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.begin_ == &(maker.move_stack_[i])) {
        begin_ = &(move_stack_[i]);
      }
      if (maker.last_ == &(maker.move_stack_[i])) {
        last_ = &(move_stack_[i]);
      }
    }
  }

  // コピー代入。
  MoveMaker& MoveMaker::operator=
  (const MoveMaker& maker) {
    engine_ptr_ = maker.engine_ptr_;
    for (std::size_t i = 0; i <= MAX_CANDIDATES; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.begin_ == &(maker.move_stack_[i])) {
        begin_ = &(move_stack_[i]);
      }
      if (maker.last_ == &(maker.move_stack_[i])) {
        last_ = &(move_stack_[i]);
      }
    }
    return *this;
  }

  // ムーブ代入。
  MoveMaker& MoveMaker::operator=
  (MoveMaker&& maker) {
    engine_ptr_ = maker.engine_ptr_;
    for (std::size_t i = 0; i <= MAX_CANDIDATES; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.begin_ == &(maker.move_stack_[i])) {
        begin_ = &(move_stack_[i]);
      }
      if (maker.last_ == &(maker.move_stack_[i])) {
        last_ = &(move_stack_[i]);
      }
    }
    return *this;
  }

  /******************/
  /* その他の関数。 */
  /******************/

  // 手をスタックに展開する。
  template<GenMoveType Type>
  int MoveMaker::GenMoves(Move prev_best, Move iid_move,
  Move killer_1, Move killer_2) {
    std::unique_lock<std::mutex> lock(mutex_);  // ロック。

    // サイド。
    Side side = engine_ptr_->to_move();
    Side enemy_side = side ^ 0x3;

    // 生成したての数。
    int num_moves = 0;

    // ナイト、ビショップ、ルーク、クイーンの候補手を作る。
    for (Piece piece_type = KNIGHT; piece_type <= QUEEN; piece_type++) {
      Bitboard pieces = engine_ptr_->position()[side][piece_type];

      for (; pieces; pieces &= pieces - 1) {
        Square from = Util::GetSquare(pieces);

        // 各ピースの動き。
        Bitboard move_bitboard;
        switch (piece_type) {
          case KNIGHT:
            move_bitboard = Util::GetKnightMove(from);
            break;
          case BISHOP:
            move_bitboard = engine_ptr_->GetBishopAttack(from);
            break;
          case ROOK:
            move_bitboard = engine_ptr_->GetRookAttack(from);
            break;
          case QUEEN:
            move_bitboard = engine_ptr_->GetQueenAttack(from);
            break;
          default:
            throw SayuriError("駒の種類が不正です。");
            break;
        }

        // 展開するタイプによって候補手を選り分ける。。
        if (Type == GenMoveType::NON_CAPTURE) {
          // キャプチャーじゃない手。
          move_bitboard &= ~(engine_ptr_->blocker_0());
        } else {
          // キャプチャーの手。
          move_bitboard &= engine_ptr_->side_pieces()[enemy_side];
        }

        for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
          // 手を作る。
          Move move = 0U;
          move_from(move, from);
          move_to(move, Util::GetSquare(move_bitboard));
          move_move_type(move, NORMAL);

          // スタックに登録。
          if (last_ < end_) {
            last_->move_ = move;
            ScoreMove<Type>(last_, prev_best, iid_move, killer_1, killer_2,
            side);
            num_moves++;
            last_++;
          }
        }
      }
    }

    // ポーンの動きを作る。
    Bitboard pieces = engine_ptr_->position()[side][PAWN];
    for (; pieces; pieces &= pieces - 1) {
      Square from = Util::GetSquare(pieces);

      Bitboard move_bitboard = 0ULL;
      if (Type == GenMoveType::NON_CAPTURE) {
        // キャプチャーじゃない手。
        // ポーンの一歩の動き。
        move_bitboard = Util::GetPawnMove(from, side)
        & ~(engine_ptr_->blocker_0());
        if (move_bitboard) {
          if (((side == WHITE) && (Util::GetRank(from) == RANK_2))
          || ((side == BLACK) && (Util::GetRank(from) == RANK_7))) {
            // ポーンの2歩の動き。
            move_bitboard |= Util::GetPawn2StepMove(from, side)
            & ~(engine_ptr_->blocker_0());
          }
        }
      } else {
        // キャプチャーの手。
        move_bitboard = Util::GetPawnAttack(from, side)
        & engine_ptr_->side_pieces()[enemy_side];
        // アンパッサンがある場合。
        if (engine_ptr_->en_passant_square()) {
          move_bitboard |= Util::SQUARE[engine_ptr_->en_passant_square()]
          & Util::GetPawnAttack(from, side);
        }
      } 

      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        // 手を作る。
        Move move = 0U;
        move_from(move, from);
        Square to = Util::GetSquare(move_bitboard);
        move_to(move, to);
        if (engine_ptr_->en_passant_square()
        && (to == engine_ptr_->en_passant_square())) {
          move_move_type(move, EN_PASSANT);
        } else {
          move_move_type(move, NORMAL);
        }

        if (((side == WHITE) && (Util::GetRank(to) == RANK_8))
        || ((side == BLACK) && (Util::GetRank(to) == RANK_1))) {
          // 昇格を設定。
          for (Piece piece_type = KNIGHT; piece_type <= QUEEN; piece_type++) {
            move_promotion(move, piece_type);
            if (last_ < end_) {
              last_->move_ = move;
              ScoreMove<Type>(last_, prev_best, iid_move, killer_1, killer_2,
              side);
              num_moves++;
              last_++;
            }
          }
        } else {
          // 昇格しない場合。
          if (last_ < end_) {
            last_->move_ = move;
            ScoreMove<Type>(last_, prev_best, iid_move, killer_1, killer_2,
            side);
            num_moves++;
            last_++;
          }
        }
      }
    }

    // キングの動きを作る。
    Square from = engine_ptr_->king()[side];
    Bitboard move_bitboard = Util::GetKingMove(from);
    if (Type == GenMoveType::NON_CAPTURE) {
      // キャプチャーじゃない手。
      move_bitboard &= ~(engine_ptr_->blocker_0());
      // キャスリングの動きを追加。
      if (side == WHITE) {
        if (engine_ptr_->CanCastling<WHITE_SHORT_CASTLING>()) {
          move_bitboard |= Util::SQUARE[G1];
        }
        if (engine_ptr_->CanCastling<WHITE_LONG_CASTLING>()) {
          move_bitboard |= Util::SQUARE[C1];
        }
      } else {
        if (engine_ptr_->CanCastling<BLACK_SHORT_CASTLING>()) {
          move_bitboard |= Util::SQUARE[G8];
        }
        if (engine_ptr_->CanCastling<BLACK_LONG_CASTLING>()) {
          move_bitboard |= Util::SQUARE[C8];
        }
      }
    } else {
      // キャプチャーの手。
      move_bitboard &= engine_ptr_->side_pieces()[enemy_side];
    }
    for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
      Move move = 0U;
      move_from(move, from);
      Square to = Util::GetSquare(move_bitboard);
      move_to(move, to);

      // キャスリングを設定。
      if (((side == WHITE) && (from == E1)
      && ((to == G1) || (to == C1)))
      || ((side == BLACK) && (from == E8)
      && ((to == G8) || (to == C8)))) {
        move_move_type(move, CASTLING);
      } else {
        move_move_type(move, NORMAL);
      }

      if (last_ < end_) {
        last_->move_ = move;
        ScoreMove<Type>(last_, prev_best, iid_move, killer_1, killer_2, side);
        num_moves++;
        last_++;
      }
    }

    return num_moves;
  }
  // 実体化。
  template int MoveMaker::GenMoves<GenMoveType::NON_CAPTURE>
  (Move prev_best, Move iid_move, Move killer_1, Move killer_2);
  template int MoveMaker::GenMoves<GenMoveType::CAPTURE>
  (Move prev_best, Move iid_move, Move killer_1, Move killer_2);
  template<>
  int MoveMaker::GenMoves<GenMoveType::ALL>
  (Move prev_best, Move iid_move, Move killer_1, Move killer_2) {
    int num_moves = GenMoves<GenMoveType::NON_CAPTURE>
    (prev_best, iid_move, killer_1, killer_2);

    num_moves += GenMoves<GenMoveType::CAPTURE>
    (prev_best, iid_move, killer_1, killer_2);

    return num_moves;
  }

  // 次の手を返す。
  Move MoveMaker::PickMove() {
    std::unique_lock<std::mutex> lock(mutex_);

    MoveSlot slot;

    // 手がなければ何もしない。
    if (last_ <= begin_) {
      return 0U;
    }

    // とりあえず最後の手をポップ。
    last_--;
    slot = *last_;

    // 一番高い手を探し、スワップ。
    for (MoveSlot* ptr = begin_; ptr < last_; ptr++) {
      if (ptr->score_ > slot.score_) {
        std::swap(slot, *ptr);
      }
    }

    return slot.move_;
  }

  // スタックに残っている候補手の数を返す。
  int MoveMaker::CountMoves() const {
    int count = 0;
    for (MoveSlot* ptr = begin_; ptr < last_; ptr++) {
      count++;
    }
    return count;
  }

  // 手に点数をつける。
  template<GenMoveType Type>
  void MoveMaker::ScoreMove(MoveMaker::MoveSlot* ptr,
  Move prev_best, Move iid_move, Move killer_1, Move killer_2, Side side) {
    // 評価値の定義。
    // 前回の繰り返しでトランスポジションテーブルから得た最善手の点数。
    constexpr int BEST_MOVE_SCORE = MAX_VALUE;
    // IIDで得た最善手の点数。
    constexpr int IID_MOVE_SCORE = BEST_MOVE_SCORE - 1;
    // 駒を取る手の下限値。20 * 20を目安に設定。
    constexpr int MIN_CAPTURE_SCORE = 403;
    // キラームーブの点数。
    constexpr int KILLER_1_MOVE_SCORE = MIN_CAPTURE_SCORE - 1;
    constexpr int KILLER_2_MOVE_SCORE = KILLER_1_MOVE_SCORE - 1;
    // ヒストリーの点数の最大値。
    constexpr std::uint64_t MAX_HISTORY_SCORE = KILLER_2_MOVE_SCORE - 1;
    // 悪い取る手の点数。
    constexpr int BAD_CAPTURE_SCORE = -1;

    // 特殊な手の点数をつける。
    if (EqualMove(ptr->move_, prev_best)) {
      // 前回の最善手。
      ptr->score_ = BEST_MOVE_SCORE;
    } else if (EqualMove(ptr->move_, iid_move)) {
      // IIDムーブ。
      ptr->score_ = IID_MOVE_SCORE;
    } else if (EqualMove(ptr->move_, killer_1)) {
      // キラームーブ。
      ptr->score_ = KILLER_1_MOVE_SCORE;
    } else if (EqualMove(ptr->move_, killer_2)) {
      // キラームーブ。
      ptr->score_ = KILLER_2_MOVE_SCORE;
    } else {
      // その他の手を各候補手のタイプに分ける。
      if ((Type == GenMoveType::CAPTURE) || (ptr->move_ & PROMOTION_MASK)) {
        // SEEで点数をつけていく。
        // 現在チェックされていれば、<取る駒> - <自分の駒>。
        if (!(engine_ptr_->IsAttacked
        (engine_ptr_->king()[side], side ^ 0x3))) {
          ptr->score_ = engine_ptr_->SEE(ptr->move_);
        } else {
          ptr->score_ =
          MATERIAL[engine_ptr_->piece_board()[move_to(ptr->move_)]]
          - MATERIAL[engine_ptr_->piece_board()[move_from(ptr->move_)]];
          // 昇格の得点を加算。
          if (Piece promotion = move_promotion(ptr->move_)) {
            ptr->score_ += MATERIAL[promotion] - MATERIAL[PAWN];
          }
        }
        ptr->score_ = ptr->score_ >= 0 ? ptr->score_ + MIN_CAPTURE_SCORE
        : BAD_CAPTURE_SCORE;
      } else {
        // ヒストリーを使って点数をつけていく。
        ptr->score_ = (engine_ptr_->history()
        [side][move_from(ptr->move_)][move_to(ptr->move_)]
        * MAX_HISTORY_SCORE) / engine_ptr_->history_max();
      }
    }
  }
  // 実体化。
  template void MoveMaker::ScoreMove<GenMoveType::NON_CAPTURE>
  (MoveMaker::MoveSlot* ptr, Move best_move, Move iid_move,
  Move killer_1, Move killer_2, Side side);
  template void MoveMaker::ScoreMove<GenMoveType::CAPTURE>
  (MoveMaker::MoveSlot* ptr, Move best_move, Move iid_move,
  Move killer_1, Move killer_2, Side side);
}  // namespace Sayuri
