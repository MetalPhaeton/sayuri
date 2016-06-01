/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Hironori Ishibashi
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
 * @file analyser.cpp
 * @author Hironori Ishibashi
 * @brief 局面分析器。
 */

#include "analyser.h"

#include <iostream>
#include <sstream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>
#include <memory>
#include "common.h"
#include "board.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /** 無名名前空間。 */
  namespace {
    // 簡易チェスボード。
    struct SimpleBoard {
      // 各サイドの駒の配置のビットボード。 [サイド]
      Bitboard side_pieces_[NUM_SIDES];
      // 全駒の配置のビットボード。 [角度]
      Bitboard blocker_[NUM_ROTS];
    };

    // 簡易チェスボードにセットする。
    void SetSimpleBoard(SimpleBoard& board,
    const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]) {
      board.blocker_[R0] = 0;
      board.blocker_[R45] = 0;
      board.blocker_[R90] = 0;
      board.blocker_[R135] = 0;

      FOR_SIDES(side) {
        board.side_pieces_[side] = 0;

        FOR_PIECE_TYPES(piece_type) {
          if (side && piece_type) {
            for (Bitboard bb = position[side][piece_type]; bb;
            NEXT_BITBOARD(bb)) {
              Square square = Util::GetSquare(bb);
              board.side_pieces_[side] |= Util::SQUARE[square][R0];
              board.blocker_[R0] |= Util::SQUARE[square][R0];
              board.blocker_[R45] |= Util::SQUARE[square][R45];
              board.blocker_[R90] |= Util::SQUARE[square][R90];
              board.blocker_[R135] |= Util::SQUARE[square][R135];
            }
          }
        }
      }
    }

    // 駒の配置のベクトルを作る。
    std::vector<Square> GenPosVector(Bitboard bitboard) {
      std::vector<Square> ret(Util::CountBits(bitboard));
      for (int i = 0; bitboard; NEXT_BITBOARD(bitboard), ++i) {
        ret[i] = Util::GetSquare(bitboard);
      }
      return ret;
    }

    // 駒の数と配置を計算する。
    void CalNumAndPos(const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES],
    ResultPositionAnalysis& result) {
      // 初期化。
      result.num_all_pieces_ = 0;
      result.num_side_pieces_[NO_SIDE] = 0;
      result.num_side_pieces_[WHITE] = 0;
      result.num_side_pieces_[BLACK] = 0;

      // 計算。
      FOR_SIDES(side) {
        FOR_PIECE_TYPES(piece_type) {
          if (side && piece_type) {
            // 実在する駒の時。
            int num = Util::CountBits(position[side][piece_type]);

            // 数を足す。
            result.num_all_pieces_ += num;
            result.num_side_pieces_[side] += num;
            result.num_each_pieces_[side][piece_type] = num;

            // 駒の配置を追加する。
            std::vector<Square> vec = GenPosVector(position[side][piece_type]);

            result.pos_all_pieces_.insert
            (result.pos_all_pieces_.end(), vec.begin(), vec.end());

            result.pos_side_pieces_[side].insert
            (result.pos_side_pieces_[side].end(), vec.begin(), vec.end());

            result.pos_each_pieces_[side][piece_type].insert
            (result.pos_each_pieces_[side][piece_type].end(),
            vec.begin(), vec.end());
          } else {
            result.num_each_pieces_[side][piece_type] = 0;
          }
        }
      }
    }
    // ボードの状態からビショップの利き筋を作る。
    Bitboard GetBishopAttack(const SimpleBoard& board, Square square) {
      return Util::GetBishopMagic(square, board.blocker_[R45],
      board.blocker_[R135]);
    }
    // ボードの状態からルークの利き筋を作る。
    Bitboard GetRookAttack(const SimpleBoard& board, Square square) {
      return Util::GetRookMagic(square, board.blocker_[R0],
      board.blocker_[R90]);
    }
//    // ボードの状態からクイーンの利き筋を作る。
//    Bitboard GetQueenAttack(const SimpleBoard& board, Square square) {
//      return Util::GetQueenMagic
//      (square, board.blocker_[R0], board.blocker_[R45],
//      board.blocker_[R90], board.blocker_[R135]);
//    }
//    // ボードの状態からポーンの動ける位置を作る。
//    Bitboard GetPawnStep(const SimpleBoard& board, Side side, Square square) {
//      return Util::GetPawnMovable(side, square, board.blocker_[R90]);
//    }

    // その位置が他の位置の駒に攻撃されているかどうかチェックする。
    // 攻撃している駒のビットボードを返す。
    Bitboard IsAttacked
    (const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES],
    const SimpleBoard& board, Square square, Side side) {
      Bitboard attacker = 0;

      // ポーンに攻撃されているかどうか調べる。
      attacker |= Util::PAWN_ATTACK[Util::GetOppositeSide(side)][square]
      & position[side][PAWN];

      // ナイトに攻撃されているかどうか調べる。
      attacker |= Util::KNIGHT_MOVE[square] & position[side][KNIGHT];

      // ビショップとクイーンの斜めに攻撃されているかどうか調べる。
      attacker |= GetBishopAttack(board, square) & (position[side][BISHOP]
      | position[side][QUEEN]);

      // ルークとクイーンの縦横に攻撃されているかどうか調べる。
      attacker |= GetRookAttack(board, square) & (position[side][ROOK]
      | position[side][QUEEN]);

      // キングに攻撃されているかどうか調べる。
      attacker |= Util::KING_MOVE[square] & position[side][KING];

      return attacker;
    }

    // チェックしている駒と数を計算する。
    void CalCheckers(const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES],
    const SimpleBoard& board, ResultPositionAnalysis& result) {
      result.num_checking_pieces_[NO_SIDE] = 0;

      // 白の相手をチェックしている駒。
      Bitboard attacker = 0;
      for (Bitboard bb = position[BLACK][KING]; bb; NEXT_BITBOARD(bb)) {
        attacker |= IsAttacked(position, board, Util::GetSquare(bb), WHITE);
      }
      result.num_checking_pieces_[WHITE] = Util::CountBits(attacker);
      std::vector<Square> temp = GenPosVector(attacker);
      result.pos_checking_pieces_[WHITE].insert
      (result.pos_checking_pieces_[WHITE].end(), temp.begin(), temp.end());

      // 黒の相手をチェックしている駒。
      attacker = 0;
      for (Bitboard bb = position[WHITE][KING]; bb; NEXT_BITBOARD(bb)) {
        attacker |= IsAttacked(position, board, Util::GetSquare(bb), BLACK);
      }
      result.num_checking_pieces_[BLACK] = Util::CountBits(attacker);
      temp = GenPosVector(attacker);
      result.pos_checking_pieces_[BLACK].insert
      (result.pos_checking_pieces_[BLACK].end(), temp.begin(), temp.end());
    }

//    void PrintBitboard(Bitboard bitboard) {
//      // 出力する文字列ストリーム。
//      std::ostringstream osstream;
//
//      // 上下のボーダー。
//      std::string border(" +-----------------+");
//
//      // 上のボーダーをストリームへ。
//      osstream << border << std::endl;
//
//      // ビットボードを出力。
//      Bitboard bit = 0x1ULL << (8 * 7);  // 初期位置a8へシフト。
//      char c = '8';  // ランクの文字。
//      FOR_RANKS(rank) {
//        osstream << c << "| ";
//        FOR_FYLES(fyle) {
//          if (bitboard & bit) {
//            osstream << "@ ";
//          } else {
//            osstream << ". ";
//          }
//          if (fyle < 7) bit <<= 1;
//        }
//        // 一つ下のランクへ。
//        bit >>= (7 + 8);
//        osstream << "|" << std::endl;
//        --c;
//      }
//
//      // 下部分を書く。
//      osstream << border << std::endl;
//      osstream << "   a b c d e f g h" << std::endl;
//
//      // 標準出力に出力。
//      std::cout << osstream.str();
//    }
  }

  // 配置の結果を返す。
  ResultPositionAnalysisPtr AnalysePosition
  (const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]) {
    ResultPositionAnalysisPtr ret_ptr(new ResultPositionAnalysis());

    // 簡易ボードを作成。
    SimpleBoard board;
    SetSimpleBoard(board, position);

    // 数と配置を計算する。
    CalNumAndPos(position, *ret_ptr);

    // 相手のキングをチェックしている駒の数と配置を計算する。
    CalCheckers(position, board, *ret_ptr);

    return ret_ptr;
  }
}  // namespace Sayuri
