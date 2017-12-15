/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Hironori Ishibashi
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
  template<GenMoveType TYPE> int MoveMaker::GenMoves(Move prev_best,
  Move iid_move, Move killer_1, Move killer_2) {
    // 初期化。
    last_ = max_ = 0;
    history_max_ = 1;

    GenMovesCore<TYPE>(prev_best, iid_move, killer_1, killer_2);
    
    max_ = last_;
    return last_;
  }
  // インスタンス化。
  template
  int MoveMaker::GenMoves<GenMoveType::NON_CAPTURE>(Move, Move, Move, Move);
  template
  int MoveMaker::GenMoves<GenMoveType::CAPTURE>(Move, Move, Move, Move);
  template<>
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

  // ヒストリーの最大値を更新する。
  template<GenMoveType>
  inline void MoveMaker::UpdateMaxHistory(Side side, Square from, Square to) {
  }
  template<>
  inline void MoveMaker::UpdateMaxHistory<GenMoveType::NON_CAPTURE>
  (Side side, Square from, Square to) {
      Util::UpdateMax(history_max_,
      engine_ptr_->shared_st_ptr_->history_[side][from][to]);
  }

  // 指し手のマスクを作成する。
  template<GenMoveType>
  inline Bitboard MoveMaker::GenBitboardMask(Side side) const {return 0;}
  template<>
  inline Bitboard MoveMaker::GenBitboardMask<GenMoveType::NON_CAPTURE>
  (Side side) const {
    return ~(engine_ptr_->basic_st_.blocker_[R0]);
  }
  template<>
  inline Bitboard MoveMaker::GenBitboardMask<GenMoveType::CAPTURE>
  (Side side) const {
    return engine_ptr_->basic_st_.side_pieces_[Util::GetOppositeSide(side)];
  }

  // ポーンの候補手のビットボードを生成する。
  template<GenMoveType TYPE>
  inline Bitboard MoveMaker::GenPawnBitboard(Side side, Square from) const {
    return 0;
  }
  template<>
  inline Bitboard MoveMaker::GenPawnBitboard<GenMoveType::NON_CAPTURE>
  (Side side, Square from) const {
    return engine_ptr_->GetPawnStep(side, from);
  }
  template<>
  inline Bitboard MoveMaker::GenPawnBitboard<GenMoveType::CAPTURE>
  (Side side, Square from) const {
    Bitboard bb = Util::PAWN_ATTACK[side][from];
    return (bb & engine_ptr_->side_pieces()[Util::GetOppositeSide(side)])
      | (engine_ptr_->basic_st_.en_passant_square_
      ? (Util::SQUARE[engine_ptr_->basic_st_.en_passant_square_][R0] & bb)
      : 0);
  }

  // スタックに候補手を生成する。 (内部用)
  template<GenMoveType TYPE>
  void MoveMaker::GenMovesCore(Move prev_best, Move iid_move,
  Move killer_1, Move killer_2) {
    // 準備。
    const ChessEngine::BasicStruct& basic_st =  engine_ptr_->basic_st_;
    // サイド。
    Side side = basic_st.to_move_;

    // 生成開始時のポインタ。
    u32 start = last_;

    // ナイト、ビショップ、ルーク、クイーンの候補手を作る。
    for (PieceType piece_type = KNIGHT; piece_type <= QUEEN; ++piece_type) {
      Bitboard pieces = basic_st.position_[side][piece_type];

      for (; pieces; NEXT_BITBOARD(pieces)) {
        Square from = Util::GetSquare(pieces);

        // 各ピースの動き。
        Bitboard move_bitboard;
        switch (piece_type) {
          case KNIGHT:
            move_bitboard =
            Util::KNIGHT_MOVE[from] & GenBitboardMask<TYPE>(side);
            break;
          case BISHOP:
            move_bitboard =
            engine_ptr_->GetBishopAttack(from) & GenBitboardMask<TYPE>(side);
            break;
          case ROOK:
            move_bitboard =
            engine_ptr_->GetRookAttack(from) & GenBitboardMask<TYPE>(side);
            break;
          case QUEEN:
            move_bitboard =
            engine_ptr_->GetQueenAttack(from) & GenBitboardMask<TYPE>(side);
            break;
          default:
            throw SayuriError("MoveMaker::GenMoveCore()_1");
            break;
        }

        for (; move_bitboard; NEXT_BITBOARD(move_bitboard)) {
          // 手を作る。
          Move move = 0;
          Set<FROM>(move, from);
          Square to = Util::GetSquare(move_bitboard);
          Set<TO>(move, to);
          Set<MOVE_TYPE>(move, NORMAL);

          // ヒストリーの最大値を更新。 (テンプレート部品)
          UpdateMaxHistory<TYPE>(side, from, to);

          // スタックに登録。
          move_stack_[last_++].move_ = move;
        }
      }
    }

    // ポーンの動きを作る。
    Bitboard pieces = basic_st.position_[side][PAWN];
    for (; pieces; NEXT_BITBOARD(pieces)) {
      Square from = Util::GetSquare(pieces);

      Bitboard move_bitboard = GenPawnBitboard<TYPE>(side, from);

      for (; move_bitboard; NEXT_BITBOARD(move_bitboard)) {
        // 手を作る。
        Move move = 0;
        Set<FROM>(move, from);
        Square to = Util::GetSquare(move_bitboard);
        Set<TO>(move, to);

        // ヒストリーの最大値を更新。 (テンプレート部品)
        UpdateMaxHistory<TYPE>(side, from, to);

        if (Util::IsEnPassant(basic_st.en_passant_square_, to)) {
          Set<MOVE_TYPE>(move, EN_PASSANT);
        } else {
          Set<MOVE_TYPE>(move, NORMAL);
        }

        if (((side == WHITE) && (Util::SquareToRank(to) == RANK_8))
        || ((side == BLACK) && (Util::SquareToRank(to) == RANK_1))) {
          // 昇格を設定。
          for (PieceType piece_type = KNIGHT;
          piece_type <= QUEEN; ++piece_type) {
            Set<PROMOTION>(move, piece_type);
            move_stack_[last_++].move_ = move;
          }
        } else {
          // 昇格しない場合。
          move_stack_[last_++].move_ = move;
        }
      }
    }

    // キングの動きを作る。
    Square from = basic_st.king_[side];
    Bitboard move_bitboard =
    Util::KING_MOVE[from] & GenBitboardMask<TYPE>(side);

    // キャスリングの動きを作る。
    constexpr Move WS_CASTLING_MOVE = E1 | (G1 << SHIFT[TO])
    | (CASTLE_WS << SHIFT[MOVE_TYPE]);
    constexpr Move WL_CASTLING_MOVE = E1 | (C1 << SHIFT[TO])
    | (CASTLE_WL << SHIFT[MOVE_TYPE]);
    constexpr Move BS_CASTLING_MOVE = E8 | (G8 << SHIFT[TO])
    | (CASTLE_BS << SHIFT[MOVE_TYPE]);
    constexpr Move BL_CASTLING_MOVE = E8 | (C8 << SHIFT[TO])
    | (CASTLE_BL << SHIFT[MOVE_TYPE]);
    if (side == WHITE) {
      if (engine_ptr_->CanWhiteShortCastling()) {
        move_stack_[last_++].move_ = WS_CASTLING_MOVE;
      }
      if (engine_ptr_->CanWhiteLongCastling()) {
        move_stack_[last_++].move_ = WL_CASTLING_MOVE;
      }
    } else {
      if (engine_ptr_->CanBlackShortCastling()) {
        move_stack_[last_++].move_ = BS_CASTLING_MOVE;
      }
      if (engine_ptr_->CanBlackLongCastling()) {
        move_stack_[last_++].move_ = BL_CASTLING_MOVE;
      }
    }

    for (; move_bitboard; NEXT_BITBOARD(move_bitboard)) {
      Move move = 0;
      Set<FROM>(move, from);
      Square to = Util::GetSquare(move_bitboard);
      Set<TO>(move, to);
      Set<MOVE_TYPE>(move, NORMAL);

      // ヒストリーの最大値を更新。 (テンプレート部品)
      UpdateMaxHistory<TYPE>(side, from, to);

      move_stack_[last_++].move_ = move;
    }

    ScoreMoves<TYPE>(start, prev_best, iid_move, killer_1, killer_2, side);
  }

  // 実体化。
  template void MoveMaker::GenMovesCore<GenMoveType::NON_CAPTURE>
  (Move, Move, Move, Move);
  template void MoveMaker::GenMovesCore<GenMoveType::CAPTURE>
  (Move, Move, Move, Move);

  // 指し手のスコアを計算する。
  template<GenMoveType>
  inline i32 MoveMaker::CalScore
  (Move move, Side side, Square from, Square to) const {return 0;}
  template<>
  i32 MoveMaker::CalScore<GenMoveType::NON_CAPTURE>
  (Move move, Side side, Square from, Square to) const {
    // ヒストリーの点数の最大値のビット桁数。 (つまり、最大値は 0x200 )
    constexpr int MAX_HISTORY_SCORE_SHIFT = 9;

    // ヒストリーを使って点数をつけていく。
    return (engine_ptr_->shared_st_ptr_->history_[side][from][to]
    << MAX_HISTORY_SCORE_SHIFT) / history_max_;
  }
  template<>
  inline i32 MoveMaker::CalScore<GenMoveType::CAPTURE>
  (Move move, Side side, Square from, Square to) const {
    // 駒を取る手のビットシフト。
    constexpr int CAPTURE_SCORE_SHIFT = 12;
    // 悪い取る手の点数。
    constexpr i32 BAD_CAPTURE_SCORE = -1;

    // SEEで点数をつけていく。
    return Util::GetMax(static_cast<i32>(engine_ptr_->SEE(move, 0))
    << CAPTURE_SCORE_SHIFT, BAD_CAPTURE_SCORE);
  }

  // 候補手に点数をつける。
  template<GenMoveType TYPE>
  void MoveMaker::ScoreMoves(u32 start, Move prev_best,
  Move iid_move, Move killer_1, Move killer_2, Side side) {
    // --- 評価値の定義 --- //
    // キラームーブの点数。
    constexpr i32 KILLER_2_MOVE_SCORE = 0x400;
    constexpr i32 KILLER_1_MOVE_SCORE = KILLER_2_MOVE_SCORE << 1;
    // 駒を取る手のビットシフト。
    // constexpr int CAPTURE_SCORE_SHIFT = 12;
    // 相手キングをチェックする手の点数。
    constexpr i32 CHECKING_MOVE_SCORE = 1 << 28;
    // IIDで得た最善手の点数。
    constexpr i32 IID_MOVE_SCORE = CHECKING_MOVE_SCORE << 1;
    // 前回の繰り返しでトランスポジションテーブルから得た最善手の点数。
    constexpr i32 BEST_MOVE_SCORE = IID_MOVE_SCORE << 1;
    // 悪い取る手の点数。
    // constexpr i32 BAD_CAPTURE_SCORE = -1;

    Bitboard enemy_king_bb =
    Util::SQUARE[engine_ptr_->basic_st_.king_
    [Util::GetOppositeSide(side)]][R0];
    for (u32 i = start; i < last_; ++i) {
      // 手の情報を得る。
      Square from = Get<FROM>(move_stack_[i].move_);
      Square to = Get<TO>(move_stack_[i].move_);

      // 相手キングをチェックする手かどうか調べる。
      bool is_checking_move = false;
      switch (engine_ptr_->basic_st_.piece_board_[from]) {
        case PAWN:
          if ((enemy_king_bb & Util::PAWN_ATTACK[side][to])) {
            is_checking_move = true;
          }
          break;
        case KNIGHT:
          if ((enemy_king_bb & Util::KNIGHT_MOVE[to])) {
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
        move_stack_[i].score_ =
        CalScore<TYPE>(move_stack_[i].move_, side, from, to);
      }
    }
  }

  // 実体化。
  template void MoveMaker::ScoreMoves<GenMoveType::NON_CAPTURE>
  (u32, Move, Move, Move, Move, Side);
  template void MoveMaker::ScoreMoves<GenMoveType::CAPTURE>
  (u32, Move, Move, Move, Move, Side);
}  // namespace Sayuri
