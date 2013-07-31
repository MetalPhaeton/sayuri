/*
   chess_util.h: チェスの便利ツール。

   The MIT License (MIT)

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

#ifndef CHESS_UTIL_H
#define CHESS_UTIL_H

#include <iostream>

#include "chess_def.h"

namespace Sayuri {
  /********************************/
  /* チェスの便利ツールのクラス。 */
  /********************************/
  class Util {
    public:
      /************************/
      /* Utilクラスの初期化。 */
      /************************/
      static void InitUtil() {
        // num_bit16_table_[]を初期化する。
        InitNumBit16Array();
        // attack_table_***_[][]を初期化する。
        InitAttackArray();
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
        // king_move_[]を初期化する。
        InitKingMove();
      }
      /************************/
      /* ビットボードの配列。 */
      /************************/
      // マスのビットボード。
      static constexpr Bitboard BIT[NUM_SQUARES] {
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
      // ファイルのビットボード。
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
      // ランクのビットボード。
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

      /**********************/
      /* 回転座標変換配列。 */
      /**********************/
      // 変換。
      //  通常から左に45度。
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
      // 通常から左に90度。
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
      // 通常から左に135度。
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
      // 逆変換。
      // 左に45度から通常へ。
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
      // 左に90度から通常へ。
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
      // 左に135度から通常へ。
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

      /**************************************/
      /* 各種方向のビットボードを得る関数。 */
      /**************************************/
      // 右のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右。
      static Bitboard GetRightBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~FYLE[FYLE_H];

        return bitboard << 1;
      }
      // 左のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左。
      static Bitboard GetLeftBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~FYLE[FYLE_A];

        return bitboard >> 1;
      }
      // 上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの上。
      static Bitboard GetUpBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~RANK[RANK_8];

        return bitboard << 8;
      }
      // 下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの下。
      static Bitboard GetDownBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~RANK[RANK_1];

        return bitboard >> 8;
      }
      // 右上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右上。
      static Bitboard GetRightUpBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | FYLE[FYLE_H]);

        return bitboard << 9;
      }
      // 右下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右下。
      static Bitboard GetRightDownBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | FYLE[FYLE_H]);

        return bitboard >> 7;
      }
      // 左上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左上。
      static Bitboard GetLeftUpBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | FYLE[FYLE_A]);

        return bitboard << 7;
      }
      // 左下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左下。
      static Bitboard GetLeftDownBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | FYLE[FYLE_A]);

        return bitboard >> 9;
      }
      // 右右上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右右上。
      static Bitboard GetRightRightUpBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | FYLE[FYLE_H] | FYLE[FYLE_G]);

        return bitboard << 10;
      }
      // 右上上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右上上。
      static Bitboard GetRightUpUpBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | RANK[RANK_7] | FYLE[FYLE_H]);

        return bitboard << 17;
      }
      // 右右下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右右下。
      static Bitboard GetRightRightDownBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | FYLE[FYLE_H] | FYLE[FYLE_G]);

        return bitboard >> 6;
      }
      // 右下下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右下下。
      static Bitboard GetRightDownDownBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | RANK[RANK_2] | FYLE[FYLE_H]);

        return bitboard >> 15;
      }
      // 左左上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左左上。
      static Bitboard GetLeftLeftUpBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | FYLE[FYLE_A] | FYLE[FYLE_B]);

        return bitboard << 6;
      }
      // 左上上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左上上。
      static Bitboard GetLeftUpUpBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | RANK[RANK_7] | FYLE[FYLE_A]);

        return bitboard << 15;
      }
      // 左左下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左左下。
      static Bitboard GetLeftLeftDownBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | FYLE[FYLE_A] | FYLE[FYLE_B]);

        return bitboard >> 10;
      }
      // 左下下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左左下。
      static Bitboard GetLeftDownDownBitboard(Bitboard bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | RANK[RANK_2] | FYLE[FYLE_A]);

        return bitboard >> 17;
      }

      /**********************/
      /* 利き筋を得る関数。 */
      /**********************/
      // 0度の利き筋を得る。
      // [引数]
      // square: 起点の位置。
      // blocker: 他の駒。0度の座標。
      // [戻り値]
      // 利き筋。
      static Bitboard GetAttack0(Square square, Bitboard blocker0) {
        return attack_table_0_[square]
        [(blocker0 >> MAGIC_SHIFT_V[square]) & MAGIC_MASK_V[square]];
      }
      // 45度の利き筋を得る。
      // [引数]
      // square: 起点の位置。
      // blocker: 他の駒。45度の座標。
      // [戻り値]
      // 利き筋。
      static Bitboard GetAttack45(Square square, Bitboard blocker45) {
        return attack_table_45_[square]
        [(blocker45 >> MAGIC_SHIFT_D[ROT45[square]])
        & MAGIC_MASK_D[ROT45[square]]];
      }
      // 90度の利き筋を得る。
      // [引数]
      // square: 起点の位置。
      // blocker: 他の駒。90度の座標。
      // [戻り値]
      // 利き筋。
      static Bitboard GetAttack90(Square square, Bitboard blocker90) {
        return attack_table_90_[square]
        [(blocker90 >> MAGIC_SHIFT_V[ROT90[square]])
        & MAGIC_MASK_V[ROT90[square]]];
      }
      // 135度の利き筋を得る。
      // [引数]
      // square: 起点の位置。135度の座標。
      // blocker: 他の駒。
      // [戻り値]
      // 利き筋。
      static Bitboard GetAttack135(Square square, Bitboard blocker135) {
        return attack_table_135_[square]
        [(blocker135 >> MAGIC_SHIFT_D[ROT135[square]])
        & MAGIC_MASK_D[ROT135[square]]];
      }

      /**************************/
      /* ビットボード作成関数。 */
      /**************************/
      // point1からpoint2までの直線のビットボードを得る。
      //（point1、point2を含む。）
      // [引数]
      // point1: 端点1。
      // point2: 端点2。
      // [戻り値]
      // point1からpoint2までの直線のビットボード。
      static Bitboard GetLine(Square point1, Square point2) {
        return line_[point1][point2];
      }
      // ポーンの通常の動きを得る。
      // [引数]
      // square: 位置。
      // side: どちらのサイドの動きか。
      // [戻り値]
      // ポーンの通常の動きのビットボード。
      static Bitboard GetPawnMove(Square square, Side side) {
        return pawn_move_[side][square];
      }
      // ポーンの2歩の動きを得る。
      // [引数]
      // square: 位置。
      // side: どちらのサイドの動きか。
      // [戻り値]
      // ポーンの2歩の動きのビットボード。
      static Bitboard GetPawn2StepMove(Square square, Side side) {
        return pawn_2step_move_[side][square];
      }
      // ポーンの攻撃の筋を得る。
      // [引数]
      // square: 位置。
      // side: どちらのサイドの動きか。
      // [戻り値]
      // ポーンの攻撃筋のビットボード。
      static Bitboard GetPawnAttack(Square square, Side side) {
        return pawn_attack_[side][square];
      }
      // ナイトの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ナイトの動きのビットボード。
      static Bitboard GetKnightMove(Square square) {
        return knight_move_[square];
      }
      // ビショップの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ビショップの動きのビットボード。
      static Bitboard GetBishopMove(Square square) {
        return bishop_move_[square];
      }
      // ルークの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ルークの動きのビットボード。
      static Bitboard GetRookMove(Square square) {
        return rook_move_[square];
      }
      // クイーンの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // クイーンの動きのビットボード。
      static Bitboard GetQueenMove(Square square) {
        return bishop_move_[square] | rook_move_[square];
      }
      // キングの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // キングの動きのビットボード。
      static Bitboard GetKingMove(Square square) {
        return king_move_[square];
      }

      /**********************/
      /* その他の便利関数。 */
      /**********************/
      // 立っているビットの個数を数える。
      // [引数]
      // bitboard: ビットを数えたいビットボード。
      // [戻り値]
      // 立っているビットの個数。
      static int CountBits(Bitboard bitboard) {
        return num_bit16_table_[bitboard & 0xffff]
        + num_bit16_table_[(bitboard >> 16) & 0xffff]
        + num_bit16_table_[(bitboard >> 32) & 0xffff]
        + num_bit16_table_[(bitboard >> 48) & 0xffff];
      }
      // 下位のゼロビットの個数を数える。
      // [引数]
      // bitboard: ゼロを数えたいビットボード。
      // [戻り値]
      // 下位のゼロビットの個数。
      static int CountZero(Bitboard bitboard) {
        return CountBits((bitboard & (-bitboard)) - 1);
      }
      // ビットボードからマスの位置を得る。
      // [引数]
      // bitboard: マスの位置を得たいビットボード。
      // [戻り値]
      // マスの位置。
      static Square GetSquare(Bitboard bitboard) {
        return CountZero(bitboard);
      }
      // マスの位置からファイルを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // その位置のファイル。
      static Fyle GetFyle(Square square) {
        return square & 0x7;
      }
      // マスの位置からランクを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // その位置のランク。
      static Rank GetRank(Square square) {
        return square >> 3;
      }

      /********************/
      /* コンストラクタ。 */
      /********************/
      Util() = delete;
      Util(const Util&) = delete;
      Util (Util&&) = delete;
      Util& operator=(const Util&) = delete;
      Util& operator=(Util&&) = delete;
      virtual ~Util();

    private:

      /**********************************/
      /* ビットを数えるときに使うもの。 */
      /**********************************/
      // 16ビットのビットの個数が入った配列。
      // 引数には16ビットのパターンを入れる。
      static int num_bit16_table_[0xffff + 1];
      // num_bit16_table_[]を初期化する。
      static void InitNumBit16Array();

      /**************/
      /* マジック。 */
      /**************/
      // マジックのシフト。0度と90度用。
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
      // マジックのシフト。45度と135度用。
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
      // マジックのマスク。0度と90度用。
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
      // マジックのマスク。45度と135度用。
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
      // 各方向への攻撃の配列。
      // ブロッカーのパターン。
      static constexpr int BLOCKER_MAP = 0xff;
      // 0度。
      static Bitboard attack_table_0_[NUM_SQUARES][BLOCKER_MAP + 1];
      // 45度。
      static Bitboard attack_table_45_[NUM_SQUARES][BLOCKER_MAP + 1];
      // 90度。
      static Bitboard attack_table_90_[NUM_SQUARES][BLOCKER_MAP + 1];
      // 135度。
      static Bitboard attack_table_135_[NUM_SQUARES][BLOCKER_MAP + 1];
      // attack_table_***_[][]を初期化する。
      static void InitAttackArray();
      // 45度座標のビットボードを通常の座標に戻す。
      // [引数]
      // bitboard45: 45度座標のビットボード。
      // [戻り値]
      // 通常の座標のビットボード。
      static Bitboard Reverse45(Bitboard bitboard45);
      // 90度座標のビットボードを通常の座標に戻す。
      // [引数]
      // bitboard90: 90度座標のビットボード。
      // [戻り値]
      // 通常の座標のビットボード。
      static Bitboard Reverse90(Bitboard bitboard90);
      // 135度座標のビットボードを通常の座標に戻す。
      // [引数]
      // bitboard135: 135度座標のビットボード。
      // [戻り値]
      // 通常の座標のビットボード。
      static Bitboard Reverse135(Bitboard bitboard135);

      /************************/
      /* ビットボードの配列。 */
      /************************/
      // 直線の入った配列。
      static Bitboard line_[NUM_SQUARES][NUM_SQUARES];
      // ポーンの通常の動きの配列。
      static Bitboard pawn_move_[NUM_SIDES][NUM_SQUARES];
      // ポーンの2歩の動きの配列。
      static Bitboard pawn_2step_move_[NUM_SIDES][NUM_SQUARES];
      // ポーンの攻撃筋の配列。
      static Bitboard pawn_attack_[NUM_SIDES][NUM_SQUARES];
      // ナイトの動きの配列。
      static Bitboard knight_move_[NUM_SQUARES];
      // ビショップの動きの配列。
      static Bitboard bishop_move_[NUM_SQUARES];
      // ルークの動きの配列。
      static Bitboard rook_move_[NUM_SQUARES];
      // キングの動きの配列。
      static Bitboard king_move_[NUM_SQUARES];
      // line_[][]を初期化する。
      static void InitLine();
      // pawn_move_[][]を初期化する。
      static void InitPawnMove();
      // pawn_2step_move_[][]を初期化する。
      static void InitPawn2StepMove();
      // pawn_attack_[][]を初期化する。
      static void InitPawnAttack();
      // knight_move_[]を初期化する。
      static void InitKnightMove();
      // bishop_move_[]を初期化する。
      static void InitBishopMove();
      // rook_move_[]を初期化する。
      static void InitRookMove();
      // king_move_[]を初期化する。
      static void InitKingMove();
  };
}  // namespace Sayuri

#endif
