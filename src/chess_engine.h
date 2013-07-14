/* chess_engine.h: チェスボード。
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
#include <boost/thread.hpp>
#include "chess_def.h"
#include "chess_util.h"
#include "move.h"
#include "game_record.h"
#include "transposition_table.h"

namespace Sayuri {
  class ChessEngine;
  struct EvalWeights;
  class TranspositionTable;
  class TranspositionTableSlotList;
  class TranspositionTableSlot;

  /************************
   * 評価の重さの構造体。 *
   ************************/
  struct EvalWeights {
    public:
      /**********************
       * 全駒の評価の重さ。 *
       **********************/
      int mobility_weight_;  // 機動力の重さ。
      int attack_center_weight_;  // センター攻撃の重さ。
      int development_weight_;  // 展開の重さ。
      int attack_around_king_weight_;  // キングの周囲への攻撃の重さ。

      /******************************
       * 駒の配置の重要度テーブル。 *
       ******************************/
      int pawn_position_table_[NUM_SQUARES];  // ポーンの配置。
      int knight_position_table_[NUM_SQUARES];  // ナイトの配置。
      int rook_position_table_[NUM_SQUARES];  // ルークの配置。
      int king_position_middle_table_[NUM_SQUARES];  // キングの中盤の配置。
      int king_position_ending_table_[NUM_SQUARES];  // キングの終盤の配置。

      /********************
       * 駒の配置の重さ。 *
       ********************/
      int pawn_position_weight_;  // ポーンの配置の重さ。
      int knight_position_weight_;  // ナイトの配置の重さ。
      int rook_position_weight_;  // ルークの配置の重さ。
      int king_position_middle_weight_;  // キングの中盤の配置の重さ。
      int king_position_ending_weight_;  // キングの終盤の配置の重さ。

      /********************
       * それ以外の重さ。 *
       ********************/
      int pass_pawn_weight_;  // パスポーンの重さ。
      int protected_pass_pawn_weight_;  // 守られたパスポーンの重さ。
      int double_pawn_weight_;  // ダブルポーンの重さ。
      int iso_pawn_weight_;  // 孤立ポーンの重さ。
      int bishop_pair_weight_;  // ビショップペアの重さ。
      int early_queen_launched_weight_;  // 早すぎるクイーンの出動の重さ。
      int pawn_shield_weight_;  // ポーンの盾の重さ。
      int canceled_castling_weight_;  // キャスリングの破棄の重さ。

      /********************
       * コンストラクタ。 *
       ********************/
      EvalWeights();
  };

  /**************************
   * チェスボードのクラス。 *
   **************************/
  std::ostream& operator<<(std::ostream& stream, const ChessEngine& board);
  class ChessEngine {
    private:
      /****************
       * 定数の定義。 *
       ****************/
      enum {
        INFINITE = 9999999
      };
      enum {
        SCORE_WIN = 1000000,
        SCORE_LOSE = -1000000,
        SCORE_DRAW = 0,
        SCORE_PAWN = 100,
        SCORE_KNIGHT = 300,
        SCORE_BISHOP = 300,
        SCORE_ROOK = 500,
        SCORE_QUEEN = 900,
        SCORE_KING = 1000000
      };

    public:
      /******************
       * テスト用関数。 *
       ******************/
      void Test();

      /******************************
       * ChessEngineクラスの初期化。 *
       ******************************/
      static void InitChessEngine() {
        // key_array_[][][]を初期化する。
        InitKeyArray();
        // pass_pawn_mask_[][]を初期化する。
        InitPassPawnMask();
        // iso_pawn_mask_[]を初期化する。
        InitIsoPawnMask();
        // pawn_shield_mask_[][]を初期化する。
        InitPawnShieldMask();
        // move_maskを初期化する。
        InitMoveMask();
      }


      /**************************************
       * インスタンスの生成とデストラクタ。 *
       **************************************/
      // インスタンスを生成する。
      // [戻り値]
      // インスタンスのポインタ。
      static ChessEngine* New() {
        return new ChessEngine();
      }
      // デストラクタ。
      virtual ~ChessEngine();

      /**********
       * 関数。 *
       **********/
      // ゲームを1つ前に戻す。
      void StepBack();
      // ゲームを1つ進める。
      void StepForward();

      /***************
       * Pondering。 *
       ***************/
      // Ponderingを開始する。
      // [引数]
      // depth: Ponderingする深さ。
      // table[inout]: 使用するトランスポジションテーブル。
      // weights: 評価の重さ。
      void StartPondering(int depth, TranspositionTable& table,
      const EvalWeights& weights) const;
      // Ponderingを停止する。
      void StopPondering() const;

      /************************
       * 局面を分析する関数。 *
       ************************/
      // キングがチェックされているかどうかチェックする。
      // [引数]
      // side: チェックされているか調べるサイド。
      // [戻り値]
      // キングがチェックされていればtrue。
      bool IsChecked(Side side) const {
        if (side == NO_SIDE) return false;

        return IsAttacked(king_[side], side ^ 0x3);
      }
      // チェックメイトされているかどうか調べる。
      // [戻り値]
      // チェックメイトされていればtrue。
      bool IsCheckmated() const {
        return IsChecked(to_move_) && !HasLegalMove(to_move_);
      }
      // ステールメイトかどうか調べる。
      // [戻り値]
      // ステールメイトされていればtrue。
      bool IsStalemated() const {
        return !IsChecked(to_move_) && !HasLegalMove(to_move_);
      }
      // 勝つのに十分な駒があるかどうか調べる。
      // [引数]
      // side: 調べるサイド。
      // [戻り値]
      // 十分な駒があればtrue。
      bool HasEnoughPieces(Side side) const;
      // 終盤かどうか判定する。
      // キングとポーン以外の駒が4個以下なら終盤。
      // [戻り値]
      // 終盤ならtrue。
      bool IsEnding() const {
        Bitboard pieces = blocker0_;
        pieces &= ~(position_[WHITE][KING] | position_[BLACK][KING]
        | position_[WHITE][PAWN] | position_[BLACK][PAWN]);
        return Util::CountBits(pieces) <= 4;
      } 
      // 動ける位置の数を得る。
      // [引数]
      // piece_square: 調べたい駒の位置。
      // [戻り値]
      // 動ける位置の数。
      int GetMobility(Square piece_square) const;
      // 攻撃している位置のビットボードを得る。
      // アンパサンは含まない。
      // [引数]
      // pieces: 調べたい駒のビットボード。
      // [戻り値]
      // piecesが攻撃している位置のビットボード。
      Bitboard GetAttack(Bitboard pieces) const;
      // パスポーンの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // パスポーンの位置のビットボード。
      Bitboard GetPassPawns(Side side) const;
      // ダブルポーンの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // ダブルポーンの位置のビットボード。
      Bitboard GetDoublePawns(Side side) const;
      // 孤立ポーンの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // 孤立ポーンの位置のビットボード。
      Bitboard GetIsoPawns(Side side) const;
      // 展開されていないマイナーピースの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // 展開されていないマイナーピースの位置のビットボード。
      Bitboard GetNotDevelopedMinorPieces(Side side) const;
      // キングのポーンの盾の位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // ポーンの盾の位置のビットボード。
      Bitboard GetPawnShield(Side side) const {
        if (side == NO_SIDE) return 0;

        return position_[side][PAWN]
        & pawn_shield_mask_[side][king_[side]];
      }
      // キャスリングしたかどうかを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // キャスリングしたかどうか。
      bool HasCastled(Side side) const {
        if (side == NO_SIDE) return false;
        return side == WHITE ? has_white_castled_ : has_black_castled_;
      }

      /************************
       * 局面を評価する関数。 *
       ************************/
      // 全てを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalAll(Side side, const EvalWeights& weights) const;
      // 機動力を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalMobility(Side side, const EvalWeights& weights) const;
      // センター攻撃を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalAttackCenter(Side side, const EvalWeights& weights) const;
      // 展開を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalDevelopment(Side side, const EvalWeights& weights) const;
      // キングの周囲への攻撃を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalAttackAroundKing(Side side, const EvalWeights& weights) const;
      // ポーンの配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalPawnPosition(Side side, const EvalWeights& weights) const;
      // ナイトの配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalKnightPosition(Side side, const EvalWeights& weights) const;
      // ルークの配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalRookPosition(Side side, const EvalWeights& weights) const;
      // キングの中盤の配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalKingPositionMiddle(Side side,
      const EvalWeights& weights) const;
      // キングの終盤の配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalKingPositionEnding(Side side,
      const EvalWeights& weights) const;
      // パスポーンを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalPassPawn(Side side, const EvalWeights& weights) const;
      // ダブルポーンを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalDoublePawn(Side side, const EvalWeights& weights) const;
      // 孤立ポーンを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalIsoPawn(Side side, const EvalWeights& weights) const;
      // ビショップペアを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalBishopPair(Side side, const EvalWeights& weights) const;
      // 早すぎるクイーンの出動を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalEarlyQueenLaunched(Side side, const EvalWeights& weights)
      const;
      // ポーンの盾を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalPawnShield(Side side, const EvalWeights& weights) const;
      // キャスリングの破棄を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalCanceledCastling(Side side, const EvalWeights& weights) const;

    protected:
      /********************
       * コンストラクタ。 *
       ********************/
      // コンストラクタ。
      ChessEngine();

      /**********************************
       * アクセサ。派生クラスのみ公開。 *
       **********************************/
      // 駒の配置のビットボードの配列。
      const Bitboard (& position() const)[NUM_SIDES][NUM_PIECE_TYPES] {
        return position_;
      }
      // 駒の種類の配置。
      const Piece (& piece_board() const)[NUM_SQUARES] {
        return piece_board_;
      }
      // サイドの配置。
      const Side (& side_board() const)[NUM_SQUARES] {
        return side_board_;
      }
      // 各サイドの駒の配置。
      const Bitboard (& side_pieces() const)[NUM_SIDES] {
        return side_pieces_;
      }
      // ブロッカーの配置。
      Bitboard blocker0() const {return blocker0_;}  // 0度。
      Bitboard blocker45() const {return blocker45_;}  // 45度
      Bitboard blocker90() const {return blocker90_;}  // 90度。
      Bitboard blocker135() const {return blocker135_;}  // 135度。
      // キングの位置。
      const Square (& king() const)[NUM_SIDES] {
        return king_;
      }
      // 手番。
      Side to_move() const {return to_move_;}
      // キャスリングの権利。
      Castling castling_rights() const {return castling_rights_;}
      // アンパッサンのターゲットの位置。
      Square en_passant_target() const {return en_passant_target_;}
      // アンパッサンできるかどうか。
      bool can_en_passant() const {return can_en_passant_;}
      // 現在のゲームの履歴の位置。
      int current_game() const {return current_game_;}

    private:
      /******************
       * コピーの禁止。 *
       ******************/
      ChessEngine(const ChessEngine&);  // 削除。
      ChessEngine& operator=(const ChessEngine&);  // 削除。

      /****************
       * 出力演算子。 *
       ****************/
      friend std::ostream& operator<<(std::ostream& stream,
      const ChessEngine& board);

      /****************
       * 駒を動かす。 *
       ****************/
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

      /******************************
       * 候補手をツリーに展開する。 *
       ******************************/
      // 手のマスク。GiveQuickScore()で使う。
      static Move move_mask_;
      static void InitMoveMask();
      // 駒を取る手を展開する。
      // [引数]
      // level: 展開するツリーのレベル。
      // [戻り値]
      // いくつ手を展開できたか。
      int GenCaptureMove(int level);
      // 駒を取らない手を展開する。
      // [引数]
      // level: 展開するツリーのレベル。
      // [戻り値]
      // いくつ手を展開できたか。
      int GenNonCaptureMove(int level);
      // 全ての手を展開する。
      // [引数]
      // level: 展開するツリーのレベル。
      // [戻り値]
      // いくつ手を展開できたか。
      int GenMove(int level);
      // チェックを逃れる手を作る。
      // [引数]
      // level: 展開するツリーのレベル。
      // [戻り値]
      // いくつ手を展開できたか。
      int GenCheckEscapeMove(int level);
      // SEE。
      // [引数]
      // move: どの手について評価したいか。
      // [戻り値]
      // 駒の取り合いの簡易評価値。
      int SEE(Move move);
      // ノードに簡易点数を付ける。
      // [引数]
      // key: その局面のハッシュキー。
      // level: 簡易点数を付けるレベル。
      // depth: その局面の深さ。
      // side: その局面のサイド。
      // table: トランスポジションテーブル。
      void GiveQuickScore(HashKey key, int level, int depth, Side side,
      TranspositionTable& table);
      // 簡易点数の高いもの順にポップする。
      // [引数]
      // level: ポップするレベル。
      // [戻り値]
      // ポップした手。
      Move PopBestMove(int level);

      /**************************
       * 探索に使う関数と変数。 *
       **************************/
      // 最善手。
      Move best_move_;
      // 探索開始時間。
      time_t start_time_;
      // 探索時間。
      double searching_time_;
      // タイムアウトしたかどうか。
      // [戻り値]
      // タイムアウトならtrue。
      bool IsTimeOut() const {
        if (std::difftime(std::time(NULL), start_time_) >= searching_time_) {
          return true;
        } else {
          return false;
        }
      }
      // 最善手のスコア。
      int best_score_;
      // MCapを得る。
      // [引数]
      // move: 動かす手。
      // [戻り値]
      // MCap。
      int GetMCap(Move move) const;
      // クイース探索。
      // [引数]
      // level: 探索のレベル。
      // depth: 探索の深さ。
      // alpha: アルファ値。
      // beta: ベータ値。
      // key: ハッシュキー。
      // table: トランスポジションテーブル。
      // weights: 評価の重さ。
      // [戻り値]
      // 探索結果の評価値。
      int Quiesce(int level, int depth, int alpha, int beta, HashKey key,
      TranspositionTable& table, const EvalWeights& weights);
      // 探索する。
      // [引数]
      // level: 探索のレベル。
      // depth: 探索の深さ。
      // alpha: アルファ値。
      // beta: ベータ値。
      // is_null_move: Null Moveの探索かどうか。
      // key: ハッシュキー。
      // table: 使用するトランスポジションテーブル。
      // weights: 評価の重さ。
      // [戻り値]
      // 探索結果の評価値。
      int Search(int level, int depth, int alpha, int beta,
      bool is_null_move, HashKey key,
      TranspositionTable& table, const EvalWeights& weights);

      /******************************
       * その他のプライベート関数。 *
       ******************************/
      // 駒を置く。（駒の種類PieceypeにEMPTYをおけば、駒を削除できる。）
      // [引数]
      // square: 置きたい位置。
      // Pieceype: 駒の種類。
      // side: 置きたい駒のサイド。
      void PutPiece(Square square, Piece Pieceype, Side side=NO_SIDE);
      // 駒の位置を変える。
      // [引数]
      // piece_square: 移動する駒の位置。
      // goal_square: 移動先の位置。
      void ReplacePiece(Square piece_square, Square goal_square);

      // ビショップの攻撃筋を作る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ビショップの攻撃筋。
      Bitboard GetBishopAttack(Square square) const {
        return Util::GetAttack45(square, blocker45_)
        | Util::GetAttack135(square, blocker135_);
      }
      // ルークの攻撃筋を作る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ルークの攻撃筋。
      Bitboard GetRookAttack(Square square) const {
        return Util::GetAttack0(square, blocker0_)
        | Util::GetAttack90(square, blocker90_);
      }
      // クイーンの攻撃筋を作る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // クイーンの攻撃筋。
      Bitboard GetQueenAttack(Square square) const {
        return GetBishopAttack(square) | GetRookAttack(square);
      }

      // キャスリングの権利を更新する。
      void UpdateCastlingRights() {
        // 白キングがe1にいなければ白のキャスリングの権利を放棄。
        if (king_[WHITE] != E1)
          castling_rights_ &= ~WHITE_CASTLING;

        // 黒キングがe8にいなければ黒のキャスリングの権利を放棄。
        if (king_[BLACK] != E8) castling_rights_ &= ~BLACK_CASTLING;

        // 白のルークがh1にいなければ白のショートキャスリングの権利を放棄。
        if (!(position_[WHITE][ROOK] & Util::BIT[H1]))
          castling_rights_ &= ~WHITE_SHORT_CASTLING;
        // 白のルークがa1にいなければ白のロングキャスリングの権利を放棄。
        if (!(position_[WHITE][ROOK] & Util::BIT[A1]))
          castling_rights_ &= ~WHITE_LONG_CASTLING;

        // 黒のルークがh8にいなければ黒のショートキャスリングの権利を放棄。
        if (!(position_[BLACK][ROOK] & Util::BIT[H8]))
          castling_rights_ &= ~BLACK_SHORT_CASTLING;
        // 黒のルークがa8にいなければ黒のロングキャスリングの権利を放棄。
        if (!(position_[BLACK][ROOK] & Util::BIT[A8]))
          castling_rights_ &= ~BLACK_LONG_CASTLING;
      }

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
      bool HasLegalMove(Side side) const;
      // その位置を攻撃している駒のビットボードを得る。
      // [引数]
      // target_square: 攻撃されている位置。
      // side: 攻撃側のサイド。
      // [戻り値]
      // 攻撃している駒のビットボード。
      Bitboard GetAttackers(Square target_square, Side side) const;

      /****************
       * メンバ変数。 *
       ****************/
      // 同期オブジェクト。
      boost::mutex sync_;
      // 駒の配置のビットボードの配列。
      Bitboard position_[NUM_SIDES][NUM_PIECE_TYPES];
      // 駒の種類の配置。
      Piece piece_board_[NUM_SQUARES];
      // サイドの配置。
      Side side_board_[NUM_SQUARES];
      // 各サイドの駒の配置。
      Bitboard side_pieces_[NUM_SIDES];
      // ブロッカーの配置。
      Bitboard blocker0_;  // 角度0度。
      Bitboard blocker45_;  // 角度45度。
      Bitboard blocker90_;  // 角度90度。
      Bitboard blocker135_;  // 角度135度。
      // キングの位置。
      Square king_[NUM_SIDES];
      // 手番。
      Side to_move_;
      // キャスリングの権利。
      Castling castling_rights_;
      // アンパッサンのターゲットの位置。
      Square en_passant_target_;
      // アンパッサンできるかどうか。
      bool can_en_passant_;
      // キャスリングをしたかどうか。
      bool has_white_castled_;  // 白。
      bool has_black_castled_;  // 黒。
      // 現在のゲームの履歴の位置。
      int current_game_;

      /****************
       * 局面分析用。 *
       ****************/
      // ポーンの配置のテーブル。
      static const int pawn_position_table_[NUM_SQUARES];
      // ナイトの配置のテーブル。
      static const int knight_position_table_[NUM_SQUARES];
      // キングの中盤での配置のテーブル。
      static const int king_position_table_middle_[NUM_SQUARES];
      // キングの終盤での配置のテーブル。
      static const int king_position_table_ending_[NUM_SQUARES];
      // テーブルを計算する。
      // [引数]
      // table: 計算するテーブル。
      // side: 計算したいサイド。
      // bitboard: 位置のビットボード。
      // [戻り値]
      // テーブルを計算した結果の値。
      static int GetTableValue(const int (& table)[NUM_SQUARES], Side side,
      Bitboard bitboard);

      // パスポーンを判定するときに使用するマスク。
      static Bitboard pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
      // pass_pawn_mask_[][]を初期化する。
      static void InitPassPawnMask();

      // 孤立ポーンを判定するときに使用するマスク。
      static Bitboard iso_pawn_mask_[NUM_SQUARES];
      // iso_pawn_mask_[]を初期化する。
      static void InitIsoPawnMask();

      // ポーン盾の位置のマスク。
      static Bitboard pawn_shield_mask_[NUM_SIDES][NUM_SQUARES];
      // pawn_shield_mask_[][]を初期化する。
      static void InitPawnShieldMask();

      /************************
       * 手を展開するツリー。 *
       ************************/
      // ツリーの定数。
      enum {
        TREE_SIZE = 10000,
        MAX_LEVEL = 32
      };
      // ノードの構造体。
      struct Node {
        public:
          Move move_;
          int quick_score_;
      };
      // ツリー。
      Node tree_[TREE_SIZE];
      // 各レベルのツリーへのポインタ。
      Node* tree_ptr_[MAX_LEVEL];
      // 各レベルのスタックポインタ。
      Node* stack_ptr_[MAX_LEVEL];
      // スタックにプッシュする。
      // [引数]
      // move: プッシュする手。
      // level: プッシュするツリーのレベル。
      void PushMove(Move move, int level) {
        if (stack_ptr_[level] >= (&tree_[TREE_SIZE - 1])) {
          return;
        }
        stack_ptr_[level]->move_ = move;
        (stack_ptr_[level])++;
      }
      // スタックからポップする。
      // [引数]
      // level: ポップするツリーのレベル。
      // [戻り値]
      // ポップした手。
      Move PopMove(int level) {
        // スタックがないなら無意味な手を返す。
        if (stack_ptr_[level] == tree_ptr_[level]) {
          Move move;
          move.all_ = 0;
          return move;
        }

        (stack_ptr_[level])--;
        return stack_ptr_[level]->move_;
      }
      // スタックをクリアする。
      // [引数]
      // level: クリアするツリーのレベル。
      void ClearMoves(int level) {
        stack_ptr_[level] = tree_ptr_[level];
      }

      /**********************
       * ハッシュキー関連。 *
       **********************/
      // ハッシュキーを得るための配列。
      // 第1インデックスはサイド。
      // 第2インデックスは駒の種類。
      // 第3インデックスは駒の位置。
      // それぞれのインデックスに値を入れると、
      // そのハッシュキーを得られる。
      static HashKey key_array_[NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
      // 乱数の種。
      static HashKey seed_;
      // key_array_[][][]を初期化する。
      static void InitKeyArray();
      // 64bit乱数を得る。
      // [戻り値]
      // 64bit乱数。
      static HashKey GetRand() {
        seed_ = (seed_ * 0x5d588b656c078965ULL) + 0x0000000000269ec3ULL;
        return seed_;
      }
      // 現在の局面と動かす手から次の局面のハッシュキーを得る。
      // [引数]
      // current_key: 現在のキー。
      // move: 次の手。
      // [戻り値]
      // 次の局面のハッシュキー。
      HashKey GetNextKey(HashKey current_key, Move move) const;

      /*******************
       * Pondering関連。 *
       *******************/
      // スレッド。
      boost::thread* pondering_thread_ptr_;
      // スレッドの中止フラグ。
      bool stop_pondering_flag_;
      // Ponderingの手を展開するバッファ。
      Move pondering_buffer_[200];
      // Ponderingする。
      // [引数]
      // depth: Ponderingする深さ。
      // table[inout]: 使用するトランスポジションテーブル。
      // weights: 評価の重さ。
      void Ponder(int depth, TranspositionTable& table,
      const EvalWeights& weights);
  };
}  // namespace Sayuri

#endif
