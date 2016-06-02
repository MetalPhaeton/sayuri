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

#include "analyse.h"

#include <iostream>
#include <sstream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <tuple>
#include <memory>
#include "common.h"
#include "board.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /** 無名名前空間。 */
  namespace {
    // 駒の初期位置のビットボード。 [サイド][駒の種類]
    constexpr Bitboard START_POSITION[NUM_SIDES][NUM_PIECE_TYPES] {
      {0, 0, 0, 0, 0, 0, 0},
      {
        0, Util::RANK[RANK_2],
        Util::SQUARE[B1][R0] | Util::SQUARE[G1][R0],
        Util::SQUARE[C1][R0] | Util::SQUARE[F1][R0],
        Util::SQUARE[A1][R0] | Util::SQUARE[H1][R0],
        Util::SQUARE[D1][R0], Util::SQUARE[E1][R0]
      },
      {
        0, Util::RANK[RANK_7],
        Util::SQUARE[B8][R0] | Util::SQUARE[G8][R0],
        Util::SQUARE[C8][R0] | Util::SQUARE[F8][R0],
        Util::SQUARE[A8][R0] | Util::SQUARE[H8][R0],
        Util::SQUARE[D8][R0], Util::SQUARE[E8][R0]
      }
    };
    constexpr Bitboard NOT_START_POSITION[NUM_SIDES][NUM_PIECE_TYPES] {
      {0, 0, 0, 0, 0, 0, 0},
      {
        0, ~START_POSITION[WHITE][PAWN],
        ~START_POSITION[WHITE][KNIGHT],
        ~START_POSITION[WHITE][BISHOP],
        ~START_POSITION[WHITE][ROOK],
        ~START_POSITION[WHITE][QUEEN],
        ~START_POSITION[WHITE][KING]
      },
      {
        0, ~START_POSITION[BLACK][PAWN],
        ~START_POSITION[BLACK][KNIGHT],
        ~START_POSITION[BLACK][BISHOP],
        ~START_POSITION[BLACK][ROOK],
        ~START_POSITION[BLACK][QUEEN],
        ~START_POSITION[BLACK][KING]
      }
    };

    // 駒の配置のベクトルを作る。
    ResultSquares BBToResult(Bitboard bitboard) {
      ResultSquares ret(Util::CountBits(bitboard));
      for (int i = 0; bitboard; NEXT_BITBOARD(bitboard), ++i) {
        ret[i] = Util::GetSquare(bitboard);
      }
      return ret;
    }

    // ボードの状態からビショップの利き筋を作る。
    Bitboard GetBishopAttack(const Board& board, Square square) {
      return Util::GetBishopMagic(square, board.blocker_[R45],
      board.blocker_[R135]);
    }
    // ボードの状態からルークの利き筋を作る。
    Bitboard GetRookAttack(const Board& board, Square square) {
      return Util::GetRookMagic(square, board.blocker_[R0],
      board.blocker_[R90]);
    }
    // ボードの状態からクイーンの利き筋を作る。
    Bitboard GetQueenAttack(const Board& board, Square square) {
      return Util::GetQueenMagic
      (square, board.blocker_[R0], board.blocker_[R45],
      board.blocker_[R90], board.blocker_[R135]);
    }
    // ボードの状態からポーンの動ける位置を作る。
    Bitboard GetPawnStep(const Board& board, Side side, Square square) {
      return Util::GetPawnMovable(side, square, board.blocker_[R90]);
    }

    // その位置が他の位置の駒に攻撃されているかどうかチェックする。
    // 攻撃している駒のビットボードを返す。
    Bitboard IsAttacked(const Board& board, Square square, Side side) {
      Bitboard attacker = 0;

      // ポーンに攻撃されているかどうか調べる。
      attacker |= Util::PAWN_ATTACK[Util::GetOppositeSide(side)][square]
      & board.position_[side][PAWN];

      // ナイトに攻撃されているかどうか調べる。
      attacker |= Util::KNIGHT_MOVE[square] & board.position_[side][KNIGHT];

      // ビショップとクイーンの斜めに攻撃されているかどうか調べる。
      attacker |= GetBishopAttack(board, square)
      & (board.position_[side][BISHOP] | board.position_[side][QUEEN]);

      // ルークとクイーンの縦横に攻撃されているかどうか調べる。
      attacker |= GetRookAttack(board, square)
      & (board.position_[side][ROOK] | board.position_[side][QUEEN]);

      // キングに攻撃されているかどうか調べる。
      attacker |= Util::KING_MOVE[square] & board.position_[side][KING];

      return attacker;
    }

    // 白のショートキャスリングが出来るかどうか判定する。
    bool CanWhiteShortCastling(const Board& board) {
      return (board.castling_rights_ & WHITE_SHORT_CASTLING)
      && !((board.blocker_[R0]
      & (Util::SQUARE[F1][R0]
      | Util::SQUARE[G1][R0]))
      || IsAttacked(board, E1, BLACK)
      || IsAttacked(board, F1, BLACK)
      || IsAttacked(board, G1, BLACK));
    }

    /**
     * 白のロングキャスリングが出来るかどうか判定する。
     * @return キャスリング可能ならtrue。
     */
    bool CanWhiteLongCastling(const Board& board) {
      return (board.castling_rights_ & WHITE_LONG_CASTLING)
      && !((board.blocker_[R0]
      & (Util::SQUARE[D1][R0]
      | Util::SQUARE[C1][R0]
      | Util::SQUARE[B1][R0]))
      || IsAttacked(board, E1, BLACK)
      || IsAttacked(board, D1, BLACK)
      || IsAttacked(board, C1, BLACK));
    }

    /**
     * 黒のショートキャスリングが出来るかどうか判定する。
     * @return キャスリング可能ならtrue。
     */
    bool CanBlackShortCastling(const Board& board) {
      return (board.castling_rights_ & BLACK_SHORT_CASTLING)
      && !((board.blocker_[R0]
      & (Util::SQUARE[F8][R0]
      | Util::SQUARE[G8][R0]))
      || IsAttacked(board, E8, WHITE)
      || IsAttacked(board, F8, WHITE)
      || IsAttacked(board, G8, WHITE));
    }

    /**
     * 黒のロングキャスリングが出来るかどうか判定する。
     * @return キャスリング可能ならtrue。
     */
    bool CanBlackLongCastling(const Board& board) {
      return (board.castling_rights_ & BLACK_LONG_CASTLING)
      && !((board.blocker_[R0]
      & (Util::SQUARE[D8][R0]
      | Util::SQUARE[C8][R0]
      | Util::SQUARE[B8][R0]))
      || IsAttacked(board, E8, WHITE)
      || IsAttacked(board, D8, WHITE)
      || IsAttacked(board, C8, WHITE));
    }
//
//    // チェックしている駒と数を計算する。
//    void CalCheckers(const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES],
//    const Board& board, ResultPositionAnalysis& result) {
//      result.num_checking_pieces_[NO_SIDE] = 0;
//
//      // 白の相手をチェックしている駒。
//      Bitboard attacker = 0;
//      for (Bitboard bb = position[BLACK][KING]; bb; NEXT_BITBOARD(bb)) {
//        attacker |= IsAttacked(position, board, Util::GetSquare(bb), WHITE);
//      }
//      result.num_checking_pieces_[WHITE] = Util::CountBits(attacker);
//      std::vector<Square> temp = BBToResult(attacker);
//      result.pos_checking_pieces_[WHITE].insert
//      (result.pos_checking_pieces_[WHITE].end(), temp.begin(), temp.end());
//
//      // 黒の相手をチェックしている駒。
//      attacker = 0;
//      for (Bitboard bb = position[WHITE][KING]; bb; NEXT_BITBOARD(bb)) {
//        attacker |= IsAttacked(position, board, Util::GetSquare(bb), BLACK);
//      }
//      result.num_checking_pieces_[BLACK] = Util::CountBits(attacker);
//      temp = BBToResult(attacker);
//      result.pos_checking_pieces_[BLACK].insert
//      (result.pos_checking_pieces_[BLACK].end(), temp.begin(), temp.end());
//    }

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

  // 駒の違い。
  int AnalyseDiff(const Board& board, PieceType piece_type) {
    if (piece_type) {
      return Util::CountBits(board.position_[WHITE][piece_type])
      - Util::CountBits(board.position_[BLACK][piece_type]);
    }

    return 0;
  }

  // 機動力。
  ResultSquares AnalyseMobility(const Board& board, Square piece_square) {
    ResultSquares ret;

    // 駒の種類を得る。
    Side piece_side = board.side_board_[piece_square];
    PieceType piece_type = board.piece_board_[piece_square];

    // 駒がなければ帰る。
    if (!piece_type) return ret;

    Bitboard mobility;
    switch (piece_type) {
      case PAWN:
        {
          // 通常の動き。
          mobility = GetPawnStep(board, piece_side, piece_square);

          // 攻撃。
          mobility |= Util::PAWN_ATTACK[piece_side][piece_square]
          & board.side_pieces_[Util::GetOppositeSide(piece_side)];

          // アンパッサン。
          Rank rank = Util::SquareToRank(board.en_passant_square_);
          if (((piece_side == WHITE) && (rank == RANK_6))
          || ((piece_side == BLACK) && (rank == RANK_3))) {
            mobility |= Util::PAWN_ATTACK[piece_side][piece_square]
            & Util::SQUARE[piece_square][R0];
          }
        }
        break;
      case KNIGHT:
        mobility = Util::KNIGHT_MOVE[piece_square]
        & ~(board.side_pieces_[piece_side]);
        break;
      case BISHOP:
        mobility = GetBishopAttack(board, piece_square)
        & ~(board.side_pieces_[piece_side]);
        break;
      case ROOK:
        mobility = GetRookAttack(board, piece_square)
        & ~(board.side_pieces_[piece_side]);
        break;
      case QUEEN:
        mobility = GetQueenAttack(board, piece_square)
        & ~(board.side_pieces_[piece_side]);
        break;
      case KING:
        {
          // 通常の動き。
          mobility = Util::KING_MOVE[piece_square]
          & ~(board.side_pieces_[piece_side]);

          // 相手に攻撃されている場所を除く。
          Side enemy_side = Util::GetOppositeSide(piece_side);
          for (Bitboard bb = mobility; bb; NEXT_BITBOARD(bb)) {
            Square square = Util::GetSquare(bb);
            if (IsAttacked(board, square, enemy_side)) {
              mobility &= ~Util::SQUARE[square][R0];
            }
          }

          // キャスリング。
          if (piece_side == WHITE) {
            if (CanWhiteShortCastling(board)) {
              mobility |= Util::SQUARE[G1][R0];
            }
            if (CanWhiteLongCastling(board)) {
              mobility |= Util::SQUARE[C1][R0];
            }
          } else {
            if (CanBlackShortCastling(board)) {
              mobility |= Util::SQUARE[G8][R0];
            }
            if (CanBlackLongCastling(board)) {
              mobility |= Util::SQUARE[C8][R0];
            }
          }
        }
        break;
    }

    return BBToResult(mobility);
  }

  // 駒の展開を分析する。
  ResultSquares AnalyseDevelopment(const Board& board, Side piece_side,
  PieceType piece_type) {
    return BBToResult(board.position_[piece_side][piece_type]
    & NOT_START_POSITION[piece_side][piece_type]) ;
  }
}  // namespace Sayuri
