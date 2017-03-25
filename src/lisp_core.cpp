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

  // 式を評価する。 LFunction版。
  LPointer LFunction::Evaluate(const LObject& target) {
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

  // 式を評価する。 LN_Function版。
  LPointer LN_Function::Evaluate(const LObject& target) {
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
    LPointerVec* expression_ptr = &expression_;
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
      expression_ptr = &expression;
    }

    // 関数呼び出し。
    LPointer ret_ptr = Lisp::NewNil();
    for (auto& expr : *expression_ptr) {
      ret_ptr = Evaluate(*expr);
    }

    if (!ret_ptr) {
      throw Lisp::GenError("@apply-error",
      "Failed to execute '" + args.car()->ToString() + "'.");
    }

    // ローカルスコープを捨てて終わる。
    scope_chain_.pop_back();
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

    func = LC_FUNCTION_OBJ(Eval);
    INSERT_LC_FUNCTION(func, "eval", "Lisp:eval");

    func = LC_FUNCTION_OBJ(ParseFunc);
    INSERT_LC_FUNCTION(func, "parse", "Lisp:parse");
    INSERT_LC_FUNCTION(func, "string->symbol", "Lisp:string->symbol");
    INSERT_LC_FUNCTION(func, "string->number", "Lisp:string->number");
    INSERT_LC_FUNCTION(func, "string->boolean", "Lisp:string->boolean");
    INSERT_LC_FUNCTION(func, "string->list", "Lisp:string->list");

    func = LC_FUNCTION_OBJ(Parval);
    INSERT_LC_FUNCTION(func, "parval", "Lisp:parval");

    func = LC_FUNCTION_OBJ(ToStringFunc);
    INSERT_LC_FUNCTION(func, "to-string", "Lisp:to-string");
    INSERT_LC_FUNCTION(func, "symbol->string", "Lisp:symbol->string");
    INSERT_LC_FUNCTION(func, "number->string", "Lisp:number->string");
    INSERT_LC_FUNCTION(func, "boolean->string", "Lisp:boolean->string");
    INSERT_LC_FUNCTION(func, "list->string", "Lisp:list->string");

    func = LC_FUNCTION_OBJ(Try);
    INSERT_LC_FUNCTION(func, "try", "Lisp:try");

    func = LC_FUNCTION_OBJ(Throw);
    INSERT_LC_FUNCTION(func, "throw", "Lisp:throw");

    func = LC_FUNCTION_OBJ(CarFunc);
    INSERT_LC_FUNCTION(func, "car", "Lisp:car");

    func = LC_FUNCTION_OBJ(CdrFunc);
    INSERT_LC_FUNCTION(func, "cdr", "Lisp:cdr");

    func = LC_FUNCTION_OBJ(Cons);
    INSERT_LC_FUNCTION(func, "cons", "Lisp:cons");

    // cxrを登録。
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
          for (int _3rd = 0; _3rd < 2; ++_3rd) {
            oss << ad[_3rd] <<  ad[_2nd] << ad[_1st];
            path = oss.str();
            oss.str("");
            func = [this, path](LPointer self, LObject* caller,
            const LObject& args) -> LPointer {
              return this->CxrFunc(path, self, caller, args);
            };
            INSERT_LC_FUNCTION(func, "c" + path + "r", "Lisp:c" + path + "r");
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
            }
          }
        }
      }
    }

    func = LC_FUNCTION_OBJ(ApplyFunc);
    INSERT_LC_FUNCTION(func, "apply", "Lisp:apply");

    func = LC_FUNCTION_OBJ(WalkFunc);
    INSERT_LC_FUNCTION(func, "walk", "Lisp:walk");

    func = LC_FUNCTION_OBJ(Quote);
    INSERT_LC_FUNCTION(func, "quote", "Lisp:quote");

    func = LC_FUNCTION_OBJ(Backquote);
    INSERT_LC_FUNCTION(func, "backquote", "Lisp:backquote");

    func = LC_FUNCTION_OBJ(Lambda);
    INSERT_LC_FUNCTION(func, "lambda", "Lisp:lambda");

    func = LC_FUNCTION_OBJ(FuncToLambda);
    INSERT_LC_FUNCTION(func, "func->lambda", "Lisp:func->lambda");

    func = LC_FUNCTION_OBJ(Let);
    INSERT_LC_FUNCTION(func, "let", "Lisp:let");

    func = LC_FUNCTION_OBJ(While);
    INSERT_LC_FUNCTION(func, "while", "Lisp:while");

    func = LC_FUNCTION_OBJ(For);
    INSERT_LC_FUNCTION(func, "for", "Lisp:for");

    func = LC_FUNCTION_OBJ(Define);
    INSERT_LC_FUNCTION(func, "define", "Lisp:define");

    func = LC_FUNCTION_OBJ(DefineMacro);
    INSERT_LC_FUNCTION(func, "define-macro", "Lisp:define-macro");

    func = LC_FUNCTION_OBJ(Set);
    INSERT_LC_FUNCTION(func, "set!", "Lisp:set!");

    func = LC_FUNCTION_OBJ(If);
    INSERT_LC_FUNCTION(func, "if", "Lisp:if");

    func = LC_FUNCTION_OBJ(Cond);
    INSERT_LC_FUNCTION(func, "cond", "Lisp:cond");

    func = LC_FUNCTION_OBJ(Begin);
    INSERT_LC_FUNCTION(func, "begin", "Lisp:begin");

    func = LC_FUNCTION_OBJ(GenScope);
    INSERT_LC_FUNCTION(func, "gen-scope", "Lisp:gen-scope");

    func = LC_FUNCTION_OBJ(Display);
    INSERT_LC_FUNCTION(func, "display", "Lisp:display");
    INSERT_LC_FUNCTION(func, "print", "Lisp:print");

    func = LC_FUNCTION_OBJ(Stdin);
    INSERT_LC_FUNCTION(func, "stdin", "Lisp:stdin");

    func = LC_FUNCTION_OBJ(Stdout);
    INSERT_LC_FUNCTION(func, "stdout", "Lisp:stdout");

    func = LC_FUNCTION_OBJ(Stderr);
    INSERT_LC_FUNCTION(func, "stderr", "Lisp:stderr");

    func = LC_FUNCTION_OBJ(Import);
    INSERT_LC_FUNCTION(func, "import", "Lisp:import");

    func = LC_FUNCTION_OBJ(Export);
    INSERT_LC_FUNCTION(func, "export", "Lisp:export");

    func = LC_FUNCTION_OBJ(EqualQ);
    INSERT_LC_FUNCTION(func, "equal?", "Lisp:equal?");

    func = LC_FUNCTION_OBJ(QFunc<LType::NIL>);
    INSERT_LC_FUNCTION(func, "nil?", "Lisp:nil?");
    INSERT_LC_FUNCTION(func, "null?", "Lisp:null?");

    func = LC_FUNCTION_OBJ(QFunc<LType::PAIR>);
    INSERT_LC_FUNCTION(func, "pair?", "Lisp:pair?");

    func = LC_FUNCTION_OBJ(QFunc<LType::SYMBOL>);
    INSERT_LC_FUNCTION(func, "symbol?", "Lisp:symbol?");

    func = LC_FUNCTION_OBJ(QFunc<LType::NUMBER>);
    INSERT_LC_FUNCTION(func, "number?", "Lisp:number?");

    func = LC_FUNCTION_OBJ(QFunc<LType::BOOLEAN>);
    INSERT_LC_FUNCTION(func, "boolean?", "Lisp:boolean?");

    func = LC_FUNCTION_OBJ(QFunc<LType::STRING>);
    INSERT_LC_FUNCTION(func, "string?", "Lisp:string?");

    func = LC_FUNCTION_OBJ(QFunc<LType::FUNCTION>);
    INSERT_LC_FUNCTION(func, "function?", "Lisp:function?");

    func = LC_FUNCTION_OBJ(QFunc<LType::N_FUNCTION>);
    INSERT_LC_FUNCTION(func, "native-function?", "Lisp:native-function?");

    func = LC_FUNCTION_OBJ(ProcedureQ);
    INSERT_LC_FUNCTION(func, "procedure?", "Lisp:procedure?");

    func = LC_FUNCTION_OBJ(OutputStream);
    INSERT_LC_FUNCTION(func, "output-stream", "Lisp:output-stream");

    func = LC_FUNCTION_OBJ(InputStream);
    INSERT_LC_FUNCTION(func, "input-stream", "Lisp:input-stream");

    func = LC_FUNCTION_OBJ(GenThread);
    INSERT_LC_FUNCTION(func, "gen-thread", "Lisp:gen-thread");

    func = LC_FUNCTION_OBJ(Sleep);
    INSERT_LC_FUNCTION(func, "sleep", "Lisp:sleep");

    func = LC_FUNCTION_OBJ(GenMutex);
    INSERT_LC_FUNCTION(func, "gen-mutex", "Lisp:gen-mutex");

    func = LC_FUNCTION_OBJ(System);
    INSERT_LC_FUNCTION(func, "system", "Lisp:system");

    func = LC_FUNCTION_OBJ(GetEnv);
    INSERT_LC_FUNCTION(func, "get-env", "Lisp:get-env");
  }

  // 基本関数を登録する。
  void Lisp::SetBasicFunctions() {
    LC_Function func;

    func = LC_FUNCTION_OBJ(Append);
    INSERT_LC_FUNCTION(func, "append", "Lisp:append");
    INSERT_LC_FUNCTION(func, "string-append", "Lisp:string-append");

    func = LC_FUNCTION_OBJ(Reverse);
    INSERT_LC_FUNCTION(func, "reverse", "Lisp:reverse");

    func = LC_FUNCTION_OBJ(Ref);
    INSERT_LC_FUNCTION(func, "ref", "Lisp:ref");
    INSERT_LC_FUNCTION(func, "list-ref", "Lisp:list-ref");
    INSERT_LC_FUNCTION(func, "string-ref", "Lisp:string-ref");

    func = LC_FUNCTION_OBJ(List);
    INSERT_LC_FUNCTION(func, "list", "Lisp:list");

    func = LC_FUNCTION_OBJ(ListReplace);
    INSERT_LC_FUNCTION(func, "list-replace", "Lisp:list-replace");

    func = LC_FUNCTION_OBJ(ListRemove);
    INSERT_LC_FUNCTION(func, "list-remove", "Lisp:list-remove");

    func = LC_FUNCTION_OBJ(ListInsert);
    INSERT_LC_FUNCTION(func, "list-insert", "Lisp:list-insert");

    func = LC_FUNCTION_OBJ(ListSearch);
    INSERT_LC_FUNCTION(func, "list-search", "Lisp:list-search");

    func = LC_FUNCTION_OBJ(ListPath);
    INSERT_LC_FUNCTION(func, "list-path", "Lisp:list-path");

    func = LC_FUNCTION_OBJ(ListPathReplace);
    INSERT_LC_FUNCTION(func, "list-path-replace", "Lisp:list-path-replace");

    func = LC_FUNCTION_OBJ(ListSort);
    INSERT_LC_FUNCTION(func, "list-sort", "Lisp:list-sort");

    func = LC_FUNCTION_OBJ(Zip);
    INSERT_LC_FUNCTION(func, "zip", "Lisp:zip");

    func = LC_FUNCTION_OBJ(Map);
    INSERT_LC_FUNCTION(func, "map", "Lisp:map");

    func = LC_FUNCTION_OBJ(Filter);
    INSERT_LC_FUNCTION(func, "filter", "Lisp:filter");

    func = LC_FUNCTION_OBJ(Range);
    INSERT_LC_FUNCTION(func, "range", "Lisp:range");

    func = LC_FUNCTION_OBJ(StartSizeInc);
    INSERT_LC_FUNCTION(func, "start-size-inc", "Lisp:start-size-inc");

    func = LC_FUNCTION_OBJ(LengthFunc);
    INSERT_LC_FUNCTION(func, "length", "Lisp:length");

    func = LC_FUNCTION_OBJ(NumEqual);
    INSERT_LC_FUNCTION(func, "=", "Lisp:=");

    func = LC_FUNCTION_OBJ(NumNotEqual);
    INSERT_LC_FUNCTION(func, "~=", "Lisp:~=");

    func = LC_FUNCTION_OBJ(NumGT);
    INSERT_LC_FUNCTION(func, ">", "Lisp:>");

    func = LC_FUNCTION_OBJ(NumGE);
    INSERT_LC_FUNCTION(func, ">=", "Lisp:>=");

    func = LC_FUNCTION_OBJ(NumLT);
    INSERT_LC_FUNCTION(func, "<", "Lisp:<");

    func = LC_FUNCTION_OBJ(NumLE);
    INSERT_LC_FUNCTION(func, "<=", "Lisp:<=");

    func = LC_FUNCTION_OBJ(EvenQ);
    INSERT_LC_FUNCTION(func, "even?", "Lisp:even?");

    func = LC_FUNCTION_OBJ(OddQ);
    INSERT_LC_FUNCTION(func, "odd?", "Lisp:odd?");

    func = LC_FUNCTION_OBJ(Not);
    INSERT_LC_FUNCTION(func, "not", "Lisp:not");

    func = LC_FUNCTION_OBJ(And);
    INSERT_LC_FUNCTION(func, "and", "Lisp:and");

    func = LC_FUNCTION_OBJ(Or);
    INSERT_LC_FUNCTION(func, "or", "Lisp:or");

    func = LC_FUNCTION_OBJ(Addition);
    INSERT_LC_FUNCTION(func, "+", "Lisp:+");

    func = LC_FUNCTION_OBJ(AdditionEx);
    INSERT_LC_FUNCTION(func, "add!", "Lisp:add!");

    func = LC_FUNCTION_OBJ(Subtraction);
    INSERT_LC_FUNCTION(func, "-", "Lisp:-");

    func = LC_FUNCTION_OBJ(SubtractionEx);
    INSERT_LC_FUNCTION(func, "sub!", "Lisp:sub!");

    func = LC_FUNCTION_OBJ(Multiplication);
    INSERT_LC_FUNCTION(func, "*", "Lisp:*");

    func = LC_FUNCTION_OBJ(MultiplicationEx);
    INSERT_LC_FUNCTION(func, "mul!", "Lisp:mul!");

    func = LC_FUNCTION_OBJ(Division);
    INSERT_LC_FUNCTION(func, "/", "Lisp:/");

    func = LC_FUNCTION_OBJ(DivisionEx);
    INSERT_LC_FUNCTION(func, "div!", "Lisp:div!");

    func = LC_FUNCTION_OBJ(Inc);
    INSERT_LC_FUNCTION(func, "++", "Lisp:++");

    func = LC_FUNCTION_OBJ(IncEx);
    INSERT_LC_FUNCTION(func, "inc!", "Lisp:inc!");

    func = LC_FUNCTION_OBJ(Dec);
    INSERT_LC_FUNCTION(func, "--", "Lisp:--");

    func = LC_FUNCTION_OBJ(DecEx);
    INSERT_LC_FUNCTION(func, "dec!", "Lisp:dec!");

    func = LC_FUNCTION_OBJ(StringSplit);
    INSERT_LC_FUNCTION(func, "string-split", "Lisp:string-split");

    func = LC_FUNCTION_OBJ(StringJoin);
    INSERT_LC_FUNCTION(func, "string-join", "Lisp:string-join");

    func = LC_FUNCTION_OBJ(Front);
    INSERT_LC_FUNCTION(func, "front", "Lisp:front");

    func = LC_FUNCTION_OBJ(Back);
    INSERT_LC_FUNCTION(func, "back", "Lisp:back");

    func = LC_FUNCTION_OBJ(PushFront);
    INSERT_LC_FUNCTION(func, "push-front", "Lisp:push-front");

    func = LC_FUNCTION_OBJ(PushFrontEx);
    INSERT_LC_FUNCTION(func, "push-front!", "Lisp:push-front!");

    func = LC_FUNCTION_OBJ(PopFront);
    INSERT_LC_FUNCTION(func, "pop-front", "Lisp:pop-front");

    func = LC_FUNCTION_OBJ(PopFrontEx);
    INSERT_LC_FUNCTION(func, "pop-front!", "Lisp:pop-front!");

    func = LC_FUNCTION_OBJ(PushBack);
    INSERT_LC_FUNCTION(func, "push-back", "Lisp:push-back");

    func = LC_FUNCTION_OBJ(PushBackEx);
    INSERT_LC_FUNCTION(func, "push-back!", "Lisp:push-back!");

    func = LC_FUNCTION_OBJ(PopBack);
    INSERT_LC_FUNCTION(func, "pop-back", "Lisp:pop-back");

    func = LC_FUNCTION_OBJ(PopBackEx);
    INSERT_LC_FUNCTION(func, "pop-back!", "Lisp:pop-back!");

    scope_chain_.InsertSymbol("PI", NewNumber(4.0 * std::atan(1.0)));

    scope_chain_.InsertSymbol("E", NewNumber(std::exp(1.0)));

    func = LC_FUNCTION_OBJ(Sin);
    INSERT_LC_FUNCTION(func, "sin", "Lisp:sin");

    func = LC_FUNCTION_OBJ(Cos);
    INSERT_LC_FUNCTION(func, "cos", "Lisp:cos");

    func = LC_FUNCTION_OBJ(Tan);
    INSERT_LC_FUNCTION(func, "tan", "Lisp:tan");

    func = LC_FUNCTION_OBJ(ASin);
    INSERT_LC_FUNCTION(func, "asin", "Lisp:asin");

    func = LC_FUNCTION_OBJ(ACos);
    INSERT_LC_FUNCTION(func, "acos", "Lisp:acos");

    func = LC_FUNCTION_OBJ(ATan);
    INSERT_LC_FUNCTION(func, "atan", "Lisp:atan");

    func = LC_FUNCTION_OBJ(Sqrt);
    INSERT_LC_FUNCTION(func, "sqrt", "Lisp:sqrt");

    func = LC_FUNCTION_OBJ(Abs);
    INSERT_LC_FUNCTION(func, "abs", "Lisp:abs");

    func = LC_FUNCTION_OBJ(Ceil);
    INSERT_LC_FUNCTION(func, "ceil", "Lisp:ceil");

    func = LC_FUNCTION_OBJ(Floor);
    INSERT_LC_FUNCTION(func, "floor", "Lisp:floor");

    func = LC_FUNCTION_OBJ(Round);
    INSERT_LC_FUNCTION(func, "round", "Lisp:round");

    func = LC_FUNCTION_OBJ(Trunc);
    INSERT_LC_FUNCTION(func, "trunc", "Lisp:trunc");

    func = LC_FUNCTION_OBJ(Exp2);
    INSERT_LC_FUNCTION(func, "exp2", "Lisp:exp2");

    func = LC_FUNCTION_OBJ(Exp);
    INSERT_LC_FUNCTION(func, "exp", "Lisp:exp");

    func = LC_FUNCTION_OBJ(Expt);
    INSERT_LC_FUNCTION(func, "expt", "Lisp:expt");

    func = LC_FUNCTION_OBJ(Log);
    INSERT_LC_FUNCTION(func, "log", "Lisp:log");

    func = LC_FUNCTION_OBJ(Log2);
    INSERT_LC_FUNCTION(func, "log2", "Lisp:log2");

    func = LC_FUNCTION_OBJ(Log10);
    INSERT_LC_FUNCTION(func, "log10", "Lisp:log10");

    std::shared_ptr<std::mt19937>
    engine_ptr(new std::mt19937(std::chrono::system_clock::to_time_t
    (std::chrono::system_clock::now())));
    func = [this, engine_ptr]
    (LPointer self, LObject* caller, const LObject& args) -> LPointer {
      return this->Random(*engine_ptr, self, caller, args);
    };
    scope_chain_.InsertSymbol("random",
    NewN_Function(func, "Lisp:random", scope_chain_));

    func = LC_FUNCTION_OBJ(Max);
    INSERT_LC_FUNCTION(func, "max", "Lisp:max");

    func = LC_FUNCTION_OBJ(Min);
    INSERT_LC_FUNCTION(func, "min", "Lisp:min");

    func = LC_FUNCTION_OBJ(RegexSearch);
    INSERT_LC_FUNCTION(func, "regex-search", "Lisp:regex-search");

    func = LC_FUNCTION_OBJ(GenNabla);
    INSERT_LC_FUNCTION(func, "gen-nabla", "Lisp:gen-nabla");

    func = LC_FUNCTION_OBJ(Integral);
    INSERT_LC_FUNCTION(func, "integral", "Lisp:integral");

    func = LC_FUNCTION_OBJ(PowerMethod);
    INSERT_LC_FUNCTION(func, "power-method", "Lisp:power-method");

    func = LC_FUNCTION_OBJ(InverseMatrix);
    INSERT_LC_FUNCTION(func, "inverse-matrix", "Lisp:inverse-matrix");

    func = LC_FUNCTION_OBJ(TransposedMatrix);
    INSERT_LC_FUNCTION(func, "transposed-matrix", "Lisp:transposed-matrix");

    func = LC_FUNCTION_OBJ(Determinant);
    INSERT_LC_FUNCTION(func, "determinant", "Lisp:determinant");

    func = LC_FUNCTION_OBJ(Bayes);
    INSERT_LC_FUNCTION(func, "bayes", "Lisp:bayes");

    func = LC_FUNCTION_OBJ(LogitToProb);
    INSERT_LC_FUNCTION(func, "logit->prob", "Lisp:logit->prob");

    func = LC_FUNCTION_OBJ(ProbToLogit);
    INSERT_LC_FUNCTION(func, "prob->logit", "Lisp:prob->logit");

    func = LC_FUNCTION_OBJ(GenPA2);
    INSERT_LC_FUNCTION(func, "gen-pa2", "Lisp:gen-pa2");

    func = LC_FUNCTION_OBJ(GenAI);
    INSERT_LC_FUNCTION(func, "gen-ai", "Lisp:gen-ai");

    func = LC_FUNCTION_OBJ(RBFKernel);
    INSERT_LC_FUNCTION(func, "rbf-kernel", "Lisp:rbf-kernel");

    func = LC_FUNCTION_OBJ(Now);
    INSERT_LC_FUNCTION(func, "now", "Lisp:now");

    func = LC_FUNCTION_OBJ(Clock);
    INSERT_LC_FUNCTION(func, "clock", "Lisp:clock");
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
    // 関数名なら関数オブジェクトを得る。
    if (func_ptr->IsSymbol()) {
      func_ptr = caller->Evaluate(*func_ptr);
    }

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
    // 関数名なら関数オブジェクトを得る。
    if (func_ptr->IsSymbol()) {
      func_ptr = caller->Evaluate(*func_ptr);
    }

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
      LPointer result = func_pair_ptr->car()->Apply(caller, *func_pair_ptr);

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
      result = func_pair_ptr->car()->Apply(caller, *func_pair_ptr);

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
    // 関数名なら関数オブジェクトを得る。
    if (func_ptr->IsSymbol()) {
      func_ptr = caller->Evaluate(*func_ptr);
    }

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
    // 関数名なら関数オブジェクトを得る。
    if (func_ptr->IsSymbol()) {
      func_ptr = caller->Evaluate(*func_ptr);
    }

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
      result = func_ptr->Apply(caller, func_pair);
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
            result = func_expr->car()->Apply(caller, *func_expr);
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
      const LPointer& car = ptr->car();
      CheckType(*car, LType::FUNCTION);
      *event_itr = NewPair(car, NewPair(WrapQuote(NewNil()), NewNil()));
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
      const LPointer& car = ptr->car();
      CheckType(*car, LType::FUNCTION);
      *cond_itr = NewPair(car, NewPair(WrapQuote(NewNil()), NewNil()));
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
        result = event_vec[i]->car()->Apply(caller, *(event_vec[i]));

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
        result = cond_vec[i]->car()->Apply(caller, *(cond_vec[i]));

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
