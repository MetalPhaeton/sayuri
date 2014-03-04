/*
   evaluator.cpp: 局面を評価するクラスの実装。

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

#include "evaluator.h"

#include <iostream>
#include "error.h"

namespace Sayuri {
  /**********************/
  /* static const定数。 */
  /**********************/
  // ポーンの配置。
  const Evaluator::Weight Evaluator::WEIGHT_PAWN_POSITION(2.0, 0.0);
  // ナイトの配置。
  const Evaluator::Weight Evaluator::WEIGHT_KNIGHT_POSITION(2.5, 0.0);
  // ビショップの配置。
  const Evaluator::Weight Evaluator::WEIGHT_BISHOP_POSITION(3.5, 0.0);
  // ルークの配置。
  const Evaluator::Weight Evaluator::WEIGHT_ROOK_POSITION(2.5, 0.0);
  // クイーンの配置。
  const Evaluator::Weight Evaluator::WEIGHT_QUEEN_POSITION(2.5, 0.0);
  // キングの配置。
  const Evaluator::Weight Evaluator::WEIGHT_KING_POSITION(10.0, 0.0);
  // 終盤のポーンの配置。
  const Evaluator::Weight Evaluator::WEIGHT_PAWN_POSITION_ENDING(0.0, 20.0);
  // 終盤のキングの配置。
  const Evaluator::Weight Evaluator::WEIGHT_KING_POSITION_ENDING(0.0, 15.0);
  // 機動力。
  const Evaluator::Weight Evaluator::WEIGHT_MOBILITY(1.0, 1.0);
  // センターコントロール。
  const Evaluator::Weight Evaluator::WEIGHT_CENTER_CONTROL(0.5, 0.0);
  // スウィートセンターのコントロール。
  const Evaluator::Weight Evaluator::WEIGHT_SWEET_CENTER_CONTROL(0.5, 0.0);
  // 駒の展開。
  const Evaluator::Weight Evaluator::WEIGHT_DEVELOPMENT(2.5, 0.0);
  // 攻撃。
  const Evaluator::Weight Evaluator::WEIGHT_ATTACK(0.0, 0.0);
  // キングによる攻撃。
  const Evaluator::Weight Evaluator::WEIGHT_ATTACK_BY_KING(1.0, 0.0);
  // 相手キング周辺への攻撃
  const Evaluator::Weight Evaluator::WEIGHT_ATTACK_AROUND_KING(0.0, 3.0);
  // パスポーン。
  const Evaluator::Weight Evaluator::WEIGHT_PASS_PAWN(7.0, 14.0);
  // 守られたパスポーン。
  const Evaluator::Weight Evaluator::WEIGHT_PROTECTED_PASS_PAWN(2.5, 2.5);
  // ダブルポーン。
  const Evaluator::Weight Evaluator::WEIGHT_DOUBLE_PAWN(-2.5, -5.0);
  // 孤立ポーン。
  const Evaluator::Weight Evaluator::WEIGHT_ISO_PAWN(-5.0, -2.5);
  // ポーンの盾。
  const Evaluator::Weight Evaluator::WEIGHT_PAWN_SHIELD(3.0, 0.0);
  // ビショップペア。
  const Evaluator::Weight Evaluator::WEIGHT_BISHOP_PAIR(10.0, 60.0);
  // バッドビショップ。
  const Evaluator::Weight Evaluator::WEIGHT_BAD_BISHOP(-0.7, 0.0);
  // ナイトをピン。
  const Evaluator::Weight Evaluator::WEIGHT_PIN_KNIGHT(10.0, 0.0);
  // ルークペア。
  const Evaluator::Weight Evaluator::WEIGHT_ROOK_PAIR(0.0, 0.0);
  // セミオープンファイルのルーク。
  const Evaluator::Weight Evaluator::WEIGHT_ROOK_SEMI_OPEN(3.5, 3.5);
  // オープンファイルのルーク。
  const Evaluator::Weight Evaluator::WEIGHT_ROOK_OPEN(3.5, 3.5);
  // 早すぎるクイーンの始動。
  const Evaluator::Weight Evaluator::WEIGHT_EARLY_QUEEN_LAUNCHED(-20.0, 0.0);
  // キング周りの弱いマス。
  const Evaluator::Weight Evaluator::WEIGHT_WEAK_SQUARE(-5.0, 0.0);
  // キャスリング。(これの2倍が評価値。権利の放棄は-1倍。)
  const Evaluator::Weight Evaluator::WEIGHT_CASTLING(45.0, 0.0);

  /****************/
  /* static定数。 */
  /****************/
  constexpr double Evaluator::POSITION_TABLE[NUM_PIECE_TYPES][NUM_SQUARES];
  constexpr double Evaluator::PAWN_POSITION_ENDING_TABLE[NUM_SQUARES];
  constexpr double Evaluator::KING_POSITION_ENDING_TABLE[NUM_SQUARES];
  constexpr double Evaluator::ATTACK_VALUE_TABLE
  [NUM_PIECE_TYPES][NUM_PIECE_TYPES];

  /****************/
  /* static変数。 */
  /****************/
  Bitboard Evaluator::start_position_[NUM_SIDES][NUM_PIECE_TYPES];
  Bitboard Evaluator::center_mask_;
  Bitboard Evaluator::sweet_center_mask_;
  Bitboard Evaluator::pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
  Bitboard Evaluator::iso_pawn_mask_[NUM_SQUARES];
  Bitboard Evaluator::pawn_shield_mask_[NUM_SIDES][NUM_SQUARES];
  Bitboard Evaluator::weak_square_mask_[NUM_SIDES][NUM_SQUARES];

  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
  // コンストラクタ。
  Evaluator::Evaluator(const ChessEngine& engine)
  : engine_ptr_(&engine) {
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

  /*****************************/
  /* Evaluatorクラスの初期化。 */
  /*****************************/
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
    // weak_square_mask_[][]を初期化する。
    InitWeakSquareMask();
  }

  /********************/
  /* パブリック関数。 */
  /********************/

  // 評価値を返す。
  int Evaluator::Evaluate() {
    // 価値の変数の初期化。
    for (int i = 0; i < NUM_PIECE_TYPES; i++) {
      position_value_[i] = 0.0;
    }
    pawn_position_ending_value_ = 0.0;
    king_position_ending_value_ = 0.0;
    mobility_value_ = 0.0;
    center_control_value_ = 0.0;
    sweet_center_control_value_ = 0.0;
    development_value_ = 0.0;
    for (int i = 0; i < NUM_PIECE_TYPES; i++) {
      attack_value_[i] = 0.0;
    }
    pass_pawn_value_ = 0.0;
    protected_pass_pawn_value_ = 0.0;
    double_pawn_value_ = 0.0;
    iso_pawn_value_ = 0.0;
    pawn_shield_value_ = 0.0;
    bishop_pair_value_ = 0.0;
    bad_bishop_value_ = 0.0;
    pin_knight_value_ = 0.0;
    rook_pair_value_ = 0.0;
    rook_semi_open_value_ = 0.0;
    rook_open_value_ = 0.0;
    early_queen_launched_value_ = 0.0;
    attack_around_king_value_ = 0.0;
    weak_square_value_ = 0.0;
    castling_value_ = 0.0;

    // サイド。
    Side side = engine_ptr_->to_move();
    Side enemy_side = side ^ 0x3;

    // 十分な駒がない場合は引き分け。
    if (!HasEnoughPieces(side) && !HasEnoughPieces(enemy_side)) {
      return SCORE_DRAW;
    }

    // 全体計算。
    // ビショップペア。
    if (Util::CountBits(engine_ptr_->position()[side][BISHOP]) >= 2) {
      bishop_pair_value_ += 1.0;
    }
    if (Util::CountBits(engine_ptr_->position()[enemy_side][BISHOP]) >= 2) {
      bishop_pair_value_ -= 1.0;
    }
    // ルークペア。
    if (Util::CountBits(engine_ptr_->position()[side][ROOK]) >= 2) {
      rook_pair_value_ += 1.0;
    }
    if (Util::CountBits(engine_ptr_->position()[enemy_side][ROOK]) >= 2) {
      rook_pair_value_ -= 1.0;
    }

    // 各駒毎に価値を計算する。
    Bitboard all_pieces = engine_ptr_->blocker_0();
    for (Bitboard pieces = all_pieces; pieces; pieces &= pieces - 1) {
      Square piece_square = Util::GetSquare(pieces);
      Side piece_side = engine_ptr_->side_board()[piece_square];
      switch (engine_ptr_->piece_board()[piece_square]) {
        case PAWN:
          CalValue<PAWN>(piece_square, piece_side);
          break;
        case KNIGHT:
          CalValue<KNIGHT>(piece_square, piece_side);
          break;
        case BISHOP:
          CalValue<BISHOP>(piece_square, piece_side);
          break;
        case ROOK:
          CalValue<ROOK>(piece_square, piece_side);
          break;
        case QUEEN:
          CalValue<QUEEN>(piece_square, piece_side);
          break;
        case KING:
          CalValue<KING>(piece_square, piece_side);
          break;
        default:
          throw SayuriError("駒の種類が不正です。");
          break;
      }
    }

    // ウェイトを付けて評価値を得る。
    constexpr int NUM_KINGS = 2;
    double num_pieces = static_cast<double>
    (Util::CountBits(all_pieces) - NUM_KINGS);
    // マテリアル。
    double score = static_cast<double>(engine_ptr_->GetMaterial(side));
    // ポーンの配置。
    score += WEIGHT_PAWN_POSITION(num_pieces) * position_value_[PAWN];
    // ナイトの配置。
    score += WEIGHT_KNIGHT_POSITION(num_pieces) * position_value_[KNIGHT];
    // ビショップの配置。
    score += WEIGHT_BISHOP_POSITION(num_pieces) * position_value_[BISHOP];
    // ルークの配置。
    score += WEIGHT_ROOK_POSITION(num_pieces) * position_value_[ROOK];
    // クイーンの配置。
    score += WEIGHT_QUEEN_POSITION(num_pieces) * position_value_[QUEEN];
    // キングの配置。
    score += WEIGHT_KING_POSITION(num_pieces) * position_value_[KING];
    // 終盤のポーンの配置。
    score += WEIGHT_PAWN_POSITION_ENDING(num_pieces)
    * pawn_position_ending_value_;
    // 終盤のキングの配置。
    score += WEIGHT_KING_POSITION_ENDING(num_pieces)
    * king_position_ending_value_;
    // 機動力。
    score += WEIGHT_MOBILITY(num_pieces) * mobility_value_;
    // センターコントロール。
    score += WEIGHT_CENTER_CONTROL(num_pieces) * center_control_value_;
    // スウィートセンターのコントロール。
    score += WEIGHT_SWEET_CENTER_CONTROL(num_pieces)
    * sweet_center_control_value_;
    // 駒の展開。
    score += WEIGHT_DEVELOPMENT(num_pieces) * development_value_;
    // 攻撃。
    double temp_weight = WEIGHT_ATTACK(num_pieces);
    for (int i = PAWN; i <= QUEEN; i++) {
      score += temp_weight * attack_value_[i];
    }
    // キングによる攻撃。
    score += WEIGHT_ATTACK_BY_KING(num_pieces) * attack_value_[KING];
    // 相手キング周辺への攻撃。
    score += WEIGHT_ATTACK_AROUND_KING(num_pieces) * attack_around_king_value_;
    // パスポーン。
    score += WEIGHT_PASS_PAWN(num_pieces) * pass_pawn_value_;
    // 守られたパスポーン。
    score += WEIGHT_PROTECTED_PASS_PAWN(num_pieces)
    * protected_pass_pawn_value_;
    // ダブルポーン。
    score += WEIGHT_DOUBLE_PAWN(num_pieces) * double_pawn_value_;
    // 孤立ポーン。
    score += WEIGHT_ISO_PAWN(num_pieces) * iso_pawn_value_;
    // ポーンの盾。
    score += WEIGHT_PAWN_SHIELD(num_pieces) * pawn_shield_value_;
    // ビショップペア。
    score += WEIGHT_BISHOP_PAIR(num_pieces) * bishop_pair_value_;
    // バッドビショップ。
    score += WEIGHT_BAD_BISHOP(num_pieces) * bad_bishop_value_;
    // ビショップにピンされたナイト。
    // ナイトをピン。
    score += WEIGHT_PIN_KNIGHT(num_pieces) * pin_knight_value_;
    // ルークペア。
    score += WEIGHT_ROOK_PAIR(num_pieces) * rook_pair_value_;
    // セミオープンファイルのルーク。
    score += WEIGHT_ROOK_SEMI_OPEN(num_pieces) * rook_semi_open_value_;
    // オープンファイルのルーク。
    score += WEIGHT_ROOK_OPEN(num_pieces) * rook_open_value_;
    // 早すぎるクイーンの始動。
    score += WEIGHT_EARLY_QUEEN_LAUNCHED(num_pieces)
    * early_queen_launched_value_;
    // キング周りの弱いマス。
    score += WEIGHT_WEAK_SQUARE(num_pieces) * weak_square_value_;
    // キャスリング。(これの2倍が評価値。)
    score += WEIGHT_CASTLING(num_pieces) * castling_value_;

    return static_cast<int>(score);
  }

  /****************************/
  /* 局面評価に使用する関数。 */
  /****************************/
  // 勝つのに十分な駒があるかどうか調べる。
  bool Evaluator::HasEnoughPieces(Side side) const {
    // ポーンがあれば大丈夫。
    if (engine_ptr_->position()[side][PAWN]) return true;

    // ルークがあれば大丈夫。
    if (engine_ptr_->position()[side][ROOK]) return true;

    // クイーンがあれば大丈夫。
    if (engine_ptr_->position()[side][QUEEN]) return true;

    // ビショップが2つあれば大丈夫。
    if (Util::CountBits(engine_ptr_->position()[side][BISHOP]) >= 2)
      return true;

    // ナイトが2つあれば大丈夫。
    if (Util::CountBits(engine_ptr_->position()[side][KNIGHT]) >= 2)
      return true;

    // ナイトとビショップの合計が2つあれば大丈夫。
    if (Util::CountBits(engine_ptr_->position()[side][KNIGHT]
    | engine_ptr_->position()[side][BISHOP]) >= 2)
      return true;

    // それ以外はダメ。
    return false;
  }

  // 進行状況を得る。
  // フェーズは一次関数で計算。
  double Evaluator::GetPhase() const {
    constexpr double MODULUS = 1.0 / 14.0;

    double num_pieces = static_cast<double>(Util::CountBits
    (engine_ptr_->blocker_0() & ~(engine_ptr_->position()[WHITE][PAWN]
    | engine_ptr_->position()[BLACK][PAWN]
    | engine_ptr_->position()[WHITE][KING]
    | engine_ptr_->position()[BLACK][KING])));
    if (num_pieces > 14.0) num_pieces = 14.0;

    return MODULUS * num_pieces;
  }

  /************************/
  /* 価値を計算する関数。 */
  /************************/
  // 各駒での価値を計算する。
  template<Piece Type>
  void Evaluator::CalValue(Square piece_square, Side piece_side) {
    // サイド。
    Side enemy_piece_side = piece_side ^ 0x3;

    // 値と符号。自分の駒ならプラス。敵の駒ならマイナス。
    double value;
    double sign = piece_side == engine_ptr_->to_move() ? 1.0 : -1.0;

    // 利き筋を作る。
    Bitboard attacks = 0ULL;
    Bitboard pawn_moves = 0ULL;
    Bitboard en_passant = 0ULL;
    Bitboard castling_moves = 0ULL;
    switch (Type) {
      case PAWN:
        // 通常の動き。
        pawn_moves = Util::GetPawnMove(piece_square, piece_side)
        & ~(engine_ptr_->blocker_0());
        // 2歩の動き。
        if (pawn_moves) {
          if (((piece_side == WHITE)
          && (Util::GetRank(piece_square) == RANK_2))
          || ((piece_side == BLACK)
          && (Util::GetRank(piece_square) == RANK_7))) {
            // ポーンの2歩の動き。
            pawn_moves |= Util::GetPawn2StepMove(piece_square, piece_side)
            & ~(engine_ptr_->blocker_0());
          }
        }
        // 攻撃。
        attacks = Util::GetPawnAttack(piece_square, piece_side);

        // アンパッサン。
        if (engine_ptr_->en_passant_square()) {
          en_passant =
          Util::SQUARE[engine_ptr_->en_passant_square()] & attacks;
        }
        break;
      case KNIGHT:
        attacks = Util::GetKnightMove(piece_square);
        break;
      case BISHOP:
        attacks = engine_ptr_->GetBishopAttack(piece_square);
        break;
      case ROOK:
        attacks = engine_ptr_->GetRookAttack(piece_square);
        break;
      case QUEEN:
        attacks = engine_ptr_->GetQueenAttack(piece_square);
        break;
      case KING:
        attacks = Util::GetKingMove(piece_square);
        castling_moves = 0ULL;
        // キャスリングの動きを追加。
        if (piece_side == WHITE) {
          if (engine_ptr_->CanCastling<WHITE_SHORT_CASTLING>()) {
            castling_moves |= Util::SQUARE[G1];
          }
          if (engine_ptr_->CanCastling<WHITE_LONG_CASTLING>()) {
            castling_moves |= Util::SQUARE[C1];
          }
        } else {
          if (engine_ptr_->CanCastling<BLACK_SHORT_CASTLING>()) {
            castling_moves |= Util::SQUARE[G8];
          }
          if (engine_ptr_->CanCastling<BLACK_LONG_CASTLING>()) {
            castling_moves |= Util::SQUARE[C8];
          }
        }
        break;
      default:
        throw SayuriError("駒の種類が不正です。");
        break;
    }

    // 駒の配置を計算。
    if (piece_side == WHITE) {
      value = POSITION_TABLE[Type][piece_square];
    } else {
      value = POSITION_TABLE[Type][Util::FLIP[piece_square]];
    }
    position_value_[Type] += sign * value;
    // ポーンの終盤の配置。
    if (Type == PAWN) {
      if (piece_side == WHITE) {
        value = PAWN_POSITION_ENDING_TABLE[piece_square];
      } else {
        value = PAWN_POSITION_ENDING_TABLE[Util::FLIP[piece_square]];
      }
      pawn_position_ending_value_ += sign * value;
    }
    // キングの終盤の配置。
    if (Type == KING) {
      if (piece_side == WHITE) {
        value = KING_POSITION_ENDING_TABLE[piece_square];
      } else {
        value = KING_POSITION_ENDING_TABLE[Util::FLIP[piece_square]];
      }
      king_position_ending_value_ += sign * value;
    }

    // 駒の動きやすさを計算。
    if (Type == PAWN) {
      value = static_cast<double>(Util::CountBits(pawn_moves
      | (attacks & engine_ptr_->side_pieces()[enemy_piece_side])
      | en_passant));
    } else if (Type == KING) {
      value = static_cast<double>(Util::CountBits(castling_moves
      | (attacks & ~(engine_ptr_->side_pieces()[piece_side]))));
    } else {
      value = static_cast<double>(Util::CountBits(attacks
      & ~(engine_ptr_->side_pieces()[piece_side])));
    }
    mobility_value_ += sign * value;

    // センターコントロールを計算。
    if (Type != KING) {
      value = static_cast<double>(Util::CountBits(attacks & center_mask_));
      center_control_value_ += sign * value;
      value =
      static_cast<double>(Util::CountBits(attacks & sweet_center_mask_));
      sweet_center_control_value_ += sign * value;
    }

    // 駒の展開を計算。
    if ((Type == KNIGHT) || (Type == BISHOP)) {
      value = 0.0;
      if (Util::SQUARE[piece_square] & ~(start_position_[piece_side][Type])) {
        value += 1.0;
      }
      development_value_ += sign * value;
    }

    // 敵への攻撃を計算。
    Bitboard temp = attacks & (engine_ptr_->side_pieces()[enemy_piece_side]);
    value = 0.0;
    for (; temp; temp &= temp - 1) {
      value += ATTACK_VALUE_TABLE
      [Type][engine_ptr_->piece_board()[Util::GetSquare(temp)]];
    }
    if ((Type == PAWN) && en_passant) {
      value += ATTACK_VALUE_TABLE[Type][PAWN];
    }
    attack_value_[Type] += sign * value;

    // 相手キング周辺への攻撃を計算。
    if (Type != KING) {
      value = static_cast<double>(Util::CountBits(attacks
      & Util::GetKingMove(engine_ptr_->king()[enemy_piece_side])));
      attack_around_king_value_ += sign * value;
    }

    // ポーンの構成を計算。
    if (Type == PAWN) {
      // パスポーンを計算。
      if (!(engine_ptr_->position()[enemy_piece_side][PAWN]
      & pass_pawn_mask_[piece_side][piece_square])) {
        pass_pawn_value_ += sign * 1.0;
        // 守られたパスポーン。
        if (engine_ptr_->position()[piece_side][PAWN]
        & Util::GetPawnAttack(piece_square, enemy_piece_side)) {
          protected_pass_pawn_value_ += sign * 1.0;
        }
      }

      // ダブルポーンを計算。
      int fyle = Util::GetFyle(piece_square);
      if (Util::CountBits(engine_ptr_->position()[piece_side][PAWN]
      & Util::FYLE[fyle]) >= 2) {
        double_pawn_value_ += sign * 1.0;
      }

      // 孤立ポーンを計算。
      if (!(engine_ptr_->position()[piece_side][PAWN]
      & iso_pawn_mask_[piece_square])) {
        iso_pawn_value_ += sign * 1.0;
      }

      // ポーンの盾を計算。
      if ((Util::SQUARE[piece_square]
      & pawn_shield_mask_[piece_side][engine_ptr_->king()[piece_side]])) {
        if (piece_side == WHITE) {
          value =
          static_cast<double>(POSITION_TABLE[PAWN][Util::FLIP[piece_square]]);
        } else {
          value = static_cast<double>(POSITION_TABLE[PAWN][piece_square]);
        }
        pawn_shield_value_ += sign * value;
      }
    }

    if (Type == BISHOP) {
      // バッドビショップを計算。
      value = 0.0;
      if ((Util::SQUARE[piece_square] & Util::SQCOLOR[WHITE])) {
        value = static_cast<double>(Util::CountBits
        (engine_ptr_->position()[piece_side][PAWN] & Util::SQCOLOR[WHITE]));
      } else {
        value = static_cast<double>(Util::CountBits
        (engine_ptr_->position()[piece_side][PAWN] & Util::SQCOLOR[BLACK]));
      }
      bad_bishop_value_ += sign * value;

      // ナイトをピンを計算。
      // 絶対ピン。
      value = 0.0;
      Bitboard line =
      Util::GetLine(piece_square, engine_ptr_->king()[enemy_piece_side]);
      if ((line & attacks
      & engine_ptr_->position()[enemy_piece_side][KNIGHT])) {
        if (Util::CountBits(line & engine_ptr_->blocker_0()) == 3) {
          value += 1.0;
        }
      }
      // クイーンへのピン。
      for (Bitboard bb = engine_ptr_->position()[enemy_piece_side][QUEEN];
      bb; bb &= bb - 1) {
        line = Util::GetLine(piece_square, Util::GetSquare(bb));
        if ((line & attacks
        & engine_ptr_->position()[enemy_piece_side][KNIGHT])) {
          if (Util::CountBits(line & engine_ptr_->blocker_0()) == 3) {
            value += 1.0;
          }
        }
      }
      pin_knight_value_ += sign * value;
    }

    // セミオープン、オープンファイルのルークを計算。
    if (Type == ROOK) {
      // セミオープン。
      if (!(engine_ptr_->position()[piece_side][PAWN]
      & Util::FYLE[Util::GetFyle(piece_square)])) {
        rook_semi_open_value_ += sign * 1.0;
        if (!(engine_ptr_->position()[enemy_piece_side][PAWN]
        & Util::FYLE[Util::GetFyle(piece_square)])) {
          rook_open_value_ += sign * 1.0;
        }
      }
    }

    // クイーンの早過ぎる始動を計算。
    if (Type == QUEEN) {
      value = 0.0;
      if (Util::SQUARE[piece_square] & ~(start_position_[piece_side][QUEEN])) {
        value += static_cast<double>
        (Util::CountBits(engine_ptr_->position()[piece_side][KNIGHT]
        & start_position_[piece_side][KNIGHT]));
        value += static_cast<double>
        (Util::CountBits(engine_ptr_->position()[piece_side][BISHOP]
        & start_position_[piece_side][BISHOP]));
      }
      early_queen_launched_value_ += sign * value;
    }

    // キングの守りを計算。
    if (Type == KING) {
      // キング周りの弱いマスを計算。
      // 弱いマス。
      value = 0.0;
      Bitboard weak = (~(engine_ptr_->position()[piece_side][PAWN]))
      & weak_square_mask_[piece_side][piece_square];
      // それぞれの色のマスの弱いマスの数。
      int white_weak = Util::CountBits(weak & Util::SQCOLOR[WHITE]);
      int black_weak = Util::CountBits(weak & Util::SQCOLOR[BLACK]);
      // 相手の白マスビショップの数と弱い白マスの数を掛け算。
      value += static_cast<double>(Util::CountBits
      (engine_ptr_->position()[enemy_piece_side][BISHOP]
      & Util::SQCOLOR[WHITE]) * white_weak);
      // 相手の黒マスビショップの数と弱い黒マスの数を掛け算。
      value += static_cast<double>(Util::CountBits
      (engine_ptr_->position()[enemy_piece_side][BISHOP]
      & Util::SQCOLOR[BLACK]) * black_weak);
      // 評価値にする。
      weak_square_value_ += sign * value;

      // キャスリングを計算する。
      value = 0.0;  // キャスリングはまだだが、放棄していない。
      Castling rights_mask =
      piece_side == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
      if (engine_ptr_->has_castled()[piece_side]) {
        // キャスリングした。
        value = 2.0;
      } else {
        if (!(engine_ptr_->castling_rights() & rights_mask)) {
          // キャスリングの権利を放棄した。
          value = -1.0;
        }
      }
      castling_value_ += sign * value;
    }
  }
  // 実体化。
  template void Evaluator::CalValue<PAWN>
  (Square piece_type, Side piece_side);
  template void Evaluator::CalValue<KNIGHT>
  (Square piece_type, Side piece_side);
  template void Evaluator::CalValue<BISHOP>
  (Square piece_type, Side piece_side);
  template void Evaluator::CalValue<ROOK>
  (Square piece_type, Side piece_side);
  template void Evaluator::CalValue<QUEEN>
  (Square piece_type, Side piece_side);
  template void Evaluator::CalValue<KING>
  (Square piece_type, Side piece_side);

  /******************************/
  /* その他のプライベート関数。 */
  /******************************/
  // 駒の初期位置を初期化。
  void Evaluator::InitStartPosition() {
    // ポーン。
    start_position_[WHITE][PAWN] = Util::RANK[RANK_2];
    start_position_[BLACK][PAWN] = Util::RANK[RANK_7];

    // ナイト。
    start_position_[WHITE][KNIGHT] = Util::SQUARE[B1] | Util::SQUARE[G1];
    start_position_[BLACK][KNIGHT] = Util::SQUARE[B8] | Util::SQUARE[G8];

    // ビショップ。
    start_position_[WHITE][BISHOP] = Util::SQUARE[C1] | Util::SQUARE[F1];
    start_position_[BLACK][BISHOP] = Util::SQUARE[C8] | Util::SQUARE[F8];

    // ルーク。
    start_position_[WHITE][ROOK] = Util::SQUARE[A1] | Util::SQUARE[H1];
    start_position_[BLACK][ROOK] = Util::SQUARE[A8] | Util::SQUARE[H8];

    // クイーン。
    start_position_[WHITE][QUEEN] = Util::SQUARE[D1];
    start_position_[BLACK][QUEEN] = Util::SQUARE[D8];

    // キング。
    start_position_[WHITE][KING] = Util::SQUARE[E1];
    start_position_[BLACK][KING] = Util::SQUARE[E8];
  }

  // センターマスクを初期化する。
  void Evaluator::InitCenterMask() {
    // センター。
    center_mask_ = Util::SQUARE[C3] | Util::SQUARE[C4]
    | Util::SQUARE[C5] | Util::SQUARE[C6]
    | Util::SQUARE[D3] | Util::SQUARE[D4]
    | Util::SQUARE[D5] | Util::SQUARE[D6]
    | Util::SQUARE[E3] | Util::SQUARE[E4]
    | Util::SQUARE[E5] | Util::SQUARE[E6]
    | Util::SQUARE[F3] | Util::SQUARE[F4]
    | Util::SQUARE[F5] | Util::SQUARE[F6];

    // スウィートセンター。
    sweet_center_mask_ = Util::SQUARE[D4] | Util::SQUARE[D5]
    | Util::SQUARE[E4] | Util::SQUARE[E5];
  }

  // pass_pawn_mask_[][]を初期化する。
  void Evaluator::InitPassPawnMask() {
    // マスクを作って初期化する。
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int square = 0; square < NUM_SQUARES; square++) {
        Bitboard mask = 0ULL;
        if (side == NO_SIDE) {  // どちらのサイドでもなければ0。
          pass_pawn_mask_[side][square] = 0;
        } else {
          // 自分のファイルと隣のファイルのマスクを作る。
          Fyle fyle = Util::GetFyle(square);
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
            Bitboard temp = (Util::SQUARE[square] - 1)
            | Util::RANK[Util::GetRank(square)];
            mask &= ~temp;
          } else {
            Bitboard temp = ~(Util::SQUARE[square] - 1)
            | Util::RANK[Util::GetRank(square)];
            mask &= ~temp;
          }

          // マスクをセット。
          pass_pawn_mask_[side][square] = mask;
        }
      }
    }
  }

  // iso_pawn_mask_[]を初期化する。
  void Evaluator::InitIsoPawnMask() {
    for (int square = 0; square < NUM_SQUARES; square++) {
      Fyle fyle = Util::GetFyle(square);
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
          pawn_shield_mask_[side][square] = 0ULL;
        } else {
          if ((side == WHITE)
          && ((square == A1) || (square == B1) || (square == C1)
          || (square == A2) || (square == B2) || (square == C2))) {
            pawn_shield_mask_[side][square] =
            Util::FYLE[FYLE_A] | Util::FYLE[FYLE_B] | Util::FYLE[FYLE_C];
          } else if ((side == WHITE)
          && ((square == F1) || (square == G1) || (square == H1)
          || (square == F2) || (square == G2) || (square == H2))) {
            pawn_shield_mask_[side][square] =
            Util::FYLE[FYLE_F] | Util::FYLE[FYLE_G] | Util::FYLE[FYLE_H];
          } else if ((side == BLACK)
          && ((square == A8) || (square == B8) || (square == C8)
          || (square == A7) || (square == B7) || (square == C7))) {
            pawn_shield_mask_[side][square] =
            Util::FYLE[FYLE_A] | Util::FYLE[FYLE_B] | Util::FYLE[FYLE_C];
          } else if ((side == BLACK)
          && ((square == F8) || (square == G8) || (square == H8)
          || (square == F7) || (square == G7) || (square == H7))) {
            pawn_shield_mask_[side][square] =
            Util::FYLE[FYLE_F] | Util::FYLE[FYLE_G] | Util::FYLE[FYLE_H];
          } else {
            pawn_shield_mask_[side][square] = 0ULL;
          }
        }
      }
    }
  }

  // weak_square_mask_[][]を初期化する。
  void Evaluator::InitWeakSquareMask() {
    for (int side = 0; side < NUM_SIDES; side++) {
      for (int square = 0; square < NUM_SQUARES; square++) {
        if (side == NO_SIDE) {  // どちらのサイドでもなければ空。
          weak_square_mask_[side][square] = 0ULL;
        } else {
          if ((side == WHITE)
          && ((square == A1) || (square == B1) || (square == C1)
          || (square == A2) || (square == B2) || (square == C2))) {
            weak_square_mask_[side][square] =
            (Util::FYLE[FYLE_A] | Util::FYLE[FYLE_B] | Util::FYLE[FYLE_C])
            & (Util::RANK[RANK_2] | Util::RANK[RANK_3]);
          } else if ((side == WHITE)
          && ((square == F1) || (square == G1) || (square == H1)
          || (square == F2) || (square == G2) || (square == H2))) {
            weak_square_mask_[side][square] =
            (Util::FYLE[FYLE_F] | Util::FYLE[FYLE_G] | Util::FYLE[FYLE_H])
            & (Util::RANK[RANK_2] | Util::RANK[RANK_3]);
          } else if ((side == BLACK)
          && ((square == A8) || (square == B8) || (square == C8)
          || (square == A7) || (square == B7) || (square == C7))) {
            weak_square_mask_[side][square] =
            (Util::FYLE[FYLE_A] | Util::FYLE[FYLE_B] | Util::FYLE[FYLE_C])
            & (Util::RANK[RANK_7] | Util::RANK[RANK_6]);
          } else if ((side == BLACK)
          && ((square == F8) || (square == G8) || (square == H8)
          || (square == F7) || (square == G7) || (square == H7))) {
            weak_square_mask_[side][square] =
            (Util::FYLE[FYLE_F] | Util::FYLE[FYLE_G] | Util::FYLE[FYLE_H])
            & (Util::RANK[RANK_7] | Util::RANK[RANK_6]);
          } else {
            weak_square_mask_[side][square] = 0ULL;
          }
        }
      }
    }
  }
}  // namespace Sayuri
