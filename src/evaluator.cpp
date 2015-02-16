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
  // static定数 //
  // ========== //
  constexpr unsigned int Evaluator::OPENING_POSITION;
  constexpr unsigned int Evaluator::ENDING_POSITION;
  constexpr unsigned int Evaluator::MOBILITY;
  constexpr unsigned int Evaluator::CENTER_CONTROL;
  constexpr unsigned int Evaluator::SWEET_CENTER_CONTROL;
  constexpr unsigned int Evaluator::DEVELOPMENT;
  constexpr unsigned int Evaluator::ATTACK;
  constexpr unsigned int Evaluator::DEFENSE;
  constexpr unsigned int Evaluator::PIN;
  constexpr unsigned int Evaluator::ATTACK_AROUND_KING;
  constexpr unsigned int Evaluator::PASS_PAWN;
  constexpr unsigned int Evaluator::PROTECTED_PASS_PAWN;
  constexpr unsigned int Evaluator::DOUBLE_PAWN;
  constexpr unsigned int Evaluator::ISO_PAWN;
  constexpr unsigned int Evaluator::PAWN_SHIELD;
  constexpr unsigned int Evaluator::BISHOP_PAIR;
  constexpr unsigned int Evaluator::BAD_BISHOP;
  constexpr unsigned int Evaluator::ROOK_PAIR;
  constexpr unsigned int Evaluator::ROOK_SEMIOPEN_FYLE;
  constexpr unsigned int Evaluator::ROOK_OPEN_FYLE;
  constexpr unsigned int Evaluator::EARLY_QUEEN_LAUNCHED;
  constexpr unsigned int Evaluator::WEAK_SQUARE;
  constexpr unsigned int Evaluator::CASTLING;
  constexpr unsigned int Evaluator::ABANDONED_CASTLING;
  constexpr std::size_t Evaluator::TABLE_SIZE;

  // ================ //
  // テンプレート部品 //
  // ================ //
  // 白番なら加算、黒番なら減算するテンプレート部品。
  template<Side PSide>
  inline void AddOrSub(double& dst, double value) {}
  template<>
  inline void AddOrSub<WHITE>(double& dst, double value) {
    dst += value;
  }
  template<>
  inline void AddOrSub<BLACK>(double& dst, double value) {
    dst -= value;
  }

  // 評価用ビットボードを作成するテンプレート部品。
  template<Side PSide, PieceType PType>
  struct GenBitboards {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {}
  };
  template<Side PSide>
  struct GenBitboards<PSide, PAWN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      // 通常の動き。
      pawn_moves = engine.GetPawnStep(PSide, square);

      // 攻撃。
      attacks = Util::GetPawnAttack(PSide, square);

      // アンパッサン。
      if (engine.en_passant_square_ && (PSide == engine.to_move_)) {
        en_passant = Util::SQUARE[engine.en_passant_square_] & attacks;
      }
    }
  };
  template<Side PSide>
  struct GenBitboards<PSide, KNIGHT> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = Util::GetKnightMove(square);
    }
  };
  template<Side PSide>
  struct GenBitboards<PSide, BISHOP> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = evaluator.engine_ptr_->GetBishopAttack(square);
    }
  };
  template<Side PSide>
  struct GenBitboards<PSide, ROOK> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = evaluator.engine_ptr_->GetRookAttack(square);
    }
  };
  template<Side PSide>
  struct GenBitboards<PSide, QUEEN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = evaluator.engine_ptr_->GetQueenAttack(square);
    }
  };
  template<Side PSide>
  struct GenBitboards<PSide, KING> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = Util::GetKingMove(square);
    }
  };

  // 駒の配置価値を計算するテンプレート部品。
  template<Side PSide, PieceType PType>
  struct CalPosition {
    static void F(Evaluator& evaluator, const EvalParams& params,
    Square square) {}
  };
  template<PieceType PType>
  struct CalPosition<WHITE, PType> {
    static void F(Evaluator& evaluator, const EvalParams& params,
    Square square) {
      // オープニング。
      evaluator.value_table_[Evaluator::OPENING_POSITION][PType] +=
      params.opening_position_value_table_[PType][square];

      // エンディング。
      evaluator.value_table_[Evaluator::ENDING_POSITION][PType] +=
      params.ending_position_value_table_[PType][square];
    }
  };
  template<PieceType PType>
  struct CalPosition<BLACK, PType> {
    static void F(Evaluator& evaluator, const EvalParams& params,
    Square square) {
      // オープニング。
      evaluator.value_table_[Evaluator::OPENING_POSITION][PType] -=
      params.opening_position_value_table_[PType][Util::FLIP[square]];

      // エンディング。
      evaluator.value_table_[Evaluator::ENDING_POSITION][PType] -=
      params.ending_position_value_table_[PType][Util::FLIP[square]];
    }
  };

  // 駒の機動力を計算するテンプレート部品。
  template<Side PSide, PieceType PType>
  struct CalMobility {
    static void F(Evaluator& evaluator, Bitboard attacks, Bitboard pawn_moves,
    Bitboard en_passant) {
      AddOrSub<PSide>(evaluator.value_table_[Evaluator::MOBILITY][PType],
      Util::CountBits(attacks
      & ~(evaluator.engine_ptr_->side_pieces_[PSide])));
    }
  };
  template<Side PSide>
  struct CalMobility<PSide, PAWN> {
    static void F(Evaluator& evaluator, Bitboard attacks, Bitboard pawn_moves,
    Bitboard en_passant) {
      constexpr Side EnemySide = Util::GetOppositeSide(PSide);

      AddOrSub<PSide>(evaluator.value_table_[Evaluator::MOBILITY][PAWN],
      Util::CountBits((attacks
      & evaluator.engine_ptr_->side_pieces_[EnemySide]) | pawn_moves
      | en_passant));
    }
  };

  // ピンのターゲットや裏駒を抽出するテンプレート部品。
  template<Side PSide, PieceType PType>
  struct GenPinTargets {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard attacks, Bitboard& target, Bitboard& back) {}
  };
  template<Side PSide>
  struct GenPinTargets<PSide, BISHOP> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard attacks, Bitboard& target, Bitboard& back) {
      constexpr Side EnemySide = Util::GetOppositeSide(PSide);

      target = attacks & engine.side_board_[EnemySide];

      back = (engine.side_board_[EnemySide]
      & Util::GetBishopMove(square)) & ~target;
    }
  };
  template<Side PSide>
  struct GenPinTargets<PSide, ROOK> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard attacks, Bitboard& target, Bitboard& back) {
      constexpr Side EnemySide = Util::GetOppositeSide(PSide);

      target = attacks & engine.side_board_[EnemySide];

      back = (engine.side_board_[EnemySide]
      & Util::GetRookMove(square)) & ~target;
    }
  };
  template<Side PSide>
  struct GenPinTargets<PSide, QUEEN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard attacks, Bitboard& target, Bitboard& back) {
      constexpr Side EnemySide = Util::GetOppositeSide(PSide);

      target = attacks & engine.side_board_[EnemySide];

      back = (engine.side_board_[EnemySide]
      & Util::GetQueenMove(square)) & ~target;
    }
  };

  // 各駒専用の価値を計算するテンプレート部品。
  template<Side PSide, PieceType PType>
  struct CalSpecial {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {}
  };
  template<Side PSide>
  struct CalSpecial<PSide, PAWN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {
      constexpr Side EnemySide = Util::GetOppositeSide(PSide);

      // パスポーンを計算。
      if (!(engine.position_[EnemySide][PAWN]
      & evaluator.pass_pawn_mask_[PSide][square])) {
        AddOrSub<PSide>(evaluator.value_table_[Evaluator::PASS_PAWN][0],
        1.0);

        // 守られたパスポーン。
        if (engine.position_[PSide][PAWN]
        & Util::GetPawnAttack(EnemySide, square)) {
          AddOrSub<PSide>
          (evaluator.value_table_[Evaluator::PROTECTED_PASS_PAWN][0], 1.0);
        }
      }

      // ダブルポーンを計算。
      if (Util::CountBits(engine.position_[PSide][PAWN]
      & Util::FYLE[Util::SQUARE_TO_FYLE[square]]) >= 2) {
        AddOrSub<PSide>(evaluator.value_table_[Evaluator::DOUBLE_PAWN][0],
        1.0);
      }

      // 孤立ポーンを計算。
      if (!(engine.position_[PSide][PAWN]
      & evaluator.iso_pawn_mask_[square])) {
        AddOrSub<PSide>(evaluator.value_table_[Evaluator::ISO_PAWN][0],
        1.0);
      }

      // ポーンの盾を計算。
      if ((Util::SQUARE[square]
      & evaluator.pawn_shield_mask_[PSide][engine.king_[PSide]])) {
        if (PSide == WHITE) {
          evaluator.value_table_[Evaluator::PAWN_SHIELD][0] +=
          engine.eval_params().pawn_shield_value_table_[square];
        } else {
          evaluator.value_table_[Evaluator::PAWN_SHIELD][0] -=
          engine.eval_params().pawn_shield_value_table_[Util::FLIP[square]];
        }
      }
    }
  };
  template<Side PSide>
  struct CalSpecial<PSide, BISHOP> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {
      // バッドビショップを計算。
      if ((Util::SQUARE[square] & Util::SQCOLOR[WHITE])) {
        AddOrSub<PSide>(evaluator.value_table_[Evaluator::BAD_BISHOP][0],
        Util::CountBits(engine.position_[PSide][PAWN] & Util::SQCOLOR[WHITE]));
      } else {
        AddOrSub<PSide>(evaluator.value_table_[Evaluator::BAD_BISHOP][0],
        Util::CountBits(engine.position_[PSide][PAWN]
        & Util::SQCOLOR[BLACK]));
      }
    }
  };
  template<Side PSide>
  struct CalSpecial<PSide, ROOK> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {
      constexpr Side EnemySide = Util::GetOppositeSide(PSide);

      // オープンファイルとセミオープンファイルを計算。
      Bitboard rook_fyle = Util::FYLE[Util::SQUARE_TO_FYLE[square]];
      if (!(engine.position_[PSide][PAWN] & rook_fyle)) {
        // セミオープン。
        AddOrSub<PSide>
        (evaluator.value_table_[Evaluator::ROOK_SEMIOPEN_FYLE][0], 1.0);
        if (!(engine.position_[EnemySide][PAWN]
        & rook_fyle)) {
          // オープン。
          AddOrSub<PSide>
          (evaluator.value_table_[Evaluator::ROOK_OPEN_FYLE][0], 1.0);
        }
      }
    }
  };
  template<Side PSide>
  struct CalSpecial<PSide, QUEEN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {

      // クイーンの早過ぎる始動を計算。
      double value = 0.0;
      if (!(Util::SQUARE[square] & evaluator.start_position_[PSide][QUEEN])) {
        value +=
        Util::CountBits(engine.position_[PSide][KNIGHT]
        & evaluator.start_position_[PSide][KNIGHT]);

        value +=
        Util::CountBits(engine.position_[PSide][BISHOP]
        & evaluator.start_position_[PSide][BISHOP]);
      }

      AddOrSub<PSide>
      (evaluator.value_table_[Evaluator::EARLY_QUEEN_LAUNCHED][0], value);
    }
  };
  template<Side PSide>
  struct CalSpecial<PSide, KING> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {
      constexpr Side EnemySide = Util::GetOppositeSide(PSide);

      // --- キング周りの弱いマスを計算 --- //
      double value = 0.0;

      // 弱いマス。
      Bitboard weak = (~(engine.position_[PSide][PAWN]))
      & evaluator.weak_square_mask_[PSide][square];

      // それぞれの色のマスの弱いマスの数。
      int white_weak = Util::CountBits(weak & Util::SQCOLOR[WHITE]);
      int black_weak = Util::CountBits(weak & Util::SQCOLOR[BLACK]);

      // 相手の白マスビショップの数と弱い白マスの数を掛け算。
      value += Util::CountBits(engine.position_[EnemySide][BISHOP]
      & Util::SQCOLOR[WHITE]) * white_weak;

      // 相手の黒マスビショップの数と弱い黒マスの数を掛け算。
      value += Util::CountBits(engine.position_[EnemySide][BISHOP]
      & Util::SQCOLOR[BLACK]) * black_weak;

      // 評価値にする。
      AddOrSub<PSide>(evaluator.value_table_[Evaluator::WEAK_SQUARE][0],
      value);

      // --- キャスリングを計算する --- //
      Castling rights_mask = PSide == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
      if (engine.has_castled_[PSide]) {
        // キャスリングした。
        AddOrSub<PSide>(evaluator.value_table_[Evaluator::CASTLING][0],
        1.0);
      } else {
        if (!(engine.castling_rights_ & rights_mask)) {
          // キャスリングの権利を放棄した。
          AddOrSub<PSide>
          (evaluator.value_table_[Evaluator::ABANDONED_CASTLING][0], 1.0);
        }
      }
    }
  };

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
    INIT_ARRAY(value_table_);
  }

  // コピーコンストラクタ。
  Evaluator::Evaluator(const Evaluator& eval)
  : engine_ptr_(eval.engine_ptr_) {
    COPY_ARRAY(value_table_, eval.value_table_);
  }

  // ムーブコンストラクタ。
  Evaluator::Evaluator(Evaluator&& eval)
  : engine_ptr_(eval.engine_ptr_) {
    COPY_ARRAY(value_table_, eval.value_table_);
  }

  // コピー代入演算子。
  Evaluator& Evaluator::operator=(const Evaluator& eval) {
    engine_ptr_ = eval.engine_ptr_;
    COPY_ARRAY(value_table_, eval.value_table_);
    return *this;
  }

  // ムーブ代入演算子。
  Evaluator& Evaluator::operator=(Evaluator&& eval) {
    engine_ptr_ = eval.engine_ptr_;
    COPY_ARRAY(value_table_, eval.value_table_);
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
    INIT_ARRAY(value_table_);

    const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES] =
    engine_ptr_->position_;

    // 全体計算。
    // ビショップペア。
    if (Util::CountBits(position[WHITE][BISHOP]) >= 2) {
      value_table_[BISHOP_PAIR][0] += 1.0;
    }
    if (Util::CountBits(position[BLACK][BISHOP]) >= 2) {
      value_table_[BISHOP_PAIR][0] -= 1.0;
    }

    // ルークペア。
    if (Util::CountBits(position[WHITE][ROOK]) >= 2) {
      value_table_[ROOK_PAIR][0] += 1.0;
    }
    if (Util::CountBits(position[BLACK][ROOK]) >= 2) {
      value_table_[ROOK_PAIR][0] -= 1.0;
    }

    // 各駒毎に価値を計算する。
    // 白のポーン。
    for (Bitboard pieces = position[WHITE][PAWN]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, PAWN>(piece_square);
    }

    // 白のナイト。
    for (Bitboard pieces = position[WHITE][KNIGHT]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, KNIGHT>(piece_square);
    }

    // 白のビショップ。
    for (Bitboard pieces = position[WHITE][BISHOP]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, BISHOP>(piece_square);
    }

    // 白のルーク。
    for (Bitboard pieces = position[WHITE][ROOK]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, ROOK>(piece_square);
    }

    // 白のクイーン。
    for (Bitboard pieces = position[WHITE][QUEEN]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<WHITE, QUEEN>(piece_square);
    }

    // 白のキング。
    CalValue<WHITE, KING>(engine_ptr_->king_[WHITE]);

    // 黒のポーン。
    for (Bitboard pieces = position[BLACK][PAWN]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, PAWN>(piece_square);
    }

    // 黒のナイト。
    for (Bitboard pieces = position[BLACK][KNIGHT]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, KNIGHT>(piece_square);
    }

    // 黒のビショップ。
    for (Bitboard pieces = position[BLACK][BISHOP]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, BISHOP>(piece_square);
    }

    // 黒のルーク。
    for (Bitboard pieces = position[BLACK][ROOK]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, ROOK>(piece_square);
    }

    // 黒のクイーン。
    for (Bitboard pieces = position[BLACK][QUEEN]; pieces;
    NEXT_BITBOARD(pieces)) {
      Square piece_square = Util::GetSquare(pieces);
      CalValue<BLACK, QUEEN>(piece_square);
    }

    // 黒のキング。
    CalValue<BLACK, KING>(engine_ptr_->king_[BLACK]);

    // ウェイトを付けて評価値を得る。
    double score = 0.0;
    unsigned int num_pieces = Util::CountBits(engine_ptr_->blocker_0_);
    const EvalParams& params = engine_ptr_->eval_params();

    // 配列型のウェイトの評価。
    for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
      score +=

      // オープニング時の駒の配置。
      (params.weight_opening_position_[piece_type][num_pieces]
      * value_table_[OPENING_POSITION][piece_type])

      // エンディング時の駒の配置。
      + (params.weight_ending_position_[piece_type][num_pieces]
      * value_table_[ENDING_POSITION][piece_type])

      // 機動力。
      + (params.weight_mobility_[piece_type][num_pieces]
      * value_table_[MOBILITY][piece_type])

      // センターコントロール。
      + (params.weight_center_control_[piece_type][num_pieces]
      * value_table_[CENTER_CONTROL][piece_type])

      // スウィートセンターのコントロール。
      + (params.weight_sweet_center_control_[piece_type][num_pieces]
      * value_table_[SWEET_CENTER_CONTROL][piece_type])

      // 駒の展開。
      + (params.weight_development_[piece_type][num_pieces]
      * value_table_[DEVELOPMENT][piece_type])

      // 攻撃。
      + (params.weight_attack_[piece_type][num_pieces]
      * value_table_[ATTACK][piece_type])

      // 防御。
      + (params.weight_defense_[piece_type][num_pieces]
      * value_table_[DEFENSE][piece_type])

      // ピン。
      + (params.weight_pin_[piece_type][num_pieces]
      * value_table_[PIN][piece_type])

      // 相手キング周辺への攻撃。
      + (params.weight_attack_around_king_[piece_type][num_pieces]
      * value_table_[ATTACK_AROUND_KING][piece_type]);
    }

    // その他のウェイトの評価。
    score +=

    // パスポーン。
    + (params.weight_pass_pawn_[num_pieces] * value_table_[PASS_PAWN][0])

    // 守られたパスポーン。
    + (params.weight_protected_pass_pawn_[num_pieces]
    * value_table_[PROTECTED_PASS_PAWN][0])

    // ダブルポーン。
    + (params.weight_double_pawn_[num_pieces] * value_table_[DOUBLE_PAWN][0])

    // 孤立ポーン。
    + (params.weight_iso_pawn_[num_pieces] * value_table_[ISO_PAWN][0])

    // ポーンの盾。
    + (params.weight_pawn_shield_[num_pieces] * value_table_[PAWN_SHIELD][0])

    // ビショップペア。
    + (params.weight_bishop_pair_[num_pieces] * value_table_[BISHOP_PAIR][0])

    // バッドビショップ。
    + (params.weight_bad_bishop_[num_pieces] * value_table_[BAD_BISHOP][0])

    // ルークペア。
    + (params.weight_rook_pair_[num_pieces] * value_table_[ROOK_PAIR][0])

    // セミオープンファイルのルーク。
    + (params.weight_rook_semiopen_fyle_[num_pieces]
    * value_table_[ROOK_SEMIOPEN_FYLE][0])

    // オープンファイルのルーク。
    + (params.weight_rook_open_fyle_[num_pieces]
    * value_table_[ROOK_OPEN_FYLE][0])

    // 早すぎるクイーンの始動。
    + (params.weight_early_queen_launched_[num_pieces]
    * value_table_[EARLY_QUEEN_LAUNCHED][0])

    // キング周りの弱いマス。
    + (params.weight_weak_square_[num_pieces] * value_table_[WEAK_SQUARE][0])

    // キャスリング。
    + (params.weight_castling_[num_pieces] * value_table_[CASTLING][0])

    // キャスリングの放棄。
    + (params.weight_abandoned_castling_[num_pieces]
    * value_table_[ABANDONED_CASTLING][0]);

    return engine_ptr_->to_move_ == WHITE ? static_cast<int>(material + score)
    : static_cast<int>(material - score);
  }

  // ================== //
  // 価値を計算する関数 //
  // ================== //
  // 各駒の価値を計算する。
  template<Side PSide, PieceType PType>
  void Evaluator::CalValue(Square piece_square) {
    constexpr Side EnemySide = Util::GetOppositeSide(PSide);

    // 評価関数用パラメータを得る。
    const EvalParams& params = engine_ptr_->eval_params();

    // 利き筋を作る。
    Bitboard attacks = 0;
    Bitboard pawn_moves = 0;
    Bitboard en_passant = 0;
    GenBitboards<PSide, PType>::F(*this, *engine_ptr_, piece_square, attacks,
    pawn_moves, en_passant);

    // --- 全駒共通 --- //
    // オープニング、エンディング時の駒の配置を計算。
    CalPosition<PSide, PType>::F(*this, params, piece_square);

    // 機動力を計算。
    CalMobility<PSide, PType>::F(*this, attacks, pawn_moves, en_passant);

    // センターコントロールを計算。
    AddOrSub<PSide>(value_table_[CENTER_CONTROL][PType],
    Util::CountBits(attacks & center_mask_));

    // スウィートセンターのコントロールを計算。
    AddOrSub<PSide>(value_table_[SWEET_CENTER_CONTROL][PType],
    Util::CountBits(attacks & sweet_center_mask_));

    // 駒の展開を計算。
    if (!(Util::SQUARE[piece_square] & start_position_[PSide][PType])) {
      AddOrSub<PSide>(value_table_[DEVELOPMENT][PType], 1.0);
    }

    // 敵への攻撃を計算。
    {
      Bitboard attacked = attacks & (engine_ptr_->side_pieces_[EnemySide]);
      double value = 0.0;
      const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
      params.attack_value_table_;

      for (; attacked; NEXT_BITBOARD(attacked)) {
        value +=
        table[PType][engine_ptr_->piece_board_[Util::GetSquare(attacked)]];
      }

      if (en_passant) {
        value += table[PAWN][PAWN];
      }

      AddOrSub<PSide>(value_table_[ATTACK][PType], value);
    }

    // 味方への防御を計算。
    {
      Bitboard defensed = attacks & (engine_ptr_->side_pieces_[PSide]);
      double value = 0.0;
      const double (& table)[NUM_PIECE_TYPES][NUM_PIECE_TYPES] =
      params.defense_value_table_;

      for (; defensed; NEXT_BITBOARD(defensed)) {
        value +=
        table[PType][engine_ptr_->piece_board_[Util::GetSquare(defensed)]];
      }

      AddOrSub<PSide>(value_table_[DEFENSE][PType], value);
    }

    // ピンを計算。
    {
      double value = 0.0;

      // ピンのターゲットと裏駒を作成。
      Bitboard pin_target = 0;
      Bitboard pin_back = 0;
      GenPinTargets<PSide, PType>::F(*this, *engine_ptr_, piece_square,
      attacks, pin_target, pin_back);

      // ピンを判定。
      for (Bitboard bb = pin_back; bb; NEXT_BITBOARD(bb)) {
        // 裏駒のマス。
        Square pin_back_sq = Util::GetSquare(bb);

        // 裏駒と自分の間のビットボード。
        Bitboard between = Util::GetBetween(piece_square, pin_back_sq);

        // 下のif文によるピンの判定条件は、
        // 「裏駒と自分との間」に「ピンの対象」が一つのみだった場合。
        if ((between & pin_target) && !(between & pin_back)) {
          value += params.pin_value_table_[PType]
          [engine_ptr_->piece_board_[Util::GetSquare(between & pin_target)]]
          [engine_ptr_->piece_board_[pin_back_sq]];
        }
      }

      AddOrSub<PSide>(value_table_[PIN][PType], value);
    }

    // 相手キング周辺への攻撃を計算。
    AddOrSub<PSide>(value_table_[ATTACK_AROUND_KING][PType],
    Util::CountBits(attacks & Util::GetKingMove
    (engine_ptr_->king_[EnemySide])));

    // 各駒専用の価値を計算。
    CalSpecial<PSide, PType>::F(*this, *engine_ptr_, piece_square);
  }

  // 実体化。
  template void Evaluator::CalValue<WHITE, PAWN>(Square piece_type);
  template void Evaluator::CalValue<WHITE, KNIGHT>(Square piece_type);
  template void Evaluator::CalValue<WHITE, BISHOP>(Square piece_type);
  template void Evaluator::CalValue<WHITE, ROOK>(Square piece_type);
  template void Evaluator::CalValue<WHITE, QUEEN>(Square piece_type);
  template void Evaluator::CalValue<WHITE, KING>(Square piece_type);
  template void Evaluator::CalValue<BLACK, PAWN>(Square piece_type);
  template void Evaluator::CalValue<BLACK, KNIGHT>(Square piece_type);
  template void Evaluator::CalValue<BLACK, BISHOP>(Square piece_type);
  template void Evaluator::CalValue<BLACK, ROOK>(Square piece_type);
  template void Evaluator::CalValue<BLACK, QUEEN>(Square piece_type);
  template void Evaluator::CalValue<BLACK, KING>(Square piece_type);

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
    FOR_SIDES(side) {
      FOR_SQUARES(square) {
        Bitboard mask = 0;
        if (side == NO_SIDE) {  // どちらのサイドでもなければ0。
          pass_pawn_mask_[side][square] = 0;
        } else {
          // 自分のファイルと隣のファイルのマスクを作る。
          Fyle fyle = Util::SQUARE_TO_FYLE[square];
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
            | Util::RANK[Util::SQUARE_TO_RANK[square]];
            mask &= ~temp;
          } else {
            Bitboard temp = ~(Util::SQUARE[square] - 1)
            | Util::RANK[Util::SQUARE_TO_RANK[square]];
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
    FOR_SQUARES(square) {
      Fyle fyle = Util::SQUARE_TO_FYLE[square];
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
    FOR_SIDES(side) {
      FOR_SQUARES(square) {
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
    FOR_SIDES(side) {
      FOR_SQUARES(square) {
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
