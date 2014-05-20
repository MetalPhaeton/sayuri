/*
   chess_def.h: チェスの定数の定義。

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

#ifndef CHESS_DEF_H
#define CHESS_DEF_H

#include <cstdint>
#include <cstddef>
#include <chrono>

namespace Sayuri {
  /******************/
  /* エンジン情報。 */
  /******************/
  constexpr const char* ID_NAME = "Sayuri 2014.05.20";
  constexpr const char* ID_AUTHOR = "Hironori Ishibashi";

  /*******************/
  /* UCIオプション。 */
  /*******************/
  constexpr std::size_t UCI_DEFAULT_TABLE_SIZE = 32ULL * 1024ULL * 1024ULL;
  constexpr std::size_t UCI_MIN_TABLE_SIZE = 8ULL * 1024ULL * 1024ULL;
#if defined(__i386__) || defined(_M_IX86)  // 32ビットCPU用定数。
  constexpr std::size_t UCI_MAX_TABLE_SIZE = 1024ULL * 1024ULL * 1024ULL;
#else  // 64ビットCPU用定数。
  constexpr std::size_t UCI_MAX_TABLE_SIZE =
  8ULL * 1024ULL * 1024ULL * 1024ULL;
#endif
  constexpr bool UCI_DEFAULT_PONDER = true;
  constexpr int UCI_DEFAULT_THREADS = 1;
  constexpr int UCI_MAX_THREADS = 64;
  constexpr bool UCI_DEFAULT_ANALYSE_MODE = false;

  /**********/
  /* 基本。 */
  /**********/
  // ビットボードの型。
  using Bitboard = std::uint64_t;

  // マスの型。
  using Square = std::uint32_t;
  constexpr Square A1 = 0;
  constexpr Square B1 = 1;
  constexpr Square C1 = 2;
  constexpr Square D1 = 3;
  constexpr Square E1 = 4;
  constexpr Square F1 = 5;
  constexpr Square G1 = 6;
  constexpr Square H1 = 7;
  constexpr Square A2 = 8;
  constexpr Square B2 = 9;
  constexpr Square C2 = 10;
  constexpr Square D2 = 11;
  constexpr Square E2 = 12;
  constexpr Square F2 = 13;
  constexpr Square G2 = 14;
  constexpr Square H2 = 15;
  constexpr Square A3 = 16;
  constexpr Square B3 = 17;
  constexpr Square C3 = 18;
  constexpr Square D3 = 19;
  constexpr Square E3 = 20;
  constexpr Square F3 = 21;
  constexpr Square G3 = 22;
  constexpr Square H3 = 23;
  constexpr Square A4 = 24;
  constexpr Square B4 = 25;
  constexpr Square C4 = 26;
  constexpr Square D4 = 27;
  constexpr Square E4 = 28;
  constexpr Square F4 = 29;
  constexpr Square G4 = 30;
  constexpr Square H4 = 31;
  constexpr Square A5 = 32;
  constexpr Square B5 = 33;
  constexpr Square C5 = 34;
  constexpr Square D5 = 35;
  constexpr Square E5 = 36;
  constexpr Square F5 = 37;
  constexpr Square G5 = 38;
  constexpr Square H5 = 39;
  constexpr Square A6 = 40;
  constexpr Square B6 = 41;
  constexpr Square C6 = 42;
  constexpr Square D6 = 43;
  constexpr Square E6 = 44;
  constexpr Square F6 = 45;
  constexpr Square G6 = 46;
  constexpr Square H6 = 47;
  constexpr Square A7 = 48;
  constexpr Square B7 = 49;
  constexpr Square C7 = 50;
  constexpr Square D7 = 51;
  constexpr Square E7 = 52;
  constexpr Square F7 = 53;
  constexpr Square G7 = 54;
  constexpr Square H7 = 55;
  constexpr Square A8 = 56;
  constexpr Square B8 = 57;
  constexpr Square C8 = 58;
  constexpr Square D8 = 59;
  constexpr Square E8 = 60;
  constexpr Square F8 = 61;
  constexpr Square G8 = 62;
  constexpr Square H8 = 63;

  // ファイルの型。
  using Fyle = std::uint32_t;
  constexpr Fyle FYLE_A = 0;
  constexpr Fyle FYLE_B = 1;
  constexpr Fyle FYLE_C = 2;
  constexpr Fyle FYLE_D = 3;
  constexpr Fyle FYLE_E = 4;
  constexpr Fyle FYLE_F = 5;
  constexpr Fyle FYLE_G = 6;
  constexpr Fyle FYLE_H = 7;

  // ランクの型。
  using Rank = std::uint32_t;
  constexpr Rank RANK_1 = 0;
  constexpr Rank RANK_2 = 1;
  constexpr Rank RANK_3 = 2;
  constexpr Rank RANK_4 = 3;
  constexpr Rank RANK_5 = 4;
  constexpr Rank RANK_6 = 5;
  constexpr Rank RANK_7 = 6;
  constexpr Rank RANK_8 = 7;

  // サイドの型。
  using Side = std::uint32_t;
  constexpr Side NO_SIDE = 0;
  constexpr Side WHITE = 1;
  constexpr Side BLACK = 2;

  // 駒の型。
  using Piece = std::uint32_t;
  constexpr Piece EMPTY = 0;
  constexpr Piece PAWN = 1;
  constexpr Piece KNIGHT = 2;
  constexpr Piece BISHOP = 3;
  constexpr Piece ROOK = 4;
  constexpr Piece QUEEN = 5;
  constexpr Piece KING = 6;

  // 数の定義。
  // マスの数。
  constexpr Square NUM_SQUARES = 64;
  // ファイルの数。
  constexpr Fyle NUM_FYLES = 8;
  // ランクの数。
  constexpr Rank NUM_RANKS = 8;
  // サイドの数。(NO_SIDEを含む。)
  constexpr Side NUM_SIDES = 3;
  // 駒の種類の数。(EMPTYを含む。)
  constexpr Piece NUM_PIECE_TYPES = 7;
  // とても大きな数。
  constexpr int MAX_VALUE = 9999999;

  // キャスリングのフラグの型。
  using Castling = std::uint8_t;
  // 白のショートキャスリング。
  constexpr Castling WHITE_SHORT_CASTLING = 1;
  // 白のロングキャスリング。
  constexpr Castling WHITE_LONG_CASTLING = 1 << 1;
  // 黒のショートキャスリング。
  constexpr Castling BLACK_SHORT_CASTLING = 1 << 2;
  // 黒のロングキャスリング。
  constexpr Castling BLACK_LONG_CASTLING = 1 << 3;
  // 白のキャスリング。
  constexpr Castling WHITE_CASTLING =
  WHITE_SHORT_CASTLING | WHITE_LONG_CASTLING;
  // 黒のキャスリング。
  constexpr Castling BLACK_CASTLING =
  BLACK_SHORT_CASTLING | BLACK_LONG_CASTLING;
  // 全キャスリング。
  constexpr Castling ALL_CASTLING = WHITE_CASTLING | BLACK_CASTLING;

  // 手の型。
  using Move = std::uint32_t;

  // 手のビットフィールドの定数。
  // 位置のマスク。6ビット。
  constexpr Move SQUARE_MASK = 0x3f;
  // 駒のマスク。3ビット。
  constexpr Move PIECE_MASK = 0x7;
  // キャスリングのマスク。4ビット。
  constexpr Move CASTLING_MASK = 0xf;
  // 手の種類のマスク。2ビット。
  constexpr Move MTYPE_MASK = 0x3;
  // 動かす駒の位置のビット位置。
  constexpr std::uint32_t FROM_SHIFT = 0;
  // 移動先の位置のビット位置。
  constexpr std::uint32_t TO_SHIFT = 6;
  // 昇格する駒の種類のビット位置。
  constexpr std::uint32_t PROMOTION_SHIFT = 12;
  // 駒の種類のビット位置。
  constexpr std::uint32_t MOVE_TYPE_SHIFT = 15;
  // 取った駒の種類のビット位置。
  constexpr std::uint32_t CAPTURED_PIECE_SHIFT = 17;
  // キャスリングの権利のビット位置。
  constexpr std::uint32_t CASTLING_RIGHTS_SHIFT = 20;
  // アンパッサンの位置のビット位置。
  constexpr std::uint32_t EN_PASSANT_SQUARE_SHIFT = 24;
  // 動かす駒の位置のマスク。
  constexpr Move FROM_MASK = SQUARE_MASK << FROM_SHIFT;
  // 移動先の位置のマスク。
  constexpr Move TO_MASK = SQUARE_MASK << TO_SHIFT;
  // 昇格する駒の位置のマスク。
  constexpr Move PROMOTION_MASK = PIECE_MASK << PROMOTION_SHIFT;
  // 手の種類のマスク。
  constexpr Move MOVE_TYPE_MASK = MTYPE_MASK << MOVE_TYPE_SHIFT;
  // 取った駒の種類のマスク。
  constexpr Move CAPTURED_PIECE_MASK = PIECE_MASK << CAPTURED_PIECE_SHIFT;
  // キャスリングの権利のマスク。
  constexpr Move CASTLING_RIGHTS_MASK =
  CASTLING_MASK << CASTLING_RIGHTS_SHIFT;
  // アンパッサンの位置のマスク。
  constexpr Move EN_PASSANT_SQUARE_MASK =
  SQUARE_MASK << EN_PASSANT_SQUARE_SHIFT;
  // 手の比較に使うマスク。
  constexpr Move BASE_MASK = FROM_MASK | TO_MASK | PROMOTION_MASK;

  // 手の種類の定数。
  // 手の種類の型。
  using MoveType = int;
  // ヌルムーブ。
  constexpr MoveType NULL_MOVE = 0;
  // 通常の手。(コマを取らない手。駒をとる手。)
  constexpr MoveType NORMAL = 1;
  // キャスリング。
  constexpr MoveType CASTLING = 2;
  // アンパッサン。
  constexpr MoveType EN_PASSANT = 3;

  // 手のビットフィールドのアクセサ。
  // 動かす駒の位置。
  inline Square move_from(Move& move) {
    return (move & FROM_MASK) >> FROM_SHIFT;
  }
  // 移動先の位置。
  inline Square move_to(Move& move) {
    return (move & TO_MASK) >> TO_SHIFT;
  }
  // 昇格する駒の種類。
  inline Piece move_promotion(Move& move) {
    return (move & PROMOTION_MASK) >> PROMOTION_SHIFT;
  }
  // 手の種類。
  inline MoveType move_move_type(Move& move) {
    return (move & MOVE_TYPE_MASK) >> MOVE_TYPE_SHIFT;
  }
  // 取った駒の種類。
  inline Piece move_captured_piece(Move& move) {
    return (move & CAPTURED_PIECE_MASK) >> CAPTURED_PIECE_SHIFT;
  }
  // キャスリングの権利。
  inline Castling move_castling_rights(Move& move) {
    return (move & CASTLING_RIGHTS_MASK) >> CASTLING_RIGHTS_SHIFT;
  }
  // アンパッサンの位置。
  inline Square move_en_passant_square(Move& move) {
    return (move & EN_PASSANT_SQUARE_MASK) >> EN_PASSANT_SQUARE_SHIFT;
  }

  // 手のビットフィールドのミューテータ。
  // 動かす駒の位置。
  inline void move_from(Move& move, Square from) {
    move = (move & ~FROM_MASK) | ((from & SQUARE_MASK) << FROM_SHIFT);
  }
  // 移動先の位置。
  inline void move_to(Move& move, Square to) {
    move = (move & ~TO_MASK) | ((to & SQUARE_MASK) << TO_SHIFT);
  }
  // 昇格する駒の種類。
  inline void move_promotion(Move& move, Piece promotion) {
    move = (move & ~PROMOTION_MASK)
    | ((promotion & PIECE_MASK) << PROMOTION_SHIFT);
  }
  // 手の種類。
  inline void move_move_type(Move& move, MoveType move_type) {
    move = (move & ~MOVE_TYPE_MASK)
    | ((move_type & MTYPE_MASK) << MOVE_TYPE_SHIFT);
  }
  // 取った駒の種類。
  inline void move_captured_piece(Move& move, Piece captured_piece) {
    move = (move & ~CAPTURED_PIECE_MASK)
    | ((captured_piece & PIECE_MASK) << CAPTURED_PIECE_SHIFT);
  }
  // キャスリングの権利。
  inline void move_castling_rights(Move& move, Castling castling_rights) {
    move = (move & ~CASTLING_RIGHTS_MASK)
    | ((castling_rights & CASTLING_MASK) << CASTLING_RIGHTS_SHIFT);
  }
  // アンパッサンの位置。
  inline void move_en_passant_square(Move& move, Square en_passant_square) {
    move = (move & ~EN_PASSANT_SQUARE_MASK)
    | ((en_passant_square & SQUARE_MASK) << EN_PASSANT_SQUARE_SHIFT);
  }

  // 手の比較。
  inline bool EqualMove(Move& move_1, Move& move_2) {
    return (move_1 & BASE_MASK) == (move_2 & BASE_MASK);
  }

  // ハッシュの型。
  using Hash = std::uint64_t;

  // 評価値の定義。
  // 勝ち。
  constexpr int SCORE_WIN = 1000000;
  // 負け。
  constexpr int SCORE_LOSE = -SCORE_WIN;
  // 引き分け。
  constexpr int SCORE_DRAW = 0;

  /*****************************/
  /* Transposition Table関連。 */
  /*****************************/
  // 評価値の種類。TTEntry::score_type。
  enum class ScoreType {
    EXACT,  // 正確な評価値。
    ALPHA,  // アルファ値。
    BETA  // ベータ値。
  };

  /********************/
  /* 候補手の最大値。 */
  /********************/
  // ポーンの候補手の最大値。
  constexpr std::uint32_t MAX_PAWN_CANDIDATES = 4;
  // ナイトの候補手の最大値。
  constexpr std::uint32_t MAX_KNIGHT_CANDIDATES = 8;
  // ビショップの候補手の最大値。
  constexpr std::uint32_t MAX_BISHOP_CANDIDATES = 13;
  // ルークの候補手の最大値。
  constexpr std::uint32_t MAX_ROOK_CANDIDATES = 14;
  // クイーンの候補手の最大値。
  constexpr std::uint32_t MAX_QUEEN_CANDIDATES =
  MAX_BISHOP_CANDIDATES + MAX_ROOK_CANDIDATES;
  // キングの候補手の最大値。
  constexpr std::uint32_t MAX_KING_CANDIDATES = 8;

  // 一つの局面における候補手の最大値。
  constexpr std::uint32_t MAX_CANDIDATES = (MAX_KNIGHT_CANDIDATES * 2)
  + (MAX_BISHOP_CANDIDATES * 2)
  + (MAX_ROOK_CANDIDATES * 2)
  + (MAX_QUEEN_CANDIDATES * 9)
  + (MAX_KING_CANDIDATES);

  /**********************/
  /* 探索エンジン関連。 */
  /**********************/
  // 最大探索手数。
  constexpr std::uint32_t MAX_PLYS = 100;

  // 最大探索ノード数。
  constexpr std::uint64_t MAX_NODES = -1ULL;

  // 探索するノードの種類。
  enum class NodeType {
    PV,  // PVノード。
    NON_PV  // その他のノード。
  };

  // 候補手の種類。
  enum class GenMoveType {
    NON_CAPTURE,  // 駒を取らない手。
    CAPTURE,  // 駒をとる手。
    ALL  // 両方。
  };

  /************/
  /* その他。 */
  /************/
  // 時間関連。
  namespace Chrono = std::chrono;
  using SysClock = Chrono::system_clock;
  using TimePoint = SysClock::time_point;
}  // namespace Sayuri

#endif
