/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
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
 * @file evaluator.cpp
 * @author Hironori Ishibashi
 * @brief 評価関数クラスの実装。
 */

#include "evaluator.h"

#include <iostream>
#include "common.h"
#include "chess_engine.h"
#include "params.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ========== //
  // static変数 //
  // ========== //
  Bitboard Evaluator::start_position_[NUM_SIDES][NUM_PIECE_TYPES];
  Bitboard Evaluator::center_mask_;
  Bitboard Evaluator::sweet_center_mask_;
  Bitboard Evaluator::pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
  Bitboard Evaluator::iso_pawn_mask_[NUM_SQUARES];
  Bitboard Evaluator::pawn_shield_mask_[NUM_SIDES][NUM_SQUARES];
  Bitboard Evaluator::weak_square_mask_[NUM_SIDES][NUM_SQUARES];

  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
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

  // コピー代入演算子。
  Evaluator& Evaluator::operator=(const Evaluator& eval) {
    engine_ptr_ = eval.engine_ptr_;
    return *this;
  }

  // ムーブ代入演算子。
  Evaluator& Evaluator::operator=(Evaluator&& eval) {
    engine_ptr_ = eval.engine_ptr_;
    return *this;
  }

  // ======================= //
  // Evaluatorクラスの初期化 //
  // ======================= //
  // static変数の初期化。
  void Evaluator::InitEvaluator() {
    // start_position_[][]を初期化。
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

  // ============== //
  // パブリック関数 //
  // ============== //
  // 現在の局面の評価値を計算する。
  int Evaluator::Evaluate(int material) {
    // 価値の変数の初期化。
    for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
      opening_position_value_[piece_type] = 0.0;
      ending_position_value_[piece_type] = 0.0;
      mobility_value_[piece_type] = 0.0;
      center_control_value_[piece_type] = 0.0;
      sweet_center_control_value_[piece_type] = 0.0;
      development_value_[piece_type] = 0.0;
      attack_value_[piece_type] = 0.0;
      defense_value_[piece_type] = 0.0;
      pin_value_[piece_type] = 0.0;
      attack_around_king_value_[piece_type] = 0.0;
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
    rook_semiopen_fyle_value_ = 0.0;
    rook_open_fyle_value_ = 0.0;
    early_queen_launched_value_ = 0.0;
    weak_square_value_ = 0.0;
    castling_value_ = 0.0;
    abandoned_castling_value_ = 0.0;

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
    constexpr double NUM_KINGS = 2.0;
    double num_pieces = Util::CountBits(all_pieces) - NUM_KINGS;
    const EvalParams& params = engine_ptr_->eval_params();

    // マテリアル。
    double score = material;

    // 配列型のウェイトの評価。
    for (Piece piece_type = PAWN; piece_type <= KING; piece_type++) {
      score +=
      // オープニング時の駒の配置。
      (params.weight_opening_position()[piece_type](num_pieces)
      * opening_position_value_[piece_type])
      // エンディング時の駒の配置。
      + (params.weight_ending_position()[piece_type](num_pieces)
      * ending_position_value_[piece_type])
      // 機動力。
      + (params.weight_mobility()[piece_type](num_pieces)
      * mobility_value_[piece_type])
      // センターコントロール。
      + (params.weight_center_control()[piece_type](num_pieces)
      * center_control_value_[piece_type])
      // スウィートセンターのコントロール。
      + (params.weight_sweet_center_control()[piece_type](num_pieces)
      * sweet_center_control_value_[piece_type])
      // 駒の展開。
      + (params.weight_development()[piece_type](num_pieces)
      * development_value_[piece_type])
      // 攻撃。
      + (params.weight_attack()[piece_type](num_pieces)
      * attack_value_[piece_type])
      // 防御。
      + (params.weight_defense()[piece_type](num_pieces)
      * defense_value_[piece_type])
      // ピン。
      + (params.weight_pin()[piece_type](num_pieces)
      * pin_value_[piece_type])
      // 相手キング周辺への攻撃。
      + (params.weight_attack_around_king()[piece_type](num_pieces)
      * attack_around_king_value_[piece_type]);
    }

    // その他のウェイトの評価。
    score +=
    // パスポーン。
    + (params.weight_pass_pawn()(num_pieces) * pass_pawn_value_)
    // 守られたパスポーン。
    + (params.weight_protected_pass_pawn()(num_pieces)
    * protected_pass_pawn_value_)
    // ダブルポーン。
    + (params.weight_double_pawn()(num_pieces) * double_pawn_value_)
    // 孤立ポーン。
    + (params.weight_iso_pawn()(num_pieces) * iso_pawn_value_)
    // ポーンの盾。
    + (params.weight_pawn_shield()(num_pieces) * pawn_shield_value_)
    // ビショップペア。
    + (params.weight_bishop_pair()(num_pieces) * bishop_pair_value_)
    // バッドビショップ。
    + (params.weight_bad_bishop()(num_pieces) * bad_bishop_value_)
    // ルークペア。
    + (params.weight_rook_pair()(num_pieces) * rook_pair_value_)
    // セミオープンファイルのルーク。
    + (params.weight_rook_semiopen_fyle()(num_pieces)
    * rook_semiopen_fyle_value_)
    // オープンファイルのルーク。
    + (params.weight_rook_open_fyle()(num_pieces) * rook_open_fyle_value_)
    // 早すぎるクイーンの始動。
    + (params.weight_early_queen_launched()(num_pieces)
    * early_queen_launched_value_)
    // キング周りの弱いマス。
    + (params.weight_weak_square()(num_pieces) * weak_square_value_)
    // キャスリング。
    + (params.weight_castling()(num_pieces) * castling_value_)
    // キャスリングの放棄。
    + (params.weight_abandoned_castling()(num_pieces)
    * abandoned_castling_value_);

    return static_cast<int>(score);
  }

  // 現在の局面を評価し、構造体にして結果を返す。
  EvalResult Evaluator::GetEvalResult() {
    EvalResult result;

    // 総合評価値。
    result.score_ = Evaluate(engine_ptr_->GetMaterial(engine_ptr_->to_move()));

    const EvalParams& params = engine_ptr_->eval_params();
    double num_pieces = Util::CountBits(engine_ptr_->blocker_0()) - 2;

    // マテリアル。
    result.material_ = engine_ptr_->GetMaterial(engine_ptr_->to_move());

    for (Piece piece_type = 0; piece_type < NUM_PIECE_TYPES; piece_type++) {
      // オープニング時の駒の配置の評価値。
      result.score_opening_position_[piece_type] =
      params.weight_opening_position()[piece_type](num_pieces)
      * opening_position_value_[piece_type];

      // エンディング時の駒の配置の評価値。
      result.score_ending_position_[piece_type] =
      params.weight_ending_position()[piece_type](num_pieces)
      * ending_position_value_[piece_type];

      // 機動力の評価値。
      result.score_mobility_[piece_type] =
      params.weight_mobility()[piece_type](num_pieces)
      * mobility_value_[piece_type];

      // センターコントロールの評価値。
      result.score_center_control_[piece_type] =
      params.weight_center_control()[piece_type](num_pieces)
      * center_control_value_[piece_type];

      // スウィートセンターのコントロールの評価値。
      result.score_sweet_center_control_[piece_type] =
      params.weight_sweet_center_control()[piece_type](num_pieces)
      * sweet_center_control_value_[piece_type];

      // 駒の展開の評価値。
      result.score_development_[piece_type] =
      params.weight_development()[piece_type](num_pieces)
      * development_value_[piece_type];

      // 攻撃の評価値。
      result.score_attack_[piece_type] =
      params.weight_attack()[piece_type](num_pieces)
      * attack_value_[piece_type];

      // 防御の評価値。
      result.score_defense_[piece_type] =
      params.weight_defense()[piece_type](num_pieces)
      * defense_value_[piece_type];

      // ピンの評価値。
      result.score_pin_[piece_type] =
      params.weight_pin()[piece_type](num_pieces)
      * pin_value_[piece_type];

      // 相手キング周辺への攻撃の評価値。
      result.score_attack_around_king_[piece_type] =
      params.weight_attack_around_king()[piece_type](num_pieces)
      * attack_around_king_value_[piece_type];
    }

    // パスポーンの評価値。
    result.score_pass_pawn_ = params.weight_pass_pawn()(num_pieces)
    * pass_pawn_value_;

    // 守られたパスポーンの評価値。
    result.score_protected_pass_pawn_ = params.weight_protected_pass_pawn()
    (num_pieces) * protected_pass_pawn_value_;

    // ダブルポーンの評価値。
    result.score_double_pawn_ = params.weight_double_pawn()(num_pieces)
    * double_pawn_value_;

    // 孤立ポーンの評価値。
    result.score_iso_pawn_ = params.weight_iso_pawn()(num_pieces)
    * iso_pawn_value_;

    // ポーンの盾の評価値。
    result.score_pawn_shield_ = params.weight_pawn_shield()(num_pieces)
    * pawn_shield_value_;

    // ビショップペアの評価値。
    result.score_bishop_pair_ = params.weight_bishop_pair()(num_pieces)
    * bishop_pair_value_;

    // バッドビショップの評価値。
    result.score_bad_bishop_ = params.weight_bad_bishop()(num_pieces)
    * bad_bishop_value_;

    // ルークペアの評価値。
    result.score_rook_pair_ = params.weight_rook_pair()(num_pieces)
    * rook_pair_value_;

    // セミオープンファイルのルークの評価値。
    result.score_rook_semiopen_fyle_ = params.weight_rook_semiopen_fyle()
    (num_pieces) * rook_semiopen_fyle_value_;

    // オープンファイルのルークの評価値。
    result.score_rook_open_fyle_ = params.weight_rook_open_fyle()(num_pieces)
    * rook_open_fyle_value_;

    // 早すぎるクイーンの始動の評価値。
    result.score_early_queen_launched_ = params.weight_early_queen_launched()
    (num_pieces) * early_queen_launched_value_;

    // キング周りの弱いマスの評価値。
    result.score_weak_square_ = params.weight_weak_square()(num_pieces)
    * weak_square_value_;

    // キャスリングの評価値。
    result.score_castling_ = params.weight_castling()(num_pieces)
    * castling_value_;

    // キャスリングの放棄の評価値。
    result.score_abandoned_castling_ = params.weight_abandoned_castling()
    (num_pieces) * abandoned_castling_value_;

    return result;
  }

  // ====================== //
  // 局面評価に使用する関数 //
  // ====================== //
  // メイトに必要な駒があるかどうか調べる。
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

  // ================== //
  // 価値を計算する関数 //
  // ================== //
  // 各駒の価値を計算する。
  template<Piece Type>
  void Evaluator::CalValue(Square piece_square, Side piece_side) {
    // サイド。
    Side enemy_piece_side = piece_side ^ 0x3;

    // 値と符号。自分の駒ならプラス。敵の駒ならマイナス。
    double value;
    double sign = piece_side == engine_ptr_->to_move() ? 1.0 : -1.0;

    // 評価関数用パラメータを得る。
    const EvalParams& params = engine_ptr_->eval_params();

    // 利き筋を作る。
    Bitboard attacks = 0;
    Bitboard pawn_moves = 0;
    Bitboard en_passant = 0;
    Bitboard castling_moves = 0;
    switch (Type) {
      case PAWN:
        // 通常の動き。
        pawn_moves = engine_ptr_->GetPawnStep(piece_side, piece_square);

        // 攻撃。
        attacks = Util::GetPawnAttack(piece_square, piece_side);

        // アンパッサン。
        if (engine_ptr_->en_passant_square()
        && (piece_side == engine_ptr_->to_move())) {
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
        castling_moves = 0;
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

    // --- 全駒共通 --- //
    // オープニング時の駒の配置を計算。
    if (piece_side == WHITE) {
      opening_position_value_[Type] += sign
      * params.opening_position_value_table()[Type][piece_square];
    } else {
      opening_position_value_[Type] += sign
      * params.opening_position_value_table()[Type][Util::FLIP[piece_square]];
    }

    // エンディング時の駒の配置を計算。
    if (piece_side == WHITE) {
      ending_position_value_[Type] += sign
      * params.ending_position_value_table()[Type][piece_square];
    } else {
      ending_position_value_[Type] += sign
      * params.ending_position_value_table()[Type][Util::FLIP[piece_square]];
    }

    // 機動力を計算。
    if (Type == PAWN) {
      mobility_value_[Type] += sign * Util::CountBits((attacks
      & engine_ptr_->side_pieces()[enemy_piece_side])
      | pawn_moves | en_passant);
    } else {
      mobility_value_[Type] += sign * Util::CountBits(attacks
      & ~(engine_ptr_->side_pieces()[piece_side]));
    }

    // センターコントロールを計算。
    center_control_value_[Type] +=
    sign * Util::CountBits(attacks & center_mask_);

    // スウィートセンターのコントロールを計算。
    sweet_center_control_value_[Type] += sign
    * Util::CountBits(attacks & sweet_center_mask_);

    // 駒の展開を計算。
    if (!(Util::SQUARE[piece_square] & start_position_[piece_side][Type])) {
      development_value_[Type] += sign * 1.0;
    }

    // 敵への攻撃を計算。
    Bitboard attacked =
    attacks & (engine_ptr_->side_pieces()[enemy_piece_side]);
    value = 0.0;
    const double (& table_1)[NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
    params.attack_value_table();
    for (; attacked; attacked &= attacked - 1) {
      value +=
      table_1[Type][engine_ptr_->piece_board()[Util::GetSquare(attacked)]];
    }
    if ((Type == PAWN) && en_passant) {
      value += table_1[PAWN][PAWN];
    }
    attack_value_[Type] += sign * value;

    // 味方への防御を計算。
    Bitboard defensed = attacks & (engine_ptr_->side_pieces()[piece_side]);
    value = 0.0;
    const double (& table_2)[NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
    params.defense_value_table();
    for (; defensed; defensed &= defensed - 1) {
      value +=
      table_2[Type][engine_ptr_->piece_board()[Util::GetSquare(defensed)]];
    }
    defense_value_[Type] += sign * value;

    // ピンを計算。
    if ((Type == BISHOP) || (Type == ROOK) || (Type == QUEEN)) {
      value = 0.0;

      // ピンの対象候補。
      Bitboard pin_target =
      attacks & engine_ptr_->side_board()[enemy_piece_side];

      // ピンの裏駒候補。
      Bitboard pin_back = 0;
      if (Type == BISHOP) {
        pin_back =
        (engine_ptr_->side_board()[enemy_piece_side]
        & Util::GetBishopMove(piece_square)) & ~pin_target;
      } else if (Type == ROOK) {
        pin_back =
        (engine_ptr_->side_board()[enemy_piece_side]
        & Util::GetRookMove(piece_square)) & ~pin_target;
      } else {
        pin_back =
        (engine_ptr_->side_board()[enemy_piece_side]
        & Util::GetQueenMove(piece_square)) & ~pin_target;
      }

      // ピンを判定。
      for (Bitboard bb = pin_back; bb; bb &= bb - 1) {
        // 裏駒のマス。
        Square pin_back_sq = Util::GetSquare(bb);

        // 裏駒と自分の間のビットボード。
        Bitboard between = Util::GetBetween(piece_square, pin_back_sq);

        // 下のif文によるピンの判定条件は、
        // 「裏駒と自分との間」に「ピンの対象」が一つのみだった場合。
        if ((between & pin_target) && !(between & pin_back)) {
          value += params.pin_value_table()[Type]
          [engine_ptr_->piece_board()[Util::GetSquare(between & pin_target)]]
          [engine_ptr_->piece_board()[pin_back_sq]];
        }
      }

      pin_value_[Type] += sign * value;
    }

    // 相手キング周辺への攻撃を計算。
    attack_around_king_value_[Type] += sign * Util::CountBits(attacks
    & Util::GetKingMove(engine_ptr_->king()[enemy_piece_side]));

    // --- ポーン --- //
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
          pawn_shield_value_ += sign
          * params.pawn_shield_value_table()[piece_square];
        } else {
          pawn_shield_value_ += sign
          * params.pawn_shield_value_table()[Util::FLIP[piece_square]];
        }
      }
    }

    // --- ビショップ --- //
    if (Type == BISHOP) {
      // バッドビショップを計算。
      if ((Util::SQUARE[piece_square] & Util::SQCOLOR[WHITE])) {
        bad_bishop_value_ += sign * Util::CountBits
        (engine_ptr_->position()[piece_side][PAWN] & Util::SQCOLOR[WHITE]);
      } else {
        bad_bishop_value_ += sign * Util::CountBits
        (engine_ptr_->position()[piece_side][PAWN] & Util::SQCOLOR[BLACK]);
      }
    }

    // --- ルーク --- //
    if (Type == ROOK) {
      Bitboard rook_fyle = Util::FYLE[Util::GetFyle(piece_square)];
      // セミオープン。
      if (!(engine_ptr_->position()[piece_side][PAWN] & rook_fyle)) {
        rook_semiopen_fyle_value_ += sign * 1.0;
        // オープン。
        if (!(engine_ptr_->position()[enemy_piece_side][PAWN] & rook_fyle)) {
          rook_open_fyle_value_ += sign * 1.0;
        }
      }
    }

    // --- クイーン --- //
    if (Type == QUEEN) {
      // クイーンの早過ぎる始動を計算。
      value = 0.0;
      if (!(Util::SQUARE[piece_square]
      & start_position_[piece_side][QUEEN])) {
        value += Util::CountBits(engine_ptr_->position()[piece_side][KNIGHT]
        & start_position_[piece_side][KNIGHT]);
        value += Util::CountBits(engine_ptr_->position()[piece_side][BISHOP]
        & start_position_[piece_side][BISHOP]);
      }
      early_queen_launched_value_ += sign * value;
    }

    // --- キング --- //
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
      value += Util::CountBits(engine_ptr_->position()
      [enemy_piece_side][BISHOP] & Util::SQCOLOR[WHITE]) * white_weak;
      // 相手の黒マスビショップの数と弱い黒マスの数を掛け算。
      value += Util::CountBits(engine_ptr_->position()
      [enemy_piece_side][BISHOP] & Util::SQCOLOR[BLACK]) * black_weak;
      // 評価値にする。
      weak_square_value_ += sign * value;

      // キャスリングを計算する。
      Castling rights_mask =
      piece_side == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
      if (engine_ptr_->has_castled()[piece_side]) {
        // キャスリングした。
        castling_value_ += sign * 1.0;
      } else {
        if (!(engine_ptr_->castling_rights() & rights_mask)) {
          // キャスリングの権利を放棄した。
          abandoned_castling_value_ += sign * 1.0;
        }
      }
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

  // ======================== //
  // その他のプライベート関数 //
  // ======================== //
  // start_position[][]を初期化。
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

  // center_mask_、sweet_center_mask_を初期化する。
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
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        Bitboard mask = 0;
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
    for (Square square = 0; square < NUM_SQUARES; square++) {
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
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        if (side == NO_SIDE) {  // どちらのサイドでもなければ空。
          pawn_shield_mask_[side][square] = 0;
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
            pawn_shield_mask_[side][square] = 0;
          }
        }
      }
    }
  }

  // weak_square_mask_[][]を初期化する。
  void Evaluator::InitWeakSquareMask() {
    for (Side side = 0; side < NUM_SIDES; side++) {
      for (Square square = 0; square < NUM_SQUARES; square++) {
        if (side == NO_SIDE) {  // どちらのサイドでもなければ空。
          weak_square_mask_[side][square] = 0;
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
            weak_square_mask_[side][square] = 0;
          }
        }
      }
    }
  }
}  // namespace Sayuri
