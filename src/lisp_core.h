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

#ifndef LISP_CORE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define LISP_CORE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

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
#include <iomanip>

/** Sayuri 名前空間。 */
namespace Sayuri {
  struct LispFunction;
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
    /** Pair。 */
    PAIR,
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

  /**
   * 実数から文字列に高精度で変換。
   * @param d 実数。
   * @return 変換後の文字列。
   */
  inline std::string DoubleToString(double d) {
    std::ostringstream oss;
    oss << std::setprecision(15) << d;
    return oss.str();
  }

  /** 関数オブジェクト構造体。 */
  struct LispFunction {
    /** 引数の名前。 */
    std::vector<std::string> arg_name_vec_;
    /** 関数定義。 */
    std::vector<LispObjectPtr> def_vec_;
  };

  /** Lispオブジェクト構造体。 */
  class LispObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      LispObject() :
      type_(LispObjectType::NIL),
      car_(nullptr), cdr_(nullptr),
      number_value_(0.0), boolean_value_(false), str_value_("") {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LispObject(const LispObject& obj) :
      type_(obj.type_),
      car_(obj.car_), cdr_(obj.cdr_),
      number_value_(obj.number_value_),
      boolean_value_(obj.boolean_value_),
      str_value_(obj.str_value_),
      function_(obj.function_),
      native_function_(obj.native_function_),
      scope_chain_(obj.scope_chain_) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LispObject(LispObject&& obj) :
      type_(obj.type_),
      car_(std::move(obj.car_)), cdr_(std::move(obj.cdr_)),
      number_value_(obj.number_value_),
      boolean_value_(obj.boolean_value_),
      str_value_(std::move(obj.str_value_)),
      function_(std::move(obj.function_)),
      native_function_(std::move(obj.native_function_)),
      scope_chain_(std::move(obj.scope_chain_)) {}
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      LispObject& operator=(const LispObject& obj) {
        type_ = obj.type_;
        car_ = obj.car_;
        cdr_ = obj.cdr_;
        number_value_ = obj.number_value_;
        boolean_value_ = obj.boolean_value_;
        str_value_ = obj.str_value_;
        function_ = obj.function_;
        native_function_ = obj.native_function_;
        scope_chain_ = obj.scope_chain_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      LispObject& operator=(LispObject&& obj) {
        type_ = obj.type_;
        car_ = std::move(obj.car_);
        cdr_ = std::move(obj.cdr_);
        number_value_ = obj.number_value_;
        boolean_value_ = obj.boolean_value_;
        str_value_ = std::move(obj.str_value_);
        function_ = std::move(obj.function_);
        native_function_ = std::move(obj.native_function_);
        scope_chain_ = std::move(obj.scope_chain_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LispObject() {}

      // ====== //
      // 演算子 //
      // ====== //
      /*
       * 比較演算子。
       * @param obj 比較対象。
       * @return 同じならtrue。
       */
      bool operator==(const LispObject& obj) const;
      /*
       * 比較演算子。
       * @param obj 比較対象。
       * @return 違うならtrue。
       */
      bool operator!=(const LispObject& obj) const {return !operator==(obj);}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * ヒープに自分のコピーを作る。
       * @return 自分のコピーのポインタ。
       */
      LispObjectPtr Clone() const {
        // 先ず自分のコピーを作る。
        LispObjectPtr ret_ptr(new LispObject(*this));

        // 子ノードをクローンする。
        if (car_) ret_ptr->car_ = car_->Clone();
        if (cdr_) ret_ptr->cdr_ = cdr_->Clone();

        return ret_ptr;
      }

      /**
       * 自分のスコープのシンボルにLispObjectPtrをバインドする。
       * @param symbol シンボル。
       * @param obj_ptr バインドするLispObjectPtr。
       */
      void BindSymbol(const std::string& symbol, LispObjectPtr obj_ptr)
      const {
        scope_chain_.back()->emplace(symbol, obj_ptr);
      }

      /**
       * シンボルを上書きする。
       * @param symbol シンボル。
       * @param obj_ptr 上書きするLispObjectPtr。
       */
      void RewriteSymbol(const std::string& symbol, LispObjectPtr obj_ptr)
      const {
        // スコープチェーンを検索する。
        ScopeChain::const_reverse_iterator citr = scope_chain_.crbegin();

        for (; citr != scope_chain_.rend(); ++citr) {
          if ((*citr)->find(symbol) != (*citr)->end()) {
            (*citr)->at(symbol) = obj_ptr;
            return;
          }
        }

        BindSymbol(symbol, obj_ptr);
      }

      /**
       * シンボルを参照する。
       * @param symbol シンボル。
       * @return シンボルにバインドされているLispObjectPtr。
       */
      LispObjectPtr ReferSymbol(const std::string& symbol) const {
        // スコープチェーンを検索する。
        ScopeChain::const_reverse_iterator citr = scope_chain_.crbegin();

        for (; citr != scope_chain_.rend(); ++citr) {
          if ((*citr)->find(symbol) != (*citr)->end()) {
            return (*citr)->at(symbol);
          }
        }

        LispObjectPtr error(new LispObject());
        error->type_ = LispObjectType::PAIR;
        error->car_.reset(new LispObject());
        error->cdr_.reset(new LispObject());
        error->cdr_->type_ = LispObjectType::PAIR;
        error->cdr_->car_.reset(new LispObject());
        error->cdr_->cdr_.reset(new LispObject());
        error->car_->type_ = LispObjectType::SYMBOL;
        error->car_->str_value_ = "@unbound";
        error->cdr_->car_->type_ = LispObjectType::STRING;
        error->cdr_->car_->str_value_ =
        "'" + symbol + "' is not bound with any object.";
        throw error;
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
        ptr = ptr->cdr_.get()) {
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
      bool IsPair() const {return type_ == LispObjectType::PAIR;}

      /**
       * 自分がNilかどうか判定する。
       * @return Nilならtrue。
       */
      bool IsNil() const {return type_ == LispObjectType::NIL;}

      /**
       * 自分がSymbolかどうか判定する。
       * @return Symbolならtrue。
       */
      bool IsSymbol() const {return type_ == LispObjectType::SYMBOL;}

      /**
       * 自分がNumberかどうか判定する。
       * @return Numberならtrue。
       */
      bool IsNumber() const {return type_ == LispObjectType::NUMBER;}

      /**
       * 自分がBooleanかどうか判定する。
       * @return Booleanならtrue。
       */
      bool IsBoolean() const {return type_ == LispObjectType::BOOLEAN;}

      /**
       * 自分がStringかどうか判定する。
       * @return Stringならtrue。
       */
      bool IsString() const {return type_ == LispObjectType::STRING;}

      /**
       * 自分がFunctionかどうか判定する。
       * @return Functionならtrue。
       */
      bool IsFunction() const {return type_ == LispObjectType::FUNCTION;}

      /**
       * 自分がNative Functionかどうか判定する。
       * @return Native Functionならtrue。
       */
      bool IsNativeFunction() const {
        return type_ == LispObjectType::NATIVE_FUNCTION;
      }

      /**
       * 自分がListかどうか判定する。
       * @return Listならtrue。
       */
      bool IsList() const {
        const LispObject* ptr = this;
        for (; ptr && ptr->IsPair(); ptr = ptr->cdr_.get()) continue;
        return ptr->IsNil();
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
        ptr = ptr->cdr_.get(),  ++i) {
          if (i == index) return *(ptr->car_);
        }

        LispObjectPtr error(new LispObject());
        error->type_ = LispObjectType::PAIR;
        error->car_.reset(new LispObject());
        error->cdr_.reset(new LispObject());
        error->cdr_->type_ = LispObjectType::PAIR;
        error->cdr_->car_.reset(new LispObject());
        error->cdr_->cdr_.reset(new LispObject());
        error->car_->type_ = LispObjectType::SYMBOL;
        error->car_->str_value_ = "@out-of-range";
        error->cdr_->car_->type_ = LispObjectType::STRING;
        error->cdr_->car_->str_value_ = "The index is out-of-range.";
        throw error;
      }

      /**
       * 自分のシンボルマップで評価する。
       * 評価結果には、適切なのシンボルマップが継承される。
       * @param target 評価する相手。
       * @return 評価結果のLispObjectPtr。
       */
      LispObjectPtr Evaluate(const LispObject& target) const;

      /**
       * リストの末尾のCdrにオブジェクトを追加する。
       * @param obj 追加するオブジェクト。
       */
      void Append(LispObjectPtr obj) {
        LispObject* ptr = this;
        for (; ptr && ptr->IsPair(); ptr = ptr->cdr_.get()) continue;

        if (ptr->IsNil()) *ptr = *obj;
      }

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
      LispObjectPtr car() const {return car_;}
      /**
       * アクセサ - Cdr。
       * @return Cdr。
       */
      LispObjectPtr cdr() const {return cdr_;}
      /**
       * アクセサ - Atomのシンボルの値。
       * @return Atomのシンボルの値。
       */
      std::string symbol_value() const {return str_value_;}
      /**
       * アクセサ - Atomの数字の値。
       * @return Atomの数字の値。
       */
      double number_value() const {return number_value_;}
      /**
       * アクセサ - Atomの真偽値の値。
       * @return Atomの真偽値の値。
       */
      bool boolean_value() const {return boolean_value_; }
      /**
       * アクセサ - Atomの文字列の値。
       * @return Atomの文字列の値。
       */
      std::string string_value() const {return str_value_;}
      /**
       * アクセサ - Atomの関数オブジェクト。
       * @return Atomの関数オブジェクト。
       */
      const LispFunction& function() const {return function_;} 
      /**
       * アクセサ - Atomの関数オブジェクトの引数名ベクトル。
       * @return Atomの関数オブジェクトの引数名ベクトル。
       */
      const std::vector<std::string>& arg_name_vec() const {
        return function_.arg_name_vec_;
      }
      /**
       * アクセサ - Atomの関数オブジェクトの関数定義ベクトル。
       * @return Atomの関数オブジェクトの関数定義ベクトル。
       */
      const std::vector<LispObjectPtr> def_vec() const {
        return function_.def_vec_;
      }
      /**
       * アクセサ - Atomのネイティブ関数オブジェクト。
       * @return Atomのネイティブ関数オブジェクト。
       */
      const NativeFunction& native_function() const {
        return native_function_;
      }
      /**
       * アクセサ - スコープチェーン。
       * @return スコープチェーン。
       */
      const ScopeChain& scope_chain() const {
        return scope_chain_;
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
      void car(LispObjectPtr ptr) {car_ = ptr;}
      /**
       * ミューテータ - Cdr。
       * @param ptr Cdr。
       */
      void cdr(LispObjectPtr ptr) {cdr_ = ptr;}
      /**
       * ミューテータ - Atomのシンボルの値。
       * @param value Atomのシンボルの値。
       */
      void symbol_value(const std::string& value) {str_value_ = value;}
      /**
       * ミューテータ - Atomの数字の値。
       * @param value Atomの数字の値。
       */
      void number_value(double value) {number_value_ = value;}
      /**
       * ミューテータ - Atomの真偽値の値。
       * @param value Atomの真偽値の値。
       */
      void boolean_value(bool value) {boolean_value_ = value; }
      /**
       * ミューテータ - Atomの文字列の値。
       * @param value Atomの文字列の値。
       */
      void string_value(const std::string& value) {str_value_ = value;}
      /**
       * ミューテータ - Atomの関数オブジェクト。
       * @param obj Atomの関数オブジェクト。
       */
      void function(const LispFunction& obj) {function_ = obj;} 
      /**
       * ミューテータ - Atomの関数オブジェクトの引数名ベクトル。
       * @param vec Atomの関数オブジェクトの引数名ベクトル。
       */
      void arg_name_vec(const std::vector<std::string>& vec) {
        function_.arg_name_vec_ = vec;
      }
      /**
       * ミューテータ - Atomの関数オブジェクトの関数定義ベクトル。
       * @param vec Atomの関数オブジェクトの関数定義ベクトル。
       */
      void def_vec(const std::vector<LispObjectPtr>& vec) {
        function_.def_vec_ = vec;
      }
      /**
       * ミューテータ - Atomのネイティブ関数オブジェクト。
       * @param obj Atomのネイティブ関数オブジェクト。
       */
      void native_function(const NativeFunction& obj) {
        native_function_ = obj;
      }
      /**
       * ミューテータ - スコープチェーン。
       * @param chain スコープチェーン。
       */
      void scope_chain(const ScopeChain& chain) {
        scope_chain_ = chain;
      }

    private:
      // フレンド。
      friend class Lisp;

      /**
       * プライベートコンストラクタ
       * @param type オブジェクトのタイプ。
       * @param number_value 実数。
       * @param boolean_value 真偽値。
       * @param str_value 文字列・シンボル。
       */
      LispObject(LispObjectType type, double number_value,
      bool boolean_value, const std::string& str_value) :
      type_(type), car_(nullptr), cdr_(nullptr),
      number_value_(number_value), boolean_value_(boolean_value),
      str_value_(str_value) {}

      // ========== //
      // メンバ変数 //
      // ========== //
      /** オブジェクトのタイプ。 */
      LispObjectType type_;

      /** Car。 */
      LispObjectPtr car_;
      /** Cdr。 */
      LispObjectPtr cdr_;

      /** 実数データ。 */
      double number_value_;
      /** 真偽値データ。 */
      bool boolean_value_;
      /** シンボル・文字列データ。 */
      std::string str_value_;

      /** 関数オブジェクト。 **/
      LispFunction function_;

      /** ネイティブ関数オブジェクト。 **/
      NativeFunction native_function_;

      /** スコープチェーン。 */
      ScopeChain scope_chain_;
  };

  /** List用イテレータ。 */
  struct LispIterator {
    /** 現在の位置。 */
    const LispObject* current_;

    // ====== //
    // 演算子 //
    // ====== //
    /**
     * 前置インクリメント演算子。
     * @return 自分自身。
     */
    LispIterator& operator++() {
      if (current_ && current_->IsPair()) current_ = current_->cdr().get();
      return *this;
    }

    /**
     * 後置インクリメント演算子。
     * @return インクリメント前のコピー。
     */
    LispIterator operator++(int dummy) {
      LispIterator ret(*this);
      if (current_ && current_->IsPair()) current_ = current_->cdr().get();
      return ret;
    }

    /**
     * 末尾に到達していなければtrue。
     */
    explicit operator bool() {
      return current_ ? current_->IsPair() : false;
    }

    /**
     * ポインタアクセス演算子。
     */
    LispObject& operator*() {
      return *(current_->car());
    }

    /**
     * LispObjectのポインタとしての演算子。
     */
    operator LispObject*() {
      return current_->car().get();
    }

    /**
     * アロー演算子。
     */
    LispObject* operator->() {
      return current_->car().get();
    }

    /**
     * 足し算挿入演算子。
     */
    LispIterator& operator+=(int inc) {
      for (int i = 0; i < inc; ++i) {
        if (current_ && current_->IsPair()) current_ = current_->cdr().get();
        else break;
      }
      return *this;
    }
  };

  /** Lispインタープリタのクラス。 */
  class Lisp {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      Lisp() : global_ptr_(new LispObject()),
      parentheses_(0), in_comment_(false), in_string_(false),
      in_escape_(false) {
        global_ptr_->type_ = LispObjectType::NATIVE_FUNCTION;
        global_ptr_->scope_chain_.push_back(SymbolMapPtr(new SymbolMap()));
        SetCoreFunctions();
        SetBasicFunctions();
      }
      /**
       * コピーコンストラクタ。
       * @param lisp コピー元。
       */
      Lisp(const Lisp& lisp) :
      global_ptr_(lisp.global_ptr_->Clone()),
      help_(lisp.help_),
      token_queue_(lisp.token_queue_),
      token_stream_(lisp.token_stream_.str()),
      parentheses_(lisp.parentheses_),
      in_comment_(lisp.in_comment_),
      in_string_(lisp.in_string_),
      in_escape_(lisp.in_escape_) {}
      /**
       * ムーブコンストラクタ。
       * @param lisp ムーブ元。
       */
      Lisp(Lisp&& lisp) :
      global_ptr_(std::move(lisp.global_ptr_)),
      help_(std::move(lisp.help_)),
      token_queue_(std::move(lisp.token_queue_)),
      token_stream_(lisp.token_stream_.str()),
      parentheses_(lisp.parentheses_),
      in_comment_(lisp.in_comment_),
      in_string_(lisp.in_string_),
      in_escape_(lisp.in_escape_) {}

      /**
       * コピー代入演算子。
       * @param lisp コピー元。
       */
      Lisp& operator=(const Lisp& lisp) {
        global_ptr_ = lisp.global_ptr_->Clone();
        help_ = lisp.help_;
        token_queue_ = lisp.token_queue_;
        token_stream_ << lisp.token_stream_.rdbuf();
        parentheses_ = lisp.parentheses_;
        in_comment_ = lisp.in_comment_;
        in_string_ = lisp.in_string_;
        in_escape_ = lisp.in_escape_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param lisp ムーブ元。
       */
      Lisp& operator=(Lisp&& lisp) {
        global_ptr_ = std::move(lisp.global_ptr_);
        help_ = std::move(lisp.help_);
        token_queue_ = std::move(lisp.token_queue_);
        token_stream_ << lisp.token_stream_.rdbuf();
        parentheses_ = lisp.parentheses_;
        in_comment_ = lisp.in_comment_;
        in_string_ = lisp.in_string_;
        in_escape_ = lisp.in_escape_;
        return *this;
      }
      /** デストラクタ。 */
      virtual ~Lisp() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * S式を実行する。
       * @param obj 実行するLispObject。
       * @return 実行結果のオブジェクト。
       */
      LispObjectPtr Execute(const LispObject& obj) {
        return global_ptr_->Evaluate(obj);
      }

      /**
       * グローバルにシンボルをバインドする。
       * @param func 登録する関数。
       * @param symbol 関数のシンボル。
       */
      void BindSymbol(const std::string& symbol,
      const LispObjectPtr& obj_ptr) {
        global_ptr_->BindSymbol(symbol, obj_ptr);
      }

      /**
       * 関数を登録する。
       * @param func 登録する関数。
       * @param symbol 関数のシンボル。
       */
      void AddNativeFunction(const NativeFunction& func,
      const std::string& symbol) {
        BindSymbol(symbol,
        NewNativeFunction(global_ptr_->scope_chain_, func));
      }
      /**
       * 関数を削除する。
       * @param symbol 削除する関数のシンボル。
       */
      void RemoveNativeFunction(const std::string& symbol) {
        if (global_ptr_->scope_chain_[0]->find(symbol)
        != global_ptr_->scope_chain_[0]->end()) {
          global_ptr_->scope_chain_[0]->erase(symbol);
        }
      }

      /**
       * ヘルプ辞書を登録する。
       * @param func 登録する関数。
       * @param symbol 関数のシンボル。
       */
      void AddHelpDict(const std::string& symbol,
      const std::string& contents) {
        help_.emplace(symbol, contents);
      }
      /**
       * ヘルプ辞書を削除する。
       * @param func 登録する関数。
       * @param symbol 関数のシンボル。
       */
      void RemoveHelpDict(const std::string& symbol) {
        if (help_.find(symbol) != help_.end()) {
          help_.erase(symbol);
        }
      }

      /**
       * 内部状態をリセットする。
       */
      void Reset() {
        token_queue_ = std::queue<std::string>();
        token_stream_.str("");
        parentheses_ = 0;
        in_comment_ = false;
        in_string_ = false;
        in_escape_ = false;
      }

      /**
       * Nilオブジェクトを作る。
       * @return Nilオブジェクト。
       */
      static LispObjectPtr NewNil() {return LispObjectPtr(new LispObject());}

      /**
       * LispPairオブジェクトを作る。 (引数あり)
       * @param car Car。
       * @param cdr Cdr。
       * @return LispPairオブジェクト。
       */
      static LispObjectPtr NewPair(LispObjectPtr car, LispObjectPtr cdr) {
        LispObjectPtr ret_ptr(new LispObject(LispObjectType::PAIR,
        0.0, false, ""));

        ret_ptr->car_ = car;
        ret_ptr->cdr_ = cdr;

        return ret_ptr;
      }

      /**
       * LispPairオブジェクトを作る。 (引数無し)
       * @return LispPairオブジェクト。
       */
      static LispObjectPtr NewPair() {
        LispObjectPtr ret_ptr(new LispObject(LispObjectType::PAIR,
        0.0, false, ""));

        ret_ptr->car_.reset(new LispObject());
        ret_ptr->cdr_.reset(new LispObject());

        return ret_ptr;
      }

      /**
       * Symbolオブジェクトを作る。
       * @param value 初期値。
       * @return Symboオブジェクト。
       */
      static LispObjectPtr NewSymbol(const std::string& value) {
        return LispObjectPtr(new LispObject(LispObjectType::SYMBOL,
        0.0, false, value));
      }

      /**
       * Numberオブジェクトを作る。
       * @param value 初期値。
       * @return Numberオブジェクト。
       */
      static LispObjectPtr NewNumber(double value) {
        return LispObjectPtr(new LispObject(LispObjectType::NUMBER,
        value, false, ""));
      }

      /**
       * Booleanオブジェクトを作る。
       * @param value 初期値。
       * @return Booleanオブジェクト。
       */
      static LispObjectPtr NewBoolean(bool value) {
        return LispObjectPtr(new LispObject(LispObjectType::BOOLEAN,
        0.0, value, ""));
      }

      /**
       * Stringオブジェクトを作る。
       * @param value 初期値。
       * @return Stringオブジェクト。
       */
      static LispObjectPtr NewString(const std::string& value) {
        return LispObjectPtr(new LispObject(LispObjectType::STRING,
        0.0, false, value));
      }

      /**
       * Functionオブジェクトを作る。
       * @param parent_scope 継承する親のスコープ。
       * @param arg_name_vec 引数名のベクトル。
       * @param def_vec 定義式のベクトル。
       * @return Functionオブジェクト。
       */
      static LispObjectPtr NewFunction(const ScopeChain& parent_scope,
      const std::vector<std::string>& arg_name_vec,
      const std::vector<LispObjectPtr>& def_vec) {
        LispObjectPtr ret_ptr(new LispObject(LispObjectType::FUNCTION,
        0.0, false, ""));

        // スコープを継承。
        ret_ptr->scope_chain_ = parent_scope;

        ret_ptr->function_.arg_name_vec_ = arg_name_vec;
        ret_ptr->function_.def_vec_ = def_vec;

        return ret_ptr;
      }

      /**
       * NativeFunctionオブジェクトを作る。
       * @param parent_scope 継承する親のスコープ。
       * @param native_function ネイティブ関数。
       * @return NativeFunctionオブジェクト。
       */
      static LispObjectPtr NewNativeFunction(const ScopeChain& parent_scope,
      const NativeFunction& native_function) {
        LispObjectPtr ret_ptr(new LispObject(LispObjectType::NATIVE_FUNCTION,
        0.0, false, ""));

        // スコープを継承。
        ret_ptr->scope_chain_ = parent_scope;

        ret_ptr->native_function_ = native_function;

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
          ptr->car_ = NewNil();
          ptr->cdr_ = NewNil();

          ptr = ptr->cdr_.get();
        }

        return ret_ptr;
      }

      /**
       * S式をパースする。
       * @param code Lispコード。
       * @return パース結果のベクトル。 パース不可能なら空のベクトル。
       */
      std::vector<LispObjectPtr> Parse(const std::string& code) {
        std::vector<LispObjectPtr> ret;

        // コードを字句解析。
        Tokenize(code);
        if (parentheses_ == 0) {
          while (!(token_queue_.empty())) {
            LispObjectPtr result(new LispObject());
            ParseCore(*result);
            ret.push_back(result);
          }

          // 終了処理。 初期化する。
          Reset();
        }

        return ret;
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

        ret_ptr->car_ = NewSymbol(error_symbol);
        ret_ptr->cdr_->car_ = NewString(message);

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
      int given);

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
      bool has_evaluated);

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - グローバルオブジェクト。
       * @return グローバルオブジェクト。
       */
      const LispObject& global() const {return *global_ptr_;}
      /**
       * アクセサ - ヘルプ辞書。
       * @return ヘルプ辞書。
       */
      const HelpDict& help() const {return help_;}

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * 文字列を字句解析する。
       * @param code 解析したいコード。
       * @return
       */
      void Tokenize(const std::string& code);

      /**
       * Parse()の本体。
       * @param target パース結果を格納するオブジェクト。
       */
      void ParseCore(LispObject& target);

      /**
       * コア関数を登録する。
       */
      void SetCoreFunctions();

      /**
       * 基本関数を登録する。
       */
      void SetBasicFunctions();

      // ========== //
      // メンバ変数 //
      // ========== //
      /** グローバルオブジェクト。 */
      LispObjectPtr global_ptr_;
      /** ヘルプ辞書。 */
      HelpDict help_;

      /** トークンを入れるキュー。 */
      std::queue<std::string> token_queue_;
      /** 解析用ストリーム。 */
      std::ostringstream token_stream_;
      /** 括弧の対応関係の数。 */
      int parentheses_;
      /** 現在コメント中かどうか。 */
      bool in_comment_;
      /** 現在文字列中かどうか。 */
      bool in_string_;
      /** 現在エスケープ文字中かどうか。 */
      bool in_escape_;
  };
}  // namespace Sayuri

#endif
