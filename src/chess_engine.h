/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Hironori Ishibashi
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

#ifndef CHESS_ENGINE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define CHESS_ENGINE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstddef>
#include <climits>
#include <queue>
#include "common.h"
#include "board.h"
#include "evaluator.h"
#include "position_record.h"
#include "helper_queue.h"
#include "params.h"
#include "cache.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  class FEN;
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
       * @param search_params 登録する探索関数用パラメータ。
       * @param eval_params 登録する評価関数用パラメータ。
       * @param table 登録するトランスポジションテーブル。
       */
      ChessEngine(const SearchParams& search_params,
      const EvalParams& eval_params, TranspositionTable& table);
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
      void LoadFEN(const FEN& fen);

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
      void SetStopper(u32 max_depth, u64 max_nodes,
      const Chrono::milliseconds& thinking_time, bool infinite_thinking);

      /**
       * 無限に思考するかどうかのフラグをセットする。
       * @param enable trueで有効。 falseで無効。
       */
      void EnableInfiniteThinking(bool enable);

      /**
       * 探索を開始する。
       * @param num_threads 探索用のスレッド数。
       * @param moves_to_search 探索する候補手。 空ならすべての候補手を探索。
       * @param shell Infoコマンドを出力するUCIShell。
       * @return 探索結果のPVライン。
       */
      PVLine Calculate(int num_threads,
      const std::vector<Move>& moves_to_search, UCIShell& shell);

      /** 探索を終了させる。 */
      void StopCalculation();

      /**
       * 手を指す。
       * @param move 指し手。
       * @return 指せればtrue。
       */
      bool PlayMove(Move move);

      /**
       * 1手戻す。
       * @return 戻された手。 戻せない時は0。
       */
      Move UndoMove();

      /**
       * 合法手かどうか判定する。
       * もし合法手なら、その手を引数moveに代入する。
       * @param move 判定したい手。
       * @return 判定結果。 合法手ならtrue。
       */
      bool IsLegalMove(Move& move) const;

      /**
       * 合法手のベクトルを得る。
       * @return 合法手のベクトル。
       */
      std::vector<Move> GetLegalMoves() const;

      /**
       * 現在の局面からAlgebraic Notationの指し手を予測する。
       * @param note 予測するAlgebraic Notation。
       * @return 候補の指し手のベクトル。
       */
      std::vector<Move> GuessNote(const std::string& note) const;

      /**
       * 現在の局面からAlgebraic Notationを作成する。
       * @param move 作成したい指し手。
       * @return Algebraic Notation。
       */
      std::string MoveToNote(Move move) const;

      /**
       * 駒を配置する。
       * (注) お互いのキングは必ず1つになるように配置される。
       * - キングの場所に他の駒を配置できない。
       * - もう一つキングを配置すると、すでにあるキングはなくなる。
       * @param square 配置する位置。
       * @param piece_type 配置する駒の種類。
       * @param piece_side 配置する駒のサイド。
       */
      void PlacePiece(Square square, PieceType piece_type,
      Side piece_side=NO_SIDE);

      /**
       * ボードが正しい状態かどうか調べる。
       * @return 正しければtrue。
       */
      bool IsCorrectPosition() const {
        // ポーンの配置を調べる。 第1ランクと第8ランクのポーンは間違い。
        if (((basic_st_.position_[WHITE][PAWN]
        | basic_st_.position_[BLACK][PAWN])
        & (Util::RANK[RANK_1] | Util::RANK[RANK_8]))) {
          return false;
        }

        // キングの個数を調べる。
        if (Util::CountBits(basic_st_.position_[WHITE][KING]) != 1) {
          return false;
        };
        if (Util::CountBits(basic_st_.position_[BLACK][KING]) != 1) {
          return false;
        }

        // キングのチェックを調べる。
        if (basic_st_.to_move_ == WHITE) {
          if (IsAttacked(basic_st_.king_[BLACK], WHITE)) return false;
        } else {
          if (IsAttacked(basic_st_.king_[WHITE], BLACK)) return false;
        }

        // 全てクリア。
        return true;
      }

      /**
       * 現在の状態をFEN文字列にする。
       * @return 現在の状態のFEN文字列。
       */
      std::string GetFENString() const;

      // --- 利き筋関連 --- //
      /**
       * ボードの状態からビショップの利き筋を作る。
       * @param square 基点のマス。
       * @return ビショップの利き筋。
       */
      Bitboard GetBishopAttack(Square square) const {
        return Util::GetBishopMagic(square, basic_st_.blocker_[R45],
        basic_st_.blocker_[R135]);
      }
      /**
       * ボードの状態からルークの利き筋を作る。
       * @param square 基点のマス。
       * @return ルークの利き筋。
       */
      Bitboard GetRookAttack(Square square) const {
        return Util::GetRookMagic(square, basic_st_.blocker_[R0],
        basic_st_.blocker_[R90]);
      }
      /**
       * ボードの状態からクイーンの利き筋を作る。
       * @param square 基点のマス。
       * @return クイーンの利き筋。
       */
      Bitboard GetQueenAttack(Square square) const {
        return Util::GetQueenMagic
        (square, basic_st_.blocker_[R0], basic_st_.blocker_[R45],
        basic_st_.blocker_[R90], basic_st_.blocker_[R135]);
      }
      /**
       * ボードの状態からポーンの動ける位置を作る。
       * @param side ポーンのサイド。
       * @param square 基点のマス。
       * @return ポーンの動ける位置。
       */
      Bitboard GetPawnStep(Side side, Square square) const {
        return Util::GetPawnMovable(side, square, basic_st_.blocker_[R90]);
      }

      /**
       * 白のショートキャスリングが出来るかどうか判定する。
       * @return キャスリング可能ならtrue。
       */
      bool CanWhiteShortCastling() const {
        return (basic_st_.castling_rights_ & WHITE_SHORT_CASTLING)
        && !((basic_st_.blocker_[R0]
        & (Util::SQUARE[F1][R0]
        | Util::SQUARE[G1][R0]))
        || IsAttacked(E1, BLACK)
        || IsAttacked(F1, BLACK)
        || IsAttacked(G1, BLACK));
      }

      /**
       * 白のロングキャスリングが出来るかどうか判定する。
       * @return キャスリング可能ならtrue。
       */
      bool CanWhiteLongCastling() const {
        return (basic_st_.castling_rights_ & WHITE_LONG_CASTLING)
        && !((basic_st_.blocker_[R0]
        & (Util::SQUARE[D1][R0]
        | Util::SQUARE[C1][R0]
        | Util::SQUARE[B1][R0]))
        || IsAttacked(E1, BLACK)
        || IsAttacked(D1, BLACK)
        || IsAttacked(C1, BLACK));
      }

      /**
       * 黒のショートキャスリングが出来るかどうか判定する。
       * @return キャスリング可能ならtrue。
       */
      bool CanBlackShortCastling() const {
        return (basic_st_.castling_rights_ & BLACK_SHORT_CASTLING)
        && !((basic_st_.blocker_[R0]
        & (Util::SQUARE[F8][R0]
        | Util::SQUARE[G8][R0]))
        || IsAttacked(E8, WHITE)
        || IsAttacked(F8, WHITE)
        || IsAttacked(G8, WHITE));
      }

      /**
       * 黒のロングキャスリングが出来るかどうか判定する。
       * @return キャスリング可能ならtrue。
       */
      bool CanBlackLongCastling() const {
        return (basic_st_.castling_rights_ & BLACK_LONG_CASTLING)
        && !((basic_st_.blocker_[R0]
        & (Util::SQUARE[D8][R0]
        | Util::SQUARE[C8][R0]
        | Util::SQUARE[B8][R0]))
        || IsAttacked(E8, WHITE)
        || IsAttacked(D8, WHITE)
        || IsAttacked(C8, WHITE));
      }

      /**
       * キャスリングの権利を更新。
       */
      void UpdateCastlingRights() {
        if ((basic_st_.castling_rights_ & WHITE_CASTLING)) {
          // 白のキャスリングの権利。
          if (basic_st_.king_[WHITE] != E1) {
            basic_st_.castling_rights_ &= ~WHITE_CASTLING;
          } else {
            if (!(basic_st_.position_[WHITE][ROOK] & Util::SQUARE[H1][R0])) {
              basic_st_.castling_rights_ &= ~WHITE_SHORT_CASTLING;
            }
            if (!(basic_st_.position_[WHITE][ROOK] & Util::SQUARE[A1][R0])) {
              basic_st_.castling_rights_ &= ~WHITE_LONG_CASTLING;
            }
          }
        }
        if ((basic_st_.castling_rights_ & BLACK_CASTLING)) {
          // 黒のキャスリングの権利。
          if (basic_st_.king_[BLACK] != E8) {
            basic_st_.castling_rights_ &= ~BLACK_CASTLING;
          } else {
            if (!(basic_st_.position_[BLACK][ROOK] & Util::SQUARE[H8][R0])) {
              basic_st_.castling_rights_ &= ~BLACK_SHORT_CASTLING;
            }
            if (!(basic_st_.position_[BLACK][ROOK] & Util::SQUARE[A8][R0])) {
              basic_st_.castling_rights_ &= ~BLACK_LONG_CASTLING;
            }
          }
        }
      }

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
       * 指定の候補手から次の局面のマテリアルを得る。
       * @param current_material 現在のマテリアル。
       * @param move 次の局面への候補手。
       * @return 次の局面のマテリアル。
       */
      int GetNextMaterial(int current_material, Move move) const {
        Cache& cache = shared_st_ptr_->cache_;
        switch (Get<MOVE_TYPE>(move)) {
          case NORMAL:
            if ((move & MASK[PROMOTION])) {
              // プロモーション。
              return -(current_material
              + cache.material_[basic_st_.piece_board_[Get<TO>(move)]]
              + cache.material_[Get<PROMOTION>(move)] - cache.material_[PAWN]);
            }
            return -(current_material
            + cache.material_[basic_st_.piece_board_[Get<TO>(move)]]);
          case EN_PASSANT:  // アンパッサン。
            return -(current_material + cache.material_[PAWN]);
        }

        return -current_material;
      }

      /**
       * 勝敗を決めるのに十分な駒があるかどうかをチェックする。
       * @return 十分な駒があればtrue。
       */
      bool HasSufficientMaterial() const {
        // キング以外の駒のビットボード。
        Bitboard bb = basic_st_.blocker_[R0]
        & ~(basic_st_.position_[WHITE][KING]
        | basic_st_.position_[BLACK][KING]);

        if (!bb) {
          // キングのみだった場合。
          return false;
        } else if (!(bb & (bb - 1))) {
          // キング以外の、残りの駒が1つのとき。
          // 残った駒がビショップやナイトだった時は十分な駒がない。
          PieceType piece_type = basic_st_.piece_board_[Util::GetSquare(bb)];
          if ((piece_type == KNIGHT) || (piece_type == BISHOP)) {
            return false;
          }
        }

        // 十分な駒がある。
        return true;
      }

      /**
       * SEEで候補手を評価する。
       * @param move 評価したい手。
       * @return 計算後の候補手の評価。
       */
      u32 SEE(Move move) const;

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
       * アクセサ - ボードの構造体。
       * @return ボードの構造体。
       */
      const Board& board() const {return basic_st_;}
      /**
       * アクセサ - 駒の配置のビットボード。
       * @return 駒の配置のビットボード。 [サイド][駒の種類]
       */
      const Bitboard (& position() const)[NUM_SIDES][NUM_PIECE_TYPES] {
        return basic_st_.position_;
      }
      /**
       * アクセサ - 駒の種類の配置。
       * @return 駒の種類の配置。 [マス]
       */
      const PieceType (& piece_board() const)[NUM_SQUARES] {
        return basic_st_.piece_board_;
      }
      /**
       * アクセサ - サイドの配置。
       * @return サイドの配置。 [マス]
       */
      const Side (& side_board() const)[NUM_SQUARES] {
        return basic_st_.side_board_;
      }
      /**
       * アクセサ - 各サイドの駒の配置のビットボード。
       * @return 各サイドの駒の配置のビットボード。 [サイド]
       */
      const Bitboard (& side_pieces() const)[NUM_SIDES] {
        return basic_st_.side_pieces_;
      }
      /**
       * アクセサ - 全駒の配置のビットボード。 [角度]
       * @return 全駒の配置のビットボード。
       */
      const Bitboard (& blocker() const)[NUM_ROTS] {
        return basic_st_.blocker_;
      }
      /**
       * アクセサ - 各サイドのキングの位置。
       * @return 各サイドのキングの位置。 [サイド]
       */
      const Square (& king() const)[NUM_SIDES] {return basic_st_.king_;}
      /**
       * アクセサ - 手番。
       * @return 手番。
       */
      Side to_move() const {return basic_st_.to_move_;}
      /**
       * アクセサ - キャスリングの権利。
       * @return キャスリングの権利。
       */
      Castling castling_rights() const {return basic_st_.castling_rights_;}
      /**
       * アクセサ - アンパッサンの位置。
       * @return アンパッサンの位置。 なければ 0。
       */
      Square en_passant_square() const {return basic_st_.en_passant_square_;}
      /**
       * アクセサ - 50手ルールの手数。
       * @return 50手ルールの手数。
       */
      int clock() const {return basic_st_.clock_;}
      /**
       * アクセサ - 現在の手数。
       * @return 現在の手数。
       */
      int ply() const {return basic_st_.ply_;}
      /**
       * アクセサ - 各サイドの、キャスリングしたかどうかのフラグ。
       * @return 各サイドの、キャスリングしたかどうかのフラグ。 [サイド]
       */
      const bool (& has_castled() const)[NUM_SIDES] {
        return basic_st_.has_castled_;
      }
      /**
       * アクセサ - 探索中の配置のメモ。
       * @return 探索中の配置のメモ。
       */
      const Hash (& position_memo() const)[MAX_PLYS + 1] {
        return basic_st_.position_memo_;
      }
      /**
       * アクセサ - ヒストリー。
       * @return ヒストリー。 [サイド][from][to]。
       */
      const u64 (& history() const)[NUM_SIDES][NUM_SQUARES][NUM_SQUARES] {
        return const_cast<const u64 (&)[NUM_SIDES][NUM_SQUARES][NUM_SQUARES]>
        (shared_st_ptr_->history_);
      }
      /**
       * アクセサ - ヒストリーの最大値。
       * @return ヒストリーの最大値。
       */
      u64 history_max() const {return shared_st_ptr_->history_max_;}
      /**
       * アクセサ - IIDでの最善手のスタック。
       * @return IIDでの最善手のスタック。 [探索レベル]
       */
      const Move (& iid_stack() const)[MAX_PLYS + 1] {
        return const_cast<const Move (&) [MAX_PLYS + 1]>
        (shared_st_ptr_->iid_stack_);
      }
      /**
       * アクセサ - キラームーブのスタック。
       * @return キラームーブのスタック。 [探索レベル][index * 2 プライ前]
       */
      const Move (& killer_stack() const)[MAX_PLYS + 2 + 1][2] {
        return const_cast<const Move (&) [MAX_PLYS + 2 + 1][2]>
        (shared_st_ptr_->killer_stack_);
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
      /**
       * アクセサ - トランスポジションテーブル。
       * @return トランスポジションテーブル。
       */
      TranspositionTable& table() {
        return *(shared_st_ptr_->table_ptr_);
      }
      /**
       * アクセサ - 駒の情報のハッシュ値のテーブル。
       * [サイド][駒の種類][駒の位置]
       * @return 駒の情報ハッシュ値のテーブル。
       */
      const Hash (& piece_hash_value_table() const)
      [NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES] {
        return shared_st_ptr_->piece_hash_value_table_;
      }
      /**
       * アクセサ - 手番のハッシュ値のテーブル。 [サイド]
       * @return 手番のハッシュ値のテーブル。
       */
      const Hash (& to_move_hash_value_table() const)[NUM_SIDES] {
        return shared_st_ptr_->to_move_hash_value_table_;
      }
      /**
       * アクセサ - キャスリングの権利のハッシュ値のテーブル。
       * @return キャスリングの権利のハッシュ値のテーブル。
       */
      const Hash (& castling_hash_value_table() const)[16] {
        return shared_st_ptr_->castling_hash_value_table_;
      }
      /**
       * アクセサ - アンパッサンのハッシュ値のテーブル。 [マス]
       * @return アンパッサンのハッシュ値のテーブル。
       */
      const Hash (& en_passant_hash_value_table() const)[NUM_SQUARES] {
        return shared_st_ptr_->en_passant_hash_value_table_;
      }

      // ============ //
      // ミューテータ //
      // ============ //
      /**
       * ミューテータ - 手番。
       * @param to_move 手番。
       */
      void to_move(Side to_move) {
        if (!to_move) return;
        if (to_move >= NUM_SIDES) return;
        basic_st_.to_move_ = to_move;
      }
      /**
       * ミューテータ - キャスリングの権利。
       * @param castling_rights キャスリングの権利。
       */
      void castling_rights(Castling castling_rights) {
        basic_st_.castling_rights_ = castling_rights;

        UpdateCastlingRights();
      }
      /**
       * ミューテータ - アンパッサンの位置。
       * @param en_passant_square アンパッサンの位置。
       */
      void en_passant_square(Square en_passant_square) {
        // 0の場合は0。
        if (!en_passant_square) {
          basic_st_.en_passant_square_ = en_passant_square;
          return;
        }

        if ((basic_st_.blocker_[R0] & Util::SQUARE[en_passant_square][R0])) {
          // 駒があるのでアンパッサンの位置に指定できない。
          return;
        }

        Rank rank = Util::SquareToRank(en_passant_square);
        if (rank == RANK_3) {
          // すぐ上に白ポーンがいなければならない。
          Square en_passant_target = en_passant_square + 8;
          if ((basic_st_.position_[WHITE][PAWN]
          & Util::SQUARE[en_passant_target][R0])) {
            basic_st_.en_passant_square_ = en_passant_square;
            return;
          }
        } else if (rank == RANK_6) {
          // すぐ下に黒ポーンがいなければならない。
          Square en_passant_target = en_passant_square - 8;
          if ((basic_st_.position_[BLACK][PAWN]
          & Util::SQUARE[en_passant_target][R0])) {
            basic_st_.en_passant_square_ = en_passant_square;
            return;
          }
        }
      }
      /**
       * ミューテータ - 50手ルールの手数。
       * @param clock 50手ルールの手数。
       */
      void clock(int clock) {
        basic_st_.clock_ = clock < 0 ? 0 : clock;
        shared_st_ptr_->clock_history_.back() = basic_st_.clock_;
      }
      /**
       * ミューテータ - 現在の手数。
       * @param ply 現在の手数。
       */
      void ply(int ply) {
        basic_st_.ply_ = ply < 0 ? 0 : ply;
      }
      /**
       * ミューテータ - キャスリングしたかどうか。
       * @param side 設定したいサイド
       * @param has_castled キャスリングしたかどうかの配列。
       */
      void has_castled(Side side, bool has_castled) {
        basic_st_.has_castled_[side] = has_castled;
      }

    private:
      /** フレンドのデバッグ用関数。 */
      friend int DebugMain(int argc, char* argv[]);

      /** 評価関数はフレンド。 */
      friend class Evaluator;
      /** 評価関数で使うテンプレート部品。 */
      template<Side SIDE, PieceType TYPE>
      friend struct GenBitboards;
      /** 評価関数で使うテンプレート部品。 */
      template<Side SIDE, PieceType TYPE>
      friend struct CalPosition;
      /** 評価関数で使うテンプレート部品。 */
      template<Side SIDE, PieceType TYPE>
      friend struct CalMobility;
      /** 評価関数で使うテンプレート部品。 */
      template<Side SIDE, PieceType TYPE>
      friend struct CalSpecial;

      /** 指し手作成クラスはフレンド。 */
      friend class MoveMaker;

      /** PositionRecordはフレンド。 */
      friend class PositionRecord;

      /** HelperQueueはフレンド。 */
      friend class HelperQueue;

      /** Jobはフレンド。 */
      friend class Job;

      /** プライベート用コンストラクタ。 */
      ChessEngine();

      // ======== //
      // 探索関数 //
      // ======== //
      /**
       * クイース探索。
       * @param level 現在のレベル。
       * @param alpha アルファ値。
       * @param beta ベータ値。
       * @param material 現在のマテリアル。
       * @return 評価値。
       */
      int Quiesce(u32 level, int alpha, int beta, int material);

      /**
       * 通常の探索。
       * @param node_type ノードの種類。
       * @param pos_hash 現在のハッシュ。
       * @param depth 現在の深さ。
       * @param level 現在のレベル。
       * @param alpha アルファ値。
       * @param beta ベータ値。
       * @param material 現在のマテリアル。
       * @return 評価値。
       */
      int Search(NodeType node_type, Hash pos_hash, int depth,
      u32 level, int alpha, int beta, int material);

      /**
       * 探索のルート。
       * @param moves_to_search 探索する候補手。 空なら全ての候補手を探索。
       * @param shell Infoコマンドの出力用UCIShell。
       * @return 探索結果のPVライン。
       */
      PVLine SearchRoot(const std::vector<Move>& moves_to_search,
      UCIShell& shell);

      /**
       * YBWC探索用スレッド。
       * @param shell Infoコマンドの出力用UCIShell。
       */
      void ThreadYBWC(UCIShell& shell);

      /**
       * 別スレッド用探索関数。
       * @param node_type ノードの種類。
       * @param job 並列探索用仕事。
       */
      void SearchParallel(NodeType node_type, Job& job);

      /**
       * ルートノードで呼び出される、別スレッド用探索関数。
       * @param job 並列探索用仕事。
       * @param shell Infoコマンドの出力用UCIShell。
       */
      void SearchRootParallel(Job& job, UCIShell& shell);

      /**
       * 現在のノードの探索を中止すべきかどうか判断する。
       * @param job 現在のノードのジョブ。
       * @return 中止すべきならtrue。
       */
      bool JudgeToStop(Job& job);

      // ======================== //
      // その他のプライベート関数 //
      // ======================== //
      /**
       * 駒を置く。
       * @param square 駒を置きたいマス。
       * @param piece_type 置きたい駒の種類。
       * @param side 置きたい駒のサイド。
       */
      void PutPiece(Square square, PieceType piece_type, Side side=NO_SIDE);

      /**
       * 駒の位置を変える。 (移動先の駒は上書きされる。)
       * @param from 位置を変えたい駒のマス。
       * @param to 移動先。
       */
      void ReplacePiece(Square from, Square to) {
        // 移動する位置と移動先の位置が同じなら何もしない。
        if (from == to) return;

        // 移動。
        PutPiece(to, basic_st_.piece_board_[from],
        basic_st_.side_board_[from]);
        PutPiece(from, EMPTY, NO_SIDE);
      }

      /**
       * 次の一手を指す。
       * 取った駒、動かす前のキャスリングの権利、アンパッサンは
       * 「move」に記録される。
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
       * Null Moveを指す。
       * 動かす前のキャスリングの権利、アンパッサンは「move」に記録される。
       * 「move」はUnmakeNullMove()で利用する。
       * @param move 候補手。
       */
      void MakeNullMove(Move& move) {
        // 手番を変える。
        basic_st_.to_move_ = Util::GetOppositeSide(basic_st_.to_move_);

        // 情報をメモ。
        Set<CASTLING_RIGHTS>(move, basic_st_.castling_rights_);
        Set<EN_PASSANT_SQUARE>(move, basic_st_.en_passant_square_);

        // アンパッサンの位置を削除。
        basic_st_.en_passant_square_ = 0;
      }

      /**
       * MakeNullMove()で動かした手を元に戻す。
       * @param move MakeNullMove()で使用した候補手。
       */
      void UnmakeNullMove(Move move) {
        // 手番を変える。
        basic_st_.to_move_ = Util::GetOppositeSide(basic_st_.to_move_);

        // 情報を戻す。
        basic_st_.castling_rights_ = Get<CASTLING_RIGHTS>(move);
        basic_st_.en_passant_square_ = Get<EN_PASSANT_SQUARE>(move);
      }

      /**
       * 探索関数でノードを抜けるときに呼び出す関数。
       * @param score そのノードの計算結果。
       * @param level そのノードのレベル。
       * @return 引数score。
       */
      int ReturnProcess(int score, u32 level) {
        if (level >= notice_cut_level_) notice_cut_level_ = MAX_PLYS + 1;
        return score;
      }

      // ========== //
      // メンバ変数 //
      // ========== //
      // ========== //
      // 基本メンバ //
      // ========== //
      /** 基本メンバ構造体。 (ボードのコピーを用意にするための構造体) */
      struct BasicStruct : public Board {
        /** 探索中の配置のメモ。 */
        Hash position_memo_[MAX_PLYS + 1];
      } basic_st_;

      // ================================================= //
      // 共有メンバ (指定した他のエンジンと共有するメンバ) //
      // ================================================= //
      /** 共有メンバの構造体。 */
      struct SharedStruct {
        /** ヒストリー。 [サイド][from][to]。 */
        volatile u64 history_[NUM_SIDES][NUM_SQUARES][NUM_SQUARES];
        /** ヒストリーの最大値。 */
        volatile u64 history_max_;
        /** IIDでの最善手スタック。 [探索レベル] */
        volatile Move iid_stack_[MAX_PLYS + 1];
        /** キラームーブスタック。[探索レベル][index * 2 プライ前] */
        volatile Move killer_stack_[MAX_PLYS + 2 + 1][2];
        /** 現在のIterative Deepeningの深さ。 */
        volatile u32 i_depth_;
        /** 探索したノード数。 */
        volatile u64 searched_nodes_;
        /** 探索したレベル。 */
        volatile u32 searched_level_;
        /** 探索開始時間。 */
        TimePoint start_time_;

        /** 探索ストップ条件: 何が何でも探索を中断。 */
        volatile bool stop_now_;
        /** 探索ストップ条件: 最大探索ノード数。 */
        u64 max_nodes_;
        /** 探索ストップ条件: 最大探索深さ。 */
        u32 max_depth_;
        /** 探索ストップ条件: 思考終了時間。 */
        TimePoint end_time_;
        /** 探索ストップ条件: 思考時間終了フラグ。 */
        volatile bool is_time_over_;
        /** 探索ストップ条件: trueなら無限に考える。 */
        volatile bool infinite_thinking_;

        /** 指し手の履歴。 */
        std::vector<Move> move_history_;
        /** 50手ルールの手数の履歴。 */
        std::vector<int> clock_history_;
        /** 局面の履歴。 */
        std::vector<PositionRecord> position_history_;
        /** スレッドのキュー。 */
        std::unique_ptr<HelperQueue> helper_queue_ptr_;
        /** 探索関数用パラメータ。 */
        const SearchParams* search_params_ptr_;
        /** 評価関数用パラメータ。 */
        const EvalParams* eval_params_ptr_;
        /** トランスポジションテーブル。 */
        TranspositionTable* table_ptr_;

        // ============ //
        // ハッシュ関連 //
        // ============ //
        /** 駒の情報のハッシュ値のテーブル。 [サイド][駒の種類][駒の位置] */
        Hash piece_hash_value_table_[NUM_SIDES][NUM_PIECE_TYPES][NUM_SQUARES];
        /** 手番のハッシュ値のテーブル。 [サイド] */
        Hash to_move_hash_value_table_[NUM_SIDES];
        /** キャスリングの権利のハッシュ値のテーブル。 */
        Hash castling_hash_value_table_[16];
        /** アンパッサンのハッシュ値のテーブル。 */
        Hash en_passant_hash_value_table_[NUM_SQUARES];

        // ========== //
        // キャッシュ //
        // ========== //
        /** 探索関数、評価関数用キャッシュ。 */
        Cache cache_;

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

        /** パラメータをキャッシュする。 */
        void CacheParams() {
          if (search_params_ptr_) {
            cache_.CacheSearchParams(*search_params_ptr_);
          }
          if (eval_params_ptr_) {
            cache_.CacheEvalParams(*eval_params_ptr_);
          }

          COPY_ARRAY(cache_.piece_hash_value_table_,
          piece_hash_value_table_);
          COPY_ARRAY(cache_.to_move_hash_value_table_,
          to_move_hash_value_table_);
          COPY_ARRAY(cache_.castling_hash_value_table_,
          castling_hash_value_table_);
          COPY_ARRAY(cache_.en_passant_hash_value_table_,
          en_passant_hash_value_table_);

          cache_.max_depth_ = max_depth_;
          cache_.max_nodes_ = max_nodes_;
        }

        /**
         * 定期処理関数。
         * - 思考時間終了判定。
         * - 定期情報出力。
         * @param shell 出力関数のあるUCIShell。
         */
        void ThreadPeriodicProcess(UCIShell& shell);
      };
      /** 共有メンバの構造体。 */
      std::shared_ptr<SharedStruct> shared_st_ptr_;

      // ===================================================== //
      // 固有メンバ (他のエンジンとコピーも共有もしないメンバ) //
      // ===================================================== //
      /** 今ヌルムーブのサーチ中かどうかのフラグ。 */
      bool is_null_searching_;
      /** 探索中のテーブルへのポインタ。 */
      TranspositionTable* table_ptr_;
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
      /** ベータカット通知。 カットされたレベルが記録される。 */
      volatile u32 notice_cut_level_;
  };
}  // namespace Sayuri

#endif
