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
 * @file chess_util.cpp
 * @author Hironori Ishibashi
 * @brief Sayuri用便利ツールの実装。
 */

#include "chess_util.h"

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <random>
#include <climits>
#include <set>
#include <map>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /** 無名名前空間。 */
  namespace {
    /** 
     * 立っているビットの数を数える。
     * @param bits 対象のビット。
     * @return 立っているビットの数。
     */
    std::uint8_t GetNumBits(unsigned int bits) {
      std::uint8_t num = 0;
      for (; bits; bits &= bits - 1) {
        ++num;
      }
      return num;
    }
  }

  // ========== //
  // static定数 //
  // ========== //
  constexpr Square Util::ROT45[NUM_SQUARES];
  constexpr Square Util::ROT90[NUM_SQUARES];
  constexpr Square Util::ROT135[NUM_SQUARES];
  constexpr Square Util::R_ROT45[NUM_SQUARES];
  constexpr Square Util::R_ROT90[NUM_SQUARES];
  constexpr Square Util::R_ROT135[NUM_SQUARES];
  constexpr Square Util::FLIP[NUM_SQUARES];
  constexpr Square Util::EN_PASSANT_TRANS_TABLE[NUM_SQUARES];
  constexpr Bitboard Util::SQUARE0[NUM_SQUARES];
  constexpr Bitboard Util::SQUARE[NUM_SQUARES][NUM_ROTS];
  constexpr Bitboard Util::FYLE[NUM_FYLES];
  constexpr Bitboard Util::RANK[NUM_RANKS];
  constexpr Bitboard Util::SQCOLOR[NUM_SIDES];
  constexpr int Util::MAGIC_SHIFT_V[NUM_SQUARES];
  constexpr int Util::MAGIC_SHIFT_D[NUM_SQUARES];
  constexpr int Util::MAGIC_SHIFT[NUM_SQUARES][NUM_ROTS];
  constexpr Bitboard Util::MAGIC_MASK_V[NUM_SQUARES];
  constexpr Bitboard Util::MAGIC_MASK_D[NUM_SQUARES];
  constexpr Bitboard Util::MAGIC_MASK[NUM_SQUARES][NUM_ROTS];

  // ================== //
  // Utilクラスの初期化 //
  // ================== //
  // static変数の初期化。
  Bitboard Util::queen_move_[NUM_SQUARES];
  Bitboard Util::between_[NUM_SQUARES][NUM_SQUARES];
  void Util::InitUtil() {
    // num_bit16_table_[]を初期化する。
    InitNumBit16Table();
    // attack_table_[][][]を初期化する。
    InitAttackTable();
    // pawn_movable_table_[][][]を初期化する。
    InitPawnMovableTable();
    // line_[][]を初期化する。
    InitLine();
    // between_[][]を初期化する。
    FOR_SQUARES(square_1) {
      FOR_SQUARES(square_2) {
        between_[square_1][square_2] = line_[square_1][square_2]
        & ~(SQUARE[square_1][R0] | SQUARE[square_2][R0]);
      }
    }
    // pawn_move_[][]を初期化する。
    InitPawnMove();
    // pawn_2step_move_[][]を初期化する。
    InitPawn2StepMove();
    // pawn_attack_[][]を初期化する。
    InitPawnAttack();
    // knight_move_[]を初期化する。
    InitKnightMove();
    // bishop_move_[]を初期化する。
    InitBishopMove();
    // rook_move_[]を初期化する。
    InitRookMove();
    // queen_move_[]を初期化する。
    FOR_SQUARES(square) {
      queen_move_[square] = bishop_move_[square] | rook_move_[square];
    }
    // distance_table_[][]を初期化する。
    InitDistanceTable();
    // is_en_passant_table_[][]を初期化する。
    InitIsEnPassantTable();
    // is_2step_move_table_[][]を初期化する。
    InitIs2StepMoveTable();
    // king_move_[]を初期化する。
    InitKingMove();
    // ランダム関連を初期化する。
    InitRandom();
  }

  // ============================ //
  // ビットを数えるときに使うもの //
  // ============================ //
  // 下位16ビットのビットの数が入った配列。 [下位16ビット]
  std::uint8_t Util::num_bit16_table_[0xffff + 1];
  // num_bit16_table_[]を初期化する。
  void Util::InitNumBit16Table() {
    for (unsigned int index = 0; index <= 0xffff; ++index) {
      num_bit16_table_[index] = GetNumBits(index);
    }
  }

  // ======== //
  // マジック //
  // ======== //
  // マジックビットボードの配列。
  Bitboard Util::attack_table_[NUM_SQUARES][0xff + 1][NUM_ROTS];
  // attack_table_[][]を初期化する。
  void Util::InitAttackTable() {
    // 初期化。
    INIT_ARRAY(attack_table_);

    FOR_SQUARES(square) {
      for (Bitboard pattern = 0; pattern <= 0xff; ++pattern) {
        // 各角度のポイント。
        Bitboard point[NUM_ROTS] {
          (Util::SQUARE[square][R0] >> Util::MAGIC_SHIFT[square][R0])
          & Util::MAGIC_MASK[square][R0],
          (Util::SQUARE[square][R45] >> Util::MAGIC_SHIFT[square][R45])
          & Util::MAGIC_MASK[square][R45],
          (Util::SQUARE[square][R90] >> Util::MAGIC_SHIFT[square][R90])
          & Util::MAGIC_MASK[square][R90],
          (Util::SQUARE[square][R135] >> Util::MAGIC_SHIFT[square][R135])
          & Util::MAGIC_MASK[square][R135]
        };
        Bitboard temp[NUM_ROTS] {0, 0, 0, 0};

        // 先ず左。
        COPY_ARRAY(temp, point);
        for (int i = 0; i < 8; ++i) {
          // シフト。
          temp[R0] = (temp[R0] << 1) & Util::MAGIC_MASK[square][R0];
          temp[R45] = (temp[R45] << 1) & Util::MAGIC_MASK[square][R45];
          temp[R90] = (temp[R90] << 1) & Util::MAGIC_MASK[square][R90];
          temp[R135] = (temp[R135] << 1) & Util::MAGIC_MASK[square][R135];

          // 0度。
          if (temp[R0]) {
            attack_table_[square][pattern][R0] |=
            temp[R0] << Util::MAGIC_SHIFT[square][R0];

            // もし他の駒にぶつかったら終了。
            if (temp[R0] & pattern) temp[R0] = 0;
          }
          // 45度。
          if (temp[R45]) {
            attack_table_[square][pattern][R45] |=
            Util::SQUARE[Util::R_ROT45[Util::GetSquare(temp[R45]
            << Util::MAGIC_SHIFT[square][R45])]][R0];

            // もし他の駒にぶつかったら終了。
            if (temp[R45] & pattern) temp[R45] = 0;
          }
          // 90度。
          if (temp[R90]) {
            attack_table_[square][pattern][R90] |=
            Util::SQUARE[Util::R_ROT90[Util::GetSquare(temp[R90]
            << Util::MAGIC_SHIFT[square][R90])]][R0];

            // もし他の駒にぶつかったら終了。
            if (temp[R90] & pattern) temp[R90] = 0;
          }
          // 135度。
          if (temp[R135]) {
            attack_table_[square][pattern][R135] |=
            Util::SQUARE[Util::R_ROT135[Util::GetSquare(temp[R135]
            << Util::MAGIC_SHIFT[square][R135])]][R0];

            // もし他の駒にぶつかったら終了。
            if (temp[R135] & pattern) temp[R135] = 0;
          }
        }

        // 次は右。
        COPY_ARRAY(temp, point);
        for (int i = 0; i < 8; ++i) {
          // シフト。
          temp[R0] = (temp[R0] >> 1) & Util::MAGIC_MASK[square][R0];
          temp[R45] = (temp[R45] >> 1) & Util::MAGIC_MASK[square][R45];
          temp[R90] = (temp[R90] >> 1) & Util::MAGIC_MASK[square][R90];
          temp[R135] = (temp[R135] >> 1) & Util::MAGIC_MASK[square][R135];

          // 0度。
          if (temp[R0]) {
            attack_table_[square][pattern][R0] |=
            temp[R0] << Util::MAGIC_SHIFT[square][R0];

            // もし他の駒にぶつかったら終了。
            if (temp[R0] & pattern) temp[R0] = 0;
          }
          // 45度。
          if (temp[R45]) {
            attack_table_[square][pattern][R45] |=
            Util::SQUARE[Util::R_ROT45[Util::GetSquare(temp[R45]
            << Util::MAGIC_SHIFT[square][R45])]][R0];

            // もし他の駒にぶつかったら終了。
            if (temp[R45] & pattern) temp[R45] = 0;
          }
          // 90度。
          if (temp[R90]) {
            attack_table_[square][pattern][R90] |=
            Util::SQUARE[Util::R_ROT90[Util::GetSquare(temp[R90]
            << Util::MAGIC_SHIFT[square][R90])]][R0];

            // もし他の駒にぶつかったら終了。
            if (temp[R90] & pattern) temp[R90] = 0;
          }
          // 135度。
          if (temp[R135]) {
            attack_table_[square][pattern][R135] |=
            Util::SQUARE[Util::R_ROT135[Util::GetSquare(temp[R135]
            << Util::MAGIC_SHIFT[square][R135])]][R0];

            // もし他の駒にぶつかったら終了。
            if (temp[R135] & pattern) temp[R135] = 0;
          }
        }
      }
    }
  }

  // ポーンの動ける位置の配列。
  Bitboard Util::pawn_movable_table_[NUM_SIDES][NUM_SQUARES][0xff + 1];
  void Util::InitPawnMovableTable() {
    // 初期化。
    INIT_ARRAY(pawn_movable_table_);

    // ポーンの動ける位置の配列を作成。
    FOR_SIDES(side) {
      FOR_SQUARES(square) {
        for (Bitboard pattern = 0; pattern <= 0xff; ++pattern) {
          if (side == WHITE) {
            if (SquareToRank(square) == RANK_2) {
              // 初期配置のポーンの位置。
              Bitboard front_bit =
              SQUARE[square + 8][R90] >> MAGIC_SHIFT[square + 8][R90];
              if (!(front_bit & pattern)) {
                // 1歩目。
                pawn_movable_table_[side][square][pattern] =
                SQUARE[square + 8][R0];

                // 2歩目。
                front_bit = SQUARE[square + 16][R90] >>
                MAGIC_SHIFT[square + 16][R90];
                if (!(front_bit & pattern)) {
                  pawn_movable_table_[side][square][pattern] |=
                  SQUARE[square + 16][R0];
                }
              } else {
                pawn_movable_table_[side][square][pattern] = 0;
              }
            } else if (SquareToRank(square) == RANK_8) {
              // 最上ランク。
              pawn_movable_table_[side][square][pattern] = 0;
            } else {
              // その他のランク。
              Bitboard front_bit =
              SQUARE[square + 8][R90] >> MAGIC_SHIFT[square + 8][R90];
              if (!(front_bit & pattern)) {
                pawn_movable_table_[side][square][pattern] =
                SQUARE[square + 8][R0];
              } else {
                pawn_movable_table_[side][square][pattern] = 0;
              }
            }
          } else if (side == BLACK) {
            if (SquareToRank(square) == RANK_7) {
              // 初期配置のポーンの位置。
              Bitboard front_bit =
              SQUARE[square - 8][R90] >> MAGIC_SHIFT[square - 8][R90];
              if (!(front_bit & pattern)) {
                // 1歩目。
                pawn_movable_table_[side][square][pattern] =
                SQUARE[square - 8][R0];

                // 2歩目。
                front_bit =
                SQUARE[square - 16][R90] >> MAGIC_SHIFT[square - 16][R90];
                if (!(front_bit & pattern)) {
                  pawn_movable_table_[side][square][pattern] |=
                  SQUARE[square - 16][R0];
                }
              } else {
                pawn_movable_table_[side][square][pattern] = 0;
              }
            } else if (SquareToRank(square) == RANK_1) {
              // 最上ランク。
              pawn_movable_table_[side][square][pattern] = 0;
            } else {
              // その他のランク。
              Bitboard front_bit =
              SQUARE[square - 8][R90] >> MAGIC_SHIFT[square - 8][R90];
              if (!(front_bit & pattern)) {
                pawn_movable_table_[side][square][pattern] =
                SQUARE[square - 8][R0];
              } else {
                pawn_movable_table_[side][square][pattern] = 0;
              }
            }
          } else {
            pawn_movable_table_[side][square][pattern] = 0;
          }
        }
      }
    }
  }

  // ================== //
  // ビットボードの配列 //
  // ================== //
  // 直線の入った配列。 [端点][端点]
  Bitboard Util::line_[NUM_SQUARES][NUM_SQUARES];
  // ポーンの通常の動きの配列。 [サイド][マス]
  Bitboard Util::pawn_move_[NUM_SIDES][NUM_SQUARES];
  // ポーンの2歩の動きの配列。 [サイド][マス]
  Bitboard Util::pawn_2step_move_[NUM_SIDES][NUM_SQUARES];
  // ポーンの攻撃筋の配列。 [サイド][マス]
  Bitboard Util::pawn_attack_[NUM_SIDES][NUM_SQUARES];
  // ナイトの動きの配列。 [マス]
  Bitboard Util::knight_move_[NUM_SQUARES];
  // ビショップの動きの配列。 [マス]
  Bitboard Util::bishop_move_[NUM_SQUARES];
  // ルークの動きの配列。 [マス]
  Bitboard Util::rook_move_[NUM_SQUARES];
  // キングの動きの配列。 [マス]
  Bitboard Util::king_move_[NUM_SQUARES];
  // line_[][]を初期化する。
  void Util::InitLine() {
    // line_を作る。
    FOR_SQUARES(square_1) {
      FOR_SQUARES(square_2) {
        // 端点を入手する。
        Bitboard point_1 = SQUARE[square_1][R0];
        Bitboard point_2 = SQUARE[square_2][R0];

        // 右から調べていく。
        Bitboard temp = point_1;
        Bitboard between = 0;
        while ((temp = GetRightBitboard(temp))) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point_2) {
            line_[square_1][square_2] = point_1 | between;
            break;
          }
        }
        // 左から調べていく。
        temp = point_1;
        between = 0;
        while ((temp = GetLeftBitboard(temp))) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point_2) {
            line_[square_1][square_2] = point_1 | between;
            break;
          }
        }
        // 上から調べていく。
        temp = point_1;
        between = 0;
        while ((temp = GetUpBitboard(temp))) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point_2) {
            line_[square_1][square_2] = point_1 | between;
            break;
          }
        }
        // 下から調べていく。
        temp = point_1;
        between = 0;
        while ((temp = GetDownBitboard(temp))) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point_2) {
            line_[square_1][square_2] = point_1 | between;
            break;
          }
        }
        // 右上から調べていく。
        temp = point_1;
        between = 0;
        while ((temp = GetRightUpBitboard(temp))) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point_2) {
            line_[square_1][square_2] = point_1 | between;
            break;
          }
        }
        // 右下から調べていく。
        temp = point_1;
        between = 0;
        while ((temp = GetRightDownBitboard(temp))) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point_2) {
            line_[square_1][square_2] = point_1 | between;
            break;
          }
        }
        // 左上から調べていく。
        temp = point_1;
        between = 0;
        while ((temp = GetLeftUpBitboard(temp))) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point_2) {
            line_[square_1][square_2] = point_1 | between;
            break;
          }
        }
        // 左下から調べていく。
        temp = point_1;
        between = 0;
        while ((temp = GetLeftDownBitboard(temp))) {
          between |= temp;
          // 端点に達したらline_に直線を入れる。
          if (temp & point_2) {
            line_[square_1][square_2] = point_1 | between;
            break;
          }
        }
      }
    }
  }
  // pawn_move_[][]を初期化する。
  void Util::InitPawnMove() {
    // ポーンの動きを作る。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[square][R0];
      // どちらのサイドでもない。
      pawn_move_[NO_SIDE][square] = 0;
      // 白。
      pawn_move_[WHITE][square] = GetUpBitboard(point);
      // 黒。
      pawn_move_[BLACK][square] = GetDownBitboard(point);
    }
  }
  // pawn_2step_move_[][]を初期化する。
  void Util::InitPawn2StepMove() {
    // ポーンの2歩の動きを作る。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[square][R0];

      // とりあえず0で初期化する。
      pawn_2step_move_[NO_SIDE][square] = 0;
      pawn_2step_move_[WHITE][square] = 0;
      pawn_2step_move_[BLACK][square] = 0;

      // ランク2の時は白に2歩の動きを入れる。
      // ランク7の時は黒に2歩の動きを入れる。
      if (point & RANK[RANK_2]) {
        pawn_2step_move_[WHITE][square] =
        GetUpBitboard(GetUpBitboard(point));
      } else if (point & RANK[RANK_7]) {
        pawn_2step_move_[BLACK][square] =
        GetDownBitboard(GetDownBitboard(point));
      }
    }
  }
  // pawn_attack_[][]を初期化する。
  void Util::InitPawnAttack() {
    // 攻撃筋を入れる。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[square][R0];

      // どちらでもない。
      pawn_attack_[NO_SIDE][square] = 0;
      // 白。
      pawn_attack_[WHITE][square] =
      GetRightUpBitboard(point) | GetLeftUpBitboard(point);
      // 黒。
      pawn_attack_[BLACK][square] =
      GetRightDownBitboard(point) | GetLeftDownBitboard(point);
    }
  }
  // knight_move_[]を初期化する。
  void Util::InitKnightMove() {
    // 動きを入れる。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[square][R0];

      knight_move_[square] = GetRightRightUpBitboard(point)
      | GetRightUpUpBitboard(point)
      | GetRightRightDownBitboard(point)
      | GetRightDownDownBitboard(point)
      | GetLeftLeftUpBitboard(point)
      | GetLeftUpUpBitboard(point)
      | GetLeftLeftDownBitboard(point)
      | GetLeftDownDownBitboard(point);
    }
  }
  // bishop_move_[]を初期化する。
  void Util::InitBishopMove() {
    // 動きを入れる。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[square][R0];
      bishop_move_[square] = 0;

      // 右上の動きを入れる。
      Bitboard temp = point;
      while ((temp = GetRightUpBitboard(temp))) {
        bishop_move_[square] |= temp;
      }
      // 右下の動きを入れる。
      temp = point;
      while ((temp = GetRightDownBitboard(temp))) {
        bishop_move_[square] |= temp;
      }
      // 左上の動きを入れる。
      temp = point;
      while ((temp = GetLeftUpBitboard(temp))) {
        bishop_move_[square] |= temp;
      }
      // 左下の動きを入れる。
      temp = point;
      while ((temp = GetLeftDownBitboard(temp))) {
        bishop_move_[square] |= temp;
      }
    }
  }
  // rook_move_[]を初期化する。
  void Util::InitRookMove() {
    // 動きを入れる。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[square][R0];
      rook_move_[square] = 0;

      // 右の動きを入れる。
      Bitboard temp = point;
      while ((temp = GetRightBitboard(temp))) {
        rook_move_[square] |= temp;
      }
      // 左の動きを入れる。
      temp = point;
      while ((temp = GetLeftBitboard(temp))) {
        rook_move_[square] |= temp;
      }
      // 上の動きを入れる。
      temp = point;
      while ((temp = GetUpBitboard(temp))) {
        rook_move_[square] |= temp;
      }
      // 下の動きを入れる。
      temp = point;
      while ((temp = GetDownBitboard(temp))) {
        rook_move_[square] |= temp;
      }
    }
  }
  // king_move_[]を初期化する。
  void Util::InitKingMove() {
    // 動きを入れる。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[square][R0];

      // 動きを入れる。
      king_move_[square] = GetRightBitboard(point)
      | GetLeftBitboard(point)
      | GetUpBitboard(point)
      | GetDownBitboard(point)
      | GetRightUpBitboard(point)
      | GetRightDownBitboard(point)
      | GetLeftUpBitboard(point)
      | GetLeftDownBitboard(point);
    }
  }

  // ================ //
  // その他の便利関数 //
  // ================ //
  // 文字列を単語に切り分ける。
  template<class CharType>
  std::vector<std::basic_string<CharType>> Util::Split
  (const std::basic_string<CharType>& str,
  const std::set<CharType>& delim,
  const std::set<CharType>& delim_and_word) {
    std::vector<std::basic_string<CharType>> ret(0);

    std::basic_string<CharType> temp;
    for (auto c : str) {
      if (delim.find(c) != delim.end()) {
        if (temp.size() > 0) {
          ret.push_back(temp);
          temp.clear();
        }
      } else if (delim_and_word.find(c) != delim_and_word.end()) {
        if (temp.size() > 0) {
          ret.push_back(temp);
          temp.clear();
        }
        CharType cs[2] {c, 0};
        ret.push_back(cs);
      } else {
        temp.push_back(c);
      }
    }

    if (temp.size() > 0) {
      ret.push_back(temp);
      temp.clear();
    }

    return ret;
  }
  // インスタンス化。
  template std::vector<std::basic_string<char>> Util::Split<char>
  (const std::basic_string<char>& str,
  const std::set<char>& delim, const std::set<char>& delim_and_word);
  template std::vector<std::basic_string<wchar_t>> Util::Split<wchar_t>
  (const std::basic_string<wchar_t>& str,
  const std::set<wchar_t>& delim, const std::set<wchar_t>& delim_and_word);

  // マス間の距離の配列。
  int Util::distance_table_[NUM_SQUARES][NUM_SQUARES];
  void Util::InitDistanceTable() {
    FOR_SQUARES(square_1) {
      FOR_SQUARES(square_2) {
        Fyle fyle_1 = SquareToFyle(square_1);
        Rank rank_1 = SquareToRank(square_1);
        Fyle fyle_2 = SquareToFyle(square_2);
        Rank rank_2 = SquareToRank(square_2);

        int fyle_diff = fyle_1 >= fyle_2
        ? (fyle_1 - fyle_2) : (fyle_2 - fyle_1);
        int rank_diff = rank_1 >= rank_2
        ? (rank_1 - rank_2) : (rank_2 - rank_1);

        distance_table_[square_1][square_2] = GetMax(fyle_diff, rank_diff);
      }
    }
  }

  // アンパッサン判定配列。
  bool Util::is_en_passant_table_[NUM_SQUARES][NUM_SQUARES];
  void Util::InitIsEnPassantTable() {
    FOR_SQUARES(en_passant_square) {
      FOR_SQUARES(to) {
        is_en_passant_table_[en_passant_square][to] = false;
        if (en_passant_square == to) {
          if ((SquareToRank(en_passant_square) == RANK_3)
          || (SquareToRank(en_passant_square) == RANK_6)) {
            is_en_passant_table_[en_passant_square][to] = true;
          }
        }
      }
    }
  }

  // ポーンの2歩の動き判定テーブル。
  bool Util::is_2step_move_table_[NUM_SQUARES][NUM_SQUARES];
  void Util::InitIs2StepMoveTable() {
    FOR_SQUARES(from) {
      FOR_SQUARES(to) {
        is_2step_move_table_[from][to] = false;
        if (SquareToRank(from) == RANK_2) {
          if (to == (from + 16)) is_2step_move_table_[from][to] = true;
        } else if (SquareToRank(from) == RANK_7) {
          if (to == (from - 16)) is_2step_move_table_[from][to] = true;
        }
      }
    }
  }

  // --- ランダム関連 --- //
  std::mt19937 Util::engine_;
  std::uniform_int_distribution<Hash> Util::dist_;
  void Util::InitRandom() {
    // メルセンヌツイスターの準備。
    engine_ = std::mt19937(SysClock::to_time_t(SysClock::now()));
    dist_ = std::uniform_int_distribution<Hash>(0, ULLONG_MAX);
  }

  // FEN/EPDをパース。
  std::map<std::string, std::string>
  Util::ParseFEN(const std::string& str) {
    // 空白文字かどうか。
    auto is_blank = [](char c) -> bool {
      switch (c) {
        case ' ':
        case '\n':
        case '\r':
        case '\t':
        case '\f':
        case '\a':
        case '\b':
          return true;
      }
      return false;
    };

    // 数字かどうか。
    auto is_num = [](const std::string& str) -> bool {
      for (auto c : str) {
        if (!((c >= '0') && (c <= '9'))) return false;
      }
      return true;
    };

    std::map<std::string, std::string> ret;
    ret["fen position"] = std::string(64, '-');
    ret["fen to_move"] = "-";
    ret["fen castling"] = "----";
    ret["fen en_passant"] = "-";

    // トークンに分ける。
    std::vector<std::string> token_vec;
    std::ostringstream oss;
    for (auto c : str) {
      if (is_blank(c)) {
        if (!(oss.str().empty())) {
          token_vec.push_back(oss.str());
          oss.str("");
        }
      } else if (c == ';') {
        if (!(oss.str().empty())) {
          token_vec.push_back(oss.str());
          oss.str("");
        }
        token_vec.push_back(std::string(1, c));
      } else {
        oss << c;
      }
    }
    if (!(oss.str().empty())) {
      token_vec.push_back(oss.str());
      oss.str("");
    }

    // FEN部分とEPDの拡張部分に分ける。
    std::vector<std::string> fen_vec;
    std::vector<std::string> epd_vec;
    if (token_vec.size() >= 5) {
      std::vector<std::string>::iterator itr = token_vec.begin() + 4;
      std::copy(token_vec.begin(), itr, std::back_inserter(fen_vec));
      if (is_num(*itr)) {  // クロックのトークン。
        fen_vec.push_back(*itr);
        ++itr;
        if ((itr != token_vec.end()) && is_num(*itr)) {  // 手番のトークン。
          fen_vec.push_back(*itr);
          ++itr;
        }
      }
      std::copy(itr, token_vec.end(), std::back_inserter(epd_vec));
    } else {
      fen_vec = token_vec;
    }

    // FENのイテレータ。
    std::vector<std::string>::iterator fen_itr = fen_vec.begin();

    // 駒の配置をパース。
    if (fen_itr != fen_vec.end()) {
      //ランクを逆順にする。
      std::vector<std::string> temp;
      for (auto c : *fen_itr) {
        if (c == '/') {
          if (!(oss.str().empty())) {
            temp.push_back(oss.str());
            oss.str("");
          }
        } else {
          oss << c;
        }
      }
      if (!(oss.str().empty())) {
        temp.push_back(oss.str());
        oss.str("");
      }
      std::reverse(temp.begin(), temp.end());

      // パース。
      for (auto& rank_str : temp) {
        for (auto c : rank_str) {
          if ((c >= '1') && (c <= '8')) {
            std::string temp_str(c - '0', '-');
            oss << temp_str;
          } else {
            switch (c) {
              case 'P': case 'N': case 'B': case 'R': case 'Q': case 'K':
              case 'p': case 'n': case 'b': case 'r': case 'q': case 'k':
                oss << c;
                break;
            }
          }
        }
      }
      if (oss.str().size() == 64) {
        ret["fen position"] = oss.str();
      }
      oss.str("");
      ++fen_itr;
    }

    // サイドをパース。
    if (fen_itr != fen_vec.end()) {
      if ((*fen_itr == "w") || (*fen_itr == "b")) {
        ret["fen to_move"] = *fen_itr;
      }
      ++fen_itr;
    }

    // キャスリングの権利をパース。
    if (fen_itr != fen_vec.end()) {
      std::string temp_str = "----";
      for (auto c : *fen_itr) {
        switch (c) {
          case 'K': temp_str[0] = 'K'; break;
          case 'Q': temp_str[1] = 'Q'; break;
          case 'k': temp_str[2] = 'k'; break;
          case 'q': temp_str[3] = 'q'; break;
          case '-': break;
        }
      }
      ret["fen castling"] = temp_str;
      ++fen_itr;
    }

    // アンパッサンのマスをパース。
    if (fen_itr != fen_vec.end()) {
      if (fen_itr->size() >= 2) {
        char fyle_c = (*fen_itr)[0];
        char rank_c = (*fen_itr)[1];
        if (((fyle_c >= 'a') && (fyle_c <= 'h'))
        && ((rank_c == '3') || (rank_c == '6'))) {
          oss << fyle_c << rank_c;
          ret["fen en_passant"] = oss.str();
          oss.str("");
        }
      }
      ++fen_itr;
    }

    // クロックをパース。
    if (fen_itr != fen_vec.end()) {
      if (is_num(*fen_itr)) {
        ret["fen clock"] = *fen_itr;
      }
      ++fen_itr;
    }

    // 手数をパース。 プライで。
    if (fen_itr != fen_vec.end()) {
      if (is_num(*fen_itr)) {
        int move_count = std::stoll(*fen_itr);
        if (move_count > 0) {
          int ply = (move_count - 1) * 2;
          if (ret["fen to_move"] == "b") ply += 1;
          ret["fen ply"] = std::to_string(ply);
        }
      }
      ++fen_itr;
    }

    // --- 拡張部分 --- //
    std::vector<std::string>::iterator epd_itr = epd_vec.begin();
    // 最初の区切り文字を無視。
    if ((epd_itr != epd_vec.end()) && (*epd_itr == ";")) ++epd_itr;
    while (epd_itr != epd_vec.end()) {
      // オペコード。
      std::string opcode = *(epd_itr++);
      ret[opcode] = "";

      // オペランド。
      while (epd_itr != epd_vec.end()) {
        if (*epd_itr == ";") {
          ++epd_itr;
          break;
        }

        ret[opcode] = *(epd_itr++) + " ";
      }

      // 最後の空白を削除。
      if (!(ret[opcode].empty())) ret[opcode].pop_back();
    }

    return ret;
  }

  // FENの駒の配置の文字列に変換する。
  std::string Util::ToFENPosition
  (const Bitboard (& position)[NUM_SIDES][NUM_PIECE_TYPES]) {
    std::ostringstream oss;

    // 64文字の文字列にする。
    Bitboard pos = 1ULL;
    FOR_SQUARES(square) {
      if ((position[WHITE][PAWN] & pos)) oss << 'P';
      else if ((position[WHITE][KNIGHT] & pos)) oss << 'N';
      else if ((position[WHITE][BISHOP] & pos)) oss << 'B';
      else if ((position[WHITE][ROOK] & pos)) oss << 'R';
      else if ((position[WHITE][QUEEN] & pos)) oss << 'Q';
      else if ((position[WHITE][KING] & pos)) oss << 'K';
      else if ((position[BLACK][PAWN] & pos)) oss << 'p';
      else if ((position[BLACK][KNIGHT] & pos)) oss << 'n';
      else if ((position[BLACK][BISHOP] & pos)) oss << 'b';
      else if ((position[BLACK][ROOK] & pos)) oss << 'r';
      else if ((position[BLACK][QUEEN] & pos)) oss << 'q';
      else if ((position[BLACK][KING] & pos)) oss << 'k';
      else oss << '-';
      pos <<= 1;
    }

    // ランクに分ける。
    std::vector<std::string> temp_vec;
    for (Square i = 0; i < NUM_SQUARES; i += 8) {
      temp_vec.push_back(oss.str().substr(i, 8));
    }

    // 各ランクの空のマスの連なりを数字に変換する。
    // ついでに一続きにする。
    oss.str("");
    for (int i = 7; i >= 0; --i) {
      bool in_empty = false;
      int count = 0;
      for (auto c : temp_vec[i]) {
        if (in_empty) {
          if (c != '-') {
            oss << count << c;
            in_empty = false;
            count = 0;
          } else {
            ++count;
          }
        } else {
          if (c == '-') {
            count = 1;
            in_empty = true;
          } else {
            oss << c;
          }
        }
      }
      if (count) oss << count;
      oss << '/';
    }

    // 一番後ろのスラッシュを消して返す。
    std::string ret = oss.str();
    ret.pop_back();
    return ret;
  }

  // Algebraic Notationかどうかを判定。
  bool Util::IsAlgebraicNotation(const std::string& str) {
    // キャスリングかどうかを判定。
    if ((str == "O-O") || (str == "O-O+") || (str == "O-O#")
    || (str == "O-O-O") || (str == "O-O-O+") || (str == "O-O-O#")) {
      return true;
    }

    auto is_piece = [](char c) -> bool {
      return (c == 'N') || (c == 'B') || (c == 'R')
      || (c == 'Q') || (c == 'K');
    };
    auto is_fyle = [](char c) -> bool {return (c >= 'a') && (c <= 'h');};
    auto is_rank = [](char c) -> bool {return (c >= '1') && (c <= '8');};
    auto is_square =
    [&str, &is_fyle, &is_rank](std::string::const_iterator itr) -> bool {
      if (itr == str.end()) return false;
      if (!is_fyle(*itr)) return false;
      ++itr;
      if (itr == str.end()) return false;
      if (!is_rank(*itr)) return false;
      return true;
    };

    std::string::const_iterator itr = str.begin();

    // 最初の文字がファイル、または駒。
    if (!(is_piece(*itr) || is_fyle(*itr))) return false;
    if (is_piece(*itr)) ++itr;

    // ここで終わったら違う。
    if (itr == str.end()) return false;

    // fromとtoを判定。
    if (is_fyle(*itr)) {
      // 次はランクかxでなくてはらない。
      ++itr;
      if (itr == str.end()) return false;
      if (is_rank(*itr)) {
        // 次は終了か=+#かxかマスでなくてはなない。
        ++itr;
        if (itr == str.end()
        || (*itr == '=') || (*itr == '+') || (*itr == '#')) {
          // 無視。
        } else if (*itr == 'x') {
          ++itr;
          // 次はマスでなくてはならない。
          if (is_square(itr)) {
            itr += 2;
          } else {
            return false;
          }
        } else if (is_square(itr)) {
          itr += 2;
        } else {
          return false;
        }
      } else if (*itr == 'x') {
        // 次はマスでなくてはならない。
        ++itr;
        if (is_square(itr)) {
          itr += 2;
        } else {
          return false;
        }
      } else if (is_square(itr)) {
        itr += 2;
      } else {
        return false;
      }
    } else if (is_rank(*itr)) {  // ランク。
      // 次はxかマスでなくてはならない。
      ++itr;
      if (*itr == 'x') {
        // 次はマスでなくてはならない。
        ++itr;
        if (!is_square(itr)) return false;
        itr += 2;
      } else if (is_square(itr)) {
        itr += 2;
      } else {
        return false;
      }
    } else if (*itr == 'x') {  // 駒を取る記号。
      // 次はマスでなくてはならない。
      ++itr;
      if (!is_square(itr)) return false;
      itr += 2;
    } else {
      return false;
    }

    // 続きがあるなら続きを判定。
    if (itr != str.end()) {
      // 次は=か+か#でなくてはならない。
      if (*itr == '=') {
        // 次はNBRQでなくてはなない。
        ++itr;
        if (itr == str.end()) return false;
        if ((*itr == 'N') || (*itr == 'B') || (*itr == 'R') || (*itr == 'Q')) {
          ++itr;
          // 次は終了か+#でなくてはならない。
          if (itr == str.end()) {
            // 無視。
          } else if ((*itr == '+') || (*itr == '#')) {
            ++itr;
          } else {
            return false;
          }
        } else {
          return false;
        }
      } else if ((*itr == '+') || (*itr == '#')) {
        ++itr;
      } else {
        return false;
      }
    }

    // しっかり終わっていればtrue。
    return itr == str.end();
  }

  // Algebraic Notationを6文字に変換。
  std::string Util::ParseAlgebraicNotation(const std::string& note) {
    // ファイル・ランクの判定関数。
    auto is_fyle = [](char c) -> bool {return (c >= 'a') && (c <= 'h');};
    auto is_rank = [](char c) -> bool {return (c >= '1') && (c <= '8');};

    // おしりのチェック・チェックメイトの印を除く。
    std::string note_2 = note;
    if ((note_2.back() == '#') || (note_2.back() == '+')) {
      note_2.pop_back();
    }

    if ((note_2 == "O-O") || (note_2 == "O-O-O")) return note_2;

    std::string::const_iterator note_itr = note_2.cbegin();
    std::string ret = "------";

    // 駒の種類を判定。
    if (note_itr == note_2.end()) return ret;
    if (is_fyle(*note_itr)) {
      ret[0] = 'P';
    } else {
      switch (*note_itr) {
        case 'N': case 'B': case 'R': case 'Q': case 'K':
          ret[0] = *note_itr;
          ++note_itr;
          break;
        default:
          return ret;
      }
    }

    // 基点を判定。
    if (note_itr == note_2.end()) return ret;
    if (*note_itr == 'x') {
      ++note_itr;
      if (note_itr == note_2.end()) return ret;
    }
    if (is_rank(*note_itr)) {
      ret[2] = *note_itr;
      ++note_itr;
    } else if (is_fyle(*note_itr)) {
      std::string::const_iterator temp = note_itr;
      for (++temp; temp != note_2.end(); ++temp) {
        if (is_fyle(*temp)) {
          ret[1] = *note_itr;
          ++note_itr;
          if (is_rank(*note_itr)) {
            ret[2] = *note_itr;
            ++note_itr;
          }
          break;
        }
      }
    } else {
      return ret;
    }

    // 終点を判定。
    if (note_itr == note_2.end()) return ret;
    if (*note_itr == 'x') {
      ++note_itr;
      if (note_itr == note_2.end()) return ret;
    }
    if (!is_fyle(*note_itr)) return ret;
    ret[3] = *note_itr;
    ++note_itr;
    if (note_itr == note_2.end()) return ret;
    if (!is_rank(*note_itr)) return ret;
    ret[4] = *note_itr;
    ++note_itr;

    // 昇格を判定。
    if (note_itr == note_2.end()) return ret;
    if (*note_itr != '=') return ret;
    ++note_itr;
    if (note_itr == note_2.end()) return ret;
    switch (*note_itr) {
      case 'N': case 'B': case 'R': case 'Q':
        ret[5] = *note_itr;
        break;
      default:
        return ret;
    }

    return ret;
  }
}  // namespace Sayuri
