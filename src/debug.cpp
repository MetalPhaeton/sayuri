/* 
   debug.cpp: Sayuriをデバッグする。

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

#include "debug.h"

#include <iostream>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <sstream>
#include <random>
#include <vector>
#include <thread>
#include "chess_def.h"
#include "chess_util.h"
#include "init.h"
#include "error.h"
#include "chess_engine.h"
#include "uci_shell.h"
#include "pv_line.h"

namespace Sayuri {
  /**************************/
  /* デバッグ用メイン関数。 */
  /**************************/
  // ================================================================
  int DebugMain(int argc, char* argv[]) {
    return 0;
  }
  // ================================================================

  // 擬似ハッシュ生成。
  Hash GenPseudoHash() {
    std::mt19937 engine(SysClock::to_time_t(SysClock::now()));
    std::uniform_int_distribution<Hash> dist(0, -1ULL);
    return dist(engine);
  }

  // ビットボードを出力する。
  void PrintBitboard(Bitboard bitboard) {
    // 出力する文字列ストリーム。
    std::ostringstream osstream;

    // 上下のボーダー。
    std::string border(" +-----------------+");

    // 上のボーダーをストリームへ。
    osstream << border << std::endl;

    // ビットボードを出力。
    Bitboard bit = 0x1ULL << (8 * 7);  // 初期位置a8へシフト。
    char c = '8';  // ランクの文字。
    for (int i = 0; i < NUM_RANKS; i++) {
      osstream << c << "| ";
      for (int j = 0; j < NUM_FYLES; j++) {
        if (bitboard & bit) {
          osstream << "@ ";
        } else {
          osstream << ". ";
        }
        if (j < 7) bit <<= 1;
      }
      // 一つ下のランクへ。
      bit >>= (7 + 8);
      osstream << "|" << std::endl;
      c--;
    }

    // 下部分を書く。
    osstream << border << std::endl;
    osstream << "   a b c d e f g h" << std::endl;

    // 標準出力に出力。
    std::cout << osstream.str();
  }

  // 手を出力する。
  void PrintMove(Move move) {
    // ファイルとランクの文字の配列。
    constexpr char fyle_table[NUM_FYLES] = {
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
    };
    constexpr char rank_table[NUM_RANKS] = {
      '1', '2', '3', '4', '5', '6', '7', '8'
    };

    // ファイルとランク。
    Fyle fyle;
    Rank rank;

    // 動かす駒の位置を出力する。
    std::cout << "From: ";
    fyle = Util::GetFyle(move.from_);
    rank = Util::GetRank(move.from_);
    std::cout << fyle_table[fyle] << rank_table[rank] << std::endl;

    // 移動先の位置を出力する。
    std::cout << "To: ";
    fyle = Util::GetFyle(move.to_);
    rank = Util::GetRank(move.to_);
    std::cout << fyle_table[fyle] << rank_table[rank] << std::endl;

    // 取った駒の種類を出力する。
    std::cout << "Captured Piece: ";
    switch (move.captured_piece_) {
      case EMPTY:
        std::cout << "None";
        break;
      case PAWN:
        std::cout << "Pawn";
        break;
      case KNIGHT:
        std::cout << "Knight";
        break;
      case BISHOP:
        std::cout << "Bishop";
        break;
      case ROOK:
        std::cout << "Rook";
        break;
      case QUEEN:
        std::cout << "Queen";
        break;
      case KING:
        std::cout << "King";
        break;
      default:
        throw SayuriError("駒の種類が不正です。");
        break;
    }
    std::cout << std::endl;

    // 昇格する駒の種類を出力する。
    std::cout << "Promotion: ";
    switch (move.promotion_) {
      case EMPTY:
        std::cout << "None";
        break;
      case PAWN:
        std::cout << "Pawn";
        break;
      case KNIGHT:
        std::cout << "Knight";
        break;
      case BISHOP:
        std::cout << "Bishop";
        break;
      case ROOK:
        std::cout << "Rook";
        break;
      case QUEEN:
        std::cout << "Queen";
        break;
      case KING:
        std::cout << "King";
        break;
      default:
        throw SayuriError("駒の種類が不正です。");
        break;
    }
    std::cout << std::endl;

    // キャスリングを出力する。
    Castling castling = move.last_castling_rights_;
    std::cout << "Last Castling Rights: ";
    if (castling & WHITE_SHORT_CASTLING)
      std::cout << "K";
    if (castling & WHITE_LONG_CASTLING)
      std::cout << "Q";
    if (castling & BLACK_SHORT_CASTLING)
      std::cout << "k";
    if (castling & BLACK_LONG_CASTLING)
      std::cout << "q";
    std::cout << std::endl;

    // アンパッサンできるかどうかを出力する。
    bool can_en_passant = move.last_can_en_passant_;
    std::cout << "Last Can En Passant: ";
    if (can_en_passant) std::cout << "True";
    else std::cout << "False";
    std::cout << std::endl;

    // アンパッサンのターゲットを出力する。
    Square en_passant_square = move.last_en_passant_square_;
    fyle = Util::GetFyle(en_passant_square);
    rank = Util::GetRank(en_passant_square);
    std::cout << "Last En Passant Square: "
    << fyle_table[fyle] << rank_table[rank] << std::endl;

    // 手の種類を出力する。
    std::cout << "Move Type: ";
    switch (move.move_type_) {
      case NORMAL:
        std::cout << "Normal";
        break;
      case CASTLING:
        std::cout << "Castling";
        break;
      case EN_PASSANT:
        std::cout << "En Passant";
        break;
      case NULL_MOVE:
        std::cout << "Null Move";
        break;
      default:
        throw SayuriError("手の種類が不正です。");
        break;
    }
    std::cout << std::endl;
  }

  //駒の配置を出力する。
  void PrintPosition(const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]) {
    // 出力する文字列ストリーム。
    std::ostringstream osstream;

    // 上下のボーダー。
    std::string border(" +-----------------+");

    // 上のボーダーをストリームへ。
    osstream << border << std::endl;

    // 駒の配置を出力。
    Bitboard bit = 0x1ULL << (8 * 7);  // 初期位置a8へシフト。
    char c = '8';  // ランクの文字。
    for (int i = 0; i < NUM_RANKS; i++) {
      osstream << c << "| ";
      for (int j = 0; j < NUM_FYLES; j++) {
        if (position[WHITE][PAWN] & bit) {
          osstream << "P ";
        } else if (position[WHITE][KNIGHT] & bit) {
          osstream << "N ";
        } else if (position[WHITE][BISHOP] & bit) {
          osstream << "B ";
        } else if (position[WHITE][ROOK] & bit) {
          osstream << "R ";
        } else if (position[WHITE][QUEEN] & bit) {
          osstream << "Q ";
        } else if (position[WHITE][KING] & bit) {
          osstream << "K ";
        } else if (position[BLACK][PAWN] & bit) {
          osstream << "p ";
        } else if (position[BLACK][KNIGHT] & bit) {
          osstream << "n ";
        } else if (position[BLACK][BISHOP] & bit) {
          osstream << "b ";
        } else if (position[BLACK][ROOK] & bit) {
          osstream << "r ";
        } else if (position[BLACK][QUEEN] & bit) {
          osstream << "q ";
        } else if (position[BLACK][KING] & bit) {
          osstream << "k ";
        } else {
          osstream << ". ";
        }

        if (j < 7) bit <<= 1;
      }
      // 一つ下のランクへ。
      bit >>= (7 + 8);
      osstream << "|" << std::endl;
      c--;
    }

    // 下部分を書く。
    osstream << border << std::endl;
    osstream << "   a b c d e f g h" << std::endl;

    // 標準出力に出力。
    std::cout << osstream.str();
  }

  /**********************
   * ストップウォッチ。 *
   **********************/
  namespace {
    TimePoint start_point;
    TimePoint end_point;
  }
  // ストップウォッチをスタートする。
  void Start() {
    start_point = SysClock::now();
  }
  // ストップウォッチをストップする。
  void Stop() {
    end_point = SysClock::now();
  }
  // ストップウォッチで計測した秒数を得る。
  int GetTime() {
    return Chrono::duration_cast<Chrono::milliseconds>
    (end_point - start_point).count();
  }
}  // namespace Sayuri
