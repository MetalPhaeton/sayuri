/* 
   chess_engine.h: チェスボード。

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
#include <memory>
#include <array>
#include <thread>
#include <mutex>
#include <cstddef>
#include "chess_def.h"
#include "chess_util.h"
#include "transposition_table.h"
#include "fen.h"
#include "pv_line.h"
#include "position_record.h"
#include "job.h"
#include "helper_queue.h"

namespace Sayuri {
  class PositionRecord;
  class Job;
  class HelperQueue;

  // チェスエンジンのクラス。
  class ChessEngine {
    public:
      /*******************************/
      /* ChessEngineクラスの初期化。 */
      /*******************************/
      static void InitChessEngine();

      /**************************/
      /* コンストラクタと代入。 */
      /**************************/
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

      // 探索のストップ条件を設定する。
      // [引数]
      // max_depth: 最大探索深さ。
      // max_nodes: 最大探索ノード数。
      // thinking_time: 思考時間。
      // infinite_thinking: 無限に思考するかどうか。
      void SetStopper(int max_depth, std::size_t max_nodes,
      Chrono::milliseconds thinking_time, bool infinite_thinking);

      // 思考の無限時間フラグをセットする。
      // [引数]
      // enable: trueは有効。falseは無効。
      void EnableInfiniteThinking(bool enable);

      // 思考を始める。
      // [引数]
      // num_cores: CPUのコアの数。
      // table: 使用するトランスポジションテーブル。
      // moves_to_search_ptr: 探索する候補手。nullptrなら全ての手を探索する。
      // [戻り値]
      // PVライン。
      PVLine Calculate(int num_cores, TranspositionTable& table,
      std::vector<Move>* moves_to_search_ptr);

      // 探索を終了させる。
      void StopCalculation();

      // 手を指す。
      // [引数]
      // move: 動かす手。
      void PlayMove(Move move);
      // 1手戻す。
      void UndoMove();

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

      // SEE。
      // [引数]
      // move: 探したい手。
      // [戻り値]
      // 評価値。
      int SEE(Move move) const;

      // 現在の局面のハッシュを計算する。
      // (注)計算に時間がかかる。
      // [戻り値]
      // 現在の局面のハッシュ。
      Hash GetCurrentHash() const;

      // 現在の局面と動かす手から次の局面のハッシュを得る。
      // [引数]
      // current_hash: 現在のハッシュ。
      // move: 次の手。
      // [戻り値]
      // 次の局面のハッシュ。
      Hash GetNextHash(Hash current_hash, Move move) const;

      /**************/
      /* アクセサ。 */
      /**************/
      // 駒の配置。
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
      // ブロッカーの配置。角度0度。
      Bitboard blocker_0() const {return blocker_0_;}
      // ブロッカーの配置。角度45度。
      Bitboard blocker_45() const {return blocker_45_;}
      // ブロッカーの配置。角度90度。
      Bitboard blocker_90() const {return blocker_90_;}
      // ブロッカーの配置。角度135度。
      Bitboard blocker_135() const {return blocker_135_;}
      // キングの位置。
      const Square (& king() const)[NUM_SIDES] {return king_;}
      // 手番。
      Side to_move() const {return to_move_;}
      // キャスリングの権利。
      Castling castling_rights() const {return castling_rights_;}
      // アンパッサンの位置。
      Square en_passant_square() const {return en_passant_square_;}
      // アンパッサンできるかどうか。
      bool can_en_passant() const {return can_en_passant_;}
      // 50手ルール。
      int ply_100() const {return ply_100_;}
      // 現在の手数。
      int ply() const {return ply_;}
      // キャスリングしたかどうか。
      const bool (& has_castled() const)[NUM_SIDES] {return has_castled_;}
      // ヒストリー。history()[side][from][to]。
      const std::array<std::array<std::array<int,
      NUM_SQUARES>, NUM_SQUARES>, NUM_SIDES>& history() const {
        return *history_ptr_;
      }
      // ヒストリーの最大値。
      int history_max() const {return *history_max_ptr_;}
      // IIDでの最善手スタック。
      const std::array<Move, MAX_PLYS>& iid_stack() const {
        return *iid_stack_ptr_;
      }
      // キラームーブスタック。
      const std::array<Move, MAX_PLYS>& killer_stack() const {
        return *killer_stack_ptr_;
      }

    private:
      /******************/
      /* 探索エンジン。 */
      /******************/
      // クイース探索。
      // [引数]
      // depth: 現在の深さ。
      // level: 現在のレベル。
      // alpha: アルファ値。
      // beta: ベータ値。
      // table: トランスポジションテーブル。
      // [戻り値]
      // 評価値。
      int Quiesce(int depth, int level, int alpha, int beta,
      TranspositionTable& table);
      // 探索する。
      // [引数]
      // <Type>: ノードの種類。
      // pos_hash: 現在のハッシュ。
      // depth: 現在の深さ。
      // level: 現在のレベル。
      // alpha: アルファ値。
      // beta: ベータ値。
      // table: トランスポジションテーブル。
      // pv_line: PVラインが格納される。
      // [戻り値]
      // 評価値。
      template<NodeType Type>
      int Search(Hash pos_hash, int depth, int level, int alpha, int beta,
      TranspositionTable& table, PVLine& pv_line);
      // 探索のルート。
      // [引数]
      // table: 使用するトランスポジションテーブル。
      // moves_to_search_ptr: 探索する候補手。nullptrなら全ての手を探索する。
      // [戻り値]
      // PVライン。
      PVLine SearchRoot(TranspositionTable& table,
      std::vector<Move>* moves_to_search_ptr);
      // 探索用子スレッド。
      static void ThreadPVSpilt();
      // 子スレッドで探索。
      // [引数]
      // job: 探索用仕事。
      template<NodeType Type> void SearchAsChild(Job& job);
      // ルートノードで子スレッドで探索。
      // [引数]
      // job: 探索用仕事。
      void SearchRootAsChild(Job& job);
      // Futility Pruningのマージンを計算する。
      // [引数]
      // move: 指し手。
      // depth: 現在の深さ。
      // [戻り値]
      // マージン。
      int GetMargin(Move move, int depth);
      // 探索を中断しなければいけないかどうか。
      // [戻り値]
      // 探索を中断しなければいけないときはtrue。
      bool ShouldBeStopped();

      /******************************/
      /* その他のプライベート関数。 */
      /******************************/
      // 他のエンジンの基本メンバをコピーする。
      // [引数]
      // engine: 他のエンジン。
      void ScanBasicMember(const ChessEngine& engine);
      // 他のエンジンの共有メンバのコピー。
      // [引数]
      // engine: 他のエンジン。
      void ScanSharedMember(const ChessEngine& engine);
      // 他のエンジンの共有メンバのムーブ。
      // [引数]
      // engine: 他のエンジン。
      void MoveSharedMember(ChessEngine&& engine);
      // 他のエンジンと共有メンバを共有する。
      // [引数]
      // engine: 他のエンジン。
      void LinkSharedMember(ChessEngine& engine);

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

      // キャスリングの権利を更新する。
      void UpdateCastlingRights();

      // SEEで使う次の手を得る。
      // [引数]
      // target: 取る駒の位置。
      Move GetNextSEEMove(Square target) const;

      /****************/
      /* メンバ変数。 */
      /****************/
      /*****************/
      /* 基本メンバ。  */
      /*****************/
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
      /********************************************************/
      /* 共有メンバ。(指定した他のエンジンと共有するメンバ。) */
      /********************************************************/
      // ヒストリー。(*history_ptr_)[side][from][to]。
      std::shared_ptr<std::array<std::array<std::array<int,
      NUM_SQUARES>, NUM_SQUARES>, NUM_SIDES>> history_ptr_;
      // ヒストリーの最大値。
      std::shared_ptr<int> history_max_ptr_;
      // IIDでの最善手スタック。
      std::shared_ptr<std::array<Move, MAX_PLYS>> iid_stack_ptr_;
      // キラームーブスタック。
      std::shared_ptr<std::array<Move, MAX_PLYS>> killer_stack_ptr_;
      // 現在のIterative Deepeningの深さ。
      std::shared_ptr<int> i_depth_ptr_;
      // 探索したノード数。
      std::shared_ptr<std::size_t> num_searched_nodes_ptr_;
      // 探索開始時間。
      std::shared_ptr<TimePoint> start_time_ptr_;
      // 探索したレベル。
      std::shared_ptr<int> searched_level_ptr_;
      // ストップ条件構造体。
      struct Stopper {
        // 何が何でも探索を中断。
        volatile bool stop_now_;
        // 最大探索ノード数。
        volatile std::size_t max_nodes_;
        // 最大探索深さ。
        volatile int max_depth_;
        // 思考時間。
        volatile int thinking_time_;
        // 無限に考える。
        volatile bool infinite_thinking_;

        // コンストラクタ。
        Stopper() :
        stop_now_(false),
        max_nodes_(MAX_NODES),
        max_depth_(MAX_PLYS),
        thinking_time_(-1U >> 1),
        infinite_thinking_(false) {}
        // コピーコンストラクタ。
        Stopper(const Stopper& stopper) :
        stop_now_(stopper.stop_now_),
        max_nodes_(stopper.max_nodes_),
        max_depth_(stopper.max_depth_),
        thinking_time_(stopper.thinking_time_),
        infinite_thinking_(stopper.infinite_thinking_) {}
        // ムーブコンストラクタ。
        Stopper(Stopper&& stopper) :
        stop_now_(stopper.stop_now_),
        max_nodes_(stopper.max_nodes_),
        max_depth_(stopper.max_depth_),
        thinking_time_(stopper.thinking_time_),
        infinite_thinking_(stopper.infinite_thinking_) {}
        // コピー代入。
        Stopper& operator=(const Stopper& stopper) {
          stop_now_ = stopper.stop_now_;
          max_nodes_ = stopper.max_nodes_;
          max_depth_ = stopper.max_depth_;
          thinking_time_ = stopper.thinking_time_;
          infinite_thinking_ = stopper.infinite_thinking_;
          return *this;
        }
        // ムーブ代入。
        Stopper& operator=(Stopper&& stopper) {
          stop_now_ = stopper.stop_now_;
          max_nodes_ = stopper.max_nodes_;
          max_depth_ = stopper.max_depth_;
          thinking_time_ = stopper.thinking_time_;
          infinite_thinking_ = stopper.infinite_thinking_;
          return *this;
        }
      };
      std::shared_ptr<Stopper> stopper_ptr_;
      // 指し手の履歴。
      std::shared_ptr<std::vector<Move>> move_history_ptr_;
      // 50手ルールの履歴。
      std::shared_ptr<std::vector<int>> ply_100_history_ptr_;
      // 配置の履歴。
      std::shared_ptr<std::vector<PositionRecord>> position_history_ptr_;
      // スレッドのキュー。
      std::shared_ptr<HelperQueue> helper_queue_ptr_;
      /************************************************************/
      /* 固有メンバ。(他のエンジンとコピーも共有もしないメンバ。) */
      /************************************************************/
      // 今ヌルムーブのサーチ中かどうか。
      bool is_null_searching_;
      // PVSplit用ミューテックス。
      std::mutex pvs_mutex_;
      // PVSplit用スレッドのベクトル。
      std::vector<std::thread> pvs_thread_vec_;

      /******************/
      /* ハッシュ関連。 */
      /******************/
      // 駒の情報からハッシュを得るための配列。
      // piece_hash_table_[サイド][駒の種類][駒の位置]
      static Hash piece_hash_table_[NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
      // 手番からハッシュを得るための配列。
      static Hash to_move_hash_table_[NUM_SIDES];
      // キャスリングの権利からハッシュを得るための配列。
      // 0: 白のショートキャスリング。
      // 1: 白のロングキャスリング。
      // 2: 黒のショートキャスリング。
      // 3: 黒のロングキャスリング。
      static Hash castling_hash_table_[4];
      // アンパッサンの位置からハッシュを得るための配列。
      static Hash en_passant_hash_table_[NUM_SQUARES];
      // ハッシュの配列を初期化する。
      static void InitHashTable();
  };
}  // namespace Sayuri

#endif
