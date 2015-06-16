/* The MIT License (MIT)
 *
 * Copyright (c) 2015 Hironori Ishibashi
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
#include <vector>
#include <random>
#include <climits>
#include <set>
#include <cstdint>
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
  constexpr Bitboard Util::SQUARE[NUM_SQUARES];
  constexpr Bitboard Util::FYLE[NUM_FYLES];
  constexpr Bitboard Util::RANK[NUM_RANKS];
  constexpr Bitboard Util::SQCOLOR[NUM_SIDES];
  constexpr Square Util::ROT45[NUM_SQUARES];
  constexpr Square Util::ROT90[NUM_SQUARES];
  constexpr Square Util::ROT135[NUM_SQUARES];
  constexpr Square Util::R_ROT45[NUM_SQUARES];
  constexpr Square Util::R_ROT90[NUM_SQUARES];
  constexpr Square Util::R_ROT135[NUM_SQUARES];
  constexpr Square Util::FLIP[NUM_SQUARES];
  constexpr Rank Util::EN_PASSANT_TRANS_TABLE[NUM_SQUARES];
  constexpr int Util::MAGIC_SHIFT_V[NUM_SQUARES];
  constexpr int Util::MAGIC_SHIFT_D[NUM_SQUARES];
  constexpr Bitboard Util::MAGIC_MASK_V[NUM_SQUARES];
  constexpr Bitboard Util::MAGIC_MASK_D[NUM_SQUARES];
  constexpr unsigned int Util::BLOCKER_MAP;

  // ================== //
  // Utilクラスの初期化 //
  // ================== //
  // static変数の初期化。
  Bitboard Util::queen_move_[NUM_SQUARES];
  Bitboard Util::between_[NUM_SQUARES][NUM_SQUARES];
  void Util::InitUtil() {
    // num_bit16_table_[]を初期化する。
    InitNumBit16Table();
    // attack_table_***_[][]を初期化する。
    InitMagicTable();
    // line_[][]を初期化する。
    InitLine();
    // between_[][]を初期化する。
    FOR_SQUARES(square_1) {
      FOR_SQUARES(square_2) {
        between_[square_1][square_2] =
        line_[square_1][square_2] & ~(SQUARE[square_1] | SQUARE[square_2]);
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
  // 0度方向への攻撃の配列。 [マス][ブロッカーのパターン8ビット]
  Bitboard Util::attack_table_0_[NUM_SQUARES][BLOCKER_MAP + 1];
  // 45度方向への攻撃の配列。 [マス][ブロッカーのパターン8ビット]
  Bitboard Util::attack_table_45_[NUM_SQUARES][BLOCKER_MAP + 1];
  // 90度方向への攻撃の配列。 [マス][ブロッカーのパターン8ビット]
  Bitboard Util::attack_table_90_[NUM_SQUARES][BLOCKER_MAP + 1];
  // 135度方向への攻撃の配列。 [マス][ブロッカーのパターン8ビット]
  Bitboard Util::attack_table_135_[NUM_SQUARES][BLOCKER_MAP + 1];
  // ポーンの動ける位置の配列。
  Bitboard Util::pawn_movable_table_[NUM_SIDES][NUM_SQUARES][BLOCKER_MAP + 1];
  // attack_table_***_[][]を初期化する。
  void Util::InitMagicTable() {
    // 0度のマップを作成。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[square];
      point >>= MAGIC_SHIFT_V[square];
      for (unsigned int map = 0; map <= BLOCKER_MAP; ++map) {
        Bitboard attack = 0;
        // 右側を作る。
        Bitboard temp = point;
        while ((temp = GetRightBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while ((temp = GetLeftBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= MAGIC_MASK_V[square];
        // 利き筋をシフトする。
        attack <<= MAGIC_SHIFT_V[square];
        // 利き筋をマップに入れる。
        attack_table_0_[square][map] = attack;
      }
    }

    // 45度のマップを作成。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[ROT45[square]];
      point >>= MAGIC_SHIFT_D[ROT45[square]];
      for (unsigned int map = 0; map <= BLOCKER_MAP; ++map) {
        Bitboard attack = 0;
        // 右側を作る。
        Bitboard temp = point;
        while ((temp = GetRightBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while ((temp = GetLeftBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= MAGIC_MASK_D[ROT45[square]];
        // 利き筋をシフトする。
        attack <<= MAGIC_SHIFT_D[ROT45[square]];
        // 利き筋を通常の座標にする。
        attack = Reverse45(attack);
        // 利き筋をマップに入れる。
        attack_table_45_[square][map] = attack;
      }
    }

    // 90度のマップを作成。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[ROT90[square]];
      point >>= MAGIC_SHIFT_V[ROT90[square]];
      for (unsigned int map = 0; map <= BLOCKER_MAP; ++map) {
        Bitboard attack = 0;
        // 右側を作る。
        Bitboard temp = point;
        while ((temp = GetRightBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while ((temp = GetLeftBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= MAGIC_MASK_V[ROT90[square]];
        // 利き筋をシフトする。
        attack <<= MAGIC_SHIFT_V[ROT90[square]];
        // 利き筋を通常の座標にする。
        attack = Reverse90(attack);
        // 利き筋をマップに入れる。
        attack_table_90_[square][map] = attack;
      }
    }

    // 135度のマップを作成。
    FOR_SQUARES(square) {
      Bitboard point = SQUARE[ROT135[square]];
      point >>= MAGIC_SHIFT_D[ROT135[square]];
      for (unsigned int map = 0; map <= BLOCKER_MAP; ++map) {
        Bitboard attack = 0;
        // 右側を作る。
        Bitboard temp = point;
        while ((temp = GetRightBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 左側を作る。
        temp = point;
        while ((temp = GetLeftBitboard(temp))) {
          attack |= temp;
          if (temp & map) break;
        }
        // 利き筋をマスクする。
        attack &= MAGIC_MASK_D[ROT135[square]];
        // 利き筋をシフトする。
        attack <<= MAGIC_SHIFT_D[ROT135[square]];
        // 利き筋を通常の座標にする。
        attack = Reverse135(attack);
        // 利き筋をマップに入れる。
        attack_table_135_[square][map] = attack;
      }
    }

    // ポーンの動ける位置の配列を作成。
    FOR_SIDES(side) {
      FOR_SQUARES(square) {
        for (unsigned int map = 0; map <= BLOCKER_MAP; ++map) {
          if (side == WHITE) {
            if (SquareToRank(square) == RANK_2) {
              // 初期配置のポーンの位置。
              Bitboard front_bit =
              SQUARE[ROT90[square + 8]] >> MAGIC_SHIFT_V[ROT90[square + 8]];
              if (!(front_bit & map)) {
                // 1歩目。
                pawn_movable_table_[side][square][map] = SQUARE[square + 8];

                // 2歩目。
                front_bit = SQUARE[ROT90[square + 16]] >>
                MAGIC_SHIFT_V[ROT90[square + 16]];
                if (!(front_bit & map)) {
                  pawn_movable_table_[side][square][map] |=
                  SQUARE[square + 16];
                }
              } else {
                pawn_movable_table_[side][square][map] = 0;
              }
            } else if (SquareToRank(square) == RANK_8) {
              // 最上ランク。
              pawn_movable_table_[side][square][map] = 0;
            } else {
              // その他のランク。
              Bitboard front_bit =
              SQUARE[ROT90[square + 8]] >> MAGIC_SHIFT_V[ROT90[square + 8]];
              if (!(front_bit & map)) {
                pawn_movable_table_[side][square][map] = SQUARE[square + 8];
              } else {
                pawn_movable_table_[side][square][map] = 0;
              }
            }
          } else if (side == BLACK) {
            if (SquareToRank(square) == RANK_7) {
              // 初期配置のポーンの位置。
              Bitboard front_bit =
              SQUARE[ROT90[square - 8]] >> MAGIC_SHIFT_V[ROT90[square - 8]];
              if (!(front_bit & map)) {
                // 1歩目。
                pawn_movable_table_[side][square][map] = SQUARE[square - 8];

                // 2歩目。
                front_bit = SQUARE[ROT90[square - 16]] >>
                MAGIC_SHIFT_V[ROT90[square - 16]];
                if (!(front_bit & map)) {
                  pawn_movable_table_[side][square][map] |=
                  SQUARE[square - 16];
                }
              } else {
                pawn_movable_table_[side][square][map] = 0;
              }
            } else if (SquareToRank(square) == RANK_1) {
              // 最上ランク。
              pawn_movable_table_[side][square][map] = 0;
            } else {
              // その他のランク。
              Bitboard front_bit =
              SQUARE[ROT90[square - 8]] >> MAGIC_SHIFT_V[ROT90[square - 8]];
              if (!(front_bit & map)) {
                pawn_movable_table_[side][square][map] = SQUARE[square - 8];
              } else {
                pawn_movable_table_[side][square][map] = 0;
              }
            }
          } else {
            pawn_movable_table_[side][square][map] = 0;
          }
        }
      }
    }
  }

  // 45度の座標を0度に逆変換する。
  Bitboard Util::Reverse45(Bitboard bitboard45) {
    // 通常座標に変換する。
    Bitboard bitboard = 0;
    for (;bitboard45; NEXT_BITBOARD(bitboard45)) {
      // 45度座標の位置を得る。
      Square square45 = CountZero(bitboard45);
      // 変換して追加。
      bitboard |= SQUARE[R_ROT45[square45]];
    }

    // 返す。
    return bitboard;
  }
  // 90度の座標を0度に逆変換する。
  Bitboard Util::Reverse90(Bitboard bitboard90) {
    // 通常座標に変換する。
    Bitboard bitboard = 0;
    for (;bitboard90; NEXT_BITBOARD(bitboard90)) {
      // 90度座標の位置を得る。
      Square square90 = CountZero(bitboard90);
      // 変換して追加。
      bitboard |= SQUARE[R_ROT90[square90]];
    }

    // 返す。
    return bitboard;
  }
  // 135度の座標を0度に逆変換する。
  Bitboard Util::Reverse135(Bitboard bitboard135) {
    // 通常座標に変換する。
    Bitboard bitboard = 0;
    for (;bitboard135; NEXT_BITBOARD(bitboard135)) {
      // 135度座標の位置を得る。
      Square square135 = CountZero(bitboard135);
      // 変換して追加。
      bitboard |= SQUARE[R_ROT135[square135]];
    }

    // 返す。
    return bitboard;
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
        Bitboard point_1 = SQUARE[square_1];
        Bitboard point_2 = SQUARE[square_2];

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
      Bitboard point = SQUARE[square];
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
      Bitboard point = SQUARE[square];

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
      Bitboard point = SQUARE[square];

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
      Bitboard point = SQUARE[square];

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
      Bitboard point = SQUARE[square];
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
      Bitboard point = SQUARE[square];
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
      Bitboard point = SQUARE[square];

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

  // --- ランダム関連 --- //
  std::mt19937 Util::engine_;
  std::uniform_int_distribution<Hash> Util::dist_;
  void Util::InitRandom() {
    // メルセンヌツイスターの準備。
    engine_ = std::mt19937(SysClock::to_time_t(SysClock::now()));
    dist_ = std::uniform_int_distribution<Hash>(0, ULLONG_MAX);
  }
}  // namespace Sayuri
