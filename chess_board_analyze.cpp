/* chess_board_analyze.cpp: チェスボード分析用ツール。
   copyright (c) 2011 石橋宏之利
 */

#include "chess_board.h"

#include <iostream>
#include "chess_def.h"
#include "chess_util.h"

namespace Misaki {
  /******************************
   * 位置ごとの数値のテーブル。 *
   ******************************/
  // ポーンの配置のテーブル。
  const int ChessBoard::pawn_position_table_[NUM_SQUARES] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    0, 0, 0, 0, 0, 0, 0, 0
  };
  // ナイトの配置のテーブル。
  const int ChessBoard::knight_position_table_[NUM_SQUARES] = {
    -3, -2, -1, -1, -1, -1, -2, -3,
    -2, -1,  0,  0,  0,  0, -1, -2,
    -1,  0,  1,  1,  1,  1,  0, -1,
     0,  1,  2,  2,  2,  2,  1,  0,
     1,  2,  3,  3,  3,  3,  2,  1,
     2,  3,  4,  4,  4,  4,  3,  2,
     1,  2,  3,  3,  3,  3,  2,  1,
     0,  1,  2,  2,  2,  2,  1,  0
  };
  // キングの中盤の配置のテーブル。
  const int ChessBoard::king_position_table_middle_[NUM_SQUARES] = {
     1,  1,  0, -1, -1,  0,  1,  1,
     0,  0, -1, -2, -2, -1,  0,  0,
    -1, -1, -2, -3, -3, -2, -1, -1,
    -2, -2, -3, -4, -4, -3, -2, -2,
    -2, -2, -3, -4, -4, -3, -2, -2,
    -1, -1, -2, -3, -3, -2, -1, -1,
     0,  0, -1, -2, -2, -1,  0,  0,
     1,  1,  0, -1, -1,  0,  1,  1
  };
  // キングの終盤の配置のテーブル。
  const int ChessBoard::king_position_table_ending_[NUM_SQUARES] = {
    0, 1, 2, 3, 3, 2, 1, 0,
    1, 2, 3, 4, 4, 3, 2, 1,
    2, 3, 4, 5, 5, 4, 3, 2,
    3, 4, 5, 6, 6, 5, 4, 3,
    3, 4, 5, 6, 6, 5, 4, 3,
    2, 3, 4, 5, 5, 4, 3, 2,
    1, 2, 3, 4, 4, 3, 2, 1,
    0, 1, 2, 3, 3, 2, 1, 0
  };
  // テーブルを計算する関数。
  int ChessBoard::GetTableValue(const int (& table)[NUM_SQUARES],
  side_t side, bitboard_t bitboard) {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // ボードを鏡対象に上下反転させる配列。
    // <配列>[flip[<位置>]]と使うと上下が反転する。
    static const square_t flip[NUM_SQUARES] = {
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
    square_t square;

    // 値。
    int value = 0;

    // 配列を足す。
    if (side == WHITE) {
      for (; bitboard; bitboard &= bitboard - 1) {
        square = ChessUtil::GetSquare(bitboard);
        value += table[square];
      }
    } else {
      for (; bitboard; bitboard &= bitboard - 1) {
        square = ChessUtil::GetSquare(bitboard);
        value += table[flip[square]];
      }
    }

    // 値を返す。
    return value;
  }

  /************************
   * 局面を分析する関数。 *
   ************************/
  // 勝つのに十分な駒があるかどうか調べる。
  bool ChessBoard::IsEnoughPieces(side_t side) const {
    // サイドを確認。
    if (side == NO_SIDE) return false;

    // ポーンがあれば大丈夫。
    if (position_[side][PAWN]) return true;

    // ルークがあれば大丈夫。
    if (position_[side][ROOK]) return true;

    // クイーンがあれば大丈夫。
    if (position_[side][QUEEN]) return true;

    // ビショップが2つあれば大丈夫。
    if (ChessUtil::CountBits(position_[side][BISHOP]) >= 2) return true;

    // ナイトが2つあれば大丈夫。
    if (ChessUtil::CountBits(position_[side][KNIGHT]) >= 2) return true;

    // ナイトとビショップの合計が2つあれば大丈夫。
    if (ChessUtil::CountBits(position_[side][KNIGHT]
    | position_[side][BISHOP]) >= 2)
      return true;

    // それ以外はダメ。
    return false;
  }
  // 動ける位置の数を得る。
  int ChessBoard::GetMobility(square_t piece_square) const {
    // 駒の種類を得る。
    piece_t piece_type = piece_board_[piece_square];

    // 駒のサイドと敵のサイドを得る。
    side_t side = side_board_[piece_square];
    side_t enemy_side = static_cast<side_t>(side ^ 0x3);

    // 駒がなければ0を返す。
    if (!piece_type) return 0;

    // 利き筋を入れる。
    bitboard_t move_bitboard;
    switch (piece_type) {
      case PAWN:
        // 通常の動き。
        move_bitboard = ChessUtil::GetPawnMove(piece_square, side) & ~blocker0_;
        // 2歩の動き。
        if (move_bitboard) {
          move_bitboard |= ChessUtil::GetPawn2StepMove(piece_square, side)
          & ~blocker0_;
        }
        // 攻撃。
        move_bitboard |= ChessUtil::GetPawnAttack(piece_square, side)
        & side_pieces_[enemy_side];
        // アンパッサン。
        if (can_en_passant_) {
          if (side_board_[en_passant_target_] != side) {
            rank_t attacker_rank = ChessUtil::GetRank(piece_square);
            rank_t target_rank = ChessUtil::GetRank(en_passant_target_);
            if (attacker_rank == target_rank) {
              if ((piece_square == (en_passant_target_ + 1))
              || (piece_square == (en_passant_target_ - 1))) {
                move_bitboard |= (side == WHITE ?
                ChessUtil::BIT[en_passant_target_ + 8]
                : ChessUtil::BIT[en_passant_target_ - 8]);
              }
            }
          }
        }
        break;
      case KNIGHT:
        move_bitboard = ChessUtil::GetKnightMove(piece_square)
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
        move_bitboard = ChessUtil::GetKingMove(piece_square)
        & ~side_pieces_[side];
        // キャスリング。
        if ((side == WHITE) && (piece_square == E1)) {  // 白。
          // ショートキャスリング。
          if (castling_rights_ & WHITE_SHORT_CASTLING) {
            if (!IsAttacked(E1, enemy_side)
            && !IsAttacked(F1, enemy_side)
            && !IsAttacked(G1, enemy_side)) {
              if (!piece_board_[F1] && !piece_board_[G1]) {
                move_bitboard |= ChessUtil::BIT[G1];
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
                move_bitboard |= ChessUtil::BIT[C1];
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
                move_bitboard |= ChessUtil::BIT[G8];
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
                move_bitboard |= ChessUtil::BIT[C8];
              }
            }
          }
        }
        break;
    }

    // ビットの数を数えて返す。
    return ChessUtil::CountBits(move_bitboard);
  }
  // 攻撃している位置のビットボードを得る。
  bitboard_t ChessBoard::GetAttack(bitboard_t pieces) const {
    // 駒を整理。
    pieces &= blocker0_;

    // 攻撃位置。
    bitboard_t attack = 0;

    // 駒を1つずつ調べていく。
    square_t piece_square;
    piece_t piece_type;
    side_t side;
    for (; pieces; pieces &= pieces - 1) {
      piece_square = ChessUtil::GetSquare(pieces);
      piece_type = piece_board_[piece_square];
      side = side_board_[piece_square];
      // 駒の種類によって分岐。
      switch (piece_type) {
        case PAWN:
          attack |= ChessUtil::GetPawnAttack(piece_square, side);
          break;
        case KNIGHT:
          attack |= ChessUtil::GetKnightMove(piece_square);
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
          attack |= ChessUtil::GetKingMove(piece_square);
          break;
      }
    }

    return attack;
  }
  // パスポーンの位置のビットボードを得る。
  bitboard_t ChessBoard::GetPassPawns(side_t side) const {
    // どちらのサイドでもなければ空のビットボードを返す。
    if (side == NO_SIDE) return 0;

    // 相手のサイドを得る。
    side_t enemy_side = static_cast<side_t>(side ^ 0x3);

    // ポーンのビットボード。
    bitboard_t pawns = position_[side][PAWN];

    // パスポーンの位置のビットボード。
    bitboard_t pass_pawns = 0;

    square_t square;
    for (; pawns; pawns &= pawns - 1) {
      square = ChessUtil::GetSquare(pawns);

      // 敵のポーンが判定内にいなければパスポーン。
      if (!(position_[enemy_side][PAWN] & pass_pawn_mask_[side][square])) {
        pass_pawns |= ChessUtil::BIT[square];
      }
    }

    return pass_pawns;
  }
  // ダブルポーンの位置のビットボードを得る。
  bitboard_t ChessBoard::GetDoublePawns(side_t side) const {
    // どちらのサイドでもなければ空を返す。
    if (side == NO_SIDE) return 0;

    // ダブルポーンを得る。
    bitboard_t double_pawns = 0;
    for (int fyle = 0; fyle < NUM_FYLES; fyle++) {
      if (ChessUtil::CountBits(position_[side][PAWN] & ChessUtil::FYLE[fyle])
      >= 2) {
        double_pawns |= position_[side][PAWN] & ChessUtil::FYLE[fyle];
      }
    }

    return double_pawns;
  }
  // 孤立ポーンの位置のビットボードを得る。
  bitboard_t ChessBoard::GetIsoPawns(side_t side) const {
    // サイドがどちらでもないなら空を返す。
    if (side == NO_SIDE) return 0;

    // ポーンのビットボード。
    bitboard_t pawns = position_[side][PAWN];

    // 孤立ポーン。
    bitboard_t iso_pawns = 0;

    // 孤立ポーンを調べる。
    square_t square;
    for (; pawns; pawns &= pawns - 1) {
      square = ChessUtil::GetSquare(pawns);
      if (!(position_[side][PAWN] & iso_pawn_mask_[square])) {
        iso_pawns |= ChessUtil::BIT[square];
      }
    }

    return iso_pawns;
  }
  // 展開されていないマイナーピースの位置のビットボードを得る。
  bitboard_t ChessBoard::GetNotDevelopedMinorPieces(side_t side) const {
    // どちらのサイドでもなければ空を返す。
    if (side == NO_SIDE) return 0;

    // 展開されていないマイナーピースを得る。
    bitboard_t pieces;
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
  bitboard_t ChessBoard::pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
  // pass_pawn_mask_[][]を初期化する。
  void ChessBoard::InitPassPawnMask() {
    bitboard_t mask;  // マスク。
    fyle_t fyle;  // その位置のファイル。
    bitboard_t bitboard;  // 自分より手前のビットボード。
    // マスクを作って初期化する。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int square = 0; square < NUM_SQUARES; square++) {
        mask = 0;
        if (side == NO_SIDE) {  // どちらのサイドでもなければ0。
          pass_pawn_mask_[side][square] = 0;
        } else {
          // 自分のファイルと隣のファイルのマスクを作る。
          fyle = ChessUtil::GetFyle(static_cast<square_t>(square));
          mask |= ChessUtil::FYLE[fyle];
          if (fyle == FYLE_A) {  // aファイルのときはbファイルが隣り。
            mask |= ChessUtil::FYLE[fyle + 1];
          } else if (fyle == FYLE_H) {  // hファイルのときはgファイルが隣り。
            mask |= ChessUtil::FYLE[fyle - 1];
          } else {  // それ以外のときは両隣。
            mask |= ChessUtil::FYLE[fyle + 1];
            mask |= ChessUtil::FYLE[fyle - 1];
          }

          // 自分の位置より手前のランクは消す。
          if (side == WHITE) {
            bitboard = (ChessUtil::BIT[square] - 1)
            | ChessUtil::RANK[ChessUtil::GetRank(static_cast<square_t>
            (square))];
            mask &= ~bitboard;
          } else {
            bitboard = ~(ChessUtil::BIT[square] - 1)
            | ChessUtil::RANK[ChessUtil::GetRank(static_cast<square_t>
            (square))];
            mask &= ~bitboard;
          }

          // マスクをセット。
          pass_pawn_mask_[side][square] = mask;
        }
      }
    }
  }

  // 孤立ポーンを判定するときに使用するマスク。
  bitboard_t ChessBoard::iso_pawn_mask_[NUM_SQUARES];
  // iso_pawn_mask_[]を初期化する。
  void ChessBoard::InitIsoPawnMask() {
    fyle_t fyle;
    for (int square = 0; square < NUM_SQUARES; square++) {
      fyle = ChessUtil::GetFyle(static_cast<square_t>(square));
      if (fyle == FYLE_A) {
        iso_pawn_mask_[square] = ChessUtil::FYLE[fyle + 1];
      } else if (fyle == FYLE_H) {
        iso_pawn_mask_[square] = ChessUtil::FYLE[fyle - 1];
      } else {
        iso_pawn_mask_[square] =
        ChessUtil::FYLE[fyle + 1] | ChessUtil::FYLE[fyle - 1];
      }
    }
  }

  // ポーンの盾の位置のマスク。
  bitboard_t ChessBoard::pawn_shield_mask_[NUM_SIDES][NUM_SQUARES];
  // pawn_shield_mask_[][]を初期化する。
  void ChessBoard::InitPawnShieldMask() {
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
            ChessUtil::BIT[A2] | ChessUtil::BIT[B2] | ChessUtil::BIT[C2];
          } else if ((side == WHITE)
          && ((square == F1) || (square == G1) || (square == H1))) {
            pawn_shield_mask_[side][square] =
            ChessUtil::BIT[F2] | ChessUtil::BIT[G2] | ChessUtil::BIT[H2];
          } else if ((side == BLACK)
          && ((square == A8) || (square == B8) || (square == C8))) {
            pawn_shield_mask_[side][square] =
            ChessUtil::BIT[A7] | ChessUtil::BIT[B7] | ChessUtil::BIT[C7];
          } else if ((side == BLACK)
          && ((square == F8) || (square == G8) || (square == H8))) {
            pawn_shield_mask_[side][square] =
            ChessUtil::BIT[F7] | ChessUtil::BIT[G7] | ChessUtil::BIT[H7];
          } else {  // キングサイドでもクイーンサイドでもない。
            pawn_shield_mask_[side][square] = 0;
          }
        }
      }
    }
  }
}  // Misaki
