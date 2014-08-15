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
 * @file chess_def.h
 * @author Hironori Ishibashi
 * @brief Sayuriで使う定義全般。
 */

#ifndef CHESS_DEF_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define CHESS_DEF_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <cstdint>
#include <cstddef>
#include <chrono>

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ============ //
  // エンジン情報 //
  // ============ //
  /** Sayuriのバージョン番号。 */
  constexpr const char* ID_NAME = "Sayuri 2014.08.16";
  /** Sayuriの作者名。 */
  constexpr const char* ID_AUTHOR = "Hironori Ishibashi";

  // ============= //
  // UCIオプション //
  // ============= //
  /** トランスポジションテーブルのデフォルトサイズ。 */
  constexpr std::size_t UCI_DEFAULT_TABLE_SIZE = 32ULL * 1024ULL * 1024ULL;

  /** トランスポジションテーブルの最小サイズ。 */
  constexpr std::size_t UCI_MIN_TABLE_SIZE = 8ULL * 1024ULL * 1024ULL;

#if defined(__arm__)  // ARMのCPU用定数。
  /** トランスポジションテーブルの最大サイズ。 (ARM用) */
  constexpr std::size_t UCI_MAX_TABLE_SIZE = 1024ULL * 1024ULL * 1024ULL;

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

  // ========== //
  // 基本の定数 //
  // ========== //
  /** ビットボードの型。 */
  using Bitboard = std::uint64_t;

  /** マスの型。 */
  using Square = std::uint32_t;
  /** マス A1 の定数。 */
  constexpr Square A1 = 0;
  /** マス B1 の定数。 */
  constexpr Square B1 = 1;
  /** マス C1 の定数。 */
  constexpr Square C1 = 2;
  /** マス D1 の定数。 */
  constexpr Square D1 = 3;
  /** マス E1 の定数。 */
  constexpr Square E1 = 4;
  /** マス F1 の定数。 */
  constexpr Square F1 = 5;
  /** マス G1 の定数。 */
  constexpr Square G1 = 6;
  /** マス H1 の定数。 */
  constexpr Square H1 = 7;
  /** マス A2 の定数。 */
  constexpr Square A2 = 8;
  /** マス B2 の定数。 */
  constexpr Square B2 = 9;
  /** マス C2 の定数。 */
  constexpr Square C2 = 10;
  /** マス D2 の定数。 */
  constexpr Square D2 = 11;
  /** マス E2 の定数。 */
  constexpr Square E2 = 12;
  /** マス F2 の定数。 */
  constexpr Square F2 = 13;
  /** マス G2 の定数。 */
  constexpr Square G2 = 14;
  /** マス H2 の定数。 */
  constexpr Square H2 = 15;
  /** マス A3 の定数。 */
  constexpr Square A3 = 16;
  /** マス B3 の定数。 */
  constexpr Square B3 = 17;
  /** マス C3 の定数。 */
  constexpr Square C3 = 18;
  /** マス D3 の定数。 */
  constexpr Square D3 = 19;
  /** マス E3 の定数。 */
  constexpr Square E3 = 20;
  /** マス F3 の定数。 */
  constexpr Square F3 = 21;
  /** マス G3 の定数。 */
  constexpr Square G3 = 22;
  /** マス H3 の定数。 */
  constexpr Square H3 = 23;
  /** マス A4 の定数。 */
  constexpr Square A4 = 24;
  /** マス B4 の定数。 */
  constexpr Square B4 = 25;
  /** マス C4 の定数。 */
  constexpr Square C4 = 26;
  /** マス D4 の定数。 */
  constexpr Square D4 = 27;
  /** マス E4 の定数。 */
  constexpr Square E4 = 28;
  /** マス F4 の定数。 */
  constexpr Square F4 = 29;
  /** マス G4 の定数。 */
  constexpr Square G4 = 30;
  /** マス H4 の定数。 */
  constexpr Square H4 = 31;
  /** マス A5 の定数。 */
  constexpr Square A5 = 32;
  /** マス B5 の定数。 */
  constexpr Square B5 = 33;
  /** マス C5 の定数。 */
  constexpr Square C5 = 34;
  /** マス D5 の定数。 */
  constexpr Square D5 = 35;
  /** マス E5 の定数。 */
  constexpr Square E5 = 36;
  /** マス F5 の定数。 */
  constexpr Square F5 = 37;
  /** マス G5 の定数。 */
  constexpr Square G5 = 38;
  /** マス H5 の定数。 */
  constexpr Square H5 = 39;
  /** マス A6 の定数。 */
  constexpr Square A6 = 40;
  /** マス B6 の定数。 */
  constexpr Square B6 = 41;
  /** マス C6 の定数。 */
  constexpr Square C6 = 42;
  /** マス D6 の定数。 */
  constexpr Square D6 = 43;
  /** マス E6 の定数。 */
  constexpr Square E6 = 44;
  /** マス F6 の定数。 */
  constexpr Square F6 = 45;
  /** マス G6 の定数。 */
  constexpr Square G6 = 46;
  /** マス H6 の定数。 */
  constexpr Square H6 = 47;
  /** マス A7 の定数。 */
  constexpr Square A7 = 48;
  /** マス B7 の定数。 */
  constexpr Square B7 = 49;
  /** マス C7 の定数。 */
  constexpr Square C7 = 50;
  /** マス D7 の定数。 */
  constexpr Square D7 = 51;
  /** マス E7 の定数。 */
  constexpr Square E7 = 52;
  /** マス F7 の定数。 */
  constexpr Square F7 = 53;
  /** マス G7 の定数。 */
  constexpr Square G7 = 54;
  /** マス H7 の定数。 */
  constexpr Square H7 = 55;
  /** マス A8 の定数。 */
  constexpr Square A8 = 56;
  /** マス B8 の定数。 */
  constexpr Square B8 = 57;
  /** マス C8 の定数。 */
  constexpr Square C8 = 58;
  /** マス D8 の定数。 */
  constexpr Square D8 = 59;
  /** マス E8 の定数。 */
  constexpr Square E8 = 60;
  /** マス F8 の定数。 */
  constexpr Square F8 = 61;
  /** マス G8 の定数。 */
  constexpr Square G8 = 62;
  /** マス H8 の定数。 */
  constexpr Square H8 = 63;

  /** ファイルの型。 */
  using Fyle = std::uint32_t;
  /** ファイル A の定数。 */
  constexpr Fyle FYLE_A = 0;
  /** ファイル B の定数。 */
  constexpr Fyle FYLE_B = 1;
  /** ファイル C の定数。 */
  constexpr Fyle FYLE_C = 2;
  /** ファイル D の定数。 */
  constexpr Fyle FYLE_D = 3;
  /** ファイル E の定数。 */
  constexpr Fyle FYLE_E = 4;
  /** ファイル F の定数。 */
  constexpr Fyle FYLE_F = 5;
  /** ファイル G の定数。 */
  constexpr Fyle FYLE_G = 6;
  /** ファイル H の定数。 */
  constexpr Fyle FYLE_H = 7;

  /** ランクの型。 */
  using Rank = std::uint32_t;
  /** ランク 1 の定数。 */
  constexpr Rank RANK_1 = 0;
  /** ランク 2 の定数。 */
  constexpr Rank RANK_2 = 1;
  /** ランク 3 の定数。 */
  constexpr Rank RANK_3 = 2;
  /** ランク 4 の定数。 */
  constexpr Rank RANK_4 = 3;
  /** ランク 5 の定数。 */
  constexpr Rank RANK_5 = 4;
  /** ランク 6 の定数。 */
  constexpr Rank RANK_6 = 5;
  /** ランク 7 の定数。 */
  constexpr Rank RANK_7 = 6;
  /** ランク 8 の定数。 */
  constexpr Rank RANK_8 = 7;

  /** サイドの型。 */
  using Side = std::uint32_t;
  /** どちらのサイドでもない。 */
  constexpr Side NO_SIDE = 0;
  /** 白側の定数。 */
  constexpr Side WHITE = 1;
  /** 黒側の定数。 */
  constexpr Side BLACK = 2;

  /** 駒の型。 */
  using Piece = std::uint32_t;
  /** どの駒でもない。 */
  constexpr Piece EMPTY = 0;
  /** ポーンの定数。 */
  constexpr Piece PAWN = 1;
  /** ナイトの定数。 */
  constexpr Piece KNIGHT = 2;
  /** ビショップの定数。 */
  constexpr Piece BISHOP = 3;
  /** ルークの定数。 */
  constexpr Piece ROOK = 4;
  /** クイーンの定数。 */
  constexpr Piece QUEEN = 5;
  /** キングの定数。 */
  constexpr Piece KING = 6;

  // --- 数の定義 --- //
  /** マスの数。 */
  constexpr Square NUM_SQUARES = 64;
  /** ファイルの数。 */
  constexpr Fyle NUM_FYLES = 8;
  /** ランクの数。 */
  constexpr Rank NUM_RANKS = 8;
  /** サイドの数。(NO_SIDEを含む。) */
  constexpr Side NUM_SIDES = 3;
  /** 駒の種類の数。(EMPTYを含む。) */
  constexpr Piece NUM_PIECE_TYPES = 7;
  /** とても大きな数。 */
  constexpr int MAX_VALUE = 9999999;

  /** キャスリングのフラグの型。 */
  using Castling = std::uint8_t;
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
  using Move = std::uint32_t;

  // --- 手のビットフィールドの定数 --- //
  /** マスのマスク。 6ビット。 */
  constexpr Move SQUARE_MASK = 0x3f;
  /** 駒のマスク。 3ビット。 */
  constexpr Move PIECE_MASK = 0x7;
  /** キャスリングのマスク。 4ビット。 */
  constexpr Move CASTLING_MASK = 0xf;
  /** 手の種類のマスク。 3ビット。 */
  constexpr Move MTYPE_MASK = 0x7;

  /** 動かす駒の位置のビット位置。 */
  constexpr unsigned int FROM_SHIFT = 0;
  /** 移動先の位置のビット位置。 */
  constexpr unsigned int TO_SHIFT = FROM_SHIFT + 6;
  /** 昇格する駒の種類のビット位置。 */
  constexpr unsigned int PROMOTION_SHIFT = TO_SHIFT + 6;
  /** 手の種類のビット位置。 */
  constexpr unsigned int MOVE_TYPE_SHIFT = PROMOTION_SHIFT + 3;
  /** 取った駒の種類のビット位置。 */
  constexpr unsigned int CAPTURED_PIECE_SHIFT = MOVE_TYPE_SHIFT + 3;
  /** キャスリングの権利のビット位置。 */
  constexpr unsigned int CASTLING_RIGHTS_SHIFT = CAPTURED_PIECE_SHIFT + 3;
  /** アンパッサンの位置のビット位置。 */
  constexpr unsigned int EN_PASSANT_SQUARE_SHIFT = CASTLING_RIGHTS_SHIFT + 4;

  /** 動かす駒の位置のマスク。 */
  constexpr Move FROM_MASK = SQUARE_MASK << FROM_SHIFT;
  /** 移動先の位置のマスク。 */
  constexpr Move TO_MASK = SQUARE_MASK << TO_SHIFT;
  /** 昇格する駒のマスク。 */
  constexpr Move PROMOTION_MASK = PIECE_MASK << PROMOTION_SHIFT;
  /** 手の種類のマスク。 */
  constexpr Move MOVE_TYPE_MASK = MTYPE_MASK << MOVE_TYPE_SHIFT;
  /** 取った駒の種類のマスク。 */
  constexpr Move CAPTURED_PIECE_MASK = PIECE_MASK << CAPTURED_PIECE_SHIFT;
  /** キャスリングの権利のマスク。 */
  constexpr Move CASTLING_RIGHTS_MASK =
  CASTLING_MASK << CASTLING_RIGHTS_SHIFT;
  /** アンパッサンの位置のマスク。 */
  constexpr Move EN_PASSANT_SQUARE_MASK =
  SQUARE_MASK << EN_PASSANT_SQUARE_SHIFT;

  /** 手の比較に使うマスク。 */
  constexpr Move BASE_MASK = FROM_MASK | TO_MASK | PROMOTION_MASK;

  // --- 手の種類の定数 --- //
  /** 手の種類の型。 */
  using MoveType = unsigned int;
  /** Null Moveを表す。 */
  constexpr MoveType NULL_MOVE = 0;
  /** 通常の手を表す。 (コマを取らない手。 駒をとる手。) */
  constexpr MoveType NORMAL = 1;
  /** アンパッサンを表す。 */
  constexpr MoveType EN_PASSANT = 2;
  /** 白のショートキャスリングを表す。 */
  constexpr MoveType CASTLE_WS = 3;
  /** 白のロングキャスリングを表す。 */
  constexpr MoveType CASTLE_WL = 4;
  /** 黒のショートキャスリングを表す。 */
  constexpr MoveType CASTLE_BS = 5;
  /** 黒のロングキャスリングを表す。 */
  constexpr MoveType CASTLE_BL = 6;

  // --- 手のビットフィールドのアクセサ --- //
  /**
   * アクセサ - 動かす駒の位置。
   * @param move 対象のオブジェクト。
   * @return 動かす駒の位置。
   */
#define GET_FROM(move) ((move & FROM_MASK) >> FROM_SHIFT)
  /**
   * アクセサ - 移動先の位置。
   * @param move 対象のオブジェクト。
   * @return 移動先の位置。
   */
#define GET_TO(move) ((move & TO_MASK) >> TO_SHIFT)
  /**
   * アクセサ - 昇格する駒の種類。
   * @param move 対象のオブジェクト。
   * @return 昇格する駒の種類。
   */
#define GET_PROMOTION(move) ((move & PROMOTION_MASK) >> PROMOTION_SHIFT)
  /**
   * アクセサ - 手の種類。
   * @param move 対象のオブジェクト。
   * @return 手の種類。
   */
#define GET_MOVE_TYPE(move) ((move & MOVE_TYPE_MASK) >> MOVE_TYPE_SHIFT)
  /**
   * アクセサ - 取った駒の種類。
   * @param move 対象のオブジェクト。
   * @return 取った駒の種類。
   */
#define GET_CAPTURED_PIECE(move) \
((move & CAPTURED_PIECE_MASK) >> CAPTURED_PIECE_SHIFT)
  /**
   * アクセサ - キャスリングの権利。
   * @param move 対象のオブジェクト。
   * @return キャスリングの権利。
   */
#define GET_CASTLING_RIGHTS(move) \
((move & CASTLING_RIGHTS_MASK) >> CASTLING_RIGHTS_SHIFT)
  /**
   * アクセサ - アンパッサンの位置。
   * @param move 対象のオブジェクト。
   * @return アンパッサンの位置。
   */
#define GET_EN_PASSANT_SQUARE(move) \
((move & EN_PASSANT_SQUARE_MASK) >> EN_PASSANT_SQUARE_SHIFT)

  // --- 手のビットフィールドのミューテータ --- //
  /**
   * ミューテータ - 動かす駒の位置。
   * @param move 対象のオブジェクト。
   * @param from 動かす駒の位置。
   */
#define SET_FROM(move, from) \
(move = (move & ~FROM_MASK) | ((from & SQUARE_MASK) << FROM_SHIFT))
  /**
   * ミューテータ - 移動先の位置。
   * @param move 対象のオブジェクト。
   * @param to 移動先の位置。
   */
#define SET_TO(move, to) \
(move = (move & ~TO_MASK) | ((to & SQUARE_MASK) << TO_SHIFT))
  /**
   * ミューテータ - 昇格する駒の種類。
   * @param move 対象のオブジェクト。
   * @param promotion 昇格する駒の種類。
   */
#define SET_PROMOTION(move, promotion) \
(move = (move & ~PROMOTION_MASK) \
| ((promotion & PIECE_MASK) << PROMOTION_SHIFT))
  /**
   * ミューテータ - 手の種類。
   * @param move 対象のオブジェクト。
   * @param move_type 手の種類。
   */
#define SET_MOVE_TYPE(move, move_type) \
(move = (move & ~MOVE_TYPE_MASK) \
| ((move_type & MTYPE_MASK) << MOVE_TYPE_SHIFT))
  /**
   * ミューテータ - 取った駒の種類。
   * @param move 対象のオブジェクト。
   * @param captured_piece 取った駒の種類。
   */
#define SET_CAPTURED_PIECE(move, captured_piece) \
(move = (move & ~CAPTURED_PIECE_MASK) \
| ((captured_piece & PIECE_MASK) << CAPTURED_PIECE_SHIFT))
  /**
   * ミューテータ - キャスリングの権利。
   * @param move 対象のオブジェクト。
   * @param castling_rights キャスリングの権利。
   */
#define SET_CASTLING_RIGHTS(move, castling_rights) \
(move = (move & ~CASTLING_RIGHTS_MASK) \
| ((castling_rights & CASTLING_MASK) << CASTLING_RIGHTS_SHIFT))
  /**
   * ミューテータ - アンパッサンの位置。
   * @param move 対象のオブジェクト。
   * @param en_passant_square アンパッサンの位置。
   */
#define SET_EN_PASSANT_SQUARE(move, en_passant_square) \
(move = (move & ~EN_PASSANT_SQUARE_MASK) \
| ((en_passant_square & SQUARE_MASK) << EN_PASSANT_SQUARE_SHIFT))

  /**
   * 手を比較する。
   * @param move_1 比較対象。 その1。
   * @param move_2 比較対象。 その2。
   * @return 同じならtrue。
   */
#define EQUAL_MOVE(move_1, move_2) \
((move_1 & BASE_MASK) == (move_2 & BASE_MASK))

  /** ハッシュの型。 */
  using Hash = std::uint64_t;

  // --- 評価値の定義 --- //
  /** 勝ちの評価値。 */
  constexpr int SCORE_WIN = 1000000;
  /** 負けの評価値。 */
  constexpr int SCORE_LOSE = -SCORE_WIN;
  /** 引き分けの評価値。 */
  constexpr int SCORE_DRAW = 0;

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
  constexpr std::uint32_t MAX_PAWN_CANDIDATES = 4;
  /** ナイトの候補手の最大値。 */
  constexpr std::uint32_t MAX_KNIGHT_CANDIDATES = 8;
  /** ビショップの候補手の最大値。 */
  constexpr std::uint32_t MAX_BISHOP_CANDIDATES = 13;
  /** ルークの候補手の最大値。 */
  constexpr std::uint32_t MAX_ROOK_CANDIDATES = 14;
  /** クイーンの候補手の最大値。 */
  constexpr std::uint32_t MAX_QUEEN_CANDIDATES =
  MAX_BISHOP_CANDIDATES + MAX_ROOK_CANDIDATES;
  /** キングの候補手の最大値。 */
  constexpr std::uint32_t MAX_KING_CANDIDATES = 8;

  /** 一つの局面における候補手の最大値。 */
  constexpr std::uint32_t MAX_CANDIDATES = (MAX_KNIGHT_CANDIDATES * 2)
  + (MAX_BISHOP_CANDIDATES * 2)
  + (MAX_ROOK_CANDIDATES * 2)
  + (MAX_QUEEN_CANDIDATES * 9)
  + (MAX_KING_CANDIDATES);

  // ================ //
  // 探索エンジン関連 //
  // ================ //
  /** 最大探索手数。 */
  constexpr std::uint32_t MAX_PLYS = 100;

  /** 最大探索ノード数。 */
  constexpr std::uint64_t MAX_NODES = -1ULL;

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

  // ================ //
  // ビットボード演算 //
  // ================ //
  /**
   * 次のマスを指定するビットボードに変換する。
   * (1のビットの一番下位のビットを0にする。)
   * @param bitboard 対象のビットボード。
   */
#define NEXT(bitboard) (bitboard &= bitboard - 1)

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
