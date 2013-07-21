/* move_maker.h: 候補手を展開するクラスのヘッダ。

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

#ifndef MOVE_MAKER_H
#define MOVE_MAKER_H

#include <iostream>
#include <cstddef>
#include "chess_def.h"
#include "chess_engine.h"

namespace Sayuri {
  class ChessEngine;

  class MoveMaker {
    // Test
    friend class ChessEngine;

    private:
      // 手の構造体。
      struct MoveSlot {
        Move move_;
        int score_;

        MoveSlot() : score_(0) {move_.all_ = 0;}
      };

      /**************/
      /* 定数など。 */
      /**************/
      // 最大スロット数。
      static constexpr std::size_t MAX_SLOTS = 200;

    public:
      /********************/
      /* コンストラクタ。 */
      /********************/
      MoveMaker(ChessEngine* engine_ptr);
      MoveMaker() = delete;
      MoveMaker(const MoveMaker& maker);
      MoveMaker(MoveMaker&& maker);
      MoveMaker& operator=(const MoveMaker& maker);
      MoveMaker& operator=(MoveMaker&& maker);
      ~MoveMaker() {}

      /********************/
      /* パブリック関数。 */
      /********************/
      // スタックに候補手を展開する関数。
      // (注)TypeがNON_CAPTURE、CAPTUREの場合、
      // 自らチェックされる手も作る。
      // [引数]
      // depth: 手を展開するノードの深さ。
      // level: 手を展開するノードのレベル。
      // table: トランスポジションテーブル。
      template<GenMoveType GType> void GenMoves(int depth, int level,
      const TranspositionTable& table);
      // 展開できた手の数を返す。
      // [戻り値]
      // 展開できたての数。
      std::size_t GetSize() const;
      // 次の手を取り出す。
      // [戻り値]
      // 次の手。
      // もしなければmove.all_が0の手を返す。
      Move PickMove();

    private:
      /**********************/
      /* プライベート関数。 */
      /**********************/
      // 手に点数をつける関数。
      // GTypeがLEGALの場合は点数を付けない。
      // [引数]
      // begin: 点数をつける最初のスロットのポインタ。
      // end: 点数をつける最後のスロットの次のポインタ。
      // depth: 手を展開するノードの深さ。
      // level: 現在のノードのレベル。
      // table: トランスポジションテーブル。
      template<GenMoveType GType>
      void ScoreMoves(MoveSlot* begin, MoveSlot* end, int depth,
      int level, const TranspositionTable& table);
      // SEE。
      // [引数]
      // move: 探したい手。
      // side: 手番。
      // [戻り値]
      // 評価値。
      int SEE(Move move, Side side);
      // 最小価値の攻撃駒の動きを得る。
      Move GetSmallestAttackerMove(Square target, Side side) const;

      /****************/
      /* メンバ変数。 */
      /****************/
      // 親のチェスエンジン。
      ChessEngine* engine_ptr_;

      // 展開されるスタック。
      MoveSlot move_stack_[MAX_SLOTS + 1];
      // スタックのポインタ。
      MoveSlot* begin_;
      MoveSlot* last_;
      MoveSlot* current_;
      MoveSlot* end_;
  };
}  // namespace Sayuri

#endif
