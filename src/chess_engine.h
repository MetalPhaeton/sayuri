/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Hironori Ishibashi
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
 * @file chess_engine.h
 * @author Hironori Ishibashi
 * @brief チェスエンジンの本体。
 */

#ifndef CHESS_ENGINE_H
#define CHESS_ENGINE_H

#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <thread>
#include <mutex>
#include <cstddef>
#include <cstdint>
#include "common.h"
#include "evaluator.h"
#include "position_record.h"
#include "helper_queue.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class Fen;
  class TranspositionTable;
  class MoveMaker;
  class Evaluator;
  class PVLine;
  class UCIShell;
  class PositionRecord;
  class Job;
  class HelperQueue;
  class SearchParams;
  class EvalParams;

  /** チェスエンジンの本体クラス。 */
  class ChessEngine {
    public:
      // ========================= //
      // ChessEngineクラスの初期化 //
      // ========================= //
      /** static変数の初期化。 */
      static void InitChessEngine();

      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * 基本コンストラクタ。
       * @param search_params 探索関数用パラメータ。
       * @param eval_params 評価関数用パラメータ。
       */
      ChessEngine(const SearchParams& search_params,
      const EvalParams& eval_params);
      /**
       * コピーコンストラクタ。
       * @param engine コピー元。
       */
      ChessEngine(const ChessEngine& engine);
      /**
       * ムーブコンストラクタ。
       * @param engine ムーブ元。
       */
      ChessEngine(ChessEngine&& engine);
      /**
       * コピー代入演算子。
       * @param engine コピー元。
       */
      ChessEngine& operator=(const ChessEngine& engine);
      /**
       * ムーブ代入演算子。
       * @param engine ムーブ元。
       */
      ChessEngine& operator=(ChessEngine&& engine);
      /** デストラクタ。 */
      virtual ~ChessEngine();

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 探索関数用パラメータを再設定する。
       * @param search_params 再設定する探索関数用パラメータ。
       */
      void ResetSearchParams(const SearchParams& search_params) {
        shared_st_ptr_->search_params_ptr_ = &search_params;
      }
      /**
       * 評価関数用パラメータを再設定する。
       * @param eval_params 再設定する評価関数用パラメータ。
       */
      void ResetEvalParams(const EvalParams& eval_params) {
        shared_st_ptr_->eval_params_ptr_ = &eval_params;
      }

      /**
       * FEN文字列を読み込む。
       * @param fen 読み込むFEN文字列。
       */
      void LoadFen(const Fen& fen);

      /**
       * PositionRecordから局面を読み込む。
       * @param record 読み込むPositionRecordオブジェクト。
       */
      void LoadRecord(const PositionRecord& record);

      /** 駒を初期配置にセットする。 */
      void SetStartPosition();

      /** 新しいゲームの準備をする。 */
      void SetNewGame();

      /**
       * 探索のストップ条件を設定する。
       * @param max_depth 最大の探索深さ。
       * @param max_nodes 最大の探索ノード数。
       * @param thinking_time 思考時間。
       * @param infinite_thinking 無限に思考するかどうかのフラグ。
       */
      void SetStopper(std::uint32_t max_depth, std::uint64_t max_nodes,
      Chrono::milliseconds thinking_time, bool infinite_thinking);

      /**
       * 無限に思考するかどうかのフラグをセットする。
       * @param enable trueで有効。 falseで無効。
       */
      void EnableInfiniteThinking(bool enable);

      /**
       * 探索を開始する。
       * @param num_threads 探索用のスレッド数。
       * @param table 探索用トランスポジションテーブル。
       * @param moves_to_search 探索する候補手。 空ならすべての候補手を探索。
       * @param shell Infoコマンドを出力するUCIShell。
       * @return 探索結果のPVライン。
       */
      PVLine Calculate(int num_threads, TranspositionTable& table,
      const std::vector<Move>& moves_to_search, UCIShell& shell);

      /** 探索を終了させる。 */
      void StopCalculation();

      /**
       * 手を指す。
       * @param move 指し手。
       */
      void PlayMove(Move move);
      /** 1手戻す。 */
      void UndoMove();

      // --- 利き筋関連 --- //
      /**
       * ボードの状態からビショップの利き筋を作る。
       * @param square 基点のマス。
       * @return ビショップの利き筋。
       */
      Bitboard GetBishopAttack(Square square) const {
        return Util::GetAttack45(square, blocker_45_)
        | Util::GetAttack135(square, blocker_135_);
      }
      /**
       * ボードの状態からルークの利き筋を作る。
       * @param square 基点のマス。
       * @return ルークの利き筋。
       */
      Bitboard GetRookAttack(Square square) const {
        return Util::GetAttack0(square, blocker_0_)
        | Util::GetAttack90(square, blocker_90_);
      }
      /**
       * ボードの状態からクイーンの利き筋を作る。
       * @param square 基点のマス。
       * @return クイーンの利き筋。
       */
      Bitboard GetQueenAttack(Square square) const {
        return GetBishopAttack(square) | GetRookAttack(square);
      }

      /**
       * キャスリング出来るかどうかを判定する。
       * @param <Flag> 調べたいキャスリングの定数。
       * @return キャスリング可能ならtrue。
       */
      template<Castling Flag> bool CanCastling() const;


      /**
       * その位置が他の位置の駒に攻撃されているかどうかチェックする。
       * @param square 調べたいマス。
       * @param side 攻撃側のサイド。
       * @return 「square」が攻撃されていればtrue。
       */
      bool IsAttacked(Square square, Side side) const;

      /**
       * 現在のマテリアルを得る。
       * @param side 得たいマテリアルのサイド。
       * @return マテリアル。
       */
      int GetMaterial(Side side) const;

      /**
       * 指定の候補手から次の局面の「自分」のマテリアルを得る。
       * (「相手」のマテリアルではない。)
       * @param current_material 現在のマテリアル。
       * @param move 次の局面への候補手。
       * @return 次の局面の「自分」のマテリアル。
       */
      int GetNextMyMaterial(int current_material, Move move) const;

      /**
       * SEEで候補手を評価する。
       * @param move 評価したい手。
       * @return 候補手の評価。
       */
      int SEE(Move move) const;

      /**
       * 現在の局面のハッシュを計算する。
       * @return 現在の局面のハッシュ。
       */
      Hash GetCurrentHash() const;

      /**
       * 現在の局面のハッシュと候補手から、次の局面のハッシュを得る。
       * @param current_hash 現在の局面のハッシュ。
       * @param move 次の局面への候補手。
       * @return 次の局面のハッシュ。
       */
      Hash GetNextHash(Hash current_hash, Move move) const;

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - 駒の配置のビットボード。
       * @return 駒の配置のビットボード。 [サイド][駒の種類]
       */
      const Bitboard (& position() const)[NUM_SIDES][NUM_PIECE_TYPES] {
        return position_;
      }
      /**
       * アクセサ - 駒の種類の配置。
       * @return 駒の種類の配置。 [マス]
       */
      const Piece (& piece_board() const)[NUM_SQUARES] {
        return piece_board_;
      }
      /**
       * アクセサ - サイドの配置。
       * @return サイドの配置。 [マス]
       */
      const Side (& side_board() const)[NUM_SQUARES] {
        return side_board_;
      }
      /**
       * アクセサ - 各サイドの駒の配置のビットボード。
       * @return 各サイドの駒の配置のビットボード。 [サイド]
       */
      const Bitboard (& side_pieces() const)[NUM_SIDES] {
        return side_pieces_;
      }
      /**
       * アクセサ - 全駒の配置のビットボード。 角度0度。
       * @return 全駒の配置のビットボード。 角度0度。
       */
      Bitboard blocker_0() const {return blocker_0_;}
      /**
       * アクセサ - 全駒の配置のビットボード。 角度45度。
       * @return 全駒の配置のビットボード。 角度45度。
       */
      Bitboard blocker_45() const {return blocker_45_;}
      /**
       * アクセサ - 全駒の配置のビットボード。 角度90度。
       * @return 全駒の配置のビットボード。 角度90度。
       */
      Bitboard blocker_90() const {return blocker_90_;}
      /**
       * アクセサ - 全駒の配置のビットボード。 角度135度。
       * @return 全駒の配置のビットボード。 角度135度。
       */
      Bitboard blocker_135() const {return blocker_135_;}
      /**
       * アクセサ - 各サイドのキングの位置。
       * @return 各サイドのキングの位置。 [サイド]
       */
      const Square (& king() const)[NUM_SIDES] {return king_;}
      /**
       * アクセサ - 手番。
       * @return 手番。
       */
      Side to_move() const {return to_move_;}
      /**
       * アクセサ - キャスリングの権利。
       * @return キャスリングの権利。
       */
      Castling castling_rights() const {return castling_rights_;}
      /**
       * アクセサ - アンパッサンの位置。
       * @return アンパッサンの位置。 なければ 0。
       */
      Square en_passant_square() const {return en_passant_square_;}
      /**
       * アクセサ - 50手ルールの手数。
       * @return 50手ルールの手数。
       */
      int ply_100() const {return ply_100_;}
      /**
       * アクセサ - 現在の手数。
       * @return 現在の手数。
       */
      int ply() const {return ply_;}
      /**
       * アクセサ - 各サイドの、キャスリングしたかどうかのフラグ。
       * @return 各サイドの、キャスリングしたかどうかのフラグ。 [サイド]
       */
      const bool (& has_castled() const)[NUM_SIDES] {return has_castled_;}
      /**
       * アクセサ - ヒストリー。
       * @return ヒストリー。 [サイド][from][to]。
       */
      const std::uint64_t
      (& history() const)[NUM_SIDES][NUM_SQUARES][NUM_SQUARES] {
        return shared_st_ptr_->history_;
      }
      /**
       * アクセサ - ヒストリーの最大値。
       * @return ヒストリーの最大値。
       */
      std::uint64_t history_max() const {return shared_st_ptr_->history_max_;}
      /**
       * アクセサ - IIDでの最善手のスタック。
       * @return IIDでの最善手のスタック。 [探索レベル]
       */
      const Move (& iid_stack() const)[MAX_PLYS + 1] {
        return shared_st_ptr_->iid_stack_;
      }
      /**
       * アクセサ - キラームーブのスタック。
       * @return キラームーブのスタック。 [探索レベル][index * 2 プライ前]
       */
      const Move (& killer_stack() const)[MAX_PLYS + 2 + 1][2] {
        return shared_st_ptr_->killer_stack_;
      }
      /**
       * アクセサ - 探索関数用パラメータ。
       * @return 探索関数用パラメータ。
       */
      const SearchParams& search_params() const {
        return *(shared_st_ptr_->search_params_ptr_);
      }
      /**
       * アクセサ - 評価関数用パラメータ。
       * @return 評価関数用パラメータ。
       */
      const EvalParams& eval_params() const {
        return *(shared_st_ptr_->eval_params_ptr_);
      }

    private:
      /** フレンドのデバッグ用関数。 */
      friend int DebugMain(int argc, char* argv[]);

      /** プライベート用コンストラクタ。 */
      ChessEngine();

      // ======== //
      // 探索関数 //
      // ======== //
      /**
       * クイース探索。
       * @param depth 現在の深さ。
       * @param level 現在のレベル。
       * @param alpha アルファ値。
       * @param beta ベータ値。
       * @param material 現在のマテリアル。
       * @param table トランスポジションテーブル。
       * @return 評価値。
       */
      int Quiesce(int depth, std::uint32_t level, int alpha, int beta,
      int material, TranspositionTable& table);
      /**
       * 通常の探索。
       * @param <Type> ノードの種類。
       * @param pos_hash 現在のハッシュ。
       * @param depth 現在の深さ。
       * @param level 現在のレベル。
       * @param alpha アルファ値。
       * @param beta ベータ値。
       * @param material 現在のマテリアル。
       * @param table トランスポジションテーブル。
       * @return 評価値。
       */
      template<NodeType Type>
      int Search(Hash pos_hash, int depth, std::uint32_t level, int alpha,
      int beta, int material, TranspositionTable& table);
      /**
       * 探索のルート。
       * @param table トランスポジションテーブル。
       * @param moves_to_search 探索する候補手。 空なら全ての候補手を探索。
       * @param shell Infoコマンドの出力用UCIShell。
       * @return 探索結果のPVライン。
       */
      PVLine SearchRoot(TranspositionTable& table,
      const std::vector<Move>& moves_to_search, UCIShell& shell);
      /**
       * YBWC探索用スレッド。
       * @param shell Infoコマンドの出力用UCIShell。
       */
      void ThreadYBWC(UCIShell& shell);
      /**
       * 別スレッド用探索関数。
       * @param job 並列探索用仕事。
       */
      template<NodeType Type> void SearchParallel(Job& job);
      /**
       * ルートノードで呼び出される、別スレッド用探索関数。
       * @param job 並列探索用仕事。
       * @param shell Infoコマンドの出力用UCIShell。
       */
      void SearchRootParallel(Job& job, UCIShell& shell);
      /**
       * Futility Pruningのマージンを計算する。
       * @param depth 現在の深さ。
       * @return マージン。
       */
      int GetMargin(int depth);
      /**
       * 探索を中断しなければならないかどうかを判断する。
       * @return 中断しなければいけない時はtrue。
       */
      bool ShouldBeStopped();

      // ======================== //
      // その他のプライベート関数 //
      // ======================== //
      /**
       * 他のエンジンの基本メンバをコピーする。
       * @param engine 他のエンジン。
       */
      void ScanBasicMember(const ChessEngine& engine);

      /**
       * 次の一手を指す。
       * 取った駒、動かす前のキャスリングの権利、アンパッサンは
       * 「move」に記録される。
       * 動かす駒の位置と移動先が同じならNull Moveとなる。 (手番が変わる。)
       * 「move」はUnmakeMove()で利用する。
       * @param move 候補手。
       */
      void MakeMove(Move& move);
      /**
       * MakeMove()で動かした手を元に戻す。
       * @param move MakeMove()で使用した候補手。
       */
      void UnmakeMove(Move move);

      /**
       * 駒を置く。
       * @param square 駒を置きたいマス。
       * @param piece_type 置きたい駒の種類。
       * @param side 置きたい駒のサイド。
       */
      void PutPiece(Square square, Piece piece_type, Side side=NO_SIDE);

      /**
       * 駒の位置を変える。 (移動先の駒は上書きされる。)
       * @param from 位置を変えたい駒のマス。
       * @param to 移動先。
       */
      void ReplacePiece(Square from, Square to);

      /**
       * SEE()で使う、次の手を得る。
       * @param target 取る駒のマス。
       * @return 次の手。
       */
      Move GetNextSEEMove(Square target) const;

      // ========== //
      // メンバ変数 //
      // ========== //
      // --- 基本メンバ --- //
      /** 駒の配置のビットボード。 [サイド][駒の種類] */
      Bitboard position_[NUM_SIDES][NUM_PIECE_TYPES];
      /** 駒の種類の配置。 [マス] */
      Piece piece_board_[NUM_SQUARES];
      /** サイドの配置。 [マス] */
      Side side_board_[NUM_SQUARES];
      /** 各サイドの駒の配置のビットボード。 [サイド] */
      Bitboard side_pieces_[NUM_SIDES];
      /** 全駒の配置のビットボード。 角度0度。 */
      Bitboard blocker_0_;
      /** 全駒の配置のビットボード。 角度45度。 */
      Bitboard blocker_45_;
      /** 全駒の配置のビットボード。 角度90度。 */
      Bitboard blocker_90_;
      /** 全駒の配置のビットボード。 角度135度。 */
      Bitboard blocker_135_;
      /** 各サイドのキングの位置。 [サイド] */
      Square king_[NUM_SIDES];
      /** 手番。 */
      Side to_move_;
      /** キャスリングの権利。 */
      Castling castling_rights_;
      /** アンパッサンの位置。アンパッサンできなければ0。 */
      Square en_passant_square_;
      /** 50手ルールの手数。 */
      int ply_100_;
      /** 現在の手数。 */
      int ply_;
      /** 各サイドのキャスリングしたかどうかのフラグ。 [サイド] */
      bool has_castled_[NUM_SIDES];

      // ================================================= //
      // 共有メンバ (指定した他のエンジンと共有するメンバ) //
      // ================================================= //
      /** 共有メンバの構造体。 */
      struct SharedStruct {
        /** ヒストリー。 [サイド][from][to]。 */
        std::uint64_t history_[NUM_SIDES][NUM_SQUARES][NUM_SQUARES];
        /** ヒストリーの最大値。 */
        std::uint64_t history_max_;
        /** IIDでの最善手スタック。 [探索レベル] */
        Move iid_stack_[MAX_PLYS + 1];
        /** キラームーブスタック。[探索レベル][index * 2 プライ前] */
        Move killer_stack_[MAX_PLYS + 2 + 1][2];
        /** 現在のIterative Deepeningの深さ。 */
        std::uint32_t i_depth_;
        /** 探索したノード数。 */
        std::uint64_t num_searched_nodes_;
        /** 探索開始時間。 */
        TimePoint start_time_;
        /** 探索ストップ条件: 何が何でも探索を中断。 */
        bool stop_now_;
        /** 探索ストップ条件: 最大探索ノード数。 */
        std::uint64_t max_nodes_;
        /** 探索ストップ条件: 最大探索深さ。 */
        std::uint32_t max_depth_;
        /** 探索ストップ条件: 思考時間。 */
        Chrono::milliseconds thinking_time_;
        /** 探索ストップ条件: trueなら無限に考える。 */
        bool infinite_thinking_;
        /** 指し手の履歴。 */
        std::vector<Move> move_history_;
        /** 50手ルールの手数の履歴。 */
        std::vector<int> ply_100_history_;
        /** 局面の履歴。 */
        std::vector<PositionRecord> position_history_;
        /** スレッドのキュー。 */
        std::unique_ptr<HelperQueue> helper_queue_ptr_;
        /** 探索関数用パラメータ。 */
        const SearchParams* search_params_ptr_;
        /** 評価関数用パラメータ。 */
        const EvalParams* eval_params_ptr_;

        // ============ //
        // ハッシュ関連 //
        // ============ //
        /** 駒の情報のハッシュ値のテーブル。 [サイド][駒の種類][駒の位置] */
        Hash piece_hash_value_table_[NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
        /** 手番のハッシュ値のテーブル。 [サイド] */
        Hash to_move_hash_value_table_[NUM_SIDES];
        /**
         * キャスリングの権利のハッシュ値のテーブル。
         * - 0: 白のショートキャスリング。
         * - 1: 白のロングキャスリング。
         * - 2: 黒のショートキャスリング。
         * - 3: 黒のロングキャスリング。
         */
        Hash castling_hash_value_table_[4];
        /** アンパッサンの位置のハッシュ値のテーブル。 */
        Hash en_passant_hash_value_table_[NUM_SQUARES];

        // ==================== //
        // コンストラクタと代入 //
        // ==================== //
        /** コンストラクタ。 */
        SharedStruct();
        /** 
         * コピーコンストラクタ。
         * @param shared_st コピー元。
         */
        SharedStruct(const SharedStruct& shared_st);
        /** 
         * ムーブコンストラクタ。
         * @param shared_st ムーブ元。
         */
        SharedStruct(SharedStruct&& shared_st);
        /** 
         * コピー代入演算子。
         * @param shared_st コピー元。
         */
        SharedStruct& operator=(const SharedStruct& shared_st);
        /** 
         * ムーブ代入演算子。
         * @param shared_st ムーブ元。
         */
        SharedStruct& operator=(SharedStruct&& shared_st);
        /** デストラクタ。 */
        virtual ~SharedStruct() {}

        // ============ //
        // その他の関数 //
        // ============ //
        /**
         * メンバをコピーする。
         * @param shared_st コピー元。
         */
        void ScanMember(const SharedStruct& shared_st);

        /** ハッシュ値のテーブルを初期化する。 */
        void InitHashValueTable();

      };
      /** 共有メンバの構造体。 */
      std::shared_ptr<SharedStruct> shared_st_ptr_;

      // ===================================================== //
      // 固有メンバ (他のエンジンとコピーも共有もしないメンバ) //
      // ===================================================== //
      /** 今ヌルムーブのサーチ中かどうかのフラグ。 */
      bool is_null_searching_;
      /** 探索したレベル。 */
      std::uint32_t searched_level_;
      /** 探索用MoveMakerのテーブル。 [探索レベル] */
      std::unique_ptr<MoveMaker[]> maker_table_;
      /** 探索用PVLineのテーブル。 [探索レベル] */
      std::unique_ptr<PVLine[]> pv_line_table_;
      /** 探索用Evaluator。 */
      Evaluator evaluator_;
      /** 並列探索用仕事のテーブル。 [探索レベル] */
      std::unique_ptr<Job[]> job_table_;
      /** 並列探索用ミューテックス。 */
      std::mutex mutex_;
      /** 並列探索用スレッドのベクトル。 */
      std::vector<std::thread> thread_vec_;
  };
}  // namespace Sayuri

#endif
