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
 * @file pgn.cpp
 * @author Hironori Ishibashi
 * @brief PGNの実装。
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
    std::set<char> delim_token_c {'(', ')', '!', '?'};

    // 結果文字。
    std::set<std::string> result_str {"0-1", "1-0", "1/2-1/2", "*"};

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
            if (Util::IsAlgebraicNotation(temp)
            || (result_str.find(temp) != result_str.end())) {
              ret.push(temp);
            }
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_line_comment = true;
        } else if (c == '{') {  // コメント開始。
          if (oss.str().size()) {
            std::string temp = remove_blank(oss.str());
            if (Util::IsAlgebraicNotation(temp)
            || (result_str.find(temp) != result_str.end())) {
              ret.push(temp);
            }
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_comment = true;
        } else if (c == '['){  // タグ開始。
          if (oss.str().size()) {
            std::string temp = remove_blank(oss.str());
            if (Util::IsAlgebraicNotation(temp)
            || (result_str.find(temp) != result_str.end())) {
              ret.push(temp);
            }
            oss.str("");
          }
          ret.push(std::string(1, c));
          in_tag = true;
        } else if (delim_c.find(c) != delim_c.end()) {  // 区切り文字。
          if (oss.str().size()) {
            std::string temp = remove_blank(oss.str());
            if (Util::IsAlgebraicNotation(temp)
            || (result_str.find(temp) != result_str.end())) {
              ret.push(temp);
            }
            oss.str("");
          }
        } else if (delim_token_c.find(c) != delim_token_c.end()) {
          // 区切りでかつトークン。
          if (oss.str().size()) {
            std::string temp = remove_blank(oss.str());
            if (Util::IsAlgebraicNotation(temp)
            || (result_str.find(temp) != result_str.end())) {
              ret.push(temp);
            }
            oss.str("");
          }
          ret.push(std::string(1, c));
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
        if (Util::IsAlgebraicNotation(temp)) {
          ret.push(temp);
        }
      }
    }

    return ret;
  }
}  // namespace Sayuri
