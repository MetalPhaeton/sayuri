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
 * @file lisp_core.h
 * @author Hironori Ishibashi
 * @brief Lispインタープリタ。 
 */

#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <functional>
#include <set>
#include <queue>
#include <map>
#include <sstream>

#ifndef LISP_CORE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define LISP_CORE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

/** Sayuri 名前空間。 */
namespace Sayuri {
  struct LispPair;
  struct LispFunction;
  struct Atom;
  class LispObject;
  using LispObjectPtr = std::shared_ptr<LispObject>;
  using SymbolMap = std::map<std::string, LispObjectPtr>;
  using SymbolMapPtr = std::shared_ptr<SymbolMap>;
  using ScopeChain = std::vector<SymbolMapPtr>;
  /** 
   * - 第1引数 : 自分自身のLispObjectPtr。
   * - 第2引数 : 呼び出し元のLispObject。
   * - 第3引数 : 呼び出されたリスト。
   */
  using NativeFunction = std::function
  <LispObjectPtr(LispObjectPtr, const LispObject&, const LispObject&)>;
  using HelpDict = std::map<std::string, std::string>;

  /** Lispオブジェクトのタイプ。 */
  enum class LispObjectType {
    /** LispPair。 */
    PAIR,
    /** Atom。 */
    ATOM
  };

  /** Atomのタイプ。 */
  enum class AtomType {
    /** Nil。 */
    NIL,
    /** シンボル。 */
    SYMBOL,
    /** 数字。 */
    NUMBER,
    /** 真偽値。 */
    BOOLEAN,
    /** 文字列。 */
    STRING,
    /** 関数オブジェクト。 */
    FUNCTION,
    /** ネイティブ関数オブジェクト。 */
    NATIVE_FUNCTION
  };

  /** LispPairの構造体。 */
  struct LispPair {
    /** Carへのポインタ。 */
    LispObjectPtr car_;
    /** Carへのポインタ。 */
    LispObjectPtr cdr_;

    // ==================== //
    // コンストラクタと代入 //
    // ==================== //
    /** コンストラクタ。 */
    LispPair();
    /**
     * コピーコンストラクタ。
     * @param cons コピー元。
     */
    LispPair(const LispPair& cons);
    /**
     * ムーブコンストラクタ。
     * @param cons ムーブ元。
     */
    LispPair(LispPair&& cons);
    /**
     * コピー代入演算子。
     * @param cons コピー元。
     */
    LispPair& operator=(const LispPair& cons);
    /**
     * ムーブ代入演算子。
     * @param cons ムーブ元。
     */
    LispPair& operator=(LispPair&& cons);
    /** デストラクタ。 */
    virtual ~LispPair();
  };

  /** 関数オブジェクト構造体。 */
  struct LispFunction {
    /** 引数の名前空間。 */
    std::vector<std::string> arg_name_vec_;
    /** 関数定義。 */
    std::vector<LispObjectPtr> def_vec_;

    // ==================== //
    // コンストラクタと代入 //
    // ==================== //
    /** コンストラクタ。 */
    LispFunction();
    /**
     * コピーコンストラクタ。
     * @param func コピー元。
     */
    LispFunction(const LispFunction& func);
    /**
     * ムーブコンストラクタ。
     * @param func ムーブ元。
     */
    LispFunction(LispFunction&& func);
    /**
     * コピー代入演算子。
     * @param func コピー元。
     */
    LispFunction& operator=(const LispFunction& func);
    /**
     * ムーブ代入演算子。
     * @param func ムーブ元。
     */
    LispFunction& operator=(LispFunction&& func);
    /** デストラクタ。 */
    virtual ~LispFunction();
  };

  /** Atomデータ構造体。 */
  struct Atom {
    /** Atomのタイプ。 */
    AtomType type_;
    /** プリミティブデータ。 */
    union {
      /** 実数データ。 */
      double number_value_;
      /** 真偽値データ。 */
      bool boolean_value_;
    };
    /** シンボル・文字列データ。 */
    std::string str_value_;
    /** 関数オブジェクト。 **/
    LispFunction function_;
    /** ネイティブ関数オブジェクト。 **/
    NativeFunction native_function_;
    /** スコープチェーン。 */
    ScopeChain scope_chain_;

    // ==================== //
    // コンストラクタと代入 //
    // ==================== //
    /** コンストラクタ。 */
    Atom();
    /**
     * コピーコンストラクタ。
     * @param atom コピー元。
     */
    Atom(const Atom& atom);
    /**
     * ムーブコンストラクタ。
     * @param atom ムーブ元。
     */
    Atom(Atom&& atom);
    /**
     * コピー代入演算子。
     * @param atom コピー元。
     */
    Atom& operator=(const Atom& atom);
    /**
     * ムーブ代入演算子。
     * @param atom ムーブ元。
     */
    Atom& operator=(Atom&& atom);
    /** デストラクタ。 */
    virtual ~Atom();
  };

  /** Lispオブジェクト構造体。 */
  class LispObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      LispObject();
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LispObject(const LispObject& obj);
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LispObject(LispObject&& obj);
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      LispObject& operator=(const LispObject& obj);
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      LispObject& operator=(LispObject&& obj);
      /** デストラクタ。 */
      virtual ~LispObject();

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * ヒープに自分のコピーを作る。
       * @return 自分のコピーのポインタ。
       */
      LispObjectPtr Clone() const {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = type_;

        ret_ptr->atom_ = atom_;

        if (pair_.car_) {
          ret_ptr->pair_.car_ = pair_.car_->Clone();
        }
        if (pair_.cdr_) {
          ret_ptr->pair_.cdr_ = pair_.cdr_->Clone();
        }

        return ret_ptr;
      }

      /**
       * 自分のスコープのシンボルにLispObjectPtrをバインドする。
       * @param symbol シンボル。
       * @param obj_ptr バインドするLispObjectPtr。
       */
      void BindSymbol(const std::string& symbol, LispObjectPtr obj_ptr) const {
        // scope_chain_.back() : SymbolMapPtr
        // *(scope_chain_.back()) : SymbolMap
        // (*(scope_chain_.back()))[symbol] : LispObjectPtr
        // *((*(scope_chain_.back()))[symbol]) : LispObject
        (*(atom_.scope_chain_.back()))[symbol] = obj_ptr;
      }

      /**
       * シンボルを上書きする。
       * @param symbol シンボル。
       * @param obj_ptr 上書きするLispObjectPtr。
       */
      void RewriteSymbol(const std::string& symbol, LispObjectPtr obj_ptr)
      const {
        // スコープチェーンを検索する。
        ScopeChain::const_reverse_iterator citr =
        atom_.scope_chain_.crbegin();

        // *citr : SymbolMapPtr
        // **citr : SymbolMap
        // (**citr)[symbol] : LispObjectPtr
        // *((**citr)[symbol]) : LispObject
        for (; citr != atom_.scope_chain_.rend(); ++citr) {
          if ((**citr).find(symbol) != (**citr).end()) {
            (**citr)[symbol] = obj_ptr;
          }
        }

        (*(atom_.scope_chain_.back()))[symbol] = obj_ptr;
      }

      /**
       * シンボルを参照する。
       * @param symbol シンボル。
       * @return シンボルにバインドされているLispObjectPtr。
       * @throw LispObjectPtr エラーメッセージ。 (シンボルが見つからなかった。)
       */
      LispObjectPtr ReferSymbol(const std::string& symbol) const
      throw (LispObjectPtr) {
        // スコープチェーンを検索する。
        ScopeChain::const_reverse_iterator citr =
        atom_.scope_chain_.crbegin();

        // *citr : SymbolMapPtr
        // **citr : SymbolMap
        // (**citr)[symbol] : LispObjectPtr
        // *((**citr)[symbol]) : LispObject
        for (; citr != atom_.scope_chain_.rend(); ++citr) {
          if ((**citr).find(symbol) != (**citr).end()) {
            return (**citr)[symbol];
          }
        }

        throw GenError
        ("@unbound", "'" + symbol + "' is not bound with any object.");
      }

      /**
       * オブジェクトの長さを得る。 CdrでつながったPairの長さ。
       * @return オブジェクトの長さ。
       */
      int Length() const {
        int count = 0;

        // ペアの数を数える。
        for (const LispObject* ptr = this;
        ptr && (ptr->type_ ==  LispObjectType::PAIR);
        ptr = ptr->pair_.cdr_.get()) {
          ++count;
        }

        return count;
      }

      /**
       * 自分の文字列表現。
       * @return 自分の文字列表現。
       */
      std::string ToString() const;

      /**
       * 自分がLispPairかどうか判定する。
       * @return LispPairならtrue。
       */
      bool IsPair() const {
        return type_ == LispObjectType::PAIR;
      }

      /**
       * 自分がNilかどうか判定する。
       * @return Nilならtrue。
       */
      bool IsNil() const {
        return (type_ == LispObjectType::ATOM)
        && (atom_.type_ == AtomType::NIL);
      }

      /**
       * 自分がSymbolかどうか判定する。
       * @return Symbolならtrue。
       */
      bool IsSymbol() const {
        return (type_ == LispObjectType::ATOM)
        && (atom_.type_ == AtomType::SYMBOL);
      }

      /**
       * 自分がNumberかどうか判定する。
       * @return Numberならtrue。
       */
      bool IsNumber() const {
        return (type_ == LispObjectType::ATOM)
        && (atom_.type_ == AtomType::NUMBER);
      }

      /**
       * 自分がBooleanかどうか判定する。
       * @return Booleanならtrue。
       */
      bool IsBoolean() const {
        return (type_ == LispObjectType::ATOM)
        && (atom_.type_ == AtomType::BOOLEAN);
      }

      /**
       * 自分がStringかどうか判定する。
       * @return Stringならtrue。
       */
      bool IsString() const {
        return (type_ == LispObjectType::ATOM)
        && (atom_.type_ == AtomType::STRING);
      }

      /**
       * 自分がFunctionかどうか判定する。
       * @return Functionならtrue。
       */
      bool IsFunction() const {
        return (type_ == LispObjectType::ATOM)
        && (atom_.type_ == AtomType::FUNCTION);
      }

      /**
       * 自分がNative Functionかどうか判定する。
       * @return Native Functionならtrue。
       */
      bool IsNativeFunction() const {
        return (type_ == LispObjectType::ATOM)
        && (atom_.type_ == AtomType::NATIVE_FUNCTION);
      }

      /**
       * 自分がListかどうか判定する。
       * @return Listならtrue。
       */
      bool IsList() const {
        const LispObject* ptr = this;
        for (; ptr && ptr->IsPair(); ptr = ptr->pair_.cdr_.get()) continue;

        if (ptr->IsNil()) return true;
        return false;
      }

      /**
       * 自分がListのとき、インデックスでアクセスする演算子。
       * @param index インデックス。 マイナスなら末尾からのインデックス。
       * @return 要素。
       */
      LispObject& operator[](int index) const {
        // 要素を検索。
        index = index < 0 ? Length() + index : index;
        const LispObject* ptr = this;
        for (int i = 0; ptr && ptr->IsPair();
        ptr = ptr->pair_.cdr_.get(),  ++i) {
          if (i == index) return *(ptr->pair_.car_);
        }

        throw LispObject::GenError
        ("@out-of-range", "The index is out-of-range.");
      }

      /**
       * 自分のシンボルマップで評価する。
       * 評価結果には、適切なのシンボルマップが継承される。
       * @param target 評価する相手。
       * @return 評価結果のLispObjectPtr。
       */
      LispObjectPtr Evaluate(const LispObject& target) const;

      /**
       * 新しいローカルスコープを生成する。
       */
      void NewLocalScope() {
        atom_.scope_chain_.push_back(SymbolMapPtr(new SymbolMap()));
      }

      /**
       * リストの末尾のCdrにオブジェクトを追加する。
       * @param obj 追加するオブジェクト。
       */
      void Append(LispObjectPtr obj) {
        LispObject* ptr = this;
        for (; ptr && ptr->IsPair(); ptr = ptr->pair_.cdr_.get()) continue;

        if (ptr->IsNil()) *ptr = *obj;
      }

      /**
       * 基本関数を登録する。
       * @param obj セットしたいルートのLispObjectPtr。
       * @param dict_ptr ヘルプ用辞書の共有ポインタ。
       */
      static void SetBasicFunctions(LispObjectPtr root_ptr,
      std::shared_ptr<HelpDict> dict_ptr);

      /**
       * グローバルオブジェクトを生成する。
       * @param obj セットしたいルートのLispObjectPtr。
       * @return グローバルオブジェクト。
       */
      static LispObjectPtr GenGlobal(std::shared_ptr<HelpDict> dict_ptr) {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::ATOM;
        ret_ptr->atom_.type_ = AtomType::FUNCTION;
        ret_ptr->atom_.scope_chain_.push_back(SymbolMapPtr(new SymbolMap()));
        SetCoreFunctions(ret_ptr, dict_ptr);
        return ret_ptr;
      }

      /**
       * LispPairオブジェクトを作る。 (引数あり)
       * @param car Car。
       * @param cdr Cdr。
       * @return LispPairオブジェクト。
       */
      static LispObjectPtr NewPair(LispObjectPtr car, LispObjectPtr cdr) {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::PAIR;
        ret_ptr->pair_.car_ = car;
        ret_ptr->pair_.cdr_ = cdr;

        return ret_ptr;
      }

      /**
       * LispPairオブジェクトを作る。 (引数無し)
       * @return LispPairオブジェクト。
       */
      static LispObjectPtr NewPair() {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::PAIR;
        ret_ptr->pair_.car_.reset(new LispObject());
        ret_ptr->pair_.car_->type_ = LispObjectType::ATOM;
        ret_ptr->pair_.car_->atom_.type_ = AtomType::NIL;
        ret_ptr->pair_.cdr_.reset(new LispObject());
        ret_ptr->pair_.cdr_->type_ = LispObjectType::ATOM;
        ret_ptr->pair_.cdr_->atom_.type_ = AtomType::NIL;

        return ret_ptr;
      }

      /**
       * Nilオブジェクトを作る。
       * @return Nilオブジェクト。
       */
      static LispObjectPtr NewNil() {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::ATOM;
        ret_ptr->atom_.type_ = AtomType::NIL;
        return ret_ptr;
      }

      /**
       * Symbolオブジェクトを作る。
       * @param value 初期値。
       * @return Symboオブジェクト。
       */
      static LispObjectPtr NewSymbol(const std::string& value) {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::ATOM;
        ret_ptr->atom_.type_ = AtomType::SYMBOL;
        ret_ptr->atom_.str_value_ = value;
        return ret_ptr;
      }

      /**
       * Numberオブジェクトを作る。
       * @param value 初期値。
       * @return Numberオブジェクト。
       */
      static LispObjectPtr NewNumber(double value) {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::ATOM;
        ret_ptr->atom_.type_ = AtomType::NUMBER;
        ret_ptr->atom_.number_value_ = value;
        return ret_ptr;
      }

      /**
       * Booleanオブジェクトを作る。
       * @param value 初期値。
       * @return Booleanオブジェクト。
       */
      static LispObjectPtr NewBoolean(bool value) {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::ATOM;
        ret_ptr->atom_.type_ = AtomType::BOOLEAN;
        ret_ptr->atom_.boolean_value_ = value;
        return ret_ptr;
      }

      /**
       * Stringオブジェクトを作る。
       * @param value 初期値。
       * @return Stringオブジェクト。
       */
      static LispObjectPtr NewString(const std::string& value) {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::ATOM;
        ret_ptr->atom_.type_ = AtomType::STRING;
        ret_ptr->atom_.str_value_ = value;
        return ret_ptr;
      }

      /**
       * Functionオブジェクトを作る。
       * @return Functionオブジェクト。
       */
      static LispObjectPtr NewFunction() {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::ATOM;
        ret_ptr->atom_.type_ = AtomType::FUNCTION;
        return ret_ptr;
      }

      /**
       * Native Functionオブジェクトを作る。
       * @return Native Functionオブジェクト。
       */
      static LispObjectPtr NewNativeFunction() {
        LispObjectPtr ret_ptr(new LispObject());
        ret_ptr->type_ = LispObjectType::ATOM;
        ret_ptr->atom_.type_ = AtomType::NATIVE_FUNCTION;
        return ret_ptr;
      }

      /**
       * 特定の長さのリストを作る。
       * @param length リストの長さ。
       * @return リスト。
       */
      static LispObjectPtr NewList(unsigned int length) {
        LispObjectPtr ret_ptr = NewNil();
        LispObject* ptr = ret_ptr.get();
        for (unsigned int i = 0; i < length; ++i) {
          ptr->type_ = LispObjectType::PAIR;
          ptr->pair_.car_ = NewNil();
          ptr->pair_.cdr_ = NewNil();

          ptr = ptr->pair_.cdr_.get();
        }

        return ret_ptr;
      }

      /**
       * エラー用リストを作成する。
       * @param error_symbol エラー識別シンボル。
       * @param message エラーメッセージ。
       * @return エラー用リスト。
       */
      static LispObjectPtr GenError(const std::string& error_symbol,
      const std::string& message) {
        LispObjectPtr ret_ptr = NewList(2);

        ret_ptr->pair_.car_ = NewSymbol(error_symbol);
        ret_ptr->pair_.cdr_->pair_.car_ = NewString(message);

        return ret_ptr;
      }
   
      /**
       * 「不十分な引数エラー」を作成する。
       * @param func_name 関数名。
       * @param require 本来必要な引数の数。
       * @param is_and_more 本来必要な引数の数のは必要最小限の数かどうか。
       * @param given 実際与えられた引数の数。
       * @return エラー用リスト。
       */
      static LispObjectPtr GenInsufficientArgumentsError
      (const std::string& func_name, int require, bool is_and_more,
      int given) {
        // 必要な引数の数。
        std::string message = "(" + func_name + ") needs "
        + std::to_string(require);
        if (require <= 1) message += " argument";
        else message += " arguments";

        // 以上かどうか。
        if (is_and_more) message += " and more. ";
        else message += ". ";

        // 実際与えられた引数の数。
        message += "Given " + std::to_string(given);
        if (given <= 1) message += " argument.";
        else message += " arguments.";

        return GenError("@insufficient-arguments", message);
      }

      /**
       * タイプ違いのエラーリストを作成する。
       * @param func_name 関数名。
       * @param required_type_str 求められている型の文字列。 以下の内どれか。
       *   - Pair
       *   - Nil
       *   - Symbol
       *   - Number
       *   - Boolean
       *   - String
       *   - List
       * @param index_stack 要素のインデックスが積まれたスタック。
       *                    深い入れ子要素ほど高い位置に積まれる。
       * @param has_evaluated 評価済みの要素かどうか。
       */
      static LispObjectPtr GenWrongTypeError(const std::string& func_name,
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
        std::string message = "";
        bool first = true;
        for (; index_vec.size() > 0; index_vec.pop_back()) {
          // 冠詞。
          if (first) {
            message += "The ";
            first = false;
          } else {
            message += "the ";
          }

          // 何番目か。
          message += std::to_string(index_vec.back());
          int column_1 = index_vec.back() % 10;
          if (column_1 == 1) message += "st ";
          else if (column_1 == 2) message += "nd ";
          else if (column_1 == 3) message += "rd ";
          else message += "th ";

          // 要素。
          if (index_vec.size() == 1) {
            message += "argument of ";
          } else {
            message += "element of ";
          }
        }

        // どの関数かを作成。
        message += "(" + func_name + ") ";
        if (has_evaluated) message += "didn't return ";
        else message += "is not ";

        // 要求されたタイプを作成。
        message += required_type_str + ".";

        return GenError(error_symbol, message);
      }

      /**
       * S式をパースする。
       * @param s_expr_str S式の文字列。
       * @return パース結果のポインタ。
       * @throw LispObjectPtr エラーメッセージ。
       */
      static LispObjectPtr Parse(const std::string& s_expr_str);

      /**
       * S式をトークンに分ける。
       * @param s_expr_str S式の文字列。
       * @return 切り分けられた文字列。
       */
      static std::vector<std::string> Split(const std::string& s_expr_str) {
        static const std::set<char> delim {
          ' ', '\n', '\t', '(', ')', '"', '\'', ';'
        };
        std::vector<std::string> ret(0);

        std::string temp;
        for (auto c : s_expr_str) {
          if (delim.find(c) != delim.end()) {
            if (temp.size() > 0) {
              ret.push_back(temp);
              temp.clear();
            }
            char cs[2] {c, 0};
            ret.push_back(cs);
          } else {
            temp.push_back(c);
          }
        }

        if (temp.size() > 0) {
          ret.push_back(temp);
          temp.clear();
        }

        return ret;
      }

      /**
       * LispObjectを標準出力に表示する。
       * @param obj_ptr 表示するLispObjectのポインタ。
       */
      static void Print(const LispObject* obj_ptr);

      /**
       * LispObjectのツリー構造を標準出力に表示する。
       * @param obj_ptr 表示するLispObjectのポインタ。
       * @param pre_str インデント。 再帰コールで使う。
       */
      static void PrintTree(const LispObject* obj_ptr,
      const std::string& pre_str);

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - オブジェクトのタイプ。
       * @return オブジェクトのタイプ。
       */
      LispObjectType type() const {return type_;}
      /**
       * アクセサ - Car。
       * @return Car。
       */
      LispObjectPtr car() const {return pair_.car_;}
      /**
       * アクセサ - Cdr。
       * @return Cdr。
       */
      LispObjectPtr cdr() const {return pair_.cdr_;}
      /**
       * アクセサ - Atomのタイプ。
       * @return Atomのタイプ。
       */
      AtomType atom_type() const {return atom_.type_;}
      /**
       * アクセサ - Atomのシンボルの値。
       * @return Atomのシンボルの値。
       */
      std::string symbol_value() const {return atom_.str_value_;}
      /**
       * アクセサ - Atomの数字の値。
       * @return Atomの数字の値。
       */
      double number_value() const {return atom_.number_value_;}
      /**
       * アクセサ - Atomの真偽値の値。
       * @return Atomの真偽値の値。
       */
      bool boolean_value() const {return atom_.boolean_value_; }
      /**
       * アクセサ - Atomの文字列の値。
       * @return Atomの文字列の値。
       */
      std::string string_value() const {return atom_.str_value_;}
      /**
       * アクセサ - Atomの関数オブジェクト。
       * @return Atomの関数オブジェクト。
       */
      const LispFunction& function_() const {return atom_.function_;} 
      /**
       * アクセサ - Atomの関数オブジェクトの引数名ベクトル。
       * @return Atomの関数オブジェクトの引数名ベクトル。
       */
      const std::vector<std::string>& arg_name_vec() const {
        return atom_.function_.arg_name_vec_;
      }
      /**
       * アクセサ - Atomの関数オブジェクトの関数定義ベクトル。
       * @return Atomの関数オブジェクトの関数定義ベクトル。
       */
      const std::vector<LispObjectPtr> def_vec() const {
        return atom_.function_.def_vec_;
      }
      /**
       * アクセサ - Atomのネイティブ関数オブジェクト。
       * @return Atomのネイティブ関数オブジェクト。
       */
      const NativeFunction& native_function() const {
        return atom_.native_function_;
      }
      /**
       * アクセサ - スコープチェーン。
       * @return スコープチェーン。
       */
      const ScopeChain& scope_chain() const {
        return atom_.scope_chain_;
      }

      // ============ //
      // ミューテータ //
      // ============ //
      /**
       * ミューテータ - オブジェクトのタイプ。
       * @param type オブジェクトのタイプ。
       */
      void type(LispObjectType type) {type_ = type;}
      /**
       * ミューテータ - Car。
       * @param ptr Car。
       */
      void car(LispObjectPtr ptr) {pair_.car_ = ptr;}
      /**
       * ミューテータ - Cdr。
       * @param ptr Cdr。
       */
      void cdr(LispObjectPtr ptr) {pair_.cdr_ = ptr;}
      /**
       * ミューテータ - Atomのタイプ。
       * @param type Atomのタイプ。
       */
      void atom_type(AtomType type) {atom_.type_ = type;}
      /**
       * ミューテータ - Atomのシンボルの値。
       * @param value Atomのシンボルの値。
       */
      void symbol_value(const std::string& value) {
        atom_.str_value_ = value;
      }
      /**
       * ミューテータ - Atomの数字の値。
       * @param value Atomの数字の値。
       */
      void number_value(double value) {atom_.number_value_ = value;}
      /**
       * ミューテータ - Atomの真偽値の値。
       * @param value Atomの真偽値の値。
       */
      void boolean_value(bool value) {atom_.boolean_value_ = value; }
      /**
       * ミューテータ - Atomの文字列の値。
       * @param value Atomの文字列の値。
       */
      void string_value(const std::string& value) {
        atom_.str_value_ = value;
      }
      /**
       * ミューテータ - Atomの関数オブジェクト。
       * @param obj Atomの関数オブジェクト。
       */
      void function(const LispFunction& obj) {atom_.function_ = obj;} 
      /**
       * ミューテータ - Atomの関数オブジェクトの引数名ベクトル。
       * @param vec Atomの関数オブジェクトの引数名ベクトル。
       */
      void arg_name_vec(const std::vector<std::string>& vec) {
        atom_.function_.arg_name_vec_ = vec;
      }
      /**
       * ミューテータ - Atomの関数オブジェクトの関数定義ベクトル。
       * @param vec Atomの関数オブジェクトの関数定義ベクトル。
       */
      void def_vec(const std::vector<LispObjectPtr>& vec) {
        atom_.function_.def_vec_ = vec;
      }
      /**
       * ミューテータ - Atomのネイティブ関数オブジェクト。
       * @param obj Atomのネイティブ関数オブジェクト。
       */
      void native_function(const NativeFunction& obj) {
        atom_.native_function_ = obj;
      }
      /**
       * ミューテータ - スコープチェーン。
       * @param chain スコープチェーン。
       */
      void scope_chain(const ScopeChain& chain) {
        atom_.scope_chain_ = chain;
      }

    private:
      /**
       * Parse()の本体。
       * @param my_obj_ptr パース結果を格納するオブジェクト。
       * @param str_queue パースする残りの文字列。
       */
      static void ParseCore(LispObjectPtr my_obj_ptr,
      std::queue<std::string>& str_queue);

      /**
       * 絶対必要なコア関数を登録する。
       * @param obj セットしたいルートのLispObjectPtr。
       * @param dict_ptr ヘルプ用辞書の共有ポインタ。
       */
      static void SetCoreFunctions(LispObjectPtr root_ptr,
      std::shared_ptr<HelpDict> dict_ptr);

      // ========== //
      // メンバ変数 //
      // ========== //
      /** オブジェクトのタイプ。 */
      LispObjectType type_;

      /** LispPairデータ。*/
      LispPair pair_;
      /** Atomデータ。 */
      Atom atom_;

  };

  /** List用イテレータ。 */
  class LispIterator {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param list_ptr リストのポインタ。
       */
      LispIterator(const LispObject* list_ptr);
      /** コンストラクタ。 */
      LispIterator();
      /**
       * コピーコンストラクタ。
       * @param itr コピー元。
       */
      LispIterator(const LispIterator& itr);
      /**
       * ムーブコンストラクタ。
       * @param itr ムーブ元。
       */
      LispIterator(LispIterator&& itr);
      /**
       * コピー代入演算子。
       * @param itr コピー元。
       */
      LispIterator& operator=(const LispIterator& itr);
      /**
       * ムーブ代入演算子。
       * @param itr ムーブ元。
       */
      LispIterator& operator=(LispIterator&& itr);
      /** デストラクタ。 */
      virtual ~LispIterator();

      // ====== //
      // 演算子 //
      // ====== //
      /**
       * 前置インクリメント演算子。
       * @return 自分自身。
       */
      LispIterator& operator++() {
        if (!current_ptr_) return *this;
        if (current_ptr_->IsPair()) {
          current_ptr_ = current_ptr_->cdr().get();
        }

        return *this;
      }

      /**
       * 後置インクリメント演算子。
       * @return インクリメント前のコピー。
       */
      LispIterator operator++(int dummy) {
        // インクリメント前のコピーを取っておく。
        LispIterator ret(*this);

        if (!current_ptr_) return ret;
        if (current_ptr_->IsPair()) {
          current_ptr_ = current_ptr_->cdr().get();
        }

        // インクリメント前のコピーを返す。
        return ret;
      }

      /**
       * 末尾に到達していなければtrue。
       */
      explicit operator bool() {
        if (!current_ptr_) return false;
        return current_ptr_->IsPair();
      }

      /**
       * ポインタアクセス演算子。
       */
      LispObject& operator*() {
        return *(current_ptr_->car());
      }

      /**
       * LispObjectのポインタとしての演算子。
       */
      operator LispObject*() {
        return current_ptr_->car().get();
      }

      /**
       * アロー演算子。
       */
      LispObject* operator->() {
        return current_ptr_->car().get();
      }

      /**
       * 足し算挿入演算子。
       */
      LispIterator& operator+=(int inc) {
        for (int i = 0; i < inc; ++i) {
          if (current_ptr_->IsPair()) {
            current_ptr_ = current_ptr_->cdr().get();
          } else {
            break;
          }
        }
        return *this;
      }

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - 現在参照している。
       * @return 現在参照しているCdr。
       */
      const LispObject& current() const {return *current_ptr_;}

    private:
      /** 現在参照しているポインタ。 */
      const LispObject* current_ptr_;
  };

  /** 文法をチェックするクラス。 */
  class SyntaxChecker {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      SyntaxChecker() :
      input_str_(""),
      no_comment_str_(""),
      count_(0),
      in_comment_(false),
      in_string_(false) {}
      /**
       * コピーコンストラクタ。
       * @param checker コピー元。
       */
      SyntaxChecker(const SyntaxChecker& checker) :
      input_str_(checker.input_str_),
      no_comment_str_(checker.no_comment_str_),
      count_(checker.count_),
      in_comment_(checker.in_comment_),
      in_string_(checker.in_string_) {}
      /**
       * ムーブコンストラクタ。
       * @param checker ムーブ元。
       */
      SyntaxChecker(SyntaxChecker&& checker) :
      input_str_(std::move(checker.input_str_)),
      no_comment_str_(std::move(checker.no_comment_str_)),
      count_(checker.count_),
      in_comment_(checker.in_comment_),
      in_string_(checker.in_string_) {}
      /**
       * コピー代入演算子。
       * @param checker コピー元。
       */
      SyntaxChecker& operator=(const SyntaxChecker& checker) {
        input_str_ = checker.input_str_;
        no_comment_str_ = checker.no_comment_str_;
        count_ = checker.count_;
        in_comment_ = checker.in_comment_;
        in_string_ = checker.in_string_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param checker ムーブ元。
       */
      SyntaxChecker& operator=(SyntaxChecker&& checker) {
        input_str_ = std::move(checker.input_str_);
        no_comment_str_ = std::move(checker.no_comment_str_);
        count_ = checker.count_;
        in_comment_ = checker.in_comment_;
        in_string_ = checker.in_string_;
        return *this;
      }
      /** デストラクタ。 */
      virtual ~SyntaxChecker() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * チェックする文字を入力する。
       * @param input_str チェックしたい文字列。
       * @return
       * - return == 0 : 括弧の対応関係が正しい。
       * - return < 0 : 閉じ括弧が多すぎる。
       * - return > 0 : 閉じ括弧が足りない。
       */
      int Input(const std::string& input_str) {
        bool escape_c = false;
        for (auto c : input_str) {
          if (in_comment_) {
            // コメント中。
            if (c == '\n') {
              no_comment_str_.push_back(c);
              in_comment_ = false;
            }
          } else if (in_string_) {
            // 文字列中。
            no_comment_str_.push_back(c);
            if (escape_c) {
              // エスケープ文字。
              escape_c = false;
            } else {
              if (c == '"') {
                in_string_ = false;
              } else if (c == '\\') {
                // バックスラッシュ。 次の文字はエスケープ文字。
                escape_c = true;
              }
            }
          } else {
            // 通常状態。
            switch (c) {
              case '(':
                no_comment_str_.push_back(c);
                ++count_;
                break;
              case ')':
                no_comment_str_.push_back(c);
                --count_;
                break;
              case ';':
                in_comment_ = true;
                break;
              case '"':
                no_comment_str_.push_back(c);
                in_string_ = true;
                break;
              default:
                no_comment_str_.push_back(c);
                break;
            }
          }
        }

        input_str_ += input_str;
        return count_;
      }

      /**
       * 状態を初期状態にリセットする。
       */
      void Reset() {
        input_str_ = "";
        no_comment_str_ = "";
        count_ = 0;
        in_comment_ = false;
        in_string_ = false;
      }

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - 蓄積された文字列。
       * @return 蓄積された文字列。
       */
      std::string input_str() const {return input_str_;}

      /**
       * アクセサ - コメントを削除した、蓄積された文字列。
       * @return コメントを削除した、蓄積された文字列。
       */
      std::string no_comment_str() const {return no_comment_str_;}

    private:
      /** 入力した文字列。 */
      std::string input_str_;
      /** コメントを削除した文字列。 */
      std::string no_comment_str_;
      /** 括弧の対応関係の数。 */
      int count_;
      /** 現在コメント中かどうか。 */
      bool in_comment_;
      /** 現在文字列中かどうか。 */
      bool in_string_;
  };
}  // namespace Sayuri

#endif
