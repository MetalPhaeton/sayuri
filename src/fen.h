/* 
   fen.h: fenパーサのヘッダファイル。
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

#ifndef FEN_H
#define FEN_H

#include <iostream>
#include <string>
#include <vector>

#include "sayuri_debug.h"

namespace Sayuri {
  /*********************/
  /* FENパーサクラス。 */
  /*********************/
  class Fen {
    public:
      /********************/
      /* コンストラクタ。 */
      /********************/
      // コンストラクタ。
      // [引数]
      // fen_str: fenデータ。
      Fen(const std::string fen_str);

      Fen();
      Fen(const Fen& fen);
      Fen(Fen&& fen);
      Fen& operator=(const Fen& fen);
      Fen& operator=(Fen&& fen);

      /**************/
      /* アクセサ。 */
      /**************/
      // 駒の配置。
      const Bitboard (& position() const)[NUM_SIDES][NUM_PIECE_TYPES] {
        return position_;
      }
      // 手番。
      Side to_move() const {return to_move_;};
      // キャスリングの権利。
      Castling castling_rights() const {return castling_rights_;}
      // アンパッサンの位置。
      Square en_passant_square() const {return en_passant_square_;}
      // アンパッサン可能かどうか。
      bool can_en_passant() const {return can_en_passant_;}
      // 50手ルール。
      int ply_100() const {return ply_100_;}
      // 現在の手数。
      int ply() const {return ply_;}

      /****************/
      /* デバッグ用。 */
      /****************/
      friend int DebugMain(int argc, char* argv[]);

    private:
      /************/
      /* パーサ。 */
      /************/
      // 駒の配置をパースする。
      void ParsePosition(const std::string& position_str);
      // 手番をパースする。
      void ParseToMove(const std::string& to_move_str);
      // キャスリングの権利をパースする。
      void ParseCastlingRights(const std::string& castling_rights_str);
      // アンパッサンをパースする。
      void ParseEnPassant(const std::string& en_passant_str);
      // 50手ルールをパースする。
      void ParsePly100(const std::string& ply_100_str);
      // 手数をパースする。
      void ParsePly(const std::string& ply_str,
      const std::string& to_move_str);

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
      // アンパッサン可能かどうか。
      bool can_en_passant_;
      // 50手ルール。
      int ply_100_;
      // 現在の手数。
      int ply_;
  };
}  // namespace Sayuri

#endif
