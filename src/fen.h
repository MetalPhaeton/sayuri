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
 * @file fen.h
 * @author Hironori Ishibashi
 * @brief FEN文字列のパーサ。
 */

#ifndef FEN_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define FEN_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <string>
#include <vector>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /** FEN文字列のパーサ。 */
  class Fen {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      // [引数]
      // fen_str: fenデータ。
      /**
       * コンストラクタ。
       * @param fen_str FEN文字列。
       */
      Fen(const std::string fen_str);
      /** コンストラクタ。 */
      Fen();
      /**
       * コピーコンストラクタ。
       * @param fen コピー元。
       */
      Fen(const Fen& fen);
      /**
       * ムーブコンストラクタ。
       * @param fen ムーブ元。
       */
      Fen(Fen&& fen);
      /**
       * コピー代入演算子。
       * @param fen コピー元。
       */
      Fen& operator=(const Fen& fen);
      /**
       * ムーブ代入演算子。
       * @param fen ムーブ元。
       */
      Fen& operator=(Fen&& fen);
      /** デストラクタ。 */
      virtual ~Fen() {}

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - 駒の配置のビットボード。
       * @return 駒の配置のビットボード。 [サイド][駒の種類]
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
       * アクセサ - アンパッサンの位置。
       * @return アンパッサンの位置。
       */
      Square en_passant_square() const {return en_passant_square_;}
      /**
       * アクセサ - 50手ルールの手数。
       * @return 50手ルールの手数。
       */
      int ply_100() const {return ply_100_;}
      /**
       * アクセサ - 現在の手数。
       * @return 現在の手数。
       */
      int ply() const {return ply_;}

    private:
      // =========== //
      // FEN評価関数 //
      // =========== //
      /**
       * 駒の配置文字列を評価する。
       * @param position_str 駒の配置文字列。
       */
      void EvalPosition(const std::string& position_str);
      /**
       * 手番文字列を評価する。
       * @param to_move_str 手番文字列。
       */
      void EvalToMove(const std::string& to_move_str);
      /**
       * キャスリングの権利文字列を評価する。
       * @param castling_rights_str キャスリングの権利文字列。
       */
      void EvalCastlingRights(const std::string& castling_rights_str);
      /**
       * アンパッサンの位置文字列を評価する。
       * @param en_passant_square_str アンパッサンの位置文字列。
       */
      void EvalEnPassant(const std::string& en_passant_square_str);
      /**
       * 50手ルールの手数文字列を評価する。
       * @param ply_100_str 50手ルールの手数文字列。
       */
      void EvalPly100(const std::string& ply_100_str);
      /**
       * 手数文字列を評価する。
       * @param ply_str 手数文字列。
       */
      void EvalPly(const std::string& ply_str);

      // ================ //
      // プライベート関数 //
      // ================ //
      /** スタートポジションにセット。 */
      void SetStartPosition();

      // ========== //
      // メンバ変数 //
      // ========== //
      /** 駒の配置のビットボード。 [サイド][駒の種類] */
      Bitboard position_[NUM_SIDES][NUM_PIECE_TYPES];
      /** 手番。 */
      Side to_move_;
      /** キャスリングの権利。 */
      Castling castling_rights_;
      /** アンパッサンの位置。アンパッサンできなければ0。 */
      Square en_passant_square_;
      /** 50手ルールの手数。 */
      int ply_100_;
      /** 現在の手数。 */
      int ply_;
  };
}  // namespace Sayuri

#endif
