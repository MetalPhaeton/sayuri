/* chess_util.h: チェスの便利ツール。
   copyright (c) 2011 石橋宏之利
 */

#ifndef CHESS_UTIL_H
#define CHESS_UTIL_H

#include <iostream>
#include "chess_def.h"

namespace Misaki {
  /********************************
   * チェスの便利ツールのクラス。 *
   ********************************/
  class ChessUtil {
    public:
      /*****************************
       * ChessUtilクラスの初期化。 *
       *****************************/
      static void InitChessUtil() {
        // num_bit16_array_[]を初期化する。
        InitNumBit16Array();
        // attack_array***_[][]を初期化する。
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
      /************************
       * ビットボードの配列。 *
       ************************/
      static const bitboard_t BIT[NUM_SQUARES];  // マスのビットボード。
      static const bitboard_t FYLE[NUM_FYLES];  // ファイルのビットボード。
      static const bitboard_t RANK[NUM_RANKS];  // ランクのビットボード。

      /**********************
       * 回転座標変換配列。 *
       **********************/
      // 変換。
      static const square_t ROT45[NUM_SQUARES];  //  通常から左に45度。
      static const square_t ROT90[NUM_SQUARES];  // 通常から左に90度。
      static const square_t ROT135[NUM_SQUARES];  // 通常から左に135度。
      // 逆変換。
      static const square_t R_ROT45[NUM_SQUARES];  // 左に45度から通常へ。
      static const square_t R_ROT90[NUM_SQUARES];  // 左に90度から通常へ。
      static const square_t R_ROT135[NUM_SQUARES];  // 左に135度から通常へ。

      /**************************************
       * 各種方向のビットボードを得る関数。 *
       **************************************/
      // 右のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右。
      static bitboard_t GetRightBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~FYLE[FYLE_H];

        return bitboard << 1;
      }
      // 左のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左。
      static bitboard_t GetLeftBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~FYLE[FYLE_A];

        return bitboard >> 1;
      }
      // 上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの上。
      static bitboard_t GetUpBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~RANK[RANK_8];

        return bitboard << 8;
      }
      // 下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの下。
      static bitboard_t GetDownBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~RANK[RANK_1];

        return bitboard >> 8;
      }
      // 右上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右上。
      static bitboard_t GetRightUpBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | FYLE[FYLE_H]);

        return bitboard << 9;
      }
      // 右下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右下。
      static bitboard_t GetRightDownBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | FYLE[FYLE_H]);

        return bitboard >> 7;
      }
      // 左上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左上。
      static bitboard_t GetLeftUpBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | FYLE[FYLE_A]);

        return bitboard << 7;
      }
      // 左下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左下。
      static bitboard_t GetLeftDownBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | FYLE[FYLE_A]);

        return bitboard >> 9;
      }
      // 右右上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右右上。
      static bitboard_t GetRightRightUpBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | FYLE[FYLE_H] | FYLE[FYLE_G]);

        return bitboard << 10;
      }
      // 右上上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右上上。
      static bitboard_t GetRightUpUpBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | RANK[RANK_7] | FYLE[FYLE_H]);

        return bitboard << 17;
      }
      // 右右下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右右下。
      static bitboard_t GetRightRightDownBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | FYLE[FYLE_H] | FYLE[FYLE_G]);

        return bitboard >> 6;
      }
      // 右下下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの右下下。
      static bitboard_t GetRightDownDownBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | RANK[RANK_2] | FYLE[FYLE_H]);

        return bitboard >> 15;
      }
      // 左左上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左左上。
      static bitboard_t GetLeftLeftUpBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | FYLE[FYLE_A] | FYLE[FYLE_B]);

        return bitboard << 6;
      }
      // 左上上のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左上上。
      static bitboard_t GetLeftUpUpBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_8] | RANK[RANK_7] | FYLE[FYLE_A]);

        return bitboard << 15;
      }
      // 左左下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左左下。
      static bitboard_t GetLeftLeftDownBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | FYLE[FYLE_A] | FYLE[FYLE_B]);

        return bitboard >> 10;
      }
      // 左下下のマスのビットボードを得る。
      // [引数]
      // bitboard: 対象のビットボード。
      // [戻り値]
      // 引数bitboardの左左下。
      static bitboard_t GetLeftDownDownBitboard(bitboard_t bitboard) {
        // 移動すると盤外になるマスは削除。
        bitboard &= ~(RANK[RANK_1] | RANK[RANK_2] | FYLE[FYLE_A]);

        return bitboard >> 17;
      }

      /**********************
       * 利き筋を得る関数。 *
       **********************/
      // 0度の利き筋を得る。
      // [引数]
      // square: 起点の位置。
      // blocker: 他の駒。0度の座標。
      // [戻り値]
      // 利き筋。
      static bitboard_t GetAttack0(square_t square, bitboard_t blocker0) {
        return attack_array0_[square]
        [(blocker0 >> magic_shift_v_[square]) & magic_mask_v_[square]];
      }
      // 45度の利き筋を得る。
      // [引数]
      // square: 起点の位置。
      // blocker: 他の駒。45度の座標。
      // [戻り値]
      // 利き筋。
      static bitboard_t GetAttack45(square_t square, bitboard_t blocker45) {
        return attack_array45_[square]
        [(blocker45 >> magic_shift_d_[ROT45[square]])
        & magic_mask_d_[ROT45[square]]];
      }
      // 90度の利き筋を得る。
      // [引数]
      // square: 起点の位置。
      // blocker: 他の駒。90度の座標。
      // [戻り値]
      // 利き筋。
      static bitboard_t GetAttack90(square_t square, bitboard_t blocker90) {
        return attack_array90_[square]
        [(blocker90 >> magic_shift_v_[ROT90[square]])
        & magic_mask_v_[ROT90[square]]];
      }
      // 135度の利き筋を得る。
      // [引数]
      // square: 起点の位置。135度の座標。
      // blocker: 他の駒。
      // [戻り値]
      // 利き筋。
      static bitboard_t GetAttack135(square_t square, bitboard_t blocker135) {
        return attack_array135_[square]
        [(blocker135 >> magic_shift_d_[ROT135[square]])
        & magic_mask_d_[ROT135[square]]];
      }

      /**************************
       * ビットボード作成関数。 *
       **************************/
      // point1からpoint2までの直線のビットボードを得る。
      //（point1、point2を含む。）
      // [引数]
      // point1: 端点1。
      // point2: 端点2。
      // [戻り値]
      // point1からpoint2までの直線のビットボード。
      static bitboard_t GetLine(square_t point1, square_t point2) {
        return line_[point1][point2];
      }
      // ポーンの通常の動きを得る。
      // [引数]
      // square: 位置。
      // side: どちらのサイドの動きか。
      // [戻り値]
      // ポーンの通常の動きのビットボード。
      static bitboard_t GetPawnMove(square_t square, side_t side) {
        return pawn_move_[side][square];
      }
      // ポーンの2歩の動きを得る。
      // [引数]
      // square: 位置。
      // side: どちらのサイドの動きか。
      // [戻り値]
      // ポーンの2歩の動きのビットボード。
      static bitboard_t GetPawn2StepMove(square_t square, side_t side) {
        return pawn_2step_move_[side][square];
      }
      // ポーンの攻撃の筋を得る。
      // [引数]
      // square: 位置。
      // side: どちらのサイドの動きか。
      // [戻り値]
      // ポーンの攻撃筋のビットボード。
      static bitboard_t GetPawnAttack(square_t square, side_t side) {
        return pawn_attack_[side][square];
      }
      // ナイトの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ナイトの動きのビットボード。
      static bitboard_t GetKnightMove(square_t square) {
        return knight_move_[square];
      }
      // ビショップの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ビショップの動きのビットボード。
      static bitboard_t GetBishopMove(square_t square) {
        return bishop_move_[square];
      }
      // ルークの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ルークの動きのビットボード。
      static bitboard_t GetRookMove(square_t square) {
        return rook_move_[square];
      }
      // クイーンの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // クイーンの動きのビットボード。
      static bitboard_t GetQueenMove(square_t square) {
        return bishop_move_[square] | rook_move_[square];
      }
      // キングの動きを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // キングの動きのビットボード。
      static bitboard_t GetKingMove(square_t square) {
        return king_move_[square];
      }

      /**********************
       * その他の便利関数。 *
       **********************/
      // 立っているビットの個数を数える。
      // [引数]
      // bitboard: ビットを数えたいビットボード。
      // [戻り値]
      // 立っているビットの個数。
      static int CountBits(bitboard_t bitboard) {
        return num_bit16_array_[bitboard & 0xffff]
        + num_bit16_array_[(bitboard >> 16) & 0xffff]
        + num_bit16_array_[(bitboard >> 32) & 0xffff]
        + num_bit16_array_[(bitboard >> 48) & 0xffff];
      }
      // 下位のゼロビットの個数を数える。
      // [引数]
      // bitboard: ゼロを数えたいビットボード。
      // [戻り値]
      // 下位のゼロビットの個数。
      static int CountZero(bitboard_t bitboard) {
        return CountBits((bitboard & (-bitboard)) - 1);
      }
      // ビットボードからマスの位置を得る。
      // [引数]
      // bitboard: マスの位置を得たいビットボード。
      // [戻り値]
      // マスの位置。
      static square_t GetSquare(bitboard_t bitboard) {
        return static_cast<square_t>(CountZero(bitboard));
      }
      // マスの位置からファイルを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // その位置のファイル。
      static fyle_t GetFyle(square_t square) {
        return static_cast<fyle_t>(square & 0x7);
      }
      // マスの位置からランクを得る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // その位置のランク。
      static rank_t GetRank(square_t square) {
        return static_cast<rank_t>(square >> 3);
      }

    private:
      /**********************************
       * インスタンス生成、継承の禁止。 *
       **********************************/
      ChessUtil();  // 削除。
      ChessUtil(const ChessUtil&);  // 削除。
      ChessUtil& operator=(const ChessUtil&);  // 削除。
      virtual ~ChessUtil();  // 削除。

      /**********************************
       * ビットを数えるときに使うもの。 *
       **********************************/
      enum {
        BIT16_PATTERN = 0xffff + 1
      };
      // 16ビットのビットの個数が入った配列。
      // 引数には16ビットのパターンを入れる。
      static int num_bit16_array_[BIT16_PATTERN];
      // num_bit16_array_[]を初期化する。
      static void InitNumBit16Array();
      // ビットを数える。
      // [引数]
      // bits: 数える16ビット。
      // [戻り値]
      // ビットの個数。
      static int GetNumBits(unsigned int bits);

      /**************
       * マジック。 *
       **************/
      // マジックのシフト。0度と90度用。
      static const int magic_shift_v_[NUM_SQUARES];
      // マジックのシフト。45度と135度用。
      static const int magic_shift_d_[NUM_SQUARES];
      // マジックのマスク。0度と90度用。
      static const bitboard_t magic_mask_v_[NUM_SQUARES];
      // マジックのマスク。45度と135度用。
      static const bitboard_t magic_mask_d_[NUM_SQUARES];
      // 各方向への攻撃の配列。
      enum {
        // ブロッカーのパターン。
        BLOCKER_MAP = 0xff + 1
      };
      static bitboard_t attack_array0_[NUM_SQUARES][BLOCKER_MAP];  // 0度。
      static bitboard_t attack_array45_[NUM_SQUARES][BLOCKER_MAP];  // 45度。
      static bitboard_t attack_array90_[NUM_SQUARES][BLOCKER_MAP];  // 90度。
      static bitboard_t attack_array135_[NUM_SQUARES][BLOCKER_MAP];  // 135度。
      // attack_array***_[][]を初期化する。
      static void InitAttackArray();
      // 45度座標のビットボードを通常の座標に戻す。
      // [引数]
      // bitboard45: 45度座標のビットボード。
      // [戻り値]
      // 通常の座標のビットボード。
      static bitboard_t Reverse45(bitboard_t bitboard45);
      // 90度座標のビットボードを通常の座標に戻す。
      // [引数]
      // bitboard90: 90度座標のビットボード。
      // [戻り値]
      // 通常の座標のビットボード。
      static bitboard_t Reverse90(bitboard_t bitboard90);
      // 135度座標のビットボードを通常の座標に戻す。
      // [引数]
      // bitboard135: 135度座標のビットボード。
      // [戻り値]
      // 通常の座標のビットボード。
      static bitboard_t Reverse135(bitboard_t bitboard135);

      /************************
       * ビットボードの配列。 *
       ************************/
      // 直線の入った配列。
      static bitboard_t line_[NUM_SQUARES][NUM_SQUARES];
      // ポーンの通常の動きの配列。
      static bitboard_t pawn_move_[NUM_SIDES][NUM_SQUARES];
      // ポーンの2歩の動きの配列。
      static bitboard_t pawn_2step_move_[NUM_SIDES][NUM_SQUARES];
      // ポーンの攻撃筋の配列。
      static bitboard_t pawn_attack_[NUM_SIDES][NUM_SQUARES];
      // ナイトの動きの配列。
      static bitboard_t knight_move_[NUM_SQUARES];
      // ビショップの動きの配列。
      static bitboard_t bishop_move_[NUM_SQUARES];
      // ルークの動きの配列。
      static bitboard_t rook_move_[NUM_SQUARES];
      // キングの動きの配列。
      static bitboard_t king_move_[NUM_SQUARES];
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
}  // Misaki

#endif
