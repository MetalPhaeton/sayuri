/* chess_def.cpp: チェスの定数の定義。
   copyright (c) 2011 石橋宏之利
 */

#include "chess_def.h"

namespace Misaki {
  // キャスリングのフラグ。
  const castling_t WHITE_SHORT_CASTLING = 1;  // 白のショートキャスリング。
  const castling_t WHITE_LONG_CASTLING = 1 << 1;  // 白のロングキャスリング。
  const castling_t BLACK_SHORT_CASTLING = 1 << 2;  // 黒のショートキャスリング。
  const castling_t BLACK_LONG_CASTLING = 1 << 3;  // 黒のロングキャスリング。
  const castling_t WHITE_CASTLING =
  WHITE_SHORT_CASTLING | WHITE_LONG_CASTLING;  // 白の全てのキャスリング。
  const castling_t BLACK_CASTLING =
  BLACK_SHORT_CASTLING | BLACK_LONG_CASTLING;  // 黒の全てのキャスリング。
  const castling_t ALL_CASTLING =
  WHITE_CASTLING | BLACK_CASTLING;  // 両方の全てのキャスリング。
}  // Misaki
