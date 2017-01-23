/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Hironori Ishibashi
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
#include "cache.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ========== //
  // static変数 //
  // ========== //
  constexpr Bitboard Evaluator::START_POSITION[NUM_SIDES][NUM_PIECE_TYPES];
  constexpr Bitboard Evaluator::NOT_START_POSITION[NUM_SIDES][NUM_PIECE_TYPES];
  constexpr Bitboard Evaluator::PASS_PAWN_MASK[NUM_SIDES][NUM_SQUARES];
  constexpr Bitboard Evaluator::ISO_PAWN_MASK[NUM_SQUARES];
  constexpr Bitboard Evaluator::PAWN_SHIELD_MASK[NUM_SIDES][NUM_SQUARES];
  constexpr Bitboard Evaluator::WEAK_SQUARE_MASK[NUM_SIDES][NUM_SQUARES];

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
      attacks = Util::PAWN_ATTACK[SIDE][square];

      // アンパッサン。
      if (engine.basic_st_.en_passant_square_
      && (SIDE == engine.basic_st_.to_move_)) {
        en_passant =
        Util::SQUARE[engine.basic_st_.en_passant_square_][R0] & attacks;
      }
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, KNIGHT> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = Util::KNIGHT_MOVE[square];
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, BISHOP> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = engine.GetBishopAttack(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, ROOK> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = engine.GetRookAttack(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, QUEEN> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = engine.GetQueenAttack(square);
    }
  };
  template<Side SIDE>
  struct GenBitboards<SIDE, KING> {
    static void F(Evaluator& evaluator, const ChessEngine& engine,
    Square square, Bitboard& attacks, Bitboard& pawn_moves,
    Bitboard& en_passant) {
      attacks = Util::KING_MOVE[square];
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
      evaluator.score_ +=
      evaluator.cache_ptr_->opening_position_cache_[TYPE][square]
      + evaluator.cache_ptr_->ending_position_cache_[TYPE][square];
    }
  };
  template<PieceType TYPE>
  struct CalPosition<BLACK, TYPE> {
    static void F(Evaluator& evaluator, Square square) {
      evaluator.score_ -=
      evaluator.cache_ptr_->opening_position_cache_[TYPE][Util::FLIP[square]]
      + evaluator.cache_ptr_->ending_position_cache_[TYPE][Util::FLIP[square]];
    }
  };

  // 駒の機動力を計算するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct CalMobility {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st, Bitboard attacks,
    Bitboard pawn_moves, Bitboard en_passant) {
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      evaluator.score_ += SIGN * evaluator.cache_ptr_->mobility_cache_[TYPE]
      [Util::CountBits(attacks & ~(basic_st.side_pieces_[SIDE]))];
    }
  };
  template<Side SIDE>
  struct CalMobility<SIDE, PAWN> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st, Bitboard attacks,
    Bitboard pawn_moves, Bitboard en_passant) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      evaluator.score_ += SIGN * evaluator.cache_ptr_->mobility_cache_[PAWN]
      [Util::CountBits((attacks & basic_st.side_pieces_[ENEMY_SIDE])
      | pawn_moves | en_passant)];
    }
  };

  // 各駒専用の価値を計算するテンプレート部品。
  template<Side SIDE, PieceType TYPE>
  struct CalSpecial {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {}
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, PAWN> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // パスポーンを計算。
      if (!(basic_st.position_[ENEMY_SIDE][PAWN]
      & Evaluator::PASS_PAWN_MASK[SIDE][square])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->pass_pawn_cache_;

        // 守られたパスポーン。
        if (basic_st.position_[SIDE][PAWN]
        & Util::PAWN_ATTACK[ENEMY_SIDE][square]) {
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->protected_pass_pawn_cache_;
        }
      }

      // ダブルポーンを計算。
      if ((basic_st.position_[SIDE][PAWN]
      & Util::FYLE[Util::SquareToFyle(square)] & ~Util::SQUARE[square][R0])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->double_pawn_cache_;
      }

      // 孤立ポーンを計算。
      if (!(basic_st.position_[SIDE][PAWN]
      & Evaluator::ISO_PAWN_MASK[square])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->iso_pawn_cache_;
      }

      // ポーンの盾を計算。
      if ((Util::SQUARE[square][R0]
      & Evaluator::PAWN_SHIELD_MASK[SIDE][basic_st.king_[SIDE]])) {
        if (SIDE == WHITE) {
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->pawn_shield_cache_[square];
        } else {
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->pawn_shield_cache_[Util::FLIP[square]];
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, BISHOP> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // バッドビショップを計算。
      if ((Util::SQUARE[square][R0] & Util::SQCOLOR[WHITE])) {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->bad_bishop_cache_
        [Util::CountBits
        (basic_st.position_[SIDE][PAWN] & Util::SQCOLOR[WHITE])];
      } else {
        evaluator.score_ += SIGN * evaluator.cache_ptr_->bad_bishop_cache_
        [Util::CountBits
        (basic_st.position_[SIDE][PAWN] & Util::SQCOLOR[BLACK])];
      }

      // ピンを計算。
      // 相手のビットボードと裏駒を作成。
      Bitboard enemy_pieces = basic_st.side_pieces_[ENEMY_SIDE];
      Bitboard pin_back = (MetaEvaluator::PIN_BACK_TABLE[square]
      [(basic_st.blocker_[R45] >> Util::MAGIC_SHIFT[square][R45])
      & Util::MAGIC_MASK[square][R45]][R45]
      | MetaEvaluator::PIN_BACK_TABLE[square]
      [(basic_st.blocker_[R135] >> Util::MAGIC_SHIFT[square][R135])
      & Util::MAGIC_MASK[square][R135]][R135])
      & enemy_pieces;

      // ピンを判定。
      for (; pin_back; NEXT_BITBOARD(pin_back)) {
        // 裏駒のマス。
        Square pin_back_sq = Util::GetSquare(pin_back);

        // 裏駒と自分の間のビットボード。
        Bitboard between = Util::GetBetween(square, pin_back_sq);

        // 下のif文によるピンの判定条件は、
        // 「裏駒と自分との間」にターゲット(相手の駒)があった場合。
        // pin_backに対応するターゲットが味方の駒の場合もあるので
        // 相手の駒に限定しなくてはならない。
        if ((between & enemy_pieces)) {
          evaluator.score_ += SIGN * evaluator.cache_ptr_->pin_cache_[BISHOP]
          [basic_st.piece_board_[Util::GetSquare(between & enemy_pieces)]]
          [basic_st.piece_board_[pin_back_sq]];
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, ROOK> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // オープンファイルとセミオープンファイルを計算。
      Bitboard rook_fyle = Util::FYLE[Util::SquareToFyle(square)];
      if (!(basic_st.position_[SIDE][PAWN] & rook_fyle)) {
        // セミオープン。
        evaluator.score_ +=
        SIGN * evaluator.cache_ptr_->rook_semiopen_fyle_cache_;
        if (!(basic_st.position_[ENEMY_SIDE][PAWN] & rook_fyle)) {
          // オープン。
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->rook_open_fyle_cache_;
        }
      }

      // ピンを計算。
      // 相手の駒と裏駒を作成。
      Bitboard enemy_pieces = basic_st.side_pieces_[ENEMY_SIDE];
      Bitboard pin_back = (MetaEvaluator::PIN_BACK_TABLE[square]
      [(basic_st.blocker_[R0] >> Util::MAGIC_SHIFT[square][R0])
      & Util::MAGIC_MASK[square][R0]][R0]
      | MetaEvaluator::PIN_BACK_TABLE[square]
      [(basic_st.blocker_[R90] >> Util::MAGIC_SHIFT[square][R90])
      & Util::MAGIC_MASK[square][R90]][R90])
      & enemy_pieces;

      // ピンを判定。
      for (; pin_back; NEXT_BITBOARD(pin_back)) {
        // 裏駒のマス。
        Square pin_back_sq = Util::GetSquare(pin_back);

        // 裏駒と自分の間のビットボード。
        Bitboard between = Util::GetBetween(square, pin_back_sq);

        // 下のif文によるピンの判定条件は、
        // 「裏駒と自分との間」にターゲット(相手の駒)があった場合。
        // pin_backに対応するターゲットが味方の駒の場合もあるので
        // 相手の駒に限定しなくてはならない。
        if ((between & enemy_pieces)) {
          evaluator.score_ += SIGN * evaluator.cache_ptr_->pin_cache_[ROOK]
          [basic_st.piece_board_[Util::GetSquare(between & enemy_pieces)]]
          [basic_st.piece_board_[pin_back_sq]];
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, QUEEN> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // クイーンの早過ぎる始動を計算。
      if (!(Util::SQUARE[square][R0]
      & Evaluator::START_POSITION[SIDE][QUEEN])) {
        evaluator.score_ +=
        SIGN * evaluator.cache_ptr_->early_queen_starting_cache_
        [Util::CountBits((basic_st.position_[SIDE][KNIGHT]
        & Evaluator::START_POSITION[SIDE][KNIGHT])
        | (basic_st.position_[SIDE][BISHOP]
        & Evaluator::START_POSITION[SIDE][BISHOP]))];
      }

      // ピンを計算。
      // 相手の駒と裏駒を作成。
      Bitboard enemy_pieces = basic_st.side_pieces_[ENEMY_SIDE];
      Bitboard pin_back = (MetaEvaluator::PIN_BACK_TABLE[square]
      [(basic_st.blocker_[R0] >> Util::MAGIC_SHIFT[square][R0])
      & Util::MAGIC_MASK[square][R0]][R0]
      | MetaEvaluator::PIN_BACK_TABLE[square]
      [(basic_st.blocker_[R45] >> Util::MAGIC_SHIFT[square][R45])
      & Util::MAGIC_MASK[square][R45]][R45]
      | MetaEvaluator::PIN_BACK_TABLE[square]
      [(basic_st.blocker_[R90] >> Util::MAGIC_SHIFT[square][R90])
      & Util::MAGIC_MASK[square][R90]][R90]
      | MetaEvaluator::PIN_BACK_TABLE[square]
      [(basic_st.blocker_[R135] >> Util::MAGIC_SHIFT[square][R135])
      & Util::MAGIC_MASK[square][R135]][R135])
      & enemy_pieces;

      // ピンを判定。
      for (; pin_back; NEXT_BITBOARD(pin_back)) {
        // 裏駒のマス。
        Square pin_back_sq = Util::GetSquare(pin_back);

        // 裏駒と自分の間のビットボード。
        Bitboard between = Util::GetBetween(square, pin_back_sq);

        // 下のif文によるピンの判定条件は、
        // 「裏駒と自分との間」にターゲット(相手の駒)があった場合。
        // pin_backに対応するターゲットが味方の駒の場合もあるので
        // 相手の駒に限定しなくてはならない。
        if ((between & enemy_pieces)) {
          evaluator.score_ += SIGN * evaluator.cache_ptr_->pin_cache_[QUEEN]
          [basic_st.piece_board_[Util::GetSquare(between & enemy_pieces)]]
          [basic_st.piece_board_[pin_back_sq]];
        }
      }
    }
  };
  template<Side SIDE>
  struct CalSpecial<SIDE, KING> {
    static void F(Evaluator& evaluator,
    const ChessEngine::BasicStruct& basic_st,
    Square square, Bitboard attacks) {
      constexpr Side ENEMY_SIDE = Util::GetOppositeSide(SIDE);
      constexpr int SIGN = SIDE == WHITE ? 1 : -1;

      // --- キング周りの弱いマスを計算 --- //
      int value = 0;

      // 弱いマス。
      Bitboard weak = (~(basic_st.position_[SIDE][PAWN]))
      & Evaluator::WEAK_SQUARE_MASK[SIDE][square];

      // 相手の白マスビショップに対する弱いマス。
      if ((basic_st.position_[ENEMY_SIDE][BISHOP] & Util::SQCOLOR[WHITE])) {
        value += Util::CountBits(weak & Util::SQCOLOR[WHITE]);
      }

      // 相手の黒マスビショップに対する弱いマス。
      if ((basic_st.position_[ENEMY_SIDE][BISHOP] & Util::SQCOLOR[BLACK])) {
        value += Util::CountBits(weak & Util::SQCOLOR[BLACK]);
      }

      // 評価値にする。
      evaluator.score_ +=
      SIGN * evaluator.cache_ptr_->weak_square_cache_[value];

      // --- キャスリングを計算する --- //
      constexpr Castling RIGHTS_MASK =
      SIDE == WHITE ? WHITE_CASTLING : BLACK_CASTLING;
      if (basic_st.has_castled_[SIDE]) {
        // キャスリングした。
        evaluator.score_ += SIGN * evaluator.cache_ptr_->castling_cache_;
      } else {
        if (!(basic_st.castling_rights_ & RIGHTS_MASK)) {
          // キャスリングの権利を放棄した。
          evaluator.score_ +=
          SIGN * evaluator.cache_ptr_->abandoned_castling_cache_;
        }
      }
    }
  };

  // ==================== //
  // コンストラクタと代入 //
  // ==================== //
  // コンストラクタ。
  Evaluator::Evaluator(const ChessEngine& engine)
  : engine_ptr_(&engine), score_(0), cache_ptr_(nullptr) {}

  // コピーコンストラクタ。
  Evaluator::Evaluator(const Evaluator& eval)
  : engine_ptr_(eval.engine_ptr_), score_(0),
  cache_ptr_(nullptr) {}

  // ムーブコンストラクタ。
  Evaluator::Evaluator(Evaluator&& eval)
  : engine_ptr_(eval.engine_ptr_), score_(0),
  cache_ptr_(nullptr) {}

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
  }

  // ============== //
  // パブリック関数 //
  // ============== //
  // 現在の局面の評価値を計算する。
  int Evaluator::Evaluate(int material) {
    // 準備。
    const ChessEngine::BasicStruct& basic_st = engine_ptr_->basic_st_;
    // 初期化。
    cache_ptr_ = &(engine_ptr_->shared_st_ptr_->cache_.eval_cache_
    [Util::CountBits(basic_st.blocker_[R0])]);
    score_ = 0;

    const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES] =
    basic_st.position_;

    // --- 全体計算 --- //
    // 駒の展開。
    for (PieceType piece_type = PAWN; piece_type <= KING; ++piece_type) {
      score_ += cache_ptr_->development_cache_[piece_type]
      [Util::CountBits(basic_st.position_[WHITE][piece_type]
      & Evaluator::NOT_START_POSITION[WHITE][piece_type])]

      - cache_ptr_->development_cache_[piece_type]
      [Util::CountBits(basic_st.position_[BLACK][piece_type]
      & Evaluator::NOT_START_POSITION[BLACK][piece_type])];
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
    CalValue<WHITE, KING>(basic_st.king_[WHITE]);

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
    CalValue<BLACK, KING>(basic_st.king_[BLACK]);

    // 256で割る。
    score_ >>= 8;

    // 手番に合わせて符号を変えて返す。
    return basic_st.to_move_ == WHITE ? (material + score_)
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

    // 準備。
    const ChessEngine::BasicStruct& basic_st = engine_ptr_->basic_st_;

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
    CalMobility<SIDE, TYPE>::F
    (*this, basic_st, attacks, pawn_moves, en_passant);

    // センターコントロールを計算。
    score_ += SIGN * cache_ptr_->center_control_cache_[TYPE]
    [Util::CountBits(attacks & CENTER_MASK)];

    // スウィートセンターのコントロールを計算。
    score_ += SIGN * cache_ptr_->sweet_center_control_cache_[TYPE]
    [Util::CountBits(attacks & SWEET_CENTER_MASK)];

    // 敵への攻撃を計算。
    {
      Bitboard attacked = attacks & (basic_st.side_pieces_[ENEMY_SIDE]);

      for (; attacked; NEXT_BITBOARD(attacked)) {
        score_ += SIGN * cache_ptr_->attack_cache_[TYPE]
        [basic_st.piece_board_[Util::GetSquare(attacked)]];
      }

      if (en_passant) {
        score_ += SIGN * cache_ptr_->attack_cache_[PAWN][PAWN];
      }
    }

    // 味方への防御を計算。
    {
      Bitboard defensed = attacks & (basic_st.side_pieces_[SIDE]);

      for (; defensed; NEXT_BITBOARD(defensed)) {
        score_ += SIGN * cache_ptr_->defense_cache_[TYPE]
        [basic_st.piece_board_[Util::GetSquare(defensed)]];
      }
    }

    // 相手キング周辺への攻撃を計算。
    score_ += SIGN * cache_ptr_->attack_around_king_cache_[TYPE]
    [Util::CountBits(attacks & Util::KING_MOVE[basic_st.king_[ENEMY_SIDE]])];

    // 各駒専用の価値を計算。
    CalSpecial<SIDE, TYPE>::F(*this, basic_st, piece_square, attacks);
  }

  // 実体化。
  template void Evaluator::CalValue<WHITE, PAWN>(Square);
  template void Evaluator::CalValue<WHITE, KNIGHT>(Square);
  template void Evaluator::CalValue<WHITE, BISHOP>(Square);
  template void Evaluator::CalValue<WHITE, ROOK>(Square);
  template void Evaluator::CalValue<WHITE, QUEEN>(Square);
  template void Evaluator::CalValue<WHITE, KING>(Square);
  template void Evaluator::CalValue<BLACK, PAWN>(Square);
  template void Evaluator::CalValue<BLACK, KNIGHT>(Square);
  template void Evaluator::CalValue<BLACK, BISHOP>(Square);
  template void Evaluator::CalValue<BLACK, ROOK>(Square);
  template void Evaluator::CalValue<BLACK, QUEEN>(Square);
  template void Evaluator::CalValue<BLACK, KING>(Square);
}  // namespace Sayuri
