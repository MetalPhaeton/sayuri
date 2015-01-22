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
 * @file move_maker.cpp
 * @author Hironori Ishibashi
 * @brief 候補手を生成するクラスの実装。
 */

#include "move_maker.h"

#include <iostream>
#include <mutex>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <cstring>
#include "common.h"
#include "chess_engine.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  //コンストラクタ。
  MoveMaker::MoveMaker(const ChessEngine& engine) :
  engine_ptr_(&engine),
  last_(0),
  max_(0),
  history_max_(1) {}

  // コピーコンストラクタ。
  MoveMaker::MoveMaker(const MoveMaker& maker) :
  engine_ptr_(maker.engine_ptr_),
  last_(maker.last_),
  max_(maker.max_),
  history_max_(maker.history_max_) {
    COPY_ARRAY(move_stack_, maker.move_stack_);
  }

  // ムーブコンストラクタ。
  MoveMaker::MoveMaker(MoveMaker&& maker) :
  engine_ptr_(maker.engine_ptr_),
  last_(maker.last_),
  max_(maker.max_),
  history_max_(maker.history_max_) {
    COPY_ARRAY(move_stack_, maker.move_stack_);
  }

  // コピー代入演算子。
  MoveMaker& MoveMaker::operator=
  (const MoveMaker& maker) {
    engine_ptr_ = maker.engine_ptr_;
    history_max_ = maker.history_max_;
    last_ = maker.last_;
    max_ = maker.max_;
    COPY_ARRAY(move_stack_, maker.move_stack_);
    return *this;
  }

  // ムーブ代入演算子。
  MoveMaker& MoveMaker::operator=
  (MoveMaker&& maker) {
    engine_ptr_ = maker.engine_ptr_;
    history_max_ = maker.history_max_;
    last_ = maker.last_;
    max_ = maker.max_;
    COPY_ARRAY(move_stack_, maker.move_stack_);
    return *this;
  }

  // ============== //
  // パブリック関数 //
  // ============== //
  // スタックに候補手を生成する。
  template<GenMoveType Type> int MoveMaker::GenMoves(Move prev_best,
  Move iid_move, Move killer_1, Move killer_2) {
    // 初期化。
    last_ = max_ = 0;
    history_max_ = 1;

    GenMovesCore<Type>(prev_best, iid_move, killer_1, killer_2);
    
    max_ = last_;
    return last_;
  }
  // インスタンス化。
  template int MoveMaker::GenMoves<GenMoveType::NON_CAPTURE>(Move prev_best,
  Move iid_move, Move killer_1, Move killer_2);
  template int MoveMaker::GenMoves<GenMoveType::CAPTURE>(Move prev_best,
  Move iid_move, Move killer_1, Move killer_2);
  template <>
  int MoveMaker::GenMoves<GenMoveType::ALL>(Move prev_best,
  Move iid_move, Move killer_1, Move killer_2) {
    // 初期化。
    last_ = max_ = 0;
    history_max_ = 1;

    GenMovesCore<GenMoveType::NON_CAPTURE>(prev_best, iid_move, killer_1,
    killer_2);

    GenMovesCore<GenMoveType::CAPTURE>(prev_best, iid_move, killer_1,
    killer_2);

    max_ = last_;
    return last_;
  }

  // 次の候補手を取り出す。
  Move MoveMaker::PickMove() {
    std::unique_lock<std::mutex> lock(mutex_);

    MoveSlot slot;

    // 手がなければ何もしない。
    if (!last_) {
      return 0;
    }

    // とりあえず最後の手をポップ。
    slot = move_stack_[--last_];

    // 一番高い手を探し、スワップ。
    for (int i = last_ - 1; i >= 0; --i) {
      if (move_stack_[i].score_ > slot.score_) {
        std::swap(slot, move_stack_[i]);
      }
    }

    return slot.move_;
  }

  // ヒストリーの最大値を更新する。 (GenMovesCore()用テンプレート部品)
  template<GenMoveType Type>
  struct UpdateMaxHistory {
    static void F(MoveMaker& maker, Side side, Square from, Square to) {}
  };
  template <>
  struct UpdateMaxHistory<GenMoveType::NON_CAPTURE> {
    static void F(MoveMaker& maker, Side side, Square from, Square to) {
      Util::UpdateMax(maker.history_max_,
      maker.engine_ptr_->history()[side][from][to]);
    }
  };

  // ポーン、キング以外の駒の動きのビットボードを作成する。
  // (GenMovesCore()用テンプレート部品)
  template<GenMoveType Type>
  struct GenPieceBitboard {
    static void F(MoveMaker& maker, Bitboard& bitboard, Side side) {}
  };
  template<>
  struct GenPieceBitboard<GenMoveType::NON_CAPTURE> {
    static void F(MoveMaker& maker, Bitboard& bitboard, Side side) {
      bitboard &= ~(maker.engine_ptr_->blocker_0());
    }
  };
  template<>
  struct GenPieceBitboard<GenMoveType::CAPTURE> {
    static void F(MoveMaker& maker, Bitboard& bitboard, Side side) {
      bitboard &=
      maker.engine_ptr_->side_pieces()[Util::GetOppositeSide(side)];
    }
  };

  // ポーンの動きのビットボードを作成する。 (GenMovesCore()用テンプレート部品)
  template<GenMoveType Type>
  struct GenPawnBitboard {
    static void F(MoveMaker& maker, Bitboard& bitboard, Side side,
    Square from) {}
  };
  template<>
  struct GenPawnBitboard<GenMoveType::NON_CAPTURE> {
    static void F(MoveMaker& maker, Bitboard& bitboard, Side side,
    Square from) {
      bitboard = maker.engine_ptr_->GetPawnStep(side, from);
    }
  };
  template<>
  struct GenPawnBitboard<GenMoveType::CAPTURE> {
    static void F(MoveMaker& maker, Bitboard& bitboard, Side side,
    Square from) {
      bitboard = Util::GetPawnAttack(side, from)
      & maker.engine_ptr_->side_pieces()[Util::GetOppositeSide(side)];

      // アンパッサンがある場合。
      if (maker.engine_ptr_->en_passant_square()) {
        bitboard |= Util::SQUARE[maker.engine_ptr_->en_passant_square()]
        & Util::GetPawnAttack(side, from);
      }
    }
  };

  // キングの動きのビットボードを作成する。 (GenMovesCore()用テンプレート部品)
  template<GenMoveType Type>
  struct GenKingBitboard {
    static void F(MoveMaker& maker, Bitboard& bitboard, Side side) {}
  };
  template<>
  struct GenKingBitboard<GenMoveType::NON_CAPTURE> {
    static void F(MoveMaker& maker, Bitboard& bitboard, Side side) {
      bitboard &= ~(maker.engine_ptr_->blocker_0());
    }
  };
  template<>
  struct GenKingBitboard<GenMoveType::CAPTURE> {
    static void F(MoveMaker& maker, Bitboard& bitboard, Side side) {
      bitboard &=
      maker.engine_ptr_->side_pieces()[Util::GetOppositeSide(side)];
    }
  };

  // スタックに候補手を生成する。 (内部用)
  template<GenMoveType Type>
  void MoveMaker::GenMovesCore(Move prev_best, Move iid_move,
  Move killer_1, Move killer_2) {
    // サイド。
    Side side = engine_ptr_->to_move();

    // 生成開始時のポインタ。
    std::size_t start = last_;

    // ナイト、ビショップ、ルーク、クイーンの候補手を作る。
    for (PieceType piece_type = KNIGHT; piece_type <= QUEEN; ++piece_type) {
      Bitboard pieces = engine_ptr_->position()[side][piece_type];

      for (; pieces; Util::SetNext(pieces)) {
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
            throw SayuriError
            ("駒の種類が不正です。 in MoveMaker::GenMoveCore()");
            break;
        }

        // 展開するタイプによって候補手を選り分ける。 (テンプレート部品)
        GenPieceBitboard<Type>::F(*this, move_bitboard, side);

        for (; move_bitboard; Util::SetNext(move_bitboard)) {
          // 手を作る。
          Move move = 0;
          SetFrom(move, from);
          Square to = Util::GetSquare(move_bitboard);
          SetTo(move, to);
          SetMoveType(move, NORMAL);

          // ヒストリーの最大値を更新。 (テンプレート部品)
          UpdateMaxHistory<Type>::F(*this, side, from, to);

          // スタックに登録。
          move_stack_[last_++].move_ = move;
        }
      }
    }

    // ポーンの動きを作る。
    Bitboard pieces = engine_ptr_->position()[side][PAWN];
    for (; pieces; Util::SetNext(pieces)) {
      Square from = Util::GetSquare(pieces);

      Bitboard move_bitboard = 0;

      // (テンプレート部品)
      GenPawnBitboard<Type>::F(*this, move_bitboard, side, from);

      for (; move_bitboard; Util::SetNext(move_bitboard)) {
        // 手を作る。
        Move move = 0;
        SetFrom(move, from);
        Square to = Util::GetSquare(move_bitboard);
        SetTo(move, to);

        // ヒストリーの最大値を更新。 (テンプレート部品)
        UpdateMaxHistory<Type>::F(*this, side, from, to);

        if (engine_ptr_->en_passant_square()
        && (to == engine_ptr_->en_passant_square())) {
          SetMoveType(move, EN_PASSANT);
        } else {
          SetMoveType(move, NORMAL);
        }

        if (((side == WHITE) && (Util::SQUARE_TO_RANK[to] == RANK_8))
        || ((side == BLACK) && (Util::SQUARE_TO_RANK[to] == RANK_1))) {
          // 昇格を設定。
          for (PieceType piece_type = KNIGHT;
          piece_type <= QUEEN; ++piece_type) {
            SetPromotion(move, piece_type);
            move_stack_[last_++].move_ = move;
          }
        } else {
          // 昇格しない場合。
          move_stack_[last_++].move_ = move;
        }
      }
    }

    // キングの動きを作る。
    Square from = engine_ptr_->king()[side];
    Bitboard move_bitboard = Util::GetKingMove(from);

    // キャスリングの動きを作る。
    constexpr Move WS_CASTLING_MOVE = E1 | (G1 << TO_SHIFT)
    | (CASTLE_WS << MOVE_TYPE_SHIFT);
    constexpr Move WL_CASTLING_MOVE = E1 | (C1 << TO_SHIFT)
    | (CASTLE_WL << MOVE_TYPE_SHIFT);
    constexpr Move BS_CASTLING_MOVE = E8 | (G8 << TO_SHIFT)
    | (CASTLE_BS << MOVE_TYPE_SHIFT);
    constexpr Move BL_CASTLING_MOVE = E8 | (C8 << TO_SHIFT)
    | (CASTLE_BL << MOVE_TYPE_SHIFT);
    if (side == WHITE) {
      if (engine_ptr_->CanCastling<WHITE_SHORT_CASTLING>()) {
        move_stack_[last_++].move_ = WS_CASTLING_MOVE;
      }
      if (engine_ptr_->CanCastling<WHITE_LONG_CASTLING>()) {
        move_stack_[last_++].move_ = WL_CASTLING_MOVE;
      }
    } else {
      if (engine_ptr_->CanCastling<BLACK_SHORT_CASTLING>()) {
        move_stack_[last_++].move_ = BS_CASTLING_MOVE;
      }
      if (engine_ptr_->CanCastling<BLACK_LONG_CASTLING>()) {
        move_stack_[last_++].move_ = BL_CASTLING_MOVE;
      }
    }

    // (テンプレート部品)
    GenKingBitboard<Type>::F(*this, move_bitboard, side);

    for (; move_bitboard; Util::SetNext(move_bitboard)) {
      Move move = 0;
      SetFrom(move, from);
      Square to = Util::GetSquare(move_bitboard);
      SetTo(move, to);
      SetMoveType(move, NORMAL);

      // ヒストリーの最大値を更新。 (テンプレート部品)
      UpdateMaxHistory<Type>::F(*this, side, from, to);

      move_stack_[last_++].move_ = move;
    }

    ScoreMoves<Type>(start, prev_best, iid_move, killer_1, killer_2, side);
  }

  // 実体化。
  template void MoveMaker::GenMovesCore<GenMoveType::NON_CAPTURE>
  (Move prev_best, Move iid_move, Move killer_1, Move killer_2);
  template void MoveMaker::GenMovesCore<GenMoveType::CAPTURE>
  (Move prev_best, Move iid_move, Move killer_1, Move killer_2);

  // 点数をセットする。 (ScoreMoves()用テンプレート部品)
  template<GenMoveType Type>
  struct SetScore {
    static void F(MoveMaker& maker, std::int64_t& score, Move move,
    Side side, Square from, Square to) {}
  };
  template<>
  struct SetScore<GenMoveType::NON_CAPTURE> {
    static void F(MoveMaker& maker, std::int64_t& score, Move move,
    Side side, Square from, Square to) {
      // ヒストリーの点数の最大値のビット桁数。 (つまり、最大値は 0x200 )
      constexpr int MAX_HISTORY_SCORE_SHIFT = 9;

      // ヒストリーを使って点数をつけていく。
      score = (maker.engine_ptr_->history()[side][from][to]
      << MAX_HISTORY_SCORE_SHIFT) / maker.history_max_;
    }
  };
  template<>
  struct SetScore<GenMoveType::CAPTURE> {
    static void F(MoveMaker& maker, std::int64_t& score, Move move,
    Side side, Square from, Square to) {
      // 駒を取る手のビットシフト。
      constexpr int CAPTURE_SCORE_SHIFT = 13;
      // 悪い取る手の点数。
      constexpr std::int64_t BAD_CAPTURE_SCORE = -1LL;

      // SEEで点数をつけていく。
      score =
      Util::GetMax(static_cast<std::int64_t>(maker.engine_ptr_->SEE(move))
      << CAPTURE_SCORE_SHIFT, BAD_CAPTURE_SCORE);
    }
  };

  // 候補手に点数をつける。
  template<GenMoveType Type>
  void MoveMaker::ScoreMoves(std::size_t start, Move prev_best, Move iid_move,
  Move killer_1, Move killer_2, Side side) {
    // --- 評価値の定義 --- //
    // キラームーブの点数。
    constexpr std::int64_t KILLER_2_MOVE_SCORE = 0x400LL;
    constexpr std::int64_t KILLER_1_MOVE_SCORE = KILLER_2_MOVE_SCORE << 1;
    // 駒を取る手のビットシフト。
    // constexpr int CAPTURE_SCORE_SHIFT = 13;
    // 相手キングをチェックする手の点数。
    constexpr std::int64_t CHECKING_MOVE_SCORE = 1LL << 32;
    // IIDで得た最善手の点数。
    constexpr std::int64_t IID_MOVE_SCORE = CHECKING_MOVE_SCORE << 1;
    // 前回の繰り返しでトランスポジションテーブルから得た最善手の点数。
    constexpr std::int64_t BEST_MOVE_SCORE = IID_MOVE_SCORE << 1;
    // 悪い取る手の点数。
    // constexpr std::int64_t BAD_CAPTURE_SCORE = -1LL;

    Bitboard enemy_king_bb =
    Util::SQUARE[engine_ptr_->king()[Util::GetOppositeSide(side)]];
    for (std::size_t i = start; i < last_; ++i) {
      // 手の情報を得る。
      Square from = GetFrom(move_stack_[i].move_);
      Square to = GetTo(move_stack_[i].move_);

      // 相手キングをチェックする手かどうか調べる。
      bool is_checking_move = false;
      switch (engine_ptr_->piece_board()[from]) {
        case PAWN:
          if ((enemy_king_bb & Util::GetPawnAttack(side, to))) {
            is_checking_move = true;
          }
          break;
        case KNIGHT:
          if ((enemy_king_bb & Util::GetKnightMove(to))) {
            is_checking_move = true;
          }
          break;
        case BISHOP:
          if ((enemy_king_bb & engine_ptr_->GetBishopAttack(to))) {
            is_checking_move = true;
          }
          break;
        case ROOK:
          if ((enemy_king_bb & engine_ptr_->GetRookAttack(to))) {
            is_checking_move = true;
          }
          break;
        case QUEEN:
          if ((enemy_king_bb & engine_ptr_->GetQueenAttack(to))) {
            is_checking_move = true;
          }
          break;
        default:
          // 何もしない。
          break;
      }

      // 特殊な手の点数をつける。
      if (EqualMove(move_stack_[i].move_, prev_best)) {
        // 前回の最善手。
        move_stack_[i].score_ = BEST_MOVE_SCORE;
      } else if (EqualMove(move_stack_[i].move_, iid_move)) {
        // IIDムーブ。
        move_stack_[i].score_ = IID_MOVE_SCORE;
      } else if (is_checking_move) {
        // 相手キングをチェックする手。
        move_stack_[i].score_ = CHECKING_MOVE_SCORE;
      } else if (EqualMove(move_stack_[i].move_, killer_1)) {
        // キラームーブ。
        move_stack_[i].score_ = KILLER_1_MOVE_SCORE;
      } else if (EqualMove(move_stack_[i].move_, killer_2)) {
        // キラームーブ。
        move_stack_[i].score_ = KILLER_2_MOVE_SCORE;
      } else {
        // その他の手を各候補手のタイプに分ける。
        if ((move_stack_[i].move_ & PROMOTION_MASK)) {
          SetScore<GenMoveType::CAPTURE>::F(*this, move_stack_[i].score_,
          move_stack_[i].move_, side, from, to);
        } else {
          SetScore<Type>::F(*this, move_stack_[i].score_, move_stack_[i].move_,
          side, from, to);
        }
      }
    }
  }

  // 実体化。
  template void MoveMaker::ScoreMoves<GenMoveType::NON_CAPTURE>
  (std::size_t start, Move best_move, Move iid_move, Move killer_1,
  Move killer_2, Side side);
  template void MoveMaker::ScoreMoves<GenMoveType::CAPTURE>
  (std::size_t start, Move best_move, Move iid_move, Move killer_1,
  Move killer_2, Side side);
}  // namespace Sayuri
