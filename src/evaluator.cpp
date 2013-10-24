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
  constexpr int Evaluator::POSITION_TABLE[NUM_PIECE_TYPES][NUM_SQUARES];
  constexpr int Evaluator::KING_POSITION_MIDDLE_TABLE[NUM_SQUARES];
  constexpr int Evaluator::KING_POSITION_ENDING_TABLE[NUM_SQUARES];
  constexpr int Evaluator::WEIGHT_MOBILITY;
  constexpr int Evaluator::WEIGHT_CENTER_CONTROL;
  constexpr int Evaluator::WEIGHT_DEVELOPMENT;
  constexpr int Evaluator::WEIGHT_ATTACK_ENEMY;
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

  /**********************/
  /* ファイルスコープ。 */
  /**********************/
  namespace {
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
  }  // namespace

  /**************************/
  /* コンストラクタと代入。 */
  /**************************/
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

  // 評価値を返す。
  int Evaluator::Evaluate() {
    // 価値の変数の初期化。
    material_value_ = 0;
    mobility_value_ = 0;
    center_control_value_ = 0;
    sweet_center_control_value_ = 0;
    development_value_ = 0;
    attack_enemy_value_ = 0;
    attack_around_king_value_ = 0;
    for (int i = 0; i < NUM_PIECE_TYPES; i++) {
      position_value_[i] = 0;
    }
    king_position_middle_value_ = 0;
    king_position_ending_value_ = 0;
    pass_pawn_value_ = 0;
    protected_pass_pawn_value_ = 0;
    double_pawn_value_ = 0;
    iso_pawn_value_ = 0;
    bishop_pair_value_ = 0;
    early_queen_launched_value_ = 0;
    pawn_shield_value_ = 0;
    castling_value_ = 0;

    // サイド。
    Side side = engine_ptr_->to_move();
    Side enemy_side = side ^ 0x3;

    // 合法手がないとき。
    if (!(engine_ptr_->HasLegalMove(side))) {
      if (engine_ptr_->IsAttacked(engine_ptr_->king()[side], enemy_side)) {
        // チェックメイトされていれば負け。
        return SCORE_LOSE;
      } else {
        // ステールメイトは引き分け。
        return SCORE_DRAW;
      }
    }

    // 十分な駒がない場合は引き分け。
    if (!HasEnoughPieces(side) && !HasEnoughPieces(enemy_side)) {
      return SCORE_DRAW;
    }

    // 全体計算。
    // マテリアル。
    material_value_ = engine_ptr_->GetMaterial(side);
    // ビショップペア。
    if (Util::CountBits(engine_ptr_->position()[side][BISHOP]) >= 2) {
      bishop_pair_value_ += 1;
    }
    if (Util::CountBits(engine_ptr_->position()[enemy_side][BISHOP]) >= 2) {
      bishop_pair_value_ -= 1;
    }

    // 各駒毎に価値を計算する。
    Square piece_square;
    Side piece_side;
    Bitboard pieces = engine_ptr_->blocker_0();
    for (; pieces; pieces &= pieces - 1) {
      piece_square = Util::GetSquare(pieces);
      piece_side = engine_ptr_->side_board()[piece_square];
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

    // フェーズ。
    double middle_game = GetPhase();
    double ending = 1.0 - middle_game;

    // フェーズ毎に得点を足し算する。
    // 全フェーズ。
    int whole_score = material_value_
    + (WEIGHT_MOBILITY * mobility_value_)
    + (WEIGHT_ATTACK_ENEMY * attack_enemy_value_)
    + (WEIGHT_ATTACK_AROUND_KING * attack_around_king_value_)
    + (WEIGHT_PAWN_POSITION * position_value_[PAWN])
    + (WEIGHT_PASS_PAWN * pass_pawn_value_)
    + (WEIGHT_PROTECTED_PASS_PAWN * protected_pass_pawn_value_)
    + (WEIGHT_DOUBLE_PAWN * double_pawn_value_)
    + (WEIGHT_BISHOP_PAIR * bishop_pair_value_);

    // 中盤。
    int middle_score = (WEIGHT_CENTER_CONTROL * center_control_value_)
    + (WEIGHT_DEVELOPMENT * development_value_)
    + (WEIGHT_KNIGHT_POSITION * position_value_[KNIGHT])
    + (WEIGHT_BISHOP_POSITION * position_value_[BISHOP])
    + (WEIGHT_ROOK_POSITION * position_value_[ROOK])
    + (WEIGHT_QUEEN_POSITION * position_value_[QUEEN])
    + (WEIGHT_KING_POSITION_MIDDLE * king_position_middle_value_)
    + (WEIGHT_ISO_PAWN * iso_pawn_value_)
    + (WEIGHT_EARLY_QUEEN_LAUNCHED * early_queen_launched_value_)
    + (WEIGHT_PAWN_SHIELD * pawn_shield_value_)
    + (WEIGHT_CASTLING * castling_value_);

    // 終盤。
    int ending_score =
    (WEIGHT_KING_POSITION_ENDING * king_position_ending_value_);

    return whole_score +
    static_cast<int>((middle_game * middle_score) + (ending * ending_score));
  }

  /****************************/
  /* 局面評価に使用する関数。 */
  /****************************/
  // 勝つのに十分な駒があるかどうか調べる。
  bool Evaluator::HasEnoughPieces(Side side) {
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
  double Evaluator::GetPhase() {
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

    // スコアと符号。自分の駒ならプラス。敵の駒ならマイナス。
    int score;
    int sign = piece_side == engine_ptr_->to_move() ? 1 : -1;

    // 利き筋を作る。
    Bitboard attacks = 0ULL;
    Bitboard pawn_moves = 0ULL;
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
        attacks = Util::GetPawnAttack(piece_square, piece_side)
        & engine_ptr_->side_pieces()[enemy_piece_side];
        // アンパッサン。
        if (engine_ptr_->can_en_passant()) {
          attacks |= Util::BIT[engine_ptr_->en_passant_square()]
          & Util::GetPawnAttack(piece_square, piece_side);
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
            castling_moves |= Util::BIT[G1];
          }
          if (engine_ptr_->CanCastling<WHITE_LONG_CASTLING>()) {
            castling_moves |= Util::BIT[C1];
          }
        } else {
          if (engine_ptr_->CanCastling<BLACK_SHORT_CASTLING>()) {
            castling_moves |= Util::BIT[G8];
          }
          if (engine_ptr_->CanCastling<BLACK_LONG_CASTLING>()) {
            castling_moves |= Util::BIT[C8];
          }
        }
        break;
      default:
        throw SayuriError("駒の種類が不正です。");
        break;
    }

    // 駒の動きやすさを計算。
    if (Type == PAWN) {
      score = Util::CountBits(pawn_moves | attacks);
    } else if (Type == KING) {
      score = Util::CountBits(castling_moves
      | (attacks & ~(engine_ptr_->side_pieces()[piece_side])));
    } else {
      score = Util::CountBits(attacks
      & ~(engine_ptr_->side_pieces()[piece_side]));
    }
    mobility_value_ += sign * score;

    // センター支配を計算。
    if (Type != KING) {
      score = Util::CountBits(attacks & center_mask_);
      center_control_value_ += sign * score;
      score = Util::CountBits(attacks & sweet_center_mask_);
      sweet_center_control_value_ += sign * score;
    }

    // 駒の展開を計算。
    if ((Type == KNIGHT) || (Type == BISHOP)) {
      score = 0;
      if (Util::BIT[piece_square] & ~(start_position_[piece_side][Type])) {
        score += 1;
      }
      development_value_ += sign * score;
    }

    // 敵への攻撃を計算。
    if (Type == PAWN) {
      score = Util::CountBits(attacks
      & (engine_ptr_->side_pieces()[enemy_piece_side]
      | Util::BIT[engine_ptr_->en_passant_square()]));
    } else if (Type != KING) {
      score = Util::CountBits(attacks
      & engine_ptr_->side_pieces()[enemy_piece_side]);
    }
    attack_enemy_value_ = sign * score;

    // 敵キング周辺への攻撃を計算。
    if (Type != KING) {
      score = Util::CountBits(attacks
      & Util::GetKingMove(engine_ptr_->king()[enemy_piece_side]));
      attack_around_king_value_ += sign * score;
    }

    // 駒の配置を計算。
    if (Type == KING) {
      // キングの中盤の配置を計算。
      if (piece_side == WHITE) {
        score = KING_POSITION_MIDDLE_TABLE[piece_square];
      } else {
        score = KING_POSITION_MIDDLE_TABLE[FLIP[piece_square]];
      }
      king_position_middle_value_ += sign * score;

      // キングの終盤の配置を計算。
      if (piece_side == WHITE) {
        score = KING_POSITION_ENDING_TABLE[piece_square];
      } else {
        score = KING_POSITION_ENDING_TABLE[FLIP[piece_square]];
      }
      king_position_ending_value_ += sign * score;
    } else {
      if (piece_side == WHITE) {
        score = POSITION_TABLE[Type][piece_square];
      } else {
        score = POSITION_TABLE[Type][FLIP[piece_square]];
      }
      position_value_[Type] += sign * score;
    }

    // ポーンの構成を計算。
    if (Type == PAWN) {
      // パスポーンを計算。
      score = 0;
      int score_2 = 0;
      if (!(engine_ptr_->position()[enemy_piece_side][PAWN]
      & pass_pawn_mask_[piece_side][piece_square])) {
        score += 1;
        // 守られたパスポーン。
        if (engine_ptr_->position()[piece_side][PAWN]
        & Util::GetPawnAttack(piece_square, enemy_piece_side)) {
          score_2 += 1;
        }
      }
      pass_pawn_value_ += sign * score;
      protected_pass_pawn_value_ += sign * score_2;

      // ダブルポーンを計算。
      int fyle = Util::GetFyle(piece_square);
      score = 0;
      if (Util::CountBits(engine_ptr_->position()[piece_side][PAWN]
      & Util::FYLE[fyle]) >= 2) {
        score += 1;
      }
      double_pawn_value_ += sign * score;

      // 孤立ポーンを計算。
      score = 0;
      if (!(engine_ptr_->position()[piece_side][PAWN]
      & iso_pawn_mask_[piece_square])) {
        score += 1;
      }
      iso_pawn_value_ += sign * score;
    }

    // クイーンの早過ぎる始動を計算。
    if (Type == QUEEN) {
      score = 0;
      if (Util::BIT[piece_square] & ~(start_position_[piece_side][QUEEN])) {
        score += Util::CountBits(engine_ptr_->position()[piece_side][KNIGHT]
        & start_position_[piece_side][KNIGHT]);
        score += Util::CountBits(engine_ptr_->position()[piece_side][BISHOP]
        & start_position_[piece_side][BISHOP]);
      }
      early_queen_launched_value_ += sign * score;
    }

    // ポーンシールドとキャスリングを計算。
    if (Type == KING) {
      // ポーンの盾を計算する。
      score = Util::CountBits(engine_ptr_->position()[piece_side][PAWN]
      & pawn_shield_mask_[piece_side][piece_square]);
      pawn_shield_value_ += sign * score;

      // キャスリングを計算する。
      score = 1;  // キャスリングはまだだが、放棄していない。
      Castling rights_mask =
      piece_side == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
      if (engine_ptr_->has_castled()[piece_side]) {
        // キャスリングした。
        score = 2;
      } else {
        if (!(engine_ptr_->castling_rights() & rights_mask)) {
          // キャスリングの権利を放棄した。
          score = 0;
        }
      }
      castling_value_ += sign * score;
    }
  }
  // 実体化。
  template void Evaluator::CalValue<PAWN>(Square piece_type, Side piece_side);
  template void Evaluator::CalValue<KNIGHT>(Square piece_type, Side piece_side);
  template void Evaluator::CalValue<BISHOP>(Square piece_type, Side piece_side);
  template void Evaluator::CalValue<ROOK>(Square piece_type, Side piece_side);
  template void Evaluator::CalValue<QUEEN>(Square piece_type, Side piece_side);
  template void Evaluator::CalValue<KING>(Square piece_type, Side piece_side);

  /******************************/
  /* その他のプライベート関数。 */
  /******************************/
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
