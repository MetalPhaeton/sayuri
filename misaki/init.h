/* init.h: Misakiの初期化。
   copyright (c) 2011 石橋宏之利
 */

#ifndef INIT_H
#define INIT_H

#include <iostream>

#include "chess_util.h"
#include "chess_board.h"

namespace Misaki {
  // Misakiの初期化。
  inline void Init() {
    ChessUtil::InitChessUtil();
    ChessBoard::InitChessBoard();
  }
}  // Misaki

#endif
