/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Hironori Ishibashi
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
 * @file pgn.h
 * @author Hironori Ishibashi
 * @brief PGNパーサ。
 */

#ifndef PGN_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define PGN_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <memory>
#include <utility>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  struct MoveNode;
  /** 指し手のノードのポインタ。 */
  using MoveNodePtr = std::shared_ptr<MoveNode>;

  class PGNGame;
  /** PGNゲームのポインタ。 */
  using PGNGamePtr = std::shared_ptr<PGNGame>;

  /** PGNのヘッダ。 */
  using PGNHeader = std::map<std::string, std::string>;

  /** 指し手のノード。 */
  struct MoveNode {
    // --- 前後 --- //
    /** 次の手。 */
    MoveNodePtr next_;
    /** 前の手。 */
    MoveNode* prev_ = nullptr;

    // --- 左右 --- //
    /** 代替手。 */
    MoveNodePtr alt_r_;
    /** 元の手。 */
    MoveNode* alt_l_ = nullptr;

    /** 指し手の文字列。 */
    std::string text_;
    /** コメントのベクトル。 */
    std::vector<std::string> comment_vec_;

    static MoveNodePtr Clone(const MoveNode& node) {
      MoveNodePtr ret_ptr(new MoveNode());

      if (node.next_) {
        ret_ptr->next_ = Clone(*(node.next_));
        ret_ptr->next_->prev_ = ret_ptr.get();
      }
      if (node.alt_r_) {
        ret_ptr->alt_r_ = Clone(*(node.alt_r_));
        ret_ptr->alt_r_->alt_l_ = ret_ptr.get();
      }

      return ret_ptr;
    }
  };

  /** ゲーム。 */
  class PGNGame {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      PGNGame() : move_tree_ptr_(nullptr) {}
      /**
       * コピーコンストラクタ。
       * @param game コピー元。
       */
      PGNGame(const PGNGame& game) :
      header_vec_(game.header_vec_),
      move_tree_ptr_(MoveNode::Clone(*(game.move_tree_ptr_))),
      comment_vec_(game.comment_vec_) {}
      /**
       * ムーブコンストラクタ。
       * @param game ムーブ元。
       */
      PGNGame(PGNGame&& game) :
      header_vec_(std::move(game.header_vec_)),
      move_tree_ptr_(game.move_tree_ptr_),
      comment_vec_(std::move(game.comment_vec_)) {}
      /**
       * コピー代入演算子。
       * @param game コピー元。
       */
      PGNGame& operator=(const PGNGame& game) {
        header_vec_ = game.header_vec_;
        move_tree_ptr_ = MoveNode::Clone(*(game.move_tree_ptr_));
        comment_vec_ = game.comment_vec_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param game ムーブ元。
       */
      PGNGame& operator=(PGNGame&& game) {
        header_vec_ = std::move(game.header_vec_);
        move_tree_ptr_ = game.move_tree_ptr_;
        comment_vec_ = std::move(game.comment_vec_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~PGNGame() {}

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - ヘッダのベクトル。
       * @return ヘッダのベクトル。
       */
      const std::vector<PGNHeader>& header_vec() const {
        return header_vec_;
      }
      /**
       * アクセサ - 指し手の木。
       * @return 指し手の木。
       */
      const MoveNode& move_tree() const {return *move_tree_ptr_;}
      /**
       * アクセサ - コメントのベクトル。
       * @return コメントのベクトル。
       */
      const std::vector<std::string>& comment_vec() const {
        return comment_vec_;
      }

      // ============ //
      // ミューテータ //
      // ============ //
      /**
       * ミューテータ - ヘッダのベクトル。
       * @param header_vec ヘッダのベクトル。
       */
      void header_vec(const std::vector<PGNHeader>& header_vec) {
        header_vec_ = header_vec;
      }
      /**
       * ミューテータ - 指し手の木。
       * @param move_tree_ptr 指し手の木。
       */
      void move_tree(MoveNodePtr move_tree_ptr) {
        move_tree_ptr_ = move_tree_ptr;
      }
      /**
       * ミューテータ - コメントのベクトル。
       * @param comment_vec コメントのベクトル。
       */
      void comment_vec(const std::vector<std::string>& comment_vec) {
        comment_vec_ = comment_vec;
      }

    private:
      // フレンド。
      friend class PGN;

      // ========== //
      // メンバ変数 //
      // ========== //
      /** ヘッダのベクトル。 */
      std::vector<PGNHeader> header_vec_;
      /** 指し手の木。 */
      MoveNodePtr move_tree_ptr_;
      /** コメントのベクトル。 */
      std::vector<std::string> comment_vec_;
  };

  /** PGNパーサ。 */
  class PGN {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      PGN() {}
      /**
       * コピーコンストラクタ。
       * @param pgn コピー元。
       */
      PGN(const PGN& pgn) :
      game_vec_(pgn.game_vec_), comment_vec_(pgn.comment_vec_) {}
      /**
       * ムーブコンストラクタ。
       * @param pgn ムーブ元。
       */
      PGN(PGN&& pgn) :
      game_vec_(std::move(pgn.game_vec_)), comment_vec_(pgn.comment_vec_) {}
      /**
       * コピー代入演算子。
       * @param pgn コピー元。
       */
      PGN& operator=(const PGN& pgn) {
        game_vec_ = pgn.game_vec_;
        comment_vec_ = pgn.comment_vec_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param pgn ムーブ元。
       */
      PGN& operator=(PGN&& pgn) {
        game_vec_ = std::move(pgn.game_vec_);
        comment_vec_ = std::move(pgn.comment_vec_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~PGN() {}

      // ====== //
      // 演算子 //
      // ====== //
      // ゲームにアクセスする。
      // @param index ゲームのインデックス。
      const PGNGame& operator[](int index) const {
        return *(game_vec_[index]);
      }

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - ゲームのベクトル。
       * @return ゲームのベクトル。
       */
      const std::vector<PGNGamePtr> game_vec() const {
        return game_vec_;
      }
      /**
       * アクセサ - コメントのベクトル。
       * @return コメントのベクトル。
       */
      const std::vector<std::string>& comment_vec() const {
        return comment_vec_;
      }

      // ============ //
      // ミューテータ //
      // ============ //
      /**
       * ミューテータ - ゲームのベクトル。
       * @param game_vec ゲームのベクトル。
       */
      void game_vec(const std::vector<PGNGamePtr>& game_vec) {
        game_vec_ = game_vec;
      }
      /**
       * ミューテータ - コメントのベクトル。
       * @param comment_vec コメントのベクトル。
       */
      void comment_vec(const std::vector<std::string>& comment_vec) {
        comment_vec_ = comment_vec;
      }

    private:
      /** フレンド。 */
      friend int DebugMain(int, char**);

      /**
       * 字句解析する。
       * @param str 字句解析したい文字列。
       * @return トークンのキュー。
       */
      static std::queue<std::string> Tokenize(const std::string& str);

      // ========== //
      // メンバ変数 //
      // ========== //
      /** ゲームのベクトル。 */
      std::vector<PGNGamePtr> game_vec_;
      /** コメントのベクトル。 */
      std::vector<std::string> comment_vec_;
  };
}  // namespace Sayuri

#endif
