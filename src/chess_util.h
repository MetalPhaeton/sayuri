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
 * @file chess_util.h
 * @author Hironori Ishibashi
 * @brief Sayuri用便利ツール。
 */

#ifndef CHESS_UTIL_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define CHESS_UTIL_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <set>
#include <cstdint>

#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  /**
   * Sayuri用便利ツールのクラス。
   */
  class Util {
    public:
      // ============ //
      // UTilの初期化 //
      // ============ //
      /** static変数の初期化。 */
      static void InitUtil();

      // ============ //
      // 座標変換配列 //
      // ============ //
      // --- 変換 --- //
      /** 0度から45度に変換。 [マス] */
      static constexpr Square ROT45[NUM_SQUARES] {
        E4, F3, H2, C2, G1, D1, B1, A1,
        E5, F4, G3, A3, D2, H1, E1, C1,
        D6, F5, G4, H3, B3, E2, A2, F1,
        B7, E6, G5, H4, A4, C3, F2, B2,
        G7, C7, F6, H5, A5, B4, D3, G2,
        C8, H7, D7, G6, A6, B5, C4, E3,
        F8, D8, A8, E7, H6, B6, C5, D4,
        H8, G8, E8, B8, F7, A7, C6, D5
      };
      /** 0度から90度に変換。 [マス] */
      static constexpr Square ROT90[NUM_SQUARES] {
        H1, H2, H3, H4, H5, H6, H7, H8,
        G1, G2, G3, G4, G5, G6, G7, G8,
        F1, F2, F3, F4, F5, F6, F7, F8,
        E1, E2, E3, E4, E5, E6, E7, E8,
        D1, D2, D3, D4, D5, D6, D7, D8,
        C1, C2, C3, C4, C5, C6, C7, C8,
        B1, B2, B3, B4, B5, B6, B7, B8,
        A1, A2, A3, A4, A5, A6, A7, A8
      };
      /** 0度から135度に変換。 [マス] */
      static constexpr Square ROT135[NUM_SQUARES] {
        A1, C1, F1, B2, G2, E3, D4, D5,
        B1, E1, A2, F2, D3, C4, C5, C6,
        D1, H1, E2, C3, B4, B5, B6, A7,
        G1, D2, B3, A4, A5, A6, H6, F7,
        C2, A3, H3, H4, H5, G6, E7, B8,
        H2, G3, G4, G5, F6, D7, A8, E8,
        F3, F4, F5, E6, C7, H7, D8, G8,
        E4, E5, D6, B7, G7, C8, F8, H8
      };

      // --- 逆変換 --- //
      /** 45度から0度に逆変換。 [45度座標のマス] */
      static constexpr Square R_ROT45[NUM_SQUARES] {
        H1,
        G1, H2,
        F1, G2, H3,
        E1, F2, G3, H4,
        D1, E2, F3, G4, H5,
        C1, D2, E3, F4, G5, H6,
        B1, C2, D3, E4, F5, G6, H7,
        A1, B2, C3, D4, E5, F6, G7, H8,
        A2, B3, C4, D5, E6, F7, G8,
        A3, B4, C5, D6, E7, F8,
        A4, B5, C6, D7, E8,
        A5, B6, C7, D8,
        A6, B7, C8,
        A7, B8,
        A8
      };
      /** 90度から0度に逆変換。 [90度座標のマス] */
      static constexpr Square R_ROT90[NUM_SQUARES] {
        A8, A7, A6, A5, A4, A3, A2, A1,
        B8, B7, B6, B5, B4, B3, B2, B1,
        C8, C7, C6, C5, C4, C3, C2, C1,
        D8, D7, D6, D5, D4, D3, D2, D1,
        E8, E7, E6, E5, E4, E3, E2, E1,
        F8, F7, F6, F5, F4, F3, F2, F1,
        G8, G7, G6, G5, G4, G3, G2, G1,
        H8, H7, H6, H5, H4, H3, H2, H1
      };
      /** 135度から0度に逆変換。 [135度座標のマス] */
      static constexpr Square R_ROT135[NUM_SQUARES] {
        A1,
        A2, B1,
        A3, B2, C1,
        A4, B3, C2, D1,
        A5, B4, C3, D2, E1,
        A6, B5, C4, D3, E2, F1,
        A7, B6, C5, D4, E3, F2, G1,
        A8, B7, C6, D5, E4, F3, G2, H1,
        B8, C7, D6, E5, F4, G3, H2,
        C8, D7, E6, F5, G4, H3,
        D8, E7, F6, G5, H4,
        E8, F7, G6, H5,
        F8, G7, H6,
        G8, H7,
        H8
      };

      /** ボードを鏡対象に上下反転。 [マス] */
      static constexpr Square FLIP[NUM_SQUARES] {
        A8, B8, C8, D8, E8, F8, G8, H8,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A1, B1, C1, D1, E1, F1, G1, H1
      };

      /**
       * アンパッサン用マス-ターゲット変換テーブル。 [マス]
       */
      static constexpr Square EN_PASSANT_TRANS_TABLE[NUM_SQUARES] {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A5, B5, C5, D5, E5, F5, G5, H5,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
      };

      // ================== //
      // ビットボードの配列 //
      // ================== //
      /** 0度のマスのビットボード。 [マス] */
      static constexpr Bitboard SQUARE0[NUM_SQUARES] {
        0x1ULL, 0x1ULL << 1, 0x1ULL << 2, 0x1ULL << 3,
        0x1ULL << 4, 0x1ULL << 5, 0x1ULL << 6, 0x1ULL << 7,
        0x1ULL << 8, 0x1ULL << 9, 0x1ULL << 10, 0x1ULL << 11,
        0x1ULL << 12, 0x1ULL << 13, 0x1ULL << 14, 0x1ULL << 15,
        0x1ULL << 16, 0x1ULL << 17, 0x1ULL << 18, 0x1ULL << 19,
        0x1ULL << 20, 0x1ULL << 21, 0x1ULL << 22, 0x1ULL << 23,
        0x1ULL << 24, 0x1ULL << 25, 0x1ULL << 26, 0x1ULL << 27,
        0x1ULL << 28, 0x1ULL << 29, 0x1ULL << 30, 0x1ULL << 31,
        0x1ULL << 32, 0x1ULL << 33, 0x1ULL << 34, 0x1ULL << 35,
        0x1ULL << 36, 0x1ULL << 37, 0x1ULL << 38, 0x1ULL << 39,
        0x1ULL << 40, 0x1ULL << 41, 0x1ULL << 42, 0x1ULL << 43,
        0x1ULL << 44, 0x1ULL << 45, 0x1ULL << 46, 0x1ULL << 47,
        0x1ULL << 48, 0x1ULL << 49, 0x1ULL << 50, 0x1ULL << 51,
        0x1ULL << 52, 0x1ULL << 53, 0x1ULL << 54, 0x1ULL << 55,
        0x1ULL << 56, 0x1ULL << 57, 0x1ULL << 58, 0x1ULL << 59,
        0x1ULL << 60, 0x1ULL << 61, 0x1ULL << 62, 0x1ULL << 63
      };

      static constexpr Bitboard SQUARE[NUM_SQUARES][NUM_ROTS] {
        {
          SQUARE0[A1], SQUARE0[ROT45[A1]],
          SQUARE0[ROT90[A1]], SQUARE0[ROT135[A1]]
        },
        {
          SQUARE0[B1], SQUARE0[ROT45[B1]],
          SQUARE0[ROT90[B1]], SQUARE0[ROT135[B1]]
        },
        {
          SQUARE0[C1], SQUARE0[ROT45[C1]],
          SQUARE0[ROT90[C1]], SQUARE0[ROT135[C1]]
        },
        {
          SQUARE0[D1], SQUARE0[ROT45[D1]],
          SQUARE0[ROT90[D1]], SQUARE0[ROT135[D1]]
        },
        {
          SQUARE0[E1], SQUARE0[ROT45[E1]],
          SQUARE0[ROT90[E1]], SQUARE0[ROT135[E1]]
        },
        {
          SQUARE0[F1], SQUARE0[ROT45[F1]],
          SQUARE0[ROT90[F1]], SQUARE0[ROT135[F1]]
        },
        {
          SQUARE0[G1], SQUARE0[ROT45[G1]],
          SQUARE0[ROT90[G1]], SQUARE0[ROT135[G1]]
        },
        {
          SQUARE0[H1], SQUARE0[ROT45[H1]],
          SQUARE0[ROT90[H1]], SQUARE0[ROT135[H1]]
        },

        {
          SQUARE0[A2], SQUARE0[ROT45[A2]],
          SQUARE0[ROT90[A2]], SQUARE0[ROT135[A2]]
        },
        {
          SQUARE0[B2], SQUARE0[ROT45[B2]],
          SQUARE0[ROT90[B2]], SQUARE0[ROT135[B2]]
        },
        {
          SQUARE0[C2], SQUARE0[ROT45[C2]],
          SQUARE0[ROT90[C2]], SQUARE0[ROT135[C2]]
        },
        {
          SQUARE0[D2], SQUARE0[ROT45[D2]],
          SQUARE0[ROT90[D2]], SQUARE0[ROT135[D2]]
        },
        {
          SQUARE0[E2], SQUARE0[ROT45[E2]],
          SQUARE0[ROT90[E2]], SQUARE0[ROT135[E2]]
        },
        {
          SQUARE0[F2], SQUARE0[ROT45[F2]],
          SQUARE0[ROT90[F2]], SQUARE0[ROT135[F2]]
        },
        {
          SQUARE0[G2], SQUARE0[ROT45[G2]],
          SQUARE0[ROT90[G2]], SQUARE0[ROT135[G2]]
        },
        {
          SQUARE0[H2], SQUARE0[ROT45[H2]],
          SQUARE0[ROT90[H2]], SQUARE0[ROT135[H2]]
        },

        {
          SQUARE0[A3], SQUARE0[ROT45[A3]],
          SQUARE0[ROT90[A3]], SQUARE0[ROT135[A3]]
        },
        {
          SQUARE0[B3], SQUARE0[ROT45[B3]],
          SQUARE0[ROT90[B3]], SQUARE0[ROT135[B3]]
        },
        {
          SQUARE0[C3], SQUARE0[ROT45[C3]],
          SQUARE0[ROT90[C3]], SQUARE0[ROT135[C3]]
        },
        {
          SQUARE0[D3], SQUARE0[ROT45[D3]],
          SQUARE0[ROT90[D3]], SQUARE0[ROT135[D3]]
        },
        {
          SQUARE0[E3], SQUARE0[ROT45[E3]],
          SQUARE0[ROT90[E3]], SQUARE0[ROT135[E3]]
        },
        {
          SQUARE0[F3], SQUARE0[ROT45[F3]],
          SQUARE0[ROT90[F3]], SQUARE0[ROT135[F3]]
        },
        {
          SQUARE0[G3], SQUARE0[ROT45[G3]],
          SQUARE0[ROT90[G3]], SQUARE0[ROT135[G3]]
        },
        {
          SQUARE0[H3], SQUARE0[ROT45[H3]],
          SQUARE0[ROT90[H3]], SQUARE0[ROT135[H3]]
        },

        {
          SQUARE0[A4], SQUARE0[ROT45[A4]],
          SQUARE0[ROT90[A4]], SQUARE0[ROT135[A4]]
        },
        {
          SQUARE0[B4], SQUARE0[ROT45[B4]],
          SQUARE0[ROT90[B4]], SQUARE0[ROT135[B4]]
        },
        {
          SQUARE0[C4], SQUARE0[ROT45[C4]],
          SQUARE0[ROT90[C4]], SQUARE0[ROT135[C4]]
        },
        {
          SQUARE0[D4], SQUARE0[ROT45[D4]],
          SQUARE0[ROT90[D4]], SQUARE0[ROT135[D4]]
        },
        {
          SQUARE0[E4], SQUARE0[ROT45[E4]],
          SQUARE0[ROT90[E4]], SQUARE0[ROT135[E4]]
        },
        {
          SQUARE0[F4], SQUARE0[ROT45[F4]],
          SQUARE0[ROT90[F4]], SQUARE0[ROT135[F4]]
        },
        {
          SQUARE0[G4], SQUARE0[ROT45[G4]],
          SQUARE0[ROT90[G4]], SQUARE0[ROT135[G4]]
        },
        {
          SQUARE0[H4], SQUARE0[ROT45[H4]],
          SQUARE0[ROT90[H4]], SQUARE0[ROT135[H4]]
        },

        {
          SQUARE0[A5], SQUARE0[ROT45[A5]],
          SQUARE0[ROT90[A5]], SQUARE0[ROT135[A5]]
        },
        {
          SQUARE0[B5], SQUARE0[ROT45[B5]],
          SQUARE0[ROT90[B5]], SQUARE0[ROT135[B5]]
        },
        {
          SQUARE0[C5], SQUARE0[ROT45[C5]],
          SQUARE0[ROT90[C5]], SQUARE0[ROT135[C5]]
        },
        {
          SQUARE0[D5], SQUARE0[ROT45[D5]],
          SQUARE0[ROT90[D5]], SQUARE0[ROT135[D5]]
        },
        {
          SQUARE0[E5], SQUARE0[ROT45[E5]],
          SQUARE0[ROT90[E5]], SQUARE0[ROT135[E5]]
        },
        {
          SQUARE0[F5], SQUARE0[ROT45[F5]],
          SQUARE0[ROT90[F5]], SQUARE0[ROT135[F5]]
        },
        {
          SQUARE0[G5], SQUARE0[ROT45[G5]],
          SQUARE0[ROT90[G5]], SQUARE0[ROT135[G5]]
        },
        {
          SQUARE0[H5], SQUARE0[ROT45[H5]],
          SQUARE0[ROT90[H5]], SQUARE0[ROT135[H5]]
        },

        {
          SQUARE0[A6], SQUARE0[ROT45[A6]],
          SQUARE0[ROT90[A6]], SQUARE0[ROT135[A6]]
        },
        {
          SQUARE0[B6], SQUARE0[ROT45[B6]],
          SQUARE0[ROT90[B6]], SQUARE0[ROT135[B6]]
        },
        {
          SQUARE0[C6], SQUARE0[ROT45[C6]],
          SQUARE0[ROT90[C6]], SQUARE0[ROT135[C6]]
        },
        {
          SQUARE0[D6], SQUARE0[ROT45[D6]],
          SQUARE0[ROT90[D6]], SQUARE0[ROT135[D6]]
        },
        {
          SQUARE0[E6], SQUARE0[ROT45[E6]],
          SQUARE0[ROT90[E6]], SQUARE0[ROT135[E6]]
        },
        {
          SQUARE0[F6], SQUARE0[ROT45[F6]],
          SQUARE0[ROT90[F6]], SQUARE0[ROT135[F6]]
        },
        {
          SQUARE0[G6], SQUARE0[ROT45[G6]],
          SQUARE0[ROT90[G6]], SQUARE0[ROT135[G6]]
        },
        {
          SQUARE0[H6], SQUARE0[ROT45[H6]],
          SQUARE0[ROT90[H6]], SQUARE0[ROT135[H6]]
        },

        {
          SQUARE0[A7], SQUARE0[ROT45[A7]],
          SQUARE0[ROT90[A7]], SQUARE0[ROT135[A7]]
        },
        {
          SQUARE0[B7], SQUARE0[ROT45[B7]],
          SQUARE0[ROT90[B7]], SQUARE0[ROT135[B7]]
        },
        {
          SQUARE0[C7], SQUARE0[ROT45[C7]],
          SQUARE0[ROT90[C7]], SQUARE0[ROT135[C7]]
        },
        {
          SQUARE0[D7], SQUARE0[ROT45[D7]],
          SQUARE0[ROT90[D7]], SQUARE0[ROT135[D7]]
        },
        {
          SQUARE0[E7], SQUARE0[ROT45[E7]],
          SQUARE0[ROT90[E7]], SQUARE0[ROT135[E7]]
        },
        {
          SQUARE0[F7], SQUARE0[ROT45[F7]],
          SQUARE0[ROT90[F7]], SQUARE0[ROT135[F7]]
        },
        {
          SQUARE0[G7], SQUARE0[ROT45[G7]],
          SQUARE0[ROT90[G7]], SQUARE0[ROT135[G7]]
        },
        {
          SQUARE0[H7], SQUARE0[ROT45[H7]],
          SQUARE0[ROT90[H7]], SQUARE0[ROT135[H7]]
        },

        {
          SQUARE0[A8], SQUARE0[ROT45[A8]],
          SQUARE0[ROT90[A8]], SQUARE0[ROT135[A8]]
        },
        {
          SQUARE0[B8], SQUARE0[ROT45[B8]],
          SQUARE0[ROT90[B8]], SQUARE0[ROT135[B8]]
        },
        {
          SQUARE0[C8], SQUARE0[ROT45[C8]],
          SQUARE0[ROT90[C8]], SQUARE0[ROT135[C8]]
        },
        {
          SQUARE0[D8], SQUARE0[ROT45[D8]],
          SQUARE0[ROT90[D8]], SQUARE0[ROT135[D8]]
        },
        {
          SQUARE0[E8], SQUARE0[ROT45[E8]],
          SQUARE0[ROT90[E8]], SQUARE0[ROT135[E8]]
        },
        {
          SQUARE0[F8], SQUARE0[ROT45[F8]],
          SQUARE0[ROT90[F8]], SQUARE0[ROT135[F8]]
        },
        {
          SQUARE0[G8], SQUARE0[ROT45[G8]],
          SQUARE0[ROT90[G8]], SQUARE0[ROT135[G8]]
        },
        {
          SQUARE0[H8], SQUARE0[ROT45[H8]],
          SQUARE0[ROT90[H8]], SQUARE0[ROT135[H8]]
        }
      };

      /** ファイルのビットボード。 [ファイル] */
      static constexpr Bitboard FYLE[NUM_FYLES] {
        0x0101010101010101ULL,
        0x0101010101010101ULL << 1,
        0x0101010101010101ULL << 2,
        0x0101010101010101ULL << 3,
        0x0101010101010101ULL << 4,
        0x0101010101010101ULL << 5,
        0x0101010101010101ULL << 6,
        0x0101010101010101ULL << 7
      };

      /** ランクのビットボード。 [ランク] */
      static constexpr Bitboard RANK[NUM_RANKS] {
        0xffULL,
        0xffULL << (1 * 8),
        0xffULL << (2 * 8),
        0xffULL << (3 * 8),
        0xffULL << (4 * 8),
        0xffULL << (5 * 8),
        0xffULL << (6 * 8),
        0xffULL << (7 * 8)
      };

      /** マスの色のビットボード。 [色 (サイド)] */
      static constexpr Bitboard SQCOLOR[NUM_SIDES] {
        0x0ULL,
        0xaa55aa55aa55aa55ULL,
        0x55aa55aa55aa55aaULL
      };

      // ======== //
      // マジック //
      // ======== //
      /** マジックのシフトの数。0度と90度用。 [マス] */
      static constexpr int MAGIC_SHIFT_V[NUM_SQUARES] {
        0, 0, 0, 0, 0, 0, 0, 0,
        8, 8, 8, 8, 8, 8, 8, 8,
        16, 16, 16, 16, 16, 16, 16, 16,
        24, 24, 24, 24, 24, 24, 24, 24,
        32, 32, 32, 32, 32, 32, 32, 32,
        40, 40, 40, 40, 40, 40, 40, 40,
        48, 48, 48, 48, 48, 48, 48, 48,
        56, 56, 56, 56, 56, 56, 56, 56
      };
      /** マジックのシフトの数。45度と135度用。 [マス] */
      static constexpr int MAGIC_SHIFT_D[NUM_SQUARES] {
        0,
        1, 1,
        3, 3, 3,
        6, 6, 6, 6,
        10, 10, 10, 10, 10,
        15, 15, 15, 15, 15, 15,
        21, 21, 21, 21, 21, 21, 21,
        28, 28, 28, 28, 28, 28, 28, 28,
        36, 36, 36, 36, 36, 36, 36,
        43, 43, 43, 43, 43, 43,
        49, 49, 49, 49, 49,
        54, 54, 54, 54,
        58, 58, 58,
        61, 61,
        63
      };
      /** マジックのシフトの数。 [マス][角度] */
      static constexpr int MAGIC_SHIFT[NUM_SQUARES][NUM_ROTS] {
        {
          MAGIC_SHIFT_V[A1], MAGIC_SHIFT_D[ROT45[A1]],
          MAGIC_SHIFT_V[ROT90[A1]], MAGIC_SHIFT_D[ROT135[A1]]
        },
        {
          MAGIC_SHIFT_V[B1], MAGIC_SHIFT_D[ROT45[B1]],
          MAGIC_SHIFT_V[ROT90[B1]], MAGIC_SHIFT_D[ROT135[B1]]
        },
        {
          MAGIC_SHIFT_V[C1], MAGIC_SHIFT_D[ROT45[C1]],
          MAGIC_SHIFT_V[ROT90[C1]], MAGIC_SHIFT_D[ROT135[C1]]
        },
        {
          MAGIC_SHIFT_V[D1], MAGIC_SHIFT_D[ROT45[D1]],
          MAGIC_SHIFT_V[ROT90[D1]], MAGIC_SHIFT_D[ROT135[D1]]
        },
        {
          MAGIC_SHIFT_V[E1], MAGIC_SHIFT_D[ROT45[E1]],
          MAGIC_SHIFT_V[ROT90[E1]], MAGIC_SHIFT_D[ROT135[E1]]
        },
        {
          MAGIC_SHIFT_V[F1], MAGIC_SHIFT_D[ROT45[F1]],
          MAGIC_SHIFT_V[ROT90[F1]], MAGIC_SHIFT_D[ROT135[F1]]
        },
        {
          MAGIC_SHIFT_V[G1], MAGIC_SHIFT_D[ROT45[G1]],
          MAGIC_SHIFT_V[ROT90[G1]], MAGIC_SHIFT_D[ROT135[G1]]
        },
        {
          MAGIC_SHIFT_V[H1], MAGIC_SHIFT_D[ROT45[H1]],
          MAGIC_SHIFT_V[ROT90[H1]], MAGIC_SHIFT_D[ROT135[H1]]
        },

        {
          MAGIC_SHIFT_V[A2], MAGIC_SHIFT_D[ROT45[A2]],
          MAGIC_SHIFT_V[ROT90[A2]], MAGIC_SHIFT_D[ROT135[A2]]
        },
        {
          MAGIC_SHIFT_V[B2], MAGIC_SHIFT_D[ROT45[B2]],
          MAGIC_SHIFT_V[ROT90[B2]], MAGIC_SHIFT_D[ROT135[B2]]
        },
        {
          MAGIC_SHIFT_V[C2], MAGIC_SHIFT_D[ROT45[C2]],
          MAGIC_SHIFT_V[ROT90[C2]], MAGIC_SHIFT_D[ROT135[C2]]
        },
        {
          MAGIC_SHIFT_V[D2], MAGIC_SHIFT_D[ROT45[D2]],
          MAGIC_SHIFT_V[ROT90[D2]], MAGIC_SHIFT_D[ROT135[D2]]
        },
        {
          MAGIC_SHIFT_V[E2], MAGIC_SHIFT_D[ROT45[E2]],
          MAGIC_SHIFT_V[ROT90[E2]], MAGIC_SHIFT_D[ROT135[E2]]
        },
        {
          MAGIC_SHIFT_V[F2], MAGIC_SHIFT_D[ROT45[F2]],
          MAGIC_SHIFT_V[ROT90[F2]], MAGIC_SHIFT_D[ROT135[F2]]
        },
        {
          MAGIC_SHIFT_V[G2], MAGIC_SHIFT_D[ROT45[G2]],
          MAGIC_SHIFT_V[ROT90[G2]], MAGIC_SHIFT_D[ROT135[G2]]
        },
        {
          MAGIC_SHIFT_V[H2], MAGIC_SHIFT_D[ROT45[H2]],
          MAGIC_SHIFT_V[ROT90[H2]], MAGIC_SHIFT_D[ROT135[H2]]
        },

        {
          MAGIC_SHIFT_V[A3], MAGIC_SHIFT_D[ROT45[A3]],
          MAGIC_SHIFT_V[ROT90[A3]], MAGIC_SHIFT_D[ROT135[A3]]
        },
        {
          MAGIC_SHIFT_V[B3], MAGIC_SHIFT_D[ROT45[B3]],
          MAGIC_SHIFT_V[ROT90[B3]], MAGIC_SHIFT_D[ROT135[B3]]
        },
        {
          MAGIC_SHIFT_V[C3], MAGIC_SHIFT_D[ROT45[C3]],
          MAGIC_SHIFT_V[ROT90[C3]], MAGIC_SHIFT_D[ROT135[C3]]
        },
        {
          MAGIC_SHIFT_V[D3], MAGIC_SHIFT_D[ROT45[D3]],
          MAGIC_SHIFT_V[ROT90[D3]], MAGIC_SHIFT_D[ROT135[D3]]
        },
        {
          MAGIC_SHIFT_V[E3], MAGIC_SHIFT_D[ROT45[E3]],
          MAGIC_SHIFT_V[ROT90[E3]], MAGIC_SHIFT_D[ROT135[E3]]
        },
        {
          MAGIC_SHIFT_V[F3], MAGIC_SHIFT_D[ROT45[F3]],
          MAGIC_SHIFT_V[ROT90[F3]], MAGIC_SHIFT_D[ROT135[F3]]
        },
        {
          MAGIC_SHIFT_V[G3], MAGIC_SHIFT_D[ROT45[G3]],
          MAGIC_SHIFT_V[ROT90[G3]], MAGIC_SHIFT_D[ROT135[G3]]
        },
        {
          MAGIC_SHIFT_V[H3], MAGIC_SHIFT_D[ROT45[H3]],
          MAGIC_SHIFT_V[ROT90[H3]], MAGIC_SHIFT_D[ROT135[H3]]
        },

        {
          MAGIC_SHIFT_V[A4], MAGIC_SHIFT_D[ROT45[A4]],
          MAGIC_SHIFT_V[ROT90[A4]], MAGIC_SHIFT_D[ROT135[A4]]
        },
        {
          MAGIC_SHIFT_V[B4], MAGIC_SHIFT_D[ROT45[B4]],
          MAGIC_SHIFT_V[ROT90[B4]], MAGIC_SHIFT_D[ROT135[B4]]
        },
        {
          MAGIC_SHIFT_V[C4], MAGIC_SHIFT_D[ROT45[C4]],
          MAGIC_SHIFT_V[ROT90[C4]], MAGIC_SHIFT_D[ROT135[C4]]
        },
        {
          MAGIC_SHIFT_V[D4], MAGIC_SHIFT_D[ROT45[D4]],
          MAGIC_SHIFT_V[ROT90[D4]], MAGIC_SHIFT_D[ROT135[D4]]
        },
        {
          MAGIC_SHIFT_V[E4], MAGIC_SHIFT_D[ROT45[E4]],
          MAGIC_SHIFT_V[ROT90[E4]], MAGIC_SHIFT_D[ROT135[E4]]
        },
        {
          MAGIC_SHIFT_V[F4], MAGIC_SHIFT_D[ROT45[F4]],
          MAGIC_SHIFT_V[ROT90[F4]], MAGIC_SHIFT_D[ROT135[F4]]
        },
        {
          MAGIC_SHIFT_V[G4], MAGIC_SHIFT_D[ROT45[G4]],
          MAGIC_SHIFT_V[ROT90[G4]], MAGIC_SHIFT_D[ROT135[G4]]
        },
        {
          MAGIC_SHIFT_V[H4], MAGIC_SHIFT_D[ROT45[H4]],
          MAGIC_SHIFT_V[ROT90[H4]], MAGIC_SHIFT_D[ROT135[H4]]
        },

        {
          MAGIC_SHIFT_V[A5], MAGIC_SHIFT_D[ROT45[A5]],
          MAGIC_SHIFT_V[ROT90[A5]], MAGIC_SHIFT_D[ROT135[A5]]
        },
        {
          MAGIC_SHIFT_V[B5], MAGIC_SHIFT_D[ROT45[B5]],
          MAGIC_SHIFT_V[ROT90[B5]], MAGIC_SHIFT_D[ROT135[B5]]
        },
        {
          MAGIC_SHIFT_V[C5], MAGIC_SHIFT_D[ROT45[C5]],
          MAGIC_SHIFT_V[ROT90[C5]], MAGIC_SHIFT_D[ROT135[C5]]
        },
        {
          MAGIC_SHIFT_V[D5], MAGIC_SHIFT_D[ROT45[D5]],
          MAGIC_SHIFT_V[ROT90[D5]], MAGIC_SHIFT_D[ROT135[D5]]
        },
        {
          MAGIC_SHIFT_V[E5], MAGIC_SHIFT_D[ROT45[E5]],
          MAGIC_SHIFT_V[ROT90[E5]], MAGIC_SHIFT_D[ROT135[E5]]
        },
        {
          MAGIC_SHIFT_V[F5], MAGIC_SHIFT_D[ROT45[F5]],
          MAGIC_SHIFT_V[ROT90[F5]], MAGIC_SHIFT_D[ROT135[F5]]
        },
        {
          MAGIC_SHIFT_V[G5], MAGIC_SHIFT_D[ROT45[G5]],
          MAGIC_SHIFT_V[ROT90[G5]], MAGIC_SHIFT_D[ROT135[G5]]
        },
        {
          MAGIC_SHIFT_V[H5], MAGIC_SHIFT_D[ROT45[H5]],
          MAGIC_SHIFT_V[ROT90[H5]], MAGIC_SHIFT_D[ROT135[H5]]
        },

        {
          MAGIC_SHIFT_V[A6], MAGIC_SHIFT_D[ROT45[A6]],
          MAGIC_SHIFT_V[ROT90[A6]], MAGIC_SHIFT_D[ROT135[A6]]
        },
        {
          MAGIC_SHIFT_V[B6], MAGIC_SHIFT_D[ROT45[B6]],
          MAGIC_SHIFT_V[ROT90[B6]], MAGIC_SHIFT_D[ROT135[B6]]
        },
        {
          MAGIC_SHIFT_V[C6], MAGIC_SHIFT_D[ROT45[C6]],
          MAGIC_SHIFT_V[ROT90[C6]], MAGIC_SHIFT_D[ROT135[C6]]
        },
        {
          MAGIC_SHIFT_V[D6], MAGIC_SHIFT_D[ROT45[D6]],
          MAGIC_SHIFT_V[ROT90[D6]], MAGIC_SHIFT_D[ROT135[D6]]
        },
        {
          MAGIC_SHIFT_V[E6], MAGIC_SHIFT_D[ROT45[E6]],
          MAGIC_SHIFT_V[ROT90[E6]], MAGIC_SHIFT_D[ROT135[E6]]
        },
        {
          MAGIC_SHIFT_V[F6], MAGIC_SHIFT_D[ROT45[F6]],
          MAGIC_SHIFT_V[ROT90[F6]], MAGIC_SHIFT_D[ROT135[F6]]
        },
        {
          MAGIC_SHIFT_V[G6], MAGIC_SHIFT_D[ROT45[G6]],
          MAGIC_SHIFT_V[ROT90[G6]], MAGIC_SHIFT_D[ROT135[G6]]
        },
        {
          MAGIC_SHIFT_V[H6], MAGIC_SHIFT_D[ROT45[H6]],
          MAGIC_SHIFT_V[ROT90[H6]], MAGIC_SHIFT_D[ROT135[H6]]
        },

        {
          MAGIC_SHIFT_V[A7], MAGIC_SHIFT_D[ROT45[A7]],
          MAGIC_SHIFT_V[ROT90[A7]], MAGIC_SHIFT_D[ROT135[A7]]
        },
        {
          MAGIC_SHIFT_V[B7], MAGIC_SHIFT_D[ROT45[B7]],
          MAGIC_SHIFT_V[ROT90[B7]], MAGIC_SHIFT_D[ROT135[B7]]
        },
        {
          MAGIC_SHIFT_V[C7], MAGIC_SHIFT_D[ROT45[C7]],
          MAGIC_SHIFT_V[ROT90[C7]], MAGIC_SHIFT_D[ROT135[C7]]
        },
        {
          MAGIC_SHIFT_V[D7], MAGIC_SHIFT_D[ROT45[D7]],
          MAGIC_SHIFT_V[ROT90[D7]], MAGIC_SHIFT_D[ROT135[D7]]
        },
        {
          MAGIC_SHIFT_V[E7], MAGIC_SHIFT_D[ROT45[E7]],
          MAGIC_SHIFT_V[ROT90[E7]], MAGIC_SHIFT_D[ROT135[E7]]
        },
        {
          MAGIC_SHIFT_V[F7], MAGIC_SHIFT_D[ROT45[F7]],
          MAGIC_SHIFT_V[ROT90[F7]], MAGIC_SHIFT_D[ROT135[F7]]
        },
        {
          MAGIC_SHIFT_V[G7], MAGIC_SHIFT_D[ROT45[G7]],
          MAGIC_SHIFT_V[ROT90[G7]], MAGIC_SHIFT_D[ROT135[G7]]
        },
        {
          MAGIC_SHIFT_V[H7], MAGIC_SHIFT_D[ROT45[H7]],
          MAGIC_SHIFT_V[ROT90[H7]], MAGIC_SHIFT_D[ROT135[H7]]
        },

        {
          MAGIC_SHIFT_V[A8], MAGIC_SHIFT_D[ROT45[A8]],
          MAGIC_SHIFT_V[ROT90[A8]], MAGIC_SHIFT_D[ROT135[A8]]
        },
        {
          MAGIC_SHIFT_V[B8], MAGIC_SHIFT_D[ROT45[B8]],
          MAGIC_SHIFT_V[ROT90[B8]], MAGIC_SHIFT_D[ROT135[B8]]
        },
        {
          MAGIC_SHIFT_V[C8], MAGIC_SHIFT_D[ROT45[C8]],
          MAGIC_SHIFT_V[ROT90[C8]], MAGIC_SHIFT_D[ROT135[C8]]
        },
        {
          MAGIC_SHIFT_V[D8], MAGIC_SHIFT_D[ROT45[D8]],
          MAGIC_SHIFT_V[ROT90[D8]], MAGIC_SHIFT_D[ROT135[D8]]
        },
        {
          MAGIC_SHIFT_V[E8], MAGIC_SHIFT_D[ROT45[E8]],
          MAGIC_SHIFT_V[ROT90[E8]], MAGIC_SHIFT_D[ROT135[E8]]
        },
        {
          MAGIC_SHIFT_V[F8], MAGIC_SHIFT_D[ROT45[F8]],
          MAGIC_SHIFT_V[ROT90[F8]], MAGIC_SHIFT_D[ROT135[F8]]
        },
        {
          MAGIC_SHIFT_V[G8], MAGIC_SHIFT_D[ROT45[G8]],
          MAGIC_SHIFT_V[ROT90[G8]], MAGIC_SHIFT_D[ROT135[G8]]
        },
        {
          MAGIC_SHIFT_V[H8], MAGIC_SHIFT_D[ROT45[H8]],
          MAGIC_SHIFT_V[ROT90[H8]], MAGIC_SHIFT_D[ROT135[H8]]
        }
      }
      /** マジックのマスク。0度と90度用。 [マス] */
      static constexpr Bitboard MAGIC_MASK_V[NUM_SQUARES] {
        0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
        0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
        0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
        0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
        0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
        0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
        0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
        0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL
      };
      /** マジックのマスク。45度と135度用。 [マス] */
      static constexpr Bitboard MAGIC_MASK_D[NUM_SQUARES] {
        0x1ULL,
        0x3ULL, 0x3ULL,
        0x7ULL, 0x7ULL, 0x7ULL,
        0xfULL, 0xfULL, 0xfULL, 0xfULL,
        0x1fULL, 0x1fULL, 0x1fULL, 0x1fULL, 0x1fULL,
        0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL,
        0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL,
        0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL, 0xffULL,
        0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL, 0x7fULL,
        0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL, 0x3fULL,
        0x1fULL, 0x1fULL, 0x1fULL, 0x1fULL, 0x1fULL,
        0xfULL, 0xfULL, 0xfULL, 0xfULL,
        0x7ULL, 0x7ULL, 0x7ULL,
        0x3ULL, 0x3ULL,
        0x1ULL
      };
      /** マジックのマスク。 [マス][角度] */
      static constexpr Bitboard MAGIC_MASK[NUM_SQUARES][NUM_ROTS] {
        {
          MAGIC_MASK_V[A1], MAGIC_MASK_D[ROT45[A1]],
          MAGIC_MASK_V[ROT90[A1]], MAGIC_MASK_D[ROT135[A1]]
        },
        {
          MAGIC_MASK_V[B1], MAGIC_MASK_D[ROT45[B1]],
          MAGIC_MASK_V[ROT90[B1]], MAGIC_MASK_D[ROT135[B1]]
        },
        {
          MAGIC_MASK_V[C1], MAGIC_MASK_D[ROT45[C1]],
          MAGIC_MASK_V[ROT90[C1]], MAGIC_MASK_D[ROT135[C1]]
        },
        {
          MAGIC_MASK_V[D1], MAGIC_MASK_D[ROT45[D1]],
          MAGIC_MASK_V[ROT90[D1]], MAGIC_MASK_D[ROT135[D1]]
        },
        {
          MAGIC_MASK_V[E1], MAGIC_MASK_D[ROT45[E1]],
          MAGIC_MASK_V[ROT90[E1]], MAGIC_MASK_D[ROT135[E1]]
        },
        {
          MAGIC_MASK_V[F1], MAGIC_MASK_D[ROT45[F1]],
          MAGIC_MASK_V[ROT90[F1]], MAGIC_MASK_D[ROT135[F1]]
        },
        {
          MAGIC_MASK_V[G1], MAGIC_MASK_D[ROT45[G1]],
          MAGIC_MASK_V[ROT90[G1]], MAGIC_MASK_D[ROT135[G1]]
        },
        {
          MAGIC_MASK_V[H1], MAGIC_MASK_D[ROT45[H1]],
          MAGIC_MASK_V[ROT90[H1]], MAGIC_MASK_D[ROT135[H1]]
        },

        {
          MAGIC_MASK_V[A2], MAGIC_MASK_D[ROT45[A2]],
          MAGIC_MASK_V[ROT90[A2]], MAGIC_MASK_D[ROT135[A2]]
        },
        {
          MAGIC_MASK_V[B2], MAGIC_MASK_D[ROT45[B2]],
          MAGIC_MASK_V[ROT90[B2]], MAGIC_MASK_D[ROT135[B2]]
        },
        {
          MAGIC_MASK_V[C2], MAGIC_MASK_D[ROT45[C2]],
          MAGIC_MASK_V[ROT90[C2]], MAGIC_MASK_D[ROT135[C2]]
        },
        {
          MAGIC_MASK_V[D2], MAGIC_MASK_D[ROT45[D2]],
          MAGIC_MASK_V[ROT90[D2]], MAGIC_MASK_D[ROT135[D2]]
        },
        {
          MAGIC_MASK_V[E2], MAGIC_MASK_D[ROT45[E2]],
          MAGIC_MASK_V[ROT90[E2]], MAGIC_MASK_D[ROT135[E2]]
        },
        {
          MAGIC_MASK_V[F2], MAGIC_MASK_D[ROT45[F2]],
          MAGIC_MASK_V[ROT90[F2]], MAGIC_MASK_D[ROT135[F2]]
        },
        {
          MAGIC_MASK_V[G2], MAGIC_MASK_D[ROT45[G2]],
          MAGIC_MASK_V[ROT90[G2]], MAGIC_MASK_D[ROT135[G2]]
        },
        {
          MAGIC_MASK_V[H2], MAGIC_MASK_D[ROT45[H2]],
          MAGIC_MASK_V[ROT90[H2]], MAGIC_MASK_D[ROT135[H2]]
        },

        {
          MAGIC_MASK_V[A3], MAGIC_MASK_D[ROT45[A3]],
          MAGIC_MASK_V[ROT90[A3]], MAGIC_MASK_D[ROT135[A3]]
        },
        {
          MAGIC_MASK_V[B3], MAGIC_MASK_D[ROT45[B3]],
          MAGIC_MASK_V[ROT90[B3]], MAGIC_MASK_D[ROT135[B3]]
        },
        {
          MAGIC_MASK_V[C3], MAGIC_MASK_D[ROT45[C3]],
          MAGIC_MASK_V[ROT90[C3]], MAGIC_MASK_D[ROT135[C3]]
        },
        {
          MAGIC_MASK_V[D3], MAGIC_MASK_D[ROT45[D3]],
          MAGIC_MASK_V[ROT90[D3]], MAGIC_MASK_D[ROT135[D3]]
        },
        {
          MAGIC_MASK_V[E3], MAGIC_MASK_D[ROT45[E3]],
          MAGIC_MASK_V[ROT90[E3]], MAGIC_MASK_D[ROT135[E3]]
        },
        {
          MAGIC_MASK_V[F3], MAGIC_MASK_D[ROT45[F3]],
          MAGIC_MASK_V[ROT90[F3]], MAGIC_MASK_D[ROT135[F3]]
        },
        {
          MAGIC_MASK_V[G3], MAGIC_MASK_D[ROT45[G3]],
          MAGIC_MASK_V[ROT90[G3]], MAGIC_MASK_D[ROT135[G3]]
        },
        {
          MAGIC_MASK_V[H3], MAGIC_MASK_D[ROT45[H3]],
          MAGIC_MASK_V[ROT90[H3]], MAGIC_MASK_D[ROT135[H3]]
        },

        {
          MAGIC_MASK_V[A4], MAGIC_MASK_D[ROT45[A4]],
          MAGIC_MASK_V[ROT90[A4]], MAGIC_MASK_D[ROT135[A4]]
        },
        {
          MAGIC_MASK_V[B4], MAGIC_MASK_D[ROT45[B4]],
          MAGIC_MASK_V[ROT90[B4]], MAGIC_MASK_D[ROT135[B4]]
        },
        {
          MAGIC_MASK_V[C4], MAGIC_MASK_D[ROT45[C4]],
          MAGIC_MASK_V[ROT90[C4]], MAGIC_MASK_D[ROT135[C4]]
        },
        {
          MAGIC_MASK_V[D4], MAGIC_MASK_D[ROT45[D4]],
          MAGIC_MASK_V[ROT90[D4]], MAGIC_MASK_D[ROT135[D4]]
        },
        {
          MAGIC_MASK_V[E4], MAGIC_MASK_D[ROT45[E4]],
          MAGIC_MASK_V[ROT90[E4]], MAGIC_MASK_D[ROT135[E4]]
        },
        {
          MAGIC_MASK_V[F4], MAGIC_MASK_D[ROT45[F4]],
          MAGIC_MASK_V[ROT90[F4]], MAGIC_MASK_D[ROT135[F4]]
        },
        {
          MAGIC_MASK_V[G4], MAGIC_MASK_D[ROT45[G4]],
          MAGIC_MASK_V[ROT90[G4]], MAGIC_MASK_D[ROT135[G4]]
        },
        {
          MAGIC_MASK_V[H4], MAGIC_MASK_D[ROT45[H4]],
          MAGIC_MASK_V[ROT90[H4]], MAGIC_MASK_D[ROT135[H4]]
        },

        {
          MAGIC_MASK_V[A5], MAGIC_MASK_D[ROT45[A5]],
          MAGIC_MASK_V[ROT90[A5]], MAGIC_MASK_D[ROT135[A5]]
        },
        {
          MAGIC_MASK_V[B5], MAGIC_MASK_D[ROT45[B5]],
          MAGIC_MASK_V[ROT90[B5]], MAGIC_MASK_D[ROT135[B5]]
        },
        {
          MAGIC_MASK_V[C5], MAGIC_MASK_D[ROT45[C5]],
          MAGIC_MASK_V[ROT90[C5]], MAGIC_MASK_D[ROT135[C5]]
        },
        {
          MAGIC_MASK_V[D5], MAGIC_MASK_D[ROT45[D5]],
          MAGIC_MASK_V[ROT90[D5]], MAGIC_MASK_D[ROT135[D5]]
        },
        {
          MAGIC_MASK_V[E5], MAGIC_MASK_D[ROT45[E5]],
          MAGIC_SHIFT_V[ROT90[E5]], MAGIC_MASK_D[ROT135[E5]]
        },
        {
          MAGIC_MASK_V[F5], MAGIC_MASK_D[ROT45[F5]],
          MAGIC_MASK_V[ROT90[F5]], MAGIC_MASK_D[ROT135[F5]]
        },
        {
          MAGIC_MASK_V[G5], MAGIC_MASK_D[ROT45[G5]],
          MAGIC_MASK_V[ROT90[G5]], MAGIC_MASK_D[ROT135[G5]]
        },
        {
          MAGIC_MASK_V[H5], MAGIC_MASK_D[ROT45[H5]],
          MAGIC_MASK_V[ROT90[H5]], MAGIC_MASK_D[ROT135[H5]]
        },

        {
          MAGIC_MASK_V[A6], MAGIC_MASK_D[ROT45[A6]],
          MAGIC_MASK_V[ROT90[A6]], MAGIC_MASK_D[ROT135[A6]]
        },
        {
          MAGIC_MASK_V[B6], MAGIC_MASK_D[ROT45[B6]],
          MAGIC_MASK_V[ROT90[B6]], MAGIC_MASK_D[ROT135[B6]]
        },
        {
          MAGIC_MASK_V[C6], MAGIC_MASK_D[ROT45[C6]],
          MAGIC_MASK_V[ROT90[C6]], MAGIC_MASK_D[ROT135[C6]]
        },
        {
          MAGIC_MASK_V[D6], MAGIC_MASK_D[ROT45[D6]],
          MAGIC_MASK_V[ROT90[D6]], MAGIC_MASK_D[ROT135[D6]]
        },
        {
          MAGIC_MASK_V[E6], MAGIC_MASK_D[ROT45[E6]],
          MAGIC_MASK_V[ROT90[E6]], MAGIC_MASK_D[ROT135[E6]]
        },
        {
          MAGIC_MASK_V[F6], MAGIC_MASK_D[ROT45[F6]],
          MAGIC_MASK_V[ROT90[F6]], MAGIC_MASK_D[ROT135[F6]]
        },
        {
          MAGIC_MASK_V[G6], MAGIC_MASK_D[ROT45[G6]],
          MAGIC_MASK_V[ROT90[G6]], MAGIC_MASK_D[ROT135[G6]]
        },
        {
          MAGIC_MASK_V[H6], MAGIC_MASK_D[ROT45[H6]],
          MAGIC_MASK_V[ROT90[H6]], MAGIC_MASK_D[ROT135[H6]]
        },

        {
          MAGIC_MASK_V[A7], MAGIC_MASK_D[ROT45[A7]],
          MAGIC_MASK_V[ROT90[A7]], MAGIC_MASK_D[ROT135[A7]]
        },
        {
          MAGIC_MASK_V[B7], MAGIC_MASK_D[ROT45[B7]],
          MAGIC_MASK_V[ROT90[B7]], MAGIC_MASK_D[ROT135[B7]]
        },
        {
          MAGIC_MASK_V[C7], MAGIC_MASK_D[ROT45[C7]],
          MAGIC_MASK_V[ROT90[C7]], MAGIC_MASK_D[ROT135[C7]]
        },
        {
          MAGIC_MASK_V[D7], MAGIC_MASK_D[ROT45[D7]],
          MAGIC_MASK_V[ROT90[D7]], MAGIC_MASK_D[ROT135[D7]]
        },
        {
          MAGIC_MASK_V[E7], MAGIC_MASK_D[ROT45[E7]],
          MAGIC_MASK_V[ROT90[E7]], MAGIC_MASK_D[ROT135[E7]]
        },
        {
          MAGIC_MASK_V[F7], MAGIC_MASK_D[ROT45[F7]],
          MAGIC_MASK_V[ROT90[F7]], MAGIC_MASK_D[ROT135[F7]]
        },
        {
          MAGIC_MASK_V[G7], MAGIC_MASK_D[ROT45[G7]],
          MAGIC_MASK_V[ROT90[G7]], MAGIC_MASK_D[ROT135[G7]]
        },
        {
          MAGIC_MASK_V[H7], MAGIC_MASK_D[ROT45[H7]],
          MAGIC_MASK_V[ROT90[H7]], MAGIC_MASK_D[ROT135[H7]]
        },

        {
          MAGIC_MASK_V[A8], MAGIC_MASK_D[ROT45[A8]],
          MAGIC_MASK_V[ROT90[A8]], MAGIC_MASK_D[ROT135[A8]]
        },
        {
          MAGIC_MASK_V[B8], MAGIC_MASK_D[ROT45[B8]],
          MAGIC_MASK_V[ROT90[B8]], MAGIC_MASK_D[ROT135[B8]]
        },
        {
          MAGIC_MASK_V[C8], MAGIC_MASK_D[ROT45[C8]],
          MAGIC_MASK_V[ROT90[C8]], MAGIC_MASK_D[ROT135[C8]]
        },
        {
          MAGIC_MASK_V[D8], MAGIC_MASK_D[ROT45[D8]],
          MAGIC_MASK_V[ROT90[D8]], MAGIC_MASK_D[ROT135[D8]]
        },
        {
          MAGIC_MASK_V[E8], MAGIC_MASK_D[ROT45[E8]],
          MAGIC_MASK_V[ROT90[E8]], MAGIC_MASK_D[ROT135[E8]]
        },
        {
          MAGIC_MASK_V[F8], MAGIC_MASK_D[ROT45[F8]],
          MAGIC_MASK_V[ROT90[F8]], MAGIC_MASK_D[ROT135[F8]]
        },
        {
          MAGIC_MASK_V[G8], MAGIC_MASK_D[ROT45[G8]],
          MAGIC_MASK_V[ROT90[G8]], MAGIC_MASK_D[ROT135[G8]]
        },
        {
          MAGIC_MASK_V[H8], MAGIC_MASK_D[ROT45[H8]],
          MAGIC_MASK_V[ROT90[H8]], MAGIC_MASK_D[ROT135[H8]]
        }
      }

      // ================================ //
      // 各種方向のビットボードを得る関数 //
      // ================================ //
      /**
       * 右に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を右に移動したビットボード。
       */
      static constexpr Bitboard GetRightBitboard(Bitboard bitboard) {
        return (bitboard & ~FYLE[FYLE_H]) << 1;
      }
      /**
       * 左に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を左に移動したビットボード。
       */
      static constexpr Bitboard GetLeftBitboard(Bitboard bitboard) {
        return (bitboard & ~FYLE[FYLE_A]) >> 1;
      }
      /**
       * 上に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を上に移動したビットボード。
       */
      static constexpr Bitboard GetUpBitboard(Bitboard bitboard) {
        return (bitboard & ~RANK[RANK_8]) << 8;
      }
      /**
       * 下に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を下に移動したビットボード。
       */
      static constexpr Bitboard GetDownBitboard(Bitboard bitboard) {
        return (bitboard & ~RANK[RANK_1]) >> 8;
      }
      /**
       * 右上に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を右上に移動したビットボード。
       */
      static constexpr Bitboard GetRightUpBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_8] | FYLE[FYLE_H])) << 9;
      }
      /**
       * 右下に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を右下に移動したビットボード。
       */
      static constexpr Bitboard GetRightDownBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_1] | FYLE[FYLE_H])) >> 7;
      }
      /**
       * 左上に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を左上に移動したビットボード。
       */
      static constexpr Bitboard GetLeftUpBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_8] | FYLE[FYLE_A])) << 7;
      }
      /**
       * 左下に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を左下に移動したビットボード。
       */
      static constexpr Bitboard GetLeftDownBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_1] | FYLE[FYLE_A])) >> 9;
      }
      /**
       * 右右上に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を右右上に移動したビットボード。
       */
      static constexpr Bitboard GetRightRightUpBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_8] | FYLE[FYLE_H] | FYLE[FYLE_G]))
        << 10;
      }
      /**
       * 右上上に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を右上上に移動したビットボード。
       */
      static constexpr Bitboard GetRightUpUpBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_8] | RANK[RANK_7] | FYLE[FYLE_H]))
        << 17;
      }
      /**
       * 右右下に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を右右下に移動したビットボード。
       */
      static constexpr Bitboard GetRightRightDownBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_1] | FYLE[FYLE_H] | FYLE[FYLE_G]))
        >> 6;
      }
      /**
       * 右下下に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を右下下に移動したビットボード。
       */
      static constexpr Bitboard GetRightDownDownBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_1] | RANK[RANK_2] | FYLE[FYLE_H]))
        >> 15;
      }
      /**
       * 左左上に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を左左上に移動したビットボード。
       */
      static constexpr Bitboard GetLeftLeftUpBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_8] | FYLE[FYLE_A] | FYLE[FYLE_B]))
        << 6;
      }
      /**
       * 左上上に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を左上上に移動したビットボード。
       */
      static constexpr Bitboard GetLeftUpUpBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_8] | RANK[RANK_7] | FYLE[FYLE_A]))
        << 15;
      }
      /**
       * 左左下に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を左左下に移動したビットボード。
       */
      static constexpr Bitboard GetLeftLeftDownBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_1] | FYLE[FYLE_A] | FYLE[FYLE_B]))
        >> 10;
      }
      /**
       * 左下下に移動したビットボードを得る。
       * @param bitboard 対象のビットボード。
       * @return 「bitboard」を左下下に移動したビットボード。
       */
      static constexpr Bitboard GetLeftDownDownBitboard(Bitboard bitboard) {
        return (bitboard & ~(RANK[RANK_1] | RANK[RANK_2] | FYLE[FYLE_A]))
        >> 17;
      }

      // ====================== //
      // 利き筋ラインを得る関数 //
      // ====================== //
      /**
       * 0度の利き筋ラインを得る。
       * @param square 基点。
       * @param blocker_0 全駒の配置のビットボード。 角度0度。
       * @return 利き筋ライン。
       */
      static Bitboard GetAttack0(Square square, Bitboard blocker_0) {
        return attack_table_0_[square]
        [(blocker_0 >> MAGIC_SHIFT_V[square]) & MAGIC_MASK_V[square]];
      }
      /**
       * 45度の利き筋ラインを得る。
       * @param square 基点。
       * @param blocker_45 全駒の配置のビットボード。 角度45度。
       * @return 利き筋ライン。
       */
      static Bitboard GetAttack45(Square square, Bitboard blocker_45) {
        return attack_table_45_[square]
        [(blocker_45 >> MAGIC_SHIFT_D[ROT45[square]])
        & MAGIC_MASK_D[ROT45[square]]];
      }
      /**
       * 90度の利き筋ラインを得る。
       * @param square 基点。
       * @param blocker_90 全駒の配置のビットボード。 角度90度。
       * @return 利き筋ライン。
       */
      static Bitboard GetAttack90(Square square, Bitboard blocker_90) {
        return attack_table_90_[square]
        [(blocker_90 >> MAGIC_SHIFT_V[ROT90[square]])
        & MAGIC_MASK_V[ROT90[square]]];
      }
      /**
       * 135度の利き筋ラインを得る。
       * @param square 基点。
       * @param blocker_135 全駒の配置のビットボード。 角度135度。
       * @return 利き筋ライン。
       */
      static Bitboard GetAttack135(Square square, Bitboard blocker_135) {
        return attack_table_135_[square]
        [(blocker_135 >> MAGIC_SHIFT_D[ROT135[square]])
        & MAGIC_MASK_D[ROT135[square]]];
      }
      /**
       * ポーンの動ける位置を得る。
       * @param side ポーンのサイド。
       * @param square 基点。
       * @param blocker_90 全駒の配置のビットボード。 角度90度。
       * @return ポーン動ける位置。
       */
      static Bitboard GetPawnMovable(Side side, Square square,
      Bitboard blocker_90) {
        return pawn_movable_table_[side][square]
        [(blocker_90 >> MAGIC_SHIFT_V[ROT90[square]])
        & MAGIC_MASK_V[ROT90[square]]];
      }

      // ==================== //
      // ビットボード作成関数 //
      // ==================== //
      /**
       * 2点を結ぶ直線のビットボードを得る。 (2点を含む。)
       * @param point_1 端点1。
       * @param point_2 端点2。
       * @return 直線のビットボード。
       */
      static Bitboard GetLine(Square point_1, Square point_2) {
        return line_[point_1][point_2];
      }
      /**
       * 2点を結ぶ直線のビットボードを得る。 (2点を除く。)
       * @param point_1 端点1。
       * @param point_2 端点2。
       * @return 直線のビットボード。
       */
      static Bitboard GetBetween(Square point_1, Square point_2) {
        return between_[point_1][point_2];
      }
      /**
       * ポーンの通常の動きのビットボードを得る。
       * @param side ポーンのサイド。
       * @param square 基点。
       * @return ポーンの通常の動きのビットボード。
       */
      static Bitboard GetPawnMove(Side side, Square square) {
        return pawn_move_[side][square];
      }
      /**
       * ポーンの2歩の動きのビットボードを得る。
       * @param side ポーンのサイド。
       * @param square 基点。
       * @return ポーンの2歩の動きのビットボード。
       */
      static Bitboard GetPawn2StepMove(Side side, Square square) {
        return pawn_2step_move_[side][square];
      }
      /**
       * ポーンの攻撃の動きのビットボードを得る。
       * @param side ポーンのサイド。
       * @param square 基点。
       * @return ポーンの攻撃の動きのビットボード。
       */
      static Bitboard GetPawnAttack(Side side, Square square) {
        return pawn_attack_[side][square];
      }
      /**
       * ナイトの動きのビットボードを得る。
       * @param square 基点。
       * @return ナイトの動きのビットボード。
       */
      static Bitboard GetKnightMove(Square square) {
        return knight_move_[square];
      }
      /**
       * ビショップの動きのビットボードを得る。
       * @param square 基点。
       * @return ビショップの動きのビットボード。
       */
      static Bitboard GetBishopMove(Square square) {
        return bishop_move_[square];
      }
      /**
       * ルークの動きのビットボードを得る。
       * @param square 基点。
       * @return ルークの動きのビットボード。
       */
      static Bitboard GetRookMove(Square square) {
        return rook_move_[square];
      }
      /**
       * クイーンの動きのビットボードを得る。
       * @param square 基点。
       * @return クイーンの動きのビットボード。
       */
      static Bitboard GetQueenMove(Square square) {
        return queen_move_[square];
      }
      /**
       * キングの動きのビットボードを得る。
       * @param square 基点。
       * @return キングの動きのビットボード。
       */
      static Bitboard GetKingMove(Square square) {
        return king_move_[square];
      }

      // ========== //
      // 位置の変換 //
      // ========== //
      /**
       * マスをファイルに変換。
       * @param マス。
       * @return ファイル。
       */
      static constexpr Fyle SquareToFyle(Square square) {
        return square & 7;
      }
      /**
       * マスをランクに変換。
       * @param マス。
       * @return ランク。
       */
      static constexpr Rank SquareToRank(Square square) {
        return (square >> 3) & 7;
      }
      /**
       * ファイルとランクをマスに変換。
       * @param fyle ファイル。
       * @param rank ランク。
       * @return マス。
       */
      static constexpr Square CoordToSquare(Fyle fyle, Rank rank) {
        return ((rank << 3) | (fyle & 7)) & 63;
      }

      // ================ //
      // その他の便利関数 //
      // ================ //
      /**
       * 最大値を返す。
       * @param val_1 値1。
       * @param val_2 値2。
       * @return val_1とval_2の大きい方。
       */
      template<class T, class S>
      constexpr static auto GetMax(T val_1, S val_2)
      -> decltype(val_1 + val_2) {
        return val_1 > val_2 ? val_1 : val_2;
      }

      /**
       * 最小値を返す。
       * @param val_1 値1。
       * @param val_2 値2。
       * @return val_1とval_2の小さい方。
       */
      template<class T, class S>
      constexpr static auto GetMin(T val_1, S val_2)
      -> decltype(val_1 + val_2) {
        return val_1 < val_2 ? val_1 : val_2;
      }

      /**
       * 変数を最大値に更新する。
       * @param dst 更新したい変数。
       * @param value 更新元の数値。
       */
      template<class T, class S>
      static void UpdateMax(T& dst, S value) {
        if (value > dst) dst = value;
      }

      /**
       * 変数を最小値に更新する。
       * @param dst 更新したい変数。
       * @param value 更新元の数値。
       */
      template<class T, class S>
      static void UpdateMin(T& dst, S value) {
        if (value < dst) dst = value;
      }

      /**
       * 立っているビットの数を数える。
       * @param bitboard 対象のビットボード。
       * @return 立っているビットボードの数。
       */
      static int CountBits(Bitboard bitboard) {
        return num_bit16_table_[bitboard & 0xffff]
        + num_bit16_table_[(bitboard >> 16) & 0xffff]
        + num_bit16_table_[(bitboard >> 32) & 0xffff]
        + num_bit16_table_[(bitboard >> 48) & 0xffff];
      }

      /**
       * 最下位で連続しているゼロビットの数を数える。
       * @param bitboard 対象のビットボード。
       * @最下位で連続しているゼロビットの数。
       */
      static int CountZero(Bitboard bitboard) {
        return CountBits((bitboard & (-bitboard)) - 1);
      }

      /**
       * ビットボードからマスを得る。 (A1に最も近いマス。)
       * @param bitboard 対象のビットボード。
       * @return マス。
       */
      static Square GetSquare(Bitboard bitboard) {
        return CountZero(bitboard);
      }

      /**
       * マス間の距離を得る。 (キングの歩数)
       * @param square_1 マス1。
       * @param square_1 マス2。
       * @return 距離。
       */
      static int GetDistance(Square square_1, Square square_2) {
        return distance_table_[square_1][square_2];
      }

      /**
       * 探索深さをヒストリー値にする。
       * @param depth 探索深さ。
       * @return ヒストリー値。
       */
      constexpr static int DepthToHistory(int depth) {
        return depth * depth;
      }

      /**
       * 逆サイドを得る。
       * @param side 元のサイド。
       * @return 逆サイド。
       */
      constexpr static Side GetOppositeSide(Side side) {
        return side ^ 0x3;
      }

      /**
       * 文字列を単語に切り分ける。
       * @param str 切り分ける文字列。
       * @param delim 区切り文字のセット。
       * @param delim_and_word 区切り文字のセット。 (単語として残す。)
       * @return 単語のベクトル。
       */
      template<class CharType>
      static std::vector<std::basic_string<CharType>> Split
      (const std::basic_string<CharType>& str,
      const std::set<CharType>& delim,
      const std::set<CharType>& delim_and_word);

      /**
       * MoveをPure Coordination Notationに変換。
       * @param move 変換したい手。
       * @return Pure Coordination Notation。
       */
      static std::string MoveToString(Move move) {
        // 準備。
        std::string ret = "";
        Square from = GetFrom(move);
        Square to = GetTo(move);
        PieceType promotion = GetPromotion(move);

        // 文字列を作成。
        ret.push_back(Util::SquareToFyle(from) + 'a');
        ret.push_back(Util::SquareToRank(from) + '1');
        ret.push_back(Util::SquareToFyle(to) + 'a');
        ret.push_back(Util::SquareToRank(to) + '1');
        switch (promotion) {
          case KNIGHT:
            ret.push_back('n');
            break;
          case BISHOP:
            ret.push_back('b');
            break;
          case ROOK:
            ret.push_back('r');
            break;
          case QUEEN:
            ret.push_back('q');
            break;
          default:
            break;
        }

        return ret;
      }

      /**
       * Pure Coordination NotationをMoveに変換。
       * @param str Pure Coordination Notation。
       * @return 変換後の手。
       */
      static Move StringToMove(const std::string& str) {
        if (str.size() < 4) return 0;

        Move ret = 0;

        // fromをパース。
        Fyle fyle = str[0] - 'a';
        if (fyle >= NUM_FYLES) return 0;
        Rank rank = str[1] - '1';
        if (rank >= NUM_RANKS) return 0;
        SetFrom(ret, CoordToSquare(fyle, rank));

        // toをパース。
        fyle = str[2] - 'a';
        if (fyle >= NUM_FYLES) return 0;
        rank = str[3] - '1';
        if (rank >= NUM_RANKS) return 0;
        SetTo(ret, CoordToSquare(fyle, rank));

        // 昇格をパース。
        if (str.size() >= 5) {
          switch (str[4]) {
            case 'n':
              SetPromotion(ret, KNIGHT);
              break;
            case 'b':
              SetPromotion(ret, BISHOP);
              break;
            case 'r':
              SetPromotion(ret, ROOK);
              break;
            case 'q':
              SetPromotion(ret, QUEEN);
              break;
            default:
              break;
          }
        }

        return ret;
      }

      // ランダムな数値を得る。
      static Hash GetRandomHash() {return dist_(engine_);}

      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 (削除) */
      Util() = delete;
      /** コピーコンストラクタ。 (削除) */
      Util(const Util&) = delete;
      /** ムーブコンストラクタ。 (削除) */
      Util (Util&&) = delete;
      /** コピー代入演算子。 (削除) */
      Util& operator=(const Util&) = delete;
      /** ムーブ代入演算子。 (削除) */
      Util& operator=(Util&&) = delete;
      /** デストラクタ。 */
      virtual ~Util();

    private:

      // ============================ //
      // ビットを数えるときに使うもの //
      // ============================ //
      /**
       * 下位16ビットのビットの数が入った配列。 [下位16ビット]
       */
      static std::uint8_t num_bit16_table_[0xffff + 1];
      /** num_bit16_table_[]を初期化する。 */
      static void InitNumBit16Table();

      // ========== //
      // マジック用 //
      // ========== //
      /** ブロッカー用8ビットマスク。 */
      static constexpr unsigned int BLOCKER_MAP = 0xff;
      /** 0度方向への攻撃の配列。 [マス][ブロッカーのパターン8ビット] */
      static Bitboard attack_table_0_[NUM_SQUARES][BLOCKER_MAP + 1];
      // 45度。
      /** 45度方向への攻撃の配列。 [マス][ブロッカーのパターン8ビット] */
      static Bitboard attack_table_45_[NUM_SQUARES][BLOCKER_MAP + 1];
      // 90度。
      /** 90度方向への攻撃の配列。 [マス][ブロッカーのパターン8ビット] */
      static Bitboard attack_table_90_[NUM_SQUARES][BLOCKER_MAP + 1];
      /** 135度方向への攻撃の配列。 [マス][ブロッカーのパターン8ビット] */
      static Bitboard attack_table_135_[NUM_SQUARES][BLOCKER_MAP + 1];
      /** ポーンの動ける位置の配列。
       *  [サイド][マス][ブロッカーのパターン8ビット]
       */
      static Bitboard pawn_movable_table_
      [NUM_SIDES][NUM_SQUARES][BLOCKER_MAP + 1];
      /** attack_table_???_[][]を初期化する。 */
      static void InitMagicTable();

      /**
       * 45度の座標を0度に逆変換する。
       * @param bitboard45 45度座標のビットボード。
       * @return 0度座標のビットボード。
       */
      static Bitboard Reverse45(Bitboard bitboard45);
      /**
       * 90度の座標を0度に逆変換する。
       * @param bitboard90 90度座標のビットボード。
       * @return 0度座標のビットボード。
       */
      static Bitboard Reverse90(Bitboard bitboard90);
      /**
       * 135度の座標を0度に逆変換する。
       * @param bitboard135 135度座標のビットボード。
       * @return 0度座標のビットボード。
       */
      static Bitboard Reverse135(Bitboard bitboard135);

      // ================== //
      // ビットボードの配列 //
      // ================== //
      /** 直線の入った配列。 [端点][端点] */
      static Bitboard line_[NUM_SQUARES][NUM_SQUARES];
      /** 直線の入った配列。 (端点を除く。) [端点][端点] */
      static Bitboard between_[NUM_SQUARES][NUM_SQUARES];
      /** ポーンの通常の動きの配列。 [サイド][マス] */
      static Bitboard pawn_move_[NUM_SIDES][NUM_SQUARES];
      /** ポーンの2歩の動きの配列。 [サイド][マス] */
      static Bitboard pawn_2step_move_[NUM_SIDES][NUM_SQUARES];
      /** ポーンの攻撃筋の配列。 [サイド][マス] */
      static Bitboard pawn_attack_[NUM_SIDES][NUM_SQUARES];
      /** ナイトの動きの配列。 [マス] */
      static Bitboard knight_move_[NUM_SQUARES];
      /** ビショップの動きの配列。 [マス] */
      static Bitboard bishop_move_[NUM_SQUARES];
      /** ルークの動きの配列。 [マス] */
      static Bitboard rook_move_[NUM_SQUARES];
      /** クイーンの動きの配列。 [マス] */
      static Bitboard queen_move_[NUM_SQUARES];
      /** キングの動きの配列。 [マス] */
      static Bitboard king_move_[NUM_SQUARES];
      /** line_[][]を初期化する。 */
      static void InitLine();
      /** pawn_move_[][]を初期化する。 */
      static void InitPawnMove();
      /** pawn_2step_move_[][]を初期化する。 */
      static void InitPawn2StepMove();
      /** pawn_attack_[][]を初期化する。 */
      static void InitPawnAttack();
      /** knight_move_[]を初期化する。 */
      static void InitKnightMove();
      /** bishop_move_[]を初期化する。 */
      static void InitBishopMove();
      /** rook_move_[]を初期化する。 */
      static void InitRookMove();
      /** king_move_[]を初期化する。 */
      static void InitKingMove();

      // ============ //
      // その他の配列 //
      // ============ //
      /** マス間の距離の配列。 [マス1][マス2] */
      static int distance_table_[NUM_SQUARES][NUM_SQUARES];
      /** distance_table_[][]を初期化する。 */
      static void InitDistanceTable();

      // ================== //
      // その他のstatic変数 //
      // ================== //
      // --- ランダム用オブジェクト --- //
      /** メルセンヌツイスターエンジン。 */
      static std::mt19937 engine_;
      /** 線形分布。 */
      static std::uniform_int_distribution<Hash> dist_;
      /** ランダム用オブジェクトを初期化する。 */
      static void InitRandom();
  };
}  // namespace Sayuri

#endif
