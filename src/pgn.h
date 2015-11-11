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
    MoveNodePtr alt_;
    /** 元の手。 */
    MoveNode* orig_ = nullptr;

    /** 指し手の文字列。 */
    std::string text_;
    /** コメントのベクトル。 */
    std::vector<std::string> comment_vec_;

    static MoveNodePtr Clone(const MoveNode& node) {
      MoveNodePtr ret_ptr(new MoveNode());
      ret_ptr->text_ = node.text_;
      ret_ptr->comment_vec_ = node.comment_vec_;

      if (node.next_) {
        ret_ptr->next_ = Clone(*(node.next_));
        ret_ptr->next_->prev_ = ret_ptr.get();
      }
      if (node.alt_) {
        ret_ptr->alt_ = Clone(*(node.alt_));
        ret_ptr->alt_->orig_ = ret_ptr.get();
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
      PGNGame() : move_tree_ptr_(nullptr) , current_node_ptr_(nullptr) {}
      /**
       * コピーコンストラクタ。
       * @param game コピー元。
       */
      PGNGame(const PGNGame& game) :
      header_(game.header_),
      move_tree_ptr_(MoveNode::Clone(*(game.move_tree_ptr_))),
      result_(game.result_),
      comment_vec_(game.comment_vec_),
      current_node_ptr_(move_tree_ptr_.get()) {}
      /**
       * ムーブコンストラクタ。
       * @param game ムーブ元。
       */
      PGNGame(PGNGame&& game) :
      header_(std::move(game.header_)),
      move_tree_ptr_(game.move_tree_ptr_),
      result_(std::move(game.result_)),
      comment_vec_(std::move(game.comment_vec_)),
      current_node_ptr_(move_tree_ptr_.get()) {}
      /**
       * コピー代入演算子。
       * @param game コピー元。
       */
      PGNGame& operator=(const PGNGame& game) {
        header_ = game.header_;
        move_tree_ptr_ = MoveNode::Clone(*(game.move_tree_ptr_));
        result_ = game.result_;
        comment_vec_ = game.comment_vec_;
        current_node_ptr_ = move_tree_ptr_.get();
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param game ムーブ元。
       */
      PGNGame& operator=(PGNGame&& game) {
        header_ = std::move(game.header_);
        move_tree_ptr_ = game.move_tree_ptr_;
        result_ = std::move(game.result_);
        comment_vec_ = std::move(game.comment_vec_);
        current_node_ptr_ = move_tree_ptr_.get();
        return *this;
      }
      /** デストラクタ。 */
      virtual ~PGNGame() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 次のノードへ移動。
       * @return 移動できればtrue。
       */
      bool Next() {
        if (current_node_ptr_->next_) {
          current_node_ptr_ = current_node_ptr_->next_.get();
          return true;
        }
        return false;
      }
      /**
       * 前のノードへ移動。
       * @return 移動できればtrue。
       */
      bool Prev() {
        if (current_node_ptr_->prev_) {
          current_node_ptr_ = current_node_ptr_->prev_;
          return true;
        }
        return false;
      }
      /**
       * 代替手のノードへ移動。
       * @return 移動できればtrue。
       */
      bool Alt() {
        if (current_node_ptr_->alt_) {
          current_node_ptr_ = current_node_ptr_->alt_.get();
          return true;
        }
        return false;
      }
      /**
       * オリジナルのノードへ移動。
       * @return 移動できればtrue。
       */
      bool Orig() {
        if (current_node_ptr_->orig_) {
          current_node_ptr_ = current_node_ptr_->orig_;
          return true;
        }
        return false;
      }
      /**
       * 前のノードへ移動。 (代替手からも移動できる)
       * @return 移動できればtrue。
       */
      bool Back() {
        MoveNode* temp = current_node_ptr_;
        while (true) {
          if (temp->orig_) {
            temp = temp->orig_;
          } else if (temp->prev_) {
            temp = temp->prev_;
            break;
          } else {
            break;
          }
        }
        if (temp != current_node_ptr_) {
          current_node_ptr_ = temp;
          return true;
        }
        return false;
      }
      /**
       * ルートに戻る。
       * @return 移動できればtrue。
       */
      bool Rewind() {
        current_node_ptr_ = move_tree_ptr_.get();
        return true;
      }

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - ヘッダのベクトル。
       * @return ヘッダのベクトル。
       */
      const PGNHeader header() const {
        return header_;
      }
      /**
       * アクセサ - 指し手の木。
       * @return 指し手の木。
       */
      const MoveNode& move_tree() const {return *move_tree_ptr_;}
      /**
       * アクセサ - 結果の文字列。
       * @return 結果の文字列。
       */
      const std::string& result() const {return result_;}
      /**
       * アクセサ - コメントのベクトル。
       * @return コメントのベクトル。
       */
      const std::vector<std::string>& comment_vec() const {
        return comment_vec_;
      }
      /**
       * アクセサ - 現在の指し手のノードのポインタ。
       * @return 現在の指し手のノードのポインタ。
       */
      const MoveNode* current_node_ptr() const {return current_node_ptr_;}

      // ============ //
      // ミューテータ //
      // ============ //
      /**
       * ミューテータ - ヘッダのベクトル。
       * @param header_vec ヘッダのベクトル。
       */
      void header(const PGNHeader& header) {
        header_ = header;
      }
      /**
       * ミューテータ - 指し手の木。
       * @param move_tree_ptr 指し手の木。
       */
      void move_tree(MoveNodePtr move_tree_ptr) {
        move_tree_ptr_ = move_tree_ptr;
        current_node_ptr_ = move_tree_ptr_.get();
      }
      /**
       * ミューテータ - 結果の文字列。
       * @param result 結果の文字列。
       */
      void result(const std::string result) {
        result_ = result;
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
      friend int DebugMain(int, char**);

      // ========== //
      // メンバ変数 //
      // ========== //
      /** PGNヘッダ。 */
      PGNHeader header_;
      /** 指し手の木。 */
      MoveNodePtr move_tree_ptr_;
      /** 結果の文字列。 */
      std::string result_;
      /** コメントのベクトル。 */
      std::vector<std::string> comment_vec_;

      /** 現在の指し手のノード。 */
      MoveNode* current_node_ptr_;
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

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * パースする。
       * @param pgn_str パースするPGN文字列。
       */
      void Parse(const std::string& pgn_str);

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
