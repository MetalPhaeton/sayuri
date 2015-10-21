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
 * @file lisp_core.cpp
 * @author Hironori Ishibashi
 * @brief Lispインタープリタの実装。 
 */

#include "lisp_core.h"

#include <iostream>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <set>
#include <queue>
#include <map>
#include <sstream>
#include <fstream>
#include <cmath>
#include <random>
#include <chrono>
#include <cstdlib>

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ============== //
  // LispObject関連 //
  // ============== //
  // 比較演算子。
  bool LispObject::operator==(const LispObject& obj) const {
    // タイプを比較。
    if (type_ != obj.type_) return false;

    // タイプ別比較。
    switch (type_) {
      case LispObjectType::PAIR:
        return (*car_ == *(obj.car_)) && (*cdr_ == *(obj.cdr_));
      case LispObjectType::NIL:
        return true;
      case LispObjectType::SYMBOL:
      case LispObjectType::STRING:
        return str_value_ == obj.str_value_;
      case LispObjectType::NUMBER:
        return number_value_ == obj.number_value_;
      case LispObjectType::BOOLEAN:
        return boolean_value_ == obj.boolean_value_;
      case LispObjectType::FUNCTION:
        {
          if (function_.arg_name_vec_ != obj.function_.arg_name_vec_) {
            return false;
          }
          std::size_t size = function_.def_vec_.size();
          if (size != obj.function_.def_vec_.size()) return false;
          for (std::size_t i = 0; i < size; ++i) {
            if (*(function_.def_vec_[i]) != *(obj.function_.def_vec_[i])) {
              return false;
            }
          }
          return true;
        }
      case LispObjectType::NATIVE_FUNCTION:
        throw Lisp::GenError("@runtime-error",
        "Native Function can't be compared,"
        " because there are many types of function.");
      default: break;
    }

    return false;  // ここには来ない。
  }
  // 自分の文字列表現。
  std::string LispObject::ToString() const {
    std::ostringstream oss;

    switch (type_) {
      case LispObjectType::PAIR:
        {
          oss << "(";

          // carを文字列へ。
          LispIterator itr {this};
          for (; itr; ++itr) {
            oss << itr->ToString() << " ";
          }

          // 最後のcdr。
          if (itr.current_->IsNil()) {
            std::string temp = oss.str();
            oss.str("");
            temp.pop_back();
            oss << temp;
          } else {
            oss << ". " + itr.current_->ToString();
          }

          oss << ")";
        }
        break;
      case LispObjectType::NIL: oss << "()"; break;
      case LispObjectType::SYMBOL: oss << str_value_; break;
      case LispObjectType::NUMBER:
        oss << DoubleToString(number_value_);
        break;
      case LispObjectType::BOOLEAN:
        oss << (boolean_value_ ? "#t" : "#f");
        break;
      case LispObjectType::STRING:
        oss << "\"";
        for (auto c : str_value_) {
          switch (c) {
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            case '\b': oss << "\\b"; break;
            case '\a': oss << "\\a"; break;
            case '\f': oss << "\\f"; break;
            case '\0': oss << "\\0"; break;
            case '\"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            default: oss << c; break;
          }
        }
        oss << "\"";
        break;
      case LispObjectType::FUNCTION:
        {
          oss << "(lambda (";

          // 引数を書き込む。
          for (auto& name : function_.arg_name_vec_) oss << name << " ";
          std::string temp = oss.str();
          oss.str("");
          temp.pop_back();
          oss << temp << ") ";

          // 定義を書き込む。
          for (auto& obj : function_.def_vec_) oss << obj->ToString();
          oss << ")";
        }
        break;
      case LispObjectType::NATIVE_FUNCTION:
        oss << ";; Native Function";
        break;
      default: break;
    }

    return oss.str();
  }

  // 自分のシンボルマップで評価する。
  LispObjectPtr LispObject::Evaluate(const LispObject& target) const {
    if (!(target.IsPair())) {  // ペアではない。
      if (target.IsSymbol()) {
        // シンボルの場合。
        LispObjectPtr ret_ptr = ReferSymbol(target.str_value_)->Clone();

        return ret_ptr;
      } else {
        // その他のAtomの場合、コピーを返す。
        return target.Clone();
      }
    } else {  // ペア。関数として処理。
      // 準備。
      LispIterator target_itr {&target};

      // 第1要素を評価して関数オブジェクトを得る。
      std::string func_name = target_itr->ToString();
      LispObjectPtr func_obj = Evaluate(*(target_itr++));

      if (func_obj->IsFunction()) {  // Function。
        // ローカルスコープを作成。
        func_obj->scope_chain_.push_back(SymbolMapPtr(new SymbolMap()));

        // 引数リストをシンボルマップにバインド。
        std::vector<std::string>::iterator arg_name_itr =
        func_obj->function_.arg_name_vec_.begin();
        LispObjectPtr arg_list =
        Lisp::NewList(target_itr.current_->Length());
        LispObject* arg_ptr = arg_list.get();
        for (; target_itr; ++target_itr) {
          // 引数を評価。
          LispObjectPtr result = Evaluate(*target_itr);

          // 引数リストに入れる。
          arg_ptr->car_ = result;
          arg_ptr = arg_ptr->cdr_.get();

          // 引数リストにバインド。
          if (arg_name_itr != func_obj->function_.arg_name_vec_.end()) {
            func_obj->BindSymbol(*arg_name_itr, result->Clone());
            ++arg_name_itr;
          }
        }
        // もし引数の名前が余っていれば、NILをバインドする。
        for (; arg_name_itr != func_obj->function_.arg_name_vec_.end();
        ++arg_name_itr) {
            func_obj->BindSymbol(*arg_name_itr, Lisp::NewNil());
        }
        // 引数リストを"#args"にバインドする。
        func_obj->BindSymbol("$@", arg_list);

        // 関数を評価。
        LispObjectPtr ret_ptr;
        for (auto& def : func_obj->function_.def_vec_) {
          // 評価。
          ret_ptr = func_obj->Evaluate(*def);
        }

        return ret_ptr;
      } else if (func_obj->IsNativeFunction()) { // NativeFunction。
        // ローカルスコープを作成。
        func_obj->scope_chain_.push_back(SymbolMapPtr(new SymbolMap()));
        return func_obj->native_function_(func_obj, *this, target);
      } else {
        // 関数オブジェクトじゃないので、エラー。
        static const std::string type_str[] {
          "Pair", "Nil.", "Symbol.", "Number.", "Boolean.", "String.",
          "Function.", "Native Function."
        };
        std::string error_symbol = "@not-procedure";

        std::string message =
        "'" + func_name + "' is not bound with Procedure."
        " This is " + type_str[static_cast<int>(func_obj->type_)];

        throw Lisp::GenError(error_symbol, message);
      } 
    }

    throw Lisp::GenError("@runtime-error",
    "Evaluate() couldn't evaluate " + target.ToString() + ".");
  }

  // ======== //
  // Lisp関連 //
  // ======== //
  // 字句解析する。
  void Lisp::Tokenize(const std::string& code) {
    // 空白文字。
    std::set<char> blank {' ', '\n', '\r', '\t', '\b', '\a', '\f', '\0'};
    // 開きカッコ。
    std::set<char> op_parenth {'(', '[', '{'};
    // 閉じカッコ。
    std::set<char> cl_parenth {')', '}', '}'};

    // token_queue_にプッシュする。
    auto push_token = [this]() {
      if (!(token_stream_.str().empty())) {
        // トークンキューに今までの文字をプッシュ。
        this->token_queue_.push(this->token_stream_.str());
        // ストリームをクリア。
        this->token_stream_.str("");
      }
    };

    // 一文字ずつ調べる。
    for (auto c : code) {
      if (in_comment_) {  // コメント中。
        if (c == '\n') in_comment_ = false;
      } else if (in_string_) {  // 文字列中。
        // 空白文字なら無視。 ただし、スペースは無視しない。
        if ((c != ' ') && (blank.find(c) != blank.end())) continue;

        if (in_escape_) {  // エスケープ文字中。
          in_escape_ = false;
          char esc[] {'\\', c, '\0'};
          token_queue_.push(esc);
        } else {
          if (c == '"') {  // 文字列終了。
            in_string_ = false;
            push_token(); token_queue_.push(std::string(1, c));
          } else if (c == '\\') { // バックスラッシュ。
            push_token();
            in_escape_ = true;
          } else {
            token_stream_ << c;
          }
        }
      } else { // コメント中でも文字列中でもない。
        if (blank.find(c) != blank.end()) {  // 空白文字。
          if (!(token_stream_.str().empty())) {
            token_queue_.push(token_stream_.str());
            token_stream_.str("");
          }
        } else if (op_parenth.find(c) != op_parenth.end()) {  // 開き括弧。
          push_token(); token_queue_.push(std::string(1, c));
          ++parentheses_;
        } else if (cl_parenth.find(c) != cl_parenth.end()) {  // 閉じ括弧。
          push_token(); token_queue_.push(std::string(1, c));
          --parentheses_;
          // マイナスになったら初期化してパースエラー。
          if (parentheses_ < 0) {
            Reset();
            throw GenError("@parse-error", "Wrong parentheses.");
          }
        } else if (c == ';') {  // コメント開始文字。
          push_token();
          in_comment_ = true;
        } else if (c == '"') {  // 文字列開始文字。
          push_token(); token_queue_.push(std::string(1, c));
          in_string_ = true;
        } else if (c == '\'') {  // (quote)の糖衣構文。
          push_token(); token_queue_.push(std::string(1, c));
        } else {  // それ以外。
          token_stream_ << c;
        }
      }
    }
    push_token();  // 最後のプッシュ。
  }

  // パーサの本体。
  void Lisp::ParseCore(LispObject& target) {
    if (token_queue_.empty()) return;

    // 最初の文字。
    std::string front = token_queue_.front();
    token_queue_.pop();

    if (front == "(") {  // リスト。
      LispObject* ptr = &target;
      while (!(token_queue_.empty())) {
        // 次の文字を調べる。
        front = token_queue_.front();

        if (front == ")") {
          // リスト終了。
          token_queue_.pop();
          break;
        } else if (front == ".") {
          // ドット対。
          // ドットを抜いてポインタへパース。
          token_queue_.pop();
          ParseCore(*ptr);
        } else {
          // それ意外。
          // 現在のポインタをPairにする。
          ptr->type_ = LispObjectType::PAIR;
          ptr->car_ = NewNil();
          ptr->cdr_ = NewNil();

          // carへパース。
          ParseCore(*(ptr->car_));

          // 次へ。
          ptr = ptr->cdr_.get();
        }
      }
    } else {  // アトム。
      if (front == "\"") {
        // String。
        target.type_ = LispObjectType::STRING;

        std::ostringstream oss;
        while (!(token_queue_.empty())) {
          front = token_queue_.front();
          token_queue_.pop();

          if (front == "\"") break;  // 文字列終了なら抜ける。

          if (front[0] == '\\') {  // エスケープシーケンスだった場合。
            switch (front[1]) {
              case 'n': oss << '\n'; break;  // 改行。
              case 'r': oss << '\r'; break;  // キャリッジリターン。
              case 't': oss << '\t'; break;  // タブ。
              case 'b': oss << '\b'; break;  // バックスペース。
              case 'f': oss << '\f'; break;  // 改ページ。
              case 'a': oss << '\a'; break;  // ベル。
              case '0': oss << '\0'; break;  // Null文字。
              default: oss << front[1]; break;  // その他。
            }
          } else {
            oss << front;
          }
        }

        target.str_value_ = oss.str();
      } else if ((front == "#t") || (front == "#T")){ // Boolean::true。
        target.type_ = LispObjectType::BOOLEAN;
        target.boolean_value_ = true;
      } else if ((front == "#f") || (front == "#F")){ // Boolean::false。
        target.type_ = LispObjectType::BOOLEAN;
        target.boolean_value_ = false;
      } else if (front == "'") { // quoteの糖衣構文。
        target.type_ = LispObjectType::PAIR;

        target.car_ = NewSymbol("quote");
        target.cdr_ = NewPair();

        // 第1引数(Cdr->Car)をパース。
        ParseCore(*(target.cdr_->car_));
      } else if ((front == ".") || (front == ")")) { // おかしなトークン。
        target.type_ = LispObjectType::NIL;
      } else {
        // 数字かどうかの判定。 最初が数字なら数字としてパースしてみる。
        char c = front[0];
        if ((front.size() >= 2) && ((c == '+') || (c == '-'))) c = front[1];

        if ((c >= '0') && (c <= '9')) {  // Number。
          try {
            target.type_ = LispObjectType::NUMBER;
            target.number_value_ = std::stod(front);
          } catch (...) { // エラーならSymbol。
            target.type_ = LispObjectType::SYMBOL;
            target.str_value_ = front;
          }
        } else {  // Symbol
          target.type_ = LispObjectType::SYMBOL;
          target.str_value_ = front;
        }
      }
    }
  }

  void Lisp::SetCoreFunctions() {
    // %%% help
    {
      auto func = [this](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();

        if (!list_itr) {
          // 引数がなければ一覧を文字列にする。
          std::ostringstream oss;
          for (auto& pair : this->help_) {
            oss << pair.second;
            oss << std::endl << std::endl;
            oss << "- - - - - - - - - - - - - - - - - - - - "
            "- - - - - - - - - - - - - - - - - - - -" << std::endl;
            oss << std::endl;
          }
          return NewString(oss.str());
        } else {
          // 引数があればその項目を文字列にする。
          LispObjectPtr first_ptr = caller.Evaluate(*list_itr);
          if (!(first_ptr->IsString())) {
            throw GenWrongTypeError
            (func_name, "String", std::vector<int> {1}, true);
          }

          HelpDict::iterator itr =
          this->help_.find(first_ptr->str_value_);
          if (itr != this->help_.end()) {
            return NewString(itr->second);
          }

          return NewString("Not found help of " + list_itr->ToString() + ".");
        }

        return NewNil();
      };
      AddNativeFunction(func, "help");
      help_["help"] =
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
    ;;
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
    }

    // %%% eval
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return caller.Evaluate(*(caller.Evaluate(*list_itr)));
      };
      AddNativeFunction(func, "eval");
      help_["eval"] =
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
    ;;
    ;; > (+ 1 2 3)
    ;; > 6)...";
    }

    // %%% parse
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsString())) {
          throw GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // パース。
        Lisp lisp;
        std::vector<LispObjectPtr> ret_vec = lisp.Parse(result->str_value_);

        // 結果を返す。
        if (ret_vec.empty()) return NewNil();
        return ret_vec[0];
      };
      LispObjectPtr func_ptr =
      NewNativeFunction(global_ptr_->scope_chain_, func);
      global_ptr_->BindSymbol("parse", func_ptr);
      global_ptr_->BindSymbol("string->symbol", func_ptr);
      global_ptr_->BindSymbol("string->number", func_ptr);
      global_ptr_->BindSymbol("string->boolean", func_ptr);
      global_ptr_->BindSymbol("string->list", func_ptr);
      std::string temp =
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
    ;;
    ;; > (1 2 3))...";
      help_["parse"] = temp;
      help_["string->symbol"] = temp;
      help_["string->number"] = temp;
      help_["string->boolean"] = temp;
      help_["string->list"] = temp;
    }

    // %%% parval
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsString())) {
          throw GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // パース。
        Lisp lisp;
        std::vector<LispObjectPtr> ret_vec = lisp.Parse(result->str_value_);

        // 評価して結果を返す。
        LispObjectPtr ret_ptr = NewNil();
        for (auto& ptr : ret_vec) {
          ret_ptr = caller.Evaluate(*ptr);
        }
        return ret_ptr;
      };
      AddNativeFunction(func, "parval");
      help_["parval"] =
R"...(### parval ###

<h6> Usage </h6>

* `(parse <S-Expression : String>)`

<h6> Description </h6>

* Parses and evaluates `<S-Expression>` and returns result.
    + It is similar to `(eval (parse <S-Expression>))`.

<h6> Example </h6>

    (parval "(display \"Hello\")(display \"World\")")
    
    ;; Output
    ;;
    ;; > Hello
    ;; > World)...";
    }

    // %%% to-string
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        
        LispObjectPtr result = caller.Evaluate(*list_itr);
        return NewString(result->ToString());
      };
      LispObjectPtr func_ptr =
      NewNativeFunction(global_ptr_->scope_chain_, func);
      global_ptr_->BindSymbol("to-string", func_ptr);
      global_ptr_->BindSymbol("symbol->string", func_ptr);
      global_ptr_->BindSymbol("number->string", func_ptr);
      global_ptr_->BindSymbol("boolean->string", func_ptr);
      global_ptr_->BindSymbol("list->string", func_ptr);
      std::string temp =
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
    ;;
    ;; > (1 2 3)
    ;;
    
    (display (string? (to-string '(1 2 3))))
    
    ;; Output
    ;;
    ;; > #t)...";
      help_["to-string"] = temp;
      help_["symbol->string"] = temp;
      help_["number->string"] = temp;
      help_["boolean->string"] = temp;
      help_["list->string"] = temp;
    }

    // %%% try
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第一引数を得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObject& first = *(list_itr++);
        if (!(first.IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, false);
        }

        LispObjectPtr ret_ptr = NewNil();
        try {
          // 第1引数をトライ。
          for (LispIterator first_itr {&first}; first_itr; ++first_itr) {
            ret_ptr = caller.Evaluate(*first_itr);
          }
        } catch (LispObjectPtr exception) {
          // 第2引数以降でキャッチ。
          if (!list_itr) {
            throw GenInsufficientArgumentsError
            (func_name, required_args, true, list.Length() - 1);
          }

          // 一時スコープに例外メッセージをバインド。
          LispObjectPtr scope_ptr = NewScopeObject(caller.scope_chain_);

          scope_ptr->BindSymbol("exception", exception);

          // 例外処理。
          for (; list_itr; ++list_itr) {
            ret_ptr = scope_ptr->Evaluate(*list_itr);
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "try");
      help_["try"] =
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
    ;;
    ;; > Error Occured!!
    
    (try ((+ 1 "Hello"))
         (display exception))
    
    ;; Output
    ;;
    ;; > (@not-number "The 2nd argument of (+) didn't return Number."))...";
    }

    // %%% throw
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        throw caller.Evaluate(*list_itr);
      };
      AddNativeFunction(func, "throw");
      help_["throw"] =
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
    ;;
    ;; > 123)...";
    }

    // %%% car
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsPair())) {
          throw GenWrongTypeError
          (func_name, "Pair", std::vector<int> {1}, true);
        }

        return result->car_->Clone();
      };
      AddNativeFunction(func, "car");
      help_["car"] =
R"...(### car ###

<h6> Usage </h6>

* `(car <Pair or List>)`

<h6> Description </h6>

* Returns Car value of `<Pair or List>`

<h6> Example </h6>

    (display (car '(111 . 222)))
    ;; Output
    ;;
    ;; > 111
    
    (display (car (list 111 222 333)))
    
    ;; Output
    ;;
    ;; > 111)...";
    }

    // %%% cdr
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsPair())) {
          throw GenWrongTypeError
          (func_name, "Pair", std::vector<int> {1}, true);
        }

        return result->cdr_->Clone();
      };
      AddNativeFunction(func, "cdr");
      help_["cdr"] =
R"...(### cdr ###

<h6> Usage </h6>

* `(cdr <Pair or List>)`

<h6> Description </h6>

* Returns Cdr value of `<Pair or List>`

<h6> Example </h6>

    (display (cdr '(111 . 222)))
    ;; Output
    ;;
    ;; > 222
    
    (display (cdr (list 111 222 333)))
    
    ;; Output
    ;;
    ;; > (222 333))...";
    }

    // %%% cons
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result_car = caller.Evaluate(*(list_itr++));
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result_cdr = caller.Evaluate(*list_itr);

        return NewPair(result_car, result_cdr);
      };
      AddNativeFunction(func, "cons");
      help_["cons"] =
R"...(### cons ###

<h6> Usage </h6>

* `(cons <Object 1> <Object 2>)`

<h6> Description </h6>

* Returns Pair. Car is `<Object 1>`, Cdr is `<Object 2>`.

<h6> Example </h6>


    (display (cons 111 222))
    
    ;; Output
    ;;
    ;; > (111 . 222)
    
    (display (cons 111 '(222 333)))
    
    ;; Output
    ;;
    ;; > (111 222 333)
    
    (display (cons 444 (cons 555 (cons 666 ()))))
    
    ;; Output
    ;;
    ;; > (444 555 666))...";
    }

    // %%% conval
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 先ず、cons。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result_car = caller.Evaluate(*(list_itr++));
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result_cdr = caller.Evaluate(*list_itr);

        // evalして返す。
        return caller.Evaluate(*(NewPair(result_car, result_cdr)));
      };
      AddNativeFunction(func, "conval");
      help_["conval"] =
R"...(### conval ###

<h6> Usage </h6>

* `(conval <Object 1> <Object 2>)`

<h6> Description </h6>

* Constructs Pair and evaluates it. (cons and eval -> conval)
  + `<Object 1>` is Car, `<Object 2>` is Cdr.
  + It is same as `(eval (cons <Object 1> <Object 2>))`.

<h6> Example </h6>

    (define a '(1 2 3))
    
    (display (conval + a))
    
    ;; Output
    ;; > 6)...";
    }

    // %%% quote
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        return list_itr->Clone();
      };
      AddNativeFunction(func, "quote");
      help_["quote"] =
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
    ;;
    ;; > (111 222 333)
    
    (display '(444 555 666))
    
    ;; Output
    ;;
    ;; > (444 555 666)
    
    (define x 123)
    (display x)
    (display 'x)
    
    ;; Output
    ;;
    ;; > 123
    ;; > Symbol: x)...";
    }

    // %%% define
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // マクロ定義と関数定義に分ける。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        if (list_itr->IsSymbol()) {  // マクロ定義。
          // バインドされるシンボル。
          std::string symbol = (list_itr++)->str_value_;

          // バインドする値。
          if (!list_itr) {
            throw GenInsufficientArgumentsError
            (func_name, required_args, true, list.Length() - 1);
          }
          LispObjectPtr value_ptr = caller.Evaluate(*list_itr);

          // 呼び出し元のスコープにバインド。
          caller.BindSymbol(symbol, value_ptr);

          // シンボルを返す。
          return NewSymbol(symbol);
        } else if (list_itr->IsList()) {  // 関数定義。
          LispIterator inner_itr {&(*(list_itr++))};

          // 第1引数内の第1要素は関数名。
          if (!(inner_itr->IsSymbol())) {
            throw GenWrongTypeError(func_name, "Symbol",
            std::vector<int> {1, 1}, false);
          }
          std::string def_func_name = (inner_itr++)->str_value_;

          // 第1引数内の第2要素以降は引数定義。
          std::vector<std::string> arg_name_vec;
          int index = 2;
          for (; inner_itr; ++inner_itr, ++index) {
            if (!(inner_itr->IsSymbol())) {
              throw GenWrongTypeError(func_name, "Symbol",
              std::vector<int> {1, index}, false);
            }
            arg_name_vec.push_back(inner_itr->str_value_);
          }

          // 関数定義をベクトルにする。
          std::vector<LispObjectPtr> def_vec;
          for (; list_itr; ++list_itr) {
            def_vec.push_back(list_itr->Clone());
          }

          // 関数オブジェクトを作成。
          LispObjectPtr func_obj =
          NewFunction(caller.scope_chain_, arg_name_vec, def_vec);

          // 呼び出し元のスコープにバインド。
          caller.BindSymbol(def_func_name, func_obj);

          return NewSymbol(def_func_name);
        }

        throw GenWrongTypeError(func_name, "List or Symbol",
        std::vector<int> {1}, false);
      };
      AddNativeFunction(func, "define");
      help_["define"] =
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

<h6> Example </h6>

    (define x 123)
    (display x)
    
    ;; Output
    ;;
    ;; > 123
    
    (define (myfunc x) (+ x 10))
    (display (myfunc 5))
    
    ;; Output
    ;;
    ;; > 15)...";
    }

    // %%% set!
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数。 シンボルでなくてはならない。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        if (!(list_itr->IsSymbol())) {
          throw GenWrongTypeError
          (func_name, "Symbol", std::vector<int> {1}, false);
        }
        std::string symbol = (list_itr++)->str_value_;

        // 書きかえる前の値を待避。 (シンボルが見つからなければエラー。)
        LispObjectPtr ret_ptr = caller.ReferSymbol(symbol)->Clone();

        // 第2引数を評価。
        LispObjectPtr value_ptr = caller.Evaluate(*list_itr);

        // シンボルを上書きする。
        caller.RewriteSymbol(symbol, value_ptr);

        // 書きかえる前を返す。
        return ret_ptr;
      };
      AddNativeFunction(func, "set!");
      help_["set!"] =
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
    ;;
    ;; > 456
    
    (define myfunc (let ((x 1)) (lambda () (set! x (+ x 1)) x)))
    (display (myfunc))
    (display (myfunc))
    (display (myfunc))
    
    ;; Output
    ;;
    ;; > 2
    ;; > 3
    ;; > 4)...";
    }

    // %%% lambda
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数はリスト。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        if (!(list_itr->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, false);
        }
        LispIterator first_itr {&(*(list_itr++))};

        // 引数リストを作成。
        int index = 1;
        std::vector<std::string> arg_name_vec;
        for (; first_itr; ++first_itr, ++index) {
          if (!(first_itr->IsSymbol())) {
            throw GenWrongTypeError
            (func_name, "Symbol", std::vector<int> {1, index}, false);
          }
          arg_name_vec.push_back(first_itr->str_value_);
        }

        // 関数定義をセット。
        std::vector<LispObjectPtr> def_vec;
        for (; list_itr; ++list_itr) {
          def_vec.push_back(list_itr->Clone());
        }

        return NewFunction(caller.scope_chain_, arg_name_vec, def_vec);
      };
      AddNativeFunction(func, "lambda");
      help_["lambda"] =
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

<h6> Example </h6>

    (define myfunc (lambda (x) (+ x 100)))
    (display (myfunc 5))
    
    ;; Output
    ;;
    ;; > 105
    
    (define gen-func (lambda (x) (lambda () (+ x 100))))
    (define myfunc2 (gen-func 50))
    (display (myfunc2))
    
    ;; Output
    ;;
    ;; > 150)...";
    }

    // %%% let
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数(ローカル変数定義リスト)を得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        if (!(list_itr->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, false);
        }
        LispIterator first_itr {&(*(list_itr++))};

        // 一時スコープを作成してバインド。
        int index = 1;
        LispObjectPtr scope_ptr = NewScopeObject(caller.scope_chain_);
        for (; first_itr; ++first_itr, ++index) {
          if (!(first_itr->IsList())) {
            throw GenWrongTypeError
            (func_name, "List", std::vector<int> {1, index}, false);
          }

          // 定義のペア。
          if (first_itr->IsPair()) {
            // 変数名。
            if (!(first_itr->car_->IsSymbol())) {
              throw GenWrongTypeError
              (func_name, "Symbol", std::vector<int> {1, index, 1}, false);
            }
            std::string var_name = first_itr->car_->str_value_;

            // 変数の初期値。
            LispObjectPtr value = NewNil();
            if (first_itr->cdr_->IsPair()) {
              value = caller.Evaluate(*(first_itr->cdr_->car_));
            }

            // バインド。
            scope_ptr->BindSymbol(var_name, value);
          }
        }

        // 第3引数以降を自分のスコープで評価する。
        LispObjectPtr ret_ptr;
        for (; list_itr; ++list_itr) {
          ret_ptr = scope_ptr->Evaluate(*list_itr);
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "let");
      help_["let"] =
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
    ;;
    ;; > 30)...";
    }

    // %%% while
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 引数が2つ以上あるかどうかを予め調べる。
        int len = list.Length();
        if (len < 3) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, len - 1);
        }

        // 一時スコープを作り、ループ開始。
        LispObjectPtr ret_ptr = NewNil();
        LispObjectPtr scope_ptr = NewScopeObject(caller.scope_chain_);
        while (true) {
          // 条件式へのポインタ。
          const LispObject* ptr = list_itr.current_;
          LispObjectPtr cond_ptr = caller.Evaluate(*(ptr->car_));
          if (!(cond_ptr->IsBoolean())) {
            throw GenWrongTypeError
            (func_name, "Boolean", std::vector<int> {1}, true);
          }

          // 条件が#fならループを抜ける。
          if (!(cond_ptr->boolean_value_)) break;

          // 自分のスコープで実行していく。
          for (; ptr->IsPair(); ptr = ptr->cdr_.get()) {
            ret_ptr = scope_ptr->Evaluate(*(ptr->car_));
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "while");
      help_["while"] =
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
    }

    // %%% for
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 要素用シンボルをローカルスコープにバインドする。
        // チェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        if (!(list_itr->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, false);
        }
        LispIterator first_itr {&(*(list_itr++))};
        if (!first_itr) {
          throw GenError("@insufficient-arguments",
          "No Symbol to bind element for loop.");
        }
        if (!(first_itr->IsSymbol())) {
          throw GenWrongTypeError
          (func_name, "Symbol", std::vector<int> {1, 1}, false);
        }
        // 一時スコープにバインド。
        LispObjectPtr scope_ptr = NewScopeObject(caller.scope_chain_);
        std::string symbol = first_itr->str_value_;
        scope_ptr->BindSymbol(symbol, NewNil());

        // ループ用リストをチェックする。
        ++first_itr;
        if (!first_itr) {
          throw GenError("@insufficient-arguments",
          "No List or String for loop.");
        }
        LispObjectPtr loop_list_ptr = caller.Evaluate(*first_itr);
        if (!(loop_list_ptr->IsList() || loop_list_ptr->IsString())) {
          throw GenWrongTypeError
          (func_name, "List or String", std::vector<int> {1, 2}, true);
        }

        // loop_list_ptrが文字列の場合、Listに変更する。
        if (loop_list_ptr->IsString()) {
          LispObjectPtr temp = NewNil();
          LispObject* ptr = temp.get();
          for (auto c : loop_list_ptr->str_value_) {
            *ptr = *(NewPair(NewString(std::string(1, c)), NewNil()));
            ptr = ptr->cdr_.get();
          }
          loop_list_ptr = temp;
        }

        // ループ開始。
        LispObjectPtr ret_ptr = NewNil();
        for (LispIterator itr {loop_list_ptr.get()}; itr; ++itr) {
          // ローカルスコープに要素をバインド。
          scope_ptr->RewriteSymbol(symbol, itr->Clone());

          // 自分のスコープで次々と実行する。
          for (LispIterator itr_2 {list_itr.current_}; itr_2; ++itr_2) {
            ret_ptr = scope_ptr->Evaluate(*itr_2);
          }
        }
        return ret_ptr;
      };
      AddNativeFunction(func, "for");
      help_["for"] =
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
    }

    // %%% if
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 3;

        // 第1引数を評価。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsBoolean())) {
          throw GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {1}, true);
        }

        // resultが#tの場合。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        if (result->boolean_value_) {
          // 第2引数を評価。
          return caller.Evaluate(*list_itr);
        }

        // resultが#fの場合。
        // 第3引数を評価。
        ++list_itr;
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        return caller.Evaluate(*list_itr);
      };
      AddNativeFunction(func, "if");
      help_["if"] =
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
    ;;
    ;; > 7)...";
    }

    // %%% cond
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();

        // 各条件リストを評価。
        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          // 各条件リストはリストでなくてはいけない。
          if (!(list_itr->IsList())) {
            throw GenWrongTypeError
            (func_name, "List", std::vector<int> {index}, false);
          }

          // 各条件リストの中身を評価。
          LispIterator cond_list_itr {&(*list_itr)};
          if (cond_list_itr) {
            // 空リストではない。
            // elseリストなら無条件で実行式を評価して終了。
            if (cond_list_itr->IsSymbol()
            && (cond_list_itr->str_value_ == "else")) {
              ++cond_list_itr;
              if (cond_list_itr) {
                return caller.Evaluate(*cond_list_itr);
              }
              return NewNil();
            }

            // 条件を評価する。
            LispObjectPtr result = caller.Evaluate(*(cond_list_itr++));
            if (!(result->IsBoolean())) {
              throw GenWrongTypeError
              (func_name, "Boolean", std::vector<int> {index, 1}, true);
            }

            // #tなら実行式を評価して終了。
            if (result->boolean_value_) {
              if (cond_list_itr) {
                return caller.Evaluate(*cond_list_itr);
              }
              return NewNil();
            }
          }
        }
        return NewNil();
      };
      AddNativeFunction(func, "cond");
      help_["cond"] =
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
    ;;
    ;; > World)...";
    }

    // %%% begin
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 各引数を評価。
        LispObjectPtr ret_ptr = NewNil();
        LispIterator list_itr {&list};
        for (++list_itr; list_itr; ++list_itr) {
          ret_ptr = caller.Evaluate(*list_itr);
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "begin");
      help_["begin"] =
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
    ;;
    ;; > Hello
    ;; > World
    ;; > World)...";
    }

    // %%% display
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        LispIterator list_itr {&list};
        std::ostringstream oss;
        for (++list_itr; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          switch (result->type_) {
            case LispObjectType::PAIR:
            case LispObjectType::NIL:
            case LispObjectType::NUMBER:
            case LispObjectType::BOOLEAN:
              oss << result->ToString();
              break;
            case LispObjectType::SYMBOL:
              oss << "Symbol:" << result->str_value_;
              break;
            case LispObjectType::STRING:
              oss << result->str_value_;
              break;
            case LispObjectType::FUNCTION:
              oss << "Function: " << result->ToString();
              break;
            case LispObjectType::NATIVE_FUNCTION:
              oss << "NativeFunction";
              break;
            default: break;
          }
        }

        // 表示。
        std::cout << oss.str() << std::endl;

        return NewString(oss.str());
      };
      AddNativeFunction(func, "display");
      help_["display"] =
R"...(### display ###

<h6> Usage </h6>

* `(display <Object>...)`

<h6> Description </h6>

* Prints `<Object>` on Standard Output.

<h6> Example </h6>

    (define x 123)
    (display x)
    
    ;; Output
    ;;
    ;; > 123
    
    (define x 123)
    (display "x is " x)
    
    ;; Output
    ;;
    ;; > x is 123)...";
    }

    // %%% stdin
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // ストリームが閉じていたらNilを返す。
        if (!(std::cin)) return NewNil();

        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数を調べる。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr message_ptr = caller.Evaluate(*(list_itr++));
        if (!(message_ptr->IsSymbol())) {
          throw GenWrongTypeError
          (func_name, "Symbol", std::vector<int> {1}, true);
        }

        // メッセージシンボルに合わせて読み方を変える。
        std::string message = message_ptr->str_value_;
        std::string input_str = "";
        if (message == "@read") {
          char c;
          while (std::cin.get(c)) input_str.push_back(c);
        } else if (message == "@read-line") {
          std::getline(std::cin, input_str);
        } else if (message == "@get") {
          char c;
          std::cin.get(c);
          input_str.push_back(c);
        }

        return NewString(input_str);
      };
      AddNativeFunction(func, "stdin");
      help_["stdin"] =
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
    }

    // %%% stdout
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr); 
        if (!(result->IsString())) {
          throw GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        std::cout << result->str_value_ << std::flush;

        return self;
      };
      AddNativeFunction(func, "stdout");
      help_["stdout"] =
R"...(### stdout ###

<h6> Usage </h6>

* `(stdout <String>)`

<h6> Description </h6>

* Prints `<String>` on Standard Output.

<h6> Example </h6>

    (stdout (to-string 123))
    (stdout "\n")
    
    ;; Output
    ;;
    ;; > 123)...";
    }

    // %%% stderr
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr); 
        if (!(result->IsString())) {
          throw GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        std::cerr << result->str_value_ << std::flush;

        return self;
      };
      AddNativeFunction(func, "stderr");
      help_["stderr"] =
R"...(### stderr ###

<h6> Usage </h6>

* `(stderr <String>)`

<h6> Description </h6>

* Prints `<String>` on Standard Error.

<h6> Example </h6>

    (stderr (to-string 123))
    (stderr "\n")
    
    ;; Output
    ;;
    ;; > 123)...";
    }

    // %%% import
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr); 
        if (!(result->IsString())) {
          throw GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        LispObjectPtr ret_ptr = NewNil();

        // 全部読み込む。
        std::string input = "";
        std::ifstream ifs(result->str_value_);
        if (!ifs) {
          throw GenError
          ("@runtime-error", "Couldn't open '" + result->str_value_ + "'.");
        }
        std::ostringstream osstream;
        osstream << ifs.rdbuf();

        // 字句解析する。
        Lisp lisp;
        std::vector<LispObjectPtr> obj_ptr_vec = lisp.Parse(osstream.str());

        // 評価する。
        for (auto& obj_ptr : obj_ptr_vec) {
          ret_ptr = caller.Evaluate(*obj_ptr);
        }

        // 最後の評価結果を返す。
        return ret_ptr;
      };
      AddNativeFunction(func, "import");
      help_["import"] =
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
    ;;
    ;; > Hello World
    :: > a: 111
    :: > b: 222)...";
    }

    // %%% equal?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr first_ptr = caller.Evaluate(*(list_itr++));

        // 比較。
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (*first_ptr != *result) return NewBoolean(false);
        }

        return NewBoolean(true);
      };
      LispObjectPtr func_ptr =
      NewNativeFunction(global_ptr_->scope_chain_, func);
      global_ptr_->BindSymbol("equal?", func_ptr);
      global_ptr_->BindSymbol("=", func_ptr);
      std::string temp =
R"...(### equal? ###

<h6> Usage </h6>

* `(equal? <Object>...)`
* `(= <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are same structure.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (equal? '(1 2 (3 4) 5) '(1 2 (3 4) 5)))
    
    ;; Output
    ;;
    ;; > #t)...";
      help_["equal?"] = temp;
      help_["="] = temp;
    }

    // %%% !=
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr first_ptr = caller.Evaluate(*(list_itr++));

        // 比較。
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (*first_ptr != *result) return NewBoolean(true);
        }

        return NewBoolean(false);
      };
      AddNativeFunction(func, "!=");
      help_["!="] =
R"...(### != ###

<h6> Usage </h6>

* `(!= <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are different structure.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (!= '(1 2 (3 4) 5) '(1 2 (3 4) 5)))
    
    ;; Output
    ;;
    ;; > #f)...";
    }

    // %%% pair?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数がLispPairならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsPair())) {
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "pair?");
      help_["pair?"] =
R"...(### pair? ###

<h6> Usage </h6>

* `(pair? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Pair.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (pair? '(1 2 3) '(4 5 6)))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% list?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数がListならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsList())) {
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "list?");
      help_["list?"] =
R"...(### list? ###

<h6> Usage </h6>

* `(list? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are List.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (list? '(1 2 3) '(4 5 6) ()))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% nil?
    // %%% null?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数がNilならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsNil())) {
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      LispObjectPtr func_ptr =
      NewNativeFunction(global_ptr_->scope_chain_, func);
      global_ptr_->BindSymbol("nil?", func_ptr);
      global_ptr_->BindSymbol("null?", func_ptr);
      std::string temp =
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
    ;;
    ;; > #t)...";
      help_["nil?"] = temp;
      help_["null?"] = temp;
    }

    // %%% symbol?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数がシンボルならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsSymbol())){
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "symbol?");
      help_["symbol?"] =
R"...(### symbol? ###

<h6> Usage </h6>

* `(symbol? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Symbol.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (symbol? 'x))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% number?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数が数字ならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsNumber())) {
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "number?");
      help_["number?"] =
R"...(### number? ###

<h6> Usage </h6>

* `(number? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Number.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (number? 123))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% boolean?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数がBooleanならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsBoolean())) {
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "boolean?");
      help_["boolean?"] =
R"...(### boolean? ###

<h6> Usage </h6>

* `(boolean? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Boolean.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (boolean? #f))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% string?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数が文字列ならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsString())) {
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "string?");
      help_["string?"] =
R"...(### string? ###

<h6> Usage </h6>

* `(string? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are String.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (string? "Hello"))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% function?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数が関数オブジェクトならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsFunction())) {
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "function?");
      help_["function?"] =
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
    ;;
    ;; > #t)...";
    }

    // %%% native-function?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数がネイティブ関数オブジェクトならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsNativeFunction())) {
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "native-function?");
      help_["native-function?"] =
R"...(### native-function? ###

<h6> Usage </h6>

* `(native-function? <Object>...)`

<h6> Description </h6>

* Returns #t if all `<Object>...` are Native Function.
  Otherwise, returns #f.

<h6> Example </h6>

    (display (native-function? +))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% procedure?
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = NewBoolean(true);

        // 全ての引数が関数、又はネイティブ関数オブジェクトならtrue。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!((result->IsFunction()) || (result->IsNativeFunction()))) {
            ret_ptr->boolean_value_ = false;
            break;
          }
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "procedure?");
      help_["procedure?"] =
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
    ;;
    ;; > #t
    
    (display (procedure? +))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% output-stream
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsString())) {
          throw GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // ヒープにファイルを開く。
        std::shared_ptr<std::ofstream>
        ofs_ptr(new std::ofstream(result->str_value_));
        if (!(*ofs_ptr)) {
          throw GenError("@not-open-stream", "Couldn't open output stream.");
        }

        // ネイティブ関数オブジェクトを作成。
        auto func = [ofs_ptr](LispObjectPtr self, const LispObject& caller,
        const LispObject& list) -> LispObjectPtr {
          // 準備。
          LispIterator list_itr {&list};
          std::string func_name = (list_itr++)->ToString();
          int required_args = 1;

          if (!list_itr) {
            throw GenInsufficientArgumentsError
            (func_name, required_args, false, list.Length() - 1);
          }
          LispObjectPtr result = caller.Evaluate(*list_itr);

          // Nilならファイルを閉じる。
          if (result->IsNil()) {
            if (*(ofs_ptr)) ofs_ptr->close();
            return self;
          }

          // ファイルを書き込む。
          if (!(result->IsString())) {
            throw GenWrongTypeError
            (func_name, "String or Nil", std::vector<int> {1}, true);
          }

          if (*ofs_ptr) {
            *ofs_ptr << result->str_value_ << std::flush;
          }

          return self;
        };

        return NewNativeFunction(caller.scope_chain_, func);
      };
      AddNativeFunction(func, "output-stream");
      help_["output-stream"] =
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
    }

    // %%% input-stream
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsString())) {
          throw GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // ヒープにファイルを開く。
        std::shared_ptr<std::ifstream>
        ifs_ptr(new std::ifstream(result->str_value_));
        if (!(*ifs_ptr)) {
          throw GenError("@not-open-stream", "Couldn't open input stream.");
        }

        // ネイティブ関数オブジェクトを作成。
        auto func = [ifs_ptr](LispObjectPtr self, const LispObject& caller,
        const LispObject& list) -> LispObjectPtr {
          // ストリームが閉じていたらNilを返す。
          if (!(*ifs_ptr)) return NewNil();

          // 準備。
          LispIterator list_itr {&list};
          std::string func_name = (list_itr++)->ToString();
          int required_args = 1;

          // 引数を調べる。
          if (!list_itr) {
            throw GenInsufficientArgumentsError
            (func_name, required_args, false, list.Length() - 1);
          }
          LispObjectPtr message_ptr = caller.Evaluate(*(list_itr++));
          if (message_ptr->IsNil()) {
            // Nilの場合は閉じる。
            ifs_ptr->close();
            return NewNil();
          }
          if (!(message_ptr->IsSymbol())) {
            throw GenWrongTypeError
            (func_name, "Symbol or Nil", std::vector<int> {1}, true);
          }

          // メッセージシンボルに合わせて読み方を変える。
          std::string message = message_ptr->str_value_;
          std::string input_str = "";
          if (message == "@read") {
            std::ostringstream oss;
            oss << ifs_ptr->rdbuf();
            input_str = oss.str();
          } else if (message == "@read-line") {
            std::getline(*ifs_ptr, input_str);
          } else if (message == "@get") {
            char c;
            ifs_ptr->get(c);
            input_str.push_back(c);
          }

          return NewString(input_str);
        };

        return NewNativeFunction(caller.scope_chain_, func);
      };
      AddNativeFunction(func, "input-stream");
      help_["input-stream"] =
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
    }
  }

  void Lisp::SetBasicFunctions() {
    // %%% append
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();

        // ペアと文字列で分岐。
        if (list_itr) {
          LispObjectPtr result = caller.Evaluate(*(list_itr++));

          if (result->IsList()) {
            // リストの場合。
            LispObject* ptr = result.get();
            for (; list_itr; ++list_itr) {
              for (; ptr->IsPair(); ptr = ptr->cdr_.get()) continue;
              if (!(ptr->IsNil())) break;
              *ptr = *(caller.Evaluate(*list_itr));
            }

            return result;
          } else if (result->IsString()) {
            // 文字列の場合。
            std::ostringstream oss;
            oss << result->str_value_;
            for (; list_itr; ++list_itr) {
              LispObjectPtr result_2 = caller.Evaluate(*list_itr);
              switch (result_2->type_) {
                case LispObjectType::STRING:
                  oss << result_2->str_value_;
                  break;
                case LispObjectType::SYMBOL:
                case LispObjectType::NUMBER:
                case LispObjectType::BOOLEAN:
                case LispObjectType::PAIR:
                  oss << result_2->ToString();
                  break;
                default: break;
              }
            }
            return NewString(oss.str());
          }
        }

        return NewNil();
      };
      LispObjectPtr func_ptr =
      NewNativeFunction(global_ptr_->scope_chain_, func);
      global_ptr_->BindSymbol("append", func_ptr);
      global_ptr_->BindSymbol("string-append", func_ptr);
      std::string temp =
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
    ;;
    ;; > (111 222 333 444 555 666 . 777)
    
    (display (append "Hello " 111 " World"))
    
    ;; Output
    ;;
    ;; > "Hello 111 World")...";
      help_["append"] = temp;
      help_["string-append"] = temp;
    }

    // %%% ref
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsList() || result->IsString())) {
          throw GenWrongTypeError
          (func_name, "List or String", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。 インデックス。 0が最初。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr index_ptr = caller.Evaluate(*list_itr);
        if (!(index_ptr->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        // 目的の要素を得る。
        if (result->IsList()) {
          // リストの場合。
          int index = index_ptr->number_value_;
          index = index < 0 ? result->Length() + index : index;
          int i = 0;
          for (LispIterator result_itr {result.get()};
          result_itr; ++result_itr, ++i) {
            if (i == index) return result_itr->Clone();
          }
        } else {
          // 文字列の場合。
          std::string result_str = result->str_value_;
          int index = index_ptr->number_value_;
          index = index < 0 ? result_str.size() + index : index;
          if (static_cast<unsigned int>(index) < result_str.size()) {
            std::string ret_str = "";
            ret_str.push_back(result->str_value_[index]);
            return NewString(ret_str);
          }
        }

        // 範囲外。
        throw GenError("@out-of-range",
        "The index number of (" + func_name + ") is out of range.");
      };
      LispObjectPtr func_ptr =
      NewNativeFunction(global_ptr_->scope_chain_, func);
      global_ptr_->BindSymbol("ref", func_ptr);
      global_ptr_->BindSymbol("list-ref", func_ptr);
      global_ptr_->BindSymbol("string-ref", func_ptr);
      std::string temp =
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
    ;;
    ;; > 222
    
    (display (ref '(111 222 333) -1))
    
    ;; Output
    ;;
    ;; > 333
    
    (display (ref "Hello World" 4))
    
    ;; Output
    ;;
    ;; > "o"
    
    (display (ref "Hello World" -3))
    
    ;; Output
    ;;
    ;; > "r")...";
      help_["ref"] = temp;
      help_["list-ref"] = temp;
      help_["string-ref"] = temp;
    }

    // %%% list
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();

        // 作成する大きさのリストを作りながらセットしていく。
        LispObjectPtr ret_ptr = NewNil();
        LispObject* ptr = ret_ptr.get();
        for (; list_itr; ++list_itr) {
          ptr->type(LispObjectType::PAIR);
          ptr->car_ = caller.Evaluate(*list_itr);
          ptr->cdr_ = NewNil();

          ptr = ptr->cdr_.get();
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "list");
      help_["list"] =
R"...(### list ###


<h6> Usage </h6>

* `(list <Object>...)`

<h6> Description </h6>

* Returns List composed of `<Object>...`.

<h6> Example </h6>

    (display (list 111 222 333))
    
    ;; Output
    ;;
    ;; > (111 222 333))...";
    }

    // %%% list-replace
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 3;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。 インデックス。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr index_ptr = caller.Evaluate(*(list_itr++));
        if (!(index_ptr->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        // 第3引数を得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr replace_obj_ptr = caller.Evaluate(*list_itr);

        // インデックスの要素を入れ替える。
        int index = index_ptr->number_value_;
        index = index < 0 ? result->Length() + index : index;
        int i = 0;
        for (LispIterator result_itr {result.get()};
        result_itr; ++result_itr, ++i) {
          if (i == index) {
            *result_itr = *replace_obj_ptr;
            return result;
          }
        }

        // 範囲外。
        throw GenError("@out-of-range",
        "The index number of (" + func_name + ") is out of range.");
      };
      AddNativeFunction(func, "list-replace");
      help_["list-replace"] =
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
    ;;
    ;; > (111 "Hello" 333))...";
    }

    // %%% list-remove
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。 インデックス。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr index_ptr = caller.Evaluate(*list_itr);
        if (!(index_ptr->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        // インデックスの要素を削除する。 後ろの要素を1つ前に持ってくる。
        int index = index_ptr->number_value_;
        index = index < 0 ? result->Length() + index : index;
        int i = 0;
        for (LispObject* ptr = result.get();
        ptr->IsPair(); ptr = ptr->cdr_.get(), ++i) {
          if (i == index) {
            *ptr = *(ptr->cdr_);
            return result;
          }
        }

        // 範囲外。
        throw GenError("@out-of-range",
        "The index number of (" + func_name + ") is out of range.");
      };
      AddNativeFunction(func, "list-remove");
      help_["list-remove"] =
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
    ;;
    ;; > (111 333))...";
    }

    // %%% search
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 検索するアトムを得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr key_ptr = caller.Evaluate(*(list_itr++));

        // リストを得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr target_ptr = caller.Evaluate(*list_itr);
        if (!(target_ptr->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {2}, true);
        }

        // 検索する。
        long long index = 0;
        for (LispIterator target_itr {target_ptr.get()}; target_itr;
        ++target_itr, ++index) {
          if (*target_itr == *key_ptr) return NewNumber(index);
        }

        return NewNil();
      };
      AddNativeFunction(func, "search");
      help_["search"] =
R"...(### search ###

<h6> Usage </h6>

* `(search <Object> <List>)`

<h6> Description </h6>

* If `<List>` has an object same as `<Object>`,
  it returns index number of the object.  
  Otherwise it returns Nil.

<h6> Example </h6>

    (define lst '(111 222 "Hello" #t))
    
    (display (search "Hello" lst))
    (display (search "World" lst))
    
    ;; Output
    ;; >  2
    ;; > ())...";
    }

    // %%% range
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 数を得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr size_ptr = caller.Evaluate(*list_itr);
        if (!(size_ptr->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        LispObjectPtr ret_ptr = NewNil();
        LispObject* ptr = ret_ptr.get();
        int size = size_ptr->number_value_;
        size = size < 0 ? 0 : size;
        for (int i = 0; i < size; ++i, ptr = ptr->cdr_.get()) {
          *ptr = *(NewPair(NewNumber(i), NewNil()));
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "range");
      help_["range"] =
R"...(### range ###

<h6> Usage </h6>

* `(range <Size : Number>)`

<h6> Description </h6>

* Returns List composed with 0...(`<Size>` - 1).

<h6> Example </h6>

    (display (range 10))
    
    ;; Output
    ;; > (0 1 2 3 4 5 6 7 8 9))...";
    }

    // %%% length
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);

        // Atomの場合。
        if (!(result->IsPair())) {
          if (result->IsNil()) return NewNumber(0);
          return NewNumber(1);
        }

        // Pairの場合。
        LispIterator result_itr {&(*result)};
        int count = 0;
        for (; result_itr; ++result_itr) ++count;
        if (!(result_itr.current_->IsNil())) ++count;

        return NewNumber(count);
      };
      AddNativeFunction(func, "length");
      help_["length"] =
R"...(### length ###

<h6> Usage </h6>

* `(length <List>)`

<h6> Description </h6>

* Returns number of `<List>`.
* If you input Atom, it returns 1. If Nil, it returns 0.

<h6> Example </h6>

    (display (length '(111 222 333 444 555 666)))
    
    ;; Output
    ;;
    ;; > 6)...";
    }

    // %%% <
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 最初の数字を得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }
        double prev = result->number_value_;

        // 2つ目以降の引数と比較する。
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          // 調べたい数字を得る。
          LispObjectPtr current_ptr = caller.Evaluate(*list_itr);
          if (!(current_ptr->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          // 比較。
          double current = current_ptr->number_value_;
          if (!(prev < current)) return NewBoolean(false);

          prev = current;
        }

        return NewBoolean(true);
      };
      AddNativeFunction(func, "<");
      help_["<"] =
R"...(### < ###

<h6> Usage </h6>

* `(< <Number>...)`

<h6> Description </h6>

* Returns #t if a Number is less than next Number.
  Otherwise, return #f.

<h6> Example </h6>

    (display (< 111 222 333))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% <=
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 最初の数字を得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }
        double prev = result->number_value_;

        // 2つ目以降の引数と比較する。
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          // 調べたい数字を得る。
          LispObjectPtr current_ptr = caller.Evaluate(*list_itr);
          if (!(current_ptr->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          // 比較。
          double current = current_ptr->number_value_;
          if (!(prev <= current)) return NewBoolean(false);

          prev = current;
        }

        return NewBoolean(true);
      };
      AddNativeFunction(func, "<=");
      help_["<="] =
R"...(### <= ###

<h6> Usage </h6>

* `(<= <Number>...)`

<h6> Description </h6>

* Returns #t if a Number is less or equal than next Number.
  Otherwise, return #f.

<h6> Example </h6>

    (display (< 111 222 333))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% >
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 最初の数字を得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }
        double prev = result->number_value_;

        // 2つ目以降の引数と比較する。
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          // 調べたい数字を得る。
          LispObjectPtr current_ptr = caller.Evaluate(*list_itr);
          if (!(current_ptr->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          // 比較。
          double current = current_ptr->number_value_;
          if (!(prev > current)) return NewBoolean(false);

          prev = current;
        }

        return NewBoolean(true);
      };
      AddNativeFunction(func, ">");
      help_[">"] =
R"...(### > ###

<h6> Usage </h6>

* `(> <Number>...)`

<h6> Description </h6>

* Returns #t if a Number is more than next Number.
  Otherwise, return #f.

<h6> Example </h6>

    (display (> 333 222 111))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% >=
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 最初の数字を得る。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }
        double prev = result->number_value_;

        // 2つ目以降の引数と比較する。
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          // 調べたい数字を得る。
          LispObjectPtr current_ptr = caller.Evaluate(*list_itr);
          if (!(current_ptr->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          // 比較。
          double current = current_ptr->number_value_;
          if (!(prev >= current)) return NewBoolean(false);

          prev = current;
        }

        return NewBoolean(true);
      };
      AddNativeFunction(func, ">=");
      help_[">="] =
R"...(### >= ###

<h6> Usage </h6>

* `(>= <Number>...)`

<h6> Description </h6>

* Returns #t if a Number is more or equal than next Number.
  Otherwise, return #f.

<h6> Example </h6>

    (display (>= 333 222 111))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% not
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数を評価してみる。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsBoolean())) {
          throw GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {1}, true);
        }

        return NewBoolean(!(result->boolean_value_));
      };
      AddNativeFunction(func, "not");
      help_["not"] =
R"...(### not ###

<h6> Usage </h6>

* `(not <Boolean>)`

<h6> Description </h6>

* Turns `<Boolean>` to opposite value. #t to #f, #f to #t.

<h6> Example </h6>

    (display (not (= 111 111)))
    
    ;; Output
    ;;
    ;; > #f)...";
    }

    // %%% and
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 各引数を調べる。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsBoolean())) {
            throw GenWrongTypeError
            (func_name, "Boolean", std::vector<int> {index}, true);
          }

          if (!(result->boolean_value_)) return NewBoolean(false);
        }
        return NewBoolean(true);
      };
      AddNativeFunction(func, "and");
      help_["and"] =
R"...(### and ###

<h6> Usage </h6>

* `(and <Boolean>...)`

<h6> Description </h6>

* Returns #t if all `<Boolean>...` are #t.
  Otherwise, return #f.

<h6> Example </h6>

    (display (and (= 111 111) (= 222 222)))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% or
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 各引数を調べる。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsBoolean())) {
            throw GenWrongTypeError
            (func_name, "Boolean", std::vector<int> {index}, true);
          }

          if (result->boolean_value_) return NewBoolean(true);
        }
        return NewBoolean(false);
      };
      AddNativeFunction(func, "or");
      help_["or"] =
R"...(### or ###

<h6> Usage </h6>

* `(or <Boolean>...)`

<h6> Description </h6>

* Returns #t if one of `<Boolean>...` is #t.
  If all `<Boolean>` are #f, return #f.

<h6> Example </h6>

    (display (or (= 111 111) (= 222 333)))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% +
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) -> LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();

        double value = 0.0;

        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          value += result->number_value_;
        }
        return NewNumber(value);
      };
      AddNativeFunction(func, "+");
      help_["+"] =
R"...(### + ###

<h6> Usage </h6>

* `(+ <Number>...)`

<h6> Description </h6>

* Sums up all `<Number>...`.

<h6> Example </h6>

    (display (+ 1 2 3))
    
    ;; Output
    ;;
    ;; > 6)...";
    }

    // %%% -
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();

        double value = 0.0;

        if (list_itr) {
          LispObjectPtr result = caller.Evaluate(*(list_itr++));
          if (!(result->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {1}, true);
          }
          value = result->number_value_;
        }
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          value -= result->number_value_;
        }
        return NewNumber(value);
      };
      AddNativeFunction(func, "-");
      help_["-"] =
R"...(### - ###

<h6> Usage </h6>

* `(- <1st number> <Number>...)`

<h6> Description </h6>

* Subtracts `<Number>...` from `<1st number>`.

<h6> Example </h6>

    (display (- 5 4 3))
    
    ;; Output
    ;;
    ;; > -2)...";
    }

    // %%% *
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();

        double value = 1.0;

        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          value *= result->number_value_;
        }
        return NewNumber(value);
      };
      AddNativeFunction(func, "*");
      help_["*"] =
R"...(### * ###

<h6> Usage </h6>

* `(* <Number>...)`

<h6> Description </h6>

* Multiplies all `<Number>...`.

<h6> Example </h6>

    (display (* 2 3 4))
    
    ;; Output
    ;;
    ;; > 24)...";
    }

    // %%% /
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();

        double value = 0.0;

        if (list_itr) {
          LispObjectPtr result = caller.Evaluate(*(list_itr++));
          if (!(result->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {1}, true);
          }
          value = result->number_value_;
        }
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          value /= result->number_value_;
        }
        return NewNumber(value);
      };
      AddNativeFunction(func, "/");
      help_["/"] =
R"...(### / ###

<h6> Usage </h6>

* `(/ <1st number> <Number>...)`

<h6> Description </h6>

* Divides `<1st number>` with `<Number>...`.

<h6> Example </h6>

    (display (/ 32 2 4))
    
    ;; Output
    ;;
    ;; > 4)...";
    }

    // %%% ++
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        // インクリメントして返す。
        return NewNumber(result->number_value_ + 1.0);
      };
      AddNativeFunction(func, "++");
      help_["++"] =
R"...(### ++ ###

<h6> Usage </h6>

* `(++ <Number>)`

<h6> Description </h6>

* Adds `<Number>` to '1'.

<h6> Example </h6>

    (display (++ 111))
    
    ;; Output
    ;;
    ;; > 112)...";
    }

    // %%% --
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        // インクリメントして返す。
        return NewNumber(result->number_value_ - 1.0);
      };
      AddNativeFunction(func, "--");
      help_["--"] =
R"...(### -- ###

<h6> Usage </h6>

* `(-- <Number>)`

<h6> Description </h6>

* Subtracts '1' from `<Number>`.

<h6> Example </h6>

    (display (-- 111))
    
    ;; Output
    ;;
    ;; > 110)...";
    }

    // %%% inc! 
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        // 第一引数はシンボルでなくてはならない。
        if (!(list_itr->IsSymbol())) {
          throw GenWrongTypeError
          (func_name, "Symbol", std::vector<int> {1}, false);
        }

        // 第一シンボルにはNumberがバインドされていなくてはならない。
        LispObjectPtr bound_ptr =
        caller.ReferSymbol(list_itr->str_value_);
        if (!(bound_ptr->IsNumber())) {
          throw GenError("@not-number",
          "\"" +  list_itr->str_value_ + "\" is not bound with Number.");
        }

        // バインドされているNumberをインクリメントする。
        bound_ptr->number_value_ = bound_ptr->number_value_ + 1;

        // シンボルを改めて評価して返す。
        return caller.Evaluate(*list_itr);
      };
      AddNativeFunction(func, "inc!");
      help_["inc!"] =
R"...(### inc! ###

<h6> Usage </h6>

* `(inc! <Symbol bound with Number : Symbol>)`

<h6> Description </h6>

* This is Special Form.
    + Rewrites `<Symbol bound with Number>`.
* Increments `<Symbol bound with Number>` and returns it.

<h6> Example </h6>

    (define i 111)
    (display (inc! i))
    (display i)
    
    ;; Output
    ;;
    ;; > 112
    ;; > 112)...";
    }

    // %%% dec! 
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        // 第一引数はシンボルでなくてはならない。
        if (!(list_itr->IsSymbol())) {
          throw GenWrongTypeError
          (func_name, "Symbol", std::vector<int> {1}, false);
        }

        // 第一シンボルにはNumberがバインドされていなくてはならない。
        LispObjectPtr bound_ptr = caller.ReferSymbol(list_itr->str_value_);
        if (!(bound_ptr->IsNumber())) {
          throw GenError("@not-number",
          "\"" +  list_itr->str_value_ + "\" is not bound with Number.");
        }

        // バインドされているNumberをインクリメントする。
        bound_ptr->number_value_ = bound_ptr->number_value_ - 1;

        // シンボルを改めて評価して返す。
        return caller.Evaluate(*list_itr);
      };
      AddNativeFunction(func, "dec!");
      help_["dec!"] =
R"...(### dec! ###

<h6> Usage </h6>

* `(dec! <Symbol bound with Number : Symbol>)`

<h6> Description </h6>

* This is Special Form.
    + Rewrites `<Symbol bound with Number>`.
* Decrements `<Symbol bound with Number>` and returns it.

<h6> Example </h6>

    (define i 111)
    (display (dec! i))
    (display i)
    
    ;; Output
    ;;
    ;; > 110
    ;; > 110)...";
    }

    // %%% string-split
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsString())) {
          throw GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。 区切り文字列。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr delim_ptr = caller.Evaluate(*list_itr);
        if (!(delim_ptr->IsString())) {
          throw GenWrongTypeError
          (func_name, "String", std::vector<int> {2}, true);
        }

        // 区切る。
        std::string origin = result->str_value_;
        std::string delim = delim_ptr->str_value_;
        std::vector<std::string> str_vec;
        for (std::string::size_type pos = origin.find(delim);
        pos != std::string::npos;
        pos = origin.find(delim)) {
          str_vec.push_back(origin.substr(0, pos));
          origin = origin.substr(pos + delim.size());
        }
        // 残りをプッシュ。
        str_vec.push_back(origin);

        // Listに区切った文字列を格納。
        LispObjectPtr ret_ptr = NewNil();
        LispObject* ptr = ret_ptr.get();
        for (auto& str : str_vec) {
          *ptr = *(NewPair(NewString(str), NewNil()));

          ptr = ptr->cdr_.get();
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "string-split");
      help_["string-split"] =
R"...(### string-split ###

<h6> Usage </h6>

* `(string-split <String> <Delim String>)`

<h6> Description </h6>

* Returns List composed of split `<String>` by `<Delim String>`.

<h6> Example </h6>

    (display (string-split "aaaaSplit!bbbSplit!ccc" "Split!"))
    
    ;; Outpu
    ;;
    ;; > ("aaa" "bbb" "ccc"))...";
    }

    // %%% front
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        LispObjectPtr ret_ptr = NewNil();
        if (result->IsPair()) ret_ptr = result->car_;

        return ret_ptr;
      };
      AddNativeFunction(func, "front");
      help_["front"] =
R"...(### front ###

<h6> Usage </h6>

* `(front <List>)`

<h6> Description </h6>

* Returns the first element of `<List>`.

<h6> Example </h6>

    (display (front '(111 222 333)))
    
    ;; Outpu
    ;;
    ;; > 111)...";
    }

    // %%% back
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        LispObjectPtr ret_ptr = NewNil();
        if (result->IsPair()) {
          LispObject* current_ptr = result.get();
          LispObject* ptr = result->cdr_.get();
          for (; ptr->IsPair(); ptr = ptr->cdr_.get()) {
            current_ptr = current_ptr->cdr_.get();
          }

          ret_ptr = current_ptr->car_;
        }

        return ret_ptr;
      };
      AddNativeFunction(func, "back");
      help_["back"] =
R"...(### back ###

<h6> Usage </h6>

* `(back <List>)`

<h6> Description </h6>

* Returns the last element of `<List>`.

<h6> Example </h6>

    (display (back '(111 222 333)))
    
    ;; Outpu
    ;;
    ;; > 333)...";
    }

    // %%% push-front
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr first_ptr = caller.Evaluate(*(list_itr++));
        if (!(first_ptr->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr second_ptr = caller.Evaluate(*list_itr);

        // くっつける。
        LispObjectPtr ret_ptr = NewPair();
        ret_ptr->car_ = second_ptr;
        ret_ptr->cdr_ = first_ptr;

        return ret_ptr;
      };
      AddNativeFunction(func, "push-front");
      help_["push-front"] =
R"...(### push-front ###

<h6> Usage </h6>

* `(push-front <List> <Object>)`

<h6> Description </h6>

* Returns List added `<Object>` at the first element of `<List>`

<h6> Example </h6>

    (display (push-front '(111 222 333) "Hello"))
    
    ;; Outpu
    ;;
    ;; > ("Hello" 111 222 333))...";
    }

    // %%% pop-front
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        LispObjectPtr ret_ptr = NewNil();
        if (result->IsPair()) ret_ptr = result->cdr_;

        return ret_ptr;
      };
      AddNativeFunction(func, "pop-front");
      help_["pop-front"] =
R"...(### pop-front ###

<h6> Usage </h6>

* `(pop-front <List>)`

<h6> Description </h6>

* Returns List removed the first element from `<List>`.

<h6> Example </h6>

    (display (pop-front '(111 222 333)))
    
    ;; Outpu
    ;;
    ;; > (222 333))...";
    }

    // %%% push-back
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr first_ptr = caller.Evaluate(*(list_itr++));
        if (!(first_ptr->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr second_ptr = caller.Evaluate(*list_itr);

        // くっつける。
        first_ptr->Append(NewPair(second_ptr, NewNil()));

        return first_ptr;
      };
      AddNativeFunction(func, "push-back");
      help_["push-back"] =
R"...(### push-back ###

<h6> Usage </h6>

* `(push-back <List> <Object>)`

<h6> Description </h6>

* Returns List added `<Object>` at the last element of `<List>`

<h6> Example </h6>

    (display (push-back '(111 222 333) "Hello"))
    
    ;; Outpu
    ;;
    ;; > (111 222 333 "Hello"))...";
    }

    // %%% pop-back
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsList())) {
          throw GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        if (result->IsPair()) {
          LispObject* current_ptr = result.get();
          LispObject* ptr = result->cdr_.get();
          for (; ptr->IsPair(); ptr = ptr->cdr_.get()) {
            current_ptr = current_ptr->cdr_.get();
          }
          *current_ptr = *(NewNil());
        }

        return result;
      };
      AddNativeFunction(func, "pop-back");
      help_["pop-back"] =
R"...(### pop-back ###

<h6> Usage </h6>

* `(pop-back <List>)`

<h6> Description </h6>

* Returns List removed the last element from `<List>`.

<h6> Example </h6>

    (display (pop-back '(111 222 333)))
    
    ;; Outpu
    ;;
    ;; > (111 222))...";
    }

    // %%% PI
    global_ptr_->BindSymbol("PI", NewNumber(4.0 * std::atan(1.0)));
    help_["PI"] =
R"...(### PI ###

<h6> Description </h6>

* Circular constant.

<h6> Example </h6>

    (display PI)
    
    ;; Output
    ;;
    ;; > 3.14159265358979)...";

    // %%% E
    global_ptr_->BindSymbol("E", NewNumber(std::exp(1.0)));
    help_["E"] =
R"...(### E ###

<h6> Description </h6>

* Napier's constant.

<h6> Example </h6>

    (display E)
    
    ;; Output
    ;;
    ;; > 2.71828182845905)...";

    // %%% sin
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::sin(result->number_value_));
      };
      AddNativeFunction(func, "sin");
      help_["sin"] =
R"...(### sin ###

<h6> Usage </h6>

* `(sin <Number>)`

<h6> Description </h6>

* Sine. A trigonometric function.
* `<Number>` is radian.

<h6> Example </h6>

    (display (sin (/ PI 2)))
    
    ;; Output
    ;;
    ;; > 1)...";
    }

    // %%% cos
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::cos(result->number_value_));
      };
      AddNativeFunction(func, "cos");
      help_["cos"] =
R"...(### cos ###

<h6> Usage </h6>

* `(cos <Number>)`

<h6> Description </h6>

* Cosine. A trigonometric function.
* `<Number>` is radian.

<h6> Example </h6>

    (display (cos PI))
    
    ;; Output
    ;;
    ;; > -1)...";
    }

    // %%% tan
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::tan(result->number_value_));
      };
      AddNativeFunction(func, "tan");
      help_["tan"] =
R"...(### tan ###

<h6> Usage </h6>

* `(tan <Number>)`

<h6> Description </h6>

* Tangent. A trigonometric function.
* `<Number>` is radian.

<h6> Example </h6>

    (display (tan (/ PI 4)))
    
    ;; Output
    ;;
    ;; > 1)...";
    }

    // %%% asin
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::asin(result->number_value_));
      };
      AddNativeFunction(func, "asin");
      help_["asin"] =
R"...(### asin ###

<h6> Usage </h6>

* `(asin <Number>)`

<h6> Description </h6>

* Arc sine. A trigonometric function.
* `<Number>` is sine.

<h6> Example </h6>

    (display (asin 0))
    
    ;; Output
    ;;
    ;; > 0)...";
    }

    // %%% acos
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::acos(result->number_value_));
      };
      AddNativeFunction(func, "acos");
      help_["acos"] =
R"...(### acos ###

<h6> Usage </h6>

* `(acos <Number>)`

<h6> Description </h6>

* Arc cosine. A trigonometric function.
* `<Number>` is cosine.

<h6> Example </h6>

    (display (acos 1))
    
    ;; Output
    ;;
    ;; > 0)...";
    }

    // %%% atan
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::atan(result->number_value_));
      };
      AddNativeFunction(func, "atan");
      help_["atan"] =
R"...(### atan ###

<h6> Usage </h6>

* `(atan <Number>)`

<h6> Description </h6>

* Arc tangent. A trigonometric function.
* `<Number>` is tangent.

<h6> Example </h6>

    (display (atan 0))
    
    ;; Output
    ;;
    ;; > 0)...";
    }

    // %%% sqrt
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::sqrt(result->number_value_));
        // 引数をチェック(result->number_value_));
      };
      AddNativeFunction(func, "sqrt");
      help_["sqrt"] =
R"...(### sqrt ###

<h6> Usage </h6>

* `(sqrt <Number>)`

<h6> Description </h6>

* Returns square root of `<Number>`.

<h6> Example </h6>

    (display (sqrt 4))
    
    ;; Output
    ;;
    ;; > 2)...";
    }

    // %%% abs
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::abs(result->number_value_));
      };
      AddNativeFunction(func, "abs");
      help_["abs"] =
R"...(### abs ###

<h6> Usage </h6>

* `(abs <Number>)`

<h6> Description </h6>

* Returns absolute value of `<Number>`.

<h6> Example </h6>

    (display (abs -111))
    
    ;; Output
    ;;
    ;; > 111)...";
    }

    // %%% ceil
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::ceil(result->number_value_));
      };
      AddNativeFunction(func, "ceil");
      help_["ceil"] =
R"...(### ceil ###

<h6> Usage </h6>

* `(ceil <Number>)`

<h6> Description </h6>

* Rounds up `<Number>` into integral value.

<h6> Example </h6>

    (display (ceil 1.3))
    
    ;; Output
    ;;
    ;; > 2)...";
    }

    // %%% floor
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::floor(result->number_value_));
      };
      AddNativeFunction(func, "floor");
      help_["floor"] =
R"...(### floor ###

<h6> Usage </h6>

* `(floor <Number>)`

<h6> Description </h6>

* Rounds down `<Number>` into integral value.

<h6> Example </h6>

    (display (floor 1.3))
    
    ;; Output
    ;;
    ;; > 1)...";
    }

    // %%% round
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::round(result->number_value_));
      };
      AddNativeFunction(func, "round");
      help_["round"] =
R"...(### round ###

<h6> Usage </h6>

* `(round <Number>)`

<h6> Description </h6>

* Rounds `<Number>` into the nearest integral value.

<h6> Example </h6>

    (display (round 1.5))
    
    ;; Output
    ;;
    ;; > 2
    
    (display (round 1.49))
    
    ;; Output
    ;;
    ;; > 1)...";
    }

    // %%% trunc
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::trunc(result->number_value_));
      };
      AddNativeFunction(func, "trunc");
      help_["trunc"] =
R"...(### trunc ###

<h6> Usage </h6>

* `(trunc <Number>)`

<h6> Description </h6>

* Truncates after decimal point of `<Number>`.

<h6> Example </h6>

    (display (trunc 1.234))
    
    ;; Output
    ;;
    ;; > 1
    
    (display (trunc -1.234))
    
    ;; Output
    ;;
    ;; > -1)...";
    }

    // %%% exp
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::exp(result->number_value_));
      };
      AddNativeFunction(func, "exp");
      help_["exp"] =
R"...(### exp ###

<h6> Usage </h6>

* `(exp <Number>)`

<h6> Description </h6>

* Exponent function of `<Number>`. The base is Napier's constant.

<h6> Example </h6>

    (display (exp 1))
    
    ;; Output
    ;;
    ;; > 2.71828)...";
    }

    // %%% expt
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr first_ptr = caller.Evaluate(*(list_itr++));
        if (!(first_ptr->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr second_ptr = caller.Evaluate(*list_itr);
        if (!(second_ptr->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        return NewNumber
        (std::pow(first_ptr->number_value_, second_ptr->number_value_));
      };
      LispObjectPtr func_ptr =
      NewNativeFunction(global_ptr_->scope_chain_, func);
      global_ptr_->BindSymbol("expt", func_ptr);
      global_ptr_->BindSymbol("^", func_ptr);
      std::string temp =
R"...(### expt ###

<h6> Usage </h6>

* `(expt <Base> <Exponent>)`
* `(^ <Base> <Exponent>)`

<h6> Description </h6>

* Exponent function of `<Exponent>`. The base is `<Base>`.

<h6> Example </h6>

    (display (expt 2 3))
    
    ;; Output
    ;;
    ;; > 8)...";
      help_["expt"] = temp;
      help_["^"] = temp;
    }

    // %%% log
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::log(result->number_value_));
      };
      LispObjectPtr func_ptr =
      NewNativeFunction(global_ptr_->scope_chain_, func);
      global_ptr_->BindSymbol("log", func_ptr);
      global_ptr_->BindSymbol("ln", func_ptr);
      std::string temp =
R"...(### log ###

<h6> Usage </h6>

* `(log <Number>)`
* `(ln <Number>)`

<h6> Description </h6>

* Logarithmic function of `<Number>`. The base is Napier's constant.

<h6> Example </h6>

    (display (log E))
    
    ;; Output
    ;;
    ;; > 1)...";
      help_["log"] = temp;
      help_["ln"] = temp;
    }

    // %%% log2
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::log2(result->number_value_));
      };
      AddNativeFunction(func, "log2");
      help_["log2"] =
R"...(### log2 ###

<h6> Usage </h6>

* `(log2 <Number>)`

<h6> Description </h6>

* Logarithmic function of `<Number>`. The base is 2.

<h6> Example </h6>

    (display (log2 8))
    
    ;; Output
    ;;
    ;; > 3)...";
    }

    // %%% log10
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return NewNumber(std::log10(result->number_value_));
      };
      AddNativeFunction(func, "log10");
      help_["log10"] =
R"...(### log10 ###

<h6> Usage </h6>

* `(log10 <Number>)`

<h6> Description </h6>

* Logarithmic function of `<Number>`. The base is 10.

<h6> Example </h6>

    (display (log10 100))
    
    ;; Output
    ;;
    ;; > 2)...";
    }

    // %%% random
    {
      using Time = std::chrono::system_clock;
      std::shared_ptr<std::mt19937>
      engine_ptr(new std::mt19937(Time::to_time_t(Time::now())));
      auto func = [engine_ptr](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        std::uniform_real_distribution<> dist(0, result->number_value_);
        return NewNumber(dist(*engine_ptr));
      };
      AddNativeFunction(func, "random");
      help_["random"] =
R"...(### random ###

<h6> Usage </h6>

* `(random <Number>)`

<h6> Description </h6>

* Returns random number from 0 to `<Number>`.

<h6> Example </h6>

    (display (random 10))
    
    ;; Output
    ;;
    ;; > 2.42356
    
    (display (random -10))
    
    ;; Output
    ;;
    ;; > -7.13453)...";
    }

    // %%% max
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr max_ptr = caller.Evaluate(*(list_itr++));
        if (!(max_ptr->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr current = caller.Evaluate(*list_itr);
          if (!(current->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          if (current->number_value_ > max_ptr->number_value_) {
            max_ptr = current;
          }
        }

        return max_ptr;
      };
      AddNativeFunction(func, "max");
      help_["max"] =
R"...(### max ###

<h6> Usage </h6>

* `(max <Number>...)`

<h6> Description </h6>

* Returns maximum number of `<Number>...`.

<h6> Example </h6>

    (display (max 1 2 3 4 3 2 1))
    
    ;; Output
    ;;
    ;; > 4)...";
    }

    // %%% min
    {
      auto func = [](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr {&list};
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数。
        if (!list_itr) {
          throw GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr min_ptr = caller.Evaluate(*(list_itr++));
        if (!(min_ptr->IsNumber())) {
          throw GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr current = caller.Evaluate(*list_itr);
          if (!(current->IsNumber())) {
            throw GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          if (current->number_value_ < min_ptr->number_value_) {
            min_ptr = current;
          }
        }

        return min_ptr;
      };
      AddNativeFunction(func, "min");
      help_["min"] =
R"...(### min ###

<h6> Usage </h6>

* `(min <Number>...)`

<h6> Description </h6>

* Returns minimum number of `<Number>...`.

<h6> Example </h6>

    (display (min 4 3 2 1 2 3 4))
    
    ;; Output
    ;;
    ;; > 1)...";
    }
  }

  // 「不十分な引数エラー」を作成する。
  LispObjectPtr Lisp::GenInsufficientArgumentsError
  (const std::string& func_name, int require, bool is_and_more,
  int given) {
      std::ostringstream oss;

      // 必要な引数の数。
      oss << "(" << func_name << ") needs " << require;
      if (require <= 1) oss << " argument";
      else oss << " arguments";

      // 以上かどうか。
      if (is_and_more) oss << " and more. ";
      else oss << ". ";

      // 実際与えられた引数の数。
      oss << "Given " << std::to_string(given);
      if (given <= 1) oss << " argument.";
      else oss << " arguments.";

      return GenError("@insufficient-arguments", oss.str());
  }

  // タイプ違いのエラーリストを作成する。
  LispObjectPtr Lisp::GenWrongTypeError(const std::string& func_name,
  const std::string& required_type_str, std::vector<int> index_vec,
  bool has_evaluated) {
    // エラー識別シンボルを作成。
    std::string error_symbol = "";
    if (required_type_str == "Pair") {
      error_symbol = "@not-pair";
    } else if (required_type_str == "Nil") {
      error_symbol = "@not-nil";
    } else if (required_type_str == "Symbol") {
      error_symbol = "@not-symbol";
    } else if (required_type_str == "Number") {
      error_symbol = "@not-number";
    } else if (required_type_str == "Boolean") {
      error_symbol = "@not-boolean";
    } else if (required_type_str == "String") {
      error_symbol = "@not-string";
    } else if (required_type_str == "List") {
      error_symbol = "@not-list";
    } else if (required_type_str == "Procedure") {
      error_symbol = "@not-procedure";
    } else if (required_type_str == "Function") {
      error_symbol = "@not-function";
    } else if (required_type_str == "Native Function") {
      error_symbol = "@not-native-function";
    } else {
      error_symbol = "@type-error";
    }

    // エラーメッセージを作成。
    // どの要素かを作成。
    std::ostringstream oss;
    bool first = true;
    for (; index_vec.size() > 0; index_vec.pop_back()) {
      // 冠詞。
      if (first) {
        oss << "The ";
        first = false;
      } else {
        oss << "the ";
      }

      // 何番目か。
      oss << std::to_string(index_vec.back());
      int column_1 = index_vec.back() % 10;
      if (column_1 == 1) oss << "st ";
      else if (column_1 == 2) oss << "nd ";
      else if (column_1 == 3) oss << "rd ";
      else oss << "th ";

      // 要素。
      if (index_vec.size() == 1) oss << "argument of ";
      else oss << "element of ";
    }

    // どの関数かを作成。
    oss << "(" << func_name << ") ";
    if (has_evaluated) oss << "didn't return ";
    else oss << "is not ";

    // 要求されたタイプを作成。
    oss << required_type_str << ".";

    return GenError(error_symbol, oss.str());
  }
}  // namespace Sayuri
