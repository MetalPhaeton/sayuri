/* The MIT License (MIT)
 *
 * Copyright (c) 2013-2017 Hironori Ishibashi
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
 * @file lisp_core.cpp
 * @author Hironori Ishibashi
 * @brief Lispインタープリタの実装。 
 */

#include "lisp_core.h"

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <map>
#include <functional>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <set>
#include <cmath>
#include <random>
#include <chrono>
#include <regex>
#include <ctime>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <system_error>

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ========== //
  // static定数 //
  // ========== //
  const LPointer LObject::dummy_ptr_;
  const std::string LObject::dummy_str_;
  const LArgNames LObject::dummy_arg_names_;
  const LPointerVec LObject::dummy_ptr_vec_;
  const LScopeChain LObject::dummy_scope_chain_;
  const LC_Function LObject::dummy_c_function_;

  // ウォーカー。
  void Walk(LObject& pair, const LFuncForWalk& func) {
    // ペアじゃなければ終了。
    if (!(pair.IsPair())) return;

    // パス用文字バッファ。
    std::string path_buf;
    path_buf.reserve(256);

    // 再帰用関数を作成。
    std::function<void(LObject&)> core;
    core = [&func, &path_buf, &core](LObject& pair) {
      // ペアじゃなければ終了。
      if (!(pair.IsPair())) return;

      // 準備。 ポインタを記憶。
      LObject* car_before = pair.car().get();
      LObject* cdr_before = pair.cdr().get();

      // 関数を実行。
      func(pair, path_buf);

      // 終わり時のポインタを記録。
      LObject* car_after = pair.car().get();
      LObject* cdr_after = pair.cdr().get();

      // ポインタが同じならそれぞれに再帰コール。
      // Car。
      if (car_after == car_before) {
        path_buf.push_back('a');  // パスをプッシュ。

        core(*(pair.car()));

        path_buf.pop_back();  // パスをポップ。
      }

      // Cdr。
      if (cdr_after == cdr_before) {
        path_buf.push_back('d');  // パスをプッシュ。

        core(*(pair.cdr()));

        path_buf.pop_back();  // パスをポップ。
      }
    };

    // 実行。
    core(pair);
  }

  // マクロ展開する。
  void DevelopMacro(LObject* ptr, const LMacroMap& macro_map) {
    // コールバック関数を作成。
    LFuncForWalk func = [&macro_map](LObject& pair, const std::string& path) {
      // Carを置き換え。
      const LPointer& car = pair.car();
      if (car->IsSymbol()) {
        for (auto& map_pair : macro_map) {
          if (map_pair.first == car->symbol()) {
            pair.car(map_pair.second->Clone());
            break;
          }
        }
      }

      // Cdrを置き換え。
      const LPointer& cdr = pair.cdr();
      if (cdr->IsSymbol()) {
        for (auto& map_pair : macro_map) {
          if (map_pair.first == cdr->symbol()) {
            pair.cdr(map_pair.second->Clone());
            break;
          }
        }
      }
    };

    // Walkする。
    Walk(*ptr, func);
  }

  // 式を評価する。
  LPointer LObject::Evaluate(const LObject& target) {
    // 自分が関数でなければ、何もしない。
    if (!(IsFunction() || IsN_Function())) {
      throw Lisp::GenError("@evaluating-error",
      "Only function object can evaluate expressions.");
    }

    // シンボル、バインドされているオブジェクトを返す。
    if (target.IsSymbol()) {
      const LPointer& result = scope_chain().SelectSymbol(target.symbol());

      // あればクローン。 なければnullptr。
      if (result) return result->Clone();

      throw Lisp::GenError("@evaluating-error",
      "No object is bound to '" + target.symbol() + "'.");
    }

    // ペア、関数呼び出し。
    if (target.IsPair()) {
      // carを評価する。
      LPointer func_obj = Evaluate(*(target.car()));
      if (!func_obj) {
        throw Lisp::GenError("@evaluating-error",
        "Couldn't evaluate function name '" + target.car()->ToString()
        + "'.");
      }

      if ((func_obj->IsFunction()) || (func_obj->IsN_Function())) {
        return func_obj->Apply(this, target);
      }

      throw Lisp::GenError("@evaluating-error",
      "'" + target.car()->ToString() + "' didn't return function object.");
    }

    // Atomなのでクローンを返す。
    return target.Clone();
  }

  // 自身の関数を適用する。 LFunction。
  LPointer LFunction::Apply(LObject* caller, const LObject& args) {
    // ローカルスコープを作る。
    scope_chain_.AppendNewScope();

    // 引数リスト。
    const LPointer& arguments = args.cdr();

    // $@のリスト。
    LPointer at_list = Lisp::NewList(Lisp::CountList(*arguments));
    LObject* at_ptr = at_list.get();

    // 引数名。
    LArgNames::const_iterator names_itr = arg_names_.begin();
    LArgNames::const_iterator names_end = arg_names_.end();

    // マクロマップ。
    LMacroMap macro_map;

    // 各引数を整理。
    // 普通の引数なら評価してバインド。
    // マクロならmacro_mapに登録。
    LPointer result;
    for (LPointer ptr = arguments; ptr->IsPair();
    ptr = ptr->cdr(), Lisp::Next(&at_ptr)) {
      if (names_itr != names_end) {  // 引数名がある。
        if ((*names_itr)[0] == '^') {
          // マクロ名。
          // マクロマップに登録。
          macro_map.push_back(LMacroElm(*names_itr, ptr->car()));
        } else if ((*names_itr)[0] == '&') {
          // マクロ名。 (残りリスト)
          // 残りをマクロマップに入れて抜ける。
          macro_map.push_back(LMacroElm(*names_itr, ptr));
          break;
        } else {
          // 普通の引数名。
          // 評価してバインド。
          result = caller->Evaluate(*(ptr->car()));
          scope_chain_.InsertSymbol(*names_itr, result);

          // at_listにも登録。
          at_ptr->car(result);
        }

        ++names_itr;
      } else {  // 引数名がない。
        // at_listに登録するだけ。
        at_ptr->car(caller->Evaluate(*(ptr->car())));
      }
    }
    // at_listをスコープにバインド。
    scope_chain_.InsertSymbol("$@", at_list);

    // names_itrが余っていたらNilをバインド。
    for (; names_itr != names_end; ++names_itr) {
      scope_chain_.InsertSymbol(*names_itr, Lisp::NewNil());
    }

    // もしマクロ引数があったなら、マクロ展開する。
    // マクロがなかったらそのまま。
    LPointerVec expression;
    if (!(macro_map.empty())) {
      LPointer clone;
      for (auto& expr : expression_) {
        clone = expr->Clone();

        // マクロ展開。
        if (clone->IsPair()) {
          DevelopMacro(clone.get(), macro_map);
        } else if (clone->IsSymbol()) {
          for (auto& map_pair : macro_map) {
            if (map_pair.first == clone->symbol()) {
              clone = map_pair.second->Clone();
              break;
            }
          }
        }

        expression.push_back(clone);
      }
    } else {
      expression = expression_;
    }

    // 関数呼び出し。
    LPointer ret_ptr = Lisp::NewNil();
    for (auto& expr : expression) {
      ret_ptr = Evaluate(*expr);
    }

    if (!ret_ptr) {
      throw Lisp::GenError("@apply-error",
      "Failed to execute '" + args.car()->ToString() + "'.");
    }
    return ret_ptr;
  }

  // ======= //
  // LParser //
  // ======= //
  // --- 字句解析器 --- //
  namespace {
    // 制御文字かどうか。
    inline bool IsControlChar(char c) {
      static const std::set<char> control_c {
        '\n', '\r', '\f', '\t', '\v', '\b', '\a', '\0'
      };
      return control_c.find(c) != control_c.end();
    }

    // 開きカッコかどうか。
    inline bool IsOpenParenth(char c) {
      static const std::set<char> open_c {'(', '{', '['};
      return open_c.find(c) != open_c.end();
    }
    // 閉じカッコかどうか。
    inline bool IsCloseParenth(char c) {
      static const std::set<char> close_c {')', '}', ']'};
      return close_c.find(c) != close_c.end();
    }

    // ベクトルに文字列とcをプッシュする。 cが0ならcはプッシュしない。
    inline void PushString(std::queue<std::string>& queue,
    std::ostringstream& oss, char c=0) {
      std::string temp = oss.str();
      if (temp.size()) {
        queue.push(temp);
        oss.str("");
      }

      if (c) queue.push(std::string(1, c));
    }

    // コメントをスキップする。
    inline void SkipComment(std::string::const_iterator& itr,
    const std::string::const_iterator& end_itr) {
      for (; itr != end_itr; ++itr) {
        if (*itr == '\n') return;  // 改行文字は飛ばさない。
      }
    }

    // 文字列トークンを得る。
    std::vector<std::string> ParseStringToken(std::string::const_iterator& itr,
    const std::string::const_iterator& end_itr, bool& in_string) {
      // クオートはない状態で始まる。

      // 準備。
      std::vector<std::string> ret;
      std::ostringstream oss;

      // 文字列をプッシュする関数。
      auto push = [&ret, &oss]() {
        std::string temp = oss.str();
        if (temp.size()) {
          ret.push_back(temp);
          oss.str("");
        }
      };

      // 文字列終了までループ。
      for (; itr != end_itr; ++itr) {
        // 制御文字は無視。
        if (IsControlChar(*itr)) continue;

        // 文字列終了。 クオートは飛ばさない。
        if (*itr == '\"') {
          in_string = false;
          push();
          return ret;
        }

        // エスケープ文字。
        if (*itr == '\\') {
          // 先ず今までのをプッシュ。
          push();

          // バックスラッシュを入れる。
          oss << *itr;
          ++itr;
          if (itr == end_itr) break;

          // バックスラッシュの次の文字を入れる。
          oss << *itr;

          // プッシュ。
          push();
          continue;
        }

        oss << *itr;
      }

      // まだ文字列解析中。
      in_string = true;
      push();
      return ret;
    }

    // コンマをパースする。
    inline std::string ParseComma(std::string::const_iterator& itr,
    const std::string::const_iterator& end_itr) {
      std::ostringstream oss;
      oss << *itr;
      if ((itr + 1) == end_itr) return oss.str();
      if (*(itr + 1) == '@') {
        ++itr;
        oss << *itr;
        return oss.str();
      }
      return oss.str();
    }
  }

  // 字句解析する。
  void LParser::Tokenize(const std::string& code) {
    // イテレータを準備。
    std::string::const_iterator itr = code.begin();
    std::string::const_iterator end_itr = code.end();

    // ストリームの準備。
    std::ostringstream oss;

    // 各文字でループ。
    for (; itr != end_itr; ++itr) {
      // まだ文字列解析中なら文字の解析を続行。
      if (in_string_) {
        for (auto& str : ParseStringToken(itr, end_itr, in_string_)) {
          token_queue_.push(str);
        }
        if (!in_string_) {  // 解析終了なら今はクオートがitr。
          PushString(token_queue_, oss, *itr);
        }

        if (itr == end_itr) break;
        continue;
      }

      // コメント開始。
      if (*itr == ';') {
        // 今までのをプッシュしてコメントを飛ばす。
        PushString(token_queue_, oss);
        SkipComment(itr, end_itr);

        if (itr == end_itr) break;
        continue;
      }

      // 文字列開始。
      if (*itr == '"') {
        // 今までのをプッシュ。
        PushString(token_queue_, oss, *itr);

        // クオートを飛ばす。
        ++itr;

        // 文字列を解析。
        for (auto& str : ParseStringToken(itr, end_itr, in_string_)) {
          token_queue_.push(str);
        }
        if (!in_string_) {  // 解析終了なら今はクオートがitr。
          PushString(token_queue_, oss, *itr);
        }

        if (itr == end_itr) break;
        continue;
      }

      // 空白、制御文字。
      if ((*itr == ' ') || (IsControlChar(*itr))) {
        // 今までのをプッシュするのみ。
        PushString(token_queue_, oss);
        continue;
      }

      // 開きカッコ。
      if (IsOpenParenth(*itr)) {
        // 今までと自身をプッシュする。
        PushString(token_queue_, oss, '(');
        // カッコのカウントを増やす。
        ++parenth_counter_;

        continue;
      }

      // 閉じカッコ。
      if (IsCloseParenth(*itr)) {
        // 今までと自身をプッシュする。
        PushString(token_queue_, oss, ')');
        // カッコのカウントを減らす。
        --parenth_counter_;

        continue;
      }

      // クオートの糖衣構文。
      if (*itr == '\'') {
        // 今までと自身をプッシュする。
        PushString(token_queue_, oss, *itr);
        continue;
      }

      // テンプレートの糖衣構文。
      if (*itr == '`') {
        // 今までと自身をプッシュする。
        PushString(token_queue_, oss, *itr);
        continue;
      }

      // コンマ。
      if (*itr == ',') {
        PushString(token_queue_, oss);
        token_queue_.push(ParseComma(itr, end_itr));
        continue;
      }

      // 普通の文字。
      oss << *itr;
    }

    // 最後にossに残った文字をプッシュして終わる。
    PushString(token_queue_, oss);
  }

  LPointerVec LParser::Parse() {
    LPointerVec ret;

    // きちんとパースできていなければパースしない。
    if ((parenth_counter_ != 0) || (in_string_)) return ret;

    // パースループ。
    while (!(token_queue_.empty())) {
      ret.push_back(ParseCore());
    }

    // 念の為、キューを空にする。
    token_queue_ = std::queue<std::string>();
    return ret;
  }

  LPointer LParser::ParseCore() {
    if (token_queue_.empty()) {
      throw Lisp::GenError("@parse-error", "Token queue is empty.");
    }

    // 最初のトークンを得る。
    std::string front = token_queue_.front();
    token_queue_.pop();

    if (front == "(") {  // リストをパース。
      LPointer ret = Lisp::NewNil();
      LObject* ret_ptr = nullptr;
      while (!(token_queue_.empty())) {
        front = token_queue_.front();

        // リスト終了。
        if (front == ")") {
          token_queue_.pop();
          return ret;
        }

        // ドット対。
        if (front == ".") {
          token_queue_.pop();

          // パース。
          LPointer result = ParseCore();

          if (ret_ptr) {  // retがペア。
            ret_ptr->cdr(result);
          } else {  // retがNil。
            ret = Lisp::NewPair(Lisp::NewNil(), result);
          }

          continue;
        }

        // Carをパース。
        LPointer result = ParseCore();
        Lisp::AppendElementAndGetBack(ret, result, &ret_ptr);
      }
    } else {  // Atomをパース。
      // エラー。
      if ((front == ".") || (front == ")")) {
        throw Lisp::GenError("@parse-error", "Couldn't parse '"
        + front + "'.");
      }

      // quoteの糖衣構文。
      if (front == "'") {
        // quoteシンボルの入ったペア。
        LPointer ret =
        Lisp::NewPair(Lisp::NewSymbol("quote"), Lisp::NewPair());

        // 次をパースする。
        LPointer result = ParseCore();

        // quoteの次にセットして返る。
        ret->cdr()->car(result);
        return ret;
      }

      // backquoteの糖衣構文。
      if (front == "`") {
        // quoteシンボルの入ったペア。
        LPointer ret =
        Lisp::NewPair(Lisp::NewSymbol("backquote"), Lisp::NewPair());

        // 次をパースする。
        LPointer result = ParseCore();

        // quoteの次にセットして返る。
        ret->cdr()->car(result);
        return ret;
      }

      // unquoteの糖衣構文。
      if (front == ",") {
        // quoteシンボルの入ったペア。
        LPointer ret =
        Lisp::NewPair(Lisp::NewSymbol("unquote"), Lisp::NewPair());

        // 次をパースする。
        LPointer result = ParseCore();

        // quoteの次にセットして返る。
        ret->cdr()->car(result);
        return ret;
      }

      // unquote-splicingの糖衣構文。
      if (front == ",@") {
        // quoteシンボルの入ったペア。
        LPointer ret =
        Lisp::NewPair(Lisp::NewSymbol("unquote-splicing"), Lisp::NewPair());

        // 次をパースする。
        LPointer result = ParseCore();

        // quoteの次にセットして返る。
        ret->cdr()->car(result);
        return ret;
      }

      // 文字列。
      if (front == "\"") {
        std::ostringstream oss;
        while (!(token_queue_.empty())) {
          front = token_queue_.front();
          token_queue_.pop();

          if (front == "\"") break;  // 文字列終了なら抜ける。

          if ((front.size() >= 2) && (front[0] == '\\')) {
            // エスケープシーケンスだった場合。
            switch (front[1]) {
              case 'n': oss << '\n'; break;  // 改行。
              case 'r': oss << '\r'; break;  // キャリッジリターン。
              case 'f': oss << '\f'; break;  // 改ページ。
              case 't': oss << '\t'; break;  // タブ。
              case 'v': oss << '\v'; break;  // 垂直タブ。
              case 'a': oss << '\a'; break;  // ベル。
              case 'b': oss << '\b'; break;  // バックスペース。
              case '0': oss << '\0'; break;  // Null文字。
              case 'e': oss << '\x1b'; break;  // ESC。
              default: oss << front[1]; break;  // その他。
            }
          } else {
            oss << front;
          }
        }

        return Lisp::NewString(oss.str());
      }

      // 真偽値。 #t。
      if ((front == "#t") || (front == "#T")) {
        return Lisp::NewBoolean(true);
      }

      // 真偽値。 #f。
      if ((front == "#f") || (front == "#F")) {
        return Lisp::NewBoolean(false);
      }

      // 数字かシンボルの判定。
      char c = front[0];
      if ((front.size() >= 2) && ((c == '+') || (c == '-'))) c = front[1];

      // 数字。
      if ((c >= '0') && (c <= '9')) {
        try {
          return Lisp::NewNumber(std::stod(front));
        } catch (...) {
          // エラーならシンボル。
          return Lisp::NewSymbol(front);
        }
      }

      // シンボル。
      return Lisp::NewSymbol(front);
    }

    throw Lisp::GenError("@parse-error", "Couldn't parse '" + front + "'.");
  }


  // ==== //
  // Lisp //
  // ==== //
  // コンストラクタ。
  Lisp::Lisp(const std::vector<std::string>& argv) : LN_Function() {
    c_function_ = *this;
    func_id_ = "Lisp";
    scope_chain_ = LScopeChain();

    SetCoreFunctions();
    SetBasicFunctions();

    // コマンド引数を設定。
    LPointer list = NewList(argv.size());
    LObject* ptr = list.get();
    for (auto& arg : argv) {
      ptr->car(NewString(arg));
      ptr = ptr->cdr().get();
    }
    scope_chain_.InsertSymbol("argv", list);
  }
  // コンストラクタ。
  Lisp::Lisp() : LN_Function() {
    c_function_ = *this;
    func_id_ = "Lisp";
    scope_chain_ = LScopeChain();

    SetCoreFunctions();
    SetBasicFunctions();
    scope_chain_.InsertSymbol("argv", NewNil());
  }
  // コピーコンストラクタ。
  Lisp::Lisp(const Lisp& lisp) : LN_Function(lisp),
  parser_(lisp.parser_) {}
  // ムーブコンストラクタ。
  Lisp::Lisp(Lisp&& lisp) : LN_Function(lisp),
  parser_(std::move(lisp.parser_)) {}
  // コピー代入演算子。
  Lisp& Lisp::operator=(const Lisp& lisp) {
    LN_Function::operator=(lisp);
    parser_ = lisp.parser_;
    return *this;
  }
  // ムーブ代入演算子。
  Lisp& Lisp::operator=(Lisp&& lisp) {
    LN_Function::operator=(lisp);
    parser_ = std::move(lisp.parser_);
    return *this;
  }

  // コア関数を登録する。
  void Lisp::SetCoreFunctions() {
    LC_Function func;
    std::string help;

    func = LC_FUNCTION_OBJ(Help);
    INSERT_LC_FUNCTION(func, "help", "Lisp:help");
    help =
R"...(### help ###

<h6> Usage </h6>

1. `(help)`
2. `(help <String>)`

<h6> Description </h6>

* 1: Returns descriptions of all help.
* 2: Returns a description of `<String>`.

<h6> Example </h6>

    (display (help "car"))
    
    ;; Output
    ;; > ### car ###
    ;; >
    ;; > <h6> Usage </h6>
    ;; >
    ;; >
    ;; > * `(car <List>)`
    ;; >
    ;; > <h6> Description </h6>
    ;; >
    ;; > * Returns the 1st element of `<List>`.
    ;; >
    ;; > <h6> Example </h6>
    ;; >
    ;; >     (display (car (list 111 222 333)))
    ;; >     
    ;; >     ;; Output
    ;; >     ;;
    ;; >     ;; > 111)...";
    help_dict_.emplace("help", help);

    func = LC_FUNCTION_OBJ(Eval);
    INSERT_LC_FUNCTION(func, "eval", "Lisp:eval");
    help =
R"...(### eval ###

<h6> Usage </h6>

* `(eval <Object>)`

<h6> Description </h6>

* Evaluates `<Object>`.

<h6> Example </h6>

    (define x '(+ 1 2 3))
    (display x)
    (display (eval x))
    
    ;; Output
    ;; > (+ 1 2 3)
    ;; > 6))...";
    help_dict_.emplace("eval", help);

    func = LC_FUNCTION_OBJ(ParseFunc);
    INSERT_LC_FUNCTION(func, "parse", "Lisp:parse");
    INSERT_LC_FUNCTION(func, "string->symbol", "Lisp:string->symbol");
    INSERT_LC_FUNCTION(func, "string->number", "Lisp:string->number");
    INSERT_LC_FUNCTION(func, "string->boolean", "Lisp:string->boolean");
    INSERT_LC_FUNCTION(func, "string->list", "Lisp:string->list");
    help =
R"...(### parse ###

<h6> Usage </h6>

* `(parse <S-Expression : String>)`
* `(string->symbol <S-Expression : String>)`
* `(string->number <S-Expression : String>)`
* `(string->boolean <S-Expression : String>)`
* `(string->list <S-Expression : String>)`

<h6> Description </h6>

* Parses `<S-Expression>` and generates a object.

<h6> Example </h6>

    (display (parse "(1 2 3)"))
    
    ;; Output
    ;; > (1 2 3))...";
    help_dict_.emplace("parse", help);
    help_dict_.emplace("string->symbol", help);
    help_dict_.emplace("string->number", help);
    help_dict_.emplace("string->boolean", help);
    help_dict_.emplace("string->list", help);

    func = LC_FUNCTION_OBJ(Parval);
    INSERT_LC_FUNCTION(func, "parval", "Lisp:parval");
    help =
R"...(### parval ###

<h6> Usage </h6>

* `(parse <S-Expression : String>)`

<h6> Description </h6>

* Parses and evaluates `<S-Expression>` and returns result.
    + It is similar to `(eval (parse <S-Expression>))`.

<h6> Example </h6>

    (parval "(display \"Hello\")(display \"World\")")
    
    ;; Output
    ;; > Hello
    ;; > World)...";
    help_dict_.emplace("parval", help);

    func = LC_FUNCTION_OBJ(ToStringFunc);
    INSERT_LC_FUNCTION(func, "to-string", "Lisp:to-string");
    INSERT_LC_FUNCTION(func, "symbol->string", "Lisp:symbol->string");
    INSERT_LC_FUNCTION(func, "number->string", "Lisp:number->string");
    INSERT_LC_FUNCTION(func, "boolean->string", "Lisp:boolean->string");
    INSERT_LC_FUNCTION(func, "list->string", "Lisp:list->string");
    help =
R"...(### to-string ###

<h6> Usage </h6>

* `(to-string <Object>)`
* `(symbol->string <Object>)`
* `(number->string <Object>)`
* `(boolean->string <Object>)`
* `(list->string <Object>)`

<h6> Description </h6>

* Converts `<Object>` to S-Expression as String.

<h6> Example </h6>

    (display (to-string '(1 2 3)))
    
    ;; Output
    ;; > (1 2 3)
    ;;
    
    (display (string? (to-string '(1 2 3))))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("to-string", help);
    help_dict_.emplace("symbol->string", help);
    help_dict_.emplace("number->string", help);
    help_dict_.emplace("boolean->string", help);
    help_dict_.emplace("list->string", help);

    func = LC_FUNCTION_OBJ(Try);
    INSERT_LC_FUNCTION(func, "try", "Lisp:try");
    help =
R"...(### try ###

<h6> Usage </h6>

* `(try (<Try Expr>...) <Catch Expr>...)`

<h6> Description </h6>

* This is Special Form.
    * `<Catch Expr>...` is evaluated if an error have been occurred
      in `<Try Expr>...`.
* Handles exceptions.
* If an exception occurs in `<Try Expr>...`, then
  it stops `<Try Expr>...` and executes `<Catch Expr>...`.
* In a scope of `<Catch Expr>...`, 'exception' symbol is defined.
* Returns a evaluated last object.

<h6> Example </h6>

    (try ((+ 1 "Hello"))
         (display "Error Occured!!"))
    
    ;; Output
    ;; > Error Occured!!
    
    (try ((+ 1 "Hello"))
         (display exception))
    
    ;; Output
    ;; > (@not-number "The 2nd argument of (+) didn't return Number."))...";
    help_dict_.emplace("try", help);

    func = LC_FUNCTION_OBJ(Throw);
    INSERT_LC_FUNCTION(func, "throw", "Lisp:throw");
    help =
R"...(### throw ###

<h6> Usage </h6>

* `(throw <Object>)`

<h6> Description </h6>

* Throws an exception.
* If you use this in (try) function,
  `<Object>` is bound to 'exception' symbol.

<h6> Example </h6>

    (try ((throw 123))
         (display exception))
    
    ;; Output
    ;; > 123)...";
    help_dict_.emplace("throw", help);

    func = LC_FUNCTION_OBJ(CarFunc);
    INSERT_LC_FUNCTION(func, "car", "Lisp:car");
    help =
R"...(### car ###

<h6> Usage </h6>

* `(car <Pair or List>)`

<h6> Description </h6>

* Returns Car value of `<Pair or List>`

<h6> Example </h6>

    (display (car '(111 . 222)))
    ;; Output
    ;; > 111
    
    (display (car (list 111 222 333)))
    
    ;; Output
    ;; > 111)...";
    help_dict_.emplace("car", help);

    func = LC_FUNCTION_OBJ(CdrFunc);
    INSERT_LC_FUNCTION(func, "cdr", "Lisp:cdr");
    help =
R"...(### cdr ###

<h6> Usage </h6>

* `(cdr <Pair or List>)`

<h6> Description </h6>

* Returns Cdr value of `<Pair or List>`

<h6> Example </h6>

    (display (cdr '(111 . 222)))
    ;; Output
    ;; > 222
    
    (display (cdr (list 111 222 333)))
    
    ;; Output
    ;; > (222 333))...";
    help_dict_.emplace("cdr", help);

    func = LC_FUNCTION_OBJ(Cons);
    INSERT_LC_FUNCTION(func, "cons", "Lisp:cons");
    help =
R"...(### cons ###

<h6> Usage </h6>

* `(cons <Object 1> <Object 2>)`

<h6> Description </h6>

* Returns Pair. Car is `<Object 1>`, Cdr is `<Object 2>`.

<h6> Example </h6>


    (display (cons 111 222))
    
    ;; Output
    ;; > (111 . 222)
    
    (display (cons 111 '(222 333)))
    
    ;; Output
    ;; > (111 222 333)
    
    (display (cons 444 (cons 555 (cons 666 ()))))
    
    ;; Output
    ;; > (444 555 666))...";
    help_dict_.emplace("cons", help);

    // cxrを登録。
    help =
R"...(### cxxxxr ###

<h6> Usage </h6>

* `(c[ad]{2,4}r <Pair or List>)`

<h6> Description </h6>

* Returns Object of `c[ad]{2,4}r` of `<Pair or List>`

<h6> Example </h6>

    (define tree '(1 (21 22) (31 32) 4))
    
    (display (cadr tree))
    ;; Output
    ;; > (21 22)
    
    (display (caaddr tree))
    ;; Output
    ;; > 31)...";
    {
      const char* ad = "ad";
      std::string path;
      std::ostringstream oss;
      for (int _1st = 0; _1st < 2; ++_1st) {
        for (int _2nd = 0; _2nd < 2; ++_2nd) {
          oss <<  ad[_2nd] << ad[_1st];
          path = oss.str();
          oss.str("");
          func = [this, path](LPointer self, LObject* caller,
          const LObject& args) -> LPointer {
            return this->CxrFunc(path, self, caller, args);
          };
          INSERT_LC_FUNCTION(func, "c" + path + "r", "Lisp:c" + path + "r");
          help_dict_.emplace("c" + path + "r", help);
          for (int _3rd = 0; _3rd < 2; ++_3rd) {
            oss << ad[_3rd] <<  ad[_2nd] << ad[_1st];
            path = oss.str();
            oss.str("");
            func = [this, path](LPointer self, LObject* caller,
            const LObject& args) -> LPointer {
              return this->CxrFunc(path, self, caller, args);
            };
            INSERT_LC_FUNCTION(func, "c" + path + "r", "Lisp:c" + path + "r");
            help_dict_.emplace("c" + path + "r", help);
            for (int _4th = 0; _4th < 2; ++_4th) {
              oss << ad[_4th] << ad[_3rd] <<  ad[_2nd] << ad[_1st];
              path = oss.str();
              oss.str("");
              func = [this, path](LPointer self, LObject* caller,
              const LObject& args) -> LPointer {
                return this->CxrFunc(path, self, caller, args);
              };
              INSERT_LC_FUNCTION(func, "c" + path + "r",
              "Lisp:c" + path + "r");
              help_dict_.emplace("c" + path + "r", help);
            }
          }
        }
      }
    }

    func = LC_FUNCTION_OBJ(ApplyFunc);
    INSERT_LC_FUNCTION(func, "apply", "Lisp:apply");
    help =
R"...(### apply ###

<h6> Usage </h6>

* `(apply <Object 1> <Object 2>)`

<h6> Description </h6>

* Constructs Pair and evaluates it.
  + `<Object 1>` is Car, `<Object 2>` is Cdr.
  + It is same as `(eval (cons <Object 1> <Object 2>))`.

<h6> Example </h6>

    (define a '(1 2 3))
    
    (display (apply '+ a))
    
    ;; Output
    ;; > 6)...";
    help_dict_.emplace("apply", help);

    func = LC_FUNCTION_OBJ(WalkFunc);
    INSERT_LC_FUNCTION(func, "walk", "Lisp:walk");
    help =
R"...(### walk ###

<h6> Usage </h6>

* `(walk <Callback : Function> <Target : Pair>)`

<h6> Description </h6>

* Walks around in `<Target>` and executes `<Callback>`.
    + `<Callback>` receives 2 arguments.
        - 1st : Current element.
        - 2nd : Path. (String)
            - 'a' is Car.
            - 'd' is cdr.
            - For example, "da" means (car (cdr <List>)).
    + If `<Callback>` returns List and its 1st element is `@replace`,
      the current element is replaced for the 2nd element of the list.
* Returns result of `<Target>`.

<h6> Example </h6>

    (define li '(1 2 (3 4) 5))
    (define (my-func elm path)
            (display "elm : " elm " || path : " path)
            (if (equal? elm 3)
                '(@replace "Hello")
                ()))
    
    (display (walk my-func li))
    ;; Output
    ;; > elm : 1 || path : a
    ;; > elm : (2 (3 4) 5) || path : d
    ;; > elm : 2 || path : da
    ;; > elm : ((3 4) 5) || path : dd
    ;; > elm : (3 4) || path : dda
    ;; > elm : (5) || path : ddd
    ;; > elm : 3 || path : ddaa
    ;; > elm : (4) || path : ddad
    ;; > elm : 4 || path : ddada
    ;; > elm : () || path : ddadd
    ;; > elm : 5 || path : ddda
    ;; > elm : () || path : dddd
    ;; > (1 2 ("Hello" 4) 5))...";
    help_dict_.emplace("walk", help);

    func = LC_FUNCTION_OBJ(Quote);
    INSERT_LC_FUNCTION(func, "quote", "Lisp:quote");
    help =
R"...(### quote ###

<h6> Usage </h6>

* `(quote <Object>)`

<h6> Description </h6>

* This is Special Form.
    + `<Object>` is not Evaluated.
* Returns `<Object>` as is.
* Syntactic suger is `'<Object>`

<h6> Example </h6>

    (display (quote (111 222 333)))
    
    ;; Output
    ;; > (111 222 333)
    
    (display '(444 555 666))
    
    ;; Output
    ;; > (444 555 666)
    
    (define x 123)
    (display x)
    (display 'x)
    
    ;; Output
    ;; > 123
    ;; > Symbol: x)...";
    help_dict_.emplace("quote", help);

    func = LC_FUNCTION_OBJ(Backquote);
    INSERT_LC_FUNCTION(func, "backquote", "Lisp:backquote");
    help =
R"...(### backquote ###

<h6> Usage </h6>

* `(backquote <Object>)`

<h6> Description </h6>

* This is Special Form.
    + `<Object>` is treated as template.
        - An object next to `,` or `,@` is evaluated. the others is not.
* Returns an object after template process.
* Syntactic suger is backquote.

<h6> Example </h6>

    (define a 111)
    (define b 222)
    (define c 333)
    (define d '(444 555 666))
    (define e '())
    
    (display (backquote (a b c)))
    (display `(a b c))
    ;; Output
    ;; > (a b c)
    ;; > (a b c)
    
    (display (backquote (a ,b c)))
    (display `(a ,b c))
    ;; Output
    ;; > (a 222 c)
    ;; > (a 222 c)
    
    (display (backquote (a ,d c)))
    (display `(a ,d c))
    ;; Output
    ;; > (a (444 555 666) c)
    ;; > (a (444 555 666) c)
    
    (display (backquote (a ,@d c)))
    (display `(a ,@d c))
    ;; Output
    ;; > (a 444 555 666 c)
    ;; > (a 444 555 666 c)
    
    (display (backquote (a (a ,@d c) c)))
    (display `(a (a ,@d c) c))
    ;; Output
    ;; > (a (a 444 555 666 c) c)
    ;; > (a (a 444 555 666 c) c)
    
    (display (backquote (a ,@e c)))
    (display `(a ,@e c))
    ;; Output
    ;; > (a c)
    ;; > (a c))...";
    help_dict_.emplace("backquote", help);

    func = LC_FUNCTION_OBJ(Lambda);
    INSERT_LC_FUNCTION(func, "lambda", "Lisp:lambda");
    help =
R"...(### lambda ###

<h6> Usage </h6>

* `(lambda (<Args : Symbol>...) <S-Expression>...)`

<h6> Description </h6>

* This is Special Form.
    + All arguments isn't evaluated.
* Returns Function defined by `<S-Expression>...`.
* (lambda) inherits parent's scope and creates its own local scope.
  So using (lambda) in (lambda), you can create closure function.
* `<Args>...` is Symbols as name of arguments.
    + If an argument name is started with `^` or `&`,
      the argument is Macro-Like Argument.

<h6> Example </h6>

    (define myfunc (lambda (x) (+ x 100)))
    (display (myfunc 5))
    ;; Output
    ;; > 105
    
    (define gen-func (lambda (x) (lambda () (+ x 100))))
    (define myfunc2 (gen-func 50))
    (display (myfunc2))
    ;; Output
    ;; > 150
    
    ;; Example of Macro-Like Argument.
    (define gen-func2
      (lambda (^x &y) (lambda () (display (apply '^x '&y)))))
    (define func1 (gen-func2 + 111 222))
    (define func2 (gen-func2 * 333 444 555))
    (func1)
    (func2)
    ;; Output
    ;; > 333
    ;; > 82057860
    (display (to-string func1))
    (display (to-string func2))
    ;; Output
    ;; > (lambda () (display (apply (quote +) (quote (111 222)))))
    ;; > (lambda () (display (apply (quote *) (quote (333 444 555))))))...";
    help_dict_.emplace("lambda", help);

    func = LC_FUNCTION_OBJ(FuncToLambda);
    INSERT_LC_FUNCTION(func, "func->lambda", "Lisp:func->lambda");
    help =
R"...(### func->lambda ###

<h6> Usage </h6>

* `(func->lambda <Function : Function>)`

<h6> Description </h6>

* Converts Function to S-Expression and returns.

<h6> Example </h6>

    (define (myfunc x) (display x))
    (display (func->lambda myfunc))
    ;; Output
    ;; > (lambda (x) (display x)))...";
    help_dict_.emplace("func->lambda", help);

    func = LC_FUNCTION_OBJ(Let);
    INSERT_LC_FUNCTION(func, "let", "Lisp:let");
    help =
R"...(### let ###

<h6> Usage </h6>

* `(let ((<Name : Symbol> <Object>)...) <S-Expression>...)`

<h6> Description </h6>

* This is Special Form.
    + `<Name : Symbol>` isn't evaluated.
    + But `<Object>` and `<S-Expression>` are evaluated.
* Executes `<S-Expression>...` in new local scope.
* (let) inherits parent's scope and creates its own local scope.
  So using (lambda) in (let), you can create closure function.
* `(<Name> <Object>)...` is local values on (let)'s local scope.

<h6> Example </h6>

    (define (gen-func x y) (let ((a x) (b y))
              (lambda () (+ a b))))
    (define myfunc (gen-func 10 20))
    (display (myfunc))
    
    ;; Output
    ;; > 30)...";
    help_dict_.emplace("let", help);

    func = LC_FUNCTION_OBJ(While);
    INSERT_LC_FUNCTION(func, "while", "Lisp:while");
    help =
R"...(### while ###

<h6> Usage </h6>

* `(while <Condition : Boolean> <S-Expression>...)`

<h6> Description </h6>

* This is Special Form.
* While `<Condition>` is #t, it iterates `<S-Expression>...`.
* Returns Object returned by the last S-Expression.

<h6> Example </h6>

    (define i 0)
    (while (< i 5)
        (display "Hello " i)
        (display "World" i)
        (set! i (++ i)))
    
    ;; Output
    ;; > Hello 0
    ;; > World 0
    ;; > Hello 1
    ;; > World 1
    ;; > Hello 2
    ;; > World 2
    ;; > Hello 3
    ;; > World 3
    ;; > Hello 4
    ;; > World 4)...";
    help_dict_.emplace("while", help);

    func = LC_FUNCTION_OBJ(For);
    INSERT_LC_FUNCTION(func, "for", "Lisp:for");
    help =
R"...(### for ###

<h6> Usage </h6>

* `(for (<Variable : Symbol> <List | String>) <S-Expression>...)`

<h6> Description </h6>

* This is Special Form.
    + `<Variable>` is not evaluated.
* Repeats `<S-Expression>...` until a number of elements of `<List | String>`.
    + The element of `<List | String>` is bound to `<Variable>`.
* Returns Object returned by the last S-Expression.

<h6> Example </h6>

    (define aaa '(1 2 3 4 5))
    
    (for (x aaa)
        (display "Hello " x)
        (display "World " (+ x 5)))
    ;; Output
    ;; > Hello 1
    ;; > World 6
    ;; > Hello 2
    ;; > World 7
    ;; > Hello 3
    ;; > World 8
    ;; > Hello 4
    ;; > World 9
    ;; > Hello 5
    ;; > World 10

    (for (x "Hello")
        (display x))
    ;; Output
    ;; > H
    ;; > e
    ;; > l
    ;; > l
    ;; > o)...";
    help_dict_.emplace("for", help);

    func = LC_FUNCTION_OBJ(Define);
    INSERT_LC_FUNCTION(func, "define", "Lisp:define");
    help =
R"...(### define ###

<h6> Usage </h6>

1. `(define <Symbol> <Object>)`
2. `(define (<Name : Symbol> <Args : Symbol>...) <S-Expression>...)`

<h6> Description </h6>

* This is Special Form.
    + 1: `<Symbol>` isn't evaluated.
    + 2: All arguments isn't evaluated.
* Binds something to its scope.
* 1: Binds `<Object>` to `<Symbol>`.
* 2: Defines `<S-Expression>` as Function named `<Name>`,
     and `<Args>...` is names of its arguments.
    + If an argument name is started with `^` or `&`,
      the argument is Macro-Like Argument.

<h6> Example </h6>

    (define x 123)
    (display x)
    
    ;; Output
    ;; > 123
    
    (define (myfunc x) (+ x 10))
    (display (myfunc 5))
    
    ;; Output
    ;; > 15
    
    ;; Example of Macro-Like Argument.
    (define (gen-func ^x &y) (^x &y) (lambda () (display (apply '^x '&y))))
    (define func1 (gen-func + 111 222))
    (define func2 (gen-func * 333 444 555))
    (func1)
    (func2)
    ;; Output
    ;; > 333
    ;; > 82057860
    (display (to-string func1))
    (display (to-string func2))
    ;; Output
    ;; > (lambda () (display (apply (quote +) (quote (111 222)))))
    ;; > (lambda () (display (apply (quote *) (quote (333 444 555))))))...";
    help_dict_.emplace("define", help);

    func = LC_FUNCTION_OBJ(DefineMacro);
    INSERT_LC_FUNCTION(func, "define-macro", "Lisp:define-macro");
    help =
R"...(### define-macro ###

<h6> Usage </h6>

* `(define-macro (<Name : Symbol> <Args : Symbol>...) <S-Expression>...)`

<h6> Description </h6>

* This is Special Form.
    + All arguments isn't evaluated.
* Defines Traditional Macros and returns `<Name>`.
* If an argument name starts with '&', it means the rest of arguments.

<h6> Example </h6>

    (define-macro (my-macro x y &z)
                  (display x)
                  (display y)
                  (display &z)
                  (cons x (cons y &z)))
    
    (display (my-macro * (+ 1 2) 3 4 5))
    ;; Output
    ;; > Symbol: *
    ;; > (+ 1 2)
    ;; > (3 4 5)
    ;; > 180)...";
    help_dict_.emplace("define-macro", help);

    func = LC_FUNCTION_OBJ(Set);
    INSERT_LC_FUNCTION(func, "set!", "Lisp:set!");
    help =
R"...(### set! ###

<h6> Usage </h6>

* `(set! <Symbol> <Object>)`

<h6> Description </h6>

* This is Special Form.
    + `<Symbol>` isn't evaluated.
* Updates `<Symbol>` to `<Object>` on the local scope.

<h6> Example </h6>

    (define x 123)
    (set! x 456)
    (display x)
    
    ;; Output
    ;; > 456
    
    (define myfunc (let ((x 1)) (lambda () (set! x (+ x 1)) x)))
    (display (myfunc))
    (display (myfunc))
    (display (myfunc))
    
    ;; Output
    ;; > 2
    ;; > 3
    ;; > 4)...";
    help_dict_.emplace("set!", help);

    func = LC_FUNCTION_OBJ(If);
    INSERT_LC_FUNCTION(func, "if", "Lisp:if");
    help =
R"...(### if ###

<h6> Usage </h6>

* `(if <Condition : Boolean> <Then> <Else>)`

<h6> Description </h6>

* This is Special Form.
    + Either of `<Then>` and `<Else>` are evaluated.
* If `<Condition>` is true, then (if) evaluates `<Then>`.
  If false, then it evaluates `<Else>`.

<h6> Example </h6>

    (display (if (< 1 2) (+ 3 4) (+ 5 6)))
    
    ;; Output
    ;; > 7)...";
    help_dict_.emplace("if", help);

    func = LC_FUNCTION_OBJ(Cond);
    INSERT_LC_FUNCTION(func, "cond", "Lisp:cond");
    help =
R"...(### cond ###

<h6> Usage </h6>

* `(cond (<Condition : Boolean> <Then>)... (else <Else>))`

<h6> Description </h6>

* This is Special Form.
    + Only one of `<Then>` or `<Else>` are evaluated.
    + `(else <Else>)` is a special list.
* If `<Condition>` is true, then (cond) returns `<Then>`.
  If false, then it evaluates next `<Condition>`.
* If all `<Condition>` are false, then (cond) returns `<Else>`.

<h6> Example </h6>

    (cond
        ((> 1 2) (display "Hello"))
        ((< 3 4) (display "World"))
        (else "Else!!"))
    
    ;; Output
    ;; > World)...";
    help_dict_.emplace("cond", help);

    func = LC_FUNCTION_OBJ(Begin);
    INSERT_LC_FUNCTION(func, "begin", "Lisp:begin");
    help =
R"...(### begin ###

<h6> Usage </h6>

* `(begin <S-Expression>...)`

<h6> Description </h6>

* Executes `<S-Expression>...` in turns and returns last.

<h6> Example </h6>

    (display (begin
                 (display "Hello")
                 (display "World")))
    
    ;; Output
    ;; > Hello
    ;; > World
    ;; > World)...";
    help_dict_.emplace("begin", help);

    func = LC_FUNCTION_OBJ(GenScope);
    INSERT_LC_FUNCTION(func, "gen-scope", "Lisp:gen-scope");
    help =
R"...(### gen-scope ###

<h6> Usage </h6>

* `(gen-scope)`
* `(<Scope Name> <S-Expressions>...)`

<h6> Description </h6>

* Generates independent scope chain and return it.
    + The scope is caller's scope plus new empty scope.
* `(<Scope Name>)` executes `<S-Expression>...` in its own scope
  and returns last.

<h6> Example </h6>

    (define my-scope-1 (gen-scope))
    (define my-scope-2 (gen-scope))
    
    (define a "Global")
    (my-scope-1 (define a "Hello"))
    (my-scope-2 (define a "World"))
    
    (display a)
    (my-scope-1 (display a))
    (my-scope-2 (display a))
    ;; Output
    ;; > Global
    ;; > Hello
    ;; > World)...";
    help_dict_.emplace("gen-scope", help);

    func = LC_FUNCTION_OBJ(Display);
    INSERT_LC_FUNCTION(func, "display", "Lisp:display");
    INSERT_LC_FUNCTION(func, "print", "Lisp:print");
    help =
R"...(### display ###

<h6> Usage </h6>

* `(display <Object>...)`
* `(print <Object>...)`

<h6> Description </h6>

* Prints `<Object>` on Standard Output.

<h6> Example </h6>

    (define x 123)
    (display x)
    
    ;; Output
    ;; > 123
    
    (define x 123)
    (display "x is " x)
    
    ;; Output
    ;; > x is 123)...";
    help_dict_.emplace("display", help);
    help_dict_.emplace("print", help);

    func = LC_FUNCTION_OBJ(Stdin);
    INSERT_LC_FUNCTION(func, "stdin", "Lisp:stdin");
    help =
R"...(### stdin ###

<h6> Usage </h6>

* `(stdin <Message Symbol>)`

<h6> Description </h6>

* Returns String from Standard Input.
* `<Message Symbol>` is a message to the input stream.
    + `@get` : Reads one charactor.
    + `@read-line` : Reads one line. ('LF(CR+LF)' is omitted.)
    + `@read` : Reads all.
* If Standard Input is already closed, it returns Nil.

<h6> Example </h6>

    ;; Reads and shows one charactor from Standard Input.
    (display (stdin '@get))
    
    ;; Reads and shows one line from Standard Input.
    (display (stdin '@read-line))
    
    ;; Reads and shows all from Standard Input.
    (display (stdin '@read)))...";
    help_dict_.emplace("stdin", help);

    func = LC_FUNCTION_OBJ(Stdout);
    INSERT_LC_FUNCTION(func, "stdout", "Lisp:stdout");
    help =
R"...(### stdout ###

<h6> Usage </h6>

* `(stdout <String>)`

<h6> Description </h6>

* Prints `<String>` on Standard Output.

<h6> Example </h6>

    (stdout (to-string 123))
    (stdout "\n")
    
    ;; Output
    ;; > 123)...";
    help_dict_.emplace("stdout", help);

    func = LC_FUNCTION_OBJ(Stderr);
    INSERT_LC_FUNCTION(func, "stderr", "Lisp:stderr");
    help =
R"...(### stderr ###

<h6> Usage </h6>

* `(stderr <String>)`

<h6> Description </h6>

* Prints `<String>` on Standard Error.

<h6> Example </h6>

    (stderr (to-string 123))
    (stderr "\n")
    
    ;; Output
    ;; > 123)...";
    help_dict_.emplace("stderr", help);

    func = LC_FUNCTION_OBJ(Import);
    INSERT_LC_FUNCTION(func, "import", "Lisp:import");
    help =
R"...(### import ###

<h6> Usage </h6>

* `(import <File name : String>)`

<h6> Description </h6>

* Reads `<File name>` and executes it.
* Returns the last evaluated Object of `<File name>`.

<h6> Example </h6>

    ;; When the following code is written in 'hello.scm'
    ;;
    ;; (define a 111)
    ;; (define b 222)
    ;; (string-append "Hello " "World")  ;; <- The last S-Expression.
    
    (display (import "hello.scm"))
    (display "a: " a)
    (display "b: " b)
    
    ;; Output
    ;; > Hello World
    :: > a: 111
    :: > b: 222)...";
    help_dict_.emplace("import", help);

    func = LC_FUNCTION_OBJ(Export);
    INSERT_LC_FUNCTION(func, "export", "Lisp:export");
    help =
R"...(### export ###

<h6> Usage </h6>

* `(export <File name : String> <Object>...)`

<h6> Description </h6>

* Converts each `<Object>...` into String and appends it to `<File name>`.
* Returns String written to `<File name>`.

<h6> Example </h6>

    (display (export "aaa.scm" (list '+ 1 2 3) (list 'define 'x "abc")))
    ;; Output
    ;; > (+ 1 2 3)
    ;; > (define x "abc")
    ;;
    ;; In "aaa.scm"
    ;; > (+ 1 2 3)
    ;; > (define x "abc"))...";
    help_dict_.emplace("export", help);

    func = LC_FUNCTION_OBJ(EqualQ);
    INSERT_LC_FUNCTION(func, "equal?", "Lisp:equal?");
    help =
R"...(### equal? ###

<h6> Usage </h6>

* `(equal? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are same structure.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (equal? '(1 2 (3 4) 5) '(1 2 (3 4) 5)))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("import", help);

    func = LC_FUNCTION_OBJ(QFunc<LType::NIL>);
    INSERT_LC_FUNCTION(func, "nil?", "Lisp:nil?");
    INSERT_LC_FUNCTION(func, "null?", "Lisp:null?");
    help =
R"...(### nil? ###

<h6> Usage </h6>

* `(nil? <Object>...)`
* `(null? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Nil.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (nil? ()))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("nil?", help);
    help_dict_.emplace("null?", help);

    func = LC_FUNCTION_OBJ(QFunc<LType::PAIR>);
    INSERT_LC_FUNCTION(func, "pair?", "Lisp:pair?");
    help =
R"...(### pair? ###

<h6> Usage </h6>

* `(pair? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Pair.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (pair? '(1 2 3) '(4 5 6)))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("pair?", help);

    func = LC_FUNCTION_OBJ(QFunc<LType::SYMBOL>);
    INSERT_LC_FUNCTION(func, "symbol?", "Lisp:symbol?");
    help =
R"...(### symbol? ###

<h6> Usage </h6>

* `(symbol? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Symbol.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (symbol? 'x))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("symbol?", help);

    func = LC_FUNCTION_OBJ(QFunc<LType::NUMBER>);
    INSERT_LC_FUNCTION(func, "number?", "Lisp:number?");
    help =
R"...(### number? ###

<h6> Usage </h6>

* `(number? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Number.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (number? 123))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("number?", help);

    func = LC_FUNCTION_OBJ(QFunc<LType::BOOLEAN>);
    INSERT_LC_FUNCTION(func, "boolean?", "Lisp:boolean?");
    help =
R"...(### boolean? ###

<h6> Usage </h6>

* `(boolean? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Boolean.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (boolean? #f))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("boolean?", help);

    func = LC_FUNCTION_OBJ(QFunc<LType::STRING>);
    INSERT_LC_FUNCTION(func, "string?", "Lisp:string?");
    help =
R"...(### string? ###

<h6> Usage </h6>

* `(string? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are String.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (string? "Hello"))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("string?", help);

    func = LC_FUNCTION_OBJ(QFunc<LType::FUNCTION>);
    INSERT_LC_FUNCTION(func, "function?", "Lisp:function?");
    help =
R"...(### function? ###

<h6> Usage </h6>

* `(function? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Function.
  Otherwise, returns #f.

<h6> Example </h6>

    (define myfunc (lambda (x) (+ x 1)))
    (display (function? myfunc))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("function?", help);

    func = LC_FUNCTION_OBJ(QFunc<LType::N_FUNCTION>);
    INSERT_LC_FUNCTION(func, "native-function?", "Lisp:native-function?");
    help =
R"...(### native-function? ###

<h6> Usage </h6>

* `(native-function? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Native Function.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (native-function? +))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("native-function?", help);

    func = LC_FUNCTION_OBJ(ProcedureQ);
    INSERT_LC_FUNCTION(func, "procedure?", "Lisp:procedure?");
    help =
R"...(### procedure? ###

<h6> Usage </h6>

* `(procedure? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Function or Native Function.
  Otherwise, returns #f.

<h6> Example </h6>

    (define myfunc (lambda (x) (+ x 1)))
    (display (procedure? myfunc))
    
    ;; Output
    ;; > #t
    
    (display (procedure? +))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("procedure?", help);

    func = LC_FUNCTION_OBJ(OutputStream);
    INSERT_LC_FUNCTION(func, "output-stream", "Lisp:output-stream");
    help =
R"...(### output-stream ###

<h6> Usage </h6>

1. `(output-stream <File name : String>)`
2. `((output-stream <File name : String>) <String>)`

<h6> Description </h6>

* 1: Returns Native Function as an output stream of `<File name>`.
* 2: Writes `<String>` to `<File name>` and returns itself.
* If you give Nil to the Function, the stream will be closed.

<h6> Example </h6>

    ;; Opens "hello.txt".
    (define myfile (output-stream "hello.txt"))
    
    ;; Writes "Hello World" to "hello.txt".
    (myfile "Hello World\n")
    
    ;; Closes "hello.txt".
    (myfile ()))...";
    help_dict_.emplace("output-stream", help);

    func = LC_FUNCTION_OBJ(InputStream);
    INSERT_LC_FUNCTION(func, "input-stream", "Lisp:input-stream");
    help =
R"...(### input-stream ###

<h6> Usage </h6>

1. `(input-stream <File name : String>)`
2. `((input-stream <File name : String>) <Message Symbol : Symbol>)`

<h6> Description </h6>

* 1: Returns Native Function as an input stream of `<File name>`.
* 2: Returns String from `<File name>`.
* 2: `<Message Symbol>` is a message to the input stream.
    + `@get` : Reads one charactor.
    + `@read-line` : Reads one line. ('LF(CR+LF)' is omitted.)
    + `@read` : Reads all.
* If you give Nil to the stream, it will be closed.
* If the stream already closed, it returns empty string.

<h6> Example </h6>

    ;; Opens "hello.txt".
    (define myfile (input-stream "hello.txt"))
    
    ;; Reads and shows one charactor from "hello.txt".
    (display (myfile '@get))
    
    ;; Reads and shows one line from "hello.txt".
    (display (myfile '@read-line))
    
    ;; Reads and shows all from "hello.txt".
    (display (myfile '@read))
    
    ;; Closes "hello.txt".
    (myfile ()))...";
    help_dict_.emplace("input-stream", help);

    func = LC_FUNCTION_OBJ(GenThread);
    INSERT_LC_FUNCTION(func, "gen-thread", "Lisp:gen-thread");
    help =
R"...(### gen-thread ###

<h6> Usage </h6>

* `(gen-thread <Function>)`

<h6> Description </h6>

* Returns Thread object.
* It is controlled by Message Symbol.
    + `@start [<Args>...]` : Starts `<Function>` on another thread.
        - if `<Function>` accepts arguments, it gives `<Function>` `<Args>...`.
    + `@join` : Waits until `<Function>` to be terminated.
    + `@terminated?` : If `<Function>` has already been terminated, returns #t.

<h6> Example </h6>

    ;; Make function object.
    (define (inner-func)
            (display "Hello")
            (sleep 10)
            (display "World"))
    
    ;; Generate Thread object.
    (define my-thread (gen-thread inner-func))
    
    ;; Start new thread.
    (my-thread '@start)
    
    ;; Judge the thread is terminated.
    (sleep 2)
    (display (my-thread '@terminated?))
    
    ;; Wait for the thread.
    (my-thread '@join)
    
    ;; Output
    ;; > Hello
    ;; > #f  ;; the thread has not been terminated yet.
    ;; > World  ;; This word is printed after 10 seconds from "Hello".)...";
    help_dict_.emplace("gen-thread", help);

    func = LC_FUNCTION_OBJ(Sleep);
    INSERT_LC_FUNCTION(func, "sleep", "Lisp:sleep");
    help =
R"...(### sleep ###

<h6> Usage </h6>

* `(sleep <Seconds : Number>)`

<h6> Description </h6>

* Sleeps the current thread for `<Seconds>` seconds.

<h6> Example </h6>

    (display "Hello")
    
    (sleep 10)
    
    (display "World")
    ;; Output
    ;; > Hello
    ;; > World  ;; This word is printed after 10 seconds from "Hello".)...";
    help_dict_.emplace("sleep", help);

    func = LC_FUNCTION_OBJ(GenMutex);
    INSERT_LC_FUNCTION(func, "gen-mutex", "Lisp:gen-mutex");
    help =
R"...(### gen-mutex ###

<h6> Usage </h6>

* `(gen-mutex)`

<h6> Description </h6>

* Generates Mutex object.
* It is controlled by Message Symbol.
    + `@lock` : Locks mutex.
    + `@unlock` : Unlocks mutex.
    + `@synchronize <S-Expressions>...` :
        - Locks mutex and executes `<S-Expressions>...`.
            - If `<S-Expression>...` is ended,
              the mutex is unlocked and returns the last expression.
        - `<S-Expressions>...` is executed on new local scope.
        - `(wait)` is bound in the new local scope.
            - If `(wait)` is called, the mutex is unlocked
              and the thread sleeps until `@notify-one` or `@notify-all`.
    + `@notify-one` : Wakes one thread up at `(wait)`.
    + `@notify-all` : Wakes all threads up at `(wait)`.

<h6> Example </h6>

    ;; --- Test `@lock` and `@unlock` --- ;;
    ;; Generate Mutex.
    (define my-mutex (gen-mutex))
    
    ;; Define function that uses `@lock` and `@unlock`.
    (define (func-1 word)
            (my-mutex '@lock)
            (display "Hello : " word)
            (sleep 10)
            (my-mutex '@unlock))
    
    ;; Generate Threads.
    (define thread-1 (gen-thread (lambda () (func-1 "Apple"))))
    (define thread-2 (gen-thread (lambda () (func-1 "Banana"))))
    
    ;; Start threads.
    (thread-1 '@start)
    (thread-2 '@start)
    
    ;; Wait for threads.
    (thread-1 '@join)
    (thread-2 '@join)
    
    ;; Output
    ;; > Hello : Banana
    ;; > Hello : Apple  ;; This is printed after 10 seconds.
    
    ;; --- Test `@synchronize` --- ;;
    (define (func-2 word)
            (my-mutex '@synchronize
                      (display "Hello : " word)
                      (sleep 10)))
    
    (define thread-3 (gen-thread (lambda () (func-2 "Water"))))
    (define thread-4 (gen-thread (lambda () (func-2 "Fire"))))
    
    (thread-3 '@start)
    (thread-4 '@start)
    
    (thread-3 '@join)
    (thread-4 '@join)
    
    ;; Output
    ;; > Hello : Water
    ;; > Hello : Fire  ;; This is printed after 10 seconds.
    
    ;; --- Test `@notify-one` and `(wait)` --- ;;
    (define (func-3 word)
            (my-mutex '@synchronize
                      (wait)
                      (display "Hello : " word)))
    
    (define thread-5 (gen-thread (lambda () (func-3 "Sun"))))
    (define thread-6 (gen-thread (lambda () (func-3 "Moon"))))
    
    (thread-5 '@start)
    (thread-6 '@start)
    
    ;; Notify each thread.
    (sleep 2)
    (my-mutex '@notify-one)
    (sleep 10)
    (my-mutex '@notify-one)
    
    (thread-5 '@join)
    (thread-6 '@join)
    
    ;; Output
    ;; > Hello : Sun
    ;; > Hello : Moon  ;; This is printed after 10 seconds.
    
    ;; --- Test `@notify-all` and `(wait)` --- ;;
    (define thread-7 (gen-thread (lambda () (func-3 "Mario"))))
    (define thread-8 (gen-thread (lambda () (func-3 "Sonic"))))
    
    (thread-7 '@start)
    (thread-8 '@start)
    
    ;; Notify all threads.
    (sleep 10)
    (my-mutex '@notify-all)
    
    (thread-7 '@join)
    (thread-8 '@join)
    
    ;; Output
    ;; > Hello : Mario
    ;; > Hello : Sonic  ;; This is printed same time.)...";
    help_dict_.emplace("gen-mutex", help);

    func = LC_FUNCTION_OBJ(System);
    INSERT_LC_FUNCTION(func, "system", "Lisp:system");
    help =
R"...(### system ###

<h6> Usage </h6>

* `(system <Command line : String>)`

<h6> Description </h6>

* Executes `<Command line>` and returns its status.
    + This is same as `system()` of libc.

<h6> Example </h6>

    (display (system "echo 'Hello World'"))
    ;; Output
    ;; > Hello World
    ;; > 0)...";
    help_dict_.emplace("system", help);

    func = LC_FUNCTION_OBJ(GetEnv);
    INSERT_LC_FUNCTION(func, "get-env", "Lisp:get-env");
    help =
R"...(### get-env ###

<h6> Usage </h6>

* `(get-env <Variable name : String>)`

<h6> Description </h6>

* Returns Environment Variable of `<Varibable name>`.
* If no such variable, it returns Nil.

<h6> Example </h6>

    (display (get-env "USER"))
    ;; Output
    ;; > hironori)...";
    help_dict_.emplace("get-env", help);
  }

  // 基本関数を登録する。
  void Lisp::SetBasicFunctions() {
    LC_Function func;
    std::string help;

    func = LC_FUNCTION_OBJ(Append);
    INSERT_LC_FUNCTION(func, "append", "Lisp:append");
    INSERT_LC_FUNCTION(func, "string-append", "Lisp:string-append");
    help =
R"...(### append ###

<h6> Usage </h6>

1. `(append <List> <Object>...)`
2. `(append <String> <Object>...)`
  + or `(string-append <String> <Object>...)`

<h6> Description </h6>

1. If the 1st argument is List, appends `<Object>...` to its Cdr.
2. If the 1st argument is String,
   converts `<Object>...` into String and concatenates them.

<h6> Example </h6>

    (display (append '(111 222) '(333 444) '(555 666) 777))
    
    ;; Output
    ;; > (111 222 333 444 555 666 . 777)
    
    (display (append "Hello " 111 " World"))
    
    ;; Output
    ;; > "Hello 111 World")...";
    help_dict_.emplace("append", help);
    help_dict_.emplace("string-append", help);

    func = LC_FUNCTION_OBJ(Reverse);
    INSERT_LC_FUNCTION(func, "reverse", "Lisp:reverse");
    help =
R"...(### reverse ###

<h6> Usage </h6>

* `(reverse <List>)`

<h6> Description </h6>

* Reverses `<List>` and returns it.

<h6> Example </h6>

    (define li '(111 222 333 444 555))
    (display (reverse li))
    ;; Output
    ;; > (555 444 333 222 111))...";
    help_dict_.emplace("reverse", help);

    func = LC_FUNCTION_OBJ(Ref);
    INSERT_LC_FUNCTION(func, "ref", "Lisp:ref");
    INSERT_LC_FUNCTION(func, "list-ref", "Lisp:list-ref");
    INSERT_LC_FUNCTION(func, "string-ref", "Lisp:string-ref");
    help =
R"...(### ref ###

<h6> Usage </h6>

1. `(ref <List> <Index : Number>)`
  + or `(list-ref <List> <Index : Number>)`
2. `(ref <String> <Index : Number>)`
  + or `(string-ref <String> <Index : Number>)`

<h6> Description </h6>

1. If the 1st argument is List, returns a element of `<Index>`th `<List>`.
1. If the 1st argument is String, returns a letter of `<Index>`th `<String>`.
* The index of 1st element is 0.
* If `<Index>` is negative number,
  It counts from the tail of `<List | String>`.

<h6> Example </h6>

    (display (ref '(111 222 333) 1))
    
    ;; Output
    ;; > 222
    
    (display (ref '(111 222 333) -1))
    
    ;; Output
    ;; > 333
    
    (display (ref "Hello World" 4))
    
    ;; Output
    ;; > "o"
    
    (display (ref "Hello World" -3))
    
    ;; Output
    ;; > "r")...";
    help_dict_.emplace("ref", help);
    help_dict_.emplace("list-ref", help);
    help_dict_.emplace("string-ref", help);

    func = LC_FUNCTION_OBJ(List);
    INSERT_LC_FUNCTION(func, "list", "Lisp:list");
    help =
R"...(### list ###


<h6> Usage </h6>

* `(list <Object>...)`

<h6> Description </h6>

* Returns List composed of `<Object>...`.

<h6> Example </h6>

    (display (list 111 222 333))
    
    ;; Output
    ;; > (111 222 333))...";
    help_dict_.emplace("list", help);

    func = LC_FUNCTION_OBJ(ListReplace);
    INSERT_LC_FUNCTION(func, "list-replace", "Lisp:list-replace");
    help =
R"...(### list-replace ###

<h6> Usage </h6>

* `(list-replace <List> <Index : Number> <Object>)`

<h6> Description </h6>

* Returns List which has replaced the `<Index>`th element of
  `<List>` for `<Object>`.
* The 1st element of `<List>` is 0.
* If `<Index>` is negative number," It counts from the tail of `<List>`.

<h6> Example </h6>

    (define lst (list 111 222 333))
    (display (list-replace lst 1 "Hello"))
    
    ;; Output
    ;; > (111 "Hello" 333))...";
    help_dict_.emplace("list-replace", help);

    func = LC_FUNCTION_OBJ(ListRemove);
    INSERT_LC_FUNCTION(func, "list-remove", "Lisp:list-remove");
    help =
R"...(### list-remove ###

<h6> Usage </h6>

* `(list-remove <List> <Index : Number>)`

<h6> Description </h6>

* Returns List which has removed the `<Index>`th element of `<List>`.
* The 1st element of `<List>` is 0.
* If `<Index>` is negative number," It counts from the tail of `<List>`.

<h6> Example </h6>

    (define lst (list 111 222 333))
    (display (list-remove lst 1))
    
    ;; Output
    ;; > (111 333))...";
    help_dict_.emplace("list-remove", help);

    func = LC_FUNCTION_OBJ(ListInsert);
    INSERT_LC_FUNCTION(func, "list-insert", "Lisp:list-insert");
    help =
R"...(### list-insert ###

<h6> Usage </h6>

* `(list-insert <List> <Index : Number> <Object>)`

<h6> Description </h6>

* Insert `<Object>` into `<Index>` of `<List>` and returns it.
* The 1st element of `<List>` is 0.
* If `<Index>` is negative number," It counts from the tail of `<List>`.

<h6> Example </h6>

    (display (list-insert '(111 222 333) 0 "Hello World"))
    (display (list-insert '(111 222 333) 1 "Hello World"))
    (display (list-insert '(111 222 333) 2 "Hello World"))
    (display (list-insert '(111 222 333) 3 "Hello World"))
    ;; Output
    ;; > ("Hello World" 111 222 333)
    ;; > (111 "Hello World" 222 333)
    ;; > (111 222 "Hello World" 333)
    ;; > (111 222 333 "Hello World"))...";
    help_dict_.emplace("list-insert", help);

    func = LC_FUNCTION_OBJ(ListSearch);
    INSERT_LC_FUNCTION(func, "list-search", "Lisp:list-search");
    help =
R"...(### list-search ###

<h6> Usage </h6>

* `(list-search <List> <Object>)`

<h6> Description </h6>

* If `<List>` has an object same as `<Object>`,
  it returns index number of the object.  
  Otherwise it returns Nil.

<h6> Example </h6>

    (define lst '(111 222 "Hello" #t))
    
    (display (list-search lst "Hello"))
    (display (list-search lst "World"))
    
    ;; Output
    ;; >  2
    ;; > ())...";
    help_dict_.emplace("list-search", help);

    func = LC_FUNCTION_OBJ(ListPath);
    INSERT_LC_FUNCTION(func, "list-path", "Lisp:list-path");
    help =
R"...(### list-path ###

<h6> Usage </h6>

* `(list-path <List> <Path : String>)`

<h6> Description </h6>

* Returns an element indicated by `<Path>` from `<List>`.
    + `<Path>` is composed of 'a' or 'd'.
        - 'a' means Car.
        - 'b' means Cdr.
        - If `<Path>` is "dadda" and name of List is 'lst',
          it means `(car (cdr (cdr (car (cdr lst)))))`.

<h6> Example </h6>

    (define lst '(111 (222 333 444) 555 666))
    
    (display (list-path lst "dda"))
    ;; Output
    ;; > 555
    
    (display (list-path lst "dada"))
    ;; Output
    ;; > 333)...";
    help_dict_.emplace("list-path", help);

    func = LC_FUNCTION_OBJ(ListPathReplace);
    INSERT_LC_FUNCTION(func, "list-path-replace", "Lisp:list-path-replace");
    help =
R"...(### list-path-replace ###

<h6> Usage </h6>

* `(list-path-replace <List> <Path : String> <Object>)`

<h6> Description </h6>

* Replaces an element of `<List>` indicated by `<Path>` for `<Object>`
  and returns it.
    + `<Path>` is composed of 'a' or 'd'.
        - 'a' means Car.
        - 'b' means Cdr.
        - If `<Path>` is "dadda" and name of List is 'lst',
          it means `(car (cdr (cdr (car (cdr lst)))))`.

<h6> Example </h6>

    (define lst '(111 (222 333 444) 555 666))
    
    (display (list-path-replace lst "dada" "Hello"))
    ;; Output
    ;; > (111 (222 "Hello" 444) 555 666))...";
    help_dict_.emplace("list-path-replace", help);

    func = LC_FUNCTION_OBJ(ListSort);
    INSERT_LC_FUNCTION(func, "list-sort", "Lisp:list-sort");
    help =
R"...(### list-sort ###

<h6> Usage </h6>

* `(list-sort <List> <Predicate : Function>)`

<h6> Description </h6>

* Sorts `<List>` for `<Predicate>` to be true and returns it.
* `<Predicate>` defines the order.
    + It accepts 2 arguments and returns Boolean.
        - The 1st argument is previous element. The 2nd is next element.
        - It must judge whether the order is right or not and returns it.

<h6> Example </h6>
(define li '(9 3 5 8 1 4 7 6 0 2))

(display (list-sort li (lambda (prev next) (< prev next))))
;; Output
;; > (0 1 2 3 4 5 6 7 8 9)

(display (list-sort li (lambda (prev next) (> prev next))))
;; Output
;; > (9 8 7 6 5 4 3 2 1 0))...";
    help_dict_.emplace("list-sort", help);

    func = LC_FUNCTION_OBJ(Zip);
    INSERT_LC_FUNCTION(func, "zip", "Lisp:zip");
    help =
R"...(### zip ###

<h6> Usage </h6>

* `(zip <List>...)`

<h6> Description </h6>

* Unions each element of `<List>...` and returns it.

<h6> Example </h6>

    (display (zip '(a b c)
                  '(1 2 3)
                  '("Hello" "World")
                  '("aaa" "bbb" "ccc" "ddd")))
    ;; Output
    ;; > ((a 1 "Hello" "aaa") (b 2 "World" "bbb") (c 3 "ccc") ("ddd")))...";
    help_dict_.emplace("zip", help);

    func = LC_FUNCTION_OBJ(Map);
    INSERT_LC_FUNCTION(func, "map", "Lisp:map");
    help =
R"...(### map ###

<h6> Usage </h6>

* `(map <Function : Symbol or Function> <Argument list : List>...)`

<h6> Description </h6>

* Applies `<Function>` to `<Argument List>...` iteratively
  and returns List composed with returned object by `<Function>`.
      + It looks like addition of Linear Algebra. See Example.

<h6> Example </h6>

    (display (map '+ '(1 2 3 4 5)
                     '(10 20 30 40 50)
                     '(100 200 300 400 500)))
    ;; The above means...
    ;; (display (list (+ 1 10 100)
    ;;                (+ 2 20 200)
    ;;                (+ 3 30 300)
    ;;                (+ 4 40 400)
    ;;                (+ 5 50 500)))
    
    ;; Output
    ;; > (111 222 333 444 555))...";
    help_dict_.emplace("map", help);

    func = LC_FUNCTION_OBJ(Filter);
    INSERT_LC_FUNCTION(func, "filter", "Lisp:filter");
    help =
R"...(### filter ###

<h6> Usage </h6>

* `(filter <Function : Symbol or Function> <List>)`

<h6> Description </h6>

* Judges each element of `<List>` by `<Function>`
  and returns List of the passed elements.
* `<Function>` must return #t or #f.
* `<Function>` accepts one argument.

<h6> Example </h6>

    (define li '(1 2 3 4 5 6))
    (define (my-func x) (> x 3))
    
    (display (filter my-func li))
    ;; Output
    ;; > (4 5 6))...";
    help_dict_.emplace("filter", help);

    func = LC_FUNCTION_OBJ(Range);
    INSERT_LC_FUNCTION(func, "range", "Lisp:range");
    help =
R"...(### range ###

<h6> Usage </h6>

* `(range <Size : Number>)`

<h6> Description </h6>

* Returns List composed with 0...(`<Size>` - 1).

<h6> Example </h6>

    (display (range 10))
    
    ;; Output
    ;; > (0 1 2 3 4 5 6 7 8 9))...";
    help_dict_.emplace("range", help);

    func = LC_FUNCTION_OBJ(StartSizeInc);
    INSERT_LC_FUNCTION(func, "start-size-inc", "Lisp:start-size-inc");
    help =
R"...(### start-size-inc ###

<h6> Usage </h6>

* `(start-size-inc <Start : Number> <Size : Number> <Increment : Number>)`

<h6> Description </h6>

* Returns List of `<Size>` elements.
    + The 1st element is `<Start>`.
    + From the 2nd element, the previous element plus `<Increment>`.

<h6> Example </h6>

    (display (start-size-inc 2 6 -0.5))
    
    ;; Output
    ;; > (2 1.5 1 0.5 0 -0.5))...";
    help_dict_.emplace("start-size-inc", help);

    func = LC_FUNCTION_OBJ(LengthFunc);
    INSERT_LC_FUNCTION(func, "length", "Lisp:length");
    help =
R"...(### length ###

<h6> Usage </h6>

* `(length <Object>)`

<h6> Description </h6>

* If `<Object>` is List, it returns the number of elements.
* If `<Object>` is String, it returns the length of string.
* If `<Object>` is Atom, it returns 1.

<h6> Example </h6>

    (display (length '(111 222 333 444 555 666)))
    ;; Output
    ;; > 6
    
    (display (length "Hello"))
    ;; Output
    ;; > 5)...";
    help_dict_.emplace("length", help);

    func = LC_FUNCTION_OBJ(NumEqual);
    INSERT_LC_FUNCTION(func, "=", "Lisp:=");
    help =
R"...(### = ###

<h6> Usage </h6>

* `(= <Number>...)`

<h6> Description </h6>

* Returns #t if the 1st Number equals the others.
  Otherwise, return #f.

<h6> Example </h6>

    (display (= 111 111 111))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("=", help);

    func = LC_FUNCTION_OBJ(NumNotEqual);
    INSERT_LC_FUNCTION(func, "~=", "Lisp:~=");
    help =
R"...(### ~= ###

<h6> Usage </h6>

* `(~= <Number>...)`

<h6> Description </h6>

* Returns #f if the 1st Number equals the others.
  Otherwise, return #t.

<h6> Example </h6>

    (display (~= 111 111 111))
    
    ;; Output
    ;; > #f)...";
    help_dict_.emplace("~=", help);

    func = LC_FUNCTION_OBJ(NumGT);
    INSERT_LC_FUNCTION(func, ">", "Lisp:>");
    help =
R"...(### > ###

<h6> Usage </h6>

* `(> <Number>...)`

<h6> Description </h6>

* Returns #t if a Number is more than next Number.
  Otherwise, return #f.

<h6> Example </h6>

    (display (> 333 222 111))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace(">", help);

    func = LC_FUNCTION_OBJ(NumGE);
    INSERT_LC_FUNCTION(func, ">=", "Lisp:>=");
    help =
R"...(### >= ###

<h6> Usage </h6>

* `(>= <Number>...)`

<h6> Description </h6>

* Returns #t if a Number is more or equal than next Number.
  Otherwise, return #f.

<h6> Example </h6>

    (display (>= 333 222 111))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace(">=", help);

    func = LC_FUNCTION_OBJ(NumLT);
    INSERT_LC_FUNCTION(func, "<", "Lisp:<");
    help =
R"...(### < ###

<h6> Usage </h6>

* `(< <Number>...)`

<h6> Description </h6>

* Returns #t if a Number is less than next Number.
  Otherwise, return #f.

<h6> Example </h6>

    (display (< 111 222 333))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("<", help);

    func = LC_FUNCTION_OBJ(NumLE);
    INSERT_LC_FUNCTION(func, "<=", "Lisp:<=");
    help =
R"...(### <= ###

<h6> Usage </h6>

* `(<= <Number>...)`

<h6> Description </h6>

* Returns #t if a Number is less or equal than next Number.
  Otherwise, return #f.

<h6> Example </h6>

    (display (< 111 222 333))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("<=", help);

    func = LC_FUNCTION_OBJ(EvenQ);
    INSERT_LC_FUNCTION(func, "even?", "Lisp:even?");
    help =
R"...(### even? ###

<h6> Usage </h6>

* `(even? <Number>)`

<h6> Description </h6>

* Judges whether `<Number>` is an even number or not.

<h6> Example </h6>

    (display (even? 1))
    ;; Output
    ;; > #f
    
    (display (even? 2))
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("even?", help);

    func = LC_FUNCTION_OBJ(OddQ);
    INSERT_LC_FUNCTION(func, "odd?", "Lisp:odd?");
    help =
R"...(### odd? ###

<h6> Usage </h6>

* `(odd? <Number>)`

<h6> Description </h6>

* Judges whether `<Number>` is an odd number or not.

<h6> Example </h6>

    (display (odd? 1))
    ;; Output
    ;; > #t
    
    (display (odd? 2))
    ;; Output
    ;; > #f)...";
    help_dict_.emplace("odd?", help);

    func = LC_FUNCTION_OBJ(Not);
    INSERT_LC_FUNCTION(func, "not", "Lisp:not");
    help =
R"...(### not ###

<h6> Usage </h6>

* `(not <Boolean>)`

<h6> Description </h6>

* Turns `<Boolean>` to opposite value. #t to #f, #f to #t.

<h6> Example </h6>

    (display (not (= 111 111)))
    
    ;; Output
    ;; > #f)...";
    help_dict_.emplace("not", help);

    func = LC_FUNCTION_OBJ(And);
    INSERT_LC_FUNCTION(func, "and", "Lisp:and");
    help =
R"...(### and ###

<h6> Usage </h6>

* `(and <Boolean>...)`

<h6> Description </h6>

* Returns #t if all `<Boolean>...` are #t.
  Otherwise, return #f.

<h6> Example </h6>

    (display (and (= 111 111) (= 222 222)))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("and", help);

    func = LC_FUNCTION_OBJ(Or);
    INSERT_LC_FUNCTION(func, "or", "Lisp:or");
    help =
R"...(### or ###

<h6> Usage </h6>

* `(or <Boolean>...)`

<h6> Description </h6>

* Returns #t if one of `<Boolean>...` is #t.
  If all `<Boolean>` are #f, return #f.

<h6> Example </h6>

    (display (or (= 111 111) (= 222 333)))
    
    ;; Output
    ;; > #t)...";
    help_dict_.emplace("or", help);

    func = LC_FUNCTION_OBJ(Addition);
    INSERT_LC_FUNCTION(func, "+", "Lisp:+");
    help =
R"...(### + ###

<h6> Usage </h6>

* `(+ <Number>...)`

<h6> Description </h6>

* Sums up all `<Number>...`.

<h6> Example </h6>

    (display (+ 1 2 3))
    
    ;; Output
    ;; > 6)...";
    help_dict_.emplace("+", help);

    func = LC_FUNCTION_OBJ(AdditionEx);
    INSERT_LC_FUNCTION(func, "add!", "Lisp:add!");
    help =
R"...(### add! ###

<h6> Usage </h6>

* `(add! <Symbol> <Number>...)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with Number.
    + `<Symbol>` is not evaluated.
* Sums up `<Number>...` and adds it to `<Symbol>`.
* Returns previous Number bound to `<Symbol>`.

<h6> Example </h6>

    (define n 5)
    (display (add! n 3 4 5))  ;; Returns previous Number.
    (display n)
    
    ;; Output
    ;; > 5
    :: > 17)...";
    help_dict_.emplace("add!", help);

    func = LC_FUNCTION_OBJ(Subtraction);
    INSERT_LC_FUNCTION(func, "-", "Lisp:-");
    help =
R"...(### - ###

<h6> Usage </h6>

* `(- <1st number> <Number>...)`

<h6> Description </h6>

* Subtracts `<Number>...` from `<1st number>`.

<h6> Example </h6>

    (display (- 5 4 3))
    
    ;; Output
    ;; > -2)...";
    help_dict_.emplace("-", help);

    func = LC_FUNCTION_OBJ(SubtractionEx);
    INSERT_LC_FUNCTION(func, "sub!", "Lisp:sub!");
    help =
R"...(### sub! ###

<h6> Usage </h6>

* `(sub! <Symbol> <Number>...)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with Number.
    + `<Symbol>` is not evaluated.
* Subtracts `<Number>...` from `<Symbol>` and rewrite it to `<Symbol>`.
* Returns previous Number bound to `<Symbol>`.

<h6> Example </h6>

    (define n 5)
    (display (sub! n 1 2))  ;; Returns previous Number.
    (display n)
    
    ;; Output
    ;; > 5
    ;; > 2)...";
    help_dict_.emplace("sub!", help);

    func = LC_FUNCTION_OBJ(Multiplication);
    INSERT_LC_FUNCTION(func, "*", "Lisp:*");
    help =
R"...(### * ###

<h6> Usage </h6>

* `(* <Number>...)`

<h6> Description </h6>

* Multiplies all `<Number>...`.

<h6> Example </h6>

    (display (* 2 3 4))
    
    ;; Output
    ;; > 24)...";
    help_dict_.emplace("*", help);

    func = LC_FUNCTION_OBJ(MultiplicationEx);
    INSERT_LC_FUNCTION(func, "mul!", "Lisp:mul!");
    help =
R"...(### mul! ###

<h6> Usage </h6>

* `(mul! <Symbol> <Number>...)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with Number.
    + `<Symbol>` is not evaluated.
* Multiplies `<Symbol>` and `<Number>...` and rewrite it to <Symbol>.
* Returns previous Number bound to `<Symbol>`.

<h6> Example </h6>

    (define n 5)
    (display (mul! n 1 2))  ;; Returns previous Number.
    (display n)
    
    ;; Output
    ;; > 5
    ;; > 10)...";
    help_dict_.emplace("mul!", help);

    func = LC_FUNCTION_OBJ(Division);
    INSERT_LC_FUNCTION(func, "/", "Lisp:/");
    help =
R"...(### / ###

<h6> Usage </h6>

* `(/ <1st number> <Number>...)`

<h6> Description </h6>

* Divides `<1st number>` with `<Number>...`.

<h6> Example </h6>

    (display (/ 32 2 4))
    
    ;; Output
    ;; > 4)...";
    help_dict_.emplace("/", help);

    func = LC_FUNCTION_OBJ(DivisionEx);
    INSERT_LC_FUNCTION(func, "div!", "Lisp:div!");
    help =
R"...(### div! ###

<h6> Usage </h6>

* `(div! <Symbol> <Number>...)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with Number.
    + `<Symbol>` is not evaluated.
* Divides `<Symbol>` with `<Number>...` and rewrites it to `<Symbol>`.
* Returns previous Number bound to `<Symbol>`.

<h6> Example </h6>

    (define n 6)
    (display (div! n 2 3))  ;; Returns previous Number.
    (display n)
    
    ;; Output
    ;; > 6
    ;; > 1)...";
    help_dict_.emplace("div!", help);

    func = LC_FUNCTION_OBJ(Inc);
    INSERT_LC_FUNCTION(func, "++", "Lisp:++");
    help =
R"...(### ++ ###

<h6> Usage </h6>

* `(++ <Number>)`

<h6> Description </h6>

* Adds `<Number>` to '1'.

<h6> Example </h6>

    (display (++ 111))
    
    ;; Output
    ;; > 112)...";
    help_dict_.emplace("++", help);

    func = LC_FUNCTION_OBJ(IncEx);
    INSERT_LC_FUNCTION(func, "inc!", "Lisp:inc!");
    help =
R"...(### inc! ###

<h6> Usage </h6>

* `(inc! <Symbol)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with Number.
    + `<Symbol>` is not evaluated.
* Increments `<Symbol>`.
* Returns previous Number bound to `<Symbol>`.

<h6> Example </h6>

    (define i 111)
    (display (inc! i))
    (display i)
    
    ;; Output
    ;; > 111
    ;; > 112)...";
    help_dict_.emplace("inc!", help);

    func = LC_FUNCTION_OBJ(Dec);
    INSERT_LC_FUNCTION(func, "--", "Lisp:--");
    help =
R"...(### -- ###

<h6> Usage </h6>

* `(-- <Number>)`

<h6> Description </h6>

* Subtracts '1' from `<Number>`.

<h6> Example </h6>

    (display (-- 111))
    
    ;; Output
    ;; > 110)...";
    help_dict_.emplace("--", help);

    func = LC_FUNCTION_OBJ(DecEx);
    INSERT_LC_FUNCTION(func, "dec!", "Lisp:dec!");
    help =
R"...(### dec! ###

<h6> Usage </h6>

* `(dec! <Symbol)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with Number.
    + `<Symbol>` is not evaluated.
* Decrements `<Symbol>`.
* Returns previous Number bound to `<Symbol>`.

<h6> Example </h6>

    (define i 111)
    (display (dec! i))
    (display i)
    
    ;; Output
    ;; > 111
    ;; > 110)...";
    help_dict_.emplace("dec!", help);

    func = LC_FUNCTION_OBJ(StringSplit);
    INSERT_LC_FUNCTION(func, "string-split", "Lisp:string-split");
    help =
R"...(### string-split ###

<h6> Usage </h6>

* `(string-split <String> <Delim :String>)`

<h6> Description </h6>

* Separates `<Sting>` by `<Delim>` and returns the List.

<h6> Example </h6>

    (display (string-split "aaaaSplit!bbbSplit!ccc" "Split!"))
    ;; Output
    ;; > ("aaa" "bbb" "ccc"))...";
    help_dict_.emplace("string-split", help);

    func = LC_FUNCTION_OBJ(StringJoin);
    INSERT_LC_FUNCTION(func, "string-join", "Lisp:string-join");
    help =
R"...(### string-join ###

<h6> Usage </h6>

* `(string-join <List> <Delim : String>)`

<h6> Description </h6>

* Puts `<Delim>` between each elements of `<List>` and returns it.

<h6> Example </h6>

    (define ls '("Hello" "World" 111 222))
    (display (string-join ls "[Delim!!]"))
    ;; Output
    ;; > Hello[Delim!!]World[Delim!!]111[Delim!!]222)...";
    help_dict_.emplace("string-join", help);

    func = LC_FUNCTION_OBJ(Front);
    INSERT_LC_FUNCTION(func, "front", "Lisp:front");
    help =
R"...(### front ###

<h6> Usage </h6>

* `(front <List>)`

<h6> Description </h6>

* Returns the first element of `<List>`.

<h6> Example </h6>

    (display (front '(111 222 333)))
    
    ;; Output
    ;; > 111)...";
    help_dict_.emplace("front", help);

    func = LC_FUNCTION_OBJ(Back);
    INSERT_LC_FUNCTION(func, "back", "Lisp:back");
    help =
R"...(### back ###

<h6> Usage </h6>

* `(back <List>)`

<h6> Description </h6>

* Returns the last element of `<List>`.

<h6> Example </h6>

    (display (back '(111 222 333)))
    
    ;; Output
    ;; > 333)...";
    help_dict_.emplace("back", help);

    func = LC_FUNCTION_OBJ(PushFront);
    INSERT_LC_FUNCTION(func, "push-front", "Lisp:push-front");
    help =
R"...(### push-front ###

<h6> Usage </h6>

* `(push-front <List> <Object>)`

<h6> Description </h6>

* Returns List added `<Object>` at the first element of `<List>`

<h6> Example </h6>

    (display (push-front '(111 222 333) "Hello"))
    
    ;; Output
    ;; > ("Hello" 111 222 333))...";
    help_dict_.emplace("push-front", help);

    func = LC_FUNCTION_OBJ(PushFrontEx);
    INSERT_LC_FUNCTION(func, "push-front!", "Lisp:push-front!");
    help =
R"...(### push-front! ###

<h6> Usage </h6>

* `(push-front! <Symbol> <Object>)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with List.
    + `<Symbol>` is not evaluated.
* Appends to the top of List of `<Symbol>`.
* Returns previous List bound to `<Symbol>`.

<h6> Example </h6>

    (define l '(111 222 333))
    (display (push-front! l 444))  ;; Returns previous List.
    (display l)
    
    ;; Output
    ;; > (111 222 333)
    ;; > (444 111 222 333))...";
    help_dict_.emplace("push-front!", help);

    func = LC_FUNCTION_OBJ(PopFront);
    INSERT_LC_FUNCTION(func, "pop-front", "Lisp:pop-front");
    help =
R"...(### pop-front ###

<h6> Usage </h6>

* `(pop-front <List>)`

<h6> Description </h6>

* Returns List removed the first element from `<List>`.

<h6> Example </h6>

    (display (pop-front '(111 222 333)))
    
    ;; Output
    ;; > (222 333))...";
    help_dict_.emplace("pop-front", help);

    func = LC_FUNCTION_OBJ(PopFrontEx);
    INSERT_LC_FUNCTION(func, "pop-front!", "Lisp:pop-front!");
    help =
R"...(### pop-front! ###

<h6> Usage </h6>

* `(pop-front! <Symbol>)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with List.
* Pops out the 1st element of List of `<Symbol>`.
* Returns previous List bound to `<Symbol>`.

<h6> Example </h6>

    (define l '(111 222 333))
    (display (pop-front! l))
    (display l)
    
    ;; Output
    ;; > (111 222 333)
    ;; > (222 333))...";
    help_dict_.emplace("pop-front!", help);

    func = LC_FUNCTION_OBJ(PushBack);
    INSERT_LC_FUNCTION(func, "push-back", "Lisp:push-back");
    help =
R"...(### push-back ###

<h6> Usage </h6>

* `(push-back <List> <Object>)`

<h6> Description </h6>

* Returns List added `<Object>` at the last element of `<List>`

<h6> Example </h6>

    (display (push-back '(111 222 333) "Hello"))
    
    ;; Output
    ;; > (111 222 333 "Hello"))...";
    help_dict_.emplace("push-back", help);

    func = LC_FUNCTION_OBJ(PushBackEx);
    INSERT_LC_FUNCTION(func, "push-back!", "Lisp:push-back!");
    help =
R"...(### push-back! ###

<h6> Usage </h6>

* `(push-back! <Symbol> <Object>)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with List.
    + `<Symbol>` is not evaluated.
* Appends to the tail of List of `<Symbol>`.
* Returns previous List bound to `<Symbol>`.

<h6> Example </h6>

    (define l '(111 222 333))
    (display (push-back! l 444))
    (display l)
    
    ;; Output
    ;; > (111 222 333)
    ;; > (111 222 333 444))...";
    help_dict_.emplace("push-back!", help);

    func = LC_FUNCTION_OBJ(PopBack);
    INSERT_LC_FUNCTION(func, "pop-back", "Lisp:pop-back");
    help =
R"...(### pop-back ###

<h6> Usage </h6>

* `(pop-back <List>)`

<h6> Description </h6>

* Returns List removed the last element from `<List>`.

<h6> Example </h6>

    (display (pop-back '(111 222 333)))
    
    ;; Output
    ;; > (111 222))...";
    help_dict_.emplace("pop-back", help);

    func = LC_FUNCTION_OBJ(PopBackEx);
    INSERT_LC_FUNCTION(func, "pop-back!", "Lisp:pop-back!");
    help =
R"...(### pop-back! ###

<h6> Usage </h6>

* `(pop-back! <Symbol>)`

<h6> Description </h6>

* This is Special Form.
    + The 1st argument must be Symbol bound with List.
* Pops out the last element of List of `<Symbol>`.
* Returns previous List bound to `<Symbol>`.

<h6> Example </h6>

    (define l '(111 222 333))
    (display (pop-back! l))
    (display l)
    
    ;; Output
    ;; > (111 222 333)
    ;; > (111 222))...";
    help_dict_.emplace("pop-back!", help);

    scope_chain_.InsertSymbol("PI", NewNumber(4.0 * std::atan(1.0)));
    help =
R"...(### PI ###

<h6> Description </h6>

* Circular constant.

<h6> Example </h6>

    (display PI)
    
    ;; Output
    ;; > 3.14159265358979)...";
    help_dict_.emplace("PI", help);

    scope_chain_.InsertSymbol("E", NewNumber(std::exp(1.0)));
    help =
R"...(### E ###

<h6> Description </h6>

* Napier's constant.

<h6> Example </h6>

    (display E)
    
    ;; Output
    ;; > 2.71828182845905)...";
    help_dict_.emplace("E", help);

    func = LC_FUNCTION_OBJ(Sin);
    INSERT_LC_FUNCTION(func, "sin", "Lisp:sin");
    help =
R"...(### sin ###

<h6> Usage </h6>

* `(sin <Number>)`

<h6> Description </h6>

* Sine. A trigonometric function.
* `<Number>` is radian.

<h6> Example </h6>

    (display (sin (/ PI 2)))
    
    ;; Output
    ;; > 1)...";
    help_dict_.emplace("sin", help);

    func = LC_FUNCTION_OBJ(Cos);
    INSERT_LC_FUNCTION(func, "cos", "Lisp:cos");
    help =
R"...(### cos ###

<h6> Usage </h6>

* `(cos <Number>)`

<h6> Description </h6>

* Cosine. A trigonometric function.
* `<Number>` is radian.

<h6> Example </h6>

    (display (cos PI))
    
    ;; Output
    ;; > -1)...";
    help_dict_.emplace("cos", help);

    func = LC_FUNCTION_OBJ(Tan);
    INSERT_LC_FUNCTION(func, "tan", "Lisp:tan");
    help =
R"...(### tan ###

<h6> Usage </h6>

* `(tan <Number>)`

<h6> Description </h6>

* Tangent. A trigonometric function.
* `<Number>` is radian.

<h6> Example </h6>

    (display (tan (/ PI 4)))
    
    ;; Output
    ;; > 1)...";
    help_dict_.emplace("tan", help);

    func = LC_FUNCTION_OBJ(ASin);
    INSERT_LC_FUNCTION(func, "asin", "Lisp:asin");
    help =
R"...(### asin ###

<h6> Usage </h6>

* `(asin <Number>)`

<h6> Description </h6>

* Arc sine. A trigonometric function.
* `<Number>` is sine.

<h6> Example </h6>

    (display (asin 0))
    
    ;; Output
    ;; > 0)...";
    help_dict_.emplace("asin", help);

    func = LC_FUNCTION_OBJ(ACos);
    INSERT_LC_FUNCTION(func, "acos", "Lisp:acos");
    help =
R"...(### acos ###

<h6> Usage </h6>

* `(acos <Number>)`

<h6> Description </h6>

* Arc cosine. A trigonometric function.
* `<Number>` is cosine.

<h6> Example </h6>

    (display (acos 1))
    
    ;; Output
    ;; > 0)...";
    help_dict_.emplace("acos", help);

    func = LC_FUNCTION_OBJ(ATan);
    INSERT_LC_FUNCTION(func, "atan", "Lisp:atan");
    help =
R"...(### atan ###

<h6> Usage </h6>

* `(atan <Number>)`

<h6> Description </h6>

* Arc tangent. A trigonometric function.
* `<Number>` is tangent.

<h6> Example </h6>

    (display (atan 0))
    
    ;; Output
    ;; > 0)...";
    help_dict_.emplace("atan", help);

    func = LC_FUNCTION_OBJ(Sqrt);
    INSERT_LC_FUNCTION(func, "sqrt", "Lisp:sqrt");
    help =
R"...(### sqrt ###

<h6> Usage </h6>

* `(sqrt <Number>)`

<h6> Description </h6>

* Returns square root of `<Number>`.

<h6> Example </h6>

    (display (sqrt 4))
    
    ;; Output
    ;; > 2)...";
    help_dict_.emplace("sqrt", help);

    func = LC_FUNCTION_OBJ(Abs);
    INSERT_LC_FUNCTION(func, "abs", "Lisp:abs");
    help =
R"...(### abs ###

<h6> Usage </h6>

* `(abs <Number>)`

<h6> Description </h6>

* Returns absolute value of `<Number>`.

<h6> Example </h6>

    (display (abs -111))
    
    ;; Output
    ;; > 111)...";
    help_dict_.emplace("abs", help);

    func = LC_FUNCTION_OBJ(Ceil);
    INSERT_LC_FUNCTION(func, "ceil", "Lisp:ceil");
    help =
R"...(### ceil ###

<h6> Usage </h6>

* `(ceil <Number>)`

<h6> Description </h6>

* Rounds up `<Number>` into integral value.

<h6> Example </h6>

    (display (ceil 1.3))
    
    ;; Output
    ;; > 2)...";
    help_dict_.emplace("ceil", help);

    func = LC_FUNCTION_OBJ(Floor);
    INSERT_LC_FUNCTION(func, "floor", "Lisp:floor");
    help =
R"...(### floor ###

<h6> Usage </h6>

* `(floor <Number>)`

<h6> Description </h6>

* Rounds down `<Number>` into integral value.

<h6> Example </h6>

    (display (floor 1.3))
    
    ;; Output
    ;; > 1)...";
    help_dict_.emplace("floor", help);

    func = LC_FUNCTION_OBJ(Round);
    INSERT_LC_FUNCTION(func, "round", "Lisp:round");
    help =
R"...(### round ###

<h6> Usage </h6>

* `(round <Number>)`

<h6> Description </h6>

* Rounds `<Number>` into the nearest integral value.

<h6> Example </h6>

    (display (round 1.5))
    
    ;; Output
    ;; > 2
    
    (display (round 1.49))
    
    ;; Output
    ;; > 1)...";
    help_dict_.emplace("round", help);

    func = LC_FUNCTION_OBJ(Trunc);
    INSERT_LC_FUNCTION(func, "trunc", "Lisp:trunc");
    help =
R"...(### trunc ###

<h6> Usage </h6>

* `(trunc <Number>)`

<h6> Description </h6>

* Truncates after decimal point of `<Number>`.

<h6> Example </h6>

    (display (trunc 1.234))
    
    ;; Output
    ;; > 1
    
    (display (trunc -1.234))
    
    ;; Output
    ;; > -1)...";
    help_dict_.emplace("trunc", help);

    func = LC_FUNCTION_OBJ(Exp2);
    INSERT_LC_FUNCTION(func, "exp2", "Lisp:exp2");
    help =
R"...(### exp2 ###

<h6> Usage </h6>

* `(exp <Number>)`

<h6> Description </h6>

* Exponent function of `<Number>`. The base is 2.

<h6> Example </h6>

    (display (exp2 3))
    ;; Output
    ;; > 8)...";
    help_dict_.emplace("exp2", help);

    func = LC_FUNCTION_OBJ(Exp);
    INSERT_LC_FUNCTION(func, "exp", "Lisp:exp");
    help =
R"...(### exp ###

<h6> Usage </h6>

* `(exp <Number>)`

<h6> Description </h6>

* Exponent function of `<Number>`. The base is Napier's constant.

<h6> Example </h6>

    (display (exp 1))
    
    ;; Output
    ;; > 2.71828)...";
    help_dict_.emplace("exp", help);

    func = LC_FUNCTION_OBJ(Expt);
    INSERT_LC_FUNCTION(func, "expt", "Lisp:expt");
    help =
R"...(### expt ###

<h6> Usage </h6>

* `(expt <Base> <Exponent>)`
* `(^ <Base> <Exponent>)`

<h6> Description </h6>

* Exponent function of `<Exponent>`. The base is `<Base>`.

<h6> Example </h6>

    (display (expt 2 3))
    
    ;; Output
    ;; > 8)...";
    help_dict_.emplace("expt", help);

    func = LC_FUNCTION_OBJ(Log);
    INSERT_LC_FUNCTION(func, "log", "Lisp:log");
    help =
R"...(### log ###

<h6> Usage </h6>

* `(log <Number>)`
* `(ln <Number>)`

<h6> Description </h6>

* Logarithmic function of `<Number>`. The base is Napier's constant.

<h6> Example </h6>

    (display (log E))
    
    ;; Output
    ;; > 1)...";
    help_dict_.emplace("log", help);

    func = LC_FUNCTION_OBJ(Log2);
    INSERT_LC_FUNCTION(func, "log2", "Lisp:log2");
    help =
R"...(### log2 ###

<h6> Usage </h6>

* `(log2 <Number>)`

<h6> Description </h6>

* Logarithmic function of `<Number>`. The base is 2.

<h6> Example </h6>

    (display (log2 8))
    
    ;; Output
    ;; > 3)...";
    help_dict_.emplace("log2", help);

    func = LC_FUNCTION_OBJ(Log10);
    INSERT_LC_FUNCTION(func, "log10", "Lisp:log10");
    help =
R"...(### log10 ###

<h6> Usage </h6>

* `(log10 <Number>)`

<h6> Description </h6>

* Logarithmic function of `<Number>`. The base is 10.

<h6> Example </h6>

    (display (log10 100))
    
    ;; Output
    ;; > 2)...";
    help_dict_.emplace("log10", help);

    std::shared_ptr<std::mt19937>
    engine_ptr(new std::mt19937(std::chrono::system_clock::to_time_t
    (std::chrono::system_clock::now())));
    func = [this, engine_ptr]
    (LPointer self, LObject* caller, const LObject& args) -> LPointer {
      return this->Random(*engine_ptr, self, caller, args);
    };
    scope_chain_.InsertSymbol("random",
    NewN_Function(func, "Lisp:random", scope_chain_));
    help =
R"...(### random ###

<h6> Usage </h6>

* `(random <Number>)`

<h6> Description </h6>

* Returns random number from 0 to `<Number>`.

<h6> Example </h6>

    (display (random 10))
    
    ;; Output
    ;; > 2.42356
    
    (display (random -10))
    
    ;; Output
    ;; > -7.13453)...";
    help_dict_.emplace("random", help);

    func = LC_FUNCTION_OBJ(Max);
    INSERT_LC_FUNCTION(func, "max", "Lisp:max");
    help =
R"...(### max ###

<h6> Usage </h6>

* `(max <Number>...)`

<h6> Description </h6>

* Returns maximum number of `<Number>...`.

<h6> Example </h6>

    (display (max 1 2 3 4 3 2 1))
    
    ;; Output
    ;; > 4)...";
    help_dict_.emplace("max", help);

    func = LC_FUNCTION_OBJ(Min);
    INSERT_LC_FUNCTION(func, "min", "Lisp:min");
    help =
R"...(### min ###

<h6> Usage </h6>

* `(min <Number>...)`

<h6> Description </h6>

* Returns minimum number of `<Number>...`.

<h6> Example </h6>

    (display (min 4 3 2 1 2 3 4))
    
    ;; Output
    ;; > 1)...";
    help_dict_.emplace("min", help);

    func = LC_FUNCTION_OBJ(RegexSearch);
    INSERT_LC_FUNCTION(func, "regex-search", "Lisp:regex-search");
    help =
R"...(### regex-search ###

<h6> Usage </h6>

* `(regex-search <Regex : String> <Target : String>)`

<h6> Description </h6>

* Search `<Regex>` from `<Target>` and returns List of results.
    + The 1st element of results is match of whole `<Target>`.
    + From the 2nd element, each group.

<h6> Example </h6>

    (define target "Hello World")
    (display (regex-search "(Hel).* (Wor).*" target))
    ;; Output
    ;; > ("Hello World" "Hel" "Wor"))...";
    help_dict_.emplace("regex-search", help);

    func = LC_FUNCTION_OBJ(GenNabla);
    INSERT_LC_FUNCTION(func, "gen-nabla", "Lisp:gen-nabla");
    help =
R"...(### gen-nabla ###

<h6> Usage </h6>

* `(gen-nabla <Mathematical function : Function> <Deltas : Number>...)`

<h6> Description </h6>

* Differentiates `<Mathematical function>` with `<Deltas>...`
  and returns a differentiated function.
    + The differentiated function returns gradient.

<h6> Example </h6>

    ;; Function of 3 variables.
    (define (func x y z) (+ (* 2 x) (* 3 y) (* 4 z)))
    
    ;; Differentiate the function with 0.01 as delta.
    (define nabla-func (gen-nabla func 0.01 0.01 0.01))
    
    ;; Calculate gradient.
    (display (nabla-func 0 0 0))
    ;; Output
    ;; > (2 3 4))...";
    help_dict_.emplace("gen-nabla", help);

    func = LC_FUNCTION_OBJ(Integral);
    INSERT_LC_FUNCTION(func, "integral", "Lisp:integral");
    help =
R"...(### integral ###

<h6> Usage </h6>

* `(integral <Mathematical function : Function> <Range and delta : List>...)`

<h6> Description </h6>

* Integrates `<Mathematical function>` with `<Range and delta>...`.
    + `<Range and delta>...` is List that
      `(<From : Number> <To : Number> <Delta : Number>)`

<h6> Example </h6>

    ;; Integral x^2 + y^2 dxdy (x from 0 to 3) (y from 0 to 5)
    (define (func x y) (+ (* x x) (* y y)))
    (display (integral func '(0 3 0.1) '(0 5 0.1)))
    ;; Output
    ;; > 169.975
    
    ;; (1/3)(x^3)y + (1/3)(y^3)x is a primitive function of x^2 + y^2
    (define (primitive x y) (+ (* (/ 1 3) x x x y) (* (/ 1 3) y y y x)))
    (display (primitive 3 5))
    ;; Output
    ;; > 170)...";
    help_dict_.emplace("integral", help);

    func = LC_FUNCTION_OBJ(PowerMethod);
    INSERT_LC_FUNCTION(func, "power-method", "Lisp:power-method");
    help =
R"...(### power-method ###

<h6> Usage </h6>

* `(power-method <Square matrix>)`

<h6> Description </h6>

* Returns `(<Maximum eigenvalue> <The eigenvector>)` by Power Method.
* `<Square matrix>` is `(<Row vectors>...)`.
* If it failed to find eigenvalue, it returns Nil.

<h6> Example </h6>

    ;; Matrix.
    ;; |  1  2  3 |
    ;; |  0  1 -3 |
    ;; |  0 -3  1 |
    (define matrix '((1 2 3) (0 1 -3) (0 -3 1)))
    
    ;; Calculate maximum eigenvalue and eigenvector.
    (display (power-method matrix))
    ;; Output
    ;; > (1 (1 0 0)))...";
    help_dict_.emplace("power-method", help);

    func = LC_FUNCTION_OBJ(InverseMatrix);
    INSERT_LC_FUNCTION(func, "inverse-matrix", "Lisp:inverse-matrix");
    help =
R"...(### inverse-matrix ###

<h6> Usage </h6>

* `(inverse-matrix <Square matrix>)`

<h6> Description </h6>

* Returns Inverse Matrix of `<Square matrix>`.
* `<Square matrix>` is `(<Row vectors>...)`.
* If it failed to find Inverse Matrix, it returns Nil.

<h6> Example </h6>

    ;; Matrix.
    ;; | 1 0 1 |
    ;; | 1 1 1 |
    ;; | 2 1 1 |
    (define matrix '((1 0 1) (1 1 1) (2 1 1)))
    
    ;; Calculate inverse matrix.
    (define inv-matrix (inverse-matrix matrix))
    (display (ref inv-matrix 0))
    (display (ref inv-matrix 1))
    (display (ref inv-matrix 2))
    ;; Output
    ;; > (0 -1 1)
    ;; > (-1 1 0)
    ;; > (1 1 -1))...";
    help_dict_.emplace("inverse-matrix", help);

    func = LC_FUNCTION_OBJ(TransposedMatrix);
    INSERT_LC_FUNCTION(func, "transposed-matrix", "Lisp:transposed-matrix");
    help =
R"...(### transposed-matrix ###

<h6> Usage </h6>

* `(transposed-matrix <Matrix>)`

<h6> Description </h6>

* Returns Transposed Matrix of `<Matrix>`.
* `<Matrix>` is `(<Row vectors>...)`.

<h6> Example </h6>

    ;; Matrix.
    ;; |  3  0  1  6 |
    ;; |  1  2  2 -1 |
    ;; |  2 -1  5  0 |
    (define matrix '((3 0 1 6) (1 2 2 -1) (2 -1 5 0)))
    (for (elm (transposed-matrix matrix))
         (display elm))
    ;; Output
    ;; > (3 1 2)
    ;; > (0 2 -1)
    ;; > (1 2 5)
    ;; > (6 -1 0))...";
    help_dict_.emplace("transposed-matrix", help);

    func = LC_FUNCTION_OBJ(Determinant);
    INSERT_LC_FUNCTION(func, "determinant", "Lisp:determinant");
    help =
R"...(### determinant ###

<h6> Usage </h6>

* `(determinant <Square matrix>)`

<h6> Description </h6>

* Returns Determinant of `<Square matrix>`.
* `<Square matrix>` is `(<Row vectors>...)`.

<h6> Example </h6>

    ;; Matrix.
    ;; |  3  0  1  6 |
    ;; |  1  2  2 -1 |
    ;; |  2 -1  5  0 |
    ;; |  1  4  1  1 |
    (define matrix '((3 0 1 6) (1 2 2 -1) (2 -1 5 0) (1 4 1 1)))
    (display (determinant matrix))
    ;; Output
    ;; > 67)...";
    help_dict_.emplace("determinant", help);

    func = LC_FUNCTION_OBJ(Bayes);
    INSERT_LC_FUNCTION(func, "bayes", "Lisp:bayes");
    help =
R"...(### bayes ###

<h6> Usage </h6>

* `(bayes <Data list : List> <Event list : List of Funtions> <Condition list : List of Functions)`

<h6> Description </h6>

* Estimates logit of each event of `<Event list>` by Naive Bayes
  and returns its list.
* Each element of `<Event ist>` or `<Conditions list>` must be Predicate.
    + Predicate accepts 1 argument(an element of `<Data list>`)
      and returns Boolean or Number(from 0.0 to 1.0).
        - Predicate can returns Fuzzy.
    + `(bayes)` gives each element of `<Data list>` to each Predicate.
      The Predicate must judge the element and return Boolean or Number.

<h6> Example </h6>

    (define logit-list ())
    
    ;; ---------- First Example ---------- ;;
    ;; List of playing cards.
    (define playing-cards
            '(("Heart" 1) ("Heart" 2) ("Heart" 3) ("Heart" 4) ("Heart" 5)
              ("Heart" 6) ("Heart" 7) ("Heart" 8) ("Heart" 9) ("Heart" 10)
              ("Heart" 11) ("Heart" 12) ("Heart" 13)
              ("Diamond" 1)("Diamond" 2)("Diamond" 3)("Diamond" 4)("Diamond" 5)
              ("Diamond" 6)("Diamond" 7)("Diamond" 8)("Diamond" 9)("Diamond" 10)
              ("Diamond" 11)("Diamond" 12)("Diamond" 13)
              ("Club" 1) ("Club" 2) ("Club" 3) ("Club" 4) ("Club" 5)
              ("Club" 6) ("Club" 7) ("Club" 8) ("Club" 9) ("Club" 10)
              ("Club" 11) ("Club" 12) ("Club" 13)
              ("Spade" 1) ("Spade" 2) ("Spade" 3) ("Spade" 4) ("Spade" 5)
              ("Spade" 6) ("Spade" 7) ("Spade" 8) ("Spade" 9) ("Spade" 10)
              ("Spade" 11) ("Spade" 12) ("Spade" 13)))
    
    ;; Anything is true.
    (define (any-true card) #t)
    
    ;; Judges a card whether the suit is "Heart" or not.
    (define (heart? card) (equal? (car card) "Heart"))
    
    ;; Judges a card whether the suit is black or not.
    (define (black? card)
            (or (equal? (car card) "Club") (equal? (car card) "Spade")))
    
    ;; Judges a card whether the card is a face card or not.
    (define (face? card) (>= (car (cdr card)) 11))
    
    ;; Judges a card whether the number is an even number or not.
    (define (even-num? card) (even? (car (cdr card))))
    
    ;; Judges a card whether the number is an odd number or not.
    (define (odd-num? card) (odd? (car (cdr card))))
    
    ;; Logit of P(Heart)
    ;; Probability is 0.25.
    (set! logit-list (bayes playing-cards
                       (list heart?)
                       (list any-true)))
    (display "Logit : " logit-list)
    (display "Probability : " (map logit->prob logit-list))
    ;; Output
    ;; > Logit : (-1.09766352199352)
    ;; > Probability : (0.25017793594306)
    
    ;; Logit of P(Even | Face) and P(Odd | Face)
    ;; Probability is 0.333... and 0.666...
    (set! logit-list (bayes playing-cards
                            (list even-num? odd-num?)
                            (list face?)))
    (display "Logit : " logit-list)
    (display "Probability : " (map logit->prob logit-list))
    ;; Output
    ;; > Logit : (-0.690840374462031 0.690840374462031)
    ;; > Probability : (0.333846153846154 0.666153846153846)
    
    ;; Logit of P(Heart | Black)
    ;; Probability is 0.0.
    (set! logit-list (bayes playing-cards
                            (list heart?)
                            (list black?)))
    (display "Logit : " logit-list)
    (display "Probability : " (map logit->prob logit-list))
    ;; Output
    ;; > Logit : (-7.24779258176785)
    ;; > Probability : (0.000711237553342816)
    
    ;; ---------- Second Example ---------- ;;
    ;; Quantity of seasoning and taste data list.
    ;; Data : (<Salt> <Sugar> <Wasabi> <Taste : "Spicy" or "Sweet">)
    (define seasoning-taste-data
            '((20 10 3 "Spicy")
              (10 50 2 "Sweet")
              (15 50 20 "Spicy")
              (50 10 5 "Spicy")
              (10 50 2 "Sweet")
              (15 40 3 "Sweet")
              (20 20 30 "Spicy")
              (10 10 40 "Spicy")))
    
    ;; Judge whether "Spicy" or not. (#t or #f)
    (define (spicy? data) (equal? (ref data 3) "Spicy"))
    
    ;; Judge whether "Sweet" or not. (#t or #f)
    (define (sweet? data) (equal? (ref data 3) "Sweet"))
    
    ;; predicate list of taste.
    (define taste-pred-list (list spicy? sweet?))
    
    ;; Judge how much salt. (from 0.0 to 1.0)
    (define (salt? data)
            (/ (ref data 0)
               (+ (ref data 0) (ref data 1) (ref data 2))))
    
    ;; Judge how much sugar. (from 0.0 to 1.0)
    (define (sugar? data)
            (/ (ref data 1)
               (+ (ref data 0) (ref data 1) (ref data 2))))
    
    ;; Judge how much wasabi. (from 0.0 to 1.0)
    (define (wasabi? data)
            (/ (ref data 2)
               (+ (ref data 0) (ref data 1) (ref data 2))))
    
    ;; Probability of taste, when seasoning is salt.
    (set! logit-list
            (bayes seasoning-taste-data taste-pred-list (list salt?)))
    (display "Logit : " logit-list)
    (display "Probability : " (map logit->prob logit-list))
    ;; Output
    ;; > Logit : (1.12780558906831 -1.12780558906831)
    ;; > Probability : (0.755433701008636 0.244566298991364)
    
    ;; Probability of taste, when seasoning is sugar.
    (set! logit-list
            (bayes seasoning-taste-data taste-pred-list (list sugar?)))
    (display "Logit : " logit-list)
    (display "Probability : " (map logit->prob logit-list))
    ;; Output
    ;; > Logit : (-0.408098829106817 0.408098829106817)
    ;; > Probability : (0.399368073757344 0.600631926242656)
    
    ;; Probability of taste, when seasoning is wasabi.
    (set! logit-list
            (bayes seasoning-taste-data taste-pred-list (list wasabi?)))
    (display "Logit : " logit-list)
    (display "Probability : " (map logit->prob logit-list))
    ;; Output
    ;; > Logit : (2.00034598677312 -2.00034598677312)
    ;; > Probability : (0.880833399583934 0.119166600416066)
    
    ;; Probability of taste, when seasoning is salt and sugar.
    (set! logit-list
            (bayes seasoning-taste-data taste-pred-list (list salt? sugar?)))
    (display "Logit : " logit-list)
    (display "Probability : " (map logit->prob logit-list))
    ;; Output
    ;; > Logit : (0.267721636218435 -0.267721636218435)
    ;; > Probability : (0.566533484706568 0.433466515293432)
    
    ;; Probability of taste, when seasoning is salt and wasabi.
    (set! logit-list
            (bayes seasoning-taste-data taste-pred-list (list salt? wasabi?)))
    (display "Logit : " logit-list)
    (display "Probability : " (map logit->prob logit-list))
    ;; Output
    ;; > Logit : (2.67616645209838 -2.67616645209838)
    ;; > Probability : (0.935605546063172 0.064394453936828)
    
    ;; Probability of taste, when seasoning is sugar and wasabi.
    (set! logit-list
            (bayes seasoning-taste-data taste-pred-list (list sugar? wasabi?)))
    (display "Logit : " logit-list)
    (display "Probability : " (map logit->prob logit-list))
    ;; Output
    ;; > Logit : (1.14026203392325 -1.14026203392325)
    ;; > Probability : (0.757727745498944 0.242272254501056))...";
    help_dict_.emplace("bayes", help);

    func = LC_FUNCTION_OBJ(LogitToProb);
    INSERT_LC_FUNCTION(func, "logit->prob", "Lisp:logit->prob");
    help =
R"...(### logit->prob ###

<h6> Usage </h6>

* `(logit->prob <Logit : Number>)`

<h6> Description </h6>

* Converts logit to probability and returns it.

<h6> Example </h6>

    (display (logit->prob 0))
    ;; Output
    ;; > 0.5)...";
    help_dict_.emplace("logit->prob", help);

    func = LC_FUNCTION_OBJ(ProbToLogit);
    INSERT_LC_FUNCTION(func, "prob->logit", "Lisp:prob->logit");
    help =
R"...(### prob->logit ###

<h6> Usage </h6>

* `(prob->logit <Probability : Number>)`

<h6> Description </h6>

* Converts probability to logit and returns it.
* `<Probability>` must be greater than 0.0 and less than 1.0.
    + 0.0 or 1.0 occurs error.

<h6> Example </h6>

    (display (prob->logit 0.5))
    ;; Output
    ;; > 0)...";
    help_dict_.emplace("prob->logit", help);

    func = LC_FUNCTION_OBJ(GenPA2);
    INSERT_LC_FUNCTION(func, "gen-pa2", "Lisp:gen-pa2");
    help =
R"...(### gen-pa2 ###

<h6> Usage </h6>

* `(gen-pa2 <Initial weights : List>)`
* `(<PA-2> <Message symbol : Symbol> [<Arguments>...])`

<h6> Description </h6>

* Generates and returns `<PA-2>` object.
* "PA-2" is algorithm of online machine learning.
    + It can train weights of linear function.
    + "Online" means that it trains from data one by one.
* It accepts message symbol.
    + `@train <plus? : Boolean> <Cost : Number> <Input : List>`
        - Trains weights and returns weights after training.
        - `<Plus?>` is that
          if output of linear function should be 0 or positive number,
          then #t, otherwise #f.
        - `<Cost>` is penalty of error.
          It must be greater than 0. (not just 0)
        - `<Input>` is input vector of linear function.
    + `@calc <Input : List>`
        - Calculates linear function.
        - `<Input>` is input vector of linear function.
    + `@get-weights`
        - Returns the current weights.

<h6> Example </h6>

    ;; Leaning whether seasoning is sweet or not.
    ;; Data List (<Sweet?> <Sugar> <Salt>)
    (define data-list
            '((#t 50 20)
              (#f 10 60)
              (#t 120 40)
              (#f 20 80)
              (#t 30 10)
              (#f 50 100)))
    
    ;; Statistics values.
    (define num-data 0.1)
    (define total-1 0)
    (define total-2 0)
    (define sq-total-1 0)
    (define sq-total-2 0)
    
    ;; Generate PA-2.
    (define weights '(0 0))
    (define cost 0.5)
    (define pa2 (gen-pa2 weights))
    
    ;; Start Online Training.
    ;; Update means, standard deviations and weights per one cycle.
    (define sugar 0)
    (define salt 0)
    (define mean-1 0)
    (define mean-2 0)
    (define sdev-1 0)
    (define sdev-2 0)
    (define normalized-1 0)
    (define normalized-2 0)
    (for (data data-list)
         (inc! num-data)
    
         (set! sugar (ref data 1))
         (set! salt (ref data 2))
    
         ;; Update means.
         (add! total-1 sugar)
         (add! total-2 salt)
         (set! mean-1 (/ total-1 num-data))
         (set! mean-2 (/ total-2 num-data))
    
         ;; Update standard deviations.
         (add! sq-total-1 (expt (- sugar mean-1) 2))
         (add! sq-total-2 (expt (- salt mean-2) 2))
         (set! sdev-1 (sqrt (/ sq-total-1 num-data)))
         (set! sdev-2 (sqrt (/ sq-total-2 num-data)))
    
         ;; Normalize sugar and salt.
         (set! normalized-1 (/ (- sugar mean-1) sdev-1))
         (set! normalized-2 (/ (- salt mean-2) sdev-2))
    
         ;; Train weights.
         (display (pa2 '@train
                       (car data) cost
                       (list normalized-1 normalized-2))))
    ;; Output
    ;; > (0.327752765053172 0.327752765053172)
    ;; > (0.608882281818588 0.0393170710326415)
    ;; > (0.608882281818588 0.0393170710326415)
    ;; > (0.714030213008641 -0.172427992527486)
    ;; > (0.55686689768203 -0.649661489515829)
    ;; > (0.55686689768203 -0.649661489515829)
    
    ;; Test Weights.
    (define (sweet? val) (if (>= val 0) "Yes!" "No..."))
    (define test-1 '(20 5))
    (define test-2 '(80 300))
    
    ;; Calculate test-1.
    (set! normalized-1 (/ (- (ref test-1 0) mean-1) sdev-1))
    (set! normalized-2 (/ (- (ref test-1 1) mean-2) sdev-2))
    (display test-1 " is sweet? => "
             (sweet? (pa2 '@calc (list normalized-1 normalized-2))))
    ;; Output
    ;; > (20 5) is sweet? => Yes!
    
    ;; Calculate test-2.
    (set! normalized-1 (/ (- (ref test-2 0) mean-1) sdev-1))
    (set! normalized-2 (/ (- (ref test-2 1) mean-2) sdev-2))
    (display test-2 " is sweet? => "
             (sweet? (pa2 '@calc (list normalized-1 normalized-2))))
    ;; Output
    ;; > (80 300) is sweet? => No...)...";
    help_dict_.emplace("gen-pa2", help);

    func = LC_FUNCTION_OBJ(GenAI);
    INSERT_LC_FUNCTION(func, "gen-ai", "Lisp:gen-ai");
    help =
R"...(### gen-ai ###

<h6> Usage </h6>

* `(gen-ai <Initial weights : List> <Initial bias : Number)`
* `(<AI> <Message symbol : Symbol> [<Arguments>...])`

<h6> Description </h6>

* Generates Artificial Intelligence `<AI>`.
    + `<Initial weights>` is List of initial weights
      that size is the number of feature vector.
    + `<Initial bias>` is initial bias.
* Difference between this and `(gen-pa2)`.
    + This has bias.
    + Supports Perceptron PA-1 PA-2 and Neural Network.
    + Feature vector can contain Boolean.
* `<AI>`'s message symbol.
    + `@get-weight`
        - Returns current weights.
    + `@get-bias`
        - Returns current bias.
    + `@calc<Feature vector : List>`
        - Calculates and returns output that is from -1 to 1.
    + `@prob <Feature vector : List>`
        - Returns probability of true.
    + `@logit <Feature vector : List>`
        - Returns Logit of probability of true.
    + `@judge <Feature vector : List>`
        - Returns #t if Logit is positive number, otherwise returns #f.
    + `@train <Learning rate : Number> <Desired output : Boolean> <Feature vector : List>`
        - Trains `<AI>` by Perceptron.
            - This can also be used as output node of Neural Network.
            - In comparison with PA-1 or PA-2,
              this learning speed is very slow.
        - Returns differentiated loss.
        - `<Rate>` determine maximum amplitude of adjusted value of weight.
            - Maximum amplitude is `+- <Rate> * feature`.
    + `@train-pa1 <Cost : Number> <Desired output : Boolean> <Feature vector : List>`
        - Trains `<AI>` by PA-1.
        - Returns differentiated loss.
        - `<Cost>` must be positive number and not 0.
        - If `<Cost>` is infinite then same as Hard Margin,
          otherwise Soft Margin.
    + `@train-pa2 <Cost : Number> <Desired output : Boolean> <Feature vector : List>`
        - Trains `<AI>` by PA-2.
        - Returns differentiated loss.
        - `<Cost>` must be positive number and not 0.
        - If `<Cost>` is infinite then same as Hard Margin,
          otherwise Soft Margin.
    + `@train-bp <Rate : Number> <Each parent's Differentiated loss : List> <Each parent's weight related to me : List > <Children's output : List>`
        - Trains as a node of middle layer of Neural Network
          by Back Propagation.
        - Returns differentiated loss.
        - `<Rate>` must be positive number and not 0.
        - `<Each parent's Differentiated loss>` is List of
          each parent's differentiated loss
          that is returned by parent's `@train`.
        - `<Each parent's weight related to me>` is
          List of weights related to itself.
            - If the `<AI>` is 3rd node in the layer,
              `<Each parent's weight related to me>` is
              List of the 3rd weight of each parent's weights.

<h6> Example </h6>

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; Leaning whether seasoning is sweet or not. ;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; Data List (<Sweet?> <Sugar> <Salt>)
    (define data-list
            '((#t 50 20)
              (#f 10 60)
              (#t 120 40)
              (#f 20 80)
              (#t 30 10)
              (#f 50 100)))
    
    (define rate 0.07)
    (define ai (gen-ai '(0 0) 0))
    (for (data data-list)
         (ai '@train-pa1 rate (car data) (cdr data)))
    
    ;; Print results.
    (define features-list '((90 10)
                            (10 90)
                            (60 40)
                            (40 60)
                            (50 50)))
    (for (features features-list)
         (display "Input : " features)
         (display "    Judge : " (ai '@judge features))
         (display "    Calc : " (ai '@calc features))
         (display "    Logit : " (ai '@logit features))
         (display "    Prob : " (ai '@prob features)))
    ;; Output
    ;; > Input : (90 10)
    ;; >     Judge : #t
    ;; >     Calc : 0.839548040409108
    ;; >     Logit : 2.43928060964351
    ;; >     Prob : 0.919774020204554
    ;; > Input : (10 90)
    ;; >     Judge : #f
    ;; >     Calc : -0.749873839397153
    ;; >     Logit : -1.94533353956129
    ;; >     Prob : 0.125063080301423
    ;; > Input : (60 40)
    ;; >     Judge : #t
    ;; >     Calc : 0.377829397451558
    ;; >     Logit : 0.795050303691708
    ;; >     Prob : 0.688914698725779
    ;; > Input : (40 60)
    ;; >     Judge : #f
    ;; >     Calc : -0.149424378524727
    ;; >     Logit : -0.301103233609493
    ;; >     Prob : 0.425287810737637
    ;; > Input : (50 50)
    ;; >     Judge : #t
    ;; >     Calc : 0.122862890154041
    ;; >     Logit : 0.246973535041108
    ;; >     Prob : 0.56143144507702
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; Neural Network. Learning XOR. ;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; AIs.
    ;; Input Middle Output
    ;;    x1     a1     y1
    ;;    x2     a2
    (define (random-weight)
            (if (>= (random 1) 0.5) (random 1) (* -1 (random 1))))
    (define a1 (gen-ai (list (random-weight) (random-weight))
                       (random-weight)))
    (define a2 (gen-ai (list (random-weight) (random-weight))
                       (random-weight)))
    
    (define y1 (gen-ai (list (random-weight) (random-weight))
                       (random-weight)))
    
    ;; Calculation.
    (define (calc input)
            (y1 '@calc (list (a1 '@calc input) (a2 '@calc input))))
    (define (cal-logit input)
            (y1 '@logit (list (a1 '@calc input) (a2 '@calc input))))
    (define (prob input)
            (y1 '@prob (list (a1 '@calc input) (a2 '@calc input))))
    (define (judge input)
            (y1 '@judge (list (a1 '@calc input) (a2 '@calc input))))
    
    ;; Training function.
    (define learning-rate 0.25)
    (define (train output input)
            ;; Jot down y weights before training
            (define y-weights
                    (transposed-matrix (list (y1 '@get-weights))))
    
            ;; Jot down a outputs before training
            (define a-outputs
                    (list (a1 '@calc input)
                          (a2 '@calc input)))
    
            ;; Train y1 and get its differentiated loss.
            (define y-loss
                    (list (y1 '@train learning-rate output a-outputs)))
    
            ;; Train a1 a2 with Back-Propagation.
            (a1 '@train-bp learning-rate
                y-loss (ref y-weights 0) input)
            (a2 '@train-bp learning-rate
                y-loss (ref y-weights 1) input)
    
            ;; Returns stuff.
            #t)
    
    ;; XOR generator.
    (define (random-xor)
            (define x1 (if (>= (random 1) 0.5) 1 -1))
            (define x2 (if (>= (random 1) 0.5) 1 -1))
            (list (if (= x1 x2) #f #t) x1 x2))
    
    ;; Train 1000 times.
    (define xor ())
    (for (i (range 1000))
         (set! xor (random-xor))
         (train (car xor) (cdr xor)))
    
    ;; Judge.
    (define logic-data-list
            '((1 1)
              (-1 1)
              (1 -1)
              (-1 -1)))
    (for (data logic-data-list)
         (display "Data : " data)
         (display "    Judge : " (judge data))
         (display "    Calc : " (calc data))
         (display "    Logit : " (cal-logit data))
         (display "    Prob : " (prob data)))
    ;; Output
    ;; > Data : (1 1)
    ;; >     Judge : #f
    ;; >     Calc : -0.901210496751019
    ;; >     Logit : -2.95725470915291
    ;; >     Prob : 0.0493947516244905
    ;; > Data : (-1 1)
    ;; >     Judge : #t
    ;; >     Calc : 0.859721779977142
    ;; >     Logit : 2.58455443841497
    ;; >     Prob : 0.929860889988571
    ;; > Data : (1 -1)
    ;; >     Judge : #t
    ;; >     Calc : 0.879465729452377
    ;; >     Logit : 2.74680871359316
    ;; >     Prob : 0.939732864726189
    ;; > Data : (-1 -1)
    ;; >     Judge : #f
    ;; >     Calc : -0.902502383508753
    ;; >     Logit : -2.97109741110352
    ;; >     Prob : 0.0487488082456237
    )...";
    help_dict_.emplace("gen-ai", help);

    func = LC_FUNCTION_OBJ(RBFKernel);
    INSERT_LC_FUNCTION(func, "rbf-kernel", "Lisp:rbf-kernel");
    help =
R"...(### rbf-kernel ###

<h6> Usage </h6>

* `(rbf-kernel <Vector 1 : List> <Vector 2 : List> <Bandwidth : Number>)`

<h6> Description </h6>

* Calculates Radial Bases Function Kernel.
* `<Vector 1>` and `<Vector 1>` is List of Numbers.
* `<Bandwidth>` is bandwidth of this Kernel.

<h6> Example </h6>

    (define vec-1 '(10 20 30))
    (define vec-2 '(10 20 30))
    (define vec-3 '(20 30 40))
    
    (display (rbf-kernel vec-1 vec-2 10))
    (display (rbf-kernel vec-1 vec-3 10))
    (display (rbf-kernel vec-1 vec-3 20))
    ;; Output
    ;; > 1
    ;; > 0.22313016014843
    ;; > 0.687289278790972)...";
    help_dict_.emplace("rbf-kernel", help);

    func = LC_FUNCTION_OBJ(Now);
    INSERT_LC_FUNCTION(func, "now", "Lisp:now");
    help =
R"...(### now ###

<h6> Usage </h6>

* `(now)`

<h6> Description </h6>

* Returns List of current time.
    + List is `(<Year> <Month> <Day> <Hour> <Minute> <Second>)`.

<h6> Example </h6>

    ;; Run at 2016-6-11 15:26:59.
    (display (now))
    ;; Output
    ;; > (2016 6 11 15 26 59))...";
    help_dict_.emplace("now", help);

    func = LC_FUNCTION_OBJ(Clock);
    INSERT_LC_FUNCTION(func, "clock", "Lisp:clock");
    help =
R"...(### clock ###

<h6> Usage </h6>

* `(clock)`

<h6> Description </h6>

* Returns execution time (seconds).

<h6> Example </h6>

    ;; After 1.29 seconds from program has run.
    (clock)
    ;; Output
    ;; 1.29091)...";
    help_dict_.emplace("clock", help);
  }

  // ============== //
  // ネイティブ関数 //
  // ============== //
  namespace {
    // Define()とLambda()で使う関数オブジェクトを作成する部品。
    inline LPointer GenFunction(const LPointer& names_ptr,
    const LPointer& expr_list_ptr, const LScopeChain& scope_chain) {
      // names_ptrがリストかどうかチェックする。
      Lisp::CheckList(*names_ptr);

      // 引数の名前リストを作成。
      LArgNames arg_names(Lisp::CountList(*names_ptr));
      if (arg_names.size()) {
        LArgNames::iterator itr = arg_names.begin();
        for (LObject* ptr = names_ptr.get(); ptr->IsPair();
        Lisp::Next(&ptr), ++itr) {
          const LPointer& elm_ptr = ptr->car();
          Lisp::CheckType(*elm_ptr, LType::SYMBOL);
          *itr = elm_ptr->symbol();
        }
      }

      // 関数定義を作成。
      LPointerVec expression = Lisp::ListToLPointerVec(*expr_list_ptr);

      // 関数オブジェクトにして返す。
      return Lisp::NewFunction(arg_names, expression, scope_chain);
    }
  }

  // リストをジップする。
  LPointer Lisp::ZipLists(LPointerVec& list_vec) {
    LPointerVec ret_vec;

    LPointerVec temp;
    while (true) {
      temp.clear();

      bool found = false;  // ペア発見フラグ。
      for (auto& list_ptr : list_vec) {
        if (list_ptr->IsPair()) {
          temp.push_back(list_ptr->car());
          list_ptr = list_ptr->cdr();

          found = true;  // ペアを発見したのでマーク。
        }
      }
      if (!found) break;  // ペアが一つもなかったら抜ける。

      ret_vec.push_back(LPointerVecToList(temp));
    }

    return LPointerVecToList(ret_vec);
  }

  // %%% help
  DEF_LC_FUNCTION(Lisp::Help) {
    int length = CountList(args) - 1;
    std::ostringstream oss;
    if (length > 0) {
      // キーを得る。
      LPointer result = caller->Evaluate(*(args.cdr()->car()));
      CheckType(*result, LType::STRING);
      const std::string& help = result->string();

      if (help_dict_.find(help) != help_dict_.end()) {
        oss << help_dict_.at(help);
      }
    } else {
      // 全表示。
      for (auto& pair : help_dict_) {
        oss << pair.second << std::endl;
        oss << std::string(79, '-') << std::endl;
      }
    }

    return NewString(oss.str());
  }

  // %%% parse
  DEF_LC_FUNCTION(Lisp::ParseFunc) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // タイプをチェック。
    LPointer result = caller->Evaluate(*(args_ptr->car()));
    CheckType(*result, LType::STRING);

    // パーサを準備。
    LParser parser;

    // 字句解析する。
    parser.Tokenize(result->string());

    // 解析できたかチェック。
    if ((parser.parenth_counter() != 0) || parser.in_string()) {
      // エラー。
      throw GenError("@parse-error",
      "Couldn't parse '" + args_ptr->car()->ToString() + "'.");
    }

    // パースする。
    LPointerVec parse_result = parser.Parse();
    if (parse_result.size() <= 0) {
      // エラー。
      throw GenError("@parse-error",
      "Couldn't parse '" + args_ptr->car()->ToString() + "'.");
    }

    // 一番最後にパースした式を返す。
    return parse_result.at(parse_result.size() - 1);
  }

  // %%% backquote
  DEF_LC_FUNCTION(Lisp::Backquote) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::function<void(LPointer)> core;
    core = [&core, &caller](LPointer pair) {
      LObject* ptr = pair.get();

      // ptr->cdr------------------>cddr
      // |    |
      // car  cadr------->cdadr
      //      |           |
      //      caadr       cadadr
      //      (unquote)   (object)
      //
      // リストの終端cdrにunqoteがあった場合。
      // ptr->cdr--------->cddr----->Nil
      // |    |            |
      // car  cadr         caddr
      //      (unquote)    (objct)
      LPointer result, temp;

      for (; ptr->IsPair(); Next(&ptr)) {
        const LPointer& car = ptr->car();
        const LPointer& cdr = ptr->cdr();

        // carの処理。
        if (car->IsPair()) {
          temp = NewPair(NewNil(), car);
          core(temp);
          ptr->car(temp->cdr());
        }

        // cdrの処理。
        if (cdr->IsPair()) {
          const LPointer& cadr = cdr->car();
          const LPointer& cddr = cdr->cdr();
          if (cadr->IsPair()) {
            const LPointer& caadr = cadr->car();
            const LPointer& cdadr = cadr->cdr();
            if (caadr->IsSymbol() && cdadr->IsPair()) {
              const LPointer& cadadr = cdadr->car();
              if (caadr->symbol() == "unquote") {
                // コンマ。
                // 評価する。
                cdadr->car(caller->Evaluate(*cadadr));

                // つなぎ替える。
                cdadr->cdr(cddr);
                ptr->cdr(cdadr);
              } else if (caadr->symbol() == "unquote-splicing") {
                // コンマアット。
                // 評価する。
                result = caller->Evaluate(*cadadr);

                if (result->IsPair()) {
                  // ペア。
                  // つなぎ替える。
                  LObject* result_ptr = result.get();
                  for (; result_ptr->cdr()->IsPair(); Next(&result_ptr)) {
                    continue;
                  }
                  result_ptr->cdr(cddr);

                  ptr->cdr(result);
                } else if (result->IsNil()) {
                  // Nil。 なかったことにする。
                  ptr->cdr(cddr);
                } else {
                  // リストじゃなかったので、コンマの動作。
                  cdadr->car(result);
                  cdadr->cdr(cddr);
                  ptr->cdr(cdadr);
                }
              }
            }
          } else if ((cadr->IsSymbol()) && (cddr->IsPair())) {
            // リストの終端cdrにunquoteがあった場合。 両方同じ処理。
            const LPointer& addar = cddr->car();
            if ((cadr->symbol() == "unquote")
            || (cadr->symbol() == "unquote-splicing")) {
              ptr->cdr(caller->Evaluate(*addar));
            }
          }
        }
      }
    };

    // 頭にダミーをつけて実行。
    LPointer temp = NewPair(NewNil(), args_ptr->car()->Clone());
    core(temp);

    return temp->cdr();
  }

  // %%% define
  DEF_LC_FUNCTION(Lisp::Define) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数。
    const LPointer& first_arg = args_ptr->car();

    // シンボルならバインド、ペアなら関数定義。
    if (first_arg->IsSymbol()) {
      // シンボルへのバインド。
      // (define <first_args> <args_ptr->cdr>...)

      // 第2引数以降を評価する。
      LPointer result(nullptr);
      for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
        result = caller->Evaluate(*(args_ptr->car()));
      }

      // 評価結果をスコープにバインド。
      caller->scope_chain().InsertSymbol(first_arg->symbol(), result);

      return first_arg->Clone();
    } else if (first_arg->IsPair()) {
      // 関数定義。
      // (define (<func_name_ptr> <first_args->cdr>...) <args_ptr->cdr>...)
      // 関数名を得る。
      const LPointer& func_name_ptr = first_arg->car();

      // スコープチェーンを得る。
      const LScopeChain& chain = caller->scope_chain();

      // スコープチェーンにバインド。
      chain.InsertSymbol(func_name_ptr->symbol(),
      GenFunction(first_arg->cdr(), args_ptr->cdr(), chain));

      return func_name_ptr->Clone();
    }

    throw GenTypeError(*first_arg, "Symbol or Pair");
  }

  // %%% define-macro
  DEF_LC_FUNCTION(Lisp::DefineMacro) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数はインターフェイス。
    const LPointer& inter_face_ptr = args_ptr->car();
    CheckType(*inter_face_ptr, LType::PAIR);
    Next(&args_ptr);

    // マクロの名。
    const LPointer& macro_name_ptr = inter_face_ptr->car();
    CheckType(*macro_name_ptr, LType::SYMBOL);
    const std::string& macro_name = macro_name_ptr->symbol();

    // マクロの引数名。
    LPointer args_name_ptr = inter_face_ptr->cdr()->Clone();
    for (LObject* ptr = args_name_ptr.get(); ptr->IsPair(); Next(&ptr)) {
      CheckType(*(ptr->car()), LType::SYMBOL);
    }

    // 第2引数は定義式。
    LPointer expression_ptr = args_ptr->Clone();

    // ローカルスコープの基礎を作る。
    LScopeChain base_chain = caller->scope_chain();

    // 関数オブジェクトを作成。
    LC_Function func =
    [macro_name_ptr, args_name_ptr, expression_ptr, base_chain]
    (LPointer self, LObject* caller, const LObject& args) -> LPointer {
      // ローカルスコープを作成。
      LScopeChain local_chain = base_chain;
      local_chain.AppendNewScope();

      // ローカルスコープに引数を評価せずにバインドしていく。
      LPointer args_ptr = args.cdr();
      LObject* names_ptr = args_name_ptr.get();
      for (; (names_ptr->IsPair()) && (args_ptr->IsPair());
      Next(&names_ptr), args_ptr = args_ptr->cdr()) {
        const std::string& name = names_ptr->car()->symbol();
        if (name[0] == '&') {
          local_chain.InsertSymbol(name, args_ptr->Clone());
          break;
        } else {
          local_chain.InsertSymbol(name, args_ptr->car()->Clone());
        }
      }

      // まだ名前が余っていたらNilをバインド。
      for (; names_ptr->IsPair(); Next(&names_ptr)) {
        local_chain.InsertSymbol(names_ptr->car()->symbol(), NewNil());
      }

      // 自身にローカルスコープをセットして各式を評価して式を得る。
      self->scope_chain(local_chain);
      LPointer result_expr = NewNil();
      for (LObject* expr = expression_ptr.get(); expr->IsPair(); Next(&expr)) {
        result_expr = self->Evaluate(*(expr->car()));
      }

      // 出来上がった式を評価して返す。
      return caller->Evaluate(*result_expr);
    };

    LPointer n_function = NewN_Function(func, "Lisp:define-macro;"
    + std::to_string(reinterpret_cast<std::size_t>(expression_ptr.get())),
    caller->scope_chain());

    // スコープにバインド。
    caller->scope_chain().InsertSymbol(macro_name, n_function);

    return macro_name_ptr->Clone();
  }

  // %%% lambda
  DEF_LC_FUNCTION(Lisp::Lambda) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 関数オブジェクトにして返す。
    return GenFunction(args_ptr->car(), args_ptr->cdr(),
    caller->scope_chain());
  }

  // %%% func->Lambda
  DEF_LC_FUNCTION(Lisp::FuncToLambda) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 関数オブジェクトを得る。
    LPointer func_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*func_ptr, LType::FUNCTION);

    // 引数名リストを得る。
    const LArgNames& arg_names = func_ptr->arg_names();
    LPointer arg_names_list = NewList(arg_names.size());
    LObject* arg_names_list_ptr = arg_names_list.get();
    for (auto& name : arg_names) {
      arg_names_list_ptr->car(NewSymbol(name));
      Next(&arg_names_list_ptr);
    }

    return NewPair(NewSymbol("lambda"), NewPair(arg_names_list,
    LPointerVecToList(func_ptr->expression())));
  }

  // %%% let
  DEF_LC_FUNCTION(Lisp::Let) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // ローカル変数用スコープチェーンを作って自身にセット。。
    LScopeChain local_chain = caller->scope_chain();
    local_chain.AppendNewScope();
    self->scope_chain(local_chain);

    // ローカル変数をローカルチェーンにバインドしていく。
    const LPointer& first_ptr = args_ptr->car();
    CheckList(*first_ptr);
    LPointer local_pair;
    for (LObject* ptr = first_ptr.get(); ptr->IsPair(); Next(&ptr)) {
      local_pair = ptr->car();
      CheckList(*local_pair);
      if (CountList(*local_pair) < 2) {
        throw GenError("@function-error",
        "'" + local_pair->ToString() + "' doesn't have 2 elements and more.");
      }

      // ローカル変数名。
      const LPointer& local_pair_first = local_pair->car();
      CheckType(*local_pair_first, LType::SYMBOL);

      // ローカル変数の初期値。
      LPointer local_pair_second =
      caller->Evaluate(*(local_pair->cdr()->car()));

      // バインドする。
      local_chain.InsertSymbol(local_pair_first->symbol(), local_pair_second);
    }

    // ローカル変数用スコープチェーンを使って各式を評価。
    LPointer ret_ptr = NewNil();
    for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
      ret_ptr = self->Evaluate(*(args_ptr->car()));
    }

    return ret_ptr;
  }

  // %%% while
  DEF_LC_FUNCTION(Lisp::While) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // ローカル変数用スコープチェーンを作って自身にセット。。
    LScopeChain local_chain = caller->scope_chain();
    local_chain.AppendNewScope();
    self->scope_chain(local_chain);

    // ループする。
    LPointer ret_ptr = NewNil();
    const LPointer& condition_expr = args_ptr->car();
    Next(&args_ptr);
    while (true) {
      // 条件を評価。 falseなら抜ける。
      LPointer condition = self->Evaluate(*condition_expr);
      CheckType(*condition, LType::BOOLEAN);
      if (!(condition->boolean())) break;

      // 各式を実行。
      for (LObject* ptr = args_ptr; ptr->IsPair(); Next(&ptr)) {
        ret_ptr = self->Evaluate(*(ptr->car()));
      }
    }

    return ret_ptr;
  }

  // %%% for
  DEF_LC_FUNCTION(Lisp::For) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // ローカル変数用スコープチェーンを作って自身にセット。。
    LScopeChain local_chain = caller->scope_chain();
    local_chain.AppendNewScope();
    self->scope_chain(local_chain);

    // ループの範囲の準備をする。
    const LPointer& range_expr = args_ptr->car();
    CheckList(*range_expr);
    if (CountList(*range_expr) < 2) {
      throw GenError("@function-error",
      "'" + range_expr->ToString() + "' doesn't have 2 elements and more.");
    }
    const LPointer& item = range_expr->car();
    LPointer range = caller->Evaluate(*(range_expr->cdr()->car()));
    CheckType(*item, LType::SYMBOL);
    const std::string& item_symbol = item->symbol();

    // リストか文字列の次の要素を取り出す関数と終了判定関数。。
    std::function<LPointer()> get_next_elm;
    std::function<bool()> is_not_end;
    LObject* range_ptr = range.get();
    const std::string& str = range->string();
    unsigned int str_i = 0;
    std::size_t str_size = str.size();
    if (range->IsList()) {
      // リスト用。
      get_next_elm = [&range_ptr]() -> LPointer {
        const LPointer& ret = range_ptr->car();
        Next(&range_ptr);
        return ret;
      };

      is_not_end = [&range_ptr]() -> bool {
        return range_ptr->IsPair();
      };
    } else if (range->IsString()) {
      // 文字列用。
      get_next_elm = [&str, &str_i]() -> LPointer {
        char c = str[str_i++];
        return NewString(std::string(1, c));
      };

      is_not_end = [&str_i, str_size]() -> bool {
        return str_i < str_size;
      };
    } else {
      throw GenTypeError(*range, "List or String");
    }

    // ローカルチェーンにシンボルをバインドする。
    local_chain.InsertSymbol(item_symbol, NewNil());

    // ループする。
    LPointer ret_ptr = NewNil();
    Next(&args_ptr);
    while (is_not_end()) {
      // ローカルチェーンに範囲の要素をバインド。
      local_chain.UpdateSymbol(item_symbol, get_next_elm());

      // 各式を実行。
      for (LObject* ptr_2 = args_ptr; ptr_2->IsPair(); Next(&ptr_2)) {
        ret_ptr = self->Evaluate(*(ptr_2->car()));
      }
    }

    return ret_ptr;
  }

  // %%% cond
  DEF_LC_FUNCTION(Lisp::Cond) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    for (; args_ptr->IsPair(); Next(&args_ptr)) {
      // 条件文リストを得る。
      const LPointer& sentence = args_ptr->car();
      CheckList(*sentence);
      if (CountList(*sentence) < 2) {
        throw GenError("@function-error",
        "'" + sentence->ToString() + "' doesn't have 2 elements and more.");
      }

      // 1要素目がelseかどうか。
      LPointer result;
      const LPointer& sentence_car = sentence->car();
      if (sentence_car->symbol() == "else") {
        result = NewBoolean(true);
      } else {
        result = caller->Evaluate(*sentence_car);
        CheckType(*result, LType::BOOLEAN);
      }

      // resultがtrueなら2要素目を実行して終わる。
      if (result->boolean()) {
        LPointer ret_ptr = NewNil();
        for (LObject* ptr = sentence->cdr().get(); ptr->IsPair();
        Next(&ptr)) {
          ret_ptr = caller->Evaluate(*(ptr->car()));
        }

        return ret_ptr;
      }
    }

    return NewNil();
  }

  // %%% gen-scope
  DEF_LC_FUNCTION(Lisp::GenScope) {
    // スコープを作成。
    LScopePtr my_scope_ptr(new LScope());

    // 関数オブジェクトを作成。
    auto func =
    [my_scope_ptr](LPointer self, LObject* caller, const LObject& args)
    -> LPointer {
      // スコープをセット。
      LScopeChain chain = caller->scope_chain();
      chain.push_back(my_scope_ptr);
      self->scope_chain(chain);

      // 式を評価していく。
      LPointer ret_ptr = NewNil();
      for (LObject* ptr = args.cdr().get(); ptr->IsPair(); Next(&ptr)) {
        ret_ptr = self->Evaluate(*(ptr->car()));
      }

      return ret_ptr;
    };

    return NewN_Function(func, "Lisp::gen-scope:"
    + std::to_string(reinterpret_cast<std::size_t>(my_scope_ptr.get())),
    caller->scope_chain());
  }

  // %%% try
  DEF_LC_FUNCTION(Lisp::Try) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第位1引数はエラー検出する式のリスト。
    const LPointer& first_ptr = args_ptr->car();
    CheckList(*first_ptr);

    LPointer ret_ptr = NewNil();

    // try{}で実行していく。
    try {
      for (LObject* ptr = first_ptr.get(); ptr->IsPair(); Next(&ptr)) {
        ret_ptr = caller->Evaluate(*(ptr->car()));
      }
    } catch (LPointer error) {
      // エラー発生。
      // 自分のローカルスコープにエラーをバインドして
      // 第2引数以降を実行していく。
      LScopeChain chain = caller->scope_chain();
      chain.AppendNewScope();
      self->scope_chain(chain);
      self->scope_chain().InsertSymbol("exception", error);
      for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
        ret_ptr = self->Evaluate(*(args_ptr->car()));
      }
    }

    return ret_ptr;
  }

  // %%% apply
  DEF_LC_FUNCTION(Lisp::ApplyFunc) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数は関数オブジェクトか関数名。
    LPointer func_ptr = caller->Evaluate(*(args_ptr->car()));

    // 第2引数は引数リスト。
    LPointer args_list_ptr = caller->Evaluate(*(args_ptr->cdr()->car()));
    CheckList(*args_list_ptr);

    // ペアにして評価して返す。
    return func_ptr->Apply(caller,
    LPair(func_ptr, WrapListQuote(args_list_ptr)));
  }

  // %%% walk
  DEF_LC_FUNCTION(Lisp::WalkFunc) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数は関数オブジェクトか関数名。
    LPointer func_ptr = caller->Evaluate(*(args_ptr->car()));

    // 第2引数は探索するペア。
    LPointer pair_ptr = caller->Evaluate(*(args_ptr->cdr()->car()));
    CheckType(*pair_ptr, LType::PAIR);

    // ペアにしておく。
    LPointer func_pair_ptr =
    NewPair(func_ptr, NewPair(WrapQuote(NewNil()), NewPair()));

    // 関数オブジェクトを作成。
    LFuncForWalk func = [func_pair_ptr, &caller]
    (LObject& pair, const std::string& path) {
      // --- Car --- //
      // コールバックを実行する。
      func_pair_ptr->cdr()->car()->cdr()->car(pair.car());
      func_pair_ptr->cdr()->cdr()->car(NewString(path + "a"));
      LPointer result = caller->Evaluate(*func_pair_ptr);

      // 結果の第1要素が@replaceなら置き換え。
      if (result->IsPair()) {
        if (result->car()->symbol() == "@replace") {
          if (result->cdr()->IsPair()) {
            pair.car(result->cdr()->car());
          }
        }
      }

      // --- Cdr --- //
      // コールバックを実行する。
      func_pair_ptr->cdr()->car()->cdr()->car(pair.cdr());
      func_pair_ptr->cdr()->cdr()->car(NewString(path + "d"));
      result = caller->Evaluate(*func_pair_ptr);

      // 結果の第1要素が@replaceなら置き換え。
      if (result->IsPair()) {
        if (result->car()->symbol() == "@replace") {
          if (result->cdr()->IsPair()) {
            pair.cdr(result->cdr()->car());
          }
        }
      }
    };

    // Walkする。
    Walk(*pair_ptr, func);

    // 編集後のpair_ptrを返す。
    return pair_ptr;
  }

  // %%% display
  DEF_LC_FUNCTION(Lisp::Display) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    std::ostringstream oss;
    LPointer result;
    for (; args_ptr->IsPair(); Next(&args_ptr)) {
      result = caller->Evaluate(*(args_ptr->car()));

      // StringとSymbol以外はToString()。
      switch (result->type()) {
        case LType::STRING:
          oss << result->string();
          break;
        case LType::SYMBOL:
          oss << "Symbol: " << result->symbol();
          break;
        default:
          oss << result->ToString();
          break;
      }
    }
    oss << std::endl;

    // 標準出力に表示。
    std::cout << oss.str() << std::flush;

    return NewString(oss.str());
  }

  // %%% stdin
  /** ネイティブ関数 - stdin */
  DEF_LC_FUNCTION(Lisp::Stdin) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 標準入力が閉じていればNil。
    if (!std::cin) return NewNil();

    // メッセージシンボルを得る。
    LPointer symbol_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*symbol_ptr, LType::SYMBOL);
    const std::string& symbol = symbol_ptr->symbol();

    // メッセージシンボルに合わせて分岐。
    if (symbol == "@read") {
      std::ostringstream oss;
      oss << std::cin.rdbuf();
      return NewString(oss.str());
    }
    if (symbol == "@read-line") {
      std::string input;
      std::getline(std::cin, input);
      return NewString(input);
    }
    if (symbol == "@get") {
      return NewString(std::string(1, std::cin.get()));
    }

    throw GenError("@function-error", "'" + args.car()->ToString()
    + "' understands '@read', '@read-line' or '@get'. Not '" + symbol + "'.");
  }

  // %%% stdout
  /** ネイティブ関数 - stdout */
  DEF_LC_FUNCTION(Lisp::Stdout) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 標準出力が閉じていればNil。
    if (!std::cout) return NewNil();

    // 出力する文字列を得る。
    LPointer str_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*str_ptr, LType::STRING);

    // 出力。
    std::cout << str_ptr->string() << std::flush;

    return self;
  }

  // %%% stderr
  /** ネイティブ関数 - stderr */
  DEF_LC_FUNCTION(Lisp::Stderr) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 標準エラーが閉じていればNil。
    if (!std::cerr) return NewNil();

    // 出力する文字列を得る。
    LPointer str_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*str_ptr, LType::STRING);

    // 出力。
    std::cerr << str_ptr->string() << std::flush;

    return self;
  }

  // %%% import
  /** ネイティブ関数 - import */
  DEF_LC_FUNCTION(Lisp::Import) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // ファイル名を読み込む。
    LPointer filename_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*filename_ptr, LType::STRING);

    // ファイルを開く。
    std::ifstream ifs(filename_ptr->string());
    if (!ifs) {
      throw GenError("@function-error", "Couldn't find '"
      + filename_ptr->string() + "'.");
    }

    // 読み込む。
    std::ostringstream oss;
    oss << ifs.rdbuf();

    // パーサを準備。
    LParser parser;

    // 字句解析する。
    parser.Tokenize(oss.str());

    // 解析できたかチェック。
    if ((parser.parenth_counter() != 0) || parser.in_string()) {
      throw GenError("@parse-error",
      "Couldn't parse '" + args_ptr->car()->ToString() + "'.");
    }

    // パースする。
    LPointerVec parse_result = parser.Parse();
    if (parse_result.size() <= 0) {
      throw GenError("@parse-error",
      "Couldn't parse '" + args_ptr->car()->ToString() + "'.");
    }

    // 実行する。
    LPointer ret_ptr = NewNil();
    for (auto& ptr : parse_result) {
      ret_ptr = caller->Evaluate(*ptr);
    }

    return ret_ptr;
  }

  // %%% export
  DEF_LC_FUNCTION(Lisp::Export) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // ファイル名をパース。
    LPointer filename_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*filename_ptr, LType::STRING);

    // ファイルを追加モードで開く。
    std::ofstream ofs(filename_ptr->string(),
    std::ios_base::out | std::ios_base::app);
    if (!ofs) {
      throw GenError("@function-error",
      "Couldn't open '" + filename_ptr->string() + "'.");
    }

    // 残り引数の評価結果を文字列に変換して文字列ストリームにストック。
    std::ostringstream oss;
    for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
      oss << caller->Evaluate(*(args_ptr->car()))->ToString() << std::endl;
    }

    // 文字列ストリームの内容をファイルに保存。
    ofs << oss.str();

    // 閉じて終了。
    ofs.close();
    return NewString(oss.str());
  }

  // %%% output-stream
  DEF_LC_FUNCTION(Lisp::OutputStream) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // ファイル名を読み込む。
    LPointer filename_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*filename_ptr, LType::STRING);

    // ストリームをヒープで開く。
    std::shared_ptr<std::ofstream>
    ofs_ptr(new std::ofstream(filename_ptr->string()));
    if (!(*ofs_ptr)) {
      throw GenError("@function-error",
      "Couldn't open '" + filename_ptr->string() + "'.");
    }

    // 関数オブジェクトを作成。
    auto c_function = [ofs_ptr](LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      // ストリームが閉じていれば終了。
      if (!(*ofs_ptr)) return NewNil();

      // 準備。
      LObject* args_ptr = nullptr;
      GetReadyForFunction(args, 1, &args_ptr);

      // 出力する文字列を得る。 Nilならストリームを閉じる。
      LPointer str_ptr = caller->Evaluate(*(args_ptr->car()));
      if (str_ptr->IsNil()) {
        ofs_ptr->close();
        return self;
      }
      CheckType(*str_ptr, LType::STRING);

      *ofs_ptr << str_ptr->string() << std::flush;

      return self;
    };

    return NewN_Function(c_function, "Lisp:output-stream:"
    + std::to_string(reinterpret_cast<std::size_t>(ofs_ptr.get())),
    caller->scope_chain());
  }

  // %%% input-stream
  DEF_LC_FUNCTION(Lisp::InputStream) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // ファイル名を読み込む。
    LPointer filename_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*filename_ptr, LType::STRING);

    // ストリームをヒープで開く。
    std::shared_ptr<std::ifstream>
    ifs_ptr(new std::ifstream(filename_ptr->string()));
    if (!(*ifs_ptr)) {
      throw GenError("@function-error",
      "Couldn't open '" + filename_ptr->string() + "'.");
    }

    // 関数オブジェクトを作成。
    LC_Function c_function = [ifs_ptr](LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      // ストリームが閉じていれば終了。
      if (!(*ifs_ptr)) return NewNil();

      // 準備。
      LObject* args_ptr = nullptr;
      GetReadyForFunction(args, 1, &args_ptr);

      // メッセージシンボルを得る。 Nilならストリームを閉じる。
      LPointer symbol_ptr = caller->Evaluate(*(args_ptr->car()));
      if (symbol_ptr->IsNil()) {
        ifs_ptr->close();
        return self;
      }
      CheckType(*symbol_ptr, LType::SYMBOL);
      const std::string& symbol = symbol_ptr->symbol();

      // メッセージシンボルに合わせて分岐。
      if (symbol == "@read") {
        std::ostringstream oss;
        oss << ifs_ptr->rdbuf();
        return NewString(oss.str());
      }
      if (symbol == "@read-line") {
        std::string input;
        std::getline(*ifs_ptr, input);
        return NewString(input);
      }
      if (symbol == "@get") {
        return NewString(std::string(1, ifs_ptr->get()));
      }

      throw GenError("@function-error", "'" + args.car()->ToString()
      + "' understands '@read', '@read-line' or '@get'. Not '"
      + symbol + "'.");
    };

    return NewN_Function(c_function, "Lisp:input-stream:"
    + std::to_string(reinterpret_cast<std::size_t>(ifs_ptr.get())),
    caller->scope_chain());
  }

  // %%% gen-thread
  DEF_LC_FUNCTION(Lisp::GenThread) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 関数オブジェクトを入手。
    LPointer func_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*func_ptr, LType::FUNCTION);

    // スレッド用ポインタ。
    std::shared_ptr<std::thread> thread_ptr(new std::thread());

    // 関数オブジェクトを作成。
    LC_Function func = [func_ptr, thread_ptr](LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      GetReadyForFunction(args, 1, &args_ptr);

      // メッセージシンボルを得る。
      LPointer message_symbol = caller->Evaluate(*(args_ptr->car()));
      CheckType(*message_symbol, LType::SYMBOL);
      const std::string& message = message_symbol->symbol();

      Next(&args_ptr);

      // 各メッセージシンボルに対応。
      if (message == "@start") {
        // 進行中なら何もしない。
        if (thread_ptr->joinable()) return NewBoolean(false);

        // 関数オブジェクトをコピー。
        LPointer func_clone = func_ptr->Clone();

        // 引数をコピー。
        LPointer args_copy = args_ptr->Clone();

        // 呼び出し元のスコープを得る。
        self->scope_chain(caller->scope_chain());

        // スレッドを起動。
        *thread_ptr = std::thread([func_clone, self, args_copy]() {
          func_clone->Apply(self.get(), LPair(func_clone, args_copy));
        });

        return NewBoolean(true);
      }

      if (message == "@join") {
        try {
          thread_ptr->join();
          return NewBoolean(true);
        } catch (std::system_error err) {
          // 無視。
        }
        return NewBoolean(false);
      }

      if (message == "@terminated?") {
        return NewBoolean(!(thread_ptr->joinable()));
      }

      throw GenError("@function-error",
      "'" + message + "' is not Thread's Message Symbol.");
    };

    LPointer ret_ptr = NewN_Function(func, "Lisp:gen-thread:"
    + std::to_string(reinterpret_cast<std::size_t>(thread_ptr.get())),
    caller->scope_chain());

    return ret_ptr;
  }

  // %%% sleep
  DEF_LC_FUNCTION(Lisp::Sleep) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    LPointer seconds_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*seconds_ptr, LType::NUMBER);

    std::chrono::milliseconds sleep_time
    (static_cast<int>(seconds_ptr->number() * 1000.0));
    std::this_thread::sleep_for(sleep_time);

    return seconds_ptr;
  }

  // %%% gen-mutex
  DEF_LC_FUNCTION(Lisp::GenMutex) {
    // 各オブジェクトを作成。
    std::shared_ptr<std::mutex> mutex_ptr(new std::mutex);
    std::shared_ptr<std::condition_variable>
    cond_var_ptr(new std::condition_variable());

    // 関数オブジェクトを作成。
    LC_Function func = [mutex_ptr, cond_var_ptr](LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      GetReadyForFunction(args, 1, &args_ptr);

      // メッセージシンボルを得る。
      LPointer symbol_ptr = caller->Evaluate(*(args_ptr->car()));
      CheckType(*symbol_ptr, LType::SYMBOL);
      const std::string& message = symbol_ptr->symbol();

      if (message == "@lock") {  // ロック。
        mutex_ptr->lock();
        return NewBoolean(true);
      }

      if (message == "@unlock") {  // アンロック。
        mutex_ptr->unlock();
        return NewBoolean(true);
      }

      if (message == "@synchronize") {
        std::unique_lock<std::mutex> lock(*mutex_ptr);  // ユニークロック。

        // ローカルスコープを作成。
        LScopeChain local_chain = caller->scope_chain();
        local_chain.AppendNewScope();

        // (wait)関数を作成。
        LC_Function wait_func =
        [&lock, &mutex_ptr, &cond_var_ptr](LPointer self,
        LObject* caller, const LObject& args) -> LPointer {
          cond_var_ptr->wait(lock);
          return NewBoolean(true);
        };

        // バインド。
        local_chain.InsertSymbol("wait", NewN_Function(wait_func, "Lisp:wait:"
        + std::to_string(reinterpret_cast<std::size_t>(mutex_ptr.get())),
        local_chain));

        // 自身にローカルチェーンをセット。
        self->scope_chain(local_chain);

        // 各S式を実行。
        LPointer ret_ptr = NewNil();
        for (LObject* ptr = args_ptr->cdr().get(); ptr->IsPair(); Next(&ptr)) {
          const LPointer& car = ptr->car();

          // 自身のスコープで実行。
          ret_ptr = self->Evaluate(*car);
        }

        return ret_ptr;
      }

      if (message == "@notify-one") {
        cond_var_ptr->notify_one();
        return NewBoolean(true);
      }

      if (message == "@notify-all") {
        cond_var_ptr->notify_all();
        return NewBoolean(true);
      }

      throw GenError("@function-error",
      "'" + message + "' is not Mutex's Message Symbol.");
    };

    LPointer ret_ptr = NewN_Function(func, "Lisp:gen-mutex:"
    + std::to_string(reinterpret_cast<std::size_t>(mutex_ptr.get())),
    caller->scope_chain());

    return ret_ptr;
  }

  // %%% append
  DEF_LC_FUNCTION(Lisp::Append) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // くっつける元のやつ。。
    LPointer first_ptr = caller->Evaluate(*(args_ptr->car()));

    // リストと文字列で分岐。
    // リストの場合。
    if (first_ptr->IsList()) {
      // ダミーに入れる。
      LPointer dummy = NewPair(NewNil(), first_ptr);
      LObject* dummy_ptr = dummy.get();

      // 残りのリストを付け加えていく。
      for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
        // dummy_ptrの最後のペアまでループ。
        for (; dummy_ptr->cdr()->IsPair(); Next(&dummy_ptr)) continue;

        // くっつける。
        dummy_ptr->cdr(caller->Evaluate(*(args_ptr->car())));
      }

      // 返す。
      return dummy->cdr();
    }

    // 文字列の場合。
    if (first_ptr->IsString()) {
      // 連結していく。
      std::ostringstream oss;
      oss << first_ptr->string();

      LPointer result;
      for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
        result = caller->Evaluate(*(args_ptr->car()));
        switch (result->type()) {
          case LType::STRING:
            oss << result->string();
            break;
          case LType::NIL:
          case LType::PAIR:
          case LType::SYMBOL:
          case LType::NUMBER:
          case LType::BOOLEAN:
            oss << result->ToString();
            break;
          default:
            break;
        }
      }

      return NewString(oss.str());
    }

    throw GenTypeError(*(args.car()), "List or String");
  }

  // %%% reverse
  DEF_LC_FUNCTION(Lisp::Reverse) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // リストを得る。
    LPointer list_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*list_ptr);

    // リストをベクトルへ。
    LPointerVec list_vec = ListToLPointerVec(*list_ptr);

    // 逆転。
    std::reverse(list_vec.begin(), list_vec.end());

    return LPointerVecToList(list_vec);
  }

  // %%% ref
  DEF_LC_FUNCTION(Lisp::Ref) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数。 ターゲットを得る。
    LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));

    Next(&args_ptr);

    // 第2引数。 インデックスを得る。
    LPointer index_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*index_ptr, LType::NUMBER);
    int index = index_ptr->number();

    // リストの場合。
    if (target_ptr->IsList()) {
      // インデックスを整理。
      if (index < 0) index = CountList(*target_ptr) + index;

      if (index >= 0) {
        for (LObject* ptr = target_ptr.get(); ptr->IsPair();
        Next(&ptr), --index) {
          if (index <= 0) return ptr->car();
        }
      }

      throw GenError("@function-error", "Index '" + index_ptr->ToString()
      + "' of '" + target_ptr->ToString() + "'is out of range.");
    }

    // 文字列の場合。
    if (target_ptr->IsString()) {
      // インデックスを整理。
      const std::string& target_str = target_ptr->string();
      if (index < 0) index =  target_str.size() + index;

      // 範囲違反。
      if ((index < 0) || (index >= static_cast<int>(target_str.size()))) {
        throw GenError("@function-error", "Index '" + index_ptr->ToString()
        + "' of '" + target_ptr->ToString() + "'is out of range.");
      }

      return NewString(std::string(1, target_str[index]));
    }

    // target_ptrが文字列でもリストでもない。
    throw GenTypeError(*target_ptr, "List or String");
  }

  // %%% list-replace
  DEF_LC_FUNCTION(Lisp::ListReplace) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 3, &args_ptr);

    // 第1引数。 ターゲットのリスト。
    LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*target_ptr);
    int length = CountList(*target_ptr);
    Next(&args_ptr);

    // 第2引数。 インデックス。
    LPointer index_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*index_ptr, LType::NUMBER);
    int index = index_ptr->number();
    if (index < 0) index = length + index;
    if ((index < 0) || (index >= length)) {
      throw GenError("@function-error", "Index '" + index_ptr->ToString()
      + "' of '" + target_ptr->ToString() + "'is out of range.");
    }
    Next(&args_ptr);

    // 第3引数。 入れ替えたい要素。
    LPointer elm_ptr = caller->Evaluate(*(args_ptr->car()));

    // ダミーに入れる。
    LPointer dummy = NewPair(NewNil(), target_ptr);
    LObject* dummy_ptr = dummy.get();

    // indexが0になるまでシフトする。
    for (; index > 0; --index) Next(&dummy_ptr);

    // 入れ替える。
    dummy_ptr->cdr()->car(elm_ptr);

    // 返す。
    return dummy->cdr();
  }

  // %%% list-remove
  DEF_LC_FUNCTION(Lisp::ListRemove) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数。 ターゲットのリスト。
    LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*target_ptr);
    int length = CountList(*target_ptr);
    Next(&args_ptr);

    // 第2引数。 インデックス。
    LPointer index_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*index_ptr, LType::NUMBER);
    int index = index_ptr->number();
    if (index < 0) index = length + index;
    if ((index < 0) || (index >= length)) {
      throw GenError("@function-error", "Index '" + index_ptr->ToString()
      + "' of '" + target_ptr->ToString() + "'is out of range.");
    }

    // ダミー要素を先頭につける。
    LPointer temp = NewPair(NewNil(), target_ptr);
    LObject* ptr = temp.get();

    // 削除する前の要素を特定する。
    for (int i = 0; i < index; ++i) Next(&ptr);

    // 削除する。
    ptr->cdr(ptr->cdr()->cdr());

    // tempのCdrが戻り値。
    return temp->cdr();
  }

  // %%% list-insert
  DEF_LC_FUNCTION(Lisp::ListInsert) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 3, &args_ptr);

    // 第1引数。 ターゲットのリスト。
    LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*target_ptr);
    int length = CountList(*target_ptr);
    Next(&args_ptr);

    // 第2引数。 インデックス。
    LPointer index_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*index_ptr, LType::NUMBER);
    int index = index_ptr->number();
    if (index < 0) index = length + index;
    if ((index < 0) || (index > length)) {  // insertの使用上、Lengthも含む。
      throw GenError("@function-error", "Index '" + index_ptr->ToString()
      + "' of '" + target_ptr->ToString() + "'is out of range.");
    }
    Next(&args_ptr);

    // 第3引数。 挿入するオブジェクト。
    LPointer obj_ptr = caller->Evaluate(*(args_ptr->car()));

    // 挿入するオブジェクトをPairにしておく。
    LPointer obj_pair = NewPair(obj_ptr, NewNil());

    // ターゲットの先頭にダミーを入れる。。
    LPointer temp = NewPair(NewNil(), target_ptr);

    // 場所を特定する。
    LObject* ptr = temp.get();
    for (int i = 0; i < index; ++i) Next(&ptr);

    // 付け替える。
    // 先ず自分のCdrにptrのCdrを入れる。
    obj_pair->cdr(ptr->cdr());
    // ptrのCdrに自分を入れる。
    ptr->cdr(obj_pair);

    // tempのCdrが戻り値。
    return temp->cdr();
  }

  // %%% list-search
  DEF_LC_FUNCTION(Lisp::ListSearch) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数。 ターゲットのリスト。
    LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*target_ptr);
    Next(&args_ptr);

    // 第2引数。 検索語。
    LPointer elm_ptr = caller->Evaluate(*(args_ptr->car()));

    // インデックスを得る。
    int index = 0;
    for (LObject* ptr = target_ptr.get(); ptr->IsPair();
    Next(&ptr), ++index) {
      if (*(ptr->car()) == *elm_ptr) return NewNumber(index);
    }

    return NewNil();
  }

  // %%% list-path
  DEF_LC_FUNCTION(Lisp::ListPath) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数。 ターゲットのリスト。
    LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
    Next(&args_ptr);

    // 第2引数。 パス。
    LPointer path_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*path_ptr, LType::STRING);

    // パスに合わせて探索する。
    for (auto c : path_ptr->string()) {
      CheckType(*target_ptr, LType::PAIR);

      if (c == 'a') {
        target_ptr = target_ptr->car();
      } else if (c == 'd') {
        target_ptr = target_ptr->cdr();
      } else {
        throw GenError("@function-error", "'" + std::string(1, c)
        + "' is not path charactor. A charactor must be 'a' or 'd'.");
      }
    }

    return target_ptr;
  }

  // %%% list-path-replace
  DEF_LC_FUNCTION(Lisp::ListPathReplace) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 3, &args_ptr);

    // 第1引数。 ターゲットのリスト。
    LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
    LObject* t_ptr = target_ptr.get();
    Next(&args_ptr);

    // 第2引数。 パス。
    LPointer path_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*path_ptr, LType::STRING);
    std::string path = path_ptr->string();
    Next(&args_ptr);

    // 第3引数。 置き換えるオブジェクト。
    LPointer obj_ptr = caller->Evaluate(*(args_ptr->car()));

    // 先ず、文字を一つ抜く。
    if (path.size() <= 0) {
      throw GenError("@function-error", "There is no path.");
    }
    char target_c = path[path.size() - 1];
    if (!((target_c == 'a') || (target_c == 'd'))) {
      throw GenError("@function-error", "'" + std::string(1, target_c)
      + "' is not path charactor. A charactor must be 'a' or 'd'.");
    }
    path.pop_back();

    // パスに合わせて探索する。
    for (auto c : path) {
      CheckType(*t_ptr, LType::PAIR);

      if (c == 'a') {
        t_ptr = t_ptr->car().get();
      } else if (c == 'd') {
        t_ptr = t_ptr->cdr().get();
      } else {
        throw GenError("@function-error", "'" + std::string(1, c)
        + "' is not path charactor. A charactor must be 'a' or 'd'.");
      }
    }

    // 置き換え。
    CheckType(*t_ptr, LType::PAIR);
    if (target_c == 'a') {
      t_ptr->car(obj_ptr);
    } else {
      t_ptr->cdr(obj_ptr);
    }

    return target_ptr;
  }

  // %%% list-sort
  DEF_LC_FUNCTION(Lisp::ListSort) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数。 ターゲットのリスト。
    LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*target_ptr);
    Next(&args_ptr);

    // Nilならそのまま返す。
    if (target_ptr->IsNil()) return target_ptr;

    // 要素が1つでも返す。
    if (!(target_ptr->cdr()->IsPair())) return target_ptr;

    // 第2引数は述語。
    LPointer predicate = caller->Evaluate(*(args_ptr->car()));

    // 述語を呼べる形にする。
    LPair callable(predicate, NewPair(WrapQuote(NewNil()),
    NewPair(WrapQuote(NewNil()), NewNil())));
    LObject* prev_arg = callable.cdr()->car()->cdr().get();
    LObject* next_arg = callable.cdr()->cdr()->car()->cdr().get();

    // バブルソートする。
    int size = CountList(*target_ptr) - 1;
    LPointer result;
    for (int i = 0; i < size; ++i) {
      LObject* ptr = target_ptr.get();
      for (int j = 0; j < (size - i); ++j, Next(&ptr)) {
        prev_arg->car(ptr->car());
        next_arg->car(ptr->cdr()->car());

        result = caller->Evaluate(callable);
        CheckType(*result, LType::BOOLEAN);

        // スワップ。
        if (!(result->boolean())) {
          LPointer temp = ptr->car();
          ptr->car(ptr->cdr()->car());
          ptr->cdr()->car(temp);
        }
      }
    }

    return target_ptr;
  }

  // %%% zip
  DEF_LC_FUNCTION(Lisp::Zip) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 引数を評価してベクトルに入れる。
    LPointerVec args_vec(CountList(*args_ptr));
    LPointerVec::iterator args_itr = args_vec.begin();
    for (; args_ptr->IsPair(); Next(&args_ptr), ++args_itr) {
      *args_itr = caller->Evaluate(*(args_ptr->car()));
    }

    return ZipLists(args_vec);
  }

  // %%% map
  DEF_LC_FUNCTION(Lisp::Map) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数は関数オブジェクトか関数名シンボル。
    LPointer func_ptr = caller->Evaluate(*(args_ptr->car()));

    // 関数をペアのCarに入れておく。
    LPair func_pair(func_ptr, NewPair());

    // 第2引数以降のベクトル。
    Next(&args_ptr);
    LPointerVec args_vec(CountList(*args_ptr));
    LPointerVec::iterator args_itr = args_vec.begin();
    for (; args_ptr->IsPair(); Next(&args_ptr), ++args_itr) {
      *args_itr = caller->Evaluate(*(args_ptr->car()));
    }

    // 引数をジップする。
    LPointer zip = ZipLists(args_vec);

    // マップしていく。
    LPointerVec ret_vec;
    for (LObject* ptr = zip.get(); ptr->IsPair(); Next(&ptr)) {
      // 関数のペアのCdrに引数リストを入れる。
      func_pair.cdr(WrapListQuote(ptr->car()));

      // 適用してベクトルにプッシュ。
      ret_vec.push_back(func_ptr->Apply(caller, func_pair));
    }

    return LPointerVecToList(ret_vec);
  }

  // %%% filter
  DEF_LC_FUNCTION(Lisp::Filter) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数は関数オブジェクトか関数名シンボル。
    LPointer func_ptr = caller->Evaluate(*(args_ptr->car()));

    // 関数をリストのCarに入れておく。
    LPair func_pair(func_ptr, NewPair(NewNil(), NewNil()));

    // 第2引数はフィルタに掛けるリスト。
    LPointer target_list = caller->Evaluate(*(args_ptr->cdr()->car()));
    CheckList(*target_list);

    // フィルターしていく。
    LPointer result;
    LPointerVec ret_vec;
    for (LObject* ptr = target_list.get(); ptr->IsPair(); Next(&ptr)) {
      // 関数のリストの第1引数に入れる。
      func_pair.cdr()->car(WrapQuote(ptr->car()));

      // 評価してみる。
      result = caller->Evaluate(func_pair);
      CheckType(*result, LType::BOOLEAN);

      // 評価結果がtrueならリストに追加。
      if (result->boolean()) ret_vec.push_back(ptr->car());
    }

    return LPointerVecToList(ret_vec);
  }

  // %%% start-size-inc
  DEF_LC_FUNCTION(Lisp::StartSizeInc) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 3, &args_ptr);

    // スタートの数字。
    LPointer result = caller->Evaluate((*args_ptr->car()));
    CheckType(*result, LType::NUMBER);
    double start = result->number();
    Next(&args_ptr);

    // 個数。
    result = caller->Evaluate(*(args_ptr->car()));
    CheckType(*result, LType::NUMBER);
    int num = result->number();
    if (num < 0) {
      throw GenError("@function-error",
      "The 2nd argument must be 0 and more.");
    }
    Next(&args_ptr);

    // インクリメント。
    result = caller->Evaluate(*(args_ptr->car()));
    CheckType(*result, LType::NUMBER);
    double inc = result->number();

    // リストを作成する。
    LPointer ret_ptr = NewList(num);
    for (LObject* ptr = ret_ptr.get(); ptr->IsPair();
    Next(&ptr), start += inc) {
      ptr->car(NewNumber(start));
    }

    return ret_ptr;
  }

#define COMMON_MATH_EX \
    LObject* args_ptr = nullptr;\
    GetReadyForFunction(args, 2, &args_ptr);\
\
    const LPointer& symbol_ptr = args_ptr->car();\
    CheckType(*symbol_ptr, LType::SYMBOL);\
    const std::string symbol = symbol_ptr->symbol();\
    Next(&args_ptr);\
\
    const LScopeChain& chain = caller->scope_chain();\
    LPointer value_ptr = chain.SelectSymbol(symbol);\
    if (!value_ptr) {\
      throw GenError("@unbound", "'" + symbol + "' doesn't bind any value.");\
    }\
\
    CheckType(*value_ptr, LType::NUMBER);\
    LPointer result = caller->Evaluate(*(args_ptr->car()));\
    CheckType(*result, LType::NUMBER);\
\
    LPointer ret_ptr = value_ptr->Clone()

  // %%% add!
  DEF_LC_FUNCTION(Lisp::AdditionEx) {
    COMMON_MATH_EX;

    value_ptr->number(value_ptr->number() + result->number());

    return ret_ptr;
  }

  // %%% sub!
  DEF_LC_FUNCTION(Lisp::SubtractionEx) {
    COMMON_MATH_EX;

    value_ptr->number(value_ptr->number() - result->number());

    return ret_ptr;
  }

  // %%% mul!
  DEF_LC_FUNCTION(Lisp::MultiplicationEx) {
    COMMON_MATH_EX;

    value_ptr->number(value_ptr->number() * result->number());

    return ret_ptr;
  }

  // %%% div!
  DEF_LC_FUNCTION(Lisp::DivisionEx) {
    COMMON_MATH_EX;

    value_ptr->number(value_ptr->number() / result->number());

    return ret_ptr;
  }


#define COMMON_INC_DEC_EX \
    LObject* args_ptr = nullptr;\
    GetReadyForFunction(args, 1, &args_ptr);\
\
    const LPointer& symbol_ptr = args_ptr->car();\
    CheckType(*symbol_ptr, LType::SYMBOL);\
    const std::string symbol = symbol_ptr->symbol();\
    Next(&args_ptr);\
\
    const LScopeChain& chain = caller->scope_chain();\
    LPointer value_ptr = chain.SelectSymbol(symbol);\
    if (!value_ptr) {\
      throw GenError("@unbound", "'" + symbol + "' doesn't bind any value.");\
    }\
\
    CheckType(*value_ptr, LType::NUMBER);\
\
    LPointer ret_ptr = value_ptr->Clone()

  // %%% inc!
  DEF_LC_FUNCTION(Lisp::IncEx) {
    COMMON_INC_DEC_EX;

    value_ptr->number(value_ptr->number() + 1.0);

    return ret_ptr;
  }

  // %%% dec!
  DEF_LC_FUNCTION(Lisp::DecEx) {
    COMMON_INC_DEC_EX;

    value_ptr->number(value_ptr->number() - 1.0);

    return ret_ptr;
  }

  // %%% string-split
  DEF_LC_FUNCTION(Lisp::StringSplit) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数。 ターゲット。
    LPointer result = caller->Evaluate(*(args_ptr->car()));
    CheckType(*result, LType::STRING);
    std::string target_str = result->string();

    // 第2引数。 区切り文字列。
    result = caller->Evaluate(*(args_ptr->cdr()->car()));
    CheckType(*result, LType::STRING);
    const std::string& delim_str = result->string();

    // 区切ってプッシュしていく。
    LPointerVec ret_vec;
    for (std::string::size_type pos = target_str.find(delim_str);
    pos != std::string::npos;
    pos = target_str.find(delim_str)) {
      ret_vec.push_back(NewString(target_str.substr(0, pos)));
      target_str = target_str.substr(pos + delim_str.size());
    }
    ret_vec.push_back(NewString(target_str));

    return LPointerVecToList(ret_vec);
  } 

  // %%% string-join
  DEF_LC_FUNCTION(Lisp::StringJoin) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数。 リスト。
    LPointer list_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*list_ptr);
    Next(&args_ptr);

    // リストが空ならそのまま返す。
    if (list_ptr->IsNil()) return list_ptr;

    // 第2引数。 区切り文字。
    LPointer delim_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*delim_ptr, LType::STRING);
    const std::string& delim = delim_ptr->string();

    // 間に挟んでいく。
    // まずは第1要素。
    std::ostringstream oss;
    if (list_ptr->car()->IsString()) oss << list_ptr->car()->string();
    else oss << list_ptr->car()->ToString();

    // 第2要素以降。
    for (LObject* ptr = list_ptr->cdr().get(); ptr->IsPair(); Next(&ptr)) {
      oss << delim;
      if (ptr->car()->IsString()) oss << ptr->car()->string();
      else oss << ptr->car()->ToString();
    }

    return NewString(oss.str());
  }

#define COMMON_PUSH_EX \
    LObject* args_ptr = nullptr;\
    GetReadyForFunction(args, 2, &args_ptr);\
\
    const LPointer& symbol_ptr = args_ptr->car();\
    CheckType(*symbol_ptr, LType::SYMBOL);\
    const std::string symbol = symbol_ptr->symbol();\
    Next(&args_ptr);\
\
    const LScopeChain& chain = caller->scope_chain();\
    LPointer value_ptr = chain.SelectSymbol(symbol);\
    if (!value_ptr) {\
      throw GenError("@unbound", "'" + symbol + "' doesn't bind any value.");\
    }\
\
    CheckList(*value_ptr);\
    LPointer supplement = caller->Evaluate(*(args_ptr->car()))

#define COMMON_POP_EX \
    LObject* args_ptr = nullptr;\
    GetReadyForFunction(args, 1, &args_ptr);\
\
    const LPointer& symbol_ptr = args_ptr->car();\
    CheckType(*symbol_ptr, LType::SYMBOL);\
    const std::string symbol = symbol_ptr->symbol();\
    Next(&args_ptr);\
\
    const LScopeChain& chain = caller->scope_chain();\
    LPointer value_ptr = chain.SelectSymbol(symbol);\
    if (!value_ptr) {\
      throw GenError("@unbound", "'" + symbol + "' doesn't bind any value.");\
    }\
\
    CheckList(*value_ptr)

  // %%% push-front!
  DEF_LC_FUNCTION(Lisp::PushFrontEx) {
    COMMON_PUSH_EX;

    // 追加する。
    chain.UpdateSymbol(symbol, NewPair(supplement, value_ptr));

    return value_ptr->Clone();
  }

  // %%% pop-front!
  DEF_LC_FUNCTION(Lisp::PopFrontEx) {
    COMMON_POP_EX;

    // 外す。
    if (value_ptr->IsPair()) {
      chain.UpdateSymbol(symbol, value_ptr->cdr());
    }

    return value_ptr->Clone();
  }

  // %%% push-back!
  DEF_LC_FUNCTION(Lisp::PushBackEx) {
    COMMON_PUSH_EX;

    // 先ず戻り値を確保。
    LPointer ret_ptr = value_ptr->Clone();

    // 追加する。
    LPair dummy = LPair(NewNil(), value_ptr);
    LObject* ptr = &dummy;
    for (; ptr->cdr()->IsPair(); Next(&ptr)) continue;
    ptr->cdr(NewPair(supplement, NewNil()));

    chain.UpdateSymbol(symbol, dummy.cdr());

    return ret_ptr;
  }

  // %%% pop-back!
  DEF_LC_FUNCTION(Lisp::PopBackEx) {
    COMMON_POP_EX;

    // 元々Nilなら何もしない。
    if (value_ptr->IsNil()) return value_ptr->Clone();

    // 先ず戻り値を確保。
    LPointer ret_ptr = value_ptr->Clone();

    LPair dummy = LPair(NewNil(), NewPair(NewNil(), value_ptr));
    LObject* ptr = &dummy;
    for (; ptr->cdr()->cdr()->IsPair(); Next(&ptr)) continue;
    ptr->cdr(NewNil());

    chain.UpdateSymbol(symbol, dummy.cdr()->cdr());

    return ret_ptr;
  }

  // %%% regex-search
  DEF_LC_FUNCTION(Lisp::RegexSearch) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 正規表現を得る。
    LPointer reg_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*reg_ptr, LType::STRING);
    Next(&args_ptr);
    std::regex reg(reg_ptr->string(), std::regex_constants::ECMAScript);

    // 対象文字列を得る。
    LPointer str_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*str_ptr, LType::STRING);

    // マッチしたものを探す。
    LPointerVec ret_vec;
    std::smatch result;
    if (std::regex_search(str_ptr->string(), result, reg)) {
      for (auto& token : result) {
        ret_vec.push_back(NewString(token));
      }
    }

    return LPointerVecToList(ret_vec);
  }

  // %%% gen-nabla
  DEF_LC_FUNCTION(Lisp::GenNabla) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数は関数。
    LPointer func_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*func_ptr, LType::FUNCTION);
    LPointer func_expr = NewPair(func_ptr, NewNil());
    Next(&args_ptr);

    // 第2引数以降はデルタ。
    int len = CountList(*args_ptr);
    std::vector<double> delta_vec(len);
    LPointer result;
    for (int i = 0; i < len; ++i, Next(&args_ptr)) {
      result = caller->Evaluate(*(args_ptr->car()));
      CheckType(*result, LType::NUMBER);
      delta_vec[i] = result->number();
    }

    // ナブラの関数オブジェクトを作る。
    auto nabla_func = [func_expr, len, delta_vec](LPointer self,
    LObject* caller, const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      GetReadyForFunction(args, len, &args_ptr);

      // 変数のリストを得る。
      LPointer var_list = NewList(len);
      LObject* var_ptr = var_list.get();
      LPointer result;
      for (int i = 0; i < len; ++i, Next(&args_ptr), Next(&var_ptr)) {
        result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        var_ptr->car(result);
      }

      // 変数のリストをセット。
      func_expr->cdr(var_list);

      // ループする。
      LPointerVec ret_vec(len);
      var_ptr = var_list.get();
      for (int i = 0; i < len; ++i, Next(&var_ptr)) {
        // 先ず数値を退避。
        double temp_number = var_ptr->car()->number();

        // 準備。
        double delta = delta_vec[i];
        double half_delta = delta / 2.0;

        // 大きい方を計算。
        var_ptr->car()->number(temp_number + half_delta);
        result = func_expr->car()->Apply(caller, *func_expr);
        CheckType(*result, LType::NUMBER);
        double big = result->number() / delta;

        // 小さい方を計算。
        var_ptr->car()->number(temp_number - half_delta);
        result = func_expr->car()->Apply(caller, *func_expr);
        CheckType(*result, LType::NUMBER);
        double small = result->number() / delta;

        // 微分係数を入れる。
        ret_vec[i] = NewNumber(big - small);

        // 退避したものを戻す。
        var_ptr->car()->number(temp_number);
      }

      return LPointerVecToList(ret_vec);
    };

    return NewN_Function(nabla_func, "Lisp:gen-nabla:"
    + std::to_string(reinterpret_cast<size_t>(func_expr.get())),
    caller->scope_chain());
  }

  // %%% integral
  DEF_LC_FUNCTION(Lisp::Integral) {
    using namespace LMath;
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 第1引数は関数。
    LPointer func_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*func_ptr, LType::FUNCTION);
    LPointer func_expr = NewPair(func_ptr, NewNil());
    Next(&args_ptr);

    // 積分用関数を作る。
    int len = CountList(*args_ptr);
    LPointer var_list = NewList(len);  // 引数リスト。
    func_expr->cdr(var_list);  // セット。
    LObject* var_ptr = var_list.get();
    std::vector<std::function<double()>> sum_func_vec(len);
    LPointer result;
    for (int i = 0; i < len; ++i, Next(&args_ptr), Next(&var_ptr)) {
      result = caller->Evaluate(*(args_ptr->car()));
      CheckList(*result);
      if (CountList(*result) < 3) {
        throw GenError("@function-error", "'" + result->ToString()
        + "' doesn't have 3 elements and more.");
      }
      const LPointer& from_ptr = result->car();
      CheckType(*from_ptr, LType::NUMBER);
      const LPointer& to_ptr = result->cdr()->car();
      CheckType(*to_ptr, LType::NUMBER);
      const LPointer& delta_ptr = result->cdr()->cdr()->car();
      CheckType(*delta_ptr, LType::NUMBER);

      double from = from_ptr->number();
      double to = to_ptr->number();
      double delta = std::fabs(delta_ptr->number());
      delta = (to - from) < 0.0 ? -delta : delta;
      from -= delta / 2.0;
      var_ptr->car(NewNumber(from));
      LObject* m_var_ptr = var_ptr->car().get();

      if (i == 0) {
        auto func =
        [func_expr, var_list, m_var_ptr, from, to, delta, caller]() -> double {
          double ret = 0.0;
          LPointer result;
          for (double current = from; current < to; current += delta) {
            m_var_ptr->number(current);
            result = caller->Evaluate(*func_expr);
            ret += result->number();
          }
          return ret * delta;
        };
        sum_func_vec[i] = func;
      } else {
        std::function<double()>* sum_func_ptr = &(sum_func_vec[i - 1]);
        auto func =
        [sum_func_ptr, m_var_ptr, from, to, delta]() -> double {
          double ret = 0.0;
          for (double current = from; current < to; current += delta) {
            m_var_ptr->number(current);
            ret += (*sum_func_ptr)();
          }
          return ret * delta;
        };
        sum_func_vec[i] = func;
      }
    }

    // 積分する。
    return NewNumber(sum_func_vec[len - 1]());
  }

  // %%% power-method
  DEF_LC_FUNCTION(Lisp::PowerMethod) {
    using namespace LMath;
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 行列を作る。
    LPointer matrix_ptr = caller->Evaluate(*(args_ptr->car()));
    Mat matrix = ListToMatrix(*matrix_ptr);
    int dim = matrix.size();

    // 固有値を計算。
    for (int i = 0; i < dim; ++i) {
    }
    double lambda = 0.0;
    Vec eigen_vec;
    std::tie(lambda, eigen_vec) = LMath::Eigen(matrix);
    if (lambda == 0.0) return NewNil();

    LPointer ret_ptr = NewPair(NewNumber(lambda),
    NewPair(MathVecToList(eigen_vec), NewNil()));

    return ret_ptr;
  }

  // %%% inverse-matrix
  DEF_LC_FUNCTION(Lisp::InverseMatrix) {
    using namespace LMath;
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);


    // 行列を作る。
    LPointer matrix_ptr = caller->Evaluate(*(args_ptr->car()));
    Mat matrix = ListToMatrix(*matrix_ptr);
    int dim = matrix.size();

    // 逆行列を計算する。
    Mat inv_matrix = LMath::Inverse(matrix);
    if (inv_matrix.empty()) return NewNil();

    // リストにする。
    LPointerVec ret_vec(dim);
    for (int i = 0; i < dim; ++i) {
      ret_vec[i] = MathVecToList(inv_matrix[i]);
    }

    return LPointerVecToList(ret_vec);
  }

  // %%% transposed-matrix
  DEF_LC_FUNCTION(Lisp::TransposedMatrix) {
    using namespace LMath;
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 行列を得る。
    LPointer matrix_ptr = caller->Evaluate(*(args_ptr->car()));

    // 転置する。
    CheckList(*matrix_ptr);
    int dim_1 = CountList(*matrix_ptr);
    CheckList(*(matrix_ptr->car()));
    int dim_2 = CountList(*(matrix_ptr->car()));
    std::vector<LPointerVec> temp_matrix(dim_2, LPointerVec(dim_1));
    LObject* ptr_1 = matrix_ptr.get();
    for (int i = 0; i < dim_1; ++i, Next(&ptr_1)) {
      const LPointer& temp = ptr_1->car();
      CheckList(*temp);
      if (CountList(*temp) != dim_2) {
        throw GenError("@function-error",
        "'" + matrix_ptr->ToString() + "' is not Matrix.");
      }

      LObject* ptr_2 = temp.get();
      for (int j = 0; j < dim_2; ++j, Next(&ptr_2)) {
        temp_matrix[j][i] = ptr_2->car();
      }
    }

    // リストにする。
    LPointerVec ret_vec(dim_2);
    for (int j = 0; j < dim_2; ++j) {
      ret_vec[j] = LPointerVecToList(temp_matrix[j]);
    }
    return LPointerVecToList(ret_vec);
  }

  // %%% determinant
  DEF_LC_FUNCTION(Lisp::Determinant) {
    using namespace LMath;
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 行列を作る。
    LPointer matrix_ptr = caller->Evaluate(*(args_ptr->car()));
    Mat matrix = ListToMatrix(*matrix_ptr);

    // 行列式を計算する。
    return NewNumber(LMath::Determinant(matrix));
  }

  // %%% bayes
  DEF_LC_FUNCTION(Lisp::Bayes) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 3, &args_ptr);

    // 第1引数はデータリスト。
    LPointer data_list_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*data_list_ptr);
    Next(&args_ptr);

    // 第2引数はイベント関数のリスト。
    LPointer event_list = caller->Evaluate(*(args_ptr->car()));
    CheckList(*event_list);
    Next(&args_ptr);

    // 第2引数を包んでベクトルへ。
    int num_event = CountList(*event_list);
    if (num_event <= 0) {
      throw GenError("@function-error", "No event predicate.");
    }
    LPointerVec event_vec(num_event);
    LPointerVec::iterator event_itr = event_vec.begin();
    for (LObject* ptr = event_list.get(); ptr->IsPair();
    Next(&ptr), ++event_itr) {
      *event_itr = NewPair(ptr->car(), NewPair(WrapQuote(NewNil()), NewNil()));
    }

    // 第3引数は条件関数のリスト。
    LPointer cond_list = caller->Evaluate(*(args_ptr->car()));
    CheckList(*cond_list);
    Next(&args_ptr);

    // 第3引数を包んでベクトルへ。
    int num_cond = CountList(*cond_list);
    if (num_cond <= 0) {
      throw GenError("@function-error", "No condition predicate.");
    }
    LPointerVec cond_vec(num_cond);
    LPointerVec::iterator cond_itr = cond_vec.begin();
    for (LObject* ptr = cond_list.get(); ptr->IsPair();
    Next(&ptr), ++cond_itr) {
      *cond_itr = NewPair(ptr->car(), NewPair(WrapQuote(NewNil()), NewNil()));
    }

    // カウント表を準備。
    double num_all = 1.0;
    std::vector<double> event_count(num_event, 0.5);
    std::vector<double> not_event_count(num_event, 0.5);
    std::vector<std::vector<double>>
    cond_count(num_event, std::vector<double>(num_cond, 0.0));
    std::vector<std::vector<double>>
    not_cond_count(num_event, std::vector<double>(num_cond, 0.0));

    // 各データでカウントしていく。
    LPointer result;
    std::vector<double> event_value_vec(num_event);
    std::vector<double> not_event_value_vec(num_event);
    std::vector<double> cond_value_vec(num_cond);
    for (LPointer ptr = data_list_ptr; ptr->IsPair(); ptr = ptr->cdr()) {
      num_all += 1.0;
      const LPointer& data = ptr->car();

      // イベント関数を評価。
      for (int i = 0; i < num_event; ++i) {
        event_vec[i]->cdr()->car()->cdr()->car(data);
        result = caller->Evaluate(*(event_vec[i]));

        if (result->IsBoolean()) {
          // 古典。
          if (result->boolean()) {
            event_value_vec[i] = 1.0;
            not_event_value_vec[i] = 0.0;
            event_count[i] += 1.0;
          } else {
            event_value_vec[i] = 0.0;
            not_event_value_vec[i] = 1.0;
            not_event_count[i] += 1.0;
          }
        } else if (result->IsNumber()) {
          // ファジー。
          double temp = result->number();
          temp = temp < 0.0 ? 0.0 : (temp > 1.0 ? 1.0 : temp);
          event_value_vec[i] = temp;
          not_event_value_vec[i] = 1.0 - temp;

          event_count[i] += temp;
          not_event_count[i] += 1.0 - temp;
        } else {
          // エラー。
          throw GenTypeError(*result, "Boolean or Number");
        }
      }

      // 条件関数を評価。
      for (int i = 0; i < num_cond; ++i) {
        cond_vec[i]->cdr()->car()->cdr()->car(data);
        result = caller->Evaluate(*(cond_vec[i]));

        if (result->IsBoolean()) {
          // 古典。
          if (result->boolean()) {
            cond_value_vec[i] = 1.0;
          } else {
            cond_value_vec[i] = 0.0;
          }
        } else if (result->IsNumber()) {
          // ファジー。
          double temp = result->number();
          temp = temp < 0.0 ? 0.0 : (temp > 1.0 ? 1.0 : temp);
          cond_value_vec[i] = temp;
        } else {
          // エラー。
          throw GenTypeError(*result, "Boolean or Number");
        }
      }

      // 真偽値を直積してカウント表に加算していく。
      for (int i = 0; i < num_event; ++i) {
        for (int j = 0; j < num_cond; ++j) {
          cond_count[i][j] += event_value_vec[i] * cond_value_vec[j];
          not_cond_count[i][j] += not_event_value_vec[i] * cond_value_vec[j];
        }
      }
    }

    // 対数化してロジットを計算する。
    LPointerVec ret_vec(num_event);
    double all_lb = std::log(num_all);
    double delta = 1.0 / (num_all + 1.0);
    for (int i = 0; i < num_event; ++i) {
      // イベントの数を対数化。
      double event_lb = std::log(event_count[i]);
      double not_event_lb = std::log(not_event_count[i]);

      // trueの対数。
      double true_lb = event_lb - all_lb;
      for (int j = 0; j < num_cond; ++j) {
        true_lb += std::log(cond_count[i][j] + delta) - event_lb;
      }

      // falseの対数。
      double false_lb = not_event_lb - all_lb;
      for (int j = 0; j < num_cond; ++j) {
        false_lb += std::log(not_cond_count[i][j] + delta) - not_event_lb;
      }

      // ロジットを記録。
      ret_vec[i] = NewNumber(true_lb - false_lb);
    }

    return LPointerVecToList(ret_vec);
  }

  // %%% logit->prob
  DEF_LC_FUNCTION(Lisp::LogitToProb) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 値を得る。
    LPointer value_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*value_ptr, LType::NUMBER);

    return NewNumber(1.0 / (1.0 + std::exp(-(value_ptr->number()))));
  }

  // %%% prob->logit
  DEF_LC_FUNCTION(Lisp::ProbToLogit) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 値を得る。
    LPointer value_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*value_ptr, LType::NUMBER);
    double value = value_ptr->number();
    if (!((value > 0.0) && (value < 1.0))) {
      throw GenError("@function-error",
      "Probability must be greater than 0.0 and less than 1.0.");
    }

    return NewNumber(std::log(value / (1.0 - value)));
  }

  // %%% gen-pa2
  DEF_LC_FUNCTION(Lisp::GenPA2) {
    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 1, &args_ptr);

    // 初期ウェイトを得る。
    LPointer weights_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*weights_ptr);
    LMath::Vec weight_vec = LMath::ListToMathVec(*weights_ptr);

    // 1つ以上あるかどうかを調べる
    if (weight_vec.size() <= 0) {
      throw GenError("@function-error",
      "PA-2 needs at least 1 element of List of weights.");
    }

    std::shared_ptr<LPA2> obj_ptr(new LPA2(weight_vec));
    LC_Function func = [obj_ptr](LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      return (*obj_ptr)(self, caller, args);
    };

    return NewN_Function(func,
    "Lisp:gen-pa2:" + std::to_string(reinterpret_cast<size_t>(obj_ptr.get())),
    caller->scope_chain());
  }

  // %%% gen-ai
  DEF_LC_FUNCTION(Lisp::GenAI) {
    using namespace LMath;

    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 2, &args_ptr);

    // 初期ウェイトを得る。
    LPointer weights_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*weights_ptr);
    Next(&args_ptr);
    LMath::Vec weight_vec = LMath::ListToMathVec(*weights_ptr);
    unsigned int len = weight_vec.size();

    // 1つ以上あるかどうかを調べる
    if (len <= 0) {
      throw GenError("@function-error",
      "(gen-ai) needs at least 1 element of List of weights.");
    }

    // 初期バイアスを得る。
    LPointer bias_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*bias_ptr, LType::NUMBER);

    std::shared_ptr<LAI> obj_ptr(new LAI(weight_vec, bias_ptr->number()));
    LC_Function func = [obj_ptr](LPointer self, LObject* caller,
    const LObject& args) -> LPointer {
      // 準備。
      LObject* args_ptr = nullptr;
      GetReadyForFunction(args, 1, &args_ptr);

      // シンボルを得る。
      LPointer symbol_ptr = caller->Evaluate(*(args_ptr->car()));
      CheckType(*symbol_ptr, LType::SYMBOL);
      const std::string& symbol = symbol_ptr->symbol();

      // 特徴ベクトルを取り出す関数。
      auto to_feature_vec = [](const LPointer& list) -> Vec {
        for (LObject* ptr = list.get(); ptr->IsPair(); Next(&ptr)) {
          const LPointer& car = ptr->car();
          if (car->IsBoolean()) {
            ptr->car(NewNumber(car->boolean() ? 1.0 : -1.0));
          }
        }
        return ListToMathVec(*list);
      };

      // アクセサ。
      if (symbol == "@get-weights") {
        return MathVecToList(obj_ptr->weights());
      }
      if (symbol == "@get-bias") {
        return NewNumber(obj_ptr->bias());
      }

      // 計算。
      if ((symbol == "@calc") || (symbol == "@logit") || (symbol == "@prob")
      || (symbol == "@judge")) {
        Next(&args_ptr);
        CheckType(*args_ptr, LType::PAIR);

        LPointer features_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckList(*features_ptr);
        Vec features = to_feature_vec(features_ptr);

        if (symbol == "@calc") {
          return NewNumber(obj_ptr->CalDoubleSigmoid(features));
        }
        if (symbol == "@logit") {
          return NewNumber(obj_ptr->CalLogit(features));
        }
        if (symbol == "@prob") {
          return NewNumber(obj_ptr->CalSigmoid(features));
        }
        if (symbol == "@judge") {
          return NewBoolean(obj_ptr->Judge(features));
        }
      }

      // 学習。
      if ((symbol == "@train") || (symbol == "@train-pa1")
      || (symbol == "@train-pa2")) {
        Next(&args_ptr);
        CheckType(*args_ptr, LType::PAIR);

        LPointer num_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckType(*num_ptr, LType::NUMBER);
        double num = num_ptr->number();

        Next(&args_ptr);
        CheckType(*args_ptr, LType::PAIR);

        LPointer output_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckType(*output_ptr, LType::BOOLEAN);
        bool desired_output = output_ptr->boolean();

        Next(&args_ptr);
        CheckType(*args_ptr, LType::PAIR);

        LPointer features_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckList(*features_ptr);
        Vec features = to_feature_vec(features_ptr);

        if (symbol == "@train") {
          return NewNumber(obj_ptr->TrainDoubleSigmoid
          (desired_output, features, num));
        }
        if (symbol == "@train-pa1") {
          return NewNumber(obj_ptr->TrainPA1(desired_output, features, num));
        }
        if (symbol == "@train-pa2") {
          return NewNumber(obj_ptr->TrainPA2(desired_output, features, num));
        }
      }
      if (symbol == "@train-bp") {
        Next(&args_ptr);
        CheckType(*args_ptr, LType::PAIR);

        LPointer rate_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckType(*rate_ptr, LType::NUMBER);
        double rate = rate_ptr->number();

        Next(&args_ptr);
        CheckType(*args_ptr, LType::PAIR);

        LPointer loss_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckList(*loss_ptr);
        Vec loss = ListToMathVec(*loss_ptr);

        Next(&args_ptr);
        CheckType(*args_ptr, LType::PAIR);

        LPointer weights_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckList(*weights_ptr);
        Vec weights = ListToMathVec(*weights_ptr);

        Next(&args_ptr);
        CheckType(*args_ptr, LType::PAIR);

        LPointer features_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckList(*features_ptr);
        Vec features = to_feature_vec(features_ptr);

        return NewNumber(obj_ptr->TrainBackPropagation(loss, weights,
        features, rate));
      }

      throw GenError("@function-error", "'" + args.car()->ToString()
      + "' couldn't understand '" + symbol + "'.");
    };

    return NewN_Function(func,
    "Lisp:gen-ai:" + std::to_string(reinterpret_cast<size_t>(obj_ptr.get())),
    caller->scope_chain());
  }

  // %%% rbf-kernel
  DEF_LC_FUNCTION(Lisp::RBFKernel) {
    using namespace LMath;

    // 準備。
    LObject* args_ptr = nullptr;
    GetReadyForFunction(args, 3, &args_ptr);

    // 第1引数は1つ目のベクトル。
    LPointer vec_1_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*vec_1_ptr);
    Vec vec_1 = ListToMathVec(*vec_1_ptr);
    unsigned int vec_1_size = vec_1.size();
    if (vec_1_size <= 0) {
      throw GenError("@function-error",
      "(gaussian-kernel) needs at least 1 element of vector.");
    }
    Next(&args_ptr);

    // 第2引数は2つ目のベクトル。
    LPointer vec_2_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckList(*vec_2_ptr);
    Vec vec_2 = ListToMathVec(*vec_2_ptr);
    unsigned int vec_2_size = vec_2.size();
    if (vec_2_size != vec_1_size) {
      throw GenError("@function-error",
      "The 1st vector and the 2nd vector is not same size.");
    }
    Next(&args_ptr);

    // 第3引数はバンド幅。
    LPointer band_ptr = caller->Evaluate(*(args_ptr->car()));
    CheckType(*band_ptr, LType::NUMBER);
    double band = band_ptr->number();

    // 計算して返す。
    Vec diff = vec_1 - vec_2;
    double sq_norm = diff * diff;
    return NewNumber(std::exp(-1.0 * (sq_norm / (2.0 * band * band))));
  }

  // %%% now
  DEF_LC_FUNCTION(Lisp::Now) {
    std::time_t time;
    std::time(&time);
    std::tm* time_st = std::localtime(&time);

    LPointer ret_ptr = NewList(6);
    LObject* ptr = ret_ptr.get();
    ptr->car(NewNumber(time_st->tm_year + 1900));
    Next(&ptr);
    ptr->car(NewNumber(time_st->tm_mon + 1));
    Next(&ptr);
    ptr->car(NewNumber(time_st->tm_mday));
    Next(&ptr);
    ptr->car(NewNumber(time_st->tm_hour));
    Next(&ptr);
    ptr->car(NewNumber(time_st->tm_min));
    Next(&ptr);
    ptr->car(NewNumber(time_st->tm_sec));

    return ret_ptr;
  }

  // ==== //
  // LPA2 //
  // ==== //
  // 学習関数。
  void LPA2::TrainWeights(bool is_plus, double cost,
  const LMath::Vec& input) {
    using namespace LMath;

    unsigned int size = weight_vec_.size();
    unsigned int size_2 = input.size();
    size = size_2 < size ? size_2 : size;

    cost = cost < 0.0 ? 0.5 : cost;

    double sign = is_plus ? 1.0 : -1.0;
    double hinge_and_sign = Hinge(sign, input) * sign;
    double denominator = (input * input) + (1.0 / (2.0 * cost));

    for (unsigned int i = 0; i < size; ++i) {
      weight_vec_[i] += (hinge_and_sign * input[i]) / denominator;
    }
  }
  // 関数オブジェクト。
  DEF_LC_FUNCTION(LPA2::operator()) {
    // 準備。
    LObject* args_ptr = nullptr;
    Lisp::GetReadyForFunction(args, 1, &args_ptr);

    // 第1引数はメッセージ。
    LPointer message_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*message_ptr, LType::SYMBOL);
    const std::string& message = message_ptr->symbol();

    if (message == "@train") return Train(self, caller, args);
    if (message == "@calc") return Calc(self, caller, args);
    if (message == "@get-weights") return GetWeights(self, caller, args);

    throw Lisp::GenError("@function-error",
    "Couldn't understand '" + message + "'.");
  }

  // %%% @train
  DEF_LC_FUNCTION(LPA2::Train) {
    // 準備。
    LObject* args_ptr = nullptr;
    Lisp::GetReadyForFunction(args, 4, &args_ptr);
    Lisp::Next(&args_ptr);

    // 第1引数はプラスかどうか。
    LPointer is_plus_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*is_plus_ptr, LType::BOOLEAN);
    Lisp::Next(&args_ptr);

    // 第2引数は許容するエラー。
    LPointer error_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckType(*error_ptr, LType::NUMBER);
    Lisp::Next(&args_ptr);
    double error = error_ptr->number();
    error = error < 0.0 ? 0.0 : error;

    // 第3引数は入力ベクトル。
    LPointer input_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckList(*input_ptr);

    // 入力ベクトルの数をチェック。
    unsigned int num_input = Lisp::CountList(*input_ptr);
    if (num_input != weight_vec_.size()) {
      throw Lisp::GenError("@function-error",
      "Not equal number of elements of weights and inputs.");
    }

    // 学習する。
    TrainWeights(is_plus_ptr->boolean(), error,
    LMath::ListToMathVec(*input_ptr));

    // 学習結果のベクトルを返す。
    return LMath::MathVecToList(weight_vec_);
  }

  // %%% @calc
  DEF_LC_FUNCTION(LPA2::Calc) {
    // 準備。
    LObject* args_ptr = nullptr;
    Lisp::GetReadyForFunction(args, 2, &args_ptr);
    Lisp::Next(&args_ptr);

    // 第1引数は入力ベクトル。
    LPointer input_ptr = caller->Evaluate(*(args_ptr->car()));
    Lisp::CheckList(*input_ptr);

    // 入力ベクトルの数をチェック。
    unsigned int num_input = Lisp::CountList(*input_ptr);
    if (num_input != weight_vec_.size()) {
      throw Lisp::GenError("@function-error",
      "Not equal number of elements of weights and inputs.");
    }

    return Lisp::NewNumber(LMath::operator*(weight_vec_,
    LMath::ListToMathVec(*input_ptr)));
  }

  // %%% @get-weights
  DEF_LC_FUNCTION(LPA2::GetWeights) {
    return LMath::MathVecToList(weight_vec_);
  }

  namespace LMath {
    // リストを行列へ。
    Mat ListToMatrix(const LObject& list) {
      Lisp::CheckList(list);
      int dim = Lisp::CountList(list);
      Mat matrix = GenMatrix(dim, dim);

      const LObject* ptr_1 = &list;
      for (int i = 0; i < dim; ++i, ptr_1 = ptr_1->cdr().get()) {
        const LPointer& vector_list = ptr_1->car();
        Lisp::CheckList(*vector_list);
        int dim_2 = Lisp::CountList(*vector_list);
        if (dim_2 != dim) {
          throw Lisp::GenError("@function-error", "'" + vector_list->ToString()
          + "' doesn't have " + std::to_string(dim) + " elements.");
        }

        const LObject* ptr_2 = vector_list.get();
        for (int j = 0; j < dim; ++j, ptr_2 = ptr_2->cdr().get()) {
          const LPointer& scalar_ptr = ptr_2->car();
          Lisp::CheckType(*scalar_ptr, LType::NUMBER);
          matrix[i][j] = scalar_ptr->number();
        }
      }

      return matrix;
    }

    // 最大の固有値固有ベクトルを求める。
    std::tuple<double, Vec> Eigen(const Mat& mat) {
      unsigned int size = mat.size();
      Vec vec(size, 0.0);
      vec[0] = 1.0;
      double lambda_old = 0.0, lambda = 0.0;
      double diff_old = std::numeric_limits<double>::max(), diff = 0.0;
      Vec result(size, 0.0);

      unsigned int count = 0;
      do {
        result = mat * vec;
        lambda = result * vec;
        vec = (1 / std::sqrt(result * result)) * result;

        // 収束判定。
        diff = std::fabs(lambda - lambda_old);
        if (diff <= 0.0) break;
        if (diff == diff_old) break;  // 誤差で無限ループをしている。

        lambda_old = lambda;
        diff_old = diff;
        ++count;
      } while (count < 1000);

      // 収束しなかった。
      if (count >= 1000) return make_tuple(0.0, Vec(size, 0.0));

      return make_tuple(lambda, vec);
    }

    // 逆行列を求める。
    Mat Inverse(const Mat& mat) {
      unsigned int size_1 = mat.size();
      unsigned int size_2 = mat[0].size();
      Mat copy(mat);

      // 単位行列を作る。
      Mat ret = GenMatrix(size_1, size_2);
      for (unsigned int i = 0; i < size_1; ++i) {
        for (unsigned int j = 0; j < size_2; ++j) {
          if (i == j) ret[i][j] = 1.0;
          else ret[i][j] = 0.0;
        }
      }

      // ピボットする関数。
      auto pivot =
      [&copy, &ret, size_1](unsigned int i, unsigned int j) {
        for (; i < (size_1 - 1); ++i) {
          std::swap(copy[i], copy[i + 1]);
          std::swap(ret[i], ret[i + 1]);
        }
      };

      // ピボット位置を1にする関数。
      auto pivot_to_one =
      [&copy, &ret](unsigned int i, unsigned int j) {
        double num = 1 / copy[i][j];
        copy[i] = num * copy[i];
        ret[i] = num * ret[i];
      };

      // 指定行のピボット列を0にする関数。
      auto to_zero_by_pivot =
      [&copy, &ret](unsigned int i, unsigned int j, unsigned int row) {
        double num = copy[row][j];
        copy[row] = copy[row] - (num * copy[i]);
        ret[row] = ret[row] - (num * ret[i]);
      };

      // 掃き出し法開始。
      for (unsigned int i = 0, j = 0; (i < size_1) && (j < size_2);
      ++i, ++j) {
        // ピボットのチェック。
        for (unsigned int count = size_1 - 1 - i;
        (copy[i][j] == 0.0) && (count > 0); --count) {
          pivot(i, j);
        }
        if (copy[i][j] == 0.0) return Mat(0);  // ピボットがこれ以上できない。

        // 割る。
        pivot_to_one(i, j);

        // 掛けて引く。
        for (unsigned int row = 0; row < size_1; ++row) {
          if (row != i) to_zero_by_pivot(i, j, row);
        }
      }

      return ret;
    }

    // 行列式。
    double Determinant(const Mat& mat) {
      unsigned int size_1 = mat.size();
      unsigned int size_2 = mat[0].size();
      Mat copy(mat);

      // ピボットする関数。
      auto pivot =
      [&copy, size_1](unsigned int i, unsigned int j) -> double {
        double sign = 1.0;
        for (; i < (size_1 - 1); ++i) {
          std::swap(copy[i], copy[i + 1]);
          sign *= -1.0;
        }
        return sign;
      };

      // ピボット位置を1にする関数。
      auto pivot_to_one =
      [&copy](unsigned int i, unsigned int j) -> double {
        double num = copy[i][j];
        copy[i] = (1 / num) * copy[i];
        return num;
      };

      // 指定行のピボット列を0にする関数。
      auto to_zero_by_pivot =
      [&copy](unsigned int i, unsigned int j, unsigned int row) {
        double num = copy[row][j];
        copy[row] = copy[row] - (num * copy[i]);
      };

      // 掃き出し法開始。
      double ret = 1.0;
      for (unsigned int i = 0, j = 0; (i < size_1) && (j < size_2);
      ++i, ++j) {
        // ピボットのチェック。
        for (unsigned int count = size_1 - 1 - i;
        (copy[i][j] == 0.0) && (count > 0); --count) {
          ret *= pivot(i, j);
        }
        if (copy[i][j] == 0.0) return 0.0;  // ピボットがこれ以上できない。

        // 割る。
        ret *= pivot_to_one(i, j);

        // 掛けて引く。
        for (unsigned int row = 0; row < size_1; ++row) {
          if (row != i) to_zero_by_pivot(i, j, row);
        }
      }

      return ret;
    }
  }  // namespace LMath
}  // namespace Sayuri
