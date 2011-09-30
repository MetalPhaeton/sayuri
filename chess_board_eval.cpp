/* chess_board_eval.cpp: チェスボードの静的評価。
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

namespace Misaki {
  /************************
   * 局面を評価する関数。 *
   ************************/
  // 全てを評価する。
  int ChessBoard::EvalAll(side_t side, const EvalWeights& weights) const {
    //サイドを整理する。
    if (side == NO_SIDE) return 0;

    // 相手のサイド。
    side_t enemy_side = static_cast<side_t>(side ^ 0x3);

    // チェックメイトされていれば負けの評価。
    if (IsCheckmated()) return SCORE_LOSE;

    // ステールメイトなら引き分けの評価。
    if (IsStalemated()) return SCORE_DRAW;

    // 両者に十分な駒がなければ引分。
    if (!IsEnoughPieces(side) && !IsEnoughPieces(enemy_side))
      return SCORE_DRAW;

    // それ以外の評価値。
    int score = GetMaterial(side);
    score += EvalMobility(side, weights);
    score += EvalPawnPosition(side, weights);
    score += EvalKnightPosition(side, weights);
    score += EvalPassPawn(side, weights);
    score += EvalDoublePawn(side, weights);
    score += EvalIsoPawn(side, weights);
    score += EvalBishopPair(side, weights);
    score += EvalCanceledCastling(side, weights);
    if (IsEnding()) {  // 終盤の場合。
      score += EvalKingPositionEnding(side, weights);
    } else {  // 序中盤の場合。
      score += EvalAttackCenter(side, weights);
      score += EvalDevelopment(side, weights);
      score += EvalAttackAroundKing(side, weights);
      score += EvalKingPositionMiddle(side, weights);
      score += EvalRook7th(side, weights);
      score += EvalEarlyQueenLaunched(side, weights);
      score += EvalPawnShield(side, weights);
      score += EvalEarlyKingLaunched(side, weights);
    }

    return score;
  }
  // 機動力を評価する。
  int ChessBoard::EvalMobility(side_t side, const EvalWeights& weights) const {
    // サイドがなければ0点。
    if (side == NO_SIDE) return 0;

    // 駒の位置。
    square_t piece_square;

    // 白のモビリティを得る。
    bitboard_t white_pieces = side_pieces_[WHITE];
    int white_mobility = 0;
    for (; white_pieces; white_pieces &= white_pieces - 1) {
      piece_square = ChessUtil::GetSquare(white_pieces);
      white_mobility += GetMobility(piece_square);
    }

    // 黒のモビリティを得る。
    bitboard_t black_pieces = side_pieces_[BLACK];
    int black_mobility = 0;
    for (; black_pieces; black_pieces &= black_pieces - 1) {
      piece_square = ChessUtil::GetSquare(black_pieces);
      black_mobility += GetMobility(piece_square);
    }

    // 点数を計算して返す。
    int score = (white_mobility - black_mobility) * weights.mobility_weight_;
    return side == WHITE ? score : -score;
  }
  // センター攻撃を評価する。
  int ChessBoard::EvalAttackCenter(side_t side, const EvalWeights& weights)
  const {
    // サイドがなければ0点。
    if (side == NO_SIDE) return 0;

    // 駒の位置。
    square_t piece_square;

    // ビットボード。
    bitboard_t bitboard;

    // 各サイドの価値。
    int white_value = 0;
    int black_value = 0;

    // 白の大中央を攻撃している駒を得る。
    bitboard_t white_attackers = GetAttackers(D4, WHITE);
    white_attackers |= GetAttackers(D5, WHITE);
    white_attackers |= GetAttackers(E4, WHITE);
    white_attackers |= GetAttackers(E5, WHITE);

    // 黒の大中央を攻撃している駒を得る。
    bitboard_t black_attackers = GetAttackers(D4, BLACK);
    black_attackers |= GetAttackers(D5, BLACK);
    black_attackers |= GetAttackers(E4, BLACK);
    black_attackers |= GetAttackers(E5, BLACK);

    // 各サイドの価値を計算する。
    white_value += ChessUtil::CountBits(white_attackers) * 2;
    black_value += ChessUtil::CountBits(black_attackers) * 2;

    // 白の小中央を攻撃している駒を得る。
    white_attackers = GetAttackers(C3, WHITE);
    white_attackers |= GetAttackers(C4, WHITE);
    white_attackers |= GetAttackers(C5, WHITE);
    white_attackers |= GetAttackers(C6, WHITE);
    white_attackers |= GetAttackers(D3, WHITE);
    white_attackers |= GetAttackers(D4, WHITE);
    white_attackers |= GetAttackers(D5, WHITE);
    white_attackers |= GetAttackers(D6, WHITE);
    white_attackers |= GetAttackers(E3, WHITE);
    white_attackers |= GetAttackers(E4, WHITE);
    white_attackers |= GetAttackers(E5, WHITE);
    white_attackers |= GetAttackers(E6, WHITE);
    white_attackers |= GetAttackers(F3, WHITE);
    white_attackers |= GetAttackers(F4, WHITE);
    white_attackers |= GetAttackers(F5, WHITE);
    white_attackers |= GetAttackers(F6, WHITE);

    // 黒の小中央を攻撃している駒を得る。
    black_attackers = GetAttackers(C3, BLACK);
    black_attackers |= GetAttackers(C4, BLACK);
    black_attackers |= GetAttackers(C5, BLACK);
    black_attackers |= GetAttackers(C6, BLACK);
    black_attackers |= GetAttackers(D3, BLACK);
    black_attackers |= GetAttackers(D4, BLACK);
    black_attackers |= GetAttackers(D5, BLACK);
    black_attackers |= GetAttackers(D6, BLACK);
    black_attackers |= GetAttackers(E3, BLACK);
    black_attackers |= GetAttackers(E4, BLACK);
    black_attackers |= GetAttackers(E5, BLACK);
    black_attackers |= GetAttackers(E6, BLACK);
    black_attackers |= GetAttackers(F3, BLACK);
    black_attackers |= GetAttackers(F4, BLACK);
    black_attackers |= GetAttackers(F5, BLACK);
    black_attackers |= GetAttackers(F6, BLACK);

    // 各サイドの価値を計算する。
    white_value += ChessUtil::CountBits(white_attackers);
    black_value += ChessUtil::CountBits(black_attackers);

    // 評価値を計算して返す。
    int score = (white_value - black_value) * weights.attack_center_weight_;
    return side == WHITE ? score : -score;
  }
  // 展開の遅さを評価する。
  int ChessBoard::EvalDevelopment(side_t side, const EvalWeights& weights)
  const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 各サイドの展開できていないマイナーピースの数。
    int white_count = ChessUtil::CountBits(GetNotDevelopedMinorPieces(WHITE));
    int black_count = ChessUtil::CountBits(GetNotDevelopedMinorPieces(BLACK));

    // 評価値にして返す。
    int score = (white_count - black_count) * -weights.development_weight_;
    return side == WHITE ? score : -score;
  }
  // キングの周囲への攻撃を評価する。
  int ChessBoard::EvalAttackAroundKing(side_t side, const EvalWeights& weights)
  const {
    // サイドがなければ0点。
    if (side == NO_SIDE) return 0;

    // それぞれのキング周りへの攻撃を得る。
    bitboard_t white_attack = GetAttack(side_pieces_[WHITE])
    & ChessUtil::GetKingMove(king_[BLACK]);
    bitboard_t black_attack = GetAttack(side_pieces_[BLACK])
    & ChessUtil::GetKingMove(king_[WHITE]);

    // 攻撃位置を数える。
    int white_attack_count = ChessUtil::CountBits(white_attack);
    int black_attack_count = ChessUtil::CountBits(black_attack);

    // 点数にして返す。
    int score = (white_attack_count - black_attack_count)
    * weights.attack_around_king_weight_;
    return side == WHITE ? score : -score;
  }
  // ポーンの配置を評価する。
  int ChessBoard::EvalPawnPosition(side_t side, const EvalWeights& weights)
  const {
    // サイドがなければ0点。
    if (side == NO_SIDE) return 0;

    // 白の価値を得る。
    int white_value = GetPawnPositionValue(WHITE);

    // 黒の価値を得る。
    int black_value = GetPawnPositionValue(BLACK);

    // 点数を計算して返す。
    int score = (white_value - black_value) * weights.pawn_position_weight_;
    return side == WHITE ? score: -score;
  }
  // ナイトの配置を評価する。
  int ChessBoard::EvalKnightPosition(side_t side, const EvalWeights& weights)
  const {
    // サイドがなければ0点。
    if (side == NO_SIDE) return 0;

    // 白の価値を得る。
    int white_value = GetKnightPositionValue(WHITE);

    // 黒の価値を得る。
    int black_value = GetKnightPositionValue(BLACK);

    // 点数を計算して返す。
    int score = (white_value - black_value) * weights.knight_position_weight_;
    return side == WHITE ? score: -score;
  }
  // キングの中盤の配置を評価する。
  int ChessBoard::EvalKingPositionMiddle(side_t side,
  const EvalWeights& weights) const {
    // サイドがなければ0点。
    if (side == NO_SIDE) return 0;

    // 白の価値を得る。
    int white_value = GetKingPositionMiddleValue(WHITE);

    // 黒の価値を得る。
    int black_value = GetKingPositionMiddleValue(BLACK);

    // 点数を計算して返す。
    int score = (white_value - black_value)
    * weights.king_position_middle_weight_;
    return side == WHITE ? score: -score;
  }
  // キングの終盤の配置を評価する。
  int ChessBoard::EvalKingPositionEnding(side_t side,
  const EvalWeights& weights) const {
    // サイドがなければ0点。
    if (side == NO_SIDE) return 0;

    // 白の価値を得る。
    int white_value = GetKingPositionEndingValue(WHITE);

    // 黒の価値を得る。
    int black_value = GetKingPositionEndingValue(BLACK);

    // 点数を計算して返す。
    int score = (white_value - black_value)
    * weights.king_position_ending_weight_;
    return side == WHITE ? score: -score;
  }
  // パスポーンを評価する。
  int ChessBoard::EvalPassPawn(side_t side, const EvalWeights& weights) const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 両サイドのパスポーンを得る。
    bitboard_t white_pass_pawns = GetPassPawns(WHITE);
    bitboard_t black_pass_pawns = GetPassPawns(BLACK);

    // パスポーンを評価値にする。
    int white_score = ChessUtil::CountBits(white_pass_pawns)
    * weights.pass_pawn_weight_;
    int black_score = ChessUtil::CountBits(black_pass_pawns)
    * weights.pass_pawn_weight_;

    // マス。
    square_t square;

    // 守られたパスポーンにはボーナス。
    // 白を調べる。
    for (; white_pass_pawns; white_pass_pawns &= white_pass_pawns - 1) {
      square = ChessUtil::GetSquare(white_pass_pawns);
      // 守られていればボーナスを追加。
      if (position_[WHITE][PAWN] & ChessUtil::GetPawnAttack(square, BLACK)) {
        white_score += weights.protected_pass_pawn_weight_;
      }
    }
    // 黒を調べる。
    for (; black_pass_pawns; black_pass_pawns &= black_pass_pawns - 1) {
      square = ChessUtil::GetSquare(black_pass_pawns);
      // 守られていればボーナスを追加。
      if (position_[WHITE][PAWN] & ChessUtil::GetPawnAttack(square, WHITE)) {
        black_score += weights.protected_pass_pawn_weight_;
      }
    }

    // 得点にして返す。
    int score = white_score - black_score;
    return side == WHITE ? score : -score;
  }
  // ダブルポーンを評価する。
  int ChessBoard::EvalDoublePawn(side_t side, const EvalWeights& weights)
  const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 各サイドのダブルポーンの数を得る。
    int white_count = ChessUtil::CountBits(GetDoublePawns(WHITE));
    int black_count = ChessUtil::CountBits(GetDoublePawns(BLACK));

    // 得点にして返す。
    int score = (white_count - black_count) * weights.double_pawn_weight_;
    return side == WHITE ? score : -score;
  }
  // 孤立ポーンを評価する。
  int ChessBoard::EvalIsoPawn(side_t side, const EvalWeights& weights) const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 各サイドの孤立ポーンの数を得る。
    int white_count = ChessUtil::CountBits(GetIsoPawns(WHITE));
    int black_count = ChessUtil::CountBits(GetIsoPawns(BLACK));

    // 得点にして返す。
    int score = (white_count - black_count) * weights.iso_pawn_weight_;
    return side == WHITE ? score : -score;
  }
  // ビショップペアを評価する。
  int ChessBoard::EvalBishopPair(side_t side, const EvalWeights& weights)
  const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 各サイドの得点。
    int white_score = 0;
    int black_score = 0;

    // ビショップが2個以上所有していればボーナス。
    if (ChessUtil::CountBits(position_[WHITE][BISHOP]) >= 2) {
      white_score += weights.bishop_pair_weight_;
    }
    if (ChessUtil::CountBits(position_[BLACK][BISHOP]) >= 2) {
      black_score += weights.bishop_pair_weight_;
    }

    // 得点にして返す。
    int score = white_score - black_score;
    return side == WHITE ? score : -score;
  }
  // 第7ランクのルークを評価する。
  int ChessBoard::EvalRook7th(side_t side, const EvalWeights& weights) const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 各サイドの第7ランクのルークの数を得る。
    int white_count = ChessUtil::CountBits(position_[WHITE][ROOK]
    & ChessUtil::RANK[RANK_7]);
    int black_count = ChessUtil::CountBits(position_[BLACK][ROOK]
    & ChessUtil::RANK[RANK_2]);

    // 得点にして返す。
    int score = (white_count - black_count) * weights.rook_7th_weight_;
    return side == WHITE ? score : -score;
  }
  // 早すぎるクイーンの出動を評価する。
  int ChessBoard::EvalEarlyQueenLaunched(side_t side,
  const EvalWeights& weights) const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 各サイドの得点。
    int white_score = 0;
    int black_score = 0;
    if (!(position_[WHITE][QUEEN] & D1)) {
      white_score = ChessUtil::CountBits(GetNotDevelopedMinorPieces(WHITE))
      * weights.early_queen_launched_weight_;
    }
    if (!(position_[BLACK][QUEEN] & D8)) {
      black_score = ChessUtil::CountBits(GetNotDevelopedMinorPieces(BLACK))
      * weights.early_queen_launched_weight_;
    }

    // 得点にして返す。
    int score = white_score - black_score;
    return side == WHITE ? score : -score;
  }
  // ポーンの盾を評価する。
  int ChessBoard::EvalPawnShield(side_t side, const EvalWeights& weights)
  const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 各サイドのポーンの盾の数を得る。
    int white_count = ChessUtil::CountBits(GetPawnShield(WHITE));
    int black_count = ChessUtil::CountBits(GetPawnShield(BLACK));

    // 得点にして返す。
    int score = (white_count - black_count) * weights.pawn_shield_weight_;
    return side == WHITE ? score : -score;
  }
  // 早すぎるキングの出動を評価する。
  int ChessBoard::EvalEarlyKingLaunched(side_t side,
  const EvalWeights& weights) const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 各サイドの得点。
    int white_score = 0;
    int black_score = 0;
    if (king_[WHITE] != E1) {
      white_score = ChessUtil::CountBits(GetNotDevelopedMinorPieces(WHITE))
      * weights.early_king_launched_weight_;
    }
    if (king_[BLACK] != E8) {
      black_score = ChessUtil::CountBits(GetNotDevelopedMinorPieces(BLACK))
      * weights.early_king_launched_weight_;
    }

    // 得点にして返す。
    int score = white_score - black_score;
    return side == WHITE ? score : -score;
  }
  // キャスリングの破棄を評価する。
  int ChessBoard::EvalCanceledCastling(side_t side, const EvalWeights& weights)
  const {
    // どちらのサイドでもなければ0点。
    if (side == NO_SIDE) return 0;

    // 各サイドの得点。
    int white_score = 0;
    int black_score = 0;
    if ((!(castling_rights_ & WHITE_CASTLING)) && (!has_white_castled_)) {
      white_score += weights.canceled_castling_weight_;
    }
    if ((!(castling_rights_ & BLACK_CASTLING)) && (!has_black_castled_)) {
      white_score += weights.canceled_castling_weight_;
    }

    // 得点にして返す。
    int score = white_score - black_score;
    return side == WHITE ? score : -score;
  }
}  // Misaki
