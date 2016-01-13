#! /usr/bin/env python3

# The MIT License (MIT)
#
# Copyright (c) 2016 Hironori Ishibashi
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

def CountBits(bits):
    count = 0
    while bits != 0:
        count += 1
        bits &= bits - 1
    return count
def SquareToFyle(square): return square & 0x7
def SquareToRank(square): return square >> 3
def CoordToSquare(fyle, rank): return (rank << 3) | fyle
def SquareToBB(square): return 0x1 << square
def CoordToBB(fyle, rank): return SquareToBB(CoordToSquare(fyle, rank))
def GenLineBB(square1, square2):
    fyle1 = SquareToFyle(square1)
    fyle2 = SquareToFyle(square2)
    rank1 = SquareToRank(square1)
    rank2 = SquareToRank(square2)
    ret = 0
    if fyle1 == fyle2:
        ret = SquareToBB(square1) | SquareToBB(square2)
        inc = 1 if rank2 > rank1 else -1
        while rank1 != rank2:
            ret |= CoordToBB(fyle1, rank1)
            rank1 += inc
    elif rank1 == rank2:
        ret = SquareToBB(square1) | SquareToBB(square2)
        inc = 1 if fyle2 > fyle1 else -1
        while fyle1 != fyle2:
            ret |= CoordToBB(fyle1, rank1)
            fyle1 += inc
    elif abs(fyle2 - fyle1) == abs(rank2 - rank1):
        ret = SquareToBB(square1) | SquareToBB(square2)
        fyle_inc = 1 if fyle2 > fyle1 else -1
        rank_inc = 1 if rank2 > rank1 else -1
        while (fyle1 != fyle2) and (rank1 != rank2):
            ret |= CoordToBB(fyle1, rank1)
            fyle1 += fyle_inc
            rank1 += rank_inc
    return ret
def GenDistance(square1, square2):
    fyle1 = SquareToFyle(square1)
    fyle2 = SquareToFyle(square2)
    rank1 = SquareToRank(square1)
    rank2 = SquareToRank(square2)
    return max((abs(fyle1 - fyle2), abs(rank1 - rank2)))
def GenIsEnPassant(e_square, to):
    if e_square == to:
        if (SquareToRank(e_square) == 2) or (SquareToRank(e_square) == 5):
            return "true"
    return "false"
def GenIs2StepMove(src, dst):
    if SquareToRank(src) == 1:
        if (dst - src) == 16: return "true"
    if SquareToRank(src) == 6:
        if (src - dst) == 16: return "true"
    return "false"
ROT = [\
    [\
         0,  1,  2,  3,  4,  5,  6,  7,\
         8,  9, 10, 11, 12, 13, 14, 15,\
        16, 17, 18, 19, 20, 21, 22, 23,\
        24, 25, 26, 27, 28, 29, 30, 31,\
        32, 33, 34, 35, 36, 37, 38, 39,\
        40, 41, 42, 43, 44, 45, 46, 47,\
        48, 49, 50, 51, 52, 53, 54, 55,\
        56, 57, 58, 59, 60, 61, 62, 63\
    ],\
    [\
        28, 21, 15, 10,  6,  3,  1,  0,\
        36, 29, 22, 16, 11,  7,  4,  2,\
        43, 37, 30, 23, 17, 12,  8,  5,\
        49, 44, 38, 31, 24, 18, 13,  9,\
        54, 50, 45, 39, 32, 25, 19, 14,\
        58, 55, 51, 46, 40, 33, 26, 20,\
        61, 59, 56, 52, 47, 41, 34, 27,\
        63, 62, 60, 57, 53, 48, 42, 35\
    ],\
    [\
         7, 15, 23, 31, 39, 47, 55, 63,\
         6, 14, 22, 30, 38, 46, 54, 62,\
         5, 13, 21, 29, 37, 45, 53, 61,\
         4, 12, 20, 28, 36, 44, 52, 60,\
         3, 11, 19, 27, 35, 43, 51, 59,\
         2, 10, 18, 26, 34, 42, 50, 58,\
         1,  9, 17, 25, 33, 41, 49, 57,\
         0,  8, 16, 24, 32, 40, 48, 56\
    ],\
    [\
         0,  2,  5,  9, 14, 20, 27, 35,\
         1,  4,  8, 13, 19, 26, 34, 42,\
         3,  7, 12, 18, 25, 33, 41, 48,\
         6, 11, 17, 24, 32, 40, 47, 53,\
        10, 16, 23, 31, 39, 46, 52, 57,\
        15, 22, 30, 38, 45, 51, 56, 60,\
        21, 29, 37, 44, 50, 55, 59, 62,\
        28, 36, 43, 49, 54, 58, 61, 63\
    ]\
]
R_ROT = [
    [\
         0,  1,  2,  3,  4,  5,  6,  7,\
         8,  9, 10, 11, 12, 13, 14, 15,\
        16, 17, 18, 19, 20, 21, 22, 23,\
        24, 25, 26, 27, 28, 29, 30, 31,\
        32, 33, 34, 35, 36, 37, 38, 39,\
        40, 41, 42, 43, 44, 45, 46, 47,\
        48, 49, 50, 51, 52, 53, 54, 55,\
        56, 57, 58, 59, 60, 61, 62, 63\
    ],\
    [\
         7,\
         6, 15,\
         5, 14, 23,\
         4, 13, 22, 31,\
         3, 12, 21, 30, 39,\
         2, 11, 20, 29, 38, 47,\
         1, 10, 19, 28, 37, 46, 55,\
         0,  9, 18, 27, 36, 45, 54, 63,\
         8, 17, 26, 35, 44, 53, 62,\
        16, 25, 34, 43, 52, 61,\
        24, 33, 42, 51, 60,\
        32, 41, 50, 59,\
        40, 49, 58,\
        48, 57,\
        56\
    ],\
    [\
        56, 48, 40, 32, 24, 16,  8,  0,
        57, 49, 41, 33, 25, 17,  9,  1,
        58, 50, 42, 34, 26, 18, 10,  2,
        59, 51, 43, 35, 27, 19, 11,  3,
        60, 52, 44, 36, 28, 20, 12,  4,
        61, 53, 45, 37, 29, 21, 13,  5,
        62, 54, 46, 38, 30, 22, 14,  6,
        63, 55, 47, 39, 31, 23, 15,  7
    ],\
    [\
         0,\
         8,  1,\
        16,  9,  2,\
        24, 17, 10,  3,\
        32, 25, 18, 11,  4,\
        40, 33, 26, 19, 12,  5,\
        48, 41, 34, 27, 20, 13,  6,\
        56, 49, 42, 35, 28, 21, 14,  7,\
        57, 50, 43, 36, 29, 22, 15,\
        58, 51, 44, 37, 30, 23,\
        59, 52, 45, 38, 31,\
        60, 53, 46, 39,\
        61, 54, 47,\
        62, 55,\
        63\
    ]\
]
MAGIC_SHIFT_V = [\
     0,  0,  0,  0,  0,  0,  0,  0,\
     8,  8,  8,  8,  8,  8,  8,  8,\
    16, 16, 16, 16, 16, 16, 16, 16,\
    24, 24, 24, 24, 24, 24, 24, 24,\
    32, 32, 32, 32, 32, 32, 32, 32,\
    40, 40, 40, 40, 40, 40, 40, 40,\
    48, 48, 48, 48, 48, 48, 48, 48,\
    56, 56, 56, 56, 56, 56, 56, 56\
]
MAGIC_SHIFT_D = [\
     0,\
     1,  1,\
     3,  3,  3,\
     6,  6,  6,  6,\
    10, 10, 10, 10, 10,\
    15, 15, 15, 15, 15, 15,\
    21, 21, 21, 21, 21, 21, 21,\
    28, 28, 28, 28, 28, 28, 28, 28,\
    36, 36, 36, 36, 36, 36, 36,\
    43, 43, 43, 43, 43, 43,\
    49, 49, 49, 49, 49,\
    54, 54, 54, 54,\
    58, 58, 58,\
    61, 61,\
    63\
]
MAGIC_SHIFT = []
for square in range(64):
    MAGIC_SHIFT += [[\
        MAGIC_SHIFT_V[ROT[0][square]], MAGIC_SHIFT_D[ROT[1][square]], \
        MAGIC_SHIFT_V[ROT[2][square]], MAGIC_SHIFT_D[ROT[3][square]]\
    ]]
MAGIC_MASK_V = [\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff\
]
MAGIC_MASK_D = [\
    0x1,\
    0x3, 0x3,\
    0x7, 0x7, 0x7,\
    0xf, 0xf, 0xf, 0xf,\
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f,\
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,\
    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,\
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,\
    0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,\
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,\
    0x1f, 0x1f, 0x1f, 0x1f, 0x1f,\
    0xf, 0xf, 0xf, 0xf,\
    0x7, 0x7, 0x7,\
    0x3, 0x3,\
    0x1\
]
MAGIC_MASK = []
for square in range(64):
    MAGIC_MASK += [[\
        MAGIC_MASK_V[ROT[0][square]], MAGIC_MASK_D[ROT[1][square]], \
        MAGIC_MASK_V[ROT[2][square]], MAGIC_MASK_D[ROT[3][square]]\
    ]]

def BBToSquare(bb): return CountBits((bb & (-bb)) - 1)
def GetPoint(square, rot):
    return (SquareToBB(ROT[rot][square]) >> MAGIC_SHIFT[square][rot]) \
    & MAGIC_MASK[square][rot]
def ToBitboard(bit, square, rot):
    return SquareToBB(R_ROT[rot][BBToSquare(bit\
    << MAGIC_SHIFT[square][rot])])
def GenAttackMagic(square, pattern):
    ret0 = 0
    ret45 = 0
    ret90 = 0
    ret135 = 0
    bit0 = GetPoint(square, 0)
    bit45 = GetPoint(square, 1)
    bit90 = GetPoint(square, 2)
    bit135 = GetPoint(square, 3)
    for i in range(8):
        bit0 = (bit0 << 1) & MAGIC_MASK[square][0]
        bit45 = (bit45 << 1) & MAGIC_MASK[square][1]
        bit90 = (bit90 << 1) & MAGIC_MASK[square][2]
        bit135 = (bit135 << 1) & MAGIC_MASK[square][3]
        if bit0 != 0:
            ret0 |= ToBitboard(bit0, square, 0)
            if (bit0 & pattern) != 0: bit0 = 0
        if bit45 != 0:
            ret45 |= ToBitboard(bit45, square, 1)
            if (bit45 & pattern) != 0: bit45 = 0
        if bit90 != 0:
            ret90 |= ToBitboard(bit90, square, 2)
            if (bit90 & pattern) != 0: bit90 = 0
        if bit135 != 0:
            ret135 |= ToBitboard(bit135, square, 3)
            if (bit135 & pattern) != 0: bit135 = 0
    bit0 = GetPoint(square, 0)
    bit45 = GetPoint(square, 1)
    bit90 = GetPoint(square, 2)
    bit135 = GetPoint(square, 3)
    for i in range(8):
        bit0 = (bit0 >> 1) & MAGIC_MASK[square][0]
        bit45 = (bit45 >> 1) & MAGIC_MASK[square][1]
        bit90 = (bit90 >> 1) & MAGIC_MASK[square][2]
        bit135 = (bit135 >> 1) & MAGIC_MASK[square][3]
        if bit0 != 0:
            ret0 |= ToBitboard(bit0, square, 0)
            if (bit0 & pattern) != 0: bit0 = 0
        if bit45 != 0:
            ret45 |= ToBitboard(bit45, square, 1)
            if (bit45 & pattern) != 0: bit45 = 0
        if bit90 != 0:
            ret90 |= ToBitboard(bit90, square, 2)
            if (bit90 & pattern) != 0: bit90 = 0
        if bit135 != 0:
            ret135 |= ToBitboard(bit135, square, 3)
            if (bit135 & pattern) != 0: bit135 = 0
    return "{" + hex(ret0) + "ULL," + hex(ret45) + "ULL," \
    + hex(ret90) + "ULL," + hex(ret135) + "ULL}"

def GenPawnMovable(side, square, pattern):
    if side == 0:
        return "0x0ULL"

    ret = 0
    rank = SquareToRank(square)
    if side == 1:
        if rank == 1:
            if (GetPoint(square + 8, 2) & pattern) == 0:
                ret |= SquareToBB(square + 8)
                if (GetPoint(square + 16, 2)& pattern) == 0:
                    ret |= SquareToBB(square + 16)
            return hex(ret) + "ULL"
        elif rank == 7:
            return "0x0ULL"
        else:
            if (GetPoint(square + 8, 2)& pattern) == 0:
                ret |= SquareToBB(square + 8)
            return hex(ret) + "ULL"
    elif side == 2:
        if rank == 6:
            if (GetPoint(square - 8, 2) & pattern) == 0:
                ret |= SquareToBB(square - 8)
                if (GetPoint(square - 16, 2) & pattern) == 0:
                    ret |= SquareToBB(square - 16)
            return hex(ret) + "ULL"
        elif rank == 0:
            return "0x0ULL"
        else:
            if (GetPoint(square - 8, 2)& pattern) == 0:
                ret |= SquareToBB(square - 8)
            return hex(ret) + "ULL"

    return "0x0ULL"


if __name__ == "__main__":
    frame = """\
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
 * @file chess_util_extra.h
 * @author Hironori Ishibashi
 * @brief chess_util.h chess_util.cppで使うテーブル。 (Pythonで生成)
 */

#ifndef CHESS_UTIL_EXTRA_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define CHESS_UTIL_EXTRA_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

namespace Sayuri {{
  namespace MetaUtil {{
    /** CountBits()で使う、立っているビットの数のテーブル。 */
    constexpr char NUM_BIT16_TABLE[0xffff + 1] {{
      {0}
    }};
 
    /** 直線のビットボード。 */
    constexpr Bitboard LINE[NUM_SQUARES][NUM_SQUARES] {{
      {1}
    }};
 
    /** 直線のビットボード。 端点なし。 */
    constexpr Bitboard BETWEEN[NUM_SQUARES][NUM_SQUARES] {{
      {2}
    }};
 
    /** 距離のテーブル。 */
    constexpr int DISTANCE[NUM_SQUARES][NUM_SQUARES] {{
      {3}
    }};
 
    /** アンパッサン判定テーブル。 */
    constexpr bool IS_EN_PASSANT[NUM_SQUARES][NUM_SQUARES] {{
      {4}
    }};
 
    /** ポーンの2歩の動き判定テーブル。 */
    constexpr bool IS_2STEP_MOVE[NUM_SQUARES][NUM_SQUARES] {{
      {5}
    }};

    /** ライン攻撃のマジック用テーブル。 */
    constexpr Bitboard ATTACK_TABLE[NUM_SQUARES][0xff + 1][NUM_ROTS] {{
      {6}
    }};

    /** ポーンの動きのマジック用テーブル。 */
    constexpr Bitboard PAWN_MOVABLE_TABLE[NUM_SIDES][NUM_SQUARES][0xff + 1] {{
      {7}
    }};
  }}
}}  // namespace Sayuri

#endif"""

    # NUM_BIT16_TABLE
    elms0 = ""
    for num in range(0xffff + 1): elms0 += str(CountBits(num)) + ","
    elms0 = elms0[:-1]

    # LINE BETWEEN DISTANCE IS_EN_PASSANT IS_2STEP_MOVE
    elms1 = ""
    elms2 = ""
    elms3 = ""
    elms4 = ""
    elms5 = ""
    for square1 in range(64):
        elms1 += "{"
        elms2 += "{"
        elms3 += "{"
        elms4 += "{"
        elms5 += "{"
        for square2 in range(64):
            elms1 += hex(GenLineBB(square1, square2)) + ","
            elms2 += hex(GenLineBB(square1, square2) \
            & ~(SquareToBB(square1) | SquareToBB(square2))) + ","
            elms3 += str(GenDistance(square1, square2)) + ","
            elms4 += GenIsEnPassant(square1, square2) + ","
            elms5 += GenIs2StepMove(square1, square2) + ","
        elms1 = elms1[:-1]
        elms2 = elms2[:-1]
        elms3 = elms3[:-1]
        elms4 = elms4[:-1]
        elms5 = elms5[:-1]
        elms1 += "},"
        elms2 += "},"
        elms3 += "},"
        elms4 += "},"
        elms5 += "},"
    elms1 = elms1[:-1]
    elms2 = elms2[:-1]
    elms3 = elms3[:-1]
    elms4 = elms4[:-1]
    elms5 = elms5[:-1]

    # ATTACK_TABLE
    elms6 = ""
    for square in range(64):
        elms6 += "{"
        for pattern in range(0xff + 1):
            elms6 += GenAttackMagic(square, pattern) + ","
        elms6 = elms6[:-1]
        elms6 += "},"
    elms6 = elms6[:-1]

    # PAWN_MOVABLE_TABLE
    elms7 = ""
    for side in range(3):
        elms7 += "{"
        for square in range(64):
            elms7 += "{"
            for pattern in range(0xff + 1):
                elms7 += GenPawnMovable(side, square, pattern) + ","
            elms7 = elms7[:-1]
            elms7 += "},"
        elms7 = elms7[:-1]
        elms7 += "},"
    elms7 = elms7[:-1]

    # 出力。
    print(frame.format(elms0, elms1, elms2, elms3, elms4, elms5, elms6, elms7))

