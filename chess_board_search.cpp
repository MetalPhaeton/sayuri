/* chess_board_search.cpp: チェスボードの探索に関するもの。
   copyright (c) 2011 石橋宏之利
 */

#include "chess_board.h"

#include <iostream>
#include "chess_def.h"
#include "chess_util.h"
#include "transposition_table.h"

#include "misaki_debug.h"

namespace Misaki {
  /********************
   * 探索に使う関数。 *
   ********************/
  // MCapを得る。
  int ChessBoard::GetMCap(move_t move) const {
    if (move.move_type_ == EN_PASSANT) return SCORE_PAWN;

    static const int score_array[NUM_PIECE_TYPES] = {
      0,
      SCORE_PAWN,
      SCORE_KNIGHT,
      SCORE_BISHOP,
      SCORE_ROOK,
      SCORE_QUEEN,
      SCORE_KING
    };

    return score_array[piece_board_[move.goal_square_]];
  }
  // クイース探索。
  int ChessBoard::Quiesce(int level, int depth, int alpha, int beta,
  hash_key_t key, TranspositionTable& table, const EvalWeights& weights) {
    // 評価値。
    int score = 0;

    // トランスポジションテーブルを調べる。
    TranspositionTableSlot* slot = NULL;
    try {
      slot = &(table.GetSameSlot(key, level, depth, to_move_));
    } catch (...) {
      // 何もしない。
    }
    if (slot) {
      int upper_bound = slot->upper_bound();
      int lower_bound = slot->lower_bound();
      if (lower_bound >= beta) {
        return lower_bound;
      }
      if (upper_bound <= alpha) {
        return upper_bound;
      }
      if (lower_bound == upper_bound) {
        return lower_bound;
      }
      alpha = (lower_bound >= alpha ? lower_bound : alpha);
      beta = (upper_bound <= beta ? upper_bound : beta);
    }

    // Stand Pat。
    int stand_pat = EvalAll(to_move_, weights);
    if (stand_pat >= beta) {
      return beta;
    }
    if (stand_pat > alpha) alpha = stand_pat;

    // 自分のサイドと敵のサイド。
    side_t side = to_move_;
    side_t enemy_side = static_cast<side_t>(side ^ 0x3);

    // お互い十分な駒がなければ評価値を返す。
    if (!IsEnoughPieces(side) && !IsEnoughPieces(enemy_side)) {
      return stand_pat;
    }

    // 合法手がなければ評価値を返す。
    if (!HasLegalMove(side)) {
      return stand_pat;
    }

    // チェックされているかどうかを得る。
    bool is_check = IsAttacked(king_[side], enemy_side);

    // チェックされていたり、取る手があれば展開する。
    int move_count = 0;
    if (is_check) {
      move_count = GenCheckEscapeMove(level);
    } else {
      move_count = GenCaptureMove(level);
    }
    // 手がなければ評価値を返す。
    if (!move_count) {
      return stand_pat;
    }

    // 手に簡易点数を付ける。
    GiveQuickScore(key, level, depth, side, table);

    // アルファベータでクイースする。
    int save_alpha = alpha;  // アルファ値をセーブする。
    score = SCORE_DRAW;  // 評価値。ステールメイトの評価で初期化する。
    move_t move;  // 手。
    move.all_ = 0;
    move_t candidate_move;  // このノードの最善手。
    candidate_move.all_ = 0;
    hash_key_t next_key;
    int m_cap;
    int material = GetMaterial(side);
    while (stack_ptr_[level] != tree_ptr_[level]) {
      // タイムアウトなら探索を止める。
      if (IsTimeOut()) {
        ClearMoves(level);
        return alpha;
      }

      move = PopBestMove(level);

      // 次の局面のハッシュキーを得る。
      next_key = GetNextKey(key, move);
      m_cap = GetMCap(move);

      // クイースする。
      MakeMove(move);
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }
      // Futility Pruning。
      if ((level != 0) && !is_check
      && ((material + m_cap + SCORE_BISHOP) <= alpha)) {
        UnmakeMove(move);
        continue;
      }
      score = -Quiesce(level + 1, depth - 1, -beta, -alpha,
      next_key, table, weights);
      UnmakeMove(move);

      // カットとアルファの更新。
      if (score > alpha) {
        // カット
        if (score >= beta) {
          // テーブルの更新。
          if (slot) {
            slot->lower_bound(score);
            slot->upper_bound(INFINITE);
            slot->best_move(move);
          } else {
            table.Add(key, level, depth, side, INFINITE, score, move);
          }

          ClearMoves(level);
          return beta;
        }

        // 最善手をの更新。
        candidate_move = move;

        // アルファの更新。
        alpha = score;
      }
    }

    // テーブルを更新。
    if (slot) {
      if (alpha <= save_alpha) {
        slot->lower_bound(-INFINITE);
        slot->upper_bound(alpha);
        slot->best_move(candidate_move);
      } else {
        slot->lower_bound(alpha);
        slot->upper_bound(alpha);
        slot->best_move(candidate_move);
      }
    } else {
      if (alpha <= save_alpha) {
        table.Add(key, level, depth, side, alpha, -INFINITE, candidate_move);
      } else {
        table.Add(key, level, depth, side, alpha, alpha, candidate_move);
      }
    }

    ClearMoves(level);
    return alpha;
  }
  // 探索する。
  int ChessBoard::Search(int level, int depth, int alpha, int beta,
  bool is_null_move, hash_key_t key, TranspositionTable& table,
  const EvalWeights& weights) {
    // TEST
    extern int max_level;
    if (level > max_level) {
      max_level = level;
    }

    // 評価値。
    int score = 0;

    // トランスポジションテーブルを調べる。
    TranspositionTableSlot* slot = NULL;
    try {
      slot = &(table.GetSameSlot(key, level, depth, to_move_));
    } catch (...) {
      // 何もしない。
    }
    if (slot) {
      int upper_bound = slot->upper_bound();
      int lower_bound = slot->lower_bound();
      if (lower_bound >= beta) {
        if (level == 0) {
          best_move_ = slot->best_move();
          best_score_ = lower_bound;
        }
        return lower_bound;
      }
      if (upper_bound <= alpha) {
        if (level == 0) {
          best_move_ = slot->best_move();
          best_score_ = upper_bound;
        }
        return upper_bound;
      }
      if (lower_bound == upper_bound) {
        if (level == 0) {
          best_move_ = slot->best_move();
          best_score_ = lower_bound;
        }
        return lower_bound;
      }
      alpha = (lower_bound >= alpha ? lower_bound : alpha);
      beta = (upper_bound <= beta ? upper_bound : beta);
    }

    // depthが0以下ならクイース。
    if (depth <= 0) {
      score = Quiesce(level, depth, alpha, beta, key, table, weights);
      return score;
    }

    // このノードのサイドと、その敵のサイド。
    side_t side = to_move_;
    side_t enemy_side = static_cast<side_t>(side ^ 0x3);

    // 合法手がなければ評価を返す。
    if (!HasLegalMove(side)) {
      return EvalAll(side, weights);
    }

    // このノードがチェックされているかどうか。
    bool is_check = IsAttacked(king_[side], enemy_side);

    // 手を生成する。
    int move_count = 0;
    move_count = GenMove(level);
    if (!move_count) {
      return EvalAll(side, weights);
    }

    // 手に簡易点数を付ける。
    GiveQuickScore(key, level, depth, side, table);

    // このノードのマテリアル。
    int material = GetMaterial(side);

    // Null Move Pruning。
    // levelが0でない場合。
    // この探索がNull Move Pruningでない場合。
    // ツークツワンク以外の場合。（展開した手が1つより大きい場合。）
    // depthが3より大きい場合。
    // チェックがかかっていない場合。
    // マテリアルがルークの評価値以上の場合。
    if ((level != 0) && !is_null_move && (move_count > 1) && (depth > 3)
    && !is_check && (material >= SCORE_ROOK)) {
      // Null Moveを準備。
      move_t null_move;
      null_move.all_ = 0;
      null_move.move_type_ = NULL_MOVE;

      // Null Moveをする。
      MakeMove(null_move);

      // Zero Windowで探索する。
      int null_score = -Search(level + 1, depth - 3, -beta, 1 - beta, true,
      key, table, weights);

      // 元に戻す。
      UnmakeMove(null_move);

      // カット。
      if (null_score >= beta) {
        ClearMoves(level);
        return null_score;
      }
    }

    // PVSearch。
    int save_alpha = alpha;  // アルファ値をセーブする。
    score = SCORE_DRAW;  // 評価値。ステールメイトの評価で初期化する。
    move_t move;  // 手。
    move.all_ = 0;
    move_t candidate_move;  // 最善手。
    candidate_move.all_ = 0;
    int m_cap;  // MCap。
    bool full_search = true;  // 全探索するフラグ。
    hash_key_t next_key;  // 次の局面のハッシュキー。
    while (stack_ptr_[level] != tree_ptr_[level]) {
      // タイムアウトなら探索を止める。
      if (IsTimeOut()) {
        ClearMoves(level);
        return alpha;
      }
      // 手をポップ。
      move = PopBestMove(level);

      // MCapを得る。
      m_cap = GetMCap(move);

      // 次の局面のハッシュキーを得る。
      next_key = GetNextKey(key, move);

      MakeMove(move);

      // レベルが0で、その手がチェックメイトなら
      // その手を最善手にして返す。
      if ((level == 0) && IsCheckmated()) {
        UnmakeMove(move);
        best_score_ = SCORE_WIN;
        best_move_ = move;
        if (slot) {
          slot->lower_bound(best_score_);
          slot->upper_bound(best_score_);
          slot->best_move(best_move_);
        } else {
          table.Add(key, level, depth, side,
          best_score_, best_score_, best_move_);
        }
        ClearMoves(level);
        return best_score_;
      }

      // チェックがかかっていれば無視。
      if (IsAttacked(king_[side], enemy_side)) {
        UnmakeMove(move);
        continue;
      }

      // Futility Pruning。
      if ((level != 0) && !is_check && (depth == 1)
      && ((material + m_cap + SCORE_BISHOP) <= alpha)) {
        UnmakeMove(move);
        continue;
      }

      // 探索する。
      if (full_search) {
        score = -Search(level + 1, depth - 1, -beta, -alpha,
        is_null_move, next_key, table, weights);
      } else {
        // Zero Windowで探索。
        score = -Search(level + 1, depth - 1, -alpha - 1, -alpha,
        is_null_move, next_key, table, weights);
        // 範囲内ならFull Windowで再探索。
        if ((score > alpha) && (score < beta)) {
          score = -Search(level + 1, depth - 1, -beta, -alpha,
          is_null_move, next_key, table, weights);
        }
      }

      UnmakeMove(move);

      // カットとアルファの更新。
      if (score > alpha) {
        // カット
        if (score >= beta) {
          ClearMoves(level);
          // テーブルを更新。
          if (slot) {
            slot->lower_bound(score);
            slot->upper_bound(INFINITE);
            slot->best_move(move);
          } else {
            table.Add(key, level, depth, side, INFINITE, score, move);
          }
          return score;
        }

        // レベル0なら最善手を更新。
        if (level == 0) {
          best_move_ = move;
          best_score_ = score;
        }

        // このノードの最善手を得る。
        candidate_move = move;

        // アルファの更新。
        alpha = score;
        full_search = false;
      }
    }

    // テーブルを更新。
    if (slot) {
      if (alpha <= save_alpha) {
        slot->lower_bound(-INFINITE);
        slot->upper_bound(alpha);
        slot->best_move(candidate_move);
      } else {
        slot->lower_bound(alpha);
        slot->upper_bound(alpha);
        slot->best_move(candidate_move);
      }
    } else {
      if (alpha <= save_alpha) {
        table.Add(key, level, depth, side, alpha, -INFINITE, candidate_move);
      } else {
        table.Add(key, level, depth, side, alpha, alpha, candidate_move);
      }
    }

    ClearMoves(level);
    return alpha;
  }
}  // Misaki
