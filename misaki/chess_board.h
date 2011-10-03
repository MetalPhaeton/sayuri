/* chess_board.h: チェスボード。
   Copyright (c) 2011 Ishibashi Hironori

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

#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include <iostream>
#include <vector>
#include <ctime>
#include <boost/thread.hpp>
#include "chess_def.h"
#include "chess_util.h"
#include "move.h"
#include "game_record.h"
#include "transposition_table.h"

namespace Misaki {
  class ChessBoard;
  struct EvalWeights;
  class GameRecord;
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
      int rook_7th_weight_;  // 第7ランクのルークの重さ。
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
  std::ostream& operator<<(std::ostream& stream, const ChessBoard& board);
  class ChessBoard {
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
       * ChessBoardクラスの初期化。 *
       ******************************/
      static void InitChessBoard() {
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
      static ChessBoard* New() {
        return new ChessBoard();
      }
      // デストラクタ。
      virtual ~ChessBoard();

      /**********
       * 関数。 *
       **********/
      // 次の手のリストを作る。
      // [引数]
      // 次の手のリストのポインタ。
      MoveList* CreateNextMoveList() const;
      // 履歴のサイズを得る。
      // [戻り値]
      // 履歴のサイズ。
      int GetHistorySize() const {return history_.size();}
      // ゲームのデータを得る。
      // [引数]
      // index: 履歴のインデックス。
      // [戻り値]
      // ゲームのデータ。
      const GameRecord& GetGameRecord(int index) const {
        return *(history_[index]);
      }
      // 現在のボードのデータを得る。
      // [戻り値]
      // 現在のゲームのデータ。
      const GameRecord& GetCurrentGameRecord() const {
        return *(history_[current_game_]);
      }
      // ゲームを1つ前に戻す。
      void StepBack();
      // ゲームを1つ進める。
      void StepForward();
      // 手を指す。
      // [引数]
      // 指す手。
      // [戻り値]
      // 指せればtrue。
      bool TakeMove(const Move& move);
      // 最善手を得る。
      // [引数]
      // searching_time: 探索時間。
      // table[inout]: 使用するトランスポジションテーブル。
      // weights: 評価の重さ。
      // [戻り値]
      // 最善手。
      Move GetBestMove(double searching_time, TranspositionTable& table,
      const EvalWeights& weights) const;

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
      bool IsChecked(side_t side) const {
        if (side == NO_SIDE) return false;
        return IsAttacked(king_[side], static_cast<side_t>(side ^ 0x3));
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
      bool HasEnoughPieces(side_t side) const;
      // 終盤かどうか判定する。
      // キングとポーン以外の駒が4個以下なら終盤。
      // [戻り値]
      // 終盤ならtrue。
      bool IsEnding() const {
        bitboard_t pieces = blocker0_;
        pieces &=
        ~(position_[WHITE][KING] | position_[BLACK][KING]
        | position_[WHITE][PAWN] | position_[BLACK][PAWN]);
        return ChessUtil::CountBits(pieces) <= 4;
      } 
      // 動ける位置の数を得る。
      // [引数]
      // piece_square: 調べたい駒の位置。
      // [戻り値]
      // 動ける位置の数。
      int GetMobility(square_t piece_square) const;
      // 攻撃している位置のビットボードを得る。
      // アンパサンは含まない。
      // [引数]
      // pieces: 調べたい駒のビットボード。
      // [戻り値]
      // piecesが攻撃している位置のビットボード。
      bitboard_t GetAttack(bitboard_t pieces) const;
      // パスポーンの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // パスポーンの位置のビットボード。
      bitboard_t GetPassPawns(side_t side) const;
      // ダブルポーンの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // ダブルポーンの位置のビットボード。
      bitboard_t GetDoublePawns(side_t side) const;
      // 孤立ポーンの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // 孤立ポーンの位置のビットボード。
      bitboard_t GetIsoPawns(side_t side) const;
      // 展開されていないマイナーピースの位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // 展開されていないマイナーピースの位置のビットボード。
      bitboard_t GetNotDevelopedMinorPieces(side_t side) const;
      // キングのポーンの盾の位置のビットボードを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // ポーンの盾の位置のビットボード。
      bitboard_t GetPawnShield(side_t side) const {
        if (side == NO_SIDE) return 0;

        return position_[side][PAWN] & pawn_shield_mask_[side][king_[side]];
      }
      // キャスリングしたかどうかを得る。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // キャスリングしたかどうか。
      bool HasCastled(side_t side) const {
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
      int EvalAll(side_t side, const EvalWeights& weights) const;
      // 機動力を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalMobility(side_t side, const EvalWeights& weights) const;
      // センター攻撃を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalAttackCenter(side_t side, const EvalWeights& weights) const;
      // 展開を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalDevelopment(side_t side, const EvalWeights& weights) const;
      // キングの周囲への攻撃を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalAttackAroundKing(side_t side, const EvalWeights& weights) const;
      // ポーンの配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalPawnPosition(side_t side, const EvalWeights& weights) const;
      // ナイトの配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalKnightPosition(side_t side, const EvalWeights& weights) const;
      // ルークの配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalRookPosition(side_t side, const EvalWeights& weights) const;
      // キングの中盤の配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalKingPositionMiddle(side_t side,
      const EvalWeights& weights) const;
      // キングの終盤の配置を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalKingPositionEnding(side_t side,
      const EvalWeights& weights) const;
      // パスポーンを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalPassPawn(side_t side, const EvalWeights& weights) const;
      // ダブルポーンを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalDoublePawn(side_t side, const EvalWeights& weights) const;
      // 孤立ポーンを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalIsoPawn(side_t side, const EvalWeights& weights) const;
      // ビショップペアを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalBishopPair(side_t side, const EvalWeights& weights) const;
      // 第7ランクのルークを評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalRook7th(side_t side, const EvalWeights& weights) const;
      // 早すぎるクイーンの出動を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalEarlyQueenLaunched(side_t side, const EvalWeights& weights)
      const;
      // ポーンの盾を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalPawnShield(side_t side, const EvalWeights& weights) const;
      // キャスリングの破棄を評価する。
      // [引数]
      // side: 評価したいサイド。
      // weights: 評価の重さ。
      // [戻り値]
      // 評価値。
      int EvalCanceledCastling(side_t side, const EvalWeights& weights) const;

    protected:
      /********************
       * コンストラクタ。 *
       ********************/
      // コンストラクタ。
      ChessBoard();

      /**********************************
       * アクセサ。派生クラスのみ公開。 *
       **********************************/
      // 駒の配置のビットボードの配列。
      const bitboard_t (& position() const)[NUM_SIDES][NUM_PIECE_TYPES] {
        return position_;
      }
      // 駒の種類の配置。
      const piece_t (& piece_board() const)[NUM_SQUARES] {
        return piece_board_;
      }
      // サイドの配置。
      const side_t (& side_board() const)[NUM_SQUARES] {
        return side_board_;
      }
      // 各サイドの駒の配置。
      const bitboard_t (& side_pieces() const)[NUM_SIDES] {
        return side_pieces_;
      }
      // ブロッカーの配置。
      bitboard_t blocker0() const {return blocker0_;}  // 0度。
      bitboard_t blocker45() const {return blocker45_;}  // 45度
      bitboard_t blocker90() const {return blocker90_;}  // 90度。
      bitboard_t blocker135() const {return blocker135_;}  // 135度。
      // キングの位置。
      const square_t (& king() const)[NUM_SIDES] {
        return king_;
      }
      // 手番。
      side_t to_move() const {return to_move_;}
      // キャスリングの権利。
      castling_t castling_rights() const {return castling_rights_;}
      // アンパッサンのターゲットの位置。
      square_t en_passant_target() const {return en_passant_target_;}
      // アンパッサンできるかどうか。
      bool can_en_passant() const {return can_en_passant_;}
      // ゲームの履歴。
      const std::vector<GameRecord*> history() const {return history_;}
      // 現在のゲームの履歴の位置。
      int current_game() const {return current_game_;}

    private:
      /******************
       * コピーの禁止。 *
       ******************/
      ChessBoard(const ChessBoard&);  // 削除。
      ChessBoard& operator=(const ChessBoard&);  // 削除。

      /****************
       * 出力演算子。 *
       ****************/
      friend std::ostream& operator<<(std::ostream& stream,
      const ChessBoard& board);

      /********************
       * フレンドクラス。 *
       ********************/
      friend class GameRecord;

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
      void MakeMove(move_t& move);
      // MakeMove()で動かした駒を元に戻す。
      // 必ず先にMakeMove()をすること。
      // [引数]
      // move: MakeMove()で動かした手。
      void UnmakeMove(move_t move);

      /******************************
       * 候補手をツリーに展開する。 *
       ******************************/
      // 手のマスク。GiveQuickScore()で使う。
      static move_t move_mask_;
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
      int SEE(move_t move);
      // ノードに簡易点数を付ける。
      // [引数]
      // key: その局面のハッシュキー。
      // level: 簡易点数を付けるレベル。
      // depth: その局面の深さ。
      // side: その局面のサイド。
      // table: トランスポジションテーブル。
      void GiveQuickScore(hash_key_t key, int level, int depth, side_t side,
      TranspositionTable& table);
      // 簡易点数の高いもの順にポップする。
      // [引数]
      // level: ポップするレベル。
      // [戻り値]
      // ポップした手。
      move_t PopBestMove(int level);

      /**************************
       * 探索に使う関数と変数。 *
       **************************/
      // 最善手。
      move_t best_move_;
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
      int GetMCap(move_t move) const;
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
      int Quiesce(int level, int depth, int alpha, int beta, hash_key_t key,
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
      bool is_null_move, hash_key_t key,
      TranspositionTable& table, const EvalWeights& weights);

      /******************************
       * その他のプライベート関数。 *
       ******************************/
      // 駒を置く。（駒の種類piece_typeにEMPTYをおけば、駒を削除できる。）
      // [引数]
      // square: 置きたい位置。
      // piece_type: 駒の種類。
      // side: 置きたい駒のサイド。
      void PutPiece(square_t square, piece_t piece_type, side_t side=NO_SIDE);
      // 駒の位置を変える。
      // [引数]
      // piece_square: 移動する駒の位置。
      // goal_square: 移動先の位置。
      void ReplacePiece(square_t piece_square, square_t goal_square);

      // ビショップの攻撃筋を作る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ビショップの攻撃筋。
      bitboard_t GetBishopAttack(square_t square) const {
        return ChessUtil::GetAttack45(square, blocker45_)
        | ChessUtil::GetAttack135(square, blocker135_);
      }
      // ルークの攻撃筋を作る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // ルークの攻撃筋。
      bitboard_t GetRookAttack(square_t square) const {
        return ChessUtil::GetAttack0(square, blocker0_)
        | ChessUtil::GetAttack90(square, blocker90_);
      }
      // クイーンの攻撃筋を作る。
      // [引数]
      // square: 位置。
      // [戻り値]
      // クイーンの攻撃筋。
      bitboard_t GetQueenAttack(square_t square) const {
        return GetBishopAttack(square) | GetRookAttack(square);
      }

      // キャスリングの権利を更新する。
      void UpdateCastlingRights() {
        // 白キングがe1にいなければ白のキャスリングの権利を放棄。
        if (king_[WHITE] != E1) castling_rights_ &= ~WHITE_CASTLING;
        // 黒キングがe8にいなければ黒のキャスリングの権利を放棄。
        if (king_[BLACK] != E8) castling_rights_ &= ~BLACK_CASTLING;

        // 白のルークがh1にいなければ白のショートキャスリングの権利を放棄。
        if (!(position_[WHITE][ROOK] & ChessUtil::BIT[H1]))
          castling_rights_ &= ~WHITE_SHORT_CASTLING;
        // 白のルークがa1にいなければ白のロングキャスリングの権利を放棄。
        if (!(position_[WHITE][ROOK] & ChessUtil::BIT[A1]))
          castling_rights_ &= ~WHITE_LONG_CASTLING;

        // 黒のルークがh8にいなければ黒のショートキャスリングの権利を放棄。
        if (!(position_[BLACK][ROOK] & ChessUtil::BIT[H8]))
          castling_rights_ &= ~BLACK_SHORT_CASTLING;
        // 黒のルークがa8にいなければ黒のロングキャスリングの権利を放棄。
        if (!(position_[BLACK][ROOK] & ChessUtil::BIT[A8]))
          castling_rights_ &= ~BLACK_LONG_CASTLING;
      }

      // その位置が攻撃されているかどうかチェックする。
      // [引数]
      // square: 調べたい位置。
      // side: 攻撃側のサイド。
      // [戻り値]
      // 攻撃されていればtrue。
      bool IsAttacked(square_t square, side_t side) const;
      // マテリアルを得る。
      // [引数]
      // side: マテリアルを得たいサイド。
      // [戻り値]
      // マテリアル。
      int GetMaterial(side_t side) const;
      // 合法手があるかどうかチェックする。
      // [引数]
      // side: 調べたいサイド。
      // [戻り値]
      // 合法手があればtrue。
      bool HasLegalMove(side_t side) const;
      // その位置を攻撃している駒のビットボードを得る。
      // [引数]
      // target_square: 攻撃されている位置。
      // side: 攻撃側のサイド。
      // [戻り値]
      // 攻撃している駒のビットボード。
      bitboard_t GetAttackers(square_t target_square, side_t side) const;

      /****************
       * メンバ変数。 *
       ****************/
      // 同期オブジェクト。
      boost::mutex sync_;
      // 駒の配置のビットボードの配列。
      bitboard_t position_[NUM_SIDES][NUM_PIECE_TYPES];
      // 駒の種類の配置。
      piece_t piece_board_[NUM_SQUARES];
      // サイドの配置。
      side_t side_board_[NUM_SQUARES];
      // 各サイドの駒の配置。
      bitboard_t side_pieces_[NUM_SIDES];
      // ブロッカーの配置。
      bitboard_t blocker0_;  // 角度0度。
      bitboard_t blocker45_;  // 角度45度。
      bitboard_t blocker90_;  // 角度90度。
      bitboard_t blocker135_;  // 角度135度。
      // キングの位置。
      square_t king_[NUM_SIDES];
      // 手番。
      side_t to_move_;
      // キャスリングの権利。
      castling_t castling_rights_;
      // アンパッサンのターゲットの位置。
      square_t en_passant_target_;
      // アンパッサンできるかどうか。
      bool can_en_passant_;
      // キャスリングをしたかどうか。
      bool has_white_castled_;  // 白。
      bool has_black_castled_;  // 黒。
      // ゲームの履歴。
      std::vector<GameRecord*> history_;
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
      static int GetTableValue(const int (& table)[NUM_SQUARES], side_t side,
      bitboard_t bitboard);

      // パスポーンを判定するときに使用するマスク。
      static bitboard_t pass_pawn_mask_[NUM_SIDES][NUM_SQUARES];
      // pass_pawn_mask_[][]を初期化する。
      static void InitPassPawnMask();

      // 孤立ポーンを判定するときに使用するマスク。
      static bitboard_t iso_pawn_mask_[NUM_SQUARES];
      // iso_pawn_mask_[]を初期化する。
      static void InitIsoPawnMask();

      // ポーン盾の位置のマスク。
      static bitboard_t pawn_shield_mask_[NUM_SIDES][NUM_SQUARES];
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
          move_t move_;
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
      void PushMove(move_t move, int level) {
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
      move_t PopMove(int level) {
        // スタックがないなら無意味な手を返す。
        if (stack_ptr_[level] == tree_ptr_[level]) {
          move_t move;
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
      static hash_key_t key_array_[NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
      // 乱数の種。
      static hash_key_t seed_;
      // key_array_[][][]を初期化する。
      static void InitKeyArray();
      // 64bit乱数を得る。
      // [戻り値]
      // 64bit乱数。
      static hash_key_t GetRand() {
        seed_ = (seed_ * 0x5d588b656c078965ULL) + 0x0000000000269ec3ULL;
        return seed_;
      }
      // 現在の局面と動かす手から次の局面のハッシュキーを得る。
      // [引数]
      // current_key: 現在のキー。
      // move: 次の手。
      // [戻り値]
      // 次の局面のハッシュキー。
      hash_key_t GetNextKey(hash_key_t current_key, move_t move) const;

      /*******************
       * Pondering関連。 *
       *******************/
      // スレッド。
      boost::thread* pondering_thread_ptr_;
      // スレッドの中止フラグ。
      bool stop_pondering_flag_;
      // Ponderingの手を展開するバッファ。
      move_t pondering_buffer_[200];
      // Ponderingする。
      // [引数]
      // depth: Ponderingする深さ。
      // table[inout]: 使用するトランスポジションテーブル。
      // weights: 評価の重さ。
      void Ponder(int depth, TranspositionTable& table,
      const EvalWeights& weights);
  };
}  // Misaki

#endif
