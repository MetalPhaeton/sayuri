/* chess_def.h: チェスの定数の定義。
   copyright (c) 2011 石橋宏之利
 */

#ifndef CHESS_DEF_H
#define CHESS_DEF_H

#include <stdint.h>

namespace Misaki {
  // ビットボードの型。
  typedef uint64_t bitboard_t;

  // マスの型。
  enum square_t {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
  };

  // ファイルの型。
  enum fyle_t {
    FYLE_A, FYLE_B, FYLE_C, FYLE_D, FYLE_E, FYLE_F, FYLE_G, FYLE_H
  };

  // ランクの型。
  enum rank_t {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
  };

  // サイドの型。
  enum side_t {
    NO_SIDE, WHITE, BLACK
  };

  // 駒の型。
  enum piece_t {
    EMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
  };

  // 数の定義。
  enum {
    NUM_SQUARES = 64,
    NUM_FYLES = 8,
    NUM_RANKS = 8,
    NUM_SIDES = 3,
    NUM_PIECE_TYPES = 7
  };

  // キャスリングのフラグの型。
  typedef uint8_t castling_t;
  // それぞれのキャスリングのフラグ。
  extern const castling_t WHITE_SHORT_CASTLING;  // 白のショートキャスリング。
  extern const castling_t WHITE_LONG_CASTLING;  // 白のロングキャスリング。
  extern const castling_t BLACK_SHORT_CASTLING;  // 黒のショートキャスリング。
  extern const castling_t BLACK_LONG_CASTLING;  // 黒のロングキャスリング。
  extern const castling_t WHITE_CASTLING;  // 白の全てのキャスリング。
  extern const castling_t BLACK_CASTLING;  // 黒の全てのキャスリング。
  extern const castling_t ALL_CASTLING;  // 両方の全てのキャスリング。

  // 手の型。
  union move_t {
    uint32_t all_;
    struct {
      square_t piece_square_ : 6;  // 駒の位置。
      square_t goal_square_ : 6;  // 移動先の位置。
      piece_t captured_piece_ : 3;  // 取った駒の種類。
      piece_t promotion_ : 3;  // 昇格する駒の種類。
      castling_t last_castling_rights_ : 4;  // 動かす前のキャスリングのフラグ。
      // 動かす前のアンパッサンできるかどうか。
      bool last_can_en_passant_ : 1;
      // 動かす前のアンパッサンのターゲットの位置。
      square_t last_en_passant_target_ : 6;
      unsigned int move_type_ : 2;  // 手の種類。
    };
  };
  // 手の種類の定数。
  enum {
    NORMAL,  // 通常の動き。取る手も含む。
    CASTLING,  // キャスリング。
    EN_PASSANT,  // アンパッサン。
    NULL_MOVE  // Null Move。
  };

  // ハッシュの型。
  typedef uint64_t hash_key_t;
}  // Misaki

#endif
