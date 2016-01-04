/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2016 Hironori Ishibashi
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
 * @file pgn.cpp
 * @author Hironori Ishibashi
 * @brief PGNパーサの実装。
 */

#include "pgn.h"

#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <queue>
#include "common.h"

/** Sayuri 名前空間。 */
namespace Sayuri {
  // 字句解析する。
  std::queue<std::string> PGN::Tokenize(const std::string& str) {
    // 空白文字。
    std::set<char> blank_c {' ', '\n', '\r', '\t', '\f', '\a', '\b'};
    // 区切り文字。
    std::set<char> delim_c {
      ' ', '\n', '\r', '\t', '\f', '\a', '\b', '.'
    };
    // 区切り文字でかつトークン。
    std::set<char> delim_token_c {'(', ')'};

    // 結果文字。
    std::set<std::string> result_str {"0-1", "1-0", "1/2-1/2", "*"};

    // 評価文字列かどうかを判定。
    auto is_assessment = [](const std::string& str) -> bool {
      for (auto c : str) {
        if ((c != '!') && (c != '?')) return false;
      }
      return true;
    };

    // 指し手のリストの文字列かどうかを判断。
    auto is_movetext =
    [&result_str, &is_assessment](const std::string& str) -> bool {
      if (Util::IsAlgebraicNotation(str)
      || (result_str.find(str) != result_str.end())
      || is_assessment(str)) {
        return true;
      }
      return false;
    };

    std::queue<std::string> ret;

    // 頭とおしりの空白文字を削除。
    auto remove_blank = [&blank_c](const std::string& str) -> std::string {
      std::string ret = "";

      // 頭を削除。
      bool start = false;
      std::ostringstream oss;
      for (auto c : str) {
        if (start) {
          oss << c;
        } else {
          if (blank_c.find(c) == blank_c.end()) {oss << c; start = true;}
        }
      }
      ret = oss.str();

      // おしりを削除。
      while (ret.size()) {
        if (blank_c.find(ret.back()) == blank_c.end()) break;
        ret.pop_back();
      }

      return ret;
    };

    std::ostringstream oss;
    bool in_comment = false;
    bool in_line_comment = false;
    bool in_tag_value = false;
    bool in_tag = false;
    for (auto c : str) {
      if (in_comment) {  // コメント。
        if (c == '}') {
          if (oss.str().size()) {
            ret.push(remove_blank(oss.str()));
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_comment = false;
        } else {
          oss << c;
        }
      } else if (in_line_comment) {  // 行コメント。
        if (c == '\n') {
          if (oss.str().size()) {
            ret.push(remove_blank(oss.str()));
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_line_comment = false;
        } else {
          oss << c;
        }
      } else if (in_tag_value) {  // タグの値。
        if (c == '"') {
          if (oss.str().size()) {
            ret.push(oss.str());
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_tag_value = false;
        } else {
          oss << c;
        }
      } else if (in_tag) {  // タグ。
        if (c == '"') {  // タグの値の始まり。
          if (oss.str().size()) {
            ret.push(remove_blank(oss.str()));
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_tag_value = true;
        } else if (c == ']') {  // タグの終了。
          if (oss.str().size()) {
            ret.push(remove_blank(oss.str()));
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_tag = false;
        } else {
          oss << c;
        }
      } else {  // それ以外。
        if (c == ';') {  // 行コメント開始。
          if (oss.str().size()) {
            std::string temp = remove_blank(oss.str());
            if (is_movetext(temp)) {
              ret.push(temp);
            }
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_line_comment = true;
        } else if (c == '{') {  // コメント開始。
          if (oss.str().size()) {
            std::string temp = remove_blank(oss.str());
            if (is_movetext(temp)) {
              ret.push(temp);
            }
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_comment = true;
        } else if (c == '['){  // タグ開始。
          if (oss.str().size()) {
            std::string temp = remove_blank(oss.str());
            if (is_movetext(temp)) {
              ret.push(temp);
            }
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_tag = true;
        } else if (delim_c.find(c) != delim_c.end()) {  // 区切り文字。
          if (oss.str().size()) {
            std::string temp = remove_blank(oss.str());
            if (is_movetext(temp)) {
              ret.push(temp);
            }
            oss.str("");
          }
        } else if (delim_token_c.find(c) != delim_token_c.end()) {
          // 区切りでかつトークン。
          if (oss.str().size()) {
            std::string temp = remove_blank(oss.str());
            if (is_movetext(temp)) {
              ret.push(temp);
            }
            oss.str("");
          }
          ret.push(std::string(1, c));
        } else if ((c == '!') || (c == '?')) {  // 評価文字。
          // oss内の文字が評価文字列なら追加。 違うなら区切って追加。
          std::string temp = remove_blank(oss.str());
          if (is_assessment(temp)) {
            oss << c;
          } else {
            if (oss.str().size()) {
              if (is_movetext(temp)) {
                ret.push(temp);
              }
              oss.str("");
            }
            oss << c;
          }
        } else {  // それ以外。
          oss << c;
        }
      }
    }

    // 最後にプッシュ。
    if (oss.str().size()) {
      std::string temp = remove_blank(oss.str());
      if (in_comment || in_line_comment || in_tag) {
        ret.push(temp);
      } else {
        if (is_movetext(temp)) {
          ret.push(temp);
        }
      }
    }

    return ret;
  }

  namespace {
    // コメント開始かどうか。
    bool IsCommentStarting(const std::string& str) {
      return (str == "{") || (str == ";");
    }

    // コメントを得る。
    std::string GetComment(std::queue<std::string>& token_queue) {
      std::string front = token_queue.front();
      std::ostringstream oss;

      if (IsCommentStarting(front)) {
        std::string end_str = "";

        if (front == "{") {
          end_str = "}";
        } else if (front == ";") {
          end_str = "\n";
        }

        token_queue.pop();
        while (!(token_queue.empty())) {
          front = token_queue.front();
          token_queue.pop();
          if (front == end_str) break;
          oss << front;
        }
      }
      return oss.str();
    }

    // 1ヘッダをパース。
    std::pair<std::string, std::string>
    ParseOneHeader(std::queue<std::string>& token_queue) {
      std::pair<std::string, std::string> ret;

      std::string front = token_queue.front();

      if (front == "[") {
        token_queue.pop();

        // タグ名。
        std::ostringstream oss;
        while (!(token_queue.empty())) {
          front = token_queue.front();
          token_queue.pop();

          if ((front == "]") || (front == "\"")) break;

          oss << front;
        }
        ret.first = oss.str();  // セット。
        oss.str("");

        // 値。
        if (front == "\"") {
          while (!(token_queue.empty())) {
            front = token_queue.front();
            token_queue.pop();

            if ((front == "]") || (front == "\"")) break;

            oss << front;
          }
          ret.second = oss.str();  // セット。
        }

        // ヘッダ終了まで空ループ。
        while (!(token_queue.empty())) {
          front = token_queue.front();
          token_queue.pop();

          if (front == "]") break;
        }
      }

      return ret;
    }

    // 指し手のリストをパース。
    MoveNodePtr ParseMoveNode(std::queue<std::string>& token_queue) {
      // 終了判定。
      auto is_end_str = [](const std::string& str) -> bool {
        return (str == ")") || (str == "*") || (str == "1-0")
        || (str == "0-1") || (str == "1/2-1/2");
      };

      // 指し手の評価かどうか。
      auto is_assessment = [](const std::string& str) -> bool {
        for (auto c : str) {
          if ((c != '!') && (c != '?')) return false;
        }
        return true;
      };

      std::string front = token_queue.front();

      // いきなり終了なら終わる。
      if (is_end_str(front)) {
        if (front == ")") token_queue.pop();
        return MoveNodePtr(nullptr);
      }

      MoveNodePtr ret_ptr(new MoveNode());
      std::vector<std::string> ret_comment_vec;

      // 左右方向にパース。
      bool has_parsed_current = false;
      MoveNode* current_alt = ret_ptr.get();
      while (!(token_queue.empty())) {
        front  = token_queue.front();

        // 終了なら抜ける。
        if (is_end_str(front)) {
          if (front == ")") token_queue.pop();
          break;
        }

        // 次のゲームが始まったら抜ける。
        if (front == "[") break;

        if (!has_parsed_current) {  // まだ現在のノードをパースしていない。
          if (Util::IsAlgebraicNotation(front)) {
            ret_ptr->text_ = front;
            has_parsed_current = true;
          }
          token_queue.pop();
        } else {   // 現在のノードのパース後。
          if (IsCommentStarting(front)) {
            ret_ptr->comment_vec_.push_back(GetComment(token_queue));
          } else if (is_assessment(front)) {
            ret_ptr->comment_vec_.push_back(front);
            token_queue.pop();
          } else if (Util::IsAlgebraicNotation(front)) {  // 次の手。
            // 次の手をパースしたらループを抜ける。
            ret_ptr->next_ = ParseMoveNode(token_queue);
            if (ret_ptr->next_) {
              ret_ptr->next_->prev_ = ret_ptr.get();
            }
            break;
          } else if (front == "(") {  // 代替手の合図。
            token_queue.pop();
            current_alt->alt_ = ParseMoveNode(token_queue);
            if (current_alt->alt_) {
              current_alt->alt_->orig_ = current_alt;
              current_alt = current_alt->alt_.get();
            }
          }
        }
      }

      return ret_ptr;
    }

    // 1ゲームをパース。
    PGNGamePtr ParseOneGame(std::queue<std::string>& token_queue) {
      PGNGamePtr ret_ptr(new PGNGame());
      PGNHeader ret_header;
      std::vector<std::string> ret_comment_vec;

      bool has_moves_started = false;
      bool has_moves_ended = false;
      while (!(token_queue.empty())) {
        std::string front = token_queue.front();
        if (!has_moves_started) {  // 指し手のリスト前。
          if (front == "[") {  // ヘッダ。
            ret_header.emplace(ParseOneHeader(token_queue));
          } else if (IsCommentStarting(front)) {  // コメント。
            ret_comment_vec.push_back(GetComment(token_queue));
          } else {
            has_moves_started = true;
          }
        } else if (has_moves_ended){  // 指し手のリスト終了。
          if (front == "[") {  // ヘッダ開始なら次のゲーム。
            break;
          } else if (IsCommentStarting(front)) {  // コメント。
            ret_comment_vec.push_back(GetComment(token_queue));
          } else if ((front == "*") || (front == "1-0") || (front == "0-1")
          || (front == "1/2-1/2")) {  // 結果の文字列。
            ret_ptr->result(front);
            token_queue.pop();
          } else {
            token_queue.pop();
          }
        } else {  // 指し手のリスト後。
          ret_ptr->move_tree(ParseMoveNode(token_queue));
          has_moves_ended = true;
        }
      }

      ret_ptr->header(ret_header);
      ret_ptr->comment_vec(ret_comment_vec);
      return ret_ptr;
    }
  }

  // パースする。
  void PGN::Parse(const std::string& pgn_str) {
    // 字句解析。
    std::queue<std::string> token_queue = Tokenize(pgn_str);

    // パース。
    bool has_started = false;
    while (!(token_queue.empty())) {
      std::string front = token_queue.front();

      if (!has_started) {  // ゲームの記述前。
        if (front == "[") {  // ヘッダ開始。
          has_started = true;
        } else if (IsCommentStarting(front)) {  // コメントスタート。
          comment_vec_.push_back(GetComment(token_queue));
        }
      } else {  // ゲームの記述。
        game_vec_.push_back(ParseOneGame(token_queue));
      }
    }
  }
}  // namespace Sayuri
