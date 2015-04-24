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
  // ================ //
  // テンプレート部品 //
  // ================ //

  // 評価用ビットボードを作成するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct GenBitboards {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {}
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, PAWN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      // 通常の動き。
      pawn_moves = engine.GetPawnStep(SIDE, square);

      // 攻撃。
      attacks = Util::GetPawnAttack(SIDE, square);

      // アンパッサン。
      if (engine.en_passant_square_ && (SIDE == engine.to_move_)) {
        en_passant = Util::SQUARE[engine.en_passant_square_] & attacks;
      }
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, KNIGHT> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = Util::GetKnightMove(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, BISHOP> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = evaluator.engine_ptr_->GetBishopAttack(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, ROOK> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = evaluator.engine_ptr_->GetRookAttack(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, QUEEN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = evaluator.engine_ptr_->GetQueenAttack(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, KING> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = Util::GetKingMove(square);
    }
  };

  // 駒の配置価値を計算するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct CalPosition {
    static void F(Evaluator& evaluator, Square square) {}
  };
  template<PieceType TYPE>
  struct CalPosition<WHITE, TYPE> {
    static void F(Evaluator& evaluator, Square square) {
      // オープニング。
      evaluator.score_ +=
      evaluator.cache_ptr_->opening_position_cache_[TYPE][square];

      // エンディング。
      evaluator.score_ +=
      evaluator.cache_ptr_->ending_position_cache_[TYPE][square];
    }
  };
  template<PieceType TYPE>
  struct CalPosition<BLACK, TYPE> {
    static void F(Evaluator& evaluator, Square square) {
      // オープニング。
      evaluator.score_ -=
      evaluator.cache_ptr_->opening_position_cache_[TYPE][Util::FLIP[square]];

      // エンディング。
      evaluator.score_ -=
      evaluator.cache_ptr_->ending_position_cache_[TYPE][Util::FLIP[square]];
    }
  };

  // 駒の機動力を計算するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct CalMobility {
    static void F(Evaluator& evaluator, Bitboard attacks, Bitboard pawn_moves,
    Bitboard en_passant) {
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      evaluator.score_ += SIGN * evaluator.cache_ptr_->mobility_cache_[TYPE]
      [Util::CountBits
      (attacks & ~(evaluator.engine_ptr_->side_pieces_[SIDE]))];
    }
  };
  template<Side SIDE>
  struct CalMobility<SIDE, PAWN> {
    static void F(Evaluator& evaluator, Bitboard attacks, Bitboard pawn_moves,
    Bitboard en_passant) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      evaluator.score_ += SIGN * evaluator.cache_ptr_->mobility_cache_[PAWN]
      [Util::CountBits
      ((attacks & evaluator.engine_ptr_->side_pieces_[ENEMY_SIDE]) | pawn_moves
      | en_passant)];
    }
  };

  // ピンのターゲットや裏駒を抽出するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct GenPinTargets {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard attacks, Bitboard& target, Bitboard& back) {}
  };
  template<Side SIDE>
  struct GenPinTargets<SIDE, BISHOP> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard attacks, Bitboard& target, Bitboard& back) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);

      target = attacks & engine.side_board_[ENEMY_SIDE];

      back = (engine.side_board_[ENEMY_SIDE]
      & Util::GetBishopMove(square)) & ~target;
    }
  };
  template<Side SIDE>
  struct GenPinTargets<SIDE, ROOK> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard attacks, Bitboard& target, Bitboard& back) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);

      target = attacks & engine.side_board_[ENEMY_SIDE];

      back = (engine.side_board_[ENEMY_SIDE]
      & Util::GetRookMove(square)) & ~target;
    }
  };
  template<Side SIDE>
  struct GenPinTargets<SIDE, QUEEN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard attacks, Bitboard& target, Bitboard& back) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);

      target = attacks & engine.side_board_[ENEMY_SIDE];

      back = (engine.side_board_[ENEMY_SIDE]
      & Util::GetQueenMove(square)) & ~target;
    }
  };

  // 各駒専用の価値を計算するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct CalSpecial {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {}
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, PAWN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // パスポーンを計算。
      if (!(engine.position_[ENEMY_SIDE][PAWN]
      & evaluator.pass_pawn_mask_[SIDE][square])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->pass_pawn_cache_;

        // 守られたパスポーン。
        if (engine.position_[SIDE][PAWN]
        & Util::GetPawnAttack(ENEMY_SIDE, square)) {
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->protected_pass_pawn_cache_;
        }
      }

      // ダブルポーンを計算。
      if (Util::CountBits(engine.position_[SIDE][PAWN]
      & Util::FYLE[Util::SQUARE_TO_FYLE[square]]) >= 2) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->double_pawn_cache_;
      }

      // 孤立ポーンを計算。
      if (!(engine.position_[SIDE][PAWN]
      & evaluator.iso_pawn_mask_[square])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->iso_pawn_cache_;
      }

      // ポーンの盾を計算。
      if ((Util::SQUARE[square]
      & evaluator.pawn_shield_mask_[SIDE][engine.king_[SIDE]])) {
        if (SIDE == WHITE) {
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->pawn_shield_cache_[square];
        } else {
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->pawn_shield_cache_
          [Util::FLIP[square]];
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, BISHOP> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // バッドビショップを計算。
      if ((Util::SQUARE[square] & Util::SQCOLOR[WHITE])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->bad_bishop_cache_
        [Util::CountBits
        (engine.position_[SIDE][PAWN] & Util::SQCOLOR[WHITE])];
      } else {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->bad_bishop_cache_
        [Util::CountBits
        (engine.position_[SIDE][PAWN] & Util::SQCOLOR[BLACK])];
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, ROOK> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // オープンファイルとセミオープンファイルを計算。
      Bitboard rook_fyle = Util::FYLE[Util::SQUARE_TO_FYLE[square]];
      if (!(engine.position_[SIDE][PAWN] & rook_fyle)) {
        // セミオープン。
        evaluator.score_ +=
        SIGN * evaluator.cache_ptr_->rook_semiopen_fyle_cache_;
        if (!(engine.position_[ENEMY_SIDE][PAWN] & rook_fyle)) {
          // オープン。
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->rook_open_fyle_cache_;
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, QUEEN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // クイーンの早過ぎる始動を計算。
      int value = 0;
      if (!(Util::SQUARE[square] & evaluator.start_position_[SIDE][QUEEN])) {
        value += Util::CountBits(engine.position_[SIDE][KNIGHT]
        & evaluator.start_position_[SIDE][KNIGHT]);

        value += Util::CountBits(engine.position_[SIDE][BISHOP]
        & evaluator.start_position_[SIDE][BISHOP]);
      }

      evaluator.score_ +=
      SIGN * evaluator.cache_ptr_->early_queen_launched_cache_[value];
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, KING> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // --- キング周りの弱いマスを計算 --- //
      int value = 0;

      // 弱いマス。
      Bitboard weak = (~(engine.position_[SIDE][PAWN]))
      & evaluator.weak_square_mask_[SIDE][square];

      // 相手の白マスビショップに対する弱いマス。
      if ((engine.position_[ENEMY_SIDE][BISHOP] & Util::SQCOLOR[WHITE])) {
        value += Util::CountBits(weak & Util::SQCOLOR[WHITE]);
      }

      // 相手の黒マスビショップに対する弱いマス。
      if ((engine.position_[ENEMY_SIDE][BISHOP] & Util::SQCOLOR[BLACK])) {
        value += Util::CountBits(weak & Util::SQCOLOR[BLACK]);
      }

      // 評価値にする。
      evaluator.score_ +=
      SIGN * evaluator.cache_ptr_->weak_square_cache_[value];

      // --- キャスリングを計算する --- //
      Castling rights_mask = SIDE == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
      if (engine.has_castled_[SIDE]) {
        // キャスリングした。
        evaluator.score_ += SIGN * evaluator.cache_ptr_->castling_cache_;
      } else {
        if (!(engine.castling_rights_ & rights_mask)) {
          // キャスリングの権利を放棄した。
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->abandoned_castling_cache_;
        }
      }
    }
  };

  // ========== //
  // static変数 //
  // ========== //
  Bitboard Evaluator::start_position_[NUM_SIDES][NUM_PIECE_TYPES];
  Bitboard Evaluator::not_start_position_[NUM_SIDES][NUM_PIECE_TYPES];
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
  : engine_ptr_(&engine), score_(0), cache_ptr_(nullptr) {
    InitCache();
  }

  // コピーコンストラクタ。
  Evaluator::Evaluator(const Evaluator& eval)
  : engine_ptr_(eval.engine_ptr_), score_(0),
  cache_ptr_(nullptr) {
    InitCache();
  }

  // ムーブコンストラクタ。
  Evaluator::Evaluator(Evaluator&& eval)
  : engine_ptr_(eval.engine_ptr_), score_(0),
  cache_ptr_(nullptr) {
    InitCache();
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
    // start_position_[][]とnot_start_position_[][]を初期化。
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
    // 初期化。
    cache_ptr_ = &(cache_[Util::CountBits(engine_ptr_->blocker_0_)]);
    score_ = 0;

    const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES] =
    engine_ptr_->position_;

    // --- 全体計算 --- //
    // 駒の展開。
    for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
      score_ += cache_ptr_->development_cache_[piece_type]
      [Util::CountBits(engine_ptr_->position_[WHITE][piece_type]
      & not_start_position_[WHITE][piece_type])]

      - cache_ptr_->development_cache_[piece_type]
      [Util::CountBits(engine_ptr_->position_[BLACK][piece_type]
      & not_start_position_[BLACK][piece_type])];
    }

    // ビショップペア。
    if ((position[WHITE][BISHOP] & Util::SQCOLOR[WHITE])
    && (position[WHITE][BISHOP] & Util::SQCOLOR[BLACK])) {
      score_ += cache_ptr_->bishop_pair_cache_;
    }
    if ((position[BLACK][BISHOP] & Util::SQCOLOR[WHITE])
    && (position[BLACK][BISHOP] & Util::SQCOLOR[BLACK])) {
      score_ -= cache_ptr_->bishop_pair_cache_;
    }

    // ルークペア。
    if ((position[WHITE][ROOK] & (position[WHITE][ROOK] - 1))) {
      score_ += cache_ptr_->rook_pair_cache_;
    }
    if ((position[BLACK][ROOK] & (position[BLACK][ROOK] - 1))) {
      score_ -= cache_ptr_->rook_pair_cache_;
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

    // 256で割る。
    score_ >>= 8;

    // 手番に合わせて符号を変えて返す。
    return engine_ptr_->to_move_ == WHITE ? (material + score_)
    : (material - score_);
  }

  // ================== //
  // 価値を計算する関数 //
  // ================== //
  // 各駒の価値を計算する。
  template<Side SIDE, PieceType TYPE>
  void Evaluator::CalValue(Square piece_square) {
    constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
    constexpr int SIGN = SIDE == WHITE ? 1 : -1;

    // 利き筋を作る。
    Bitboard attacks = 0;
    Bitboard pawn_moves = 0;
    Bitboard en_passant = 0;
    GenBitboards<SIDE, TYPE>::F(*this, *engine_ptr_, piece_square, attacks,
    pawn_moves, en_passant);

    // --- 全駒共通 --- //
    // オープニング、エンディング時の駒の配置を計算。
    CalPosition<SIDE, TYPE>::F(*this, piece_square);

    // 機動力を計算。
    CalMobility<SIDE, TYPE>::F(*this, attacks, pawn_moves, en_passant);

    // センターコントロールを計算。
    score_ += SIGN * cache_ptr_->center_control_cache_[TYPE]
    [Util::CountBits(attacks & center_mask_)];

    // スウィートセンターのコントロールを計算。
    score_ += SIGN * cache_ptr_->sweet_center_control_cache_[TYPE]
    [Util::CountBits(attacks & sweet_center_mask_)];

    // 敵への攻撃を計算。
    {
      Bitboard attacked = attacks & (engine_ptr_->side_pieces_[ENEMY_SIDE]);

      for (; attacked; NEXT_BITBOARD(attacked)) {
        score_ += SIGN * cache_ptr_->attack_cache_[TYPE]
        [engine_ptr_->piece_board_[Util::GetSquare(attacked)]];
      }

      if (en_passant) {
        score_ += SIGN * cache_ptr_->attack_cache_[PAWN][PAWN];
      }
    }

    // 味方への防御を計算。
    {
      Bitboard defensed = attacks & (engine_ptr_->side_pieces_[SIDE]);

      for (; defensed; NEXT_BITBOARD(defensed)) {
        score_ += SIGN * cache_ptr_->defense_cache_[TYPE]
        [engine_ptr_->piece_board_[Util::GetSquare(defensed)]];
      }
    }

    // ピンを計算。
    {
      // ピンのターゲットと裏駒を作成。
      Bitboard pin_target = 0;
      Bitboard pin_back = 0;
      GenPinTargets<SIDE, TYPE>::F(*this, *engine_ptr_, piece_square,
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
          score_ += SIGN * cache_ptr_->pin_cache_[TYPE]
          [engine_ptr_->piece_board_[Util::GetSquare(between & pin_target)]]
          [engine_ptr_->piece_board_[pin_back_sq]];
        }
      }
    }

    // 相手キング周辺への攻撃を計算。
    score_ += SIGN * cache_ptr_->attack_around_king_cache_[TYPE]
    [Util::CountBits(attacks & Util::GetKingMove
    (engine_ptr_->king_[ENEMY_SIDE]))];

    // 各駒専用の価値を計算。
    CalSpecial<SIDE, TYPE>::F(*this, *engine_ptr_, piece_square);
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
    // 先ずはゼロで初期化。
    INIT_ARRAY(start_position_);
    INIT_ARRAY(not_start_position_);

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

    // 論理否定を初期化。
    for (Side side = WHITE; side <= BLACK; ++side) {
      for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
        not_start_position_[side][piece_type] =
        ~(start_position_[side][piece_type]);
      }
    }
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

  // キャッシュを初期化する。
  void Evaluator::InitCache() {
    for (unsigned int num_pieces = 0; num_pieces < (NUM_SQUARES + 1);
    ++num_pieces) {
      INIT_ARRAY(cache_[num_pieces].opening_position_cache_);
      INIT_ARRAY(cache_[num_pieces].ending_position_cache_);
      INIT_ARRAY(cache_[num_pieces].mobility_cache_);
      INIT_ARRAY(cache_[num_pieces].center_control_cache_);
      INIT_ARRAY(cache_[num_pieces].sweet_center_control_cache_);
      INIT_ARRAY(cache_[num_pieces].development_cache_);
      INIT_ARRAY(cache_[num_pieces].attack_cache_);
      INIT_ARRAY(cache_[num_pieces].defense_cache_);
      INIT_ARRAY(cache_[num_pieces].pin_cache_);
      INIT_ARRAY(cache_[num_pieces].attack_around_king_cache_);
      cache_[num_pieces].pass_pawn_cache_ = 0;
      cache_[num_pieces].protected_pass_pawn_cache_ = 0;
      cache_[num_pieces].double_pawn_cache_ = 0;
      cache_[num_pieces].iso_pawn_cache_ = 0;
      INIT_ARRAY(cache_[num_pieces].pawn_shield_cache_);
      cache_[num_pieces].bishop_pair_cache_ = 0;
      INIT_ARRAY(cache_[num_pieces].bad_bishop_cache_);
      cache_[num_pieces].rook_pair_cache_ = 0;
      cache_[num_pieces].rook_semiopen_fyle_cache_ = 0;
      cache_[num_pieces].rook_open_fyle_cache_ = 0;
      INIT_ARRAY(cache_[num_pieces].early_queen_launched_cache_);
      INIT_ARRAY(cache_[num_pieces].weak_square_cache_);
      cache_[num_pieces].castling_cache_ = 0;
      cache_[num_pieces].abandoned_castling_cache_ = 0;
    }
  }

  // EvalParamsをキャッシュする。
  void Evaluator::CacheEvalParams() {
    const EvalParams& params = engine_ptr_->eval_params();
    for (unsigned int num_pieces = 0; num_pieces < (NUM_SQUARES + 1);
    ++num_pieces) {
      Cache* ptr = &(cache_[num_pieces]);

      FOR_PIECE_TYPES(piece_type) {
        FOR_SQUARES(square) {
          ptr->opening_position_cache_[piece_type][square] =
          256.0 * params.opening_position_value_table()[piece_type][square]
          * params.weight_opening_position()[piece_type](num_pieces);

          ptr->ending_position_cache_[piece_type][square] =
          256.0 * params.ending_position_value_table()[piece_type][square]
          * params.weight_ending_position()[piece_type](num_pieces);
        }

        for (unsigned int num_attacks = 0; num_attacks < (MAX_ATTACKS + 1);
        ++num_attacks) {
          ptr->mobility_cache_[piece_type][num_attacks] =
          256.0 * params.weight_mobility()[piece_type](num_pieces);
        }

        for (unsigned int num_center = 0; num_center < (NUM_CENTER + 1);
        ++num_center) {
          ptr->center_control_cache_[piece_type][num_center] =
          256.0 * num_center
          * params.weight_center_control()[piece_type](num_pieces);
        }

        for (unsigned int num_sweet_center = 0;
        num_sweet_center < (NUM_SWEET_CENTER + 1); ++num_sweet_center) {
          ptr->sweet_center_control_cache_[piece_type]
          [num_sweet_center] =
          256.0 * num_sweet_center
          * params.weight_sweet_center_control()[piece_type](num_pieces);
        }

        for (unsigned int num_pieces_2 = 0; num_pieces_2 < (NUM_SQUARES + 1);
        ++num_pieces_2) {
          ptr->development_cache_[piece_type][num_pieces_2] =
          256.0 * num_pieces_2
          * params.weight_development()[piece_type](num_pieces);
        }

        FOR_PIECE_TYPES(piece_type_2) {
          ptr->attack_cache_[piece_type][piece_type_2] =
          256.0 * params.attack_value_table()[piece_type][piece_type_2]
          * params.weight_attack()[piece_type](num_pieces);

          ptr->defense_cache_[piece_type][piece_type_2] =
          256.0 * params.defense_value_table()[piece_type][piece_type_2]
          * params.weight_defense()[piece_type](num_pieces);

          FOR_PIECE_TYPES(piece_type_3) {
            ptr->pin_cache_[piece_type][piece_type_2]
            [piece_type_3] =
            256.0 * params.pin_value_table()[piece_type][piece_type_2]
            [piece_type_3]
            * params.weight_pin()[piece_type](num_pieces);
          }
        }

        for (unsigned int num_around_king = 0;
        num_around_king < (NUM_AROUND_KING + 1); ++num_around_king) {
          ptr->attack_around_king_cache_[piece_type]
          [num_around_king] = 256.0 * num_around_king
          * params.weight_attack_around_king()[piece_type](num_pieces);
        }
      }

      ptr->pass_pawn_cache_ = 256.0 * params.weight_pass_pawn()(num_pieces);

      ptr->protected_pass_pawn_cache_ =
      256.0 * params.weight_protected_pass_pawn()(num_pieces);

      ptr->double_pawn_cache_ =
      256.0 * params.weight_double_pawn()(num_pieces);

      ptr->iso_pawn_cache_ =
      256.0 * params.weight_iso_pawn()(num_pieces);

      FOR_SQUARES(square) {
        ptr->pawn_shield_cache_[square] =
        256.0 * params.pawn_shield_value_table()[square]
        * params.weight_pawn_shield()(num_pieces);
      }

      ptr->bishop_pair_cache_ =
      256.0 * params.weight_bishop_pair()(num_pieces);

      for (unsigned int num_pawn = 0; num_pawn < (NUM_SQUARES + 1);
      ++num_pawn) {
        ptr->bad_bishop_cache_[num_pawn] =
        256.0 * num_pawn * params.weight_bad_bishop()(num_pieces);
      }

      ptr->rook_pair_cache_ =
      256.0 * params.weight_rook_pair()(num_pieces);

      ptr->rook_open_fyle_cache_ =
      256.0 * params.weight_rook_open_fyle()(num_pieces);

      ptr->rook_semiopen_fyle_cache_ =
      256.0 * params.weight_rook_semiopen_fyle()(num_pieces);

      for (unsigned int num_minor = 0; num_minor < (NUM_SQUARES + 1);
      ++num_minor) {
        ptr->early_queen_launched_cache_[num_minor] =
        256.0 * num_minor * params.weight_early_queen_launched()(num_pieces);
      }

      for (unsigned int num_square = 0; num_square < (NUM_SQUARES + 1);
      ++num_square) {
        ptr->weak_square_cache_[num_square] =
        256.0 * num_square * params.weight_weak_square()(num_pieces);
      }

      ptr->castling_cache_ =
      256.0 * params.weight_castling()(num_pieces);

      ptr->abandoned_castling_cache_ =
      256.0 * params.weight_abandoned_castling()(num_pieces);
    }
  }
}  // namespace Sayuri
