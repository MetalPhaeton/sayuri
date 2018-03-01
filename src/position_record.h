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
 * @file position_record.h
 * @author Hironori Ishibashi
 * @brief エンジンのボードの状態を記録するクラス。
 */

#ifndef POSITION_RECORD_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define POSITION_RECORD_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include "board.h"
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;

  /** エンジンのボードの状態を記録するクラス。 */
  class PositionRecord : protected Board {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param engine 記録するエンジン。
       * @param pos_hash 現在のハッシュ。
       */
      PositionRecord(const ChessEngine& engine) {
        ScanMember(engine);
      }
      /** コンストラクタ。 */
      PositionRecord();
      PositionRecord(const PositionRecord&) = default;
      PositionRecord(PositionRecord&&) = default;
      PositionRecord& operator=(const PositionRecord&) = default;
      PositionRecord& operator=(PositionRecord&&) = default;
      /** デストラクタ。 */
      virtual ~PositionRecord() {}

      // ========== //
      // 比較演算子 //
      // ========== //
      /**
       * エンジンと状態が同じか比較する。
       * @param engine 比較するエンジン。
       * @return ボードの状態が同じならtrue。
       */
      bool operator==(const ChessEngine& engine) const;
      /**
       * エンジンと状態が違うか比較する。
       * @param engine 比較するエンジン。
       * @return ボードの状態が違うならtrue。
       */
      bool operator!=(const ChessEngine& engine) const;
      /**
       * PositionRecordと状態が同じか比較する。
       * @param record 比較するPositionRecord。
       * @return ボードの状態が同じならtrue。
       */
      bool operator==(const PositionRecord& record) const;
      /**
       * PositionRecordと状態が違うか比較する。
       * @param record 比較するPositionRecord。
       * @return ボードの状態が違うならtrue。
       */
      bool operator!=(const PositionRecord& record) const;

      // ============== //
      // パブリック関数 //
      // ============== //
      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - チェスボード。
       * @return チェスボード。
       */
      const Board& board() const {return *this;}
      /**
       * アクセサ - 駒の配置のビットボード。 [サイド][駒の種類]
       * @return 駒の配置のビットボード。
       */
      const Bitboard (& position() const)[NUM_SIDES][NUM_PIECE_TYPES] {
        return position_;
      }
      /**
       * アクセサ - 駒の種類の配置。
       * @return 駒の種類の配置。 [マス]
       */
      const PieceType (& piece_board() const)[NUM_SQUARES] {
        return piece_board_;
      }
      /**
       * アクセサ - サイドの配置。
       * @return サイドの配置。 [マス]
       */
      const Side (& side_board() const)[NUM_SQUARES] {
        return side_board_;
      }
      /**
       * アクセサ - 各サイドの駒の配置のビットボード。
       * @return 各サイドの駒の配置のビットボード。 [サイド]
       */
      const Bitboard (& side_pieces() const)[NUM_SIDES] {
        return side_pieces_;
      }
      /**
       * アクセサ - 全駒の配置のビットボード。 [角度]
       * @return 全駒の配置のビットボード。
       */
      const Bitboard (& blocker() const)[NUM_ROTS] {return blocker_;}
      /**
       * アクセサ - 各サイドのキングの位置。
       * @return 各サイドのキングの位置。 [サイド]
       */
      const Square (& king() const)[NUM_SIDES] {return king_;}
      /**
       * アクセサ - 手番。
       * @return 手番。
       */
      Side to_move() const {return to_move_;}
      /**
       * アクセサ - キャスリングの権利。
       * @return キャスリングの権利。
       */
      Castling castling_rights() const {return castling_rights_;}
      /**
       * アクセサ - アンパッサンの位置。 なければ0。
       * @return アンパッサンの位置。 なければ0。
       */
      Square en_passant_square() const {return en_passant_square_;}
      /**
       * アクセサ - 50手ルールの手数。
       * @return 50手ルールの手数。
       */
      int clock() const {return clock_;}
      /**
       * アクセサ - 手数。
       * @return 手数。
       */
      int ply() const {return ply_;}
      /**
       * アクセサ - キャスリングしたかどうかのフラグ。
       * @return キャスリングしたかどうかのフラグ。
       */
      const bool (& has_castled() const)[NUM_SIDES] {return has_castled_;}
      /**
       * アクセサ - 現在の局面のハッシュ。
       * @return 現在の局面のハッシュ。
       */
      Hash pos_hash() const {return pos_hash_;}
      /**
       * アクセサ - 探索中の配置のメモ。
       * @return 探索中の配置のメモ。
       */
      const Hash (& position_memo() const)[MAX_PLYS + 1] {
        return position_memo_;
      }

    private:
      /** ChessEngineはフレンド。 */
      friend class ChessEngine;

      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * メンバをエンジンからコピーする。
       * @param engine コピー元。
       */
      void ScanMember(const ChessEngine& engine);

      // ========== //
      // メンバ変数 //
      // ========== //
      /** 探索中の配置のメモ。 */
      Hash position_memo_[MAX_PLYS + 1];
      /** 現在の局面のハッシュ。 */
      Hash pos_hash_;
  };
}  // namespace Sayuri

#endif
