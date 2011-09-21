/* chess_board_gen_move.cpp: 手を作る実装。
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
#include "chess_def.h"
#include "chess_util.h"

#include "misaki_debug.h"

namespace Misaki {
  // 駒を取る手を作る。
  int ChessBoard::GenCaptureMove(int level) {
    // 手の数。
    int move_count = 0;

    // 最大レベルより大きければ何もしない。
    if (level > (MAX_LEVEL - 1)) return move_count;

    // 調べる駒。
    bitboard_t pieces;
    // 攻撃の筋。
    bitboard_t attack;
    // 自分のサイド。
    side_t side = to_move_;
    // 相手のサイド。
    side_t enemy_side = static_cast<side_t>(side ^ 0x3);

    // 移動する駒の位置。
    square_t piece_square;
    // 移動先の位置。
    square_t goal_square;

    // 手。
    move_t move;

    // ポーンの手を作る。
    pieces = position_[side][PAWN];
    attack = 0;
    // アンパッサン。
    square_t en_passant_square = static_cast<square_t>
    (side == WHITE ? en_passant_target_ + 8 : en_passant_target_ - 8);
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 攻撃の筋を入れる。
      attack = ChessUtil::GetPawnAttack(piece_square, side)
      & side_pieces_[enemy_side];
      // アンパッサンならアンパッサンの攻撃を追加。
      if (can_en_passant_) {
        if ((side == WHITE) && (side_board_[en_passant_target_] == BLACK)) {
          rank_t target_rank = ChessUtil::GetRank(en_passant_target_);
          rank_t attacker_rank = ChessUtil::GetRank(piece_square);
          if (target_rank == attacker_rank) {
            if ((piece_square == (en_passant_target_ - 1))
            || (piece_square == (en_passant_target_ + 1))) {
              attack |= ChessUtil::BIT[en_passant_square];
            }
          }
        }
        if ((side == BLACK) && (side_board_[en_passant_target_] == WHITE)) {
          rank_t target_rank = ChessUtil::GetRank(en_passant_target_);
          rank_t attacker_rank = ChessUtil::GetRank(piece_square);
          if (target_rank == attacker_rank) {
            if ((piece_square == (en_passant_target_ - 1))
            || (piece_square == (en_passant_target_ + 1))) {
              attack |= ChessUtil::BIT[en_passant_square];
            }
          }
        }
      }
      for (; attack; attack &= attack - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(attack);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // アンパッサンなら手の種類に追加する。
        if (can_en_passant_ && (goal_square == en_passant_square)) {
          move.move_type_ = EN_PASSANT;
        }
        // 昇格しなければいけないなら昇格の手を入れる。
        if ((side == WHITE)
        && ((goal_square >= A8) && (goal_square <= H8))) {
          move.promotion_ = KNIGHT;
          PushMove(move, level);
          move_count++;
          move.promotion_ = BISHOP;
          PushMove(move, level);
          move_count++;
          move.promotion_ = ROOK;
          PushMove(move, level);
          move_count++;
          move.promotion_ = QUEEN;
          PushMove(move, level);
          move_count++;
          // 昇格する駒を元に戻す。
          move.promotion_ = EMPTY;
        } else if ((side == BLACK)
        && ((goal_square >= A1) && (goal_square <= H1))) {
          move.promotion_ = KNIGHT;
          PushMove(move, level);
          move_count++;
          move.promotion_ = BISHOP;
          PushMove(move, level);
          move_count++;
          move.promotion_ = ROOK;
          PushMove(move, level);
          move_count++;
          move.promotion_ = QUEEN;
          PushMove(move, level);
          move_count++;
          // 昇格する駒を元に戻す。
          move.promotion_ = EMPTY;
        } else {
          PushMove(move, level);
          move_count++;
        }
      }
    }

    // ナイトの手を作る。
    pieces = position_[side][KNIGHT];
    attack = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 攻撃の筋を入れる。
      attack = ChessUtil::GetKnightMove(piece_square)
      & side_pieces_[enemy_side];
      for (; attack; attack &= attack - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(attack);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // プッシュする。
        PushMove(move, level);
        move_count++;
      }
    }

    // ビショップの手を作る。
    pieces = position_[side][BISHOP];
    attack = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 攻撃の筋を入れる。
      attack = GetBishopAttack(piece_square) & side_pieces_[enemy_side];
      for (; attack; attack &= attack - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(attack);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // プッシュする。
        PushMove(move, level);
        move_count++;
      }
    }

    // ルークの手を作る。
    pieces = position_[side][ROOK];
    attack = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 攻撃の筋を作る。
      attack = GetRookAttack(piece_square) & side_pieces_[enemy_side];
      for (; attack; attack &= attack - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(attack);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // プッシュする。
        PushMove(move, level);
        move_count++;
      }
    }

    // クイーンの手を作る。
    pieces = position_[side][QUEEN];
    attack = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 攻撃の筋を作る。
      attack = GetQueenAttack(piece_square) & side_pieces_[enemy_side];
      for (; attack; attack &= attack - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(attack);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // プッシュする。
        PushMove(move, level);
        move_count++;
      }
    }

    // キングの手を作る。
    attack = ChessUtil::GetKingMove(king_[side]) & side_pieces_[enemy_side];
    for (; attack; attack &= attack - 1) {
      move.all_ = 0;
      goal_square = ChessUtil::GetSquare(attack);
      // 手を作る。
      move.piece_square_ = king_[side];
      move.goal_square_ = goal_square;
      // プッシュする。
      PushMove(move, level);
      move_count++;
    }

    // 次のレベルへの準備をする。
    if (level < (MAX_LEVEL - 1)) {
      tree_ptr_[level + 1] = stack_ptr_[level] + 1;
      stack_ptr_[level + 1] = tree_ptr_[level + 1];
    }

    // 手の数を返す。
    return move_count;
  }
  // 駒を取らない手を展開する。
  int ChessBoard::GenNonCaptureMove(int level) {
    // 手の数。
    int move_count = 0;

    // 最大レベルより大きければ何もしない。
    if (level > (MAX_LEVEL - 1)) return move_count;

    // 調べる駒。
    bitboard_t pieces;
    // 移動する位置。
    bitboard_t move_bitboard;
    // 自分のサイド。
    side_t side = to_move_;
    // 相手のサイド。
    side_t enemy_side = static_cast<side_t>(side ^ 0x3);

    // 移動する駒の位置。
    square_t piece_square;
    // 移動先の位置。
    square_t goal_square;

    // 手。
    move_t move;

    // ポーンの手を作る。
    pieces = position_[side][PAWN];
    move_bitboard = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 動かすビットボードを得る。
      move_bitboard = ChessUtil::GetPawnMove(piece_square, side) & ~blocker0_;
      // 動かすビットボードがあれば2歩の動きのビットボードを得る。
      if (move_bitboard) {
        move_bitboard |=
        ChessUtil::GetPawn2StepMove(piece_square, side) & ~blocker0_;
      }
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // 昇格しなければいけないなら昇格の手を入れる。
        if ((side == WHITE)
        && ((goal_square >= A8) && (goal_square <= H8))) {
          move.promotion_ = KNIGHT;
          PushMove(move, level);
          move_count++;
          move.promotion_ = BISHOP;
          PushMove(move, level);
          move_count++;
          move.promotion_ = ROOK;
          PushMove(move, level);
          move_count++;
          move.promotion_ = QUEEN;
          PushMove(move, level);
          move_count++;
          // 昇格する駒を元に戻す。
          move.promotion_ = EMPTY;
        } else if ((side == BLACK)
        && ((goal_square >= A1) && (goal_square <= H1))) {
          move.promotion_ = KNIGHT;
          PushMove(move, level);
          move_count++;
          move.promotion_ = BISHOP;
          PushMove(move, level);
          move_count++;
          move.promotion_ = ROOK;
          PushMove(move, level);
          move_count++;
          move.promotion_ = QUEEN;
          PushMove(move, level);
          move_count++;
          // 昇格する駒を元に戻す。
          move.promotion_ = EMPTY;
        } else {
          PushMove(move, level);
          move_count++;
        }
      }
    }

    // ナイトの手を作る。
    pieces = position_[side][KNIGHT];
    move_bitboard = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 動きを作る。
      move_bitboard = ChessUtil::GetKnightMove(piece_square) & ~blocker0_;
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // プッシュする。
        PushMove(move, level);
        move_count++;
      }
    }

    // ビショップの手を作る。
    pieces = position_[side][BISHOP];
    move_bitboard = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 攻撃の筋を入れる。
      move_bitboard = GetBishopAttack(piece_square) & ~blocker0_;
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // プッシュする。
        PushMove(move, level);
        move_count++;
      }
    }

    // ルークの手を作る。
    pieces = position_[side][ROOK];
    move_bitboard = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 攻撃の筋を作る。
      move_bitboard = GetRookAttack(piece_square) & ~blocker0_;
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // プッシュする。
        PushMove(move, level);
        move_count++;
      }
    }

    // クイーンの手を作る。
    pieces = position_[side][QUEEN];
    move_bitboard = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 攻撃の筋を作る。
      move_bitboard = GetQueenAttack(piece_square) & ~blocker0_;
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // プッシュする。
        PushMove(move, level);
        move_count++;
      }
    }

    // キングの手を作る。
    move_bitboard = ChessUtil::GetKingMove(king_[side]) & ~blocker0_;
    // キャスリングの動きを作る。
    if (side == WHITE) {
      if (castling_rights_ & WHITE_SHORT_CASTLING) {
        if (!IsAttacked(E1, enemy_side)
        && !IsAttacked(F1, enemy_side)
        && !IsAttacked(G1, enemy_side)) {
          if (!piece_board_[F1] && !piece_board_[G1]) {
            move_bitboard |= ChessUtil::BIT[G1];
          }
        }
      }
      if (castling_rights_ & WHITE_LONG_CASTLING) {
        if (!IsAttacked(E1, enemy_side)
        && !IsAttacked(D1, enemy_side)
        && !IsAttacked(C1, enemy_side)) {
          if (!piece_board_[D1] && !piece_board_[C1] && !piece_board_[B1]) {
            move_bitboard |= ChessUtil::BIT[C1];
          }
        }
      }
    } else {
      if (castling_rights_ & BLACK_SHORT_CASTLING) {
        if (!IsAttacked(E8, enemy_side)
        && !IsAttacked(F8, enemy_side)
        && !IsAttacked(G8, enemy_side)) {
          if (!piece_board_[F8] && !piece_board_[G8]) {
            move_bitboard |= ChessUtil::BIT[G8];
          }
        }
      }
      if (castling_rights_ & BLACK_LONG_CASTLING) {
        if (!IsAttacked(E8, enemy_side)
        && !IsAttacked(D8, enemy_side)
        && !IsAttacked(C8, enemy_side)) {
          if (!piece_board_[D8] && !piece_board_[C8] && !piece_board_[B8]) {
            move_bitboard |= ChessUtil::BIT[C8];
          }
        }
      }
    }
    for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
      move.all_ = 0;
      goal_square = ChessUtil::GetSquare(move_bitboard);
      // 手を作る。
      move.piece_square_ = king_[side];
      move.goal_square_ = goal_square;
      // キャスリングなら手の種類をキャスリングにする。
      if (((side == WHITE) && (king_[side] == E1) && (goal_square == G1))
      || ((side == WHITE) && (king_[side] == E1) && (goal_square == C1))
      || ((side == BLACK) && (king_[side] == E8) && (goal_square == G8))
      || ((side == BLACK) && (king_[side] == E8) && (goal_square == C8))) {
        move.move_type_ = CASTLING;
      }
      // プッシュする。
      PushMove(move, level);
      move_count++;
    }

    // 次のレベルへの準備をする。
    if (level < (MAX_LEVEL - 1)) {
      tree_ptr_[level + 1] = stack_ptr_[level] + 1;
      stack_ptr_[level + 1] = tree_ptr_[level + 1];
    }

    // 手の数を返す。
    return move_count;
  }
  // 全ての手を展開する。
  int ChessBoard::GenMove(int level) {
    int count = 0;

    count += GenNonCaptureMove(level);
    count += GenCaptureMove(level);

    return count;
  }
  int ChessBoard::GenCheckEscapeMove(int level) {
    // 手の数。
    int move_count = 0;

    // 最大レベルより大きければ何もしない。
    if (level > (MAX_LEVEL - 1)) return move_count;

    // 調べる駒。
    bitboard_t pieces;
    // 手のビットボード。
    bitboard_t move_bitboard;
    // 自分のサイド。
    side_t side = to_move_;
    // 相手のサイド。
    side_t enemy_side = static_cast<side_t>(side ^ 0x3);

    // 移動する駒の位置。
    square_t piece_square;
    // 移動先の位置。
    square_t goal_square;

    // 手。
    move_t move;

    // ポーンの手を作る。
    pieces = position_[side][PAWN];
    move_bitboard = 0;
    // アンパッサン。
    square_t en_passant_square = static_cast<square_t>
    (side == WHITE ? en_passant_target_ + 8 : en_passant_target_ - 8);
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 通常の動きを入れる。
      move_bitboard = ChessUtil::GetPawnMove(piece_square, side) & ~blocker0_;
      if (move_bitboard) {
        move_bitboard |= ChessUtil::GetPawn2StepMove(piece_square, side)
        & ~blocker0_;
      }
      // 攻撃の筋を入れる。
      move_bitboard |= ChessUtil::GetPawnAttack(piece_square, side)
      & side_pieces_[enemy_side];
      // アンパッサンならアンパッサンの攻撃を追加。
      if (can_en_passant_) {
        if ((side == WHITE) && (side_board_[en_passant_target_] == BLACK)) {
          rank_t target_rank = ChessUtil::GetRank(en_passant_target_);
          rank_t move_bitboarder_rank = ChessUtil::GetRank(piece_square);
          if (target_rank == move_bitboarder_rank) {
            if ((piece_square == (en_passant_target_ - 1))
            || (piece_square == (en_passant_target_ + 1))) {
              move_bitboard |= ChessUtil::BIT[en_passant_square];
            }
          }
        }
        if ((side == BLACK) && (side_board_[en_passant_target_] == WHITE)) {
          rank_t target_rank = ChessUtil::GetRank(en_passant_target_);
          rank_t move_bitboarder_rank = ChessUtil::GetRank(piece_square);
          if (target_rank == move_bitboarder_rank) {
            if ((piece_square == (en_passant_target_ - 1))
            || (piece_square == (en_passant_target_ + 1))) {
              move_bitboard |= ChessUtil::BIT[en_passant_square];
            }
          }
        }
      }
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // アンパッサンなら手の種類に追加する。
        if (can_en_passant_ && (goal_square == en_passant_square)) {
          move.move_type_ = EN_PASSANT;
        }
        // キングがチェックされていなければプッシュする。
        MakeMove(move);
        if (!IsAttacked(king_[side], enemy_side)) {
          // 昇格しなければいけないなら昇格の手を入れる。
          if ((side == WHITE)
          && ((goal_square >= A8) && (goal_square <= H8))) {
            move.promotion_ = KNIGHT;
            PushMove(move, level);
            move_count++;
            move.promotion_ = BISHOP;
            PushMove(move, level);
            move_count++;
            move.promotion_ = ROOK;
            PushMove(move, level);
            move_count++;
            move.promotion_ = QUEEN;
            PushMove(move, level);
            move_count++;
            // 昇格する駒を元に戻す。
            move.promotion_ = EMPTY;
          } else if ((side == BLACK)
          && ((goal_square >= A1) && (goal_square <= H1))) {
            move.promotion_ = KNIGHT;
            PushMove(move, level);
            move_count++;
            move.promotion_ = BISHOP;
            PushMove(move, level);
            move_count++;
            move.promotion_ = ROOK;
            PushMove(move, level);
            move_count++;
            move.promotion_ = QUEEN;
            PushMove(move, level);
            move_count++;
            // 昇格する駒を元に戻す。
            move.promotion_ = EMPTY;
          } else {
            PushMove(move, level);
            move_count++;
          }
        }
        UnmakeMove(move);
      }
    }

    // ナイトの手を作る。
    pieces = position_[side][KNIGHT];
    move_bitboard = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 動きを入れる。
      move_bitboard = ChessUtil::GetKnightMove(piece_square)
      & ~side_pieces_[side];
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // キングがチェックされていなければプッシュする。
        MakeMove(move);
        if (!IsAttacked(king_[side], enemy_side)) {
          PushMove(move, level);
          move_count++;
        }
        UnmakeMove(move);
      }
    }

    // ビショップの手を作る。
    pieces = position_[side][BISHOP];
    move_bitboard = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 動きを入れる。
      move_bitboard = GetBishopAttack(piece_square) & ~side_pieces_[side];
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // キングがチェックされていなければプッシュする。
        MakeMove(move);
        if (!IsAttacked(king_[side], enemy_side)) {
          PushMove(move, level);
          move_count++;
        }
        UnmakeMove(move);
      }
    }

    // ルークの手を作る。
    pieces = position_[side][ROOK];
    move_bitboard = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 動きを作る。
      move_bitboard = GetRookAttack(piece_square) & ~side_pieces_[side];
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // キングがチェックされていなければプッシュする。
        MakeMove(move);
        if (!IsAttacked(king_[side], enemy_side)) {
          PushMove(move, level);
          move_count++;
        }
        UnmakeMove(move);
      }
    }

    // クイーンの手を作る。
    pieces = position_[side][QUEEN];
    move_bitboard = 0;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      // 動きを作る。
      move_bitboard = GetQueenAttack(piece_square) & ~side_pieces_[side];
      for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
        move.all_ = 0;
        goal_square = ChessUtil::GetSquare(move_bitboard);
        // 手を作る。
        move.piece_square_ = piece_square;
        move.goal_square_ = goal_square;
        // キングがチェックされていなければプッシュする。
        MakeMove(move);
        if (!IsAttacked(king_[side], enemy_side)) {
          PushMove(move, level);
          move_count++;
        }
        UnmakeMove(move);
      }
    }

    // キングの手を作る。
    move_bitboard = ChessUtil::GetKingMove(king_[side]) & ~side_pieces_[side];
    // キャスリングの動きを作る。
    if (side == WHITE) {
      if (castling_rights_ & WHITE_SHORT_CASTLING) {
        if (!IsAttacked(E1, enemy_side)
        && !IsAttacked(F1, enemy_side)
        && !IsAttacked(G1, enemy_side)) {
          if (!piece_board_[F1] && !piece_board_[G1]) {
            move_bitboard |= ChessUtil::BIT[G1];
          }
        }
      }
      if (castling_rights_ & WHITE_LONG_CASTLING) {
        if (!IsAttacked(E1, enemy_side)
        && !IsAttacked(D1, enemy_side)
        && !IsAttacked(C1, enemy_side)) {
          if (!piece_board_[D1] && !piece_board_[C1] && !piece_board_[B1]) {
            move_bitboard |= ChessUtil::BIT[C1];
          }
        }
      }
    } else {
      if (castling_rights_ & BLACK_SHORT_CASTLING) {
        if (!IsAttacked(E8, enemy_side)
        && !IsAttacked(F8, enemy_side)
        && !IsAttacked(G8, enemy_side)) {
          if (!piece_board_[F8] && !piece_board_[G8]) {
            move_bitboard |= ChessUtil::BIT[G8];
          }
        }
      }
      if (castling_rights_ & BLACK_LONG_CASTLING) {
        if (!IsAttacked(E8, enemy_side)
        && !IsAttacked(D8, enemy_side)
        && !IsAttacked(C8, enemy_side)) {
          if (!piece_board_[D8] && !piece_board_[C8] && !piece_board_[B8]) {
            move_bitboard |= ChessUtil::BIT[C8];
          }
        }
      }
    }
    for (; move_bitboard; move_bitboard &= move_bitboard - 1) {
      move.all_ = 0;
      goal_square = ChessUtil::GetSquare(move_bitboard);
      // 手を作る。
      move.piece_square_ = king_[side];
      move.goal_square_ = goal_square;
      // キャスリングならキャスリングの種類を入れる。
      if (((side == WHITE) && (king_[side] == E1) && (goal_square == G1))
      || ((side == WHITE) && (king_[side] == E1) && (goal_square == C1))
      || ((side == BLACK) && (king_[side] == E8) && (goal_square == G8))
      || ((side == BLACK) && (king_[side] == E8) && (goal_square == C8))) {
        move.move_type_ = CASTLING;
      }
      // キングがチェックされていなければプッシュする。
      MakeMove(move);
      if (!IsAttacked(king_[side], enemy_side)) {
        PushMove(move, level);
        move_count++;
      }
      UnmakeMove(move);
    }

    // 次のレベルへの準備をする。
    if (level < (MAX_LEVEL - 1)) {
      tree_ptr_[level + 1] = stack_ptr_[level] + 1;
      stack_ptr_[level + 1] = tree_ptr_[level + 1];
    }

    // 手の数を返す。
    return move_count;
  }
  // SEE。
  int ChessBoard::SEE(move_t move) {
    // 価値の配列。
    static const int value_array[NUM_PIECE_TYPES] = {
      0,
      SCORE_PAWN,
      SCORE_KNIGHT,
      SCORE_BISHOP,
      SCORE_ROOK,
      SCORE_QUEEN,
      SCORE_KING
    };

    // 価値。
    int value;

    // 動かす駒と移動先の位置の駒の種類を得る。
    piece_t piece_type = piece_board_[move.piece_square_];
    piece_t target_type = piece_board_[move.goal_square_];

    // 移動先の駒が動かす駒よりも価値が高ければ計算して返す。
    value = value_array[target_type] - value_array[piece_type];
    if (value > 0) return value;

    // 自分のサイドと敵のサイド。
    side_t side = to_move_;
    side_t enemy_side = static_cast<side_t>(side ^ 0x3);

    // 攻撃する駒の、各サイドのビットボード。
    bitboard_t attackers = GetAttackers(move.goal_square_, side)
    & ~ChessUtil::BIT[move.piece_square_];
    bitboard_t enemy_attackers = GetAttackers(move.goal_square_, enemy_side);

    // 攻撃する駒のビットボード。
    bitboard_t attacker_pieces[NUM_SIDES][NUM_PIECE_TYPES];
    for (int piece_type = PAWN; piece_type < NUM_PIECE_TYPES; piece_type++) {
      attacker_pieces[side][piece_type] =
      position_[side][piece_type] & attackers;
      attacker_pieces[enemy_side][piece_type] =
      position_[enemy_side][piece_type] & enemy_attackers;
    }

    // 取り合いの駒の評価。
    int index = 2;
    piece_t prev_piece;  // 前の駒。
    piece_t current_piece = piece_type;  // 現在調べている駒。
    side_t search_side = enemy_side;  //  現在調べているサイド。
    int prev_value = 0;  // 前の評価値。
    int prev_prev_value = 0;  // 前の前の評価値。
    value = value_array[target_type];  // 現在の評価値。
    while (index < MAX_LEVEL) {
      prev_piece = current_piece;
      prev_prev_value = prev_value;
      prev_value = value;
      // 現在の取る駒をセットする。
      if (attacker_pieces[search_side][PAWN]) {
        current_piece = PAWN;
        attacker_pieces[search_side][PAWN] &=
        attacker_pieces[search_side][PAWN] - 1;
      } else if (attacker_pieces[search_side][KNIGHT]) {
        current_piece = KNIGHT;
        attacker_pieces[search_side][KNIGHT] &=
        attacker_pieces[search_side][KNIGHT] - 1;
      } else if (attacker_pieces[search_side][BISHOP]) {
        current_piece = BISHOP;
        attacker_pieces[search_side][BISHOP] &=
        attacker_pieces[search_side][BISHOP] - 1;
      } else if (attacker_pieces[search_side][ROOK]) {
        current_piece = ROOK;
        attacker_pieces[search_side][ROOK] &=
        attacker_pieces[search_side][ROOK] - 1;
      } else if (attacker_pieces[search_side][QUEEN]) {
        current_piece = QUEEN;
        attacker_pieces[search_side][QUEEN] &=
        attacker_pieces[search_side][QUEEN] - 1;
      } else if (attacker_pieces[search_side][KING]) {
        current_piece = KING;
        attacker_pieces[search_side][KING] &=
        attacker_pieces[search_side][KING] - 1;
      } else {
        current_piece = EMPTY;
      }
      // もし現在のこまがなければ価値を返す。
      if (current_piece == EMPTY) {
        return prev_value;
      }
      if (prev_piece == KING) {
        return prev_prev_value;
      }
      // 現在の評価値をセットする。
      if (search_side == side) {
        value += value_array[prev_piece];
      } else {
        value -= value_array[prev_piece];
      }
      search_side = static_cast<side_t>(search_side ^ 0x3);
      index++;
    }

    return value;
  }
  // ノードに簡易点数を付ける。
  void ChessBoard::GiveQuickScore(hash_key_t key, int level, int depth,
  side_t side, TranspositionTable& table) {
    // 点数を付けるのノード。
    Node* node = stack_ptr_[level] - 1;

    // スロットがあればスロットの最善手を得る。
    // 前回の探索は深さが1つ小さいのでdepthを1つ小さくする。
    depth -= 1;
    TranspositionTableSlot* slot = NULL;
    try {
      slot = &(table.GetSameSlot(key, level, depth, to_move_));
    } catch (...) {
      // 何もしない。
    }
    move_t best_move;
    best_move.all_ = 0;
    if (slot) {
      best_move = slot->best_move();
    }

    // SEEする。
    for (; node >= tree_ptr_[level]; node--) {
      if ((best_move.all_ != 0)
      && ((node->move_.all_ & 0x38fff) == (best_move.all_ & 0x38fff))) {
        node->quick_score_ = INFINITE;
      } else {
        node->quick_score_ = SEE(node->move_);
      }
    }
  }
  // 簡易点数の高いもの順にポップする。
  move_t ChessBoard::PopBestMove(int level) {
    // もしレベルがマックスを越えていたら無意味な手を返す。。
    if (level > (MAX_LEVEL - 1)) {
      move_t move;
      move.all_ = 0;
      return move;
    }

    // もし手がなければ無意味な手を返す。。
    if (stack_ptr_[level] == tree_ptr_[level]) {
      move_t move;
      move.all_ = 0;
      return move;
    }

    // スタックの最初のノード。
    // ついでにスタックも1つ下げておく。
    Node* top_node = --(stack_ptr_[level]);

    // 調べるノード。
    Node* node = top_node;

    // 簡易得点の一番高いノード。
    Node* best_node = node;

    // 最高簡易得点を得る。
    for (; node >= tree_ptr_[level]; node--) {
      if (node->quick_score_ > best_node->quick_score_) {
        best_node = node;
      }
    }

    // スワップする。
    Node temp = *top_node;
    *top_node = *best_node;
    *best_node = temp;

    // 返る。
    return top_node->move_;
  }
}  // Misaki
