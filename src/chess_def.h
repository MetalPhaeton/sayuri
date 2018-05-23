/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Hironori Ishibashi
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
 * @file chess_def.h
 * @author Hironori Ishibashi
 * @brief Sayuriで使う定義全般。
 */

#ifndef CHESS_DEF_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define CHESS_DEF_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <chrono>
#include <climits>

/** Sayuri 名前空間。 */
namespace Sayuri {

#if defined(_WIN32)  // Visual Studio用。

#pragma warning(disable: 4146)  // 符号なしへのマイナス符号の警告を無効。

#endif

  // ============ //
  // エンジン情報 //
  // ============ //
  /** Sayuriのバージョン番号。 */
  constexpr const char* ID_NAME = "Sayuri 2018.05.23";
  /** Sayuriの作者名。 */
  constexpr const char* ID_AUTHOR = "Hironori Ishibashi";
  /** ライセンス文。 */
  constexpr const char* LICENSE =
R"...(The MIT License (MIT)

Copyright (c) 2013-2018 Hironori Ishibashi

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
IN THE SOFTWARE.)...";

  // 基本整数型。
  using i8 = std::int8_t;
  using i16 = std::int16_t;
  using i32 = std::int32_t;
  using i64 = std::int64_t;
  using u8 = std::uint8_t;
  using u16 = std::uint16_t;
  using u32 = std::uint32_t;
  using u64 = std::uint64_t;

  // ============= //
  // UCIオプション //
  // ============= //
  /** トランスポジションテーブルのデフォルトサイズ。 */
  constexpr std::size_t UCI_DEFAULT_TABLE_SIZE = 1ULL * 1024ULL * 1024ULL;

  /** トランスポジションテーブルの最小サイズ。 */
  constexpr std::size_t UCI_MIN_TABLE_SIZE = 1ULL * 1024ULL * 1024ULL;

#if defined(__arm__)  // ARMのCPU用定数。
  /** トランスポジションテーブルの最大サイズ。 (ARM用) */
  constexpr std::size_t UCI_MAX_TABLE_SIZE =
  2ULL * 1024ULL * 1024ULL * 1024ULL;

#elif defined(__i386__) || defined(_M_IX86)  // 32ビットCPU用定数。
  /** トランスポジションテーブルの最大サイズ。 (32ビットCPU) */
  constexpr std::size_t UCI_MAX_TABLE_SIZE = 1024ULL * 1024ULL * 1024ULL;

#else  // 64ビットCPU用定数。
  /** トランスポジションテーブルの最大サイズ。 (64ビットCPU) */
  constexpr std::size_t UCI_MAX_TABLE_SIZE =
  8ULL * 1024ULL * 1024ULL * 1024ULL;

#endif

  /** Ponder機能のデフォルト設定。 */
  constexpr bool UCI_DEFAULT_PONDER = true;

  /** スレッドのデフォルトの数。 */
  constexpr int UCI_DEFAULT_THREADS = 1;

  /** スレッドの最大数。 */
  constexpr int UCI_MAX_THREADS = 64;

  /** アナライズモードのデフォルト設定。 */
  constexpr bool UCI_DEFAULT_ANALYSE_MODE = false;

  // ====== //
  // マクロ //
  // ====== //
  /**
   * std::memset()で配列を初期化。
   * @param array 初期化する配列。
   */
#define INIT_ARRAY(array) (std::memset((array), 0, sizeof(array)))

  /**
   * std::memcpy()で配列をコピーする。
   * @param dst コピー先。
   * @param src コピー元。
   */
#define COPY_ARRAY(dst, src) (std::memcpy((dst), (src), sizeof(dst)))

  /**
   * maskで指定のビットフィールドを更新する。
   * @param origin_data 更新したいデータ。
   * @param new_data 新しいデータ。
   * @param mask 更新したい場所。
   */
#define UPDATE_FIELD(origin_data, new_data, mask) \
((origin_data) = ((origin_data) & ~(mask)) | (new_data))

  /**
   * 次のマスのビットボードに更新する。
   * @param bitboard 更新したいビットボード。
   */
#define NEXT_BITBOARD(bitboard) ((bitboard) &= (bitboard) - 1)

  /**
   * 全マスをループ。
   * @param var_name 変数名。
   */
#define FOR_SQUARES(var_name) \
for (Square var_name = 0; var_name < NUM_SQUARES; ++var_name)

  /**
   * 全ファイルをループ。
   * @param var_name 変数名。
   */
#define FOR_FYLES(var_name) \
for (Fyle var_name = 0; var_name < NUM_FYLES; ++var_name)

  /**
   * 全ランクをループ。
   * @param var_name 変数名。
   */
#define FOR_RANKS(var_name) \
for (Rank var_name = 0; var_name < NUM_RANKS; ++var_name)

  /**
   * 全サイドの種類をループ。
   * @param var_name 変数名。
   */
#define FOR_SIDES(var_name) \
for (Side var_name = 0; var_name < NUM_SIDES; ++var_name)

  /**
   * 全駒の種類をループ。
   * @param var_name 変数名。
   */
#define FOR_PIECE_TYPES(var_name) \
for (PieceType var_name = 0; var_name < NUM_PIECE_TYPES; ++var_name)

  // ========== //
  // 基本の定数 //
  // ========== //
  /** ビットボードの型。 */
  using Bitboard = u64;

  // --- マスの定義 --- //
  /** マスの型。 */
  using Square = u32;
  /** マスの定数。 */
  enum : Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
  };
  /** マスの数。 */
  constexpr Square NUM_SQUARES = H8 + 1;

  // --- ファイルの定義 --- //
  /** ファイルの型。 */
  using Fyle = u32;
  /** ファイルの定数。 */
  enum : Fyle {
    FYLE_A, FYLE_B, FYLE_C, FYLE_D, FYLE_E, FYLE_F, FYLE_G, FYLE_H
  };
  /** ファイルの数。 */
  constexpr Fyle NUM_FYLES = FYLE_H + 1;

  // --- ランクの定義 --- //
  /** ランクの型。 */
  using Rank = u32;
  /** ランクの定数。 */
  enum : Rank {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
  };
  /** ランクの数。 */
  constexpr Rank NUM_RANKS = RANK_8 + 1;

  // --- サイドの定義 --- //
  /** サイドの型。 */
  using Side = u32;
  /** サイドの定数。 */
  enum : Side {
    NO_SIDE, WHITE, BLACK
  };
  /** サイドの数。 */
  constexpr Side NUM_SIDES = BLACK + 1;

  // --- 駒の定義 --- //
  /** 駒の型。 */
  using PieceType = u32;
  /** 駒の定数 */
  enum : PieceType {
    EMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
  };
  /** 駒の種類の数。 */
  constexpr PieceType NUM_PIECE_TYPES = KING + 1;

  /** キャスリングのフラグの型。 */
  using Castling = u8;
  /** 白のショートキャスリング。 */
  constexpr Castling WHITE_SHORT_CASTLING = 1;
  /** 白のロングキャスリング。 */
  constexpr Castling WHITE_LONG_CASTLING = 1 << 1;
  /** 黒のショートキャスリング。 */
  constexpr Castling BLACK_SHORT_CASTLING = 1 << 2;
  /** 黒のロングキャスリング。 */
  constexpr Castling BLACK_LONG_CASTLING = 1 << 3;
  /** 白のキャスリング。 */
  constexpr Castling WHITE_CASTLING =
  WHITE_SHORT_CASTLING | WHITE_LONG_CASTLING;
  /** 黒のキャスリング。 */
  constexpr Castling BLACK_CASTLING =
  BLACK_SHORT_CASTLING | BLACK_LONG_CASTLING;
  /** 全てのキャスリング。 */
  constexpr Castling ALL_CASTLING = WHITE_CASTLING | BLACK_CASTLING;

  /** 手の型。 */
  using Move = u32;

  // --- 手のビットフィールドの定数 --- //
  /** ビットフィールド名の定数。 */
  enum {
    FROM, TO, PROMOTION, MOVE_TYPE,
    CAPTURED_PIECE, CASTLING_RIGHTS, EN_PASSANT_SQUARE
  };
  /** マスのマスク。 6ビット。 */
  constexpr Move SQUARE_MASK = 0x3f;
  /** 駒のマスク。 3ビット。 */
  constexpr Move PIECE_MASK = 0x7;
  /** キャスリングのマスク。 4ビット。 */
  constexpr Move CASTLING_MASK = 0xf;
  /** 手の種類のマスク。 3ビット。 */
  constexpr Move MOVE_TYPE_MASK = 0x7;

  /** 各ビットフィールド用シフト。 */
  constexpr int SHIFT[EN_PASSANT_SQUARE + 1] {
    /** 動かす駒の位置のビット位置。 */
    0,
    /** 移動先の位置のビット位置。 */
    6,
    /** 昇格する駒の種類のビット位置。 */
    6 + 6,
    /** 手の種類のビット位置。 */
    6 + 6 + 3,
    /** 取った駒の種類のビット位置。 */
    6 + 6 + 3 + 3,
    /** キャスリングの権利のビット位置。 */
    6 + 6 + 3 + 3 + 3,
    /** アンパッサンの位置のビット位置。 */
    6 + 6 + 3 + 3 + 3 + 4
  };

  /** 各ビットフィールド用マスク。 */
  constexpr Move MASK[EN_PASSANT_SQUARE + 1] {
    /** 動かす駒の位置のマスク。 */
    SQUARE_MASK << SHIFT[FROM],
    /** 移動先の位置のマスク。 */
    SQUARE_MASK << SHIFT[TO],
    /** 昇格する駒のマスク。 */
    PIECE_MASK << SHIFT[PROMOTION],
    /** 手の種類のマスク。 */
    MOVE_TYPE_MASK << SHIFT[MOVE_TYPE],
    /** 取った駒の種類のマスク。 */
    PIECE_MASK << SHIFT[CAPTURED_PIECE],
    /** キャスリングの権利のマスク。 */
    CASTLING_MASK << SHIFT[CASTLING_RIGHTS],
    /** アンパッサンの位置のマスク。 */
    SQUARE_MASK << SHIFT[EN_PASSANT_SQUARE]
  };

  /** 手の比較に使うマスク。 */
  constexpr Move BASE_MASK = MASK[FROM] | MASK[TO] | MASK[PROMOTION];

  // --- 手の種類の定義 --- //
  /** 手の種類の型。 */
  using MoveType = unsigned int;
  /** 手の種類の定数。 */
  enum : MoveType {
    NULL_MOVE, NORMAL, EN_PASSANT, CASTLE_WS, CASTLE_WL, CASTLE_BS, CASTLE_BL
  };

  /**
   * アクセサ - Moveの各ビットフィールド。
   * @param <FIELD> ビットフィールド名。
   * @param move 対象のオブジェクト。
   * @return ビットフィールドの値。
   */
  template<int FIELD>
  inline u32 Get(Move move) {
    return (move & MASK[FIELD]) >> SHIFT[FIELD];
  }

  /**
   * ミューテータ - Moveの各ビットフィールド。
   * @param <FIELD> ビットフィールド名。
   * @param move 対象のオブジェクト。
   * @param val セットする値。
   */
  template<int FIELD>
  inline void Set(Move& move, u32 val) {
    move = (move & ~(MASK[FIELD])) | (val << SHIFT[FIELD]);
  }

  /**
   * 手を比較する。
   * @param move_1 比較対象。 その1。
   * @param move_2 比較対象。 その2。
   * @return 同じならtrue。
   */
  inline bool EqualMove(Move move_1, Move move_2) {
    return (move_1 & BASE_MASK) == (move_2 & BASE_MASK);
  }

  /** ハッシュの型。 */
  using Hash = u64;

  // --- 評価値の定義 --- //
  /** 勝ちの評価値。 */
  constexpr int SCORE_WIN = 1000000;
  /** 負けの評価値。 */
  constexpr int SCORE_LOSE = -SCORE_WIN;
  /** 引き分けの評価値。 */
  constexpr int SCORE_DRAW = 0;

  // 回転の角度を表す。
  enum {
    /** 0度。 */
    R0,
    /** 45度。 */
    R45,
    /** 90度。 */
    R90,
    /** 135度。 */
    R135
  };
  constexpr int NUM_ROTS = 4;

  // ======================= //
  // Transposition Table関連 //
  // ======================= //
  /** 評価値の種類。 TTEntry::score_type で使用。 */
  enum class ScoreType {
    /** 「正確な評価値」を表す。 */
    EXACT,
    /** 「アルファ値」を表す。 */
    ALPHA,
    /** 「ベータ値」を表す。 */
    BETA
  };

  // ============== //
  // 候補手の最大値 //
  // ============== //
  /** ポーンの候補手の最大値。 */
  constexpr u32 MAX_PAWN_CANDIDATES = 4;
  /** ナイトの候補手の最大値。 */
  constexpr u32 MAX_KNIGHT_CANDIDATES = 8;
  /** ビショップの候補手の最大値。 */
  constexpr u32 MAX_BISHOP_CANDIDATES = 13;
  /** ルークの候補手の最大値。 */
  constexpr u32 MAX_ROOK_CANDIDATES = 14;
  /** クイーンの候補手の最大値。 */
  constexpr u32 MAX_QUEEN_CANDIDATES =
  MAX_BISHOP_CANDIDATES + MAX_ROOK_CANDIDATES;
  /** キングの候補手の最大値。 */
  constexpr u32 MAX_KING_CANDIDATES = 8;

  /** 一つの局面における候補手の最大値。 */
  constexpr u32 MAX_CANDIDATES = (MAX_KNIGHT_CANDIDATES * 2)
  + (MAX_BISHOP_CANDIDATES * 2)
  + (MAX_ROOK_CANDIDATES * 2)
  + (MAX_QUEEN_CANDIDATES * 9)
  + (MAX_KING_CANDIDATES);

  // ================ //
  // 探索エンジン関連 //
  // ================ //
  /** とても大きな数。 */
  constexpr int MAX_VALUE = 9999999;

  /** 最大探索手数。 */
  constexpr u32 MAX_PLYS = 100;

  /** 最大探索ノード数。 */
  constexpr u64 MAX_NODES = ULLONG_MAX;

  /** 探索するノードの種類。 */
  enum class NodeType {
    /** PVノード。 */
    PV,
    /** PVノードではないノード。 */
    NON_PV
  };

  /** 生成する候補手の種類。 */
  enum class GenMoveType {
    /** 駒を取らない手。 */
    NON_CAPTURE,
    /** 駒を取る手。 */
    CAPTURE,
    /** 全ての手。 */
    ALL
  };

  /**
   * 制限時間から思考時間を計算する。
   * @param time_limit 制限時間。
   * @return 思考時間。
   */
  constexpr inline int TimeLimitToMoveTime(int time_limit) {
    return time_limit / 40;
  }

  // ====== //
  // その他 //
  // ====== //
  // --- 時間関連 --- //
  /** std::chrono の別名。 (長いので短くした。) */
  namespace Chrono = std::chrono;
  /** std::chrono::system_clock の別名。 (長いので短くした。) */
  using SysClock = Chrono::system_clock;
  /** std::chrono::system_clock::time_point の別名。 (長いので短くした。) */
  using TimePoint = SysClock::time_point;
}  // namespace Sayuri

#endif
