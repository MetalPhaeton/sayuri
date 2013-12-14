/* move_maker.h: 候補手を展開するクラスのヘッダ。

   The MIT License (MIT)

   Copyright (c) 2013 Hironori Ishibashi

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
#include <mutex>
#include <cstddef>
#include "chess_def.h"
#include "chess_engine.h"

namespace Sayuri {
  class ChessEngine;

  class MoveMaker {
    private:
      // 手の構造体。
      struct MoveSlot {
        Move move_;
        int score_;

        MoveSlot() : score_(-MAX_VALUE) {}
      };

      /**************/
      /* 定数など。 */
      /**************/
      // 最大スロット数。
      static constexpr std::size_t MAX_SLOTS = 200;

    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // [引数]
      // engine: 手を作る対象のエンジン。
      MoveMaker(const ChessEngine& engine);
      MoveMaker(const MoveMaker& maker);
      MoveMaker(MoveMaker&& maker);
      MoveMaker& operator=(const MoveMaker& maker);
      MoveMaker& operator=(MoveMaker&& maker);
      MoveMaker() = delete;
      virtual ~MoveMaker() {}

      /********************/
      /* パブリック関数。 */
      /********************/
      // スタックに候補手を展開する関数。
      // (注)自らチェックされる手も作る。
      // [引数]
      // prev_best: TTに登録された前回の繰り返しの最善手。
      // iid_move: IIDによる最善手。
      // killer_1: キラームーブ。同一レベルのノードで記録した手。
      // killer_2: キラームーブ。2プライ前のレベルのノードで記録した手。
      template<GenMoveType Type> void GenMoves(Move prev_best,
      Move iid_move, Move killer_1, Move killer_2);
      // 次の手を取り出す。
      // [戻り値]
      // 次の手。
      // もしなければmove.all_が0の手を返す。
      Move PickMove();
      // スタック内に残っている候補手の数を返す。
      // [戻り値]
      // 残っている候補手の数。
      int CountMoves() const;

    private:
      /**********************/
      /* プライベート関数。 */
      /**********************/
      // 手に点数をつける。
      // [引数]
      // ptr: 点数をつけるスロットのポインタ。
      // prev_best: TTに登録された前回の繰り返しの最善手。
      // iid_move: IIDで得た手。
      // killer: キラームーブ。
      // killer_1: キラームーブ。同一レベルのノードで記録した手。
      // killer_2: キラームーブ。2プライ前のレベルのノードで記録した手。
      // side: 手のサイド。
      template<GenMoveType Type>
      void ScoreMove(MoveSlot* ptr,
      Move prev_best, Move iid_move, Move killer_1, Move killer_2, Side side);

      /****************/
      /* メンバ変数。 */
      /****************/
      // 親のチェスエンジン。
      const ChessEngine* engine_ptr_;

      // 展開されるスタック。
      MoveSlot move_stack_[MAX_SLOTS + 1];
      // スタックのポインタ。
      MoveSlot* begin_;
      MoveSlot* last_;
      MoveSlot* end_;
      // ミューテックス。
      std::mutex mutex_;
  };
}  // namespace Sayuri

#endif
