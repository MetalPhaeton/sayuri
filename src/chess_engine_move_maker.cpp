/* 
   chess_engine_move_maker.cpp: 手を作るクラスの実装。

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

#include "chess_engine.h"

#include <iostream>

namespace Sayuri {
  /****************/
  /* static定数。 */
  /****************/
  constexpr int ChessEngine::MoveMaker::MAX_SLOTS;

  /********************/
  /* コンストラクタ。 */
  /********************/
  ChessEngine::MoveMaker::MoveMaker(ChessEngine* engine_ptr) :
  engine_ptr_(engine_ptr) {
    // スタックのポインターをセット。
    first_ = last_ = current_ = move_stack_;
    end_ = &(move_stack_[MAX_SLOTS]);
  }
  // コピーコンストラクタ。
  ChessEngine::MoveMaker::MoveMaker(const MoveMaker& maker) :
  engine_ptr_(maker.engine_ptr_) {
    for (int i; i <= MAX_SLOTS; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.first_ == &(maker.move_stack_[i])) {
        first_ = &(move_stack_[i]);
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
  ChessEngine::MoveMaker::MoveMaker(MoveMaker&& maker) :
  engine_ptr_(maker.engine_ptr_) {
    for (int i; i <= MAX_SLOTS; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.first_ == &(maker.move_stack_[i])) {
        first_ = &(move_stack_[i]);
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
  ChessEngine::MoveMaker& ChessEngine::MoveMaker::operator=
  (const MoveMaker& maker) {
    engine_ptr_ = maker.engine_ptr_;
    for (int i; i <= MAX_SLOTS; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.first_ == &(maker.move_stack_[i])) {
        first_ = &(move_stack_[i]);
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
  ChessEngine::MoveMaker& ChessEngine::MoveMaker::operator=
  (MoveMaker&& maker) {
    engine_ptr_ = maker.engine_ptr_;
    for (int i; i <= MAX_SLOTS; i++) {
      move_stack_[i] = maker.move_stack_[i];
      if (maker.first_ == &(maker.move_stack_[i])) {
        first_ = &(move_stack_[i]);
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
  // キャスリングできるかどうか。
  template<Castling Which>
  bool ChessEngine::MoveMaker::CanCastling() const {
    if (!(engine_ptr_->castling_rights_ & Which)) return false;

    if (Which == WHITE_SHORT_CASTLING) {
      if (engine_ptr_->IsAttacked(E1, BLACK)) return false;
      if (engine_ptr_->IsAttacked(F1, BLACK)) return false;
      if (engine_ptr_->IsAttacked(G1, BLACK)) return false;
      if (engine_ptr_->piece_board_[F1]) return false;
      if (engine_ptr_->piece_board_[G1]) return false;
    } else if (Which == WHITE_LONG_CASTLING) {
      if (engine_ptr_->IsAttacked(E1, BLACK)) return false;
      if (engine_ptr_->IsAttacked(D1, BLACK)) return false;
      if (engine_ptr_->IsAttacked(C1, BLACK)) return false;
      if (engine_ptr_->piece_board_[D1]) return false;
      if (engine_ptr_->piece_board_[C1]) return false;
      if (engine_ptr_->piece_board_[B1]) return false;
    } else if (Which == BLACK_SHORT_CASTLING) {
      if (engine_ptr_->IsAttacked(E8, WHITE)) return false;
      if (engine_ptr_->IsAttacked(F8, WHITE)) return false;
      if (engine_ptr_->IsAttacked(G8, WHITE)) return false;
      if (engine_ptr_->piece_board_[F8]) return false;
      if (engine_ptr_->piece_board_[G8]) return false;
    } else if (Which == BLACK_LONG_CASTLING){
      if (engine_ptr_->IsAttacked(E8, WHITE)) return false;
      if (engine_ptr_->IsAttacked(D8, WHITE)) return false;
      if (engine_ptr_->IsAttacked(C8, WHITE)) return false;
      if (engine_ptr_->piece_board_[D8]) return false;
      if (engine_ptr_->piece_board_[C8]) return false;
      if (engine_ptr_->piece_board_[B8]) return false;
    } else {
      Assert(false);
    }

    return true;
  }
  // 実体化。
  template bool ChessEngine::MoveMaker::CanCastling
  <WHITE_SHORT_CASTLING>() const;
  template bool ChessEngine::MoveMaker::CanCastling
  <WHITE_LONG_CASTLING>() const;
  template bool ChessEngine::MoveMaker::CanCastling
  <BLACK_SHORT_CASTLING>() const;
  template bool ChessEngine::MoveMaker::CanCastling
  <BLACK_LONG_CASTLING>() const;

  // 手をスタックに展開する。
  template<ChessEngine::GenMoveType Type>
  void ChessEngine::MoveMaker::GenMoves() {
    // スタックのポインタを設定。
    // MoveSlot* start = last_;
    // MoveSlot* end = last_;

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
        if (Type == GenMoveType::NON_CAPTURE) {
          move_bitboard &= ~(engine_ptr_->blocker0_);
        } else if (Type == GenMoveType::CAPTURE) {
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
          if (Type == GenMoveType::LEGAL) {
            engine_ptr_->MakeMove(move);
            if (engine_ptr_->IsAttacked(engine_ptr_->king_[side],
            enemy_side)) {
              engine_ptr_->UnmakeMove(move);
              continue;
            }
            engine_ptr_->UnmakeMove(move);
          }

          // スタックに登録。
          last_->move_.all_ = move.all_;
          last_++;
          // end = last_;
        }
      }
    }

    // ポーンの動きを作る。
    pieces = engine_ptr_->position_[side][PAWN];
    move_bitboard = 0ULL;
    for (; pieces; pieces &= pieces - 1) {
      from = Util::GetSquare(pieces);

      if (Type == GenMoveType::NON_CAPTURE) {  // ノンキャプチャームーブ。
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
      } else if (Type == GenMoveType::CAPTURE) {  // キャプチャームーブ。
        move_bitboard = Util::GetPawnAttack(from, side)
        & engine_ptr_->side_pieces_[enemy_side];
        // アンパッサンがある場合。
        if (engine_ptr_->can_en_passant_) {
          move_bitboard |= Util::BIT[engine_ptr_->en_passant_square_]
          & Util::GetPawnAttack(from, side);
        }
      } else {  // 合法手。
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
        & engine_ptr_->blocker0_;
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
        if (Type == GenMoveType::LEGAL) {
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
            last_->move_.all_ = move.all_;
            last_++;
            // end = last_;
          }
        } else {
          // 昇格しない場合。
          last_->move_.all_ = move.all_;
          last_++;
          // end = last_;
        }
      }
    }

    // キングの動きを作る。
    from = engine_ptr_->king_[side];
    move_bitboard = Util::GetKingMove(from);
    if (Type == GenMoveType::NON_CAPTURE) {
      move_bitboard &= ~(engine_ptr_->blocker0_);
      // キャスリングの動きを追加。
      if (side == WHITE) {
        if (CanCastling<WHITE_SHORT_CASTLING>()) {
          move_bitboard |= Util::BIT[G1];
        }
        if (CanCastling<WHITE_LONG_CASTLING>()) {
          move_bitboard |= Util::BIT[C1];
        }
      } else {
        if (CanCastling<BLACK_SHORT_CASTLING>()) {
          move_bitboard |= Util::BIT[G8];
        }
        if (CanCastling<BLACK_LONG_CASTLING>()) {
          move_bitboard |= Util::BIT[C8];
        }
      }
    } else if (Type == GenMoveType::CAPTURE) {
      move_bitboard &= engine_ptr_->side_pieces_[enemy_side];
    } else {
      move_bitboard &= ~(engine_ptr_->side_pieces_[side]);
      // キャスリングの動きを追加。
      if (side == WHITE) {
        if (CanCastling<WHITE_SHORT_CASTLING>()) {
          move_bitboard |= Util::BIT[G1];
        }
        if (CanCastling<WHITE_LONG_CASTLING>()) {
          move_bitboard |= Util::BIT[C1];
        }
      } else {
        if (CanCastling<BLACK_SHORT_CASTLING>()) {
          move_bitboard |= Util::BIT[G8];
        }
        if (CanCastling<BLACK_LONG_CASTLING>()) {
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
      if (Type == GenMoveType::LEGAL) {
        engine_ptr_->MakeMove(move);
        if (engine_ptr_->IsAttacked(engine_ptr_->king_[side], enemy_side)) {
          engine_ptr_->UnmakeMove(move);
          continue;
        }
        engine_ptr_->UnmakeMove(move);
      }

      last_->move_.all_ = move.all_;
      last_++;
      // end = last_;
    }

    // 得点をつける。
    // TODO: score<Type>(start, end);
  }
  // テンプレートの実体化。
  template void ChessEngine::MoveMaker::GenMoves
  <ChessEngine::GenMoveType::NON_CAPTURE>();
  template void ChessEngine::MoveMaker::GenMoves
  <ChessEngine::GenMoveType::CAPTURE>();
  template void ChessEngine::MoveMaker::GenMoves
  <ChessEngine::GenMoveType::LEGAL>();
  // SEE。
}  // namespace Sayuri
