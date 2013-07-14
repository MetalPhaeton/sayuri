/* chess_engine_analyze.cpp: チェスボード分析用ツール。
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
#include "chess_def.h"
#include "chess_util.h"

namespace Sayuri {
  /************************
   * 局面を分析する関数。 *
   ************************/
  // テーブルを計算する関数。
  int ChessEngine::GetTableValue(const int (& table)[NUM_SQUARES],
  Side side, Bitboard bitboard) {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // ボードを鏡対象に上下反転させる配列。
    // <配列>[flip[<位置>]]と使うと上下が反転する。
    static const Square flip[NUM_SQUARES] = {
      A8, B8, C8, D8, E8, F8, G8, H8,
      A7, B7, C7, D7, E7, F7, G7, H7,
      A6, B6, C6, D6, E6, F6, G6, H6,
      A5, B5, C5, D5, E5, F5, G5, H5,
      A4, B4, C4, D4, E4, F4, G4, H4,
      A3, B3, C3, D3, E3, F3, G3, H3,
      A2, B2, C2, D2, E2, F2, G2, H2,
      A1, B1, C1, D1, E1, F1, G1, H1
    };

    // 位置。
    Square square;

    // 値。
    int value = 0;

    // 配列を足す。
    if (side == WHITE) {
      for (; bitboard; bitboard &= bitboard - 1) {
        square = Util::GetSquare(bitboard);
        value += table[square];
      }
    } else {
      for (; bitboard; bitboard &= bitboard - 1) {
        square = Util::GetSquare(bitboard);
        value += table[flip[square]];
      }
    }

    // 値を返す。
    return value;
  }
  // 勝つのに十分な駒があるかどうか調べる。
  bool ChessEngine::HasEnoughPieces(Side side) const {
    // サイドを確認。
    if (side == NO_SIDE) return false;

    // ポーンがあれば大丈夫。
    if (position_[side][PAWN]) return true;

    // ルークがあれば大丈夫。
    if (position_[side][ROOK]) return true;

    // クイーンがあれば大丈夫。
    if (position_[side][QUEEN]) return true;

    // ビショップが2つあれば大丈夫。
    if (Util::CountBits(position_[side][BISHOP]) >= 2) return true;

    // ナイトが2つあれば大丈夫。
    if (Util::CountBits(position_[side][KNIGHT]) >= 2) return true;

    // ナイトとビショップの合計が2つあれば大丈夫。
    if (Util::CountBits(position_[side][KNIGHT]
    | position_[side][BISHOP]) >= 2)
      return true;

    // それ以外はダメ。
    return false;
  }
  // 動ける位置の数を得る。
  int ChessEngine::GetMobility(Square piece_square) const {
    // 駒の種類を得る。
    Piece Pieceype = piece_board_[piece_square];

    // 駒のサイドと敵のサイドを得る。
    Side side = side_board_[piece_square];
    Side enemy_side = side ^ 0x3;

    // 駒がなければ0を返す。
    if (!Pieceype) return 0;

    // 利き筋を入れる。
    Bitboard move_bitboard;
    switch (Pieceype) {
      case PAWN:
        // 通常の動き。
        move_bitboard = Util::GetPawnMove(piece_square, side) & ~blocker0_;
        // 2歩の動き。
        if (move_bitboard) {
          move_bitboard |= Util::GetPawn2StepMove(piece_square, side)
          & ~blocker0_;
        }
        // 攻撃。
        move_bitboard |= Util::GetPawnAttack(piece_square, side)
        & side_pieces_[enemy_side];
        // アンパッサン。
        if (can_en_passant_) {
          if (side_board_[en_passant_target_] != side) {
            Rank attacker_rank = Util::GetRank(piece_square);
            Rank target_rank = Util::GetRank(en_passant_target_);
            if (attacker_rank == target_rank) {
              if ((piece_square == (en_passant_target_ + 1))
              || (piece_square == (en_passant_target_ - 1))) {
                move_bitboard |= (side == WHITE ?
                Util::BIT[en_passant_target_ + 8]
                : Util::BIT[en_passant_target_ - 8]);
              }
            }
          }
        }
        break;
      case KNIGHT:
        move_bitboard = Util::GetKnightMove(piece_square)
        & ~side_pieces_[side];
        break;
      case BISHOP:
        move_bitboard = GetBishopAttack(piece_square)
        & ~side_pieces_[side];
        break;
      case ROOK:
        move_bitboard = GetRookAttack(piece_square)
        & ~side_pieces_[side];
        break;
      case QUEEN:
        move_bitboard = GetQueenAttack(piece_square)
        & ~side_pieces_[side];
        break;
      case KING:
        move_bitboard = Util::GetKingMove(piece_square)
        & ~side_pieces_[side];
        // キャスリング。
        if ((side == WHITE) && (piece_square == E1)) {  // 白。
          // ショートキャスリング。
          if (castling_rights_ & WHITE_SHORT_CASTLING) {
            if (!IsAttacked(E1, enemy_side)
            && !IsAttacked(F1, enemy_side)
            && !IsAttacked(G1, enemy_side)) {
              if (!piece_board_[F1] && !piece_board_[G1]) {
                move_bitboard |= Util::BIT[G1];
              }
            }
          }
          // ロングキャスリング。
          if (castling_rights_ & WHITE_LONG_CASTLING) {
            if (!IsAttacked(E1, enemy_side)
            && !IsAttacked(D1, enemy_side)
            && !IsAttacked(C1, enemy_side)) {
              if (!piece_board_[D1] && !piece_board_[C1]
              && !piece_board_[B1]) {
                move_bitboard |= Util::BIT[C1];
              }
            }
          }
        } else if ((side == BLACK) && (piece_square == E8)) {  // 黒。
          // ショートキャスリング。
          if (castling_rights_ & BLACK_SHORT_CASTLING) {
            if (!IsAttacked(E8, enemy_side)
            && !IsAttacked(F8, enemy_side)
            && !IsAttacked(G8, enemy_side)) {
              if (!piece_board_[F8] && !piece_board_[G8]) {
                move_bitboard |= Util::BIT[G8];
              }
            }
          }
          // ロングキャスリング。
          if (castling_rights_ & BLACK_LONG_CASTLING) {
            if (!IsAttacked(E8, enemy_side)
            && !IsAttacked(D8, enemy_side)
            && !IsAttacked(C8, enemy_side)) {
              if (!piece_board_[D8] && !piece_board_[C8]
              && !piece_board_[B8]) {
                move_bitboard |= Util::BIT[C8];
              }
            }
          }
        }
        break;
    }

    // ビットの数を数えて返す。
    return Util::CountBits(move_bitboard);
  }
  // 攻撃している位置のビットボードを得る。
  Bitboard ChessEngine::GetAttack(Bitboard pieces) const {
    // 駒を整理。
    pieces &= blocker0_;

    // 攻撃位置。
    Bitboard attack = 0;

    // 駒を1つずつ調べていく。
    Square piece_square;
    Piece Pieceype;
    Side side;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = Util::GetSquare(pieces);
      Pieceype = piece_board_[piece_square];
      side = side_board_[piece_square];
      // 駒の種類によって分岐。
      switch (Pieceype) {
        case PAWN:
          attack |= Util::GetPawnAttack(piece_square, side);
          break;
        case KNIGHT:
          attack |= Util::GetKnightMove(piece_square);
          break;
        case BISHOP:
          attack |= GetBishopAttack(piece_square);
          break;
        case ROOK:
          attack |= GetRookAttack(piece_square);
          break;
        case QUEEN:
          attack |= GetQueenAttack(piece_square);
          break;
        case KING:
          attack |= Util::GetKingMove(piece_square);
          break;
      }
    }

    return attack;
  }
  // パスポーンの位置のビットボードを得る。
  Bitboard ChessEngine::GetPassPawns(Side side) const {
    // どちらのサイドでもなければ空のビットボードを返す。
    if (side == NO_SIDE) return 0;

    // 相手のサイドを得る。
    Side enemy_side = side ^ 0x3;

    // ポーンのビットボード。
    Bitboard pawns = position_[side][PAWN];

    // パスポーンの位置のビットボード。
    Bitboard pass_pawns = 0;

    Square square;
    for (; pawns; pawns &= pawns - 1) {
      square = Util::GetSquare(pawns);

      // 敵のポーンが判定内にいなければパスポーン。
      if (!(position_[enemy_side][PAWN] & pass_pawn_mask_[side][square])) {
        pass_pawns |= Util::BIT[square];
      }
    }

    return pass_pawns;
  }
  // ダブルポーンの位置のビットボードを得る。
  Bitboard ChessEngine::GetDoublePawns(Side side) const {
    // どちらのサイドでもなければ空を返す。
    if (side == NO_SIDE) return 0;

    // ダブルポーンを得る。
    Bitboard double_pawns = 0;
    for (int fyle = 0; fyle < NUM_FYLES; fyle++) {
      if (Util::CountBits(position_[side][PAWN] & Util::FYLE[fyle])
      >= 2) {
        double_pawns |= position_[side][PAWN] & Util::FYLE[fyle];
      }
    }

    return double_pawns;
  }
  // 孤立ポーンの位置のビットボードを得る。
  Bitboard ChessEngine::GetIsoPawns(Side side) const {
    // サイドがどちらでもないなら空を返す。
    if (side == NO_SIDE) return 0;

    // ポーンのビットボード。
    Bitboard pawns = position_[side][PAWN];

    // 孤立ポーン。
    Bitboard iso_pawns = 0;

    // 孤立ポーンを調べる。
    Square square;
    for (; pawns; pawns &= pawns - 1) {
      square = Util::GetSquare(pawns);
      if (!(position_[side][PAWN] & iso_pawn_mask_[square])) {
        iso_pawns |= Util::BIT[square];
      }
    }

    return iso_pawns;
  }
  // 展開されていないマイナーピースの位置のビットボードを得る。
  Bitboard ChessEngine::GetNotDevelopedMinorPieces(Side side) const {
    // どちらのサイドでもなければ空を返す。
    if (side == NO_SIDE) return 0;

    // 展開されていないマイナーピースを得る。
    Bitboard pieces;
    if (side == WHITE) {
      pieces = (position_[WHITE][KNIGHT] & (B1 | G1))
      | (position_[WHITE][BISHOP] & (C1 | F1));
    } else {
      pieces = (position_[BLACK][KNIGHT] & (B8 | G8))
      | (position_[BLACK][BISHOP] & (C8 | F8));
    }

    return pieces;
  }

  // パスポーンを判定するときに使用するマスク。
  Bitboard ChessEngine::pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
  // pass_pawn_mask_[][]を初期化する。
  void ChessEngine::InitPassPawnMask() {
    Bitboard mask;  // マスク。
    Fyle fyle;  // その位置のファイル。
    Bitboard bitboard;  // 自分より手前のビットボード。
    // マスクを作って初期化する。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int square = 0; square < NUM_SQUARES; square++) {
        mask = 0;
        if (side == NO_SIDE) {  // どちらのサイドでもなければ0。
          pass_pawn_mask_[side][square] = 0;
        } else {
          // 自分のファイルと隣のファイルのマスクを作る。
          fyle = Util::GetFyle(square);
          mask |= Util::FYLE[fyle];
          if (fyle == FYLE_A) {  // aファイルのときはbファイルが隣り。
            mask |= Util::FYLE[fyle + 1];
          } else if (fyle == FYLE_H) {  // hファイルのときはgファイルが隣り。
            mask |= Util::FYLE[fyle - 1];
          } else {  // それ以外のときは両隣。
            mask |= Util::FYLE[fyle + 1];
            mask |= Util::FYLE[fyle - 1];
          }

          // 自分の位置より手前のランクは消す。
          if (side == WHITE) {
            bitboard = (Util::BIT[square] - 1)
            | Util::RANK[Util::GetRank(square)];
            mask &= ~bitboard;
          } else {
            bitboard = ~(Util::BIT[square] - 1)
            | Util::RANK[Util::GetRank(square)];
            mask &= ~bitboard;
          }

          // マスクをセット。
          pass_pawn_mask_[side][square] = mask;
        }
      }
    }
  }

  // 孤立ポーンを判定するときに使用するマスク。
  Bitboard ChessEngine::iso_pawn_mask_[NUM_SQUARES];
  // iso_pawn_mask_[]を初期化する。
  void ChessEngine::InitIsoPawnMask() {
    Fyle fyle;
    for (int square = 0; square < NUM_SQUARES; square++) {
      fyle = Util::GetFyle(square);
      if (fyle == FYLE_A) {
        iso_pawn_mask_[square] = Util::FYLE[fyle + 1];
      } else if (fyle == FYLE_H) {
        iso_pawn_mask_[square] = Util::FYLE[fyle - 1];
      } else {
        iso_pawn_mask_[square] =
        Util::FYLE[fyle + 1] | Util::FYLE[fyle - 1];
      }
    }
  }

  // ポーンの盾の位置のマスク。
  Bitboard ChessEngine::pawn_shield_mask_[NUM_SIDES][NUM_SQUARES];
  // pawn_shield_mask_[][]を初期化する。
  void ChessEngine::InitPawnShieldMask() {
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int square = 0; square < NUM_SQUARES; square++) {
        if (side == NO_SIDE) {  // どちらのサイドでもなければ空。
          pawn_shield_mask_[side][square] = 0;
        } else {
          // 第1ランクのキングサイドとクイーンサイドのとき
          // ポーンの盾の位置を記録する。
          if ((side == WHITE)
          && ((square == A1) || (square == B1) || (square == C1))) {
            pawn_shield_mask_[side][square] =
            Util::BIT[A2] | Util::BIT[B2] | Util::BIT[C2];
          } else if ((side == WHITE)
          && ((square == F1) || (square == G1) || (square == H1))) {
            pawn_shield_mask_[side][square] =
            Util::BIT[F2] | Util::BIT[G2] | Util::BIT[H2];
          } else if ((side == BLACK)
          && ((square == A8) || (square == B8) || (square == C8))) {
            pawn_shield_mask_[side][square] =
            Util::BIT[A7] | Util::BIT[B7] | Util::BIT[C7];
          } else if ((side == BLACK)
          && ((square == F8) || (square == G8) || (square == H8))) {
            pawn_shield_mask_[side][square] =
            Util::BIT[F7] | Util::BIT[G7] | Util::BIT[H7];
          } else {  // キングサイドでもクイーンサイドでもない。
            pawn_shield_mask_[side][square] = 0;
          }
        }
      }
    }
  }
}  // namespace Sayuri
