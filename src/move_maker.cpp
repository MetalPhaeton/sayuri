/* 
   move_maker.cpp: 候補手を展開するクラスの実装。

   The MIT License (MIT)

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

#include "move_maker.h"

#include <iostream>
#include <cstddef>
#include "chess_def.h"
#include "chess_engine.h"

namespace Sayuri {
  /****************/
  /* static定数。 */
  /****************/
  constexpr int MoveMaker::MAX_SLOTS;

  /********************/
  /* コンストラクタ。 */
  /********************/
  //コンストラクタ。
  MoveMaker::MoveMaker(ChessEngine* engine_ptr) :
  engine_ptr_(engine_ptr) {
    // スタックのポインターをセット。
    begin_ = last_ = current_ = move_stack_;
    end_ = &(move_stack_[MAX_SLOTS]);
  }

  // コピーコンストラクタ。
  MoveMaker::MoveMaker(const MoveMaker& maker) :
  engine_ptr_(maker.engine_ptr_) {
    for (int i; i <= MAX_SLOTS; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.begin_ == &(maker.move_stack_[i])) {
        begin_ = &(move_stack_[i]);
      }
      if (maker.last_ == &(maker.move_stack_[i])) {
        last_ = &(move_stack_[i]);
      }
      if (maker.current_ == &(maker.move_stack_[i])) {
        current_ = &(move_stack_[i]);
      }
    }
  }

  // ムーブコンストラクタ。
  MoveMaker::MoveMaker(MoveMaker&& maker) :
  engine_ptr_(maker.engine_ptr_) {
    for (int i; i <= MAX_SLOTS; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.begin_ == &(maker.move_stack_[i])) {
        begin_ = &(move_stack_[i]);
      }
      if (maker.last_ == &(maker.move_stack_[i])) {
        last_ = &(move_stack_[i]);
      }
      if (maker.current_ == &(maker.move_stack_[i])) {
        current_ = &(move_stack_[i]);
      }
    }
  }

  // コピー代入。
  MoveMaker& MoveMaker::operator=
  (const MoveMaker& maker) {
    engine_ptr_ = maker.engine_ptr_;
    for (int i; i <= MAX_SLOTS; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.begin_ == &(maker.move_stack_[i])) {
        begin_ = &(move_stack_[i]);
      }
      if (maker.last_ == &(maker.move_stack_[i])) {
        last_ = &(move_stack_[i]);
      }
      if (maker.current_ == &(maker.move_stack_[i])) {
        current_ = &(move_stack_[i]);
      }
    }
    return *this;
  }

  // ムーブ代入。
  MoveMaker& MoveMaker::operator=
  (MoveMaker&& maker) {
    engine_ptr_ = maker.engine_ptr_;
    for (int i; i <= MAX_SLOTS; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.begin_ == &(maker.move_stack_[i])) {
        begin_ = &(move_stack_[i]);
      }
      if (maker.last_ == &(maker.move_stack_[i])) {
        last_ = &(move_stack_[i]);
      }
      if (maker.current_ == &(maker.move_stack_[i])) {
        current_ = &(move_stack_[i]);
      }
    }
    return *this;
  }

  /******************/
  /* その他の関数。 */
  /******************/

  // 手をスタックに展開する。
  template<GenMoveType GType>
  void MoveMaker::GenMoves(int depth, int level,
  const TranspositionTable& table) {
    // スタックのポインタを設定。
    MoveSlot* begin = last_;
    MoveSlot* end = last_;

    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 変数。
    Square from;  // 基点。
    Move move;  // 候補手。

    // ナイト、ビショップ、ルーク、クイーンの候補手を作る。
    Bitboard pieces = 0ULL;
    Bitboard move_bitboard = 0ULL;
    for (int piece_type = KNIGHT; piece_type <= QUEEN; piece_type++) {
      pieces = engine_ptr_->position_[side][piece_type];

      for (; pieces; pieces &= pieces - 1) {
        from = Util::GetSquare(pieces);

        // 各ピースの動き。
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
            Assert(false);
            break;
        }

        // 展開するタイプによって候補手を選り分ける。。
        if (GType == GenMoveType::NON_CAPTURE) {
          move_bitboard &= ~(engine_ptr_->blocker0_);
        } else if (GType == GenMoveType::CAPTURE) {
          move_bitboard &= engine_ptr_->side_pieces_[enemy_side];
        } else {
          move_bitboard &= ~(engine_ptr_->side_pieces_[side]);
        }

        for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
          move.all_ = 0;
          // 手を作る。
          move.from_ = from;
          move.to_ = Util::GetSquare(move_bitboard);
          move.move_type_ = NORMAL;

          // タイプが合法手の場合、
          // その手を指した後、自分のキングが攻撃されているか調べる。
          // 攻撃されていれば違法。
          if (GType == GenMoveType::LEGAL) {
            engine_ptr_->MakeMove(move);
            if (engine_ptr_->IsAttacked(engine_ptr_->king_[side],
            enemy_side)) {
              engine_ptr_->UnmakeMove(move);
              continue;
            }
            engine_ptr_->UnmakeMove(move);
          }

          // スタックに登録。
          last_->move_ = move;
          last_++;
          end = last_;
        }
      }
    }

    // ポーンの動きを作る。
    pieces = engine_ptr_->position_[side][PAWN];
    move_bitboard = 0ULL;
    for (; pieces; pieces &= pieces - 1) {
      from = Util::GetSquare(pieces);

      if (GType == GenMoveType::NON_CAPTURE) {
        // ポーンの一歩の動き。
        move_bitboard = Util::GetPawnMove(from, side)
        & ~(engine_ptr_->blocker0_);
        if (move_bitboard) {
          if (((side == WHITE) && (Util::GetRank(from) == RANK_2))
          || ((side == BLACK) && (Util::GetRank(from) == RANK_7))) {
            // ポーンの2歩の動き。
            move_bitboard |= Util::GetPawn2StepMove(from, side)
            & ~(engine_ptr_->blocker0_);
          }
        }
      } else if (GType == GenMoveType::CAPTURE) {
        move_bitboard = Util::GetPawnAttack(from, side)
        & engine_ptr_->side_pieces_[enemy_side];
        // アンパッサンがある場合。
        if (engine_ptr_->can_en_passant_) {
          move_bitboard |= Util::BIT[engine_ptr_->en_passant_square_]
          & Util::GetPawnAttack(from, side);
        }
      } else { 
        // ポーンの一歩の動き。
        move_bitboard = Util::GetPawnMove(from, side)
        & ~(engine_ptr_->blocker0_);
        if (move_bitboard) {
          if (((side == WHITE) && (Util::GetRank(from) == RANK_2))
          || ((side == BLACK) && (Util::GetRank(from) == RANK_7))) {
            // ポーンの2歩の動き。
            move_bitboard |= Util::GetPawn2StepMove(from, side)
            & ~(engine_ptr_->blocker0_);
          }
        }
        // 駒を取る動き。
        move_bitboard |= Util::GetPawnAttack(from, side)
        & engine_ptr_->side_pieces_[enemy_side];
        if (engine_ptr_->can_en_passant_) {
          move_bitboard |= Util::BIT[engine_ptr_->en_passant_square_]
          & Util::GetPawnAttack(from, side);
        }
      }

      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        // 手を作る。
        move.from_ = from;
        move.to_ = Util::GetSquare(move_bitboard);
        if (engine_ptr_->can_en_passant_
        && (move.to_ == engine_ptr_->en_passant_square_)) {
          move.move_type_ = EN_PASSANT;
        } else {
          move.move_type_ = NORMAL;
        }

        // タイプが合法手の場合、
        // その手を指した後、自分のキングが攻撃されているか調べる。
        // 攻撃されていれば違法。
        if (GType == GenMoveType::LEGAL) {
          engine_ptr_->MakeMove(move);
          if (engine_ptr_->IsAttacked(engine_ptr_->king_[side],
          enemy_side)) {
            engine_ptr_->UnmakeMove(move);
            continue;
          }
          engine_ptr_->UnmakeMove(move);
        }

        if (((side == WHITE) && (Util::GetRank(move.to_) == RANK_8))
        || ((side == BLACK) && (Util::GetRank(move.to_) == RANK_1))) {
          // 昇格を設定。
          for (Piece piece_type = KNIGHT; piece_type <= QUEEN; piece_type++) {
            move.promotion_ = piece_type;
            last_->move_ = move;
            last_++;
            end = last_;
          }
        } else {
          // 昇格しない場合。
          last_->move_ = move;
          last_++;
          end = last_;
        }
      }
    }

    // キングの動きを作る。
    from = engine_ptr_->king_[side];
    move_bitboard = Util::GetKingMove(from);
    if (GType == GenMoveType::NON_CAPTURE) {
      move_bitboard &= ~(engine_ptr_->blocker0_);
      // キャスリングの動きを追加。
      if (side == WHITE) {
        if (engine_ptr_->CanCastling<WHITE_SHORT_CASTLING>()) {
          move_bitboard |= Util::BIT[G1];
        }
        if (engine_ptr_->CanCastling<WHITE_LONG_CASTLING>()) {
          move_bitboard |= Util::BIT[C1];
        }
      } else {
        if (engine_ptr_->CanCastling<BLACK_SHORT_CASTLING>()) {
          move_bitboard |= Util::BIT[G8];
        }
        if (engine_ptr_->CanCastling<BLACK_LONG_CASTLING>()) {
          move_bitboard |= Util::BIT[C8];
        }
      }
    } else if (GType == GenMoveType::CAPTURE) {
      move_bitboard &= engine_ptr_->side_pieces_[enemy_side];
    } else {
      move_bitboard &= ~(engine_ptr_->side_pieces_[side]);
      // キャスリングの動きを追加。
      if (side == WHITE) {
        if (engine_ptr_->CanCastling<WHITE_SHORT_CASTLING>()) {
          move_bitboard |= Util::BIT[G1];
        }
        if (engine_ptr_->CanCastling<WHITE_LONG_CASTLING>()) {
          move_bitboard |= Util::BIT[C1];
        }
      } else {
        if (engine_ptr_->CanCastling<BLACK_SHORT_CASTLING>()) {
          move_bitboard |= Util::BIT[G8];
        }
        if (engine_ptr_->CanCastling<BLACK_LONG_CASTLING>()) {
          move_bitboard |= Util::BIT[C8];
        }
      }
    }
    for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
      move.all_ = 0;
      move.from_ = from;
      move.to_ = Util::GetSquare(move_bitboard);

      // キャスリングを設定。
      if (((side == WHITE) && (from == E1)
      && ((move.to_ == G1) || (move.to_ == C1)))
      || ((side == BLACK) && (from == E8)
      && ((move.to_ == G8) || (move.to_ == C8)))) {
        move.move_type_ = CASTLING;
      } else {
        move.move_type_ = NORMAL;
      }

      // タイプが合法手の場合、
      // その手を指した後、自分のキングが攻撃されているか調べる。
      // 攻撃されていれば違法。
      if (GType == GenMoveType::LEGAL) {
        engine_ptr_->MakeMove(move);
        if (engine_ptr_->IsAttacked(engine_ptr_->king_[side], enemy_side)) {
          engine_ptr_->UnmakeMove(move);
          continue;
        }
        engine_ptr_->UnmakeMove(move);
      }

      last_->move_ = move;
      last_++;
      end = last_;
    }

    // 得点をつける。
    ScoreMoves<GType>(begin, end, depth, level, table);
  }
  // 実体化。
  template void MoveMaker::GenMoves
  <GenMoveType::NON_CAPTURE>(int depth, int level,
  const TranspositionTable& table);
  template void MoveMaker::GenMoves
  <GenMoveType::CAPTURE>(int depth, int level,
  const TranspositionTable& table);
  template void MoveMaker::GenMoves
  <GenMoveType::LEGAL>(int depth, int level,
  const TranspositionTable& table);

  // 展開した手の数を返す。
  std::size_t MoveMaker::GetSize() const {
    std::size_t size = 0;
    for (MoveSlot* ptr = begin_; ptr < last_; ptr++) {
      size++;
    }
    return size;
  }

  // 次の手を返す。
  Move MoveMaker::PickMove() {
    // なければ無意味な手を返す。
    if (begin_ == last_) {
      Move move;
      move.all_ = 0;
      return move;
    }
    // とりあえず最後の手を取り出す。
    MoveSlot slot = *last_;

    // 一番高い手を探し、スワップ。
    MoveSlot temp;
    for (MoveSlot* ptr = begin_; ptr < last_; ptr++) {
      if (ptr->score_ > slot.score_) {
        temp = *ptr;
        *ptr = slot;
        slot = temp;
      }
    }

    return slot.move_;
  }

  // 展開した候補手に得点をつける。
  template<GenMoveType GType>
  void MoveMaker::ScoreMoves
  (MoveMaker::MoveSlot* begin,
   MoveMaker::MoveSlot* end,
   int depth, int level, const TranspositionTable& table) {
    if (begin == end) return;

    // 評価値の定義。
    // 前回の繰り返しでトランスポジションテーブルに記録された最善手の点数。
    constexpr int TABLE_MOVE_SCORE = INFINITE;
    // IIDで得た最善手の点数。
    constexpr int IID_MOVE_SCORE = TABLE_MOVE_SCORE - 1;
    // キラームーブの点数。
    constexpr int KILLER_MOVE_SCORE = IID_MOVE_SCORE - 1;

    // トランスポジションテーブルから前回の繰り返しの最善手を得る。
    const TTEntry* entry_ptr =
    table.GetFulfiledEntry(engine_ptr_->search_stack_[level].current_pos_key_,
    depth - 1, level, engine_ptr_->to_move_);
    Move prev_best;
    if (entry_ptr && entry_ptr->value_flag() == TTValueFlag::EXACT) {
      prev_best = entry_ptr->best_move();
    } else {
      prev_best.all_ = 0;
    }

    // IIDムーブを得る。
    Move iid_move;
    iid_move = engine_ptr_->search_stack_[level].iid_move_;

    // キラームーブを得る。
    Move killer;
    killer = engine_ptr_->search_stack_[level].killer_;

    for (MoveSlot* ptr = begin; ptr < end; ptr++) {
      // 各候補手のタイプに分ける。
      if (GType == GenMoveType::NON_CAPTURE) {
        // ヒストリーを使って点数をつけていく。
        ptr->score_ = engine_ptr_->history_[ptr->move_.from_][ptr->move_.to_];
      } else if (GType == GenMoveType::CAPTURE) {
        Side side = engine_ptr_->to_move_;
        // SEEで点数をつけていく。
        // 現在チェックされていれば、<取る駒> - <自分の駒>。
        if (!(engine_ptr_->IsAttacked(engine_ptr_->king_[side], side ^ 0x3))) {
          ptr->score_ = SEE(ptr->move_, side);
        } else {
          ptr->score_ = MATERIAL[engine_ptr_->piece_board_[ptr->move_.to_]]
          - MATERIAL[engine_ptr_->piece_board_[ptr->move_.from_]];
        }
      } else {
        // 合法手は0点。
        ptr->score_ = 0;
      }

      // 特殊な手の点数をつける。
      if (GType != GenMoveType::LEGAL) {
        if ((ptr->move_.to_ == prev_best.to_)
        && (ptr->move_.from_ == prev_best.from_)) {
          // 前回の最善手。
          ptr->score_ = TABLE_MOVE_SCORE;
        } else if ((ptr->move_.to_ == iid_move.to_)
        && (ptr->move_.from_ == iid_move.from_)) {
          // IIDムーブ。
          ptr->score_ = IID_MOVE_SCORE;
        } else if ((ptr->move_.to_ == killer.to_)
        && (ptr->move_.from_ == killer.from_)){
          // キラームーブ。
          ptr->score_ = KILLER_MOVE_SCORE;
        }
      }
    }
  }
  // 実体化。
  template void MoveMaker::ScoreMoves <GenMoveType::NON_CAPTURE>
  (MoveMaker::MoveSlot* begin, MoveMaker::MoveSlot* end,
   int depth, int level, const TranspositionTable& table);
  template void MoveMaker::ScoreMoves <GenMoveType::CAPTURE>
  (MoveMaker::MoveSlot* begin, MoveMaker::MoveSlot* end,
   int depth, int level, const TranspositionTable& table);
  template void MoveMaker::ScoreMoves <GenMoveType::LEGAL>
  (MoveMaker::MoveSlot* begin, MoveMaker::MoveSlot* end,
   int depth, int level, const TranspositionTable& table);

  // SEE。
  int MoveMaker::SEE(Move move, Side side) {
    int value = 0;

    if (move.all_) {
      // 取る駒の価値を得る。
      int capture_value = MATERIAL[engine_ptr_->piece_board_[move.to_]];

      engine_ptr_->MakeMove(move);

      // 違法な手なら計算しない。
      if (!(engine_ptr_->IsAttacked(engine_ptr_->king_[side], side ^ 0x3))) {
        // 再帰して次の局面の評価値を得る。
        Move next_move = GetSmallestAttackerMove(move.to_, side ^ 0x3);
        value = capture_value - SEE(next_move, side ^ 0x3);
      }

      engine_ptr_->UnmakeMove(move);
    }

    return value;
  }

  // 最小の攻撃駒の攻撃の手を得る。
  Move MoveMaker::GetSmallestAttackerMove(Square target,
  Side side) const {
    // 変数。
    Bitboard attack;
    Move move;

    // キングがターゲットの時はなし。
    if (target == engine_ptr_->king_[side]) {
      move.all_ = 0;
      return move;
    }

    // 価値の低いものから調べる。
    for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
      switch (piece_type) {
        case PAWN:
          attack = Util::GetPawnAttack(target, side ^ 0x3);
          attack &= engine_ptr_->position_[side][PAWN];
          break;
        case KNIGHT:
          attack = Util::GetKnightMove(target);
          attack &= engine_ptr_->position_[side][KNIGHT];
          break;
        case BISHOP:
          attack = engine_ptr_->GetBishopAttack(target);
          attack &= engine_ptr_->position_[side][BISHOP];
          break;
        case ROOK:
          attack = engine_ptr_->GetRookAttack(target);
          attack &= engine_ptr_->position_[side][ROOK];
          break;
        case QUEEN:
          attack = engine_ptr_->GetQueenAttack(target);
          attack &= engine_ptr_->position_[side][QUEEN];
          break;
        case KING:
          attack = Util::GetKingMove(target);
          attack &= engine_ptr_->position_[side][KING];
          break;
        default:
          Assert(false);
          break;
      }
      if (attack) {
        move.all_ = 0;
        move.from_ = Util::GetSquare(attack);
        move.to_  = target;
        move.move_type_ = NORMAL;
        return move;
      }
    }

    move.all_ = 0;
    return move;
  }
}  // namespace Sayuri
