/* chess_engine.h: チェスボード。

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

#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include <iostream>
#include <vector>
#include <ctime>
#include "chess_def.h"
#include "chess_util.h"
#include "transposition_table.h"
#include "fen.h"
#include "move_maker.h"
#include "evaluator.h"

namespace Sayuri {
  class MoveMaker;
  class Evaluator;

  /****************************/
  /* チェスエンジンのクラス。 */
  /****************************/
  class ChessEngine {
    public:
      /******************/
      /* テスト用関数。 */
      /******************/
      void Test();
      void PrintEvaluator(Evaluator& eval);
      void PrintEvaluator2(Evaluator& eval);

      /*******************************/
      /* ChessEngineクラスの初期化。 */
      /*******************************/
      static void InitChessEngine() {
        // key_table_[][][]を初期化する。
        InitKeyTable();
      }


      /********************/
      /* コンストラクタ。 */
      /********************/
      ChessEngine();
      ChessEngine(const ChessEngine& engine);
      ChessEngine(ChessEngine&& engine);
      ChessEngine& operator=(const ChessEngine& engine);
      ChessEngine& operator=(ChessEngine&& engine);
      virtual ~ChessEngine();

      /********************/
      /* パブリック関数。 */
      /********************/
      // FENを読み込む。
      // [引数]
      // fen: 読み込むFenオブジェクト。
      void LoadFen(const Fen& fen);

      // 新しいゲームの準備をする。
      void SetNewGame();

    private:
      /**************/
      /* フレンド。 */
      /**************/
      friend class MoveMaker;
      friend class Evaluator;


      /****************/
      /* 駒を動かす。 */
      /****************/
      // 駒を動かす。
      // 動かす前のキャスリングの権利とアンパッサンは記録される。
      // 駒を取る場合は取った駒がmoveに記録される。
      // 動かす駒の位置と移動先の位置が同じ場合はNull Move。
      // そのmoveはUnmakeMove()で使われる。
      // [引数]
      // move[inout]: 動かす手。
      void MakeMove(Move& move);
      // MakeMove()で動かした駒を元に戻す。
      // 必ず先にMakeMove()をすること。
      // [引数]
      // move: MakeMove()で動かした手。
      void UnmakeMove(Move move);

      /******************************/
      /* その他のプライベート関数。 */
      /******************************/
      // 駒を置く。（駒の種類piece_typeにEMPTYをおけば、駒を削除できる。）
      // [引数]
      // square: 置きたい位置。
      // piece_type: 駒の種類。
      // side: 置きたい駒のサイド。
      void PutPiece(Square square, Piece piece_type, Side side=NO_SIDE);
      // 駒の位置を変える。
      // (注)移動先の駒は上書きされる。
      // [引数]
      // from: 変えたい駒の位置。
      // to: 移動先。
      void ReplacePiece(Square from, Square to);

      // ビショップの攻撃筋を作る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ビショップの攻撃筋。
      Bitboard GetBishopAttack(Square square) const {
        return Util::GetAttack45(square, blocker_45_)
        | Util::GetAttack135(square, blocker_135_);
      }
      // ルークの攻撃筋を作る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ルークの攻撃筋。
      Bitboard GetRookAttack(Square square) const {
        return Util::GetAttack0(square, blocker_0_)
        | Util::GetAttack90(square, blocker_90_);
      }
      // クイーンの攻撃筋を作る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // クイーンの攻撃筋。
      Bitboard GetQueenAttack(Square square) const {
        return GetBishopAttack(square) | GetRookAttack(square);
      }

      // キャスリングできるかどうか判定。
      template<Castling Flag> bool CanCastling() const;

      // キャスリングの権利を更新する。
      void UpdateCastlingRights();

      // その位置が攻撃されているかどうかチェックする。
      // [引数]
      // square: 調べたい位置。
      // side: 攻撃側のサイド。
      // [戻り値]
      // 攻撃されていればtrue。
      bool IsAttacked(Square square, Side side) const;
      // マテリアルを得る。
      // [引数]
      // side: マテリアルを得たいサイド。
      // [戻り値]
      // マテリアル。
      int GetMaterial(Side side) const;
      // 合法手があるかどうかチェックする。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // 合法手があればtrue。
      bool HasLegalMove(Side side);
      // その位置を攻撃している駒のビットボードを得る。
      // [引数]
      // target_square: 攻撃されている位置。
      // side: 攻撃側のサイド。
      // [戻り値]
      // 攻撃している駒のビットボード。
      Bitboard GetAttackers(Square target_square, Side side) const;

      /****************/
      /* メンバ変数。 */
      /****************/
      // 駒の配置のビットボードの配列。
      Bitboard position_[NUM_SIDES][NUM_PIECE_TYPES];
      // 駒の種類の配置。
      Piece piece_board_[NUM_SQUARES];
      // サイドの配置。
      Side side_board_[NUM_SQUARES];
      // 各サイドの駒の配置。
      Bitboard side_pieces_[NUM_SIDES];
      // ブロッカーの配置。
      Bitboard blocker_0_;  // 角度0度。
      Bitboard blocker_45_;  // 角度45度。
      Bitboard blocker_90_;  // 角度90度。
      Bitboard blocker_135_;  // 角度135度。
      // キングの位置。
      Square king_[NUM_SIDES];
      // 手番。
      Side to_move_;
      // キャスリングの権利。
      Castling castling_rights_;
      // アンパッサンの位置。
      Square en_passant_square_;
      // アンパッサンできるかどうか。
      bool can_en_passant_;
      // 50手ルール。
      int ply_100_;
      // 現在の手数。
      int ply_;
      // キャスリングしたかどうか。
      bool has_castled_[NUM_SIDES];
      // ヒストリー。history_[from][to]。
      int history_[NUM_SQUARES][NUM_SQUARES];
      // 探索情報用スタック。
      struct SearchInfoSlot {
        // 現在の手。何を指されて今のノードになったのか。
        Move current_move_;
        // 現在の局面のハッシュキー。
        HashKey current_pos_key_;
        // IIDによる最善手。
        Move iid_move_;
        // キラームーブ。
        Move killer_;

        SearchInfoSlot() :
        current_pos_key_(0ULL) {
          current_move_.all_ = 0;
          iid_move_.all_ = 0;
          killer_.all_ = 0;
        }
      };
      SearchInfoSlot search_info_stack_[MAX_PLY];

      /**********************/
      /* ハッシュキー関連。 */
      /**********************/
      // ハッシュキーを得るための配列。
      // 第1インデックスはサイド。
      // 第2インデックスは駒の種類。
      // 第3インデックスは駒の位置。
      // それぞれのインデックスに値を入れると、
      // そのハッシュキーを得られる。
      static HashKey key_table_[NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
      // key_table_[][][]を初期化する。
      static void InitKeyTable();
      // 現在の局面のハッシュキーを計算する。
      // (注)計算に時間がかかる。
      // [戻り値]
      // 現在の局面のハッシュキー。
      HashKey GetCurrentKey() const;
      // 現在の局面と動かす手から次の局面のハッシュキーを得る。
      // [引数]
      // current_key: 現在のキー。
      // move: 次の手。
      // [戻り値]
      // 次の局面のハッシュキー。
      HashKey GetNextKey(HashKey current_key, Move move) const;
  };
}  // namespace Sayuri

#endif
