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
 * @file position_record.h
 * @author Hironori Ishibashi
 * @brief エンジンのボードの状態を記録するクラス。
 */

#ifndef POSITION_RECORD_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define POSITION_RECORD_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class ChessEngine;

  /** エンジンのボードの状態を記録するクラス。 */
  class PositionRecord {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param engine 記録するエンジン。
       */
      PositionRecord(const ChessEngine& engine);
      /** コンストラクタ。 */
      PositionRecord();
      /**
       * コピーコンストラクタ。
       * @param record コピー元。
       */
      PositionRecord(const PositionRecord& record);
      /**
       * ムーブコンストラクタ。
       * @param record ムーブ元。
       */
      PositionRecord(PositionRecord&& record);
      /**
       * コピー代入演算子。
       * @param record コピー元。
       */
      PositionRecord& operator=(const PositionRecord& record);
      /**
       * ムーブ代入演算子。
       * @param record ムーブ元。
       */
      PositionRecord& operator=(PositionRecord&& record);
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

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - 駒の配置のビットボード。 [サイド][駒の種類]
       * @return 駒の配置のビットボード。
       */
      const Bitboard (& position() const)[NUM_SIDES][NUM_PIECE_TYPES] {
        return position_;
      }
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
      int ply_100() const {return ply_100_;}
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

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * メンバをコピーする。
       * @param record コピー元。
       */
      void ScanMember(const PositionRecord& record);

      // ========== //
      // メンバ変数 //
      // ========== //
      /** 駒の配置のビットボード。 */
      Bitboard position_[NUM_SIDES][NUM_PIECE_TYPES];
      /** 手番。 */
      Side to_move_;
      /** キャスリングの権利。 */
      Castling castling_rights_;
      /** アンパッサンの位置。 */
      Square en_passant_square_;
      /** 50手ルールの手数。 */
      int ply_100_;
      /** 手数。 */
      int ply_;
      /** キャスリングしたかどうかのフラグ。 */
      bool has_castled_[NUM_SIDES];
      /** 現在の局面のハッシュ。 */
      Hash pos_hash_;
  };
}  // namespace Sayuri

#endif
