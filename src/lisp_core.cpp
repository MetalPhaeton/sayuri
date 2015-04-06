/* The MIT License (MIT)
 *
 * Copyright (c) 2015 Hironori Ishibashi
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

/** Sayuri 名前空間。 */
namespace Sayuri {
  // ============= //
  // LispFunction関連 //
  // ============= //
  // コンストラクタ。
  LispFunction::LispFunction() {}

  // コピーコンストラクタ。
  LispFunction::LispFunction(const LispFunction& func) :
  arg_name_vec_(func.arg_name_vec_), def_vec_(func.def_vec_) {}

  // ムーブコンストラクタ。
  LispFunction::LispFunction(LispFunction&& func) :
  arg_name_vec_(std::move(func.arg_name_vec_)),
  def_vec_(std::move(func.def_vec_)) {}

  // コピー代入演算子。
  LispFunction& LispFunction::operator=(const LispFunction& func) {
    arg_name_vec_ = func.arg_name_vec_;
    def_vec_ = func.def_vec_;
    return *this;
  }

  // ムーブ代入演算子。
  LispFunction& LispFunction::operator=(LispFunction&& func) {
    arg_name_vec_ = std::move(func.arg_name_vec_);
    def_vec_ = std::move(func.def_vec_);
    return *this;
  }

  // デストラクタ。
  LispFunction::~LispFunction() {}

  // =========== //
  // LispObject関連 //
  // =========== //
  // コンストラクタ。
  LispObject::LispObject() :
  type_(LispObjectType::NIL),
  car_(nullptr),
  cdr_(nullptr),
  number_value_(0.0),
  str_value_("") {}

  // コピーコンストラクタ。
  LispObject::LispObject(const LispObject& obj) :
  type_(obj.type_),
  car_(obj.car_),
  cdr_(obj.cdr_),
  number_value_(obj.number_value_),
  str_value_(obj.str_value_),
  function_(obj.function_),
  native_function_(obj.native_function_),
  scope_chain_(obj.scope_chain_) {}

  // ムーブコンストラクタ。
  LispObject::LispObject(LispObject&& obj) :
  type_(obj.type_),
  car_(std::move(obj.car_)),
  cdr_(std::move(obj.cdr_)),
  number_value_(obj.number_value_),
  str_value_(std::move(obj.str_value_)),
  function_(std::move(obj.function_)),
  native_function_(std::move(obj.native_function_)),
  scope_chain_(std::move(obj.scope_chain_)) {}

  // コピー代入演算子。
  LispObject& LispObject::operator=(const LispObject& obj) {
    type_ = obj.type_;
    car_ = obj.car_;
    cdr_ = obj.cdr_;
    number_value_ = obj.number_value_;
    str_value_ = obj.str_value_;
    function_ = obj.function_;
    native_function_ = obj.native_function_;
    scope_chain_ = obj.scope_chain_;
    return *this;
  }

  // ムーブ代入演算子。
  LispObject& LispObject::operator=(LispObject&& obj) {
    type_ = obj.type_;
    car_ = std::move(obj.car_);
    cdr_ = std::move(obj.cdr_);
    number_value_ = obj.number_value_;
    str_value_ = std::move(obj.str_value_);
    function_ = std::move(obj.function_);
    native_function_ = std::move(obj.native_function_);
    scope_chain_ = std::move(obj.scope_chain_);
    return *this;
  }

  // デストラクタ。
  LispObject::~LispObject() {}

  // 自分の文字列表現。
  std::string LispObject::ToString() const {
    std::ostringstream stream;

    if (IsNil()) {
      // Nil。
      stream << "()";
    } else if (IsSymbol()) {
      // Symbol。
      stream << str_value_;
    } else if (IsNumber()) {
      // Number。
      stream << number_value_;
    } else if (IsBoolean()) {
      // Boolean。
      if (boolean_value_) {
        stream << "#t";
      } else {
        stream << "#f";
      }
    } else if (IsString()) {
      // String。
      stream << "\"";
      for (auto c : str_value_) {
        switch (c) {
          case '\n':
            stream << "\\n";
            break;
          case '\r':
            stream << "\\r";
            break;
          case '\t':
            stream << "\\t";
            break;
          case '\b':
            stream << "\\b";
            break;
          case '\a':
            stream << "\\a";
            break;
          case '\f':
            stream << "\\f";
            break;
          case '\0':
            stream << "\\0";
            break;
          case '\"':
            stream << "\\\"";
            break;
          case '\\':
            stream << "\\\\";
            break;
          default:
            stream << c;
            break;
        }
      }
      stream << "\"";
    } else if (IsFunction()) {
      // Function。
      stream << "(lambda (";

      // 引数を書き込む。
      std::string temp = "";
      for (auto& name : function_.arg_name_vec_) {
        temp += name + " ";
      }
      temp.pop_back();
      stream << temp << ") ";

      // 定義を書き込む。
      for (auto& obj : function_.def_vec_) {
        stream << obj->ToString();
      }
      stream << ")";
    } else if (IsNativeFunction()) {
      // Native Function。
      stream << ";; Native Function";
    } else if (IsPair()) {
      std::string temp = "";
      LispIterator itr(this);
      for (; itr; ++itr) {
        temp += itr->ToString() + " ";
      }
      if (itr.current().IsNil()) {
        temp.pop_back();
      } else {
        temp += ". " + itr.current().ToString();
      }
      stream << "(" << temp << ")";
    }

    return stream.str();
  }

  // 自分のシンボルマップで評価する。
  LispObjectPtr LispObject::Evaluate(const LispObject& target) const {
    // 自分がFUNCTION又は、NATIVE_FUNCTIONではない場合は不正。
    if (!((IsFunction() || IsNativeFunction()))) {
      throw GenError("@not-procedure",
      "Caller of Evaluate() is not Function or Native Function.");
    }

    if (!(target.IsList())) {
      // Listではない場合。
      if (target.IsSymbol()) {
        // シンボルの場合。
        LispObjectPtr ret_ptr = ReferSymbol(target.str_value_)->Clone();

        return ret_ptr;
      } else {
        // その他のAtomの場合、コピーを返す。
        return target.Clone();
      }
    } else {
      // Listの場合。
      if (target.IsNil()) {
        // Nilならコピーを返す。
        return target.Clone();
      }

      // 準備。
      LispIterator target_itr(&target);

      // 第1要素を評価して関数オブジェクトを得る。
      std::string func_name = target_itr->ToString();
      LispObjectPtr func_obj = Evaluate(*(target_itr++));

      if (func_obj->IsFunction()) {
        // 最初のCarが関数オブジェクトの場合。
        // ローカルスコープを作成。
        func_obj->NewLocalScope();

        // 引数リストをシンボルマップにバインド。
        std::vector<std::string>& arg_name_vec =
        func_obj->function_.arg_name_vec_;
        for (auto& arg_name : arg_name_vec) {
          if (!target_itr) {
            throw GenInsufficientArgumentsError
            (func_name, arg_name_vec.size(), false, target.Length() - 1);
          }

          func_obj->BindSymbol(arg_name, Evaluate(*(target_itr++)));
        }

        // 関数を評価。
        LispObjectPtr ret_ptr;
        for (auto& def : func_obj->function_.def_vec_) {
          // 評価。
          ret_ptr = func_obj->Evaluate(*def);
        }

        return ret_ptr;
      } else if (func_obj->IsNativeFunction()) {
        // ネイティブ関数オブジェクトの場合。
        LispObjectPtr ret_ptr =
        func_obj->native_function_(func_obj, *this, target);

        return ret_ptr;
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

        throw GenError(error_symbol, message);
      } 
    }

    throw GenError("@runtime-error",
    "Evaluate() couldn't evaluate " + target.ToString() + ".");
  }

  // S式をパースする。
  std::vector<LispObjectPtr>
  LispObject::Parse(std::queue<std::string>& token_queue) {
    std::vector<LispObjectPtr> ret_vec;

    // パース。
    while (token_queue.size() > 0) {
      LispObjectPtr temp_ptr = LispObject::NewNil();

      ParseCore(*temp_ptr, token_queue);

      ret_vec.push_back(temp_ptr);
    }

    return ret_vec;
  }

  // Parse()の本体。
  void LispObject::ParseCore(LispObject& target,
  std::queue<std::string>& token_queue) {
    if (token_queue.empty()) return;

    // 最初の文字。
    std::string front = token_queue.front();
    token_queue.pop();

    if (front == "(") {
      // リストの場合。
      // 空のリストならNil。
      if (token_queue.front() == ")") {
        target.type_ = LispObjectType::NIL;

        token_queue.pop();
        return;
      }

      // 空のリストでない。
      LispObject* current = &target;
      while (!(token_queue.empty())) {
        current->type_ = LispObjectType::PAIR;
        current->car_ = LispObject::NewNil();
        current->cdr_ = LispObject::NewNil();

        // Carの処理。
        // なぜか次の文字がドットだったら、CarはNilのまま。
        if (token_queue.front() != ".") {
          ParseCore(*(current->car_), token_queue);
        }

        // Cdrの処理。
        if (token_queue.empty()) return;
        if (token_queue.front() == ".") {
          // ドット対の記号。
          token_queue.pop();
          if (token_queue.empty()) return;

          // Cdrにパース。 なぜか次の文字が閉じ括弧だったら、CdrはNilのまま。
          if (token_queue.front() != ")") {
            ParseCore(*(current->cdr_), token_queue);
          }

          // リストの終端まで読み飛ばして終了。
          if (token_queue.empty()) return;
          while (token_queue.front() != ")") {
            token_queue.pop();
            if (token_queue.empty()) return;
          }
          token_queue.pop();

          return;
        } else if (token_queue.front() == ")") {
          // リストの終端。
          token_queue.pop();
          return;
        } else {
          // まだ続きがある。
          current = current->cdr_.get();
        }
      }
    } else {
      if (front == "\"") {
        // String。
        if (token_queue.empty()) return;
        target.type_ = LispObjectType::STRING;
        target.str_value_ = token_queue.front();
        token_queue.pop();

        // 次の'"'までポップ。
        while (!(token_queue.empty())) {
          if (token_queue.front() == "\"") {
            token_queue.pop();
            return;
          }
          token_queue.pop();
        }
      } else if ((front == "#t") || (front == "#T")){
        // Boolean::true。
        target.type_ = LispObjectType::BOOLEAN;
        target.boolean_value_ = true;
      } else if ((front == "#f") || (front == "#F")){
        // Boolean::false。
        target.type_ = LispObjectType::BOOLEAN;
        target.boolean_value_ = false;
      } else if (front == "'") {
        // quoteの糖衣構文。
        target.type_ = LispObjectType::PAIR;
        target.car_ = NewSymbol("quote");
        target.cdr_ = NewPair();

        // 第1引数(Cdr->Car)をパース。
        ParseCore(*(target.cdr_->car_), token_queue);
      } else if (front == ".") {
        // 妙な所にドット対発見。 CarはNilとする。
        target.type_ = LispObjectType::NIL;
      } else {
        try {
          // Number。
          target.type_ = LispObjectType::NUMBER;
          target.number_value_ = std::stod(front);
        } catch (...) {
          // Symbol。
          target.type_ = LispObjectType::SYMBOL;
          target.str_value_ = front;
        }
      }
    }
  }

  // コアのネイティブ関数をセットする。
  void LispObject::SetCoreFunctions(LispObjectPtr root_ptr,
  std::shared_ptr<HelpDict> dict_ptr) {
    // %%% help
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [dict_ptr]
      (LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        if (!list_itr) {
          // 引数がなければ一覧を文字列にする。
          std::ostringstream stream;
          for (auto& pair : *dict_ptr) {
            stream << pair.second;
            stream << std::endl << std::endl;
            stream << "- - - - - - - - - - - - - - - - - - - - "
            "- - - - - - - - - - - - - - - - - - - -" << std::endl;
            stream << std::endl;
          }
          return LispObject::NewString(stream.str());
        } else {
          // 引数があればその項目を文字列にする。
          LispObjectPtr first_ptr = caller.Evaluate(*list_itr);
          if (!(first_ptr->IsSymbol())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Symbol", std::vector<int> {1}, true);
          }

          HelpDict::iterator itr = dict_ptr->find(first_ptr->symbol_value());
          if (itr != dict_ptr->end()) {
            return LispObject::NewString(itr->second);
          }

          return LispObject::NewString
          ("Not found help of " + list_itr->ToString() + ".");
        }

        return LispObject::NewNil();
      };
      root_ptr->BindSymbol("help", func_ptr);
      (*dict_ptr)["help"] =
R"...(### help ###

__Usage__

1. `(help)`
2. `(help <Symbol>)`

__Description__

* 1: Returns descriptions of all symbols.
* 2: Returns a description of `<Symbol>`.
* All descriptions are Markdown format.

__Example__

    (display (help 'car))
    
    ;; Output
    ;;
    ;; > ### car ###
    ;; >
    ;; > __Usage__
    ;; >
    ;; >
    ;; > * `(car <List>)`
    ;; >
    ;; > __Description__
    ;; >
    ;; > * Returns the 1st element of `<List>`.
    ;; >
    ;; > __Example__
    ;; >
    ;; >     (display (car (list 111 222 333)))
    ;; >     
    ;; >     ;; Output
    ;; >     ;;
    ;; >     ;; > 111)...";
    }

    // %%% eval
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        return caller.Evaluate(*(caller.Evaluate(*list_itr)));
      };
      root_ptr->BindSymbol("eval", func_ptr);
      (*dict_ptr)["eval"] =
R"...(### eval ###

__Usage__

* `(eval <Object>)`

__Description__

* Evaluates `<Object>`.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsString())) {
          throw LispObject::GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // パース。
        LispTokenizer tokenizer;
        tokenizer.Analyse(result->string_value());
        std::queue<std::string> token_queue = tokenizer.token_queue();
        std::vector<LispObjectPtr> ret_vec = LispObject::Parse(token_queue);

        // 結果を返す。
        if (ret_vec.empty()) return LispObject::NewNil();
        return ret_vec[0];
      };
      root_ptr->BindSymbol("parse", func_ptr);
      root_ptr->BindSymbol("string->symbol", func_ptr);
      root_ptr->BindSymbol("string->number", func_ptr);
      root_ptr->BindSymbol("string->boolean", func_ptr);
      root_ptr->BindSymbol("string->list", func_ptr);
      std::string temp =
R"...(### parse ###

__Usage__

* `(parse <S-Expression : String>)`
* `(string->symbol <S-Expression : String>)`
* `(string->number <S-Expression : String>)`
* `(string->boolean <S-Expression : String>)`
* `(string->list <S-Expression : String>)`

__Description__

* Parses `<S-Expression>` and generates a object.

__Example__

    (display (parse "(1 2 3)"))
    
    ;; Output
    ;;
    ;; > (1 2 3))...";
      (*dict_ptr)["parse"] = temp;
      (*dict_ptr)["string->symbol"] = temp;
      (*dict_ptr)["string->number"] = temp;
      (*dict_ptr)["string->boolean"] = temp;
      (*dict_ptr)["string->list"] = temp;
    }

    // %%% to-string
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        
        LispObjectPtr result = caller.Evaluate(*list_itr);
        return LispObject::NewString(result->ToString());
      };
      root_ptr->BindSymbol("to-string", func_ptr);
      root_ptr->BindSymbol("symbol->string", func_ptr);
      root_ptr->BindSymbol("number->string", func_ptr);
      root_ptr->BindSymbol("boolean->string", func_ptr);
      root_ptr->BindSymbol("list->string", func_ptr);
      std::string temp =
R"...(### to-string ###

__Usage__

* `(to-string <Object>)`
* `(symbol->string <Object>)`
* `(number->string <Object>)`
* `(boolean->string <Object>)`
* `(list->string <Object>)`

__Description__

* Converts `<Object>` to S-Expression as String.

__Example__

    (display (to-string '(1 2 3)))
    
    ;; Output
    ;;
    ;; > (1 2 3)
    ;;
    
    (display (string? (to-string '(1 2 3))))
    
    ;; Output
    ;;
    ;; > #t)...";
      (*dict_ptr)["to-string"] = temp;
      (*dict_ptr)["symbol->string"] = temp;
      (*dict_ptr)["number->string"] = temp;
      (*dict_ptr)["boolean->string"] = temp;
      (*dict_ptr)["list->string"] = temp;
    }

    // %%% try
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第一引数を得る。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObject& first = *(list_itr++);
        if (!(first.IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, false);
        }

        LispObjectPtr ret_ptr = LispObject::NewNil();
        try {
          // 第1引数をトライ。
          for (LispIterator first_itr(&first); first_itr; ++first_itr) {
            ret_ptr = caller.Evaluate(*first_itr);
          }
        } catch (LispObjectPtr exception) {
          // 第2引数以降でキャッチ。
          if (!list_itr) {
            throw LispObject::GenInsufficientArgumentsError
            (func_name, required_args, true, list.Length() - 1);
          }

          // 自分のスコープに例外メッセージをバインド。
          self->scope_chain(caller.scope_chain());
          self->NewLocalScope();
          self->BindSymbol("exception", exception);

          // 例外処理。
          for (; list_itr; ++list_itr) {
            ret_ptr = self->Evaluate(*list_itr);
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("try", func_ptr);
      (*dict_ptr)["try"] =
R"...(### try ###

__Usage__

* `(try (<Try Expr>...) <Catch Expr>...)`

__Description__

* This is Special Form.
    * `<Catch Expr>...` is evaluated if an error have been occurred
      in `<Try Expr>...`.
* Handles exceptions.
* If an exception occurs in `<Try Expr>...`, then
  it stops `<Try Expr>...` and executes `<Catch Expr>...`.
* In a scope of `<Catch Expr>...`, 'exception' symbol is defined.
* Returns a evaluated last object.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }

        throw caller.Evaluate(*list_itr);
      };
      root_ptr->BindSymbol("throw", func_ptr);
      (*dict_ptr)["throw"] =
R"...(### throw ###

__Usage__

* `(throw <Object>)`

__Description__

* Throws an exception.
* If you use this in (try) function,
  `<Object>` is bound to 'exception' symbol.

__Example__

    (try ((throw 123))
         (display exception))
    
    ;; Output
    ;;
    ;; > 123)...";
    }

    // %%% car
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsPair())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Pair", std::vector<int> {1}, true);
        }

        return result->car()->Clone();
      };
      root_ptr->BindSymbol("car", func_ptr);
      (*dict_ptr)["car"] =
R"...(### car ###

__Usage__

* `(car <Pair or List>)`

__Description__

* Returns Car value of `<Pair or List>`

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsPair())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Pair", std::vector<int> {1}, true);
        }

        return result->cdr()->Clone();
      };
      root_ptr->BindSymbol("cdr", func_ptr);
      (*dict_ptr)["cdr"] =
R"...(### cdr ###

__Usage__

* `(cdr <Pair or List>)`

__Description__

* Returns Cdr value of `<Pair or List>`

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result_car = caller.Evaluate(*(list_itr++));
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result_cdr = caller.Evaluate(*list_itr);

        return LispObject::NewPair(result_car, result_cdr);
      };
      root_ptr->BindSymbol("cons", func_ptr);
      (*dict_ptr)["cons"] =
R"...(### cons ###

__Usage__

* `(cons <Object 1> <Object 2>)`

__Description__

* Returns Pair. Car is `<Object 1>`, Cdr is `<Object 2>`.

__Example__


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

    // %%% quote
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        return list_itr->Clone();
      };
      root_ptr->BindSymbol("quote", func_ptr);
      (*dict_ptr)["quote"] =
R"...(### quote ###

__Usage__

* `(quote <Object>)`

__Description__

* This is Special Form.
    + `<Object>` is not Evaluated.
* Returns `<Object>` as is.
* Syntactic suger is `'<Object>`

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // マクロ定義と関数定義に分ける。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        if (list_itr->IsSymbol()) {
          // マクロ定義。
          // バインドされるシンボル。
          std::string symbol = (list_itr++)->symbol_value();

          // バインドする値。
          if (!list_itr) {
            throw LispObject::GenInsufficientArgumentsError
            (func_name, required_args, true, list.Length() - 1);
          }
          LispObjectPtr value_ptr = caller.Evaluate(*list_itr);

          // 呼び出し元のスコープにバインド。
          caller.BindSymbol(symbol, value_ptr);

          // シンボルを返す。
          return LispObject::NewSymbol(symbol);
        } else if (list_itr->IsList()) {
          // 関数定義。
          LispIterator inner_itr(&(*(list_itr++)));

          // 第1引数内の第1要素は関数名。
          if (!(inner_itr->IsSymbol())) {
            throw LispObject::GenWrongTypeError(func_name, "Symbol",
            std::vector<int> {1, 1}, false);
          }
          std::string def_func_name = (inner_itr++)->symbol_value();

          // 第1引数内の第2要素以降は引数定義。
          std::vector<std::string> arg_name_vec;
          int index = 2;
          for (; inner_itr; ++inner_itr, ++index) {
            if (!(inner_itr->IsSymbol())) {
              throw LispObject::GenWrongTypeError(func_name, "Symbol",
              std::vector<int> {1, index}, false);
            }
            arg_name_vec.push_back(inner_itr->symbol_value());
          }

          // 関数定義をベクトルにする。
          std::vector<LispObjectPtr> def_vec;
          for (; list_itr; ++list_itr) {
            def_vec.push_back(list_itr->Clone());
          }

          // 関数オブジェクトを作成。
          LispObjectPtr func_obj = LispObject::NewFunction();
          func_obj->scope_chain(caller.scope_chain());
          func_obj->arg_name_vec(arg_name_vec);
          func_obj->def_vec(def_vec);

          // 呼び出し元のスコープにバインド。
          caller.BindSymbol(def_func_name, func_obj);

          return LispObject::NewSymbol(def_func_name);
        }

        throw LispObject::GenWrongTypeError(func_name, "List or Symbol",
        std::vector<int> {1}, false);
      };
      root_ptr->BindSymbol("define", func_ptr);
      (*dict_ptr)["define"] =
R"...(### define ###

__Usage__

1. `(define <Symbol> <Object>)`
2. `(define (<Name : Symbol> <Args : Symbol>...) <S-Expression>...)`

__Description__

* This is Special Form.
    + 1: `<Symbol>` isn't evaluated.
    + 2: All arguments isn't evaluated.
* Binds something to its scope.
* 1: Binds `<Object>` to `<Symbol>`.
* 2: Defines `<S-Expression>` as Function named `<Name>`,
     and `<Args>...` is names of its arguments.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数。 シンボルでなくてはならない。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        if (!(list_itr->IsSymbol())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Symbol", std::vector<int> {1}, false);
        }
        std::string symbol = (list_itr++)->symbol_value();

        // 書きかえる前の値を待避。 (シンボルが見つからなければエラー。)
        LispObjectPtr ret_ptr = caller.ReferSymbol(symbol)->Clone();

        // 第2引数を評価。
        LispObjectPtr value_ptr = caller.Evaluate(*list_itr);

        // シンボルを上書きする。
        caller.RewriteSymbol(symbol, value_ptr);

        // 書きかえる前を返す。
        return ret_ptr;
      };
      root_ptr->BindSymbol("set!", func_ptr);
      (*dict_ptr)["set!"] =
R"...(### set! ###

__Usage__

* `(set! <Symbol> <Object>)`

__Description__

* This is Special Form.
    + `<Symbol>` isn't evaluated.
* Updates `<Symbol>` to `<Object>` on the local scope.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 関数オブジェクトを作る。
        LispObjectPtr ret_ptr = LispObject::NewFunction();
        ret_ptr->scope_chain(caller.scope_chain());

        // 第1引数はリスト。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        if (!(list_itr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, false);
        }
        LispIterator first_itr(&(*(list_itr++)));

        // 引数リストを作成。
        std::vector<std::string> arg_name_vec;
        int index = 1;
        for (; first_itr; ++first_itr, ++index) {
          if (!(first_itr->IsSymbol())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Symbol", std::vector<int> {1, index}, false);
          }
          arg_name_vec.push_back(first_itr->symbol_value());
        }
        ret_ptr->arg_name_vec(arg_name_vec);

        // 関数定義をセット。
        std::vector<LispObjectPtr> def_vec;
        for (; list_itr; ++list_itr) {
          def_vec.push_back(list_itr->Clone());
        }
        ret_ptr->def_vec(def_vec);

        return ret_ptr;
      };
      root_ptr->BindSymbol("lambda", func_ptr);
      (*dict_ptr)["lambda"] =
R"...(### lambda ###

__Usage__

* `(lambda (<Args : Symbol>...) <S-Expression>...)`

__Description__

* This is Special Form.
    + All arguments isn't evaluated.
* Returns Function defined by `<S-Expression>...`.
* (lambda) inherits parent's scope and creates its own local scope.
  So using (lambda) in (lambda), you can create closure function.
* `<Args>...` is Symbols as name of arguments.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 自身にスコープチェーンを継承。
        // その後、ローカルスコープを作成。
        self->scope_chain(caller.scope_chain());
        self->NewLocalScope();

        // 第1引数(ローカル変数定義リスト)を得る。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        if (!(list_itr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, false);
        }
        LispIterator first_itr(&(*(list_itr++)));

        // ローカル変数定義をローカルスコープにバインドする。
        int index = 1;
        for (; first_itr; ++first_itr, ++index) {
          if (!(first_itr->IsList())) {
            throw LispObject::GenWrongTypeError
            (func_name, "List", std::vector<int> {1, index}, false);
          }

          // 定義のペア。
          if (first_itr->IsPair()) {
            // 変数名。
            if (!(first_itr->car()->IsSymbol())) {
              throw LispObject::GenWrongTypeError
              (func_name, "Symbol", std::vector<int> {1, index, 1}, false);
            }
            std::string var_name = first_itr->car()->symbol_value();

            // 変数の初期値。
            LispObjectPtr value = LispObject::NewNil();
            if (first_itr->cdr()->IsPair()) {
              value = caller.Evaluate(*(first_itr->cdr()->car()));
            }

            // バインド。
            self->BindSymbol(var_name, value);
          }
        }

        // 第3引数以降を自分のスコープで評価する。
        LispObjectPtr ret_ptr;
        for (; list_itr; ++list_itr) {
          ret_ptr = self->Evaluate(*list_itr);
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("let", func_ptr);
      (*dict_ptr)["let"] =
R"...(### let ###

__Usage__

* `(let ((<Name : Symbol> <Object>)...) <S-Expression>...)`

__Description__

* This is Special Form.
    + `<Name : Symbol>` isn't evaluated.
    + But `<Object>` and `<S-Expression>` are evaluated.
* Executes `<S-Expression>...` in new local scope.
* (let) inherits parent's scope and creates its own local scope.
  So using (lambda) in (let), you can create closure function.
* `(<Name> <Object>)...` is local values on (let)'s local scope.

__Example__

    (define (gen-func x y) (let ((a x) (b y))
              (lambda () (+ a b))))
    (define myfunc (gen-func 10 20))
    (display (myfunc))
    
    ;; Output
    ;;
    ;; > 30)...";
    }

    // %%% if
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 3;

        // 第1引数を評価。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {1}, true);
        }

        // resultが#tの場合。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        if (result->boolean_value()) {
          // 第2引数を評価。
          return caller.Evaluate(*list_itr);
        }

        // resultが#fの場合。
        // 第3引数を評価。
        ++list_itr;
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        return caller.Evaluate(*list_itr);
      };
      root_ptr->BindSymbol("if", func_ptr);
      (*dict_ptr)["if"] =
R"...(### if ###

__Usage__

* `(if <Condition : Boolean> <Then> <Else>)`

__Description__

* This is Special Form.
    + Either of `<Then>` and `<Else>` are evaluated.
* If `<Condition>` is true, then (if) evaluates `<Then>`.
  If false, then it evaluates `<Else>`.

__Example__

    (display (if (< 1 2) (+ 3 4) (+ 5 6)))
    
    ;; Output
    ;;
    ;; > 7)...";
    }

    // %%% cond
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        // 各条件リストを評価。
        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          // 各条件リストはリストでなくてはいけない。
          if (!(list_itr->IsList())) {
            throw LispObject::GenWrongTypeError
            (func_name, "List", std::vector<int> {index}, false);
          }

          // 各条件リストの中身を評価。
          LispIterator cond_list_itr(&(*list_itr));
          if (cond_list_itr) {
            // 空リストではない。
            // elseリストなら無条件で実行式を評価して終了。
            if (cond_list_itr->IsSymbol()
            && (cond_list_itr->symbol_value() == "else")) {
              ++cond_list_itr;
              if (cond_list_itr) {
                return caller.Evaluate(*cond_list_itr);
              }
              return LispObject::NewNil();
            }

            // 条件を評価する。
            LispObjectPtr result = caller.Evaluate(*(cond_list_itr++));
            if (!(result->IsBoolean())) {
              throw LispObject::GenWrongTypeError
              (func_name, "Boolean", std::vector<int> {index, 1}, true);
            }

            // #tなら実行式を評価して終了。
            if (result->boolean_value()) {
              if (cond_list_itr) {
                return caller.Evaluate(*cond_list_itr);
              }
              return LispObject::NewNil();
            }
          }
        }
        return LispObject::NewNil();
      };
      root_ptr->BindSymbol("cond", func_ptr);
      (*dict_ptr)["cond"] =
R"...(### cond ###

__Usage__

* `(cond (<Condition : Boolean> <Then>)... (else <Else>))`

__Description__

* This is Special Form.
    + Only one of `<Then>` or `<Else>` are evaluated.
    + `(else <Else>)` is a special list.
* If `<Condition>` is true, then (cond) returns `<Then>`.
  If false, then it evaluates next `<Condition>`.
* If all `<Condition>` are false, then (cond) returns `<Else>`.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        // 各引数を評価。
        LispObjectPtr ret_ptr = LispObject::NewNil();
        for (; list_itr; ++list_itr) {
          ret_ptr = caller.Evaluate(*list_itr);
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("begin", func_ptr);
      (*dict_ptr)["begin"] =
R"...(### begin ###

__Usage__

* `(begin <S-Expression>...)`

__Description__

* Executes `<S-Expression>...` in turns and returns last.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        // 表示する関数。
        std::function<void(const LispObject&)> print_display;
        print_display = [&print_display](const LispObject& obj) {
          if (obj.IsNil()) {
            std::cout << "()";
          } else if (obj.IsSymbol()) {
            std::cout << "Symbol: " << obj.symbol_value();
          } else if (obj.IsNumber()) {
            std::cout << obj.number_value();
          } else if (obj.IsBoolean()) {
            if (obj.boolean_value()) {
              std::cout << "#t";
            } else {
              std::cout << "#f";
            }
          } else if (obj.IsString()) {
            std::cout << obj.string_value();
          } else if (obj.IsList()) {
            std::cout << obj.ToString();
          } else if (obj.IsPair()) {
            std::cout << obj.ToString();
          } else if (obj.IsFunction()) {
            std::cout << "Function:";
            const std::vector<LispObjectPtr>& def_vec = obj.def_vec();
            for (auto& def : def_vec) {
              std::cout << " ";
              print_display(*def);
            }
          } else if (obj.IsNativeFunction()) {
            std::cout << "This is NativeFunction";
          }
        };

        // 表示。
        LispObjectPtr ret_ptr = LispObject::NewNil();
        for (; list_itr; ++list_itr) {
          ret_ptr = caller.Evaluate(*list_itr);
          print_display(*ret_ptr);
        }
        std::cout << std::endl;

        return ret_ptr;
      };
      root_ptr->BindSymbol("display", func_ptr);
      (*dict_ptr)["display"] =
R"...(### display ###

__Usage__

* `(display <Object>...)`

__Description__

* Prints `<Object>` on Standard Output.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // ストリームが閉じていたらNilを返す。
        if (!(std::cin)) return LispObject::NewNil();

        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数を調べる。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr message_ptr = caller.Evaluate(*(list_itr++));
        if (message_ptr->IsNil())       if (!(message_ptr->IsSymbol())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Symbol", std::vector<int> {1}, true);
        }

        // メッセージシンボルに合わせて読み方を変える。
        std::string message = message_ptr->symbol_value();
        std::string input_str = "";
        if (message == "@read") {
          char c;
          while (std::cin.get(c)) {
            input_str.push_back(c);
          }
        } else if (message == "@read-line") {
          std::getline(std::cin, input_str);
        } else if (message == "@get") {
          char c;
          std::cin.get(c);
          input_str.push_back(c);
        }

        return LispObject::NewString(input_str);
      };
      root_ptr->BindSymbol("stdin", func_ptr);
      (*dict_ptr)["stdin"] =
R"...(### stdin ###

__Usage__

* `(stdin <Message Symbol>)`

__Description__

* Returns String from Standard Input.
* `<Message Symbol>` is a message to the input stream.
    + `@get` : Reads one charactor.
    + `@read-line` : Reads one line. ('LF(CR+LF)' is omitted.)
    + `@read` : Reads all.
* If Standard Input is already closed, it returns Nil.

__Example__

    ;; Reads and shows one charactor from Standard Input.
    (display (stdin '@get))
    
    ;; Reads and shows one line from Standard Input.
    (display (stdin '@read-line))
    
    ;; Reads and shows all from Standard Input.
    (display (stdin '@read)))...";
    }

    // %%% stdout
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr); 
        if (!(result->IsString())) {
          throw LispObject::GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        std::cout << result->string_value() << std::flush;

        return self;
      };
      root_ptr->BindSymbol("stdout", func_ptr);
      (*dict_ptr)["stdout"] =
R"...(### stdout ###

__Usage__

* `(stdout <String>)`

__Description__

* Prints `<String>` on Standard Output.

__Example__

    (stdout (to-string 123))
    (stdout "\n")
    
    ;; Output
    ;;
    ;; > 123)...";
    }

    // %%% stderr
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr); 
        if (!(result->IsString())) {
          throw LispObject::GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        std::cerr << result->string_value() << std::flush;

        return self;
      };
      root_ptr->BindSymbol("stderr", func_ptr);
      (*dict_ptr)["stderr"] =
R"...(### stderr ###

__Usage__

* `(stderr <String>)`

__Description__

* Prints `<String>` on Standard Error.

__Example__

    (stderr (to-string 123))
    (stderr "\n")
    
    ;; Output
    ;;
    ;; > 123)...";
    }

    // %%% equal?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 2つのツリーを比べる関数を準備。
        std::function<bool(const LispObject&, const LispObject&)> compare;
        compare = [&compare]
        (const LispObject& left, const LispObject& right) -> bool {
          if (left.type() != right.type()) return false;

          if (!(left.IsPair())) {
            // Atomの場合。
            switch (left.type()) {
              case LispObjectType::NIL:
                return true;
              case LispObjectType::SYMBOL:
                return left.symbol_value() == right.symbol_value();
              case LispObjectType::NUMBER:
                return left.number_value() == right.number_value();
              case LispObjectType::BOOLEAN:
                return left.boolean_value() == right.boolean_value();
              case LispObjectType::STRING:
                return left.string_value() == right.string_value();
              case LispObjectType::FUNCTION:
                {
                  const std::vector<std::string>&
                  left_arg_name_vec = left.arg_name_vec();
                  const std::vector<std::string>&
                  right_arg_name_vec = right.arg_name_vec();

                  const std::vector<LispObjectPtr>&
                  left_def_vec = left.def_vec();
                  const std::vector<LispObjectPtr>&
                  right_def_vec = right.def_vec();

                  if (left_arg_name_vec.size() != right_arg_name_vec.size()) {
                    return false;
                  }

                  std::vector<std::string>::const_iterator
                  left_arg_name_itr = left_arg_name_vec.begin();
                  std::vector<std::string>::const_iterator
                  right_arg_name_itr = right_arg_name_vec.begin();

                  for (; (left_arg_name_itr != left_arg_name_vec.end())
                  && (left_arg_name_itr != left_arg_name_vec.end());
                  ++left_arg_name_itr, ++right_arg_name_itr) {
                    if (*left_arg_name_itr != *right_arg_name_itr) {
                      return false;
                    }
                  }

                  if (left_def_vec.size() != right_def_vec.size()) {
                    return false;
                  }

                  std::vector<LispObjectPtr>::const_iterator left_def_itr =
                  left_def_vec.begin();
                  std::vector<LispObjectPtr>::const_iterator right_def_itr =
                  right_def_vec.begin();
                  for (; (left_def_itr != left_def_vec.end())
                  && (right_def_itr != right_def_vec.end());
                  ++left_def_itr, ++right_def_itr) {
                    if (!compare(**left_def_itr, **right_def_itr)) {
                      return false;
                    }
                  }
                  return true;
                }
              case LispObjectType::NATIVE_FUNCTION:
                throw LispObject::GenError("@runtime-error",
                "Native Function can't be compared,"
                " because there are many types of function.");
              default :
                return false;
            }
          }

          // Pairの場合。
          if (!compare(*(left.car()), *(right.car()))) {
            return false;
          }
          if (!compare(*(left.cdr()), *(right.cdr()))) {
            return false;
          }

          return true;
        };

        // compareを使って各引数をチェック。
        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr first_ptr = caller.Evaluate(*(list_itr++));

        for (; list_itr; ++list_itr) {
          if (!compare(*first_ptr, *(caller.Evaluate(*list_itr)))) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("equal?", func_ptr);
      (*dict_ptr)["equal?"] =
R"...(### equal? ###

__Usage__

* `(equal? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are same structure.
  Otherwise, returns #f.

__Example__

    (display (equal? '(1 2 (3 4) 5) '(1 2 (3 4) 5)))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% pair?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数がLispPairならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsPair())) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("pair?", func_ptr);
      (*dict_ptr)["pair?"] =
R"...(### pair? ###

__Usage__

* `(pair? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are Pair.
  Otherwise, returns #f.

__Example__

    (display (pair? '(1 2 3) '(4 5 6)))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% list?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数がListならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsList())) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("list?", func_ptr);
      (*dict_ptr)["list?"] =
R"...(### list? ###

__Usage__

* `(list? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are List.
  Otherwise, returns #f.

__Example__

    (display (list? '(1 2 3) '(4 5 6) ()))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% nil?
    // %%% null?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数がNilならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsNil())) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("nil?", func_ptr);
      root_ptr->BindSymbol("null?", func_ptr);
      std::string temp =
R"...(### nil? ###

__Usage__

* `(nil? <Object>...)`
* `(null? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are Nil.
  Otherwise, returns #f.

__Example__

    (display (nil? ()))
    
    ;; Output
    ;;
    ;; > #t)...";
      (*dict_ptr)["nil?"] = temp;
      (*dict_ptr)["null?"] = temp;
    }

    // %%% symbol?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数がシンボルならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsSymbol())){
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("symbol?", func_ptr);
      (*dict_ptr)["symbol?"] =
R"...(### symbol? ###

__Usage__

* `(symbol? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are Symbol.
  Otherwise, returns #f.

__Example__

    (display (symbol? 'x))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% number?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数が数字ならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsNumber())) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("number?", func_ptr);
      (*dict_ptr)["number?"] =
R"...(### number? ###

__Usage__

* `(number? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are Number.
  Otherwise, returns #f.

__Example__

    (display (number? 123))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% boolean?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数がBooleanならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsBoolean())) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("boolean?", func_ptr);
      (*dict_ptr)["boolean?"] =
R"...(### boolean? ###

__Usage__

* `(boolean? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are Boolean.
  Otherwise, returns #f.

__Example__

    (display (boolean? #f))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% string?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数が文字列ならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsString())) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("string?", func_ptr);
      (*dict_ptr)["string?"] =
R"...(### string? ###

__Usage__

* `(string? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are String.
  Otherwise, returns #f.

__Example__

    (display (string? "Hello"))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% function?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数が関数オブジェクトならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsFunction())) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("function?", func_ptr);
      (*dict_ptr)["function?"] =
R"...(### function? ###

__Usage__

* `(function? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are Function.
  Otherwise, returns #f.

__Example__

    (define myfunc (lambda (x) (+ x 1)))
    (display (function? myfunc))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% native-function?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数がネイティブ関数オブジェクトならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!(result->IsNativeFunction())) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("native-function?", func_ptr);
      (*dict_ptr)["native-function?"] =
R"...(### native-function? ###

__Usage__

* `(native-function? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are Native Function.
  Otherwise, returns #f.

__Example__

    (display (native-function? +))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% procedure?
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        LispObjectPtr ret_ptr = LispObject::NewBoolean(true);

        // 全ての引数が関数、又はネイティブ関数オブジェクトならtrue。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        for (; list_itr; ++list_itr) {
          LispObjectPtr result = caller.Evaluate(*list_itr);

          if (!((result->IsFunction()) || (result->IsNativeFunction()))) {
            ret_ptr->boolean_value(false);
            break;
          }
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("procedure?", func_ptr);
      (*dict_ptr)["procedure?"] =
R"...(### procedure? ###

__Usage__

* `(procedure? <Object>...)`

__Description__

* Returns #t if all `<Object>...` are Function or Native Function.
  Otherwise, returns #f.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsString())) {
          throw LispObject::GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // ヒープにファイルを開く。
        std::shared_ptr<std::ofstream>
        stream_ptr(new std::ofstream(result->string_value()));
        if (!(*stream_ptr)) {
          throw LispObject::GenError
          ("@not-open-stream", "Couldn't open output stream.");
        }

        // ネイティブ関数オブジェクトを作成。
        LispObjectPtr ret_ptr = LispObject::NewNativeFunction();
        ret_ptr->scope_chain(caller.scope_chain());
        ret_ptr->native_function
        ([stream_ptr](LispObjectPtr self, const LispObject& caller,
        const LispObject& list) -> LispObjectPtr {
          // 準備。
          LispIterator list_itr(&list);
          std::string func_name = (list_itr++)->ToString();
          int required_args = 1;

          if (!list_itr) {
            throw LispObject::GenInsufficientArgumentsError
            (func_name, required_args, false, list.Length() - 1);
          }
          LispObjectPtr result = caller.Evaluate(*list_itr);

          // Nilならファイルを閉じる。
          if (result->IsNil()) {
            if (*(stream_ptr)) stream_ptr->close();
            return self;
          }

          // ファイルを書き込む。
          if (!(result->IsString())) {
            throw LispObject::GenWrongTypeError
            (func_name, "String or Nil", std::vector<int> {1}, true);
          }

          if (*stream_ptr) {
            *stream_ptr << result->string_value() << std::flush;
          }

          return self;
        });

        return ret_ptr;
      };
      root_ptr->BindSymbol("output-stream", func_ptr);
      (*dict_ptr)["output-stream"] =
R"...(### output-stream ###

__Usage__

1. `(output-stream <File name : String>)`
2. `((output-stream <File name : String>) <String>)`

__Description__

* 1: Returns Native Function as an output stream of `<File name>`.
* 2: Writes `<String>` to `<File name>` and returns itself.
* If you give Nil to the Function, the stream will be closed.

__Example__

    ;; Opens "hello.txt".
    (define myfile (output-stream "hello.txt"))
    
    ;; Writes "Hello World" to "hello.txt".
    (myfile "Hello World\n")
    
    ;; Closes "hello.txt".
    (myfile ()))...";
    }

    // %%% input-stream
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsString())) {
          throw LispObject::GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // ヒープにファイルを開く。
        std::shared_ptr<std::ifstream>
        stream_ptr(new std::ifstream(result->string_value()));
        if (!(*stream_ptr)) {
          throw LispObject::GenError
          ("@not-open-stream", "Couldn't open input stream.");
        }

        // ネイティブ関数オブジェクトを作成。
        LispObjectPtr ret_ptr = LispObject::NewNativeFunction();
        ret_ptr->scope_chain(caller.scope_chain());
        ret_ptr->native_function
        ([stream_ptr](LispObjectPtr self, const LispObject& caller,
        const LispObject& list) -> LispObjectPtr {
          // ストリームが閉じていたらNilを返す。
          if (!(*stream_ptr)) return LispObject::NewNil();

          // 準備。
          LispIterator list_itr(&list);
          std::string func_name = (list_itr++)->ToString();
          int required_args = 1;

          // 引数を調べる。
          if (!list_itr) {
            throw LispObject::GenInsufficientArgumentsError
            (func_name, required_args, false, list.Length() - 1);
          }
          LispObjectPtr message_ptr = caller.Evaluate(*(list_itr++));
          if (message_ptr->IsNil()) {
            // Nilの場合は閉じる。
            stream_ptr->close();
            return LispObject::NewNil();
          }
          if (!(message_ptr->IsSymbol())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Symbol or Nil", std::vector<int> {1}, true);
          }

          // メッセージシンボルに合わせて読み方を変える。
          std::string message = message_ptr->symbol_value();
          std::string input_str = "";
          if (message == "@read") {
            char c;
            while (stream_ptr->get(c)) {
              input_str.push_back(c);
            }
          } else if (message == "@read-line") {
            std::getline(*stream_ptr, input_str);
          } else if (message == "@get") {
            char c;
            stream_ptr->get(c);
            input_str.push_back(c);
          }

          return LispObject::NewString(input_str);
        });

        return ret_ptr;
      };
      root_ptr->BindSymbol("input-stream", func_ptr);
      (*dict_ptr)["input-stream"] =
R"...(### input-stream ###

__Usage__

1. `(input-stream <File name : String>)`
2. `((input-stream <File name : String>) <Message Symbol : Symbol>)`

__Description__

* 1: Returns Native Function as an input stream of `<File name>`.
* 2: Returns String from `<File name>`.
* 2: `<Message Symbol>` is a message to the input stream.
    + `@get` : Reads one charactor.
    + `@read-line` : Reads one line. ('LF(CR+LF)' is omitted.)
    + `@read` : Reads all.
* If you give Nil to the stream, it will be closed.
* If the stream already closed, it returns empty string.

__Example__

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

  // 基本的なネイティブ関数をセットする。
  void LispObject::SetBasicFunctions(LispObjectPtr root_ptr,
  std::shared_ptr<HelpDict> dict_ptr) {
    // %%% append
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr ret_ptr = caller.Evaluate(*(list_itr++));
        if (!(ret_ptr->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        // 第2引数を追加。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        ret_ptr->Append(caller.Evaluate(*list_itr));

        return ret_ptr;
      };
      root_ptr->BindSymbol("append", func_ptr);
      (*dict_ptr)["append"] =
R"...(### append ###

__Usage__

* `(append <List 1> <List 2>)`

__Description__

* Appends `<List 2>` after the end of `<List 1>`.
* In other words,
  (append) replaces last Cdr of `<List 1>` from Nil to `<List 2>`.

__Example__

    (display (append '(111 222 333) '(444 555 666)))
    
    ;; Output
    ;;
    ;; > (111 222 333 444 555 666)
    
    (display (append '(111 222 333) "Not List"))
    
    ;; Output
    ;;
    ;; > (111 222 333 . "Not List"))...";
    }

    // %%% list
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        // 作成する大きさのリストを作りながらセットしていく。
        LispObjectPtr ret_ptr = LispObject::NewNil();
        LispObject* ptr = ret_ptr.get();
        for (; list_itr; ++list_itr) {
          ptr->type(LispObjectType::PAIR);
          ptr->car(caller.Evaluate(*list_itr));
          ptr->cdr(LispObject::NewNil());

          ptr = ptr->cdr().get();
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("list", func_ptr);
      (*dict_ptr)["list"] =
R"...(### list ###


__Usage__

* `(list <Object>...)`

__Description__

* Returns List consists of `<Object>...`.

__Example__

    (display (list 111 222 333))
    
    ;; Output
    ;;
    ;; > (111 222 333))...";
    }

    // %%% list-ref
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。 インデックス。 0が最初。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr index_ptr = caller.Evaluate(*list_itr);
        if (!(index_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        // 目的の要素を得る。
        int index = index_ptr->number_value();
        index = index < 0 ? result->Length() + index : index;
        int i = 0;
        for (LispIterator result_itr(result.get());
        result_itr; ++result_itr, ++i) {
          if (i == index) return result_itr->Clone();
        }

        // 範囲外。
        throw LispObject::GenError("@out-of-range",
        "The index number of (" + func_name + ") is out of range.");
      };
      root_ptr->BindSymbol("list-ref", func_ptr);
      (*dict_ptr)["list-ref"] =
R"...(### list-ref ###

__Usage__

* `(list-ref <List> <Index : Number>)`

__Description__

* Returns a element of `<Index>` of `<List>`.
* The 1st element of `<List>` is 0.
* If `<Index>` is negative number," It counts from the tail of `<List>`.

__Example__

    (display (list-ref (111 222 333) 1))
    
    ;; Output
    ;;
    ;; > 222
    
    (display (list-ref (111 222 333) -1))
    
    ;; Output
    ;;
    ;; > 333)...";
    }

    // %%% list-replace
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 3;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。 インデックス。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr index_ptr = caller.Evaluate(*(list_itr++));
        if (!(index_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        // 第3引数を得る。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr replace_obj_ptr = caller.Evaluate(*list_itr);

        // インデックスの要素を入れ替える。
        int index = index_ptr->number_value();
        index = index < 0 ? result->Length() + index : index;
        int i = 0;
        for (LispIterator result_itr(result.get());
        result_itr; ++result_itr, ++i) {
          if (i == index) {
            *result_itr = *replace_obj_ptr;
            return result;
          }
        }

        // 範囲外。
        throw LispObject::GenError("@out-of-range",
        "The index number of (" + func_name + ") is out of range.");
      };
      root_ptr->BindSymbol("list-replace", func_ptr);
      (*dict_ptr)["list-replace"] =
R"...(### list-replace ###

__Usage__

* `(list-replace <List> <Index : Number> <Object>)`

__Description__

* Returns List which has replaced the `<Index>`th element of
  `<List>` for `<Object>`.
* The 1st element of `<List>` is 0.
* If `<Index>` is negative number," It counts from the tail of `<List>`.

__Example__

    (define lst (list 111 222 333))
    (display (list-replace lst 1 "Hello"))
    
    ;; Output
    ;;
    ;; > (111 "Hello" 333))...";
    }

    // %%% list-remove
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsList())) {
          throw LispObject::GenWrongTypeError
          (func_name, "List", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。 インデックス。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr index_ptr = caller.Evaluate(*list_itr);
        if (!(index_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        // インデックスの要素を削除する。 後ろの要素を1つ前に持ってくる。
        int index = index_ptr->number_value();
        index = index < 0 ? result->Length() + index : index;
        int i = 0;
        for (LispObject* ptr = result.get();
        ptr->IsPair(); ptr = ptr->cdr().get(), ++i) {
          if (i == index) {
            *ptr = *(ptr->cdr());
            return result;
          }
        }

        // 範囲外。
        throw LispObject::GenError("@out-of-range",
        "The index number of (" + func_name + ") is out of range.");
      };
      root_ptr->BindSymbol("list-remove", func_ptr);
      (*dict_ptr)["list-remove"] =
R"...(### list-remove ###

__Usage__

* `(list-remove <List> <Index : Number>)`

__Description__

* Returns List which has removed the `<Index>`th element of `<List>`.
* The 1st element of `<List>` is 0.
* If `<Index>` is negative number," It counts from the tail of `<List>`.

__Example__

    (define lst (list 111 222 333))
    (display (list-remove lst 1))
    
    ;; Output
    ;;
    ;; > (111 333))...";
    }

    // %%% length
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);

        // Atomの場合。
        if (!(result->IsPair())) {
          if (result->IsNil()) {
            return LispObject::NewNumber(0);
          }
          return LispObject::NewNumber(1);
        }

        // Pairの場合。
        LispIterator result_itr(&(*result));
        int count = 0;
        for (; result_itr; ++result_itr) {
          ++count;
        }
        if (!(result_itr.current().IsNil())) {
          ++count;
        }

        return LispObject::NewNumber(count);
      };
      root_ptr->BindSymbol("length", func_ptr);
      (*dict_ptr)["length"] =
R"...(### length ###

__Usage__

* `(length <List>)`

__Description__

* Returns number of `<List>`.
* If you input Atom, it returns 1. If Nil, it returns 0.

__Example__

    (display (length '(111 222 333 444 555 666)))
    
    ;; Output
    ;;
    ;; > 6)...";
    }

    // %%% =
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 最初の数字を得る。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr first_ptr = caller.Evaluate(*(list_itr++));
        if (!(first_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }
        double first = first_ptr->number_value();

        // 2つ目以降の引数と比較する。
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          // 調べたい数字を得る。
          LispObjectPtr current_ptr = caller.Evaluate(*list_itr);
          if (!(current_ptr->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "List", std::vector<int> {index}, true);
          }

          // 比較。
          double current = current_ptr->number_value();
          if (current != first) {
            return LispObject::NewBoolean(false);
          }
        }

        return LispObject::NewBoolean(true);
      };
      root_ptr->BindSymbol("=", func_ptr);
      (*dict_ptr)["="] =
R"...(### = ###

__Usage__

* `(= <Number>...)`

__Description__

* Returns #t if all `<Number>...` are same.
  Otherwise, return #f.

__Example__

    (display (= 111 111 111))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% <
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 最初の数字を得る。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }
        double prev = result->number_value();

        // 2つ目以降の引数と比較する。
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          // 調べたい数字を得る。
          LispObjectPtr current_ptr = caller.Evaluate(*list_itr);
          if (!(current_ptr->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          // 比較。
          double current = current_ptr->number_value();
          if (!(prev < current)) {
            return LispObject::NewBoolean(false);
          }

          prev = current;
        }

        return LispObject::NewBoolean(true);
      };
      root_ptr->BindSymbol("<", func_ptr);
      (*dict_ptr)["<"] =
R"...(### < ###

__Usage__

* `(< <Number>...)`

__Description__

* Returns #t if a Number is less than next Number.
  Otherwise, return #f.

__Example__

    (display (< 111 222 333))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% <=
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 最初の数字を得る。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }
        double prev = result->number_value();

        // 2つ目以降の引数と比較する。
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          // 調べたい数字を得る。
          LispObjectPtr current_ptr = caller.Evaluate(*list_itr);
          if (!(current_ptr->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          // 比較。
          double current = current_ptr->number_value();
          if (!(prev <= current)) {
            return LispObject::NewBoolean(false);
          }

          prev = current;
        }

        return LispObject::NewBoolean(true);
      };
      root_ptr->BindSymbol("<=", func_ptr);
      (*dict_ptr)["<="] =
R"...(### <= ###

__Usage__

* `(<= <Number>...)`

__Description__

* Returns #t if a Number is less or equal than next Number.
  Otherwise, return #f.

__Example__

    (display (< 111 222 333))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% >
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 最初の数字を得る。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }
        double prev = result->number_value();

        // 2つ目以降の引数と比較する。
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          // 調べたい数字を得る。
          LispObjectPtr current_ptr = caller.Evaluate(*list_itr);
          if (!(current_ptr->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          // 比較。
          double current = current_ptr->number_value();
          if (!(prev > current)) {
            return LispObject::NewBoolean(false);
          }

          prev = current;
        }

        return LispObject::NewBoolean(true);
      };
      root_ptr->BindSymbol(">", func_ptr);
      (*dict_ptr)[">"] =
R"...(### > ###

__Usage__

* `(> <Number>...)`

__Description__

* Returns #t if a Number is more than next Number.
  Otherwise, return #f.

__Example__

    (display (> 333 222 111))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% >=
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 最初の数字を得る。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }
        double prev = result->number_value();

        // 2つ目以降の引数と比較する。
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          // 調べたい数字を得る。
          LispObjectPtr current_ptr = caller.Evaluate(*list_itr);
          if (!(current_ptr->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          // 比較。
          double current = current_ptr->number_value();
          if (!(prev >= current)) {
            return LispObject::NewBoolean(false);
          }

          prev = current;
        }

        return LispObject::NewBoolean(true);
      };
      root_ptr->BindSymbol(">=", func_ptr);
      (*dict_ptr)[">="] =
R"...(### >= ###

__Usage__

* `(>= <Number>...)`

__Description__

* Returns #t if a Number is more or equal than next Number.
  Otherwise, return #f.

__Example__

    (display (>= 333 222 111))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% not
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 引数を評価してみる。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsBoolean())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Boolean", std::vector<int> {1}, true);
        }

        return LispObject::NewBoolean(!(result->boolean_value()));
      };
      root_ptr->BindSymbol("not", func_ptr);
      (*dict_ptr)["not"] =
R"...(### not ###

__Usage__

* `(not <Boolean>)`

__Description__

* Turns `<Boolean>` to opposite value. #t to #f, #f to #t.

__Example__

    (display (not (= 111 111)))
    
    ;; Output
    ;;
    ;; > #f)...";
    }

    // %%% and
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 各引数を調べる。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsBoolean())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Boolean", std::vector<int> {index}, true);
          }

          if (!(result->boolean_value())) {
            return LispObject::NewBoolean(false);
          }
        }
        return LispObject::NewBoolean(true);
      };
      root_ptr->BindSymbol("and", func_ptr);
      (*dict_ptr)["and"] =
R"...(### and ###

__Usage__

* `(and <Boolean>...)`

__Description__

* Returns #t if all `<Boolean>...` are #t.
  Otherwise, return #f.

__Example__

    (display (and (= 111 111) (= 222 222)))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% or
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 各引数を調べる。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsBoolean())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Boolean", std::vector<int> {index}, true);
          }

          if (result->boolean_value()) {
            return LispObject::NewBoolean(true);
          }
        }
        return LispObject::NewBoolean(false);
      };
      root_ptr->BindSymbol("or", func_ptr);
      (*dict_ptr)["or"] =
R"...(### or ###

__Usage__

* `(or <Boolean>...)`

__Description__

* Returns #t if one of `<Boolean>...` is #t.
  If all `<Boolean>` are #f, return #f.

__Example__

    (display (or (= 111 111) (= 222 333)))
    
    ;; Output
    ;;
    ;; > #t)...";
    }

    // %%% +
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      -> LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        double value = 0.0;

        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          value += result->number_value();
        }
        return LispObject::NewNumber(value);
      };
      root_ptr->BindSymbol("+", func_ptr);
      (*dict_ptr)["+"] =
R"...(### + ###

__Usage__

* `(+ <Number>...)`

__Description__

* Sums up all `<Number>...`.

__Example__

    (display (+ 1 2 3))
    
    ;; Output
    ;;
    ;; > 6)...";
    }

    // %%% -
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        double value = 0.0;

        if (list_itr) {
          LispObjectPtr result = caller.Evaluate(*(list_itr++));
          if (!(result->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {1}, true);
          }
          value = result->number_value();
        }
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          value -= result->number_value();
        }
        return LispObject::NewNumber(value);
      };
      root_ptr->BindSymbol("-", func_ptr);
      (*dict_ptr)["-"] =
R"...(### - ###

__Usage__

* `(- <1st number> <Number>...)`

__Description__

* Subtracts `<Number>...` from `<1st number>`.

__Example__

    (display (- 5 4 3))
    
    ;; Output
    ;;
    ;; > -2)...";
    }

    // %%% *
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        double value = 1.0;

        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          value *= result->number_value();
        }
        return LispObject::NewNumber(value);
      };
      root_ptr->BindSymbol("*", func_ptr);
      (*dict_ptr)["*"] =
R"...(### * ###

__Usage__

* `(* <Number>...)`

__Description__

* Multiplies all `<Number>...`.

__Example__

    (display (* 2 3 4))
    
    ;; Output
    ;;
    ;; > 24)...";
    }

    // %%% /
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        double value = 0.0;

        if (list_itr) {
          LispObjectPtr result = caller.Evaluate(*(list_itr++));
          if (!(result->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {1}, true);
          }
          value = result->number_value();
        }
        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          value /= result->number_value();
        }
        return LispObject::NewNumber(value);
      };
      root_ptr->BindSymbol("/", func_ptr);
      (*dict_ptr)["/"] =
R"...(### / ###

__Usage__

* `(/ <1st number> <Number>...)`

__Description__

* Divides `<1st number>` with `<Number>...`.

__Example__

    (display (/ 32 2 4))
    
    ;; Output
    ;;
    ;; > 4)...";
    }

    // %%% string-append
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();

        std::ostringstream stream;
        int index = 1;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr result = caller.Evaluate(*list_itr);
          if (!(result->IsString())) {
            throw LispObject::GenWrongTypeError
            (func_name, "String", std::vector<int> {1}, true);
          }

          stream << result->string_value();
        }

        return LispObject::NewString(stream.str());
      };
      root_ptr->BindSymbol("string-append", func_ptr);
      (*dict_ptr)["string-append"] =
R"...(### string-append ###

__Usage__

* `(string-append <String>...)`

__Description__

* Concatenates `<String>...`.

__Example__

    (display (string-append "Hello" " " "World"))
    
    ;; Output
    ;;
    ;; > Hello World)...";
    }

    // %%% string-ref
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsString())) {
          throw LispObject::GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。 インデックス。 0が最初。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr index_ptr = caller.Evaluate(*list_itr);
        if (!(index_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        // 指定の文字を得る。
        std::string result_str = result->string_value();
        int index = index_ptr->number_value();
        index = index < 0 ? result_str.size() + index : index;
        if ((index >= 0) && (index < static_cast<int>(result_str.size()))) {
          return LispObject::NewString(std::string {result_str[index]});
        }

        return LispObject::NewString("");
      };
      root_ptr->BindSymbol("string-ref", func_ptr);
      (*dict_ptr)["string-ref"] =
R"...(### string-ref ###

__Usage__

* `(string-ref <String> <index>)`

__Description__

* Returns the `<index>`th letter of `<String>`.
* `<index>` of the 1st letter is 0.

__Example__

    (display (string-ref "Hello World" 6))
    
    ;; Output
    ;;
    ;; > W)...";
    }

    // %%% string-split
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*(list_itr++));
        if (!(result->IsString())) {
          throw LispObject::GenWrongTypeError
          (func_name, "String", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。 区切り文字列。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr delim_ptr = caller.Evaluate(*list_itr);
        if (!(delim_ptr->IsString())) {
          throw LispObject::GenWrongTypeError
          (func_name, "String", std::vector<int> {2}, true);
        }

        // 区切る。
        std::string origin = result->string_value();
        std::string delim = delim_ptr->string_value();
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
        LispObjectPtr ret_ptr = LispObject::NewNil();
        LispObject* ptr = ret_ptr.get();
        for (auto& str : str_vec) {
          *ptr = *(LispObject::NewPair
          (LispObject::NewString(str), LispObject::NewNil()));

          ptr = ptr->cdr().get();
        }

        return ret_ptr;
      };
      root_ptr->BindSymbol("string-split", func_ptr);
      (*dict_ptr)["string-split"] =
R"...(### string-split ###

__Usage__

* `(string-split <String> <Delim String>)`

__Description__

* Returns List consist of split `<String>` by `<Delim String>`.

__Example__

    (display (string-split "aaaaSplit!bbbSplit!ccc" "Split!"))
    
    ;; Outpu
    ;;
    ;; > ("aaa" "bbb" "ccc"))...";
    }

    // %%% PI
    root_ptr->BindSymbol("PI", NewNumber(3.141592653589793));
    (*dict_ptr)["PI"] =
R"...(### PI ###

__Description__

* Circular constant.

__Example__

    (display (* 1 PI))
    
    ;; Output
    ;;
    ;; > 3.14159)...";

    // %%% E
    root_ptr->BindSymbol("E", NewNumber(2.718281828459045));
    (*dict_ptr)["E"] =
R"...(### E ###

__Description__

* Napier's constant.

__Example__

    (display (* 1 E))
    
    ;; Output
    ;;
    ;; > 2.71828)...";

    // %%% sin
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::sin(result->number_value()));
      };
      root_ptr->BindSymbol("sin", func_ptr);
      (*dict_ptr)["sin"] =
R"...(### sin ###

__Usage__

* `(sin <Number>)`

__Description__

* Sine. A trigonometric function.
* `<Number>` is radian.

__Example__

    (display (sin (/ PI 2)))
    
    ;; Output
    ;;
    ;; > 1)...";
    }

    // %%% cos
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::cos(result->number_value()));
      };
      root_ptr->BindSymbol("cos", func_ptr);
      (*dict_ptr)["cos"] =
R"...(### cos ###

__Usage__

* `(cos <Number>)`

__Description__

* Cosine. A trigonometric function.
* `<Number>` is radian.

__Example__

    (display (cos PI))
    
    ;; Output
    ;;
    ;; > -1)...";
    }

    // %%% tan
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::tan(result->number_value()));
      };
      root_ptr->BindSymbol("tan", func_ptr);
      (*dict_ptr)["tan"] =
R"...(### tan ###

__Usage__

* `(tan <Number>)`

__Description__

* Tangent. A trigonometric function.
* `<Number>` is radian.

__Example__

    (display (tan (/ PI 4)))
    
    ;; Output
    ;;
    ;; > 1)...";
    }

    // %%% asin
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::asin(result->number_value()));
      };
      root_ptr->BindSymbol("asin", func_ptr);
      (*dict_ptr)["asin"] =
R"...(### asin ###

__Usage__

* `(asin <Number>)`

__Description__

* Arc sine. A trigonometric function.
* `<Number>` is sine.

__Example__

    (display (asin 0))
    
    ;; Output
    ;;
    ;; > 0)...";
    }

    // %%% acos
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::acos(result->number_value()));
      };
      root_ptr->BindSymbol("acos", func_ptr);
      (*dict_ptr)["acos"] =
R"...(### acos ###

__Usage__

* `(acos <Number>)`

__Description__

* Arc cosine. A trigonometric function.
* `<Number>` is cosine.

__Example__

    (display (acos 1))
    
    ;; Output
    ;;
    ;; > 0)...";
    }

    // %%% atan
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::atan(result->number_value()));
      };
      root_ptr->BindSymbol("atan", func_ptr);
      (*dict_ptr)["atan"] =
R"...(### atan ###

__Usage__

* `(atan <Number>)`

__Description__

* Arc tangent. A trigonometric function.
* `<Number>` is tangent.

__Example__

    (display (atan 0))
    
    ;; Output
    ;;
    ;; > 0)...";
    }

    // %%% sqrt
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::sqrt(result->number_value()));
        // 引数をチェック(result->number_value()));
      };
      root_ptr->BindSymbol("sqrt", func_ptr);
      (*dict_ptr)["sqrt"] =
R"...(### sqrt ###

__Usage__

* `(sqrt <Number>)`

__Description__

* Returns square root of `<Number>`.

__Example__

    (display (sqrt 4))
    
    ;; Output
    ;;
    ;; > 2)...";
    }

    // %%% abs
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::abs(result->number_value()));
      };
      root_ptr->BindSymbol("abs", func_ptr);
      (*dict_ptr)["abs"] =
R"...(### abs ###

__Usage__

* `(abs <Number>)`

__Description__

* Returns absolute value of `<Number>`.

__Example__

    (display (abs -111))
    
    ;; Output
    ;;
    ;; > 111)...";
    }

    // %%% ceil
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::ceil(result->number_value()));
      };
      root_ptr->BindSymbol("ceil", func_ptr);
      (*dict_ptr)["ceil"] =
R"...(### ceil ###

__Usage__

* `(ceil <Number>)`

__Description__

* Rounds up `<Number>` into integral value.

__Example__

    (display (ceil 1.3))
    
    ;; Output
    ;;
    ;; > 2)...";
    }

    // %%% floor
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::floor(result->number_value()));
      };
      root_ptr->BindSymbol("floor", func_ptr);
      (*dict_ptr)["floor"] =
R"...(### floor ###

__Usage__

* `(floor <Number>)`

__Description__

* Rounds down `<Number>` into integral value.

__Example__

    (display (floor 1.3))
    
    ;; Output
    ;;
    ;; > 1)...";
    }

    // %%% round
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::round(result->number_value()));
      };
      root_ptr->BindSymbol("round", func_ptr);
      (*dict_ptr)["round"] =
R"...(### round ###

__Usage__

* `(round <Number>)`

__Description__

* Rounds `<Number>` into the nearest integral value.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::trunc(result->number_value()));
      };
      root_ptr->BindSymbol("trunc", func_ptr);
      (*dict_ptr)["trunc"] =
R"...(### trunc ###

__Usage__

* `(trunc <Number>)`

__Description__

* Truncates after decimal point of `<Number>`.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::exp(result->number_value()));
      };
      root_ptr->BindSymbol("exp", func_ptr);
      (*dict_ptr)["exp"] =
R"...(### exp ###

__Usage__

* `(exp <Number>)`

__Description__

* Exponent function of `<Number>`. The base is Napier's constant.

__Example__

    (display (exp 1))
    
    ;; Output
    ;;
    ;; > 2.71828)...";
    }

    // %%% expt
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 2;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr first_ptr = caller.Evaluate(*(list_itr++));
        if (!(first_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        // 第2引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr second_ptr = caller.Evaluate(*list_itr);
        if (!(second_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {2}, true);
        }

        return LispObject::NewNumber
        (std::pow(first_ptr->number_value(), second_ptr->number_value()));
      };
      root_ptr->BindSymbol("expt", func_ptr);
      (*dict_ptr)["expt"] =
R"...(### expt ###

__Usage__

* `(expt <Base> <Exponent>)`

__Description__

* Exponent function of `<Exponent>`. The base is `<Base>`.

__Example__

    (display (expt 2 3))
    
    ;; Output
    ;;
    ;; > 8)...";
    }

    // %%% log
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::log(result->number_value()));
      };
      root_ptr->BindSymbol("log", func_ptr);
      (*dict_ptr)["log"] =
R"...(### log ###

__Usage__

* `(log <Number>)`

__Description__

* Logarithmic function of `<Number>`. The base is Napier's constant.

__Example__

    (display (log E))
    
    ;; Output
    ;;
    ;; > 1)...";
    }

    // %%% log2
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::log2(result->number_value()));
      };
      root_ptr->BindSymbol("log2", func_ptr);
      (*dict_ptr)["log2"] =
R"...(### log2 ###

__Usage__

* `(log2 <Number>)`

__Description__

* Logarithmic function of `<Number>`. The base is 2.

__Example__

    (display (log2 8))
    
    ;; Output
    ;;
    ;; > 3)...";
    }

    // %%% log10
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        return LispObject::NewNumber(std::log10(result->number_value()));
      };
      root_ptr->BindSymbol("log10", func_ptr);
      (*dict_ptr)["log10"] =
R"...(### log10 ###

__Usage__

* `(log10 <Number>)`

__Description__

* Logarithmic function of `<Number>`. The base is 10.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [engine_ptr](LispObjectPtr self, const LispObject& caller,
      const LispObject& list) ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数をチェック。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, false, list.Length() - 1);
        }
        LispObjectPtr result = caller.Evaluate(*list_itr);
        if (!(result->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        std::uniform_real_distribution<> dist(0, result->number_value());
        return LispObject::NewNumber(dist(*engine_ptr));
      };
      root_ptr->BindSymbol("random", func_ptr);
      (*dict_ptr)["random"] =
R"...(### random ###

__Usage__

* `(random <Number>)`

__Description__

* Returns random number from 0 to `<Number>`.

__Example__

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
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr max_ptr = caller.Evaluate(*(list_itr++));
        if (!(max_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr current = caller.Evaluate(*list_itr);
          if (!(current->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          if (current->number_value() > max_ptr->number_value()) {
            max_ptr = current;
          }
        }

        return max_ptr;
      };
      root_ptr->BindSymbol("max", func_ptr);
      (*dict_ptr)["max"] =
R"...(### max ###

__Usage__

* `(max <Number>...)`

__Description__

* Returns maximum number of `<Number>...`.

__Example__

    (display (max 1 2 3 4 3 2 1))
    
    ;; Output
    ;;
    ;; > 4)...";
    }

    // %%% min
    {
      LispObjectPtr func_ptr = LispObject::NewNativeFunction();
      func_ptr->scope_chain_ = root_ptr->scope_chain_;
      func_ptr->native_function_ =
      [](LispObjectPtr self, const LispObject& caller, const LispObject& list)
      ->LispObjectPtr {
        // 準備。
        LispIterator list_itr(&list);
        std::string func_name = (list_itr++)->ToString();
        int required_args = 1;

        // 第1引数。
        if (!list_itr) {
          throw LispObject::GenInsufficientArgumentsError
          (func_name, required_args, true, list.Length() - 1);
        }
        LispObjectPtr min_ptr = caller.Evaluate(*(list_itr++));
        if (!(min_ptr->IsNumber())) {
          throw LispObject::GenWrongTypeError
          (func_name, "Number", std::vector<int> {1}, true);
        }

        int index = 2;
        for (; list_itr; ++list_itr, ++index) {
          LispObjectPtr current = caller.Evaluate(*list_itr);
          if (!(current->IsNumber())) {
            throw LispObject::GenWrongTypeError
            (func_name, "Number", std::vector<int> {index}, true);
          }

          if (current->number_value() < min_ptr->number_value()) {
            min_ptr = current;
          }
        }

        return min_ptr;
      };
      root_ptr->BindSymbol("min", func_ptr);
      (*dict_ptr)["min"] =
R"...(### min ###

__Usage__

* `(min <Number>...)`

__Description__

* Returns minimum number of `<Number>...`.

__Example__

    (display (min 4 3 2 1 2 3 4))
    
    ;; Output
    ;;
    ;; > 1)...";
    }
  }

  // LispObjectを標準出力に表示する。
  void LispObject::Print(const LispObject* obj_ptr) {
    if (!obj_ptr) {
      std::cout << "Null Pointer" << std::endl;
      return;
    }

    std::string type_str[] {
      "PAIR", "NIL", "SYMBOL", "NUMBER", "BOOLEAN", "STRING",
      "FUNCTION", "NATIVE_FUNCTION"
    };

    std::cout << type_str[static_cast<int>(obj_ptr->type())];

    switch (obj_ptr->type()) {
      case LispObjectType::SYMBOL:
        std::cout << " : " << obj_ptr->symbol_value();
        break;
      case LispObjectType::NUMBER:
        std::cout << " : " << obj_ptr->number_value();
        break;
      case LispObjectType::BOOLEAN:
        if (obj_ptr->boolean_value()) {
          std::cout << " : #t";
        } else {
          std::cout << " : #f";
        }
        break;
      case LispObjectType::STRING:
        std::cout << " : " << obj_ptr->string_value();
        break;
      default:
        break;
    }
    std::cout << std::endl;
  }

  // ツリーを表示。
  void LispObject::PrintTree(const LispObject* obj_ptr,
  const std::string& pre_str) {
    std::cout << pre_str << std::flush;

    if (!obj_ptr) {
      std::cout << "Null Pointer" << std::endl;
      return;
    }

    std::string type_str[] {
      "PAIR", "NIL", "SYMBOL", "NUMBER", "BOOLEAN", "STRING",
      "FUNCTION", "NATIVE_FUNCTION"
    };

    std::cout << type_str[static_cast<int>(obj_ptr->type())];

    switch (obj_ptr->type()) {
      case LispObjectType::SYMBOL:
        std::cout << " : " << obj_ptr->symbol_value();
        break;
      case LispObjectType::NUMBER:
        std::cout << " : " << obj_ptr->number_value();
        break;
      case LispObjectType::BOOLEAN:
        if (obj_ptr->boolean_value()) {
          std::cout << " : #t";
        } else {
          std::cout << " : #f";
        }
        break;
      case LispObjectType::STRING:
        std::cout << " : " << obj_ptr->string_value();
        break;
      default:
        break;
    }
    std::cout << std::endl;

    PrintTree(obj_ptr->car_.get(), pre_str + "  Car | ");
    PrintTree(obj_ptr->cdr_.get(), pre_str + "  --- | ");
  }

  // ============= //
  // LispIterator関連 //
  // ============= //
  // コンストラクタ。
  LispIterator::LispIterator(const LispObject* list_ptr)
  : current_ptr_(list_ptr) {}

  // コンストラクタ。
  LispIterator::LispIterator() : current_ptr_(nullptr) {}

  // コピーコンストラクタ。
  LispIterator::LispIterator(const LispIterator& itr) :
  current_ptr_(itr.current_ptr_) {}

  // ムーブコンストラクタ。
  LispIterator::LispIterator(LispIterator&& itr) :
  current_ptr_(itr.current_ptr_) {}

  // コピー代入演算子。
  LispIterator& LispIterator::operator=(const LispIterator& itr) {
    current_ptr_ = itr.current_ptr_;
    return *this;
  }

  // ムーブ代入演算子。
  LispIterator& LispIterator::operator=(LispIterator&& itr) {
    current_ptr_ = itr.current_ptr_;
    return *this;
  }

  // デストラクタ。
  LispIterator::~LispIterator() {}
}  // namespace Sayuri
