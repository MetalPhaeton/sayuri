/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Hironori Ishibashi
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
#include <array>
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

    Bitboard GenAttackBB(const Board& board, Square square) {
      Bitboard ret;

      // 駒の種類を得る。
      Side piece_side = board.side_board_[square];
      PieceType piece_type = board.piece_board_[square];

      // 駒がなければ帰る。
      if (!piece_type) return 0;

      switch (piece_type) {
        case PAWN:
          ret = Util::PAWN_ATTACK[piece_side][square];
          break;
        case KNIGHT:
          ret = Util::KNIGHT_MOVE[square];
          break;
        case BISHOP:
          ret = GetBishopAttack(board, square);
          break;
        case ROOK:
          ret = GetRookAttack(board, square);
          break;
        case QUEEN:
          ret = GetQueenAttack(board, square);
          break;
        case KING:
          ret = Util::KING_MOVE[square];
          break;
      }

      return ret;
    }

    Bitboard GenMobilityBB(const Board& board, Square square) {
      // 駒の種類を得る。
      Side piece_side = board.side_board_[square];
      PieceType piece_type = board.piece_board_[square];

      // 駒がなければ帰る。
      if (!piece_type) return 0;

      // とりあえず、攻撃のビットボードを得る。
      Bitboard ret = GenAttackBB(board, square);


      switch (piece_type) {
        case PAWN:
          {
            // 攻撃を限定する。
            ret &= board.side_pieces_[Util::GetOppositeSide(piece_side)];

            // アンパッサン。
            Rank rank = Util::SquareToRank(board.en_passant_square_);
            if (((piece_side == WHITE) && (rank == RANK_6))
            || ((piece_side == BLACK) && (rank == RANK_3))) {
              ret |= Util::PAWN_ATTACK[piece_side][square]
              & Util::SQUARE[square][R0];
            }

            // 通常の動き。
            ret |= GetPawnStep(board, piece_side, square);
          }
          break;
        case KING:
          {
            // 攻撃を限定。
            ret &= ~(board.side_pieces_[piece_side]);

            // 相手に攻撃されている場所を除く。
            Side enemy_side = Util::GetOppositeSide(piece_side);
            for (Bitboard bb = ret; bb; NEXT_BITBOARD(bb)) {
              Square square = Util::GetSquare(bb);
              if (IsAttacked(board, square, enemy_side)) {
                ret &= ~Util::SQUARE[square][R0];
              }
            }

            // キャスリング。
            if (piece_side == WHITE) {
              if (CanWhiteShortCastling(board)) {
                ret |= Util::SQUARE[G1][R0];
              }
              if (CanWhiteLongCastling(board)) {
                ret |= Util::SQUARE[C1][R0];
              }
            } else {
              if (CanBlackShortCastling(board)) {
                ret |= Util::SQUARE[G8][R0];
              }
              if (CanBlackLongCastling(board)) {
                ret |= Util::SQUARE[C8][R0];
              }
            }
          }
          break;
        default:
          ret &= ~(board.side_pieces_[piece_side]);
          break;
      }

      return ret;
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
    return BBToResult(GenMobilityBB(board, piece_square));
  }

  // 攻撃者。
  ResultSquares AnalyseAttackers(const Board& board, Square square) {
    Bitboard attackers = IsAttacked(board, square, WHITE)
    | IsAttacked(board, square, BLACK);
    return BBToResult(attackers);
  }

  // どの駒を攻撃しているか。
  ResultSquares AnalyseAttacking(const Board& board, Square piece_square) {
    return BBToResult(GenAttackBB(board, piece_square)
    & board.side_pieces_
    [Util::GetOppositeSide(board.side_board_[piece_square])]);
  }

  // どの駒に攻撃されているか。
  ResultSquares AnalyseAttacked(const Board& board, Square piece_square) {
    return BBToResult(IsAttacked(board, piece_square,
    Util::GetOppositeSide(board.side_board_[piece_square])));
  }

  // どの駒を防御しているか。
  ResultSquares AnalyseDefensing(const Board& board, Square piece_square) {
    return BBToResult(GenAttackBB(board, piece_square)
    & board.side_pieces_[board.side_board_[piece_square]]);
  }

  // どの駒に防御されているか。
  ResultSquares AnalyseDefensed(const Board& board, Square piece_square) {
    return BBToResult(IsAttacked(board, piece_square,
    board.side_board_[piece_square]));
  }

  // センターのコントロール。
  ResultSquares AnalyseCenterControl(const Board& board, Square piece_square) {
    static const Bitboard CENTER_BB =
    Util::SQUARE[C3][R0] | Util::SQUARE[C4][R0]
    | Util::SQUARE[C5][R0] | Util::SQUARE[C6][R0]
    | Util::SQUARE[D3][R0] | Util::SQUARE[D4][R0]
    | Util::SQUARE[D5][R0] | Util::SQUARE[D6][R0]
    | Util::SQUARE[E3][R0] | Util::SQUARE[E4][R0]
    | Util::SQUARE[E5][R0] | Util::SQUARE[E6][R0]
    | Util::SQUARE[F3][R0] | Util::SQUARE[F4][R0]
    | Util::SQUARE[F5][R0] | Util::SQUARE[F6][R0];

    return BBToResult(GenAttackBB(board, piece_square) & CENTER_BB);
  }

  // スウィートセンターのコントロール。
  ResultSquares AnalyseSweetCenterControl(const Board& board,
  Square piece_square) {
    static const Bitboard CENTER_BB = 
    Util::SQUARE[D4][R0] | Util::SQUARE[D5][R0]
    | Util::SQUARE[E4][R0] | Util::SQUARE[E5][R0];

    return BBToResult(GenAttackBB(board, piece_square) & CENTER_BB);
  }

  // オープンファイル。
  ResultFyles AnalyseOpenFyle(const Board& board) {
    ResultFyles ret;

    Bitboard pawns = board.position_[WHITE][PAWN]
    | board.position_[BLACK][PAWN];

    FOR_FYLES(fyle) {
      if (!(Util::FYLE[fyle] & pawns)) {
        ret.push_back(fyle);
      }
    }

    return ret;
  }

  // 駒の展開を分析する。
  ResultSquares AnalyseDevelopment(const Board& board, Side piece_side,
  PieceType piece_type) {
    return BBToResult(board.position_[piece_side][piece_type]
    & NOT_START_POSITION[piece_side][piece_type]) ;
  }

  // ダブルポーン。
  ResultSquares AnalyseDoublePawn(const Board& board, Side side) {
    Bitboard pawns = board.position_[side][PAWN];

    Bitboard result = 0;
    FOR_FYLES(fyle) {
      Bitboard temp = pawns & Util::FYLE[fyle];
      if (Util::CountBits(temp) >= 2) {
        result |= temp;
      }
    }

    return BBToResult(result);
  }

  // 孤立ポーン。
  ResultSquares AnalyseIsoPawn(const Board& board, Side side) {
    Bitboard pawns = board.position_[side][PAWN];

    Bitboard result = 0;
    FOR_FYLES(fyle) {
      Bitboard temp = 0;
      if (fyle == FYLE_A) {
        temp = pawns & Util::FYLE[FYLE_B];
      } else if (fyle == FYLE_H) {
        temp = pawns & Util::FYLE[FYLE_G];
      } else {
        temp = pawns & (Util::FYLE[fyle - 1] | Util::FYLE[fyle + 1]);
      }

      if (!Util::CountBits(temp)) {
        result |= pawns & Util::FYLE[fyle];
      }
    }

    return BBToResult(result);
  }

  // パスポーン。
  ResultSquares AnalysePassPawn(const Board& board, Side side) {
    static const Bitboard FRONT_RANKS[NUM_SIDES][NUM_RANKS] {
      {0, 0, 0, 0, 0, 0, 0, 0},
      {
        Util::RANK[RANK_8] | Util::RANK[RANK_7] | Util::RANK[RANK_6]
        | Util::RANK[RANK_5] | Util::RANK[RANK_4] |Util::RANK[RANK_3]
        | Util::RANK[RANK_2],

        Util::RANK[RANK_8] | Util::RANK[RANK_7] | Util::RANK[RANK_6]
        | Util::RANK[RANK_5] | Util::RANK[RANK_4] | Util::RANK[RANK_3],

        Util::RANK[RANK_8] | Util::RANK[RANK_7] | Util::RANK[RANK_6]
        | Util::RANK[RANK_5] | Util::RANK[RANK_4],

        Util::RANK[RANK_8] | Util::RANK[RANK_7] | Util::RANK[RANK_6]
        | Util::RANK[RANK_5],

        Util::RANK[RANK_8] | Util::RANK[RANK_7] | Util::RANK[RANK_6],

        Util::RANK[RANK_8] | Util::RANK[RANK_7],

        Util::RANK[RANK_8],

        0
      },
      {
        0,

        Util::RANK[RANK_1],

        Util::RANK[RANK_1] | Util::RANK[RANK_2],

        Util::RANK[RANK_1] | Util::RANK[RANK_2] | Util::RANK[RANK_3],

        Util::RANK[RANK_1] | Util::RANK[RANK_2] | Util::RANK[RANK_3]
        | Util::RANK[RANK_4],

        Util::RANK[RANK_1] | Util::RANK[RANK_2] | Util::RANK[RANK_3]
        | Util::RANK[RANK_4] | Util::RANK[RANK_5],

        Util::RANK[RANK_1] | Util::RANK[RANK_2] | Util::RANK[RANK_3]
        | Util::RANK[RANK_4] | Util::RANK[RANK_5] | Util::RANK[RANK_6],

        Util::RANK[RANK_1] | Util::RANK[RANK_2] | Util::RANK[RANK_3]
        | Util::RANK[RANK_4] | Util::RANK[RANK_5] | Util::RANK[RANK_6]
        | Util::RANK[RANK_7]
      }
    };

    Side enemy_side = Util::GetOppositeSide(side);
    Bitboard pawns = board.position_[side][PAWN];
    Bitboard enemy_pawns = board.position_[enemy_side][PAWN];

    Bitboard result = 0;
    for (; pawns; NEXT_BITBOARD(pawns)) {
      Square square = Util::GetSquare(pawns);
      Fyle fyle = Util::SquareToFyle(square);
      Rank rank = Util::SquareToRank(square);
      Bitboard temp = Util::FYLE[fyle];

      if (fyle == FYLE_A) {
        temp |= Util::FYLE[FYLE_B];
      } else if (fyle == FYLE_H) {
        temp |= Util::FYLE[FYLE_G];
      } else {
        temp |= Util::FYLE[fyle - 1] | Util::FYLE[fyle + 1];
      }

      temp &= FRONT_RANKS[side][rank];

      if (!(enemy_pawns & temp)) {
        result |= Util::SQUARE[square][R0];
      }
    }

    return BBToResult(result);
  }
}  // namespace Sayuri
