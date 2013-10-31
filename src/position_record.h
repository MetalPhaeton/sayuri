/* 
   position_record.h: ボードの状態を記録するクラス。

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

#ifndef POSITION_RECORD_H
#define POSITION_RECORD_H

#include <iostream>
#include "chess_def.h"
#include "chess_engine.h"

namespace Sayuri {
  class ChessEngine;

  // ボードの状態を記録するクラス。
  class PositionRecord {
    public:
      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
      // [引数]
      // engine: 記録したい配置のエンジン。
      PositionRecord(const ChessEngine& engine);
      PositionRecord();
      PositionRecord(const PositionRecord& record);
      PositionRecord(PositionRecord&& record);
      PositionRecord& operator=(const PositionRecord& record);
      PositionRecord& operator=(PositionRecord&& record);
      virtual ~PositionRecord() {}

      /****************/
      /* 比較演算子。 */
      /****************/
      bool operator==(const ChessEngine& engine) const;
      bool operator!=(const ChessEngine& engine) const;
      bool operator==(const PositionRecord& record) const;
      bool operator!=(const PositionRecord& record) const;

      /**************/
      /* アクセサ。 */
      /**************/
      // 駒の配置。
      const Bitboard (& position() const)[NUM_SIDES][NUM_PIECE_TYPES] {
        return position_;
      }
      // 手番。
      Side to_move() const {return to_move_;}
      // キャスリングの権利。
      Castling castling_rights() const {return castling_rights_;}
      // アンパッサンの位置。
      Square en_passant_square() const {return en_passant_square_;}
      // アンパッサンできるかどうか。
      bool can_en_passant() const {return can_en_passant_;}
      // 配置のハッシュ。
      Hash pos_hash() const {return pos_hash_;}

    private:
      /**********************/
      /* プライベート関数。 */
      /**********************/
      // パラメータをコピーする。
      // [引数]
      // コピー元のオブジェクト。
      void ScanMember(const PositionRecord& record);

      /****************/
      /* メンバ変数。 */
      /****************/
      // 駒の配置。
      Bitboard position_[NUM_SIDES][NUM_PIECE_TYPES];
      // 手番。
      Side to_move_;
      // キャスリングの権利。
      Castling castling_rights_;
      // アンパッサンの位置。
      Square en_passant_square_;
      // アンパッサンできるかどうか。
      bool can_en_passant_;
      // 配置のハッシュ。
      Hash pos_hash_;
  };
}  // namespace Sayuri

#endif
