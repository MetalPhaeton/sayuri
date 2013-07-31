/*
   evaluator.cpp: 局面を評価するクラスの実装。

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

#include "evaluator.h"

#include <iostream>
#include "error.h"

namespace Sayuri {
  /****************/
  /* static定数。 */
  /****************/
  constexpr int Evaluator::PAWN_POSITION_TABLE[NUM_SQUARES];
  constexpr int Evaluator::KNIGHT_POSITION_TABLE[NUM_SQUARES];
  constexpr int Evaluator::ROOK_POSITION_TABLE[NUM_SQUARES];
  constexpr int Evaluator::KING_POSITION_MIDDLE_TABLE[NUM_SQUARES];
  constexpr int Evaluator::KING_POSITION_ENDING_TABLE[NUM_SQUARES];
  constexpr int Evaluator::WEIGHT_MOBILITY;
  constexpr int Evaluator::WEIGHT_ATTACK_CENTER;
  constexpr int Evaluator::WEIGHT_DEVELOPMENT;
  constexpr int Evaluator::WEIGHT_ATTACK_AROUND_KING;
  constexpr int Evaluator::WEIGHT_PAWN_POSITION;
  constexpr int Evaluator::WEIGHT_KNIGHT_POSITION;
  constexpr int Evaluator::WEIGHT_ROOK_POSITION;
  constexpr int Evaluator::WEIGHT_KING_POSITION_MIDDLE;
  constexpr int Evaluator::WEIGHT_KING_POSITION_ENDING;
  constexpr int Evaluator::WEIGHT_PASS_PAWN;
  constexpr int Evaluator::WEIGHT_PROTECTED_PASS_PAWN;
  constexpr int Evaluator::WEIGHT_DOUBLE_PAWN;
  constexpr int Evaluator::WEIGHT_ISO_PAWN;
  constexpr int Evaluator::WEIGHT_BISHOP_PAIR;
  constexpr int Evaluator::WEIGHT_EARLY_QUEEN_LAUNCHED;
  constexpr int Evaluator::WEIGHT_PAWN_SHIELD;
  constexpr int Evaluator::WEIGHT_CASTLING;

  /****************/
  /* static変数。 */
  /****************/
  Bitboard Evaluator::start_position_[NUM_SIDES][NUM_PIECE_TYPES];
  Bitboard Evaluator::center_mask_;
  Bitboard Evaluator::sweet_center_mask_;
  Bitboard Evaluator::pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
  Bitboard Evaluator::iso_pawn_mask_[NUM_SQUARES];
  Bitboard Evaluator::pawn_shield_mask_[NUM_SIDES][NUM_SQUARES];

  /********************/
  /* コンストラクタ。 */
  /********************/
  // コンストラクタ。
  Evaluator::Evaluator(ChessEngine* engine_ptr)
  : engine_ptr_(engine_ptr) {
  }

  // コピーコンストラクタ。
  Evaluator::Evaluator(const Evaluator& eval)
  : engine_ptr_(eval.engine_ptr_) {
  }

  // ムーブコンストラクタ。
  Evaluator::Evaluator(Evaluator&& eval)
  : engine_ptr_(eval.engine_ptr_) {
  }

  // コピー代入。
  Evaluator& Evaluator::operator=(const Evaluator& eval) {
    engine_ptr_ = eval.engine_ptr_;
    return *this;
  }

  // ムーブ代入。
  Evaluator& Evaluator::operator=(Evaluator&& eval) {
    engine_ptr_ = eval.engine_ptr_;
    return *this;
  }

  /********************/
  /* パブリック関数。 */
  /********************/
  // static変数の初期化。
  void Evaluator::InitEvaluator() {
    // 駒の初期位置を初期化。
    InitStartPosition();
    // センターマスクを初期化する。
    InitCenterMask();
    // pass_pawn_mask_[][]を初期化する。
    InitPassPawnMask();
    // iso_pawn_mask_[]を初期化する。
    InitIsoPawnMask();
    // pawn_shield_mask_[][]を初期化する。
    InitPawnShieldMask();
  }

  /****************************/
  /* 局面評価に使用する関数。 */
  /****************************/
  // 勝つのに十分な駒があるかどうか調べる。
  bool Evaluator::HasEnoughPieces(Side side) {
    // ポーンがあれば大丈夫。
    if (engine_ptr_->position_[side][PAWN]) return true;

    // ルークがあれば大丈夫。
    if (engine_ptr_->position_[side][ROOK]) return true;

    // クイーンがあれば大丈夫。
    if (engine_ptr_->position_[side][QUEEN]) return true;

    // ビショップが2つあれば大丈夫。
    if (Util::CountBits(engine_ptr_->position_[side][BISHOP]) >= 2)
      return true;

    // ナイトが2つあれば大丈夫。
    if (Util::CountBits(engine_ptr_->position_[side][KNIGHT]) >= 2)
      return true;

    // ナイトとビショップの合計が2つあれば大丈夫。
    if (Util::CountBits(engine_ptr_->position_[side][KNIGHT]
    | engine_ptr_->position_[side][BISHOP]) >= 2)
      return true;

    // それ以外はダメ。
    return false;
  }

  // 動ける位置の数を得る。
  int Evaluator::GetMobility(Square square) {
    // 駒の種類を得る。
    Piece piece_type = engine_ptr_->piece_board_[square];
    if ((piece_type < PAWN) || (piece_type > KING)) return 0;

    // 駒のサイドと敵のサイドを得る。
    Side side = engine_ptr_->side_board_[square];
    Side enemy_side = side ^ 0x3;

    // 利き筋を入れる。
    Bitboard move_bitboard;
    switch (piece_type) {
      case PAWN:
        // 通常の動き。
        move_bitboard = Util::GetPawnMove(square, side)
        & ~(engine_ptr_->blocker_0_);
        // 2歩の動き。
        if (move_bitboard) {
          if (((side == WHITE) && (Util::GetRank(square) == RANK_2))
          || ((side == BLACK) && (Util::GetRank(square) == RANK_7))) {
            // ポーンの2歩の動き。
            move_bitboard |= Util::GetPawn2StepMove(square, side)
            & ~(engine_ptr_->blocker_0_);
          }
        }
        // 攻撃。
        move_bitboard |= Util::GetPawnAttack(square, side)
        & engine_ptr_->side_pieces_[enemy_side];
        // アンパッサン。
        if (engine_ptr_->can_en_passant_) {
          move_bitboard |= Util::BIT[engine_ptr_->en_passant_square_]
          & Util::GetPawnAttack(square, side);
        }
        break;
      case KNIGHT:
        move_bitboard = Util::GetKnightMove(square)
        & ~(engine_ptr_->side_pieces_[side]);
        break;
      case BISHOP:
        move_bitboard = engine_ptr_->GetBishopAttack(square)
        & ~(engine_ptr_->side_pieces_[side]);
        break;
      case ROOK:
        move_bitboard = engine_ptr_->GetRookAttack(square)
        & ~(engine_ptr_->side_pieces_[side]);
        break;
      case QUEEN:
        move_bitboard = engine_ptr_->GetQueenAttack(square)
        & ~(engine_ptr_->side_pieces_[side]);
        break;
      case KING:
        move_bitboard = Util::GetKingMove(square)
        & ~(engine_ptr_->side_pieces_[side]);
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
        break;
      default:
        throw SayuriError("駒の種類が不正です。");
        break;
    }

    // ビットの数を数えて返す。
    return Util::CountBits(move_bitboard);
  }

  // パスポーンのビットボードを返す。
  Bitboard Evaluator::GetPassPawns(Side side) {
    Assert((side == WHITE) || (side == BLACK));

    // 相手のサイド。
    Side enemy_side = side ^ 0x3;

    // ポーンのビットボード。
    Bitboard pawns = engine_ptr_->position_[side][PAWN];

    // パスポーンを見つける。
    Bitboard pass_pawns = 0ULL;
    Square square;
    for (; pawns; pawns &= pawns - 1) {
      square = Util::GetSquare(pawns);
      if (!(engine_ptr_->position_[enemy_side][PAWN]
      & pass_pawn_mask_[side][square])) {
        pass_pawns |= Util::BIT[square];
      }
    }

    return pass_pawns;
  }

  // ダブルポーンの位置のビットボードを得る。
  Bitboard Evaluator::GetDoublePawns(Side side) {
    // ダブルポーンを得る。
    Bitboard double_pawns = 0ULL;
    for (int fyle = 0; fyle < NUM_FYLES; fyle++) {
      if (Util::CountBits(engine_ptr_->position_[side][PAWN] & Util::FYLE[fyle])
      >= 2) {
        double_pawns |= engine_ptr_->position_[side][PAWN] & Util::FYLE[fyle];
      }
    }

    return double_pawns;
  }

  // 孤立ポーンのビットボードを得る。
  Bitboard Evaluator::GetIsoPawns(Side side) {
    Bitboard pawns = engine_ptr_->position_[side][PAWN];
    Bitboard iso_pawns = 0ULL;
    Square square;
    for (; pawns; pawns &= pawns - 1) {
      square = Util::GetSquare(pawns);
      if (!(engine_ptr_->position_[side][PAWN] & iso_pawn_mask_[square])) {
        iso_pawns |= Util::BIT[square];
      }
    }

    return iso_pawns;
  }

  /************************/
  /* 局面を評価する関数。 */
  /************************/
  // 機動力を評価する。
  int Evaluator::EvalMobility() {

    // 駒の位置。
    Square piece_square;

    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 自分のモビリティを得る。
    Bitboard my_pieces = engine_ptr_->side_pieces_[side];
    int my_mobility = 0;
    for (; my_pieces; my_pieces &= my_pieces - 1) {
      piece_square = Util::GetSquare(my_pieces);
      my_mobility += GetMobility(piece_square);
    }

    // 黒のモビリティを得る。
    // 相手のモビリティを得る。
    Bitboard enemy_pieces = engine_ptr_->side_pieces_[enemy_side];
    int enemy_mobility = 0;
    for (; enemy_pieces; enemy_pieces &= enemy_pieces - 1) {
      piece_square = Util::GetSquare(enemy_pieces);
      enemy_mobility += GetMobility(piece_square);
    }

    // 点数を計算して返す。
    return WEIGHT_MOBILITY * (my_mobility - enemy_mobility);
  }

  // センター攻撃を評価する。
  int Evaluator::EvalAttackCenter() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // センターを攻撃している攻撃の数を入れる変数。
    int num_my_center_attacked = 0;
    int num_enemy_center_attacked = 0;
    int num_my_sweet_center_attacked = 0;
    int num_enemy_sweet_center_attacked = 0;

    // 自分の攻撃回数をカウントする。
    Bitboard pieces = engine_ptr_->side_pieces_[side];
    Bitboard attacks;
    Square square;
    for (; pieces; pieces &= pieces -1) {
      square = Util::GetSquare(pieces);
      switch (engine_ptr_->piece_board_[square]) {
        case PAWN:
          attacks = Util::GetPawnAttack(square, side);
          break;
        case KNIGHT:
          attacks = Util::GetKnightMove(square);
          break;
        case BISHOP:
          attacks = engine_ptr_->GetBishopAttack(square);
          break;
        case ROOK:
          attacks = engine_ptr_->GetRookAttack(square);
          break;
        case QUEEN:
          attacks = engine_ptr_->GetQueenAttack(square);
          break;
        case KING:
          attacks = Util::GetKingMove(square);
          break;
        default:
          throw SayuriError("駒の種類が不正です。");
          break;
      }
      num_my_center_attacked += Util::CountBits(attacks & center_mask_);
      num_my_sweet_center_attacked +=
      Util::CountBits(attacks & sweet_center_mask_);
    }

    // 相手の攻撃回数をカウントする。
    pieces = engine_ptr_->side_pieces_[enemy_side];
    for (; pieces; pieces &= pieces -1) {
      square = Util::GetSquare(pieces);
      switch (engine_ptr_->piece_board_[square]) {
        case PAWN:
          attacks = Util::GetPawnAttack(square, enemy_side);
          break;
        case KNIGHT:
          attacks = Util::GetKnightMove(square);
          break;
        case BISHOP:
          attacks = engine_ptr_->GetBishopAttack(square);
          break;
        case ROOK:
          attacks = engine_ptr_->GetRookAttack(square);
          break;
        case QUEEN:
          attacks = engine_ptr_->GetQueenAttack(square);
          break;
        case KING:
          attacks = Util::GetKingMove(square);
          break;
        default:
          throw SayuriError("駒の種類が不正です。");
          break;
      }
      num_enemy_center_attacked += Util::CountBits(attacks & center_mask_);
      num_enemy_sweet_center_attacked +=
      Util::CountBits(attacks & sweet_center_mask_);
    }

    // 点数を返す。
    int score = WEIGHT_ATTACK_CENTER
    * (num_my_center_attacked - num_enemy_center_attacked);
    score += WEIGHT_ATTACK_SWEET_CENTER
    * (num_my_sweet_center_attacked - num_enemy_sweet_center_attacked);

    return score;
  }

  // マイナーピースの展開を評価する。
  int Evaluator::EvalDevelopment() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 展開してるナイトの数。初期位置にいないナイトの数。
    int num_my_pieces =
    Util::CountBits(engine_ptr_->position_[side][KNIGHT]
    & ~(start_position_[side][KNIGHT]));
    int num_enemy_pieces =
    Util::CountBits(engine_ptr_->position_[enemy_side][KNIGHT]
    & ~(start_position_[enemy_side][KNIGHT]));

    // 展開しているビショップの数。初期位置にいないビショップの数。
    num_my_pieces +=
    Util::CountBits(engine_ptr_->position_[side][BISHOP]
    & ~(start_position_[side][BISHOP]));
    num_enemy_pieces +=
    Util::CountBits(engine_ptr_->position_[enemy_side][BISHOP]
    & ~(start_position_[enemy_side][BISHOP]));

    return WEIGHT_DEVELOPMENT * (num_my_pieces - num_enemy_pieces);
  }

  // キングの周囲への攻撃を評価する。
  int Evaluator::EvalAttackAroundKing() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 相手のキングへの攻撃回数をカウント。
    int num_my_attacks = 0;
    Bitboard pieces = engine_ptr_->side_pieces_[side];
    Square square;
    Bitboard attacks;
    for (; pieces; pieces &= pieces - 1) {
      square = Util::GetSquare(pieces);
      switch (engine_ptr_->piece_board_[square]) {
        case PAWN:
          attacks =
          Util::GetPawnAttack(square, side);
          break;
        case KNIGHT:
          attacks = Util::GetKnightMove(square);
          break;
        case BISHOP:
          attacks = engine_ptr_->GetBishopAttack(square);
          break;
        case ROOK:
          attacks = engine_ptr_->GetRookAttack(square);
          break;
        case QUEEN:
          attacks = engine_ptr_->GetQueenAttack(square);
          break;
        case KING:
          attacks = Util::GetKingMove(square);
          break;
        default:
          throw SayuriError("駒の種類が不正です。");
          break;
      }
      num_my_attacks += Util::CountBits(attacks
      & Util::GetKingMove(engine_ptr_->king_[enemy_side]));
    }

    // 自分のキングへの攻撃回数をカウント。
    int num_enemy_attacks = 0;
    pieces = engine_ptr_->side_pieces_[enemy_side];
    for (; pieces; pieces &= pieces - 1) {
      square = Util::GetSquare(pieces);
      switch (engine_ptr_->piece_board_[square]) {
        case PAWN:
          attacks =
          Util::GetPawnAttack(square, enemy_side);
          break;
        case KNIGHT:
          attacks = Util::GetKnightMove(square);
          break;
        case BISHOP:
          attacks = engine_ptr_->GetBishopAttack(square);
          break;
        case ROOK:
          attacks = engine_ptr_->GetRookAttack(square);
          break;
        case QUEEN:
          attacks = engine_ptr_->GetQueenAttack(square);
          break;
        case KING:
          attacks = Util::GetKingMove(square);
          break;
        default:
          throw SayuriError("駒の種類が不正です。");
          break;
      }
      num_enemy_attacks += Util::CountBits(attacks
      & Util::GetKingMove(engine_ptr_->king_[enemy_side]));
    }

    return WEIGHT_ATTACK_AROUND_KING * (num_my_attacks - num_enemy_attacks);
  }

  // ポーンの配置を評価する。
  int Evaluator::EvalPawnPosition() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 価値を計算。
    int my_value = GetTableValue(PAWN_POSITION_TABLE, side,
    engine_ptr_->position_[side][PAWN]);
    int enemy_value = GetTableValue(PAWN_POSITION_TABLE, enemy_side,
    engine_ptr_->position_[enemy_side][PAWN]);

    return WEIGHT_PAWN_POSITION * (my_value - enemy_value);
  }

  // ナイトの配置を評価する。
  int Evaluator::EvalKnightPosition() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 価値を計算。
    int my_value = GetTableValue(KNIGHT_POSITION_TABLE, side,
    engine_ptr_->position_[side][KNIGHT]);
    int enemy_value = GetTableValue(KNIGHT_POSITION_TABLE, enemy_side,
    engine_ptr_->position_[enemy_side][KNIGHT]);

    return WEIGHT_KNIGHT_POSITION * (my_value - enemy_value);
  }

  // ルークの配置を評価する。
  int Evaluator::EvalRookPosition() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 価値を計算。
    int my_value = GetTableValue(ROOK_POSITION_TABLE, side,
    engine_ptr_->position_[side][ROOK]);
    int enemy_value = GetTableValue(ROOK_POSITION_TABLE, enemy_side,
    engine_ptr_->position_[enemy_side][ROOK]);

    return WEIGHT_ROOK_POSITION * (my_value - enemy_value);
  }

  // キングの中盤の配置を評価する。
  int Evaluator::EvalKingPositionMiddle() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 価値を計算。
    int my_value = GetTableValue(KING_POSITION_MIDDLE_TABLE, side,
    engine_ptr_->position_[side][KING]);
    int enemy_value = GetTableValue(KING_POSITION_MIDDLE_TABLE, enemy_side,
    engine_ptr_->position_[enemy_side][KING]);

    return WEIGHT_KING_POSITION_MIDDLE * (my_value - enemy_value);
  }

  // キングの終盤の配置を評価する。
  int Evaluator::EvalKingPositionEnding() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 価値を計算。
    int my_value = GetTableValue(KING_POSITION_ENDING_TABLE, side,
    engine_ptr_->position_[side][KING]);
    int enemy_value = GetTableValue(KING_POSITION_ENDING_TABLE, enemy_side,
    engine_ptr_->position_[enemy_side][KING]);

    return WEIGHT_KING_POSITION_ENDING * (my_value - enemy_value);
  }

  // パスポーンを評価する。
  int Evaluator::EvalPassPawn() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 両サイドのパスポーンを得る。
    Bitboard my_pass_pawns = GetPassPawns(side);
    Bitboard enemy_pass_pawns = GetPassPawns(enemy_side);

    // パスポーンを評価値にする。
    int my_score = WEIGHT_PASS_PAWN * Util::CountBits(my_pass_pawns);
    int enemy_score = WEIGHT_PASS_PAWN * Util::CountBits(enemy_pass_pawns);

    // 守られたパスポーンにはボーナス。
    // 自分の得点を加算。
    Square square;
    for (; my_pass_pawns; my_pass_pawns &= my_pass_pawns - 1) {
      square = Util::GetSquare(my_pass_pawns);
      // 守られていればボーナスを追加。
      if (engine_ptr_->position_[side][PAWN]
      & Util::GetPawnAttack(square, enemy_side)) {
        my_score += WEIGHT_PROTECTED_PASS_PAWN;
      }
    }
    // 相手の得点を加算。
    for (; enemy_pass_pawns; enemy_pass_pawns &= enemy_pass_pawns - 1) {
      square = Util::GetSquare(enemy_pass_pawns);
      // 守られていればボーナスを追加。
      if (engine_ptr_->position_[enemy_side][PAWN]
      & Util::GetPawnAttack(square, side)) {
        enemy_score += WEIGHT_PROTECTED_PASS_PAWN;
      }
    }

    return my_score - enemy_score;
  }

  // ダブルポーンを評価する。
  int Evaluator::EvalDoublePawn() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 各サイドのダブルポーンの数を得る。
    int num_my_pawns = Util::CountBits(GetDoublePawns(side));
    int num_enemy_pawns = Util::CountBits(GetDoublePawns(enemy_side));

    return WEIGHT_DOUBLE_PAWN * (num_my_pawns - num_enemy_pawns);
  }

  // 孤立ポーンを評価する。
  int Evaluator::EvalIsoPawn() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 各サイドの孤立ポーンの数を得る。
    int num_my_pawns = Util::CountBits(GetIsoPawns(side));
    int num_enemy_pawns = Util::CountBits(GetIsoPawns(enemy_side));

    return WEIGHT_ISO_PAWN * (num_my_pawns - num_enemy_pawns);
  }

  // ビショップペアを評価する。
  int Evaluator::EvalBishopPair() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 各サイドの得点。
    int my_score = 0;
    int enemy_score = 0;

    // ビショップが2個以上所有していればボーナス。
    if (Util::CountBits(engine_ptr_->position_[side][BISHOP]) >= 2) {
      my_score += WEIGHT_BISHOP_PAIR;
    }
    if (Util::CountBits(engine_ptr_->position_[enemy_side][BISHOP]) >= 2) {
      enemy_score += WEIGHT_BISHOP_PAIR;
    }

    return my_score - enemy_score;
  }

  // 早すぎるクイーンの出動を評価する。
  int Evaluator::EvalEarlyQueenLaunched() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 各サイドの展開されていないマイナーピースの個数を得る。
    int num_my_pieces = 0;
    int num_enemy_pieces = 0;
    Bitboard my_queen_start_pos =
    side == WHITE ? (Util::BIT[D1]) : (Util::BIT[D8]) ;
    Bitboard enemy_queen_start_pos =
    enemy_side == WHITE ? (Util::BIT[D1]) : (Util::BIT[D8]) ;
    if (engine_ptr_->position_[side][QUEEN] & ~(my_queen_start_pos)) {
      num_my_pieces =
      Util::CountBits((engine_ptr_->position_[side][KNIGHT]
      & start_position_[side][KNIGHT])
      | (engine_ptr_->position_[side][BISHOP]
      & start_position_[side][BISHOP]));
    }
    if (engine_ptr_->position_[enemy_side][QUEEN] & ~(enemy_queen_start_pos)) {
      num_enemy_pieces =
      Util::CountBits((engine_ptr_->position_[enemy_side][KNIGHT]
      & start_position_[enemy_side][KNIGHT])
      | (engine_ptr_->position_[enemy_side][BISHOP]
      & start_position_[enemy_side][BISHOP]));
    }

    return WEIGHT_EARLY_QUEEN_LAUNCHED * (num_my_pieces - num_enemy_pieces);
  }

  // ポーンの盾を評価する。
  int Evaluator::EvalPawnShield() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // 各サイドのポーンの盾の数を得る。
    int num_my_pawns =
    Util::CountBits(engine_ptr_->position_[side][PAWN]
    & pawn_shield_mask_[side][engine_ptr_->king_[side]]);
    int num_enemy_pawns =
    Util::CountBits(engine_ptr_->position_[enemy_side][PAWN]
    & pawn_shield_mask_[enemy_side][engine_ptr_->king_[enemy_side]]);

    return WEIGHT_PAWN_SHIELD * (num_my_pawns - num_enemy_pawns);
  }

  // キャスリングを評価する。
  int Evaluator::EvalCastling() {
    // サイド。
    Side side = engine_ptr_->to_move_;
    Side enemy_side = side ^ 0x3;

    // とりあえずキャスリングボーナスを加算。
    int my_score = WEIGHT_CASTLING;
    int enemy_score = WEIGHT_CASTLING;

    // キャスリングを放棄したら0点。
    Castling my_rights =
    side == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
    Castling enemy_rights =
    enemy_side == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
    if ((!(engine_ptr_->has_castled_[side]))
    && (!(engine_ptr_->castling_rights_ & my_rights))) {
      my_score = 0;
    }
    if ((!(engine_ptr_->has_castled_[enemy_side]))
    && (!(engine_ptr_->castling_rights_ & enemy_rights))) {
      enemy_score = 0;
    }

    return my_score - enemy_score;
  }
  /******************************/
  /* その他のプライベート関数。 */
  /******************************/
  // テーブルを計算する関数。
  int Evaluator::GetTableValue(const int (& table)[NUM_SQUARES],
  Side side, Bitboard bitboard) {
    // ボードを鏡対象に上下反転させる配列。
    // <配列>[flip[<位置>]]と使うと上下が反転する。
    constexpr Square FLIP[NUM_SQUARES] {
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
    } else if (side == BLACK) {
      for (; bitboard; bitboard &= bitboard - 1) {
        square = Util::GetSquare(bitboard);
        value += table[FLIP[square]];
      }
    }

    // 値を返す。
    return value;
  }

  // 駒の初期位置を初期化。
  void Evaluator::InitStartPosition() {
    // ポーン。
    start_position_[WHITE][PAWN] = Util::RANK[RANK_2];
    start_position_[BLACK][PAWN] = Util::RANK[RANK_7];

    // ナイト。
    start_position_[WHITE][KNIGHT] = Util::BIT[B1] | Util::BIT[G1];
    start_position_[BLACK][KNIGHT] = Util::BIT[B8] | Util::BIT[G8];

    // ビショップ。
    start_position_[WHITE][BISHOP] = Util::BIT[C1] | Util::BIT[F1];
    start_position_[BLACK][BISHOP] = Util::BIT[C8] | Util::BIT[F8];

    // ルーク。
    start_position_[WHITE][ROOK] = Util::BIT[A1] | Util::BIT[H1];
    start_position_[BLACK][ROOK] = Util::BIT[A8] | Util::BIT[H8];

    // クイーン。
    start_position_[WHITE][QUEEN] = Util::BIT[D1];
    start_position_[BLACK][QUEEN] = Util::BIT[D8];

    // キング。
    start_position_[WHITE][KING] = Util::BIT[E1];
    start_position_[BLACK][KING] = Util::BIT[E8];
  }

  // センターマスクを初期化する。
  void Evaluator::InitCenterMask() {
    // センター。
    center_mask_ =
    Util::BIT[C3] | Util::BIT[C4] | Util::BIT[C5] | Util::BIT[C6]
    | Util::BIT[D3] | Util::BIT[D4] | Util::BIT[D5] | Util::BIT[D6]
    | Util::BIT[E3] | Util::BIT[E4] | Util::BIT[E5] | Util::BIT[E6]
    | Util::BIT[F3] | Util::BIT[F4] | Util::BIT[F5] | Util::BIT[F6];

    // スウィートセンター。
    sweet_center_mask_ = Util::BIT[D4] | Util::BIT[D5]
    | Util::BIT[E4] | Util::BIT[E5];
  }

  // pass_pawn_mask_[][]を初期化する。
  void Evaluator::InitPassPawnMask() {
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

  // iso_pawn_mask_[]を初期化する。
  void Evaluator::InitIsoPawnMask() {
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

  // pawn_shield_mask_[][]を初期化する。
  void Evaluator::InitPawnShieldMask() {
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
