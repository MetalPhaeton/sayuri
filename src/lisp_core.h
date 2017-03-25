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
 * @file lisp_core.h
 * @author Hironori Ishibashi
 * @brief Lispインタープリタ。 
 */

#ifndef LISP_CORE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063
#define LISP_CORE_H_dd1bb50e_83bf_4b24_af8b_7c7bf60bc063

#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <memory>
#include <map>
#include <functional>
#include <sstream>
#include <iomanip>
#include <queue>
#include <cmath>
#include <random>
#include <chrono>
#include <ctime>
#include <thread>
#include <mutex>
#include <limits>
#include <stdexcept>

/** Sayuri 名前空間。 */
namespace Sayuri {
  class LObject;

  /** オブジェクトのタイプ定数。 */
  enum class LType {
    /** Nil。 */
    NIL,
    /** ペア。 */
    PAIR,
    /** シンボル。 */
    SYMBOL,
    /** 数字。 */
    NUMBER,
    /** 真偽値。 */
    BOOLEAN,
    /** 文字列。 */
    STRING,
    /** 関数。 */
    FUNCTION,
    /** ネイティブ関数。 */
    N_FUNCTION
  };

  /** オブジェクトのポインタ。 */
  using LPointer = std::shared_ptr<LObject>;
  /** オブジェクトのポインタのベクトル。 */
  using LPointerVec = std::vector<LPointer>;

  /** スコープ。 */
  using LScope = std::map<std::string, LPointer>;
  /** スコープのポインタ。 */
  using LScopePtr = std::shared_ptr<LScope>;
  /** スコープチェーン。 */
  class LScopeChain : public std::vector<LScopePtr> {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      LScopeChain() : std::vector<LScopePtr>() {
        push_back(LScopePtr(new LScope()));
      }
      /**
       * コピーコンストラクタ。
       * @param chain コピー元。
       */
      LScopeChain(const LScopeChain& chain) : std::vector<LScopePtr>(chain) {}
      /**
       * ムーブコンストラクタ。
       * @param chain ムーブ元。
       */
      LScopeChain(LScopeChain&& chain) : std::vector<LScopePtr>(chain) {}
      /**
       * コピー代入演算子。
       * @param chain コピー元。
       */
      virtual LScopeChain& operator=(const LScopeChain& chain) {
        std::vector<LScopePtr>::operator=(chain);
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param chain ムーブ元。
       */
      virtual LScopeChain& operator=(LScopeChain&& chain) {
        std::vector<LScopePtr>::operator=(chain);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LScopeChain() {}

      /**
       * シンボルを参照する。
       * @param symbol 参照するシンボル。
       * @return そのシンボルにバインドされているオブジェクトのポインタ。
       */
      const LPointer& SelectSymbol(const std::string& symbol) const {
        // 手前のスコープから順番に調べる。
        LScopeChain::const_reverse_iterator citr = crbegin();
        for (; citr != crend(); ++citr) {
          if ((*citr)->find(symbol) != (*citr)->end()) {
            return (*citr)->at(symbol);
          }
        }

        static const LPointer dummy_;
        return dummy_;
      }
      /**
       * シンボルを追加。
       * @param symbol 追加するシンボル。 一番近くのスコープに追加される。
       * @param ptr シンボルにバインドするオブジェクトのポインタ。
       */
      void InsertSymbol(const std::string& symbol, const LPointer& ptr) const {
        back()->emplace(symbol, ptr);
      }
      /**
       * シンボルを更新。
       * @param symbol 更新するシンボル。
       * @param ptr シンボルにバインドするオブジェクトのポインタ。
       */
      void UpdateSymbol(const std::string& symbol, const LPointer& ptr) const {
        // 手前のスコープから順番に調べる。
        LScopeChain::const_reverse_iterator citr = crbegin();
        for (; citr != crend(); ++citr) {
          if ((*citr)->find(symbol) != (*citr)->end()) {
            (*citr)->at(symbol) = ptr;
            return;
          }
        }
        InsertSymbol(symbol, ptr);
      }
      /**
       * シンボルを削除。
       * @param symbol 削除するシンボル。
       */
      void DeleteSymbol(const std::string& symbol) const {
        // 手前のスコープから順番に調べる。
        LScopeChain::const_reverse_iterator citr = crbegin();
        for (; citr != crend(); ++citr) {
          if ((*citr)->find(symbol) != (*citr)->end()) {
            (*citr)->erase(symbol);
            return;
          }
        }
      }

      /**
       * 新しいスコープを末尾に追加する。
       */
      void AppendNewScope() {
        push_back(LScopePtr(new LScope()));
      }
  };

  /** ウォーカー用関数。 void(current pair, current path) */
  using LFuncForWalk = std::function<void(LObject&, const std::string&)>;

  /**
   * ペアウォーカー。
   * @param func ペアの各要素に対して何かを行う関数。
   * @param pair 処理するペア。
   */
  void Walk(LObject& pair, const LFuncForWalk& func);

  /** マクロマップ用要素。 <置換前のシンボル名, 置換後> */
  using LMacroElm = std::pair<std::string, LPointer>;
  /** マクロマップ。 */
  using LMacroMap = std::vector<LMacroElm>;
  /**
   * マクロ展開用関数。
   * @param ptr マクロ展開するオブジェクトのポインタ。
   * @param macro_map マクロ用マップ。
   */
  void DevelopMacro(LObject* ptr, const LMacroMap& macro_map);

  /** 関数の引数名ベクトル。 */
  using LArgNames = std::vector<std::string>;

  /** C言語関数オブジェクト。 <結果(自分自身, 呼び出し元, 引数リスト)> */
  using LC_Function =
  std::function<LPointer(LPointer, LObject*, const LObject&)>;

  /** C言語関数宣言マクロ。 */
#define DEF_LC_FUNCTION(func_name) \
  LPointer func_name(LPointer self, LObject* caller, const LObject& args)

  /** 登録用C言語関数ラムダオブジェクトマクロ。 */
#define LC_FUNCTION_OBJ(func_name) \
  [this](LPointer self, LObject* caller, const LObject& args) -> LPointer {\
    return this->func_name(self, caller, args);\
  }

  /** スコープチェーン登録マクロ。 */
#define INSERT_LC_FUNCTION(funcobj, symbol, id) \
  scope_chain_.InsertSymbol(symbol, NewN_Function(funcobj, id, scope_chain_))

  /** オブジェクトのインターフェイス。 */
  class LObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      LObject() {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LObject(const LObject& obj) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LObject(LObject&& obj) {}
      /** デストラクタ。 */
      virtual ~LObject() {}

      // --- 実装必須関数 --- //
      /**
       * 自身をクローンコピーする。
       * @return 自身のクローン。
       */
      virtual LPointer Clone() const = 0;
      /**
       * 比較関数。 (==)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 同じならtrue。
       */
      virtual bool operator==(const LObject& obj) const = 0;
      /**
       * 比較関数。 (!=)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 違うならtrue。
       */
      virtual bool operator!=(const LObject& obj) const {
        return !(*this == obj);
      }
      /**
       * 自身のタイプを返す。
       * @return 自分のタイプ。
       */
      virtual LType type() const = 0;

      /**
       * 自身を文字列にする。
       * @return 自身の文字列。
       */
      virtual std::string ToString() const = 0;

      // --- オーバーライド用関数 --- //
      /**
       * 比較関数。 (>)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objより大きければtrue。
       */
      virtual bool operator>(const LObject& obj) const {return false;}
      /**
       * 比較関数。 (>=)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objと同じか大きければtrue。
       */
      virtual bool operator>=(const LObject& obj) const {return false;}
      /**
       * 比較関数。 (<)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objより小さければtrue。
       */
      virtual bool operator<(const LObject& obj) const {return false;}
      /**
       * 比較関数。 (<=)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objと同じか小さければtrue。
       */
      virtual bool operator<=(const LObject& obj) const {return false;}

      /**
       * オブジェクトを評価する。
       * @param target 評価するオブジェクト。
       * @return 結果。
       */
      virtual LPointer Evaluate(const LPointer& target) {
        throw std::logic_error("Called invalid Evaluate().");
      }

      /**
       * 自身の関数を適用する。
       * @param caller 関数の呼び出し元。
       * @param target 引数オブジェクト。
       * @return 結果。
       */
      virtual LPointer Apply(LObject* caller, const LObject& args) {
        throw std::logic_error("Called invalid Apply().");
      }

      /**
       * 自分がリストかどうか。
       * @return リストならtrue。
       */
      virtual bool IsList() const {return false;}

      /**
       * アクセサ - Car。
       * @return Car。
       */
      virtual const LPointer& car() const {
        throw std::logic_error("Called invalid car().");
      }
      /**
       * アクセサ - Cdr。
       * @return Cdr。
       */
      virtual const LPointer& cdr() const {
        throw std::logic_error("Called invalid cdr().");
      }
      /**
       * アクセサ - シンボル。
       * @return シンボル。
       */
      virtual const std::string& symbol() const {
        throw std::logic_error("Called invalid symbol().");
      }
      /**
       * アクセサ - 数字。
       * @return 数字。
       */
      virtual double number() const {
        throw std::logic_error("Called invalid number().");
      }
      /**
       * アクセサ - 真偽値。
       * @return 真偽値。
       */
      virtual bool boolean() const {
        throw std::logic_error("Called invalid number().");
      }
      /**
       * アクセサ - 文字列。
       * @return 文字列。
       */
      virtual const std::string& string() const {
        throw std::logic_error("Called invalid string().");
      }
      /**
       * アクセサ - 関数の引数名ベクトル。
       * @return 関数の引数名ベクトル。
       */
      virtual const LArgNames& arg_names() const {
        throw std::logic_error("Called invalid arg_name().");
      }
      /**
       * アクセサ - 関数の式。
       * @return 関数の式。
       */
      virtual const LPointerVec& expression() const {
        throw std::logic_error("Called invalid expression().");
      }
      /**
       * アクセサ - ネイティブ関数の実体。
       * @return ネイティブ関数の実体。
       */
      virtual const LC_Function& c_function() const {
        throw std::logic_error("Called invalid c_function().");
      }
      /**
       * アクセサ - ネイティブ関数用識別文字列。
       * @return ネイティブ関数用識別文字列。
       */
      virtual const std::string& func_id() const {
        throw std::logic_error("Called invalid func_id().");
      }
      /**
       * アクセサ - スコープチェーン。
       * @return スコープチェーン。
       */
      virtual const LScopeChain& scope_chain() const {
        throw std::logic_error("Called invalid scope_chain().");
      }

      /**
       * ミューテータ - Car。
       * @param ptr Car。
       */
      virtual void car(const LPointer& ptr) {
        throw std::logic_error("Called invalid car(...).");
      }
      /**
       * ミューテータ - Cdr。
       * @param ptr Cdr。
       */
      virtual void cdr(const LPointer& ptr) {
        throw std::logic_error("Called invalid cdr(...).");
      }
      /**
       * ミューテータ - シンボル。
       * @param symbol シンボル。
       */
      virtual void symbol(const std::string& symbol) {
        throw std::logic_error("Called invalid symbol(...).");
      }
      /**
       * ミューテータ - 数字。
       * @param number 数字。
       */
      virtual void number(double number) {
        throw std::logic_error("Called invalid number(...).");
      }
      /**
       * ミューテータ - 真偽値。
       * @param boolean 真偽値。
       */
      virtual void boolean(bool boolean) {
        throw std::logic_error("Called invalid boolean(...).");
      }
      /**
       * ミューテータ - 文字列。
       * @param string 文字列。
       */
      virtual void string(const std::string& string) {
        throw std::logic_error("Called invalid string(...).");
      }
      /**
       * ミューテータ - 関数の引数名ベクトル。
       * @param arg_names 関数の引数名ベクトル。
       */
      virtual void arg_names(const LArgNames& arg_name) {
        throw std::logic_error("Called invalid arg_name(...).");
      }
      /**
       * ミューテータ - 関数の式。
       * @param expr 関数の式。
       */
      virtual void expression(const LPointerVec& expression) {
        throw std::logic_error("Called invalid expression(...).");
      }
      /**
       * ミューテータ - ネイティブ関数の実体。
       * @param c_function ネイティブ関数の実体。
       */
      virtual void c_function(const LC_Function& c_function) {
        throw std::logic_error("Called invalid c_function(...).");
      }
      /**
       * ミューテータ - ネイティブ関数用識別文字列。
       * @return ネイティブ関数用識別文字列。
       */
      virtual void func_id(const std::string& id) {
        throw std::logic_error("Called invalid func_id(...).");
      }
      /**
       * ミューテータ - スコープチェーン。
       * @param scope_chain スコープチェーン。
       */
      virtual void scope_chain(const LScopeChain& scope_chain) {
        throw std::logic_error("Called invalid scope_chain(...).");
      }

      // --- オーバーライドしなくていい関数 --- //
      /**
       * 自分がNilかどうか。
       * @return Nilならtrue。
       */
      virtual bool IsNil() const {return type() == LType::NIL;}
      /**
       * 自分がペアかどうか。
       * @return ペアならtrue。
       */
      virtual bool IsPair() const {return type() == LType::PAIR;}
      /**
       * 自分がシンボルかどうか。
       * @return シンボルならtrue。
       */
      virtual bool IsSymbol() const {return type() == LType::SYMBOL;}
      /**
       * 自分が数字かどうか。
       * @return 数字ならtrue。
       */
      virtual bool IsNumber() const {return type() == LType::NUMBER;}
      /**
       * 自分が真偽値かどうか。
       * @return 真偽値ならtrue。
       */
      virtual bool IsBoolean() const {return type() == LType::BOOLEAN;}
      /**
       * 自分が文字列かどうか。
       * @return 文字列ならtrue。
       */
      virtual bool IsString() const {return type() == LType::STRING;}
      /**
       * 自分が関数かどうか。
       * @return 関数ならtrue。
       */
      virtual bool IsFunction() const {return type() == LType::FUNCTION;}
      /**
       * 自分がネイティブ関数かどうか。
       * @return ネイティブ関数ならtrue。
       */
      virtual bool IsN_Function() const {return type() == LType::N_FUNCTION;}
  };

  /** Nilオブジェクト。 */
  class LNil : public LObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      LNil() {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LNil(const LNil& obj) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LNil(LNil&& obj) {}
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      virtual LNil& operator=(const LNil& obj) {
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      virtual LNil& operator=(LNil&& obj) {
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LNil() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 自身をクローンコピーする。
       * @return 自身のクローン。
       */
      virtual LPointer Clone() const override {
        return std::make_shared<LNil>();
      }
      /**
       * 比較関数。 (==)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 同じならtrue。
       */
      virtual bool operator==(const LObject& obj) const override {
        return obj.IsNil();
      }
      /**
       * 自身のタイプを返す。
       * @return 自分のタイプ。
       */
      virtual LType type() const override {
        return LType::NIL;
      };

      /**
       * 自分がリストかどうか。
       * @return リストならtrue。
       */
      virtual bool IsList() const override {return true;}

      /**
       * 自身を文字列にする。
       * @return 自身の文字列。
       */
      virtual std::string ToString() const override {
        return std::string("()");
      }
  };

  /** ペアオブジェクト。 */
  class LPair : public LObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param car Car。
       * @param cdr Cdr。
       */
      LPair(const LPointer& car, const LPointer& cdr) :
      car_(car), cdr_(cdr) {}
      /** コンストラクタ。 */
      LPair() : car_(std::make_shared<LNil>()),
      cdr_(std::make_shared<LNil>()) {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LPair(const LPair& obj) : car_(obj.car_), cdr_(obj.cdr_) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LPair(LPair&& obj) :
      car_(std::move(obj.car_)), cdr_(std::move(obj.cdr_)) {}
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      virtual LPair& operator=(const LPair& obj) {
        car_ = obj.car_;
        cdr_ = obj.cdr_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      virtual LPair& operator=(LPair&& obj) {
        car_ = std::move(obj.car_);
        cdr_ = std::move(obj.cdr_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LPair() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 自身をクローンコピーする。
       * @return 自身のクローン。
       */
      virtual LPointer Clone() const override {
        return std::make_shared<LPair>(car_->Clone(), cdr_->Clone());
      }
      /**
       * 比較関数。 (==)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 同じならtrue。
       */
      virtual bool operator==(const LObject& obj) const override {
        if (obj.IsPair()) {
          return (*car_ == *(obj.car())) && (*cdr_ == *(obj.cdr()));
        }
        return false;
      }
      /**
       * 自身のタイプを返す。
       * @return 自分のタイプ。
       */
      virtual LType type() const override {
        return LType::PAIR;
      };

      /**
       * 自身を文字列にする。
       * @return 自身の文字列。
       */
      virtual std::string ToString() const override {
        std::ostringstream oss;
        oss << "(";
        const LObject* ptr = this;
        for (;ptr->IsPair(); ptr = ptr->cdr().get()) {
          oss << ptr->car()->ToString() << " ";
        }
        if (!(ptr->IsNil())) {
          oss << ". " << ptr->ToString() << ")";
        } else {
          oss.seekp(oss.str().size() - 1);
          oss << ")";
        }
        return oss.str();
      }

      /**
       * 自分がリストかどうか。
       * @return リストならtrue。
       */
      virtual bool IsList() const override {
        const LObject* ptr = this;
        for (; ptr->IsPair(); ptr = ptr->cdr().get()) continue;
        return ptr->IsList();
      }

      /**
       * アクセサ - Car。
       * @return Car。
       */
      virtual const LPointer& car() const override {return car_;}
      /**
       * アクセサ - Cdr。
       * @return Cdr。
       */
      virtual const LPointer& cdr() const override {return cdr_;}

      /**
       * ミューテータ - Car。
       * @param ptr Car。
       */
      virtual void car(const LPointer& ptr) override {car_ = ptr;}
      /**
       * ミューテータ - Cdr。
       * @param ptr Cdr。
       */
      virtual void cdr(const LPointer& ptr) override {cdr_ = ptr;}

    protected:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** Car。 */
      LPointer car_;
      /** Cdr。 */
      LPointer cdr_;
  };

  /** シンボルオブジェクト。 */
  class LSymbol : public LObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param symbol シンボル。
       */
      LSymbol(const std::string& symbol) : symbol_(symbol) {}
      /**
       * コンストラクタ2。
       * @param symbol シンボル。
       */
      LSymbol(std::string&& symbol) : symbol_(symbol) {}
      /** コンストラクタ。 */
      LSymbol() : symbol_("") {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LSymbol(const LSymbol& obj) : symbol_(obj.symbol_) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LSymbol(LSymbol&& obj) :
      symbol_(std::move(obj.symbol_)) {}
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      virtual LSymbol& operator=(const LSymbol& obj) {
        symbol_ = obj.symbol_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      virtual LSymbol& operator=(LSymbol&& obj) {
        symbol_ = std::move(obj.symbol_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LSymbol() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 自身をクローンコピーする。
       * @return 自身のクローン。
       */
      virtual LPointer Clone() const override {
        return std::make_shared<LSymbol>(symbol_);
      }
      /**
       * 比較関数。 (==)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 同じならtrue。
       */
      virtual bool operator==(const LObject& obj) const override {
        if (obj.IsSymbol()) {
          return symbol_ == obj.symbol();
        }
        return false;
      }
      /**
       * 自身のタイプを返す。
       * @return 自分のタイプ。
       */
      virtual LType type() const override {
        return LType::SYMBOL;
      };

      /**
       * 自身を文字列にする。
       * @return 自身の文字列。
       */
      virtual std::string ToString() const override {
        return symbol_;
      }

      /**
       * アクセサ - シンボル。
       * @return シンボル。
       */
      virtual const std::string& symbol() const override {
        return symbol_;
      }
      /**
       * ミューテータ - シンボル。
       * @param symbol シンボル。
       */
      virtual void symbol(const std::string& symbol) override {
        symbol_ = symbol;
      }
      /**
       * ミューテータ - シンボル。
       * @param symbol シンボル。
       */
      virtual void symbol(std::string&& symbol) {
        symbol_ = symbol;
      }

    protected:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** シンボル。 */
      std::string symbol_;
  };

  /** 数字オブジェクト。 */
  class LNumber : public LObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param number 数字。
       */
      LNumber(double number) : number_(number) {}
      /** コンストラクタ。 */
      LNumber() : number_(0.0) {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LNumber(const LNumber& obj) : number_(obj.number_) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LNumber(LNumber&& obj) : number_(obj.number_) {}
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      virtual LNumber& operator=(const LNumber& obj) {
        number_ = obj.number_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      virtual LNumber& operator=(LNumber&& obj) {
        number_ = obj.number_;
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LNumber() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 自身をクローンコピーする。
       * @return 自身のクローン。
       */
      virtual LPointer Clone() const override {
        return std::make_shared<LNumber>(number_);
      }
      /**
       * 比較関数。 (==)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 同じならtrue。
       */
      virtual bool operator==(const LObject& obj) const override {
        if (obj.IsNumber()) {
          return number_ == obj.number();
        }
        return false;
      }
      /**
       * 自身のタイプを返す。
       * @return 自分のタイプ。
       */
      virtual LType type() const override {
        return LType::NUMBER;
      };

      /**
       * 自身を文字列にする。
       * @return 自身の文字列。
       */
      virtual std::string ToString() const override {
        std::ostringstream oss;
        oss << std::setprecision(15) << number_;
        return oss.str();
      }

      /**
       * 比較関数。 (>)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objより大きければtrue。
       */
      virtual bool operator>(const LObject& obj) const override {
        if (obj.IsNumber()) {
          return number_ > obj.number();
        }
        return false;
      }
      /**
       * 比較関数。 (>=)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objと同じか大きければtrue。
       */
      virtual bool operator>=(const LObject& obj) const override {
        if (obj.IsNumber()) {
          return number_ >= obj.number();
        }
        return false;
      }
      /**
       * 比較関数。 (<)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objより小さければtrue。
       */
      virtual bool operator<(const LObject& obj) const override {
        if (obj.IsNumber()) {
          return number_ < obj.number();
        }
        return false;
      }
      /**
       * 比較関数。 (<=)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objと同じか小さければtrue。
       */
      virtual bool operator<=(const LObject& obj) const override {
        if (obj.IsNumber()) {
          return number_ <= obj.number();
        }
        return false;
      }
      /**
       * アクセサ - 数字。
       * @return 数字。
       */
      virtual double number() const override {
        return number_;
      }

      /**
       * ミューテータ - 数字。
       * @param number 数字。
       */
      virtual void number(double number) override {
        number_ = number;
      }

    protected:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** 数字。 */
      double number_;
  };

  /** 真偽値オブジェクト。 */
  class LBoolean : public LObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param boolean 真偽値。
       */
      LBoolean(bool boolean) : boolean_(boolean) {}
      /** コンストラクタ。 */
      LBoolean() : boolean_(false) {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LBoolean(const LBoolean& obj) : boolean_(obj.boolean_) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LBoolean(LBoolean&& obj) : boolean_(obj.boolean_) {}
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      virtual LBoolean& operator=(const LBoolean& obj) {
        boolean_ = obj.boolean_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      virtual LBoolean& operator=(LBoolean&& obj) {
        boolean_ = obj.boolean_;
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LBoolean() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 自身をクローンコピーする。
       * @return 自身のクローン。
       */
      virtual LPointer Clone() const override {
        return std::make_shared<LBoolean>(boolean_);
      }
      /**
       * 比較関数。 (==)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 同じならtrue。
       */
      virtual bool operator==(const LObject& obj) const override {
        if (obj.IsBoolean()) {
          return boolean_ == obj.boolean();
        }
        return false;
      }
      /**
       * 自身のタイプを返す。
       * @return 自分のタイプ。
       */
      virtual LType type() const override {
        return LType::BOOLEAN;
      };

      /**
       * 自身を文字列にする。
       * @return 自身の文字列。
       */
      virtual std::string ToString() const override {
        return boolean_ ? std::string("#t") : std::string("#f");
      }

      /**
       * 比較関数。 (>)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objより大きければtrue。
       */
      virtual bool operator>(const LObject& obj) const override {
        if (obj.IsBoolean()) {
          return boolean_ > obj.boolean();
        }
        return false;
      }
      /**
       * 比較関数。 (>=)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objと同じか大きければtrue。
       */
      virtual bool operator>=(const LObject& obj) const override {
        if (obj.IsBoolean()) {
          return boolean_ >= obj.boolean();
        }
        return false;
      }
      /**
       * 比較関数。 (<)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objより小さければtrue。
       */
      virtual bool operator<(const LObject& obj) const override {
        if (obj.IsBoolean()) {
          return boolean_ < obj.boolean();
        }
        return false;
      }
      /**
       * 比較関数。 (<=)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objと同じか小さければtrue。
       */
      virtual bool operator<=(const LObject& obj) const override {
        if (obj.IsBoolean()) {
          return boolean_ <= obj.boolean();
        }
        return false;
      }

      /**
       * アクセサ - 真偽値。
       * @return 真偽値。
       */
      virtual bool boolean() const override {
        return boolean_;
      }

      /**
       * ミューテータ - 真偽値。
       * @param boolean 真偽値。
       */
      virtual void boolean(bool boolean) override {
        boolean_ = boolean;
      }

    protected:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** 真偽値。 */
      bool boolean_;
  };

  /** 文字列オブジェクト。 */
  class LString : public LObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param string 文字列。
       */
      LString(const std::string& string) : string_(string) {}
      /**
       * コンストラクタ2。
       * @param string 文字列。
       */
      LString(std::string&& string) : string_(string) {}
      /** コンストラクタ。 */
      LString() : string_("") {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LString(const LString& obj) : string_(obj.string_) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LString(LString&& obj) : string_(std::move(obj.string_)) {}
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      virtual LString& operator=(const LString& obj) {
        string_ = obj.string_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      virtual LString& operator=(LString&& obj) {
        string_ = std::move(obj.string_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LString() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 自身をクローンコピーする。
       * @return 自身のクローン。
       */
      virtual LPointer Clone() const override {
        return std::make_shared<LString>(string_);
      }
      /**
       * 比較関数。 (==)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 同じならtrue。
       */
      virtual bool operator==(const LObject& obj) const override {
        if (obj.IsString()) {
          return string_ == obj.string();
        }
        return false;
      }
      /**
       * 自身のタイプを返す。
       * @return 自分のタイプ。
       */
      virtual LType type() const override {
        return LType::STRING;
      };

      /**
       * 自身を文字列にする。
       * @return 自身の文字列。
       */
      virtual std::string ToString() const override {
        std::ostringstream oss;
        for (auto c : string_) {
          switch (c) {
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\f': oss << "\\f"; break;
            case '\t': oss << "\\t"; break;
            case '\v': oss << "\\v"; break;
            case '\a': oss << "\\a"; break;
            case '\b': oss << "\\b"; break;
            case '\0': oss << "\\0"; break;
            case '"': oss << "\\\""; break;
            case '\x1b': oss << "\\e"; break;
            default: oss << c;
          }
        }
        return "\"" + oss.str() + "\"";
      }

      /**
       * 比較関数。 (>)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objより大きければtrue。
       */
      virtual bool operator>(const LObject& obj) const override {
        if (obj.IsString()) {
          return string_ > obj.string();
        }
        return false;
      }
      /**
       * 比較関数。 (>=)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objと同じか大きければtrue。
       */
      virtual bool operator>=(const LObject& obj) const override {
        if (obj.IsString()) {
          return string_ >= obj.string();
        }
        return false;
      }
      /**
       * 比較関数。 (<)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objより小さければtrue。
       */
      virtual bool operator<(const LObject& obj) const override {
        if (obj.IsString()) {
          return string_ < obj.string();
        }
        return false;
      }
      /**
       * 比較関数。 (<=)
       * @param obj 比較するオブジェクトのポインタ。
       * @return objと同じか小さければtrue。
       */
      virtual bool operator<=(const LObject& obj) const override {
        if (obj.IsString()) {
          return string_ <= obj.string();
        }
        return false;
      }

      /**
       * アクセサ - 文字列。
       * @return 文字列。
       */
      virtual const std::string& string() const override {
        return string_;
      }

      /**
       * ミューテータ - 文字列。
       * @param string 文字列。
       */
      virtual void string(const std::string& string) override {
        string_ = string;
      }

      /**
       * ミューテータ - 文字列。
       * @param string 文字列。
       */
      virtual void string(std::string&& string) {
        string_ = string;
      }

    protected:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** 文字列。 */
      std::string string_;
  };

  /** リスプの関数オブジェクト。 */
  class LFunction : public LObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param arg_names 引数のベクトル。
       * @param expression 式のベクトル。
       * @param scope_chain スコープチェーン。
       */
      LFunction(const LArgNames& arg_names, const LPointerVec& expression,
      const LScopeChain& scope_chain) :
      arg_names_(arg_names), expression_(expression),
      scope_chain_(scope_chain) {}
      /** コンストラクタ。 */
      LFunction() {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LFunction(const LFunction& obj) :
      arg_names_(obj.arg_names_), expression_(obj.expression_),
      scope_chain_(obj.scope_chain_) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LFunction(LFunction&& obj) :
      arg_names_(std::move(obj.arg_names_)),
      expression_(std::move(obj.expression_)),
      scope_chain_(std::move(obj.scope_chain_)) {}
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      virtual LFunction& operator=(const LFunction& obj) {
        arg_names_ = obj.arg_names_;
        expression_ = obj.expression_;
        scope_chain_ = obj.scope_chain_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      virtual LFunction& operator=(LFunction&& obj) {
        arg_names_ = std::move(obj.arg_names_);
        expression_ = std::move(obj.expression_);
        scope_chain_ = std::move(obj.scope_chain_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LFunction() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 自身をクローンコピーする。
       * @return 自身のクローン。
       */
      virtual LPointer Clone() const override {
        return std::make_shared<LFunction>(*this);
      }
      /**
       * 比較関数。 (==)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 同じならtrue。
       */
      virtual bool operator==(const LObject& obj) const override {
        if (obj.IsFunction()) {
          LArgNames target_arg_names = obj.arg_names();
          LPointerVec target_expression = obj.expression();
          LScopeChain target_chain = obj.scope_chain();
          if (arg_names_.size() != target_arg_names.size()) return false;
          for (unsigned int i = 0; i < arg_names_.size(); ++i) {
            if (arg_names_[i] != target_arg_names[i]) return false;
          }
          if (expression_.size() != target_expression.size()) return false;
          for (unsigned int i = 0; i < expression_.size(); ++i) {
            if (expression_[i] != target_expression[i]) return false;
          }
          return true;
        }
        return false;
      }
      /**
       * オブジェクトを評価する。
       * @param target 評価するオブジェクト。
       * @return 結果。
       */
      virtual LPointer Evaluate(const LPointer& target) override;
      /**
       * 自身の関数を適用する。
       * @param caller 関数の呼び出し元。
       * @param target 引数オブジェクト。
       * @return 結果。
       */
      virtual LPointer Apply(LObject* caller, const LObject& args) override;
      /**
       * 自身のタイプを返す。
       * @return 自分のタイプ。
       */
      virtual LType type() const override {
        return LType::FUNCTION;
      };

      /**
       * 自身を文字列にする。
       * @return 自身の文字列。
       */
      virtual std::string ToString() const override {
        std::ostringstream oss;
        oss << "(lambda (";
        if (arg_names_.size()) {
          for (auto& name : arg_names_) {
            oss << name << " ";
          }
          oss.seekp(oss.str().size() - 1);
        }
        oss << ") ";
        if (expression_.size()) {
          for (auto& expr : expression_) {
            oss << expr->ToString() << " ";
          }
          oss.seekp(oss.str().size() - 1);
        }
        oss << ")";
        return oss.str();
      }

      /**
       * アクセサ - 関数の引数名ベクトル。
       * @return 関数の引数名ベクトル。
       */
      virtual const LArgNames& arg_names() const override {
        return arg_names_;
      }
      /**
       * アクセサ - 関数の式。
       * @return 関数の式。
       */
      virtual const LPointerVec& expression() const override {
        return expression_;
      }
      /**
       * アクセサ - スコープチェーン。
       * @return スコープチェーン。
       */
      virtual const LScopeChain& scope_chain() const override {
        return scope_chain_;
      }

      /**
       * ミューテータ - 関数の引数名ベクトル。
       * @param arg_names 関数の引数名ベクトル。
       */
      virtual void arg_names(const LArgNames& arg_name) override {
        arg_names_ = arg_name;
      }
      /**
       * ミューテータ - 関数の式。
       * @param expr 関数の式。
       */
      virtual void expression(const LPointerVec& expression) override {
        expression_ = expression;
      }
      /**
       * ミューテータ - スコープチェーン。
       * @param scope_chain スコープチェーン。
       */
      virtual void scope_chain(const LScopeChain& scope_chain) override {
        scope_chain_ = scope_chain;
      }

    protected:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** 引数名ベクトル。 */
      LArgNames arg_names_;
      /** 式のベクトル。 */
      LPointerVec expression_;
      /** スコープチェーン。 */
      LScopeChain scope_chain_;
  };

  /** ネイティブ関数オブジェクト。 */
  class LN_Function : public LObject {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param c_function C言語関数オブジェクト。
       * @param func_id 関数識別文字列。
       * @param scope_chain スコープチェーン。
       */
      LN_Function(const LC_Function& c_function,
      const std::string& func_id, const LScopeChain& scope_chain) :
      c_function_(c_function),
      func_id_(func_id), scope_chain_(scope_chain) {}
      /** コンストラクタ。 */
      LN_Function() {}
      /**
       * コピーコンストラクタ。
       * @param obj コピー元。
       */
      LN_Function(const LN_Function& obj) :
      c_function_(obj.c_function_),
      func_id_(obj.func_id_),
      scope_chain_(obj.scope_chain_) {}
      /**
       * ムーブコンストラクタ。
       * @param obj ムーブ元。
       */
      LN_Function(LN_Function&& obj) :
      c_function_(std::move(obj.c_function_)),
      func_id_(std::move(obj.func_id_)),
      scope_chain_(std::move(obj.scope_chain_)) {}
      /**
       * コピー代入演算子。
       * @param obj コピー元。
       */
      virtual LN_Function& operator=(const LN_Function& obj) {
        c_function_ = obj.c_function_;
        func_id_ = obj.func_id_;
        scope_chain_ = obj.scope_chain_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param obj ムーブ元。
       */
      virtual LN_Function& operator=(LN_Function&& obj) {
        c_function_ = std::move(obj.c_function_);
        func_id_ = std::move(obj.func_id_);
        scope_chain_ = std::move(obj.scope_chain_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LN_Function() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 自身をクローンコピーする。
       * @return 自身のクローン。
       */
      virtual LPointer Clone() const override {
        return std::make_shared<LN_Function>(*this);
      }
      /**
       * 比較関数。 (==)
       * @param obj 比較するオブジェクトのポインタ。
       * @return 同じならtrue。
       */
      virtual bool operator==(const LObject& obj) const override {
        if (obj.IsN_Function()) {
          if (func_id_ == obj.func_id()) {
            return true;
          }
        }
        return false;
      }
      /**
       * オブジェクトを評価する。
       * @param target 評価するオブジェクト。
       * @return 結果。
       */
      virtual LPointer Evaluate(const LPointer& target) override;
      /**
       * 自身の関数を適用する。
       * @param caller 関数の呼び出し元。
       * @param target 引数オブジェクト。
       * @return 結果。
       */
      virtual LPointer Apply(LObject* caller, const LObject& args) override {
        scope_chain_.AppendNewScope();
        LPointer ret_ptr = c_function_(this->Clone(), caller, args);
        scope_chain_.pop_back();
        return ret_ptr;
      }
      /**
       * 自身のタイプを返す。
       * @return 自分のタイプ。
       */
      virtual LType type() const override {
        return LType::N_FUNCTION;
      };

      /**
       * 自身を文字列にする。
       * @return 自身の文字列。
       */
      virtual std::string ToString() const override {
        std::ostringstream oss;
        oss << "Native Function: " << func_id();
        return oss.str();
      }

      /**
       * アクセサ - ネイティブ関数の実体。
       * @return ネイティブ関数の実体。
       */
      virtual const LC_Function& c_function() const override {
        return c_function_;
      }
      /**
       * アクセサ - ネイティブ関数用識別文字列。
       * @return ネイティブ関数用識別文字列。
       */
      virtual const std::string& func_id() const override {
        return func_id_;
      }
      /**
       * アクセサ - スコープチェーン。
       * @return スコープチェーン。
       */
      virtual const LScopeChain& scope_chain() const override {
        return scope_chain_;
      }

      /**
       * ミューテータ - ネイティブ関数の実体。
       * @param c_function ネイティブ関数の実体。
       */
      virtual void c_function(const LC_Function& c_function) override {
        c_function_ = c_function;
      }
      /**
       * ミューテータ - ネイティブ関数用識別文字列。
       * @param func_id ネイティブ関数用識別文字列。
       */
      virtual void func_id(const std::string& func_id) override {
        func_id_ = func_id;
      }
      /**
       * ミューテータ - スコープチェーン。
       * @param scope_chain スコープチェーン。
       */
      virtual void scope_chain(const LScopeChain& scope_chain) override {
        scope_chain_ = scope_chain;
      }

    protected:
      // ========== //
      // メンバ変数 //
      // ========== //
      /** C言語関数。 */
      LC_Function c_function_;
      /** 関数用識別文字列。 */
      std::string func_id_;
      /** スコープチェーン。 */
      LScopeChain scope_chain_;
  };

  /** パーサクラス。 */
  class LParser {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /** コンストラクタ。 */
      LParser() : parenth_counter_(0), in_string_(false) {}
      /**
       * コピーコンストラクタ。
       * @param parser コピー元。
       */
      LParser(const LParser& parser) :
      parenth_counter_(parser.parenth_counter_),
      in_string_(parser.in_string_),
      token_queue_(parser.token_queue_) {}
      /**
       * ムーブコンストラクタ。
       * @param parser ムーブ元。
       */
      LParser(LParser&& parser) :
      parenth_counter_(parser.parenth_counter_),
      in_string_(parser.in_string_),
      token_queue_(std::move(parser.token_queue_)) {}
      /**
       * コピー代入演算子。
       * @param parser コピー元。
       */
      LParser& operator=(const LParser& parser) {
        parenth_counter_ = parser.parenth_counter_;
        in_string_ = parser.in_string_;
        token_queue_ = parser.token_queue_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param parser ムーブ元。
       */
      LParser& operator=(LParser&& parser) {
        parenth_counter_ = parser.parenth_counter_;
        in_string_ = parser.in_string_;
        token_queue_ = std::move(parser.token_queue_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LParser() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 字句解析する。 token_queue_に格納される。
       * @param code 解析するコード。
       */
      void Tokenize(const std::string& code);
      /**
       * 字句解析したトークンキューをパースする。
       * @return パース結果のベクトル。
       */
      LPointerVec Parse();

      /**
       * パーサの状態をリセットする。　
       */
      void Reset() {
        parenth_counter_ = 0;
        in_string_ = false;
        token_queue_ = std::queue<std::string>();
      }

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - カッコの整合性。
       * @return カッコの整合性。
       */
      int parenth_counter() const {return parenth_counter_;}
      /**
       * アクセサ - 文字列中かどうか。
       * @return 文字列中かどうか。
       */
      bool in_string() const {return in_string_;}
      /**
       * アクセサ - 解析中のトークンのキュー。
       * @return 解析中のトークンのキュー。
       */
      const std::queue<std::string>& token_quene() const {
        return token_queue_;
      }

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * 再帰コール用パーサ。
       * @return パース結果のポインタ。
       */
      LPointer ParseCore();

      // ========== //
      // メンバ変数 //
      // ========== //
      // --- 字句解析器 --- //
      /** カッコの数合わせ。 */
      int parenth_counter_;
      /** 文字列解析中かどうか。 */
      bool in_string_;
      /** トークンのベクトル。 */
      std::queue<std::string> token_queue_;
  };

  /** インタープリタの実体。 */
  class Lisp : public LN_Function {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param argv コマンド引数。
       */
      Lisp(const std::vector<std::string>& argv);
      /** コンストラクタ。 */
      Lisp();
      /**
       * コピーコンストラクタ。
       * @param lisp コピー元。
       */
      Lisp(const Lisp& lisp);
      /**
       * ムーブコンストラクタ。
       * @param lisp ムーブ元。
       */
      Lisp(Lisp&& lisp);
      /**
       * コピー代入演算子。
       * @param lisp コピー元。
       */
      Lisp& operator=(const Lisp& lisp);
      /**
       * ムーブ代入演算子。
       * @param lisp ムーブ元。
       */
      Lisp& operator=(Lisp&& lisp);
      /** デストラクタ。 */
      virtual ~Lisp() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * Nilを作る。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewNil() {
        return std::make_shared<LNil>();
      }
      /**
       * ペアを作る。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewPair() {
        return std::make_shared<LPair>();
      }
      /**
       * ペアを作る。
       * @param car Car。
       * @param cdr Cdr。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewPair(const LPointer& car, const LPointer& cdr) {
        return std::make_shared<LPair>(car, cdr);
      }
      /**
       * シンボルを作る。
       * @param symbol シンボル。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewSymbol(const std::string& symbol) {
        return std::make_shared<LSymbol>(symbol);
      }
      /**
       * シンボルを作る2。
       * @param symbol シンボル。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewSymbol(std::string&& symbol) {
        return std::make_shared<LSymbol>(symbol);
      }
      /**
       * 数字を作る。
       * @param number 数字。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewNumber(double number) {
        return std::make_shared<LNumber>(number);
      }
      /**
       * 真偽値を作る。
       * @param boolean 真偽値。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewBoolean(bool boolean) {
        return std::make_shared<LBoolean>(boolean);
      }
      /**
       * 文字列を作る。
       * @param string 文字列。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewString(const std::string& string) {
        return std::make_shared<LString>(string);
      }
      /**
       * 文字列を作る2。
       * @param string 文字列。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewString(std::string&& string) {
        return std::make_shared<LString>(string);
      }
      /**
       * 関数オブジェクトを作る。
       * @param arg_names 引数のベクトル。
       * @param expression 式のベクトル。
       * @param scope_chain スコープチェーン。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewFunction(const LArgNames& arg_names,
      const LPointerVec& expression, const LScopeChain& scope_chain) {
        return std::make_shared<LFunction>(arg_names, expression, scope_chain);
      }
      /**
       * ネイティブ関数オブジェクトを作る。
       * @param c_function ネイティブ関数。
       * @param func_id 識別文字列。
       * @param scope_chain スコープチェーン。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewN_Function(const LC_Function& c_function,
      const std::string& func_id,
      const LScopeChain& scope_chain) {
        return std::make_shared<LN_Function>(c_function, func_id, scope_chain);
      }

      /**
       * 次の要素のPairのポインタにシフトする。
       * @param ptr シフトするポインタのポインタ。
       */
      static void Next(LObject** ptr_ptr) {
        *ptr_ptr = (*ptr_ptr)->cdr().get();
      }
      /**
       * リストを作る。
       * @param length リストの長さ。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewList(int length) {
        if (length <= 0) return NewNil();
        return NewPair(NewNil(), NewList(length - 1));
      }

      /**
       * エラーオブジェクトを作る。
       * @param symbol エラーのシンボル。
       * @param message エラーメッセージ。
       * @return エラーオブジェクト。
       */
      static LPointer GenError(const std::string& symbol,
      const std::string& message) {
        return NewPair(NewSymbol(symbol), NewPair(NewString(message),
        NewNil()));
      }

      /**
       * タイプからタイプ名の名前を得る。
       * @param type 名前を得たいタイプ。
       * @return 名前。
       */
      static const std::string& TypeToName(LType type) {
        static const std::string type_names[] {
          "Nil", "Pair", "Symbol", "Number", "Boolean", "String",
          "Function", "Native Function"
        };
        return type_names[static_cast<int>(type)];
      }

      /**
       * Listの長さを数える。
       * @param list 数えるリスト。
       * @return Listの長さ。
       */
      static int CountList(const LObject& list) {
        int ret = 0;
        for (const LObject* ptr = &list; ptr->IsPair();
        ptr = ptr->cdr().get()) {
          ++ret;
        }

        return ret;
      }

      /**
       * リストの後ろに要素を追加し、その要素のペアへのポインタを得る。
       * @param list_ptr そのリストのポインタ。
       * @param element 追加したい要素。
       * @param back_ptr_ptr 最終ペアへのポインタのポインタ。
       */
      static void AppendElementAndGetBack(LPointer& list_ptr,
      const LPointer& element, LObject** back_ptr_ptr) {
        if (*back_ptr_ptr) {
          (*back_ptr_ptr)->cdr(NewPair(element, NewNil()));
          *back_ptr_ptr = (*back_ptr_ptr)->cdr().get();
        } else {
          list_ptr = NewPair(element, NewNil());
          *back_ptr_ptr = list_ptr.get();
        }
      }

      /**
       * ListからLPointerVecを作成する。
       * @param list 変換するリスト。
       * @return 変換後のLPointerVec。
       */
      static LPointerVec ListToLPointerVec(const LObject& list) {
        // ベクトルを作る。 一気にアロケーションする。
        LPointerVec ret(CountList(list));

        // 格納していく。
        LPointerVec::iterator itr = ret.begin();
        for (const LObject* ptr = &list; ptr->IsPair();
        ptr = ptr->cdr().get(), ++itr) {
          *itr = ptr->car();
        }

        return ret;
      }
      /**
       * LPointerVecからListを作成する。
       * @param pointer_vec 変換するLPointerVec。
       * @return 変換後のListのポインタ。
       */
      static LPointer LPointerVecToList(const LPointerVec& pointer_vec) {
        LPointer ret = NewList(pointer_vec.size());
        LObject* ptr = ret.get();
        for (auto& pointer : pointer_vec) {
          ptr->car(pointer);
          Next(&ptr);
        }

        return ret;
      }

      /**
       * タイプをチェックする。 ダメなら例外を発生。
       * @param obj チェックするオブジェクト。
       * @param required_type 要求するタイプ。
       * @return 実際のタイプ。
       */
      static void CheckType(const LObject& obj, LType required_type) {
        static const std::string not_type_symbol[] {
          "@not-nil", "@not-pair", "@not-symbol", "@not-number",
          "@not-boolean", "@not-string", "@not-procedure", "@not-procedure"
        };

        LType ret = obj.type();
        if (ret != required_type) {
          throw GenError(not_type_symbol[static_cast<int>(required_type)],
          "'" + obj.ToString() + "' is not " + TypeToName(required_type)
          + ". It's " + TypeToName(ret) + ".");
        }
      }

      /**
       * リストかどうかをチェックする。 ダメなら例外を発生。
       * @param obj チェックするオブジェクト。
       * @return リストならtrue。
       */
      static void CheckList(const LObject& obj) {
        if (obj.IsList()) return;

        throw GenError("@not-list","'" + obj.ToString()
        + "' is not List. It's " + TypeToName(obj.type()) + ".");
      }

      /**
       * タイプエラーを作成する。
       * @param obj チェックしたオブジェクト。
       * @param required_type_name 要求したタイプの名前。
       * @return タイプエラー。
       */
      static LPointer GenTypeError(const LObject& obj,
      const std::string& required_type_name) {
        return GenError("@type-error","'" + obj.ToString()
        + "' is not " + required_type_name + ". It's "
        + TypeToName(obj.type()) + ".");
      }

      /**
       * リストの長さをチェック。 ダメなら例外を発生。
       * @param list_ptr チェックしたいリストのポインタ。
       * @param required_length 要求しているリストの長さ。
       * @return 実際のリストの長さ。
       */
      static int CheckLength(const LObject& list, int required_length) {
        int ret = CountList(list);
        if (ret < required_length) {
          throw GenError("@insufficient-elements", "'" + list.ToString()
          + "' doesn't have enough elements. Required "
          + std::to_string(required_length) + ". Not "
          + std::to_string(ret) + ".");
        }
        return ret;
      }

      /**
       * エラーをプリントする。
       * @param error プリントするエラー。
       */
      static void PrintError(const LPointer& error) {
        std::cerr << "Error";
        if (error->IsPair()) {
          if (error->car()->IsSymbol()) {
            std::cerr << ": " << error->car()->ToString();
            if (error->cdr()->IsPair()) {
              std::cerr << ": " << error->cdr()->car()->string();
            }
          }
        }
        std::cerr << std::endl;
      }

      /**
       * ネイティブ関数の冒頭で準備する。
       * @param args 引数リスト。 (関数名を含む)
       * @param required_args 要求される引数の数。
       * @param args_ptr_ptr 引数リストへのポインタのポインタ。
       */
      static void GetReadyForFunction(const LObject& args,
      int required_args, LObject** args_ptr_ptr) {
        // 引数の数をカウント。
        int num_args = CountList(args) - 1;

        // 第一引数のポインタを得る。
        if (num_args >= 1) *args_ptr_ptr = args.cdr().get();

        // 引数の数をチェック。
        if (num_args < required_args) {
          throw GenError("@insufficient-arguments", "'"
          + args.car()->ToString() + "' requires "
          + std::to_string(required_args) + " arguments and more. Not "
          + std::to_string(num_args) + ".");
        }
      }

      /**
       * リストをジップする。
       * @param list_vec リストのリスト。 (実行後、空になる副作用あり。)
       * @return ジップ後のリスト。
       */
      static LPointer ZipLists(LPointerVec& list_vec);

      /**
       * クオートで要素をくるむ。
       * @param ptr くるむオブジェクトのポインタ。
       * @return クオートでくるまれたのポインタ。
       */
      static LPointer WrapQuote(const LPointer& ptr) {
        return NewPair(NewSymbol("quote"), NewPair(ptr, NewNil()));
      }
      /**
       * リストの各要素をクオートでくるむ。
       * @param ptr くるむオブジェクトのリストのポインタ。
       * @return 各要素をクオートでくるまれたのリストのポインタ。
       */
      static LPointer WrapListQuote(const LPointer& list_ptr) {
        LPointer ret_ptr = NewList(CountList(*list_ptr));
        LObject* ret_ptr_ptr = ret_ptr.get();

        for (LObject* ptr = list_ptr.get(); ptr->IsPair();
        Next(&ptr), Next(&ret_ptr_ptr)) {
          ret_ptr_ptr->car(WrapQuote(ptr->car()));
        }

        return ret_ptr;
      }

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 字句解析する。 token_queue_に格納される。
       * @param code 解析するコード。
       */
      void Tokenize(const std::string& code) {
        parser_.Tokenize(code);
      }
      /**
       * 字句解析したトークンキューをパースする。
       * @return パース結果のベクトル。
       */
      LPointerVec Parse() {
        return parser_.Parse();
      }

      /**
       * パーサをリセットする。
       */
      void ResetParser() {parser_.Reset();}

      /**
       * 関数オブジェクト。 (LC_Function)
       */
      DEF_LC_FUNCTION(operator()) {
        return NewNil();
      }

      /**
       * 関数を登録する。
       * @param symbol 関数のシンボル。
       * @param c_function 登録すす関数。
       * @param func_id 関数の識別文字列。
       */
      void AddFunction(const std::string& symbol,
      const LC_Function& c_function, const std::string& func_id) {
        scope_chain_.InsertSymbol(symbol,
        NewN_Function(c_function, func_id, scope_chain_));
      }

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - カッコの整合性。
       * @return カッコの整合性。
       */
      int parenth_counter() const {return parser_.parenth_counter();}
      /**
       * アクセサ - 文字列中かどうか。
       * @return 文字列中かどうか。
       */
      bool in_string() const {return parser_.in_string();}
      /**
       * アクセサ - 解析中のトークンのキュー。
       * @return 解析中のトークンのキュー。
       */
      const std::queue<std::string> token_quene() const {
        return parser_.token_quene();
      }

      // ============== //
      // ネイティブ関数 //
      // ============== //
      // %%% eval
      /** ネイティブ関数 - eval */
      DEF_LC_FUNCTION(Eval) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        return caller->Evaluate(caller->Evaluate(args_ptr->car()));
      }

      /** ネイティブ関数 - parse */
      DEF_LC_FUNCTION(ParseFunc);

      // %%% parval
      /** ネイティブ関数 - parval */
      DEF_LC_FUNCTION(Parval) {
        LPointer result = ParseFunc(self, caller, args);
        return caller->Evaluate(result);
      }

      // %%% to-string
      /** ネイティブ関数 - to-string */
      DEF_LC_FUNCTION(ToStringFunc) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        return NewString(caller->Evaluate(args_ptr->car())->ToString());
      }

      /** ネイティブ関数 - try */
      DEF_LC_FUNCTION(Try) ;

      // %%% throw
      /** ネイティブ関数 - throw */
      DEF_LC_FUNCTION(Throw) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        throw caller->Evaluate(args_ptr->cdr());
      }

      // %%% car
      /** ネイティブ関数 - car */
      DEF_LC_FUNCTION(CarFunc) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // ペアを得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::PAIR);

        // Carを返す。
        return result->car();
      }

      // %%% cdr
      /** ネイティブ関数 - cdr */
      DEF_LC_FUNCTION(CdrFunc) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // ペアを得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::PAIR);

        // Cdrを返す。
        return result->cdr();
      }

      // %%% c*r
      /** ネイティブ関数 - c*r */
      LPointer CxrFunc(const std::string& path, LPointer self, LObject* caller,
      const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 第1引数。 ターゲットのリスト。
        LPointer target_ptr = caller->Evaluate(args_ptr->car());
        Next(&args_ptr);

        // パスに合わせて探索する。
        std::string::const_reverse_iterator itr = path.crbegin();
        std::string::const_reverse_iterator end_itr = path.crend();
        for (; itr != end_itr; ++itr) {
          CheckType(*target_ptr, LType::PAIR);

          if (*itr == 'a') {
            target_ptr = target_ptr->car();
          } else {
            target_ptr = target_ptr->cdr();
          }
        }

        return target_ptr;
      }

      // %%% cons
      /** ネイティブ関数 - cons */
      DEF_LC_FUNCTION(Cons) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        return NewPair(caller->Evaluate(args_ptr->car()),
        caller->Evaluate(args_ptr->cdr()->car()));
      }

      /** ネイティブ関数 - apply */
      DEF_LC_FUNCTION(ApplyFunc);

      /** ネイティブ関数 - walk */
      DEF_LC_FUNCTION(WalkFunc);

      // %%% quote
      /** ネイティブ関数 - quote */
      DEF_LC_FUNCTION(Quote) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        return args_ptr->car()->Clone();
      }

      /** ネイティブ関数 - backquote */
      DEF_LC_FUNCTION(Backquote);

      /**
       * !関数用シンボル上書き関数。
       * @param chain 対象のチェーン。
       * @param symbol シンボル。
       * @param ptr 上書きするポインタ。
       * @return 前の値。
       */
      const LPointer& SetCore(LScopeChain& chain, const std::string& symbol,
      const LPointer& ptr) {
        // 前の値を得る。
        const LPointer& prev = chain.SelectSymbol(symbol);
        if (!prev) {
          throw GenError("@unbound",
          "'" + symbol + "' doesn't bind any value.");
        }

        // スコープにバインド。
        chain.UpdateSymbol(symbol, ptr);

        return prev;
      }

      // %%% set!
      /** ネイティブ関数 - set! */
      DEF_LC_FUNCTION(Set) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // シンボルを得る。
        const LPointer& symbol_ptr = args_ptr->car();
        CheckType(*symbol_ptr, LType::SYMBOL);

        // スコープチェーンを得る。
        LScopeChain chain = caller->scope_chain();

        return SetCore(chain, symbol_ptr->symbol(),
        caller->Evaluate(args_ptr->cdr()->car()));
      }

      /** ネイティブ関数 - define */
      DEF_LC_FUNCTION(Define);

      /** ネイティブ関数 - define-macro */
      DEF_LC_FUNCTION(DefineMacro);

      /** ネイティブ関数 - lambda */
      DEF_LC_FUNCTION(Lambda);

      /** ネイティブ関数 - func->lambda */
      DEF_LC_FUNCTION(FuncToLambda);

      /** ネイティブ関数 - let */
      DEF_LC_FUNCTION(Let);

      /** ネイティブ関数 - while */
      DEF_LC_FUNCTION(While);

      /** ネイティブ関数 - for */
      DEF_LC_FUNCTION(For);

      // %%% if
      /** ネイティブ関数 - if */
      DEF_LC_FUNCTION(If) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 3, &args_ptr);

        // 第1引数、条件を評価。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::BOOLEAN);

        // #tなら第2引数を評価して返す。
        if (result->boolean()) {
          return caller->Evaluate(args_ptr->cdr()->car());
        }

        // #tではなかったので第3引数を評価して返す。
        return caller->Evaluate(args_ptr->cdr()->cdr()->car());
      }

      /** ネイティブ関数 - cond */
      DEF_LC_FUNCTION(Cond);

      // %%% begin
      /** ネイティブ関数 - begin */
      DEF_LC_FUNCTION(Begin) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 順次実行。
        LPointer ret_ptr = NewNil();
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          ret_ptr = caller->Evaluate(args_ptr->car());
        }

        return ret_ptr;
      }

      /** ネイティブ関数 - gen-scope */
      DEF_LC_FUNCTION(GenScope);

      /** ネイティブ関数 - display */
      DEF_LC_FUNCTION(Display);

      /** ネイティブ関数 - stdin */
      DEF_LC_FUNCTION(Stdin);

      /** ネイティブ関数 - stdout */
      DEF_LC_FUNCTION(Stdout);

      /** ネイティブ関数 - stderr */
      DEF_LC_FUNCTION(Stderr);

      /** ネイティブ関数 - import */
      DEF_LC_FUNCTION(Import);

      /** ネイティブ関数 - export */
      DEF_LC_FUNCTION(Export);

      // %%% equal?
      /** ネイティブ関数 - equal? */
      DEF_LC_FUNCTION(EqualQ) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer first = caller->Evaluate(args_ptr->car());

        // 比較していく。
        LPointer result;
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(args_ptr->car());
          if (*first != *result) return NewBoolean(false);
        }

        return NewBoolean(true);
      }

      // %%% nil?
      // %%% pair?
      // %%% symbol?
      // %%% number?
      // %%% boolean?
      // %%% string?
      // %%% function?
      // %%% native-function?
      template<LType LTYPE>
      DEF_LC_FUNCTION(QFunc) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 調べていく。
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          if (caller->Evaluate(args_ptr->car())->type() != LTYPE) {
            return NewBoolean(false);
          }
        }

        return NewBoolean(true);
      }

      // %%% procedure?
      DEF_LC_FUNCTION(ProcedureQ) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 調べていく。
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(args_ptr->car());
          if (!(result->IsFunction() || result->IsN_Function())) {
            return NewBoolean(false);
          }
        }

        return NewBoolean(true);
      }

      /** ネイティブ関数 - output-stream */
      DEF_LC_FUNCTION(OutputStream);

      /** ネイティブ関数 - input-stream */
      DEF_LC_FUNCTION(InputStream);

      // %%% system
      /** ネイティブ関数 - system */
      DEF_LC_FUNCTION(System) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::STRING);

        return NewNumber(std::system(result->string().c_str()));
      }

      // %%% get-env
      /** ネイティブ関数 - get-env */
      DEF_LC_FUNCTION(GetEnv) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::STRING);

        const char* env_str = std::getenv(result->string().c_str());
        if (env_str) {
          return NewString(env_str);
        }
        return NewNil();
      }

      /** ネイティブ関数 - gen-thread */
      DEF_LC_FUNCTION(GenThread);

      /** ネイティブ関数 - sleep */
      DEF_LC_FUNCTION(Sleep);

      /** ネイティブ関数 - gen-mutex */
      DEF_LC_FUNCTION(GenMutex);

      /** ネイティブ関数 - append */
      DEF_LC_FUNCTION(Append);

      /** ネイティブ関数 - reverse */
      DEF_LC_FUNCTION(Reverse);

      /** ネイティブ関数 - ref */
      DEF_LC_FUNCTION(Ref);

      // %%% list
      /** ネイティブ関数 - list */
      DEF_LC_FUNCTION(List) {
        // 準備。
        LObject* args_ptr = args.cdr().get();
        LPointer ret_ptr = NewList(CountList(*args_ptr));

        for (LObject* ptr = ret_ptr.get(); args_ptr->IsPair();
        Next(&args_ptr), Next(&ptr)) {
          ptr->car(caller->Evaluate(args_ptr->car()));
        }

        return ret_ptr;
      }

      /** ネイティブ関数 - list-replace */
      DEF_LC_FUNCTION(ListReplace);

      /** ネイティブ関数 - list-remove */
      DEF_LC_FUNCTION(ListRemove);

      /** ネイティブ関数 - list-insert */
      DEF_LC_FUNCTION(ListInsert);

      /** ネイティブ関数 - list-search */
      DEF_LC_FUNCTION(ListSearch);

      /** ネイティブ関数 - list-path */
      DEF_LC_FUNCTION(ListPath);

      /** ネイティブ関数 - list-path-replace */
      DEF_LC_FUNCTION(ListPathReplace);

      /** ネイティブ関数 - list-sort */
      DEF_LC_FUNCTION(ListSort);

      /** ネイティブ関数 - zip */
      DEF_LC_FUNCTION(Zip);

      /** ネイティブ関数 - map */
      DEF_LC_FUNCTION(Map);

      /** ネイティブ関数 - filter */
      DEF_LC_FUNCTION(Filter);

      // %%% range
      /** ネイティブ関数 - range */
      DEF_LC_FUNCTION(Range) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 個数を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        int num = result->number();
        if (num <= 0) {
          throw GenError("@function-error", "Number must be 1 and more.");
        }

        // リストを作る。
        LPointer ret_ptr = NewList(num);
        int i = 0;
        for (LObject* ptr = ret_ptr.get(); ptr->IsPair(); Next(&ptr)) {
          ptr->car(NewNumber(i));
          ++i;
        }

        return ret_ptr;
      }

      /** ネイティブ関数 - start-size-inc */
      DEF_LC_FUNCTION(StartSizeInc);

      // %%% length
      /** ネイティブ関数 - length */
      DEF_LC_FUNCTION(LengthFunc) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());

        // ペアなら連結しているペアの数。
        if (result->IsPair()) {
          int len = 0;
          for (LObject* ptr = result.get(); ptr->IsPair(); Next(&ptr)) {
            ++len;
          }
          return NewNumber(len);
        }

        // Nilは0。
        if (result->IsNil()) return NewNumber(0);

        // 文字列は文字の数。
        if (result->IsString()) return NewNumber(result->string().size());

        // それ以外は1。
        return NewNumber(1);
      }

      // %%% =
      /** ネイティブ関数 - = */
      DEF_LC_FUNCTION(NumEqual) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 比較。
          if (result->number() != value) return NewBoolean(false);
        }

        return NewBoolean(true);
      }

      // %%% ~=
      /** ネイティブ関数 - ~= */
      DEF_LC_FUNCTION(NumNotEqual) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 比較。
          if (result->number() != value) return NewBoolean(true);
        }

        return NewBoolean(false);
      }

      // %%% >
      /** ネイティブ関数 - > */
      DEF_LC_FUNCTION(NumGT) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 比較。
          double result_value = result->number();
          if (value <= result_value) {
            return NewBoolean(false);
          }

          // 数値を更新。
          value = result_value;
        }

        return NewBoolean(true);
      }

      // %%% >=
      /** ネイティブ関数 - >= */
      DEF_LC_FUNCTION(NumGE) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 比較。
          double result_value = result->number();
          if (value < result_value) {
            return NewBoolean(false);
          }

          // 数値を更新。
          value = result_value;
        }

        return NewBoolean(true);
      }

      // %%% <
      /** ネイティブ関数 - < */
      DEF_LC_FUNCTION(NumLT) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 比較。
          double result_value = result->number();
          if (value >= result_value) {
            return NewBoolean(false);
          }

          // 数値を更新。
          value = result_value;
        }

        return NewBoolean(true);
      }

      // %%% <=
      /** ネイティブ関数 - <= */
      DEF_LC_FUNCTION(NumLE) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 比較。
          double result_value = result->number();
          if (value > result_value) {
            return NewBoolean(false);
          }

          // 数値を更新。
          value = result_value;
        }

        return NewBoolean(true);
      }

      // %%% even?
      /** ネイティブ関数 - even? */
      DEF_LC_FUNCTION(EvenQ) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        int num = result->number();

        return NewBoolean(!(num % 2));
      }

      // %%% odd?
      /** ネイティブ関数 - odd? */
      DEF_LC_FUNCTION(OddQ) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        int num = result->number();

        return NewBoolean(num % 2);
      }

      // %%% not 
      /** ネイティブ関数 - not */
      DEF_LC_FUNCTION(Not) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::BOOLEAN);

        return NewBoolean(!(result->boolean()));
      }

      // %%% and 
      /** ネイティブ関数 - and */
      DEF_LC_FUNCTION(And) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 1つでもfalseがあればfalse。
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(args_ptr->car());
          CheckType(*result, LType::BOOLEAN);

          if (!(result->boolean())) return NewBoolean(false);
        }

        return NewBoolean(true);
      }

      // %%% or 
      /** ネイティブ関数 - or */
      DEF_LC_FUNCTION(Or) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 1つでもtrueがあればtrue。
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(args_ptr->car());
          CheckType(*result, LType::BOOLEAN);

          if (result->boolean()) return NewBoolean(true);
        }

        return NewBoolean(false);
      }

      // %%% +
      /** ネイティブ関数 - + */
      DEF_LC_FUNCTION(Addition) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 全部足す。
        double value = 0.0;
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 足す。
          value += result->number();
        }

        return NewNumber(value);
      }

      /** ネイティブ関数 - add! */
      DEF_LC_FUNCTION(AdditionEx);

      // %%% -
      /** ネイティブ関数 - - */
      DEF_LC_FUNCTION(Subtraction) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 引き算の元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 引いていく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 引く。
          value -= result->number();
        }

        return NewNumber(value);
      }

      /** ネイティブ関数 - sub! */
      DEF_LC_FUNCTION(SubtractionEx);

      // %%% *
      /** ネイティブ関数 - * */
      DEF_LC_FUNCTION(Multiplication) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        double value = 1.0;

        // 掛けていく。
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 掛ける。
          value *= result->number();
        }

        return NewNumber(value);
      }

      /** ネイティブ関数 - mul! */
      DEF_LC_FUNCTION(MultiplicationEx);

      // %%% /
      /** ネイティブ関数 - / */
      DEF_LC_FUNCTION(Division) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 割り算の元になる値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 割っていく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(args_ptr->car());

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 割る。
          value /= result->number();
        }

        return NewNumber(value);
      }

      /** ネイティブ関数 - div! */
      DEF_LC_FUNCTION(DivisionEx);

      // %%% ++
      /** ネイティブ関数 - ++ */
      DEF_LC_FUNCTION(Inc) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(result->number() + 1.0);
      }

      /** ネイティブ関数 - inc! */
      DEF_LC_FUNCTION(IncEx);

      // %%% --
      /** ネイティブ関数 - -- */
      DEF_LC_FUNCTION(Dec) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(result->number() - 1.0);
      }

      /** ネイティブ関数 - dec! */
      DEF_LC_FUNCTION(DecEx);

      /** ネイティブ関数 - string-split */
      DEF_LC_FUNCTION(StringSplit); 

      /** ネイティブ関数 - string-join */
      DEF_LC_FUNCTION(StringJoin); 

      // %%% front
      /** ネイティブ関数 - front */
      DEF_LC_FUNCTION(Front) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckList(*result);

        if (result->IsNil()) return result;

        return result->car();
      }

      // %%% back
      /** ネイティブ関数 - back */
      DEF_LC_FUNCTION(Back) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 値を得る。
        LPointer result = caller->Evaluate(args_ptr->car());
        CheckList(*result);

        LPointer next;
        for (; result->IsPair(); result = next) {
          next = result->cdr();
          if (next->IsNil()) return result->car();
        }

        return NewNil();
      }

      // %%% push-front
      /** ネイティブ関数 - push-front */
      DEF_LC_FUNCTION(PushFront) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 第1引数。 リスト。
        LPointer target_ptr = caller->Evaluate(args_ptr->car());
        CheckList(*target_ptr);

        // プッシュする。
        return NewPair(caller->Evaluate(args_ptr->cdr()->car()),
        target_ptr);
      }

      /** ネイティブ関数 - push-front! */
      DEF_LC_FUNCTION(PushFrontEx);

      // %%% pop-front
      /** ネイティブ関数 - pop-front */
      DEF_LC_FUNCTION(PopFront) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 第1引数。 リスト。
        LPointer target_ptr = caller->Evaluate(args_ptr->car());
        CheckList(*target_ptr);

        if (target_ptr->IsNil()) return NewNil();
        return target_ptr->cdr();
      }

      /** ネイティブ関数 - pop-front! */
      DEF_LC_FUNCTION(PopFrontEx);

      // %%% push-back
      /** ネイティブ関数 - push-back */
      DEF_LC_FUNCTION(PushBack) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 第1引数。 リスト。
        LPointer target_ptr = caller->Evaluate(args_ptr->car());
        CheckList(*target_ptr);

        if (target_ptr->IsNil()) {
          return NewPair(caller->Evaluate(args_ptr->cdr()->car()),
          target_ptr);
        }
        
        LPointer next;
        for (LObject* ptr = target_ptr.get(); ptr->IsPair(); Next(&ptr)) {
          if (ptr->cdr()->IsNil()) {
            ptr->cdr(NewPair(caller->Evaluate(args_ptr->cdr()->car()),
            NewNil()));
            return target_ptr;
          }
        }

        return NewNil();
      }

      /** ネイティブ関数 - push-back! */
      DEF_LC_FUNCTION(PushBackEx);

      // %%% pop-back
      /** ネイティブ関数 - pop-back */
      DEF_LC_FUNCTION(PopBack) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 第1引数。 リスト。
        LPointer target_ptr = caller->Evaluate(args_ptr->car());
        CheckList(*target_ptr);

        // target_ptrの要素が1つもない。
        if (target_ptr->IsNil()) return NewNil();

        // 要素が1つ。
        LObject* prev = target_ptr.get();
        LObject* current = target_ptr->cdr().get();
        if (current->IsNil()) return NewNil();
        
        // 要素が2つ以上。
        LPointer next;
        for (; current->IsPair(); Next(&prev), Next(&current)) {
          if (current->cdr()->IsNil()) {
            prev->cdr(NewNil());
            return target_ptr;
          }
        }

        return NewNil();
      }

      /** ネイティブ関数 - pop-back! */
      DEF_LC_FUNCTION(PopBackEx);

      // %%% sin
      /** ネイティブ関数 - sin */
      DEF_LC_FUNCTION(Sin) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::sin(result->number()));
      }

      // %%% cos
      /** ネイティブ関数 - cos */
      DEF_LC_FUNCTION(Cos) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::cos(result->number()));
      }

      // %%% tan
      /** ネイティブ関数 - tan */
      DEF_LC_FUNCTION(Tan) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::tan(result->number()));
      }

      // %%% asin
      /** ネイティブ関数 - asin */
      DEF_LC_FUNCTION(ASin) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::asin(result->number()));
      }

      // %%% acos
      /** ネイティブ関数 - acos */
      DEF_LC_FUNCTION(ACos) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::acos(result->number()));
      }

      // %%% atan
      /** ネイティブ関数 - atan */
      DEF_LC_FUNCTION(ATan) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::atan(result->number()));
      }

      // %%% sqrt
      /** ネイティブ関数 - sqrt */
      DEF_LC_FUNCTION(Sqrt) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::sqrt(result->number()));
      }

      // %%% abs
      /** ネイティブ関数 - abs */
      DEF_LC_FUNCTION(Abs) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::fabs(result->number()));
      }

      // %%% ceil
      /** ネイティブ関数 - ceil */
      DEF_LC_FUNCTION(Ceil) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::ceil(result->number()));
      }

      // %%% floor
      /** ネイティブ関数 - floor */
      DEF_LC_FUNCTION(Floor) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::floor(result->number()));
      }

      // %%% round
      /** ネイティブ関数 - round */
      DEF_LC_FUNCTION(Round) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::round(result->number()));
      }

      // %%% trunc
      /** ネイティブ関数 - trunc */
      DEF_LC_FUNCTION(Trunc) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::trunc(result->number()));
      }

      // %%% exp
      /** ネイティブ関数 - exp */
      DEF_LC_FUNCTION(Exp) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::exp(result->number()));
      }

      // %%% exp2
      /** ネイティブ関数 - exp2 */
      DEF_LC_FUNCTION(Exp2) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::exp2(result->number()));
      }

      // %%% expt
      /** ネイティブ関数 - expt */
      DEF_LC_FUNCTION(Expt) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer first_ptr = caller->Evaluate(args_ptr->car());
        CheckType(*first_ptr, LType::NUMBER);

        LPointer second_ptr = caller->Evaluate(args_ptr->cdr()->car());
        CheckType(*second_ptr, LType::NUMBER);

        return NewNumber(std::pow(first_ptr->number(), second_ptr->number()));
      }

      // %%% log
      /** ネイティブ関数 - log */
      DEF_LC_FUNCTION(Log) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::log(result->number()));
      }

      // %%% log2
      /** ネイティブ関数 - log2 */
      DEF_LC_FUNCTION(Log2) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::log2(result->number()));
      }

      // %%% log10
      /** ネイティブ関数 - log10 */
      DEF_LC_FUNCTION(Log10) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::log10(result->number()));
      }

      // %%% random
      /** ネイティブ関数 - random */
      LPointer Random(std::mt19937& engine, LPointer self,
      LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);

        std::uniform_real_distribution<> dist(0, result->number());
        return NewNumber(dist(engine));
      }

      // %%% max
      /** ネイティブ関数 - max */
      DEF_LC_FUNCTION(Max) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double max = result->number();
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(args_ptr->car());
          CheckType(*result, LType::NUMBER);

          double current = result->number();
          if (current > max) max = current;
        }

        return NewNumber(max);
      }

      // %%% min
      /** ネイティブ関数 - min */
      DEF_LC_FUNCTION(Min) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(args_ptr->car());
        CheckType(*result, LType::NUMBER);
        double min = result->number();
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(args_ptr->car());
          CheckType(*result, LType::NUMBER);

          double current = result->number();
          if (current < min) min = current;
        }

        return NewNumber(min);
      }

      /** ネイティブ関数 - regex-search */
      DEF_LC_FUNCTION(RegexSearch);

      /** ネイティブ関数 - gen-nabla */
      DEF_LC_FUNCTION(GenNabla);

      /** ネイティブ関数 - integral */
      DEF_LC_FUNCTION(Integral);

      /** ネイティブ関数 - power-method */
      DEF_LC_FUNCTION(PowerMethod);

      /** ネイティブ関数 - inverse-matrix */
      DEF_LC_FUNCTION(InverseMatrix);

      /** ネイティブ関数 - transposed-matrix */
      DEF_LC_FUNCTION(TransposedMatrix);

      /** ネイティブ関数 - determinant */
      DEF_LC_FUNCTION(Determinant);

      /** ネイティブ関数 - bayes */
      DEF_LC_FUNCTION(Bayes);

      /** ネイティブ関数 - logit->prob */
      DEF_LC_FUNCTION(LogitToProb);

      /** ネイティブ関数 - prob->logit */
      DEF_LC_FUNCTION(ProbToLogit);

      /** ネイティブ関数 - gen-pa2 */
      DEF_LC_FUNCTION(GenPA2);

      /** ネイティブ関数 - gen-ai */
      DEF_LC_FUNCTION(GenAI);

      /** ネイティブ関数 - rbf-kernel */
      DEF_LC_FUNCTION(RBFKernel);

      /** ネイティブ関数 - now */
      DEF_LC_FUNCTION(Now);

      /** ネイティブ関数 - clock */
      DEF_LC_FUNCTION(Clock) {
        return NewNumber(static_cast<double>(std::clock())
        / static_cast<double>(CLOCKS_PER_SEC));
      }

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
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
      LParser parser_;
  };

  namespace LMath {
    /** ベクトル。*/
    using Vec = std::vector<double>;
    /** 行列。 (横ベクトルのベクトル。) */
    using Mat = std::vector<std::vector<double>>;

    /**
     * リストをベクトルにする。
     * @param list リスト。
     * @return ベクトル。
     */
    inline Vec ListToMathVec(const LObject& list) {
      unsigned int size = Lisp::CountList(list);

      LMath::Vec ret(size);
      LMath::Vec::iterator itr = ret.begin();

      for (const LObject* ptr = &list; ptr->IsPair();
      ptr = ptr->cdr().get(), ++itr) {
        const LPointer& car = ptr->car();
        Lisp::CheckType(*car, LType::NUMBER);
        *itr = car->number();
      }

      return ret;
    }

    /**
     * ベクトルをリストにする。
     * @param vec ベクトル。
     * @return リストのポインタ。
     */
    inline LPointer MathVecToList(const Vec& vec) {
      unsigned int size = vec.size();
      LPointerVec ret_vec(size);
      for (unsigned int i = 0; i < size; ++i) {
        ret_vec[i] = Lisp::NewNumber(vec[i]);
      }
      return Lisp::LPointerVecToList(ret_vec);
    }

    /**
     * 行列を作る。
     * @param num_rows 行数。
     * @param num_cols 列数。
     * @return 行列。
     */
    inline Mat GenMatrix(unsigned int num_rows, unsigned int num_cols) {
      return Mat(num_rows, Vec(num_cols, 0.0));
    }

    /**
     * ベクトルの足し算。
     * @param vec_1 ベクトル1
     * @param vec_2 ベクトル2
     * @return 結果。
     */
    inline Vec operator+(const Vec& vec_1, const Vec& vec_2) {
      unsigned int size = vec_1.size();
      unsigned int size_2 = vec_2.size();
      size = size_2 < size ? size_2 : size;

      Vec ret(size);
      for (unsigned int i = 0; i < size; ++i) ret[i] = vec_1[i] + vec_2[i];

      return ret;
    }

    /**
     * ベクトルの引き算。
     * @param vec_1 ベクトル1
     * @param vec_2 ベクトル2
     * @return 結果。
     */
    inline Vec operator-(const Vec& vec_1, const Vec& vec_2) {
      unsigned int size = vec_1.size();
      unsigned int size_2 = vec_2.size();
      size = size_2 < size ? size_2 : size;

      Vec ret(size);
      for (unsigned int i = 0; i < size; ++i) ret[i] = vec_1[i] - vec_2[i];

      return ret;
    }

    /**
     * 行列の足し算。
     * @param mat_1 行列1
     * @param mat_2 行列2
     * @return 結果。
     */
    inline Mat operator+(const Mat& mat_1, const Mat& mat_2) {
      unsigned int size = mat_1.size();
      unsigned int size_2 = mat_2.size();
      size = size_2 < size ? size_2 : size;

      Mat ret(size);
      for (unsigned int i = 0; i < size; ++i) ret[i] = mat_1[i] + mat_2[i];

      return ret;
    }

    /**
     * 行列の引き算。
     * @param mat_1 行列1
     * @param mat_2 行列2
     * @return 結果。
     */
    inline Mat operator-(const Mat& mat_1, const Mat& mat_2) {
      unsigned int size = mat_1.size();
      unsigned int size_2 = mat_2.size();
      size = size_2 < size ? size_2 : size;

      Mat ret(size);
      for (unsigned int i = 0; i < size; ++i) ret[i] = mat_1[i] - mat_2[i];

      return ret;
    }

    /**
     * 内積。
     * @param vec_1 ベクトル1
     * @param vec_2 ベクトル2
     * @return 内積。
     */
    inline double operator*(const Vec& vec_1, const Vec& vec_2) {
      unsigned int size = vec_1.size();
      unsigned int size_2 = vec_2.size();
      size = size_2 < size ? size_2 : size;

      double ret = 0.0;
      for (unsigned int i = 0; i < size; ++i) ret += vec_1[i] * vec_2[i];

      return ret;
    }

    /**
     * スカラー倍。
     * @param d スカラー値。
     * @param vec ベクトル
     * @return スカラー倍。
     */
    inline Vec operator*(double d, const Vec& vec) {
      unsigned int size = vec.size();
      Vec ret(size);

      for (unsigned int i = 0; i < size; ++i) ret[i] = vec[i] * d;

      return ret;
    }

    /**
     * 行列のスカラー倍。
     * @param d スカラー値。
     * @param mat 行列。
     * @return スカラー倍。
     */
    inline Mat operator*(double d, const Mat& mat) {
      unsigned int size = mat.size();
      Mat ret(size);

      for (unsigned int i = 0; i < size; ++i) {
        ret[i] = d * mat[i];
      }

      return ret;
    }

    /**
     * 線形変換。
     * @param mat 変換用行列。
     * @param vec 変換する行列。
     * @return 変換後のベクトル。
     */
    inline Vec operator*(const Mat& mat, const Vec& vec) {
      unsigned int size = mat.size();
      Vec ret(size);

      for (unsigned int i = 0; i < size; ++i) ret[i] = mat[i] * vec;

      return ret;
    }

    /**
     * 直積。
     * @param vec_1 ベクトル1
     * @param vec_2 ベクトル2
     * @return 直積。
     */
    inline Mat Direct(const Vec& vec_1, const Vec& vec_2) {
      unsigned int size_1 = vec_1.size();
      unsigned int size_2 = vec_2.size();

      Mat ret(size_1, Vec(size_2));

      for (unsigned int i = 0; i < size_1; ++i) {
        for (unsigned int j = 0; j < size_2; ++j) {
          ret[i][j] = vec_1[i] * vec_2[j];
        }
      }

      return ret;
    }

    /**
     * リストを行列にする。
     * @param list 行列にしたいリスト。
     * @return 行列。
     */
    Mat ListToMatrix(const LObject& list);

    /**
     * べき乗法でもっとも大きい固有値、固有ベクトルを計算する。
     * @param mat 固有値を調べたい行列。
     * @return (固有値, 固有ベクトル)のタプル。
     */
    std::tuple<double, Vec> Eigen(const Mat& mat);

    /**
     * 逆行列を求める。
     * @param mat 行列。
     * @return 逆行列。 なかったら空の行列を返す。
     */
    Mat Inverse(const Mat& mat);

    /**
     * 行列式を求める。
     * @param mat 行列。
     * @return 行列式。
     */
    double Determinant(const Mat& mat);
  }  // namespace LMath

  /** Passive-Aggressive-2 */
  class LPA2 {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param weight_vec 初期ウェイト。
       */
      LPA2(const LMath::Vec& weight_vec) : weight_vec_(weight_vec) {}
      /** コンストラクタ。 */
      LPA2() {}
      /**
       * コピーコンストラクタ。
       * @param pa コピー元。
       */
      LPA2(const LPA2& pa) : weight_vec_(pa.weight_vec_) {}
      /**
       * ムーブコンストラクタ。
       * @param pa ムーブ元。
       */
      LPA2(LPA2&& pa) : weight_vec_(std::move(pa.weight_vec_)) {}
      /**
       * コピー代入演算子。
       * @param pa コピー元。
       */
      LPA2& operator=(const LPA2& pa) {
        weight_vec_ = pa.weight_vec_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param pa ムーブ元。
       */
      LPA2& operator=(LPA2&& pa) {
        weight_vec_ = std::move(pa.weight_vec_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LPA2() {}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * ウェイトを学習する。
       * @param is_plus 出力が0以上ならtrue。
       * @param cost エラーに対するコスト。
       * @param 入力ベクトル。
       */
      void TrainWeights(bool is_plus, double cost, const LMath::Vec& input);

      /**
       * 関数を計算する。
       * @param input 入力ベクトル。
       * @return 出力。
       */
      double CalcFunc(const LMath::Vec& input) {
        return LMath::operator*(weight_vec_, input);
      }

      /**
       * アクセサ - ウェイトのベクトル。
       * @return ウェイトのベクトル。
       */
      const LMath::Vec& weight_vec() const {return weight_vec_;}

      // --- Lisp用関数 --- //
      /** 関数オブジェクト。 */
      DEF_LC_FUNCTION(operator());

      /** メッセージ関数 - @train */
      DEF_LC_FUNCTION(Train);
      /** メッセージ関数 - @calc */
      DEF_LC_FUNCTION(Calc);
      /** メッセージ関数 - @get-weights */
      DEF_LC_FUNCTION(GetWeights);

    private:
      /**
       * ヒンジ損失を計算する。
       * @param sign 正の数なら1、違うなら-1。
       * @param input 入力ベクトル。
       * @return ヒンジ損失。
       */
      double Hinge(double sign, const LMath::Vec& input) {
        using namespace LMath;

        double l = 1.0 - ((input * weight_vec_) * sign);
        return l < 0.0 ? 0.0 : l;
      }

      // ========== //
      // メンバ変数 //
      // ========== //
      /** ウェイト。 */
      LMath::Vec weight_vec_;
  };

  // 人工知能。
  class LAI {
    public:
      // ==================== //
      // コンストラクタと代入 //
      // ==================== //
      /**
       * コンストラクタ。
       * @param initial_weights 初期ウェイト。
       * @param inital_bias 初期バイアス。
       */
      LAI(const LMath::Vec& inital_weights, double inital_bias) :
      weights_(inital_weights) {
        weights_.push_back(inital_bias);
      }
      /** コンストラクタ。 */
      LAI() : weights_(1, 0) {}
      /**
       * コピーコンストラクタ。
       * @param ai コピー元。
       */
      LAI(const LAI& ai) : weights_(ai.weights_) {}
      /**
       * ムーブコンストラクタ。
       * @param ai ムーブ元。
       */
      LAI(LAI&& ai) : weights_(std::move(ai.weights_)) {}
      /**
       * コピー代入演算子。
       * @param ai コピー元。
       */
      LAI& operator=(const LAI& ai) {
        weights_ = ai.weights_;
        return *this;
      }
      /**
       * ムーブ代入演算子。
       * @param ai ムーブ元。
       */
      LAI& operator=(LAI&& ai) {
        weights_ = std::move(ai.weights_);
        return *this;
      }
      /** デストラクタ。 */
      virtual ~LAI() {}

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - ウェイトベクトル。
       * @return ウェイトベクトル。
       */
      LMath::Vec weights() const {
        LMath::Vec ret = weights_;
        ret.pop_back();
        return ret;
      }
      /**
       * アクセサ - バイアス。
       * @return バイアス。
       */
      double bias() const {return weights_.back();}

      // ============== //
      // パブリック関数 //
      // ============== //
      /**
       * 訓練出力を数値にする。
       * @param desired_output 訓練出力。
       * @return 数値。
       */
      static constexpr double OutputToNumber(bool desired_output) {
        return desired_output ? 1.0 : -1.0;
      }

      /**
       * ロジットを計算する。
       * @param features 特徴ベクトル。
       * @return 得点。
       */
      double CalLogit(const LMath::Vec& features) const {
        using namespace LMath;

        Vec copy = features;
        copy.push_back(1.0);

        return copy * weights_;
      }

      /**
       * 真偽をジャッジする。
       * @param features 特徴ベクトル。
       * @return 真偽値。
       */
      bool Judge(const LMath::Vec& features) const {
        return CalLogit(features) >= 0.0 ? true : false;
      }

      /**
       * 得点を2倍シグモイドで加工して出力する。
       * @param features 特徴ベクトル。
       * @return 得点。 (-1 < f < 1)
       */
      double CalDoubleSigmoid(const LMath::Vec& features) const {
        return DoubleSigmoid(CalLogit(features));
      }

      /**
       * シグモイドで加工して出力する。
       * @param features 特徴ベクトル。
       * @return 得点。 (0 < f < 1)
       */
      double CalSigmoid(const LMath::Vec& features) const {
        return Sigmoid(CalLogit(features));
      }

      /**
       * 学習する。 (DoubleSigmoid)
       * @param desired_output 訓練出力。
       * @param features 特徴ベクトル。
       * @param rate 学習率。
       * @return 微分された損失。
       */
      double TrainDoubleSigmoid(bool desired_output,
      const LMath::Vec& features, double rate) {
        using namespace LMath;

        double y = OutputToNumber(desired_output);
        Vec copy = features;
        copy.push_back(1.0);

        double loss =
        -y * DSigHingeLoss(y, copy) * DDoubleSigmoid(weights_ * copy);

        weights_ = weights_ - ((rate * loss) * copy);

        return loss;
      }

      /**
       * 学習する。 (PA1)
       * @param desired_output 訓練出力。
       * @param features 特徴ベクトル。
       * @param cost コスト。
       * @return 微分された損失。
       */
      double TrainPA1(bool desired_output, const LMath::Vec& features,
      double cost) {
        using namespace LMath;

        double y = OutputToNumber(desired_output);
        Vec copy = features;
        copy.push_back(1.0);

        double hinge = HingeLoss(y, copy);
        if (hinge <= 0.0) return 0.0;
        double loss = hinge / CalDenominatorPA1(copy);

        loss = loss < cost ? loss : cost;

        weights_ = weights_ + ((y * loss) * copy);

        return -y * hinge;
      }

      /**
       * 学習する。 (PA2)
       * @param desired_output 訓練出力。
       * @param features 特徴ベクトル。
       * @param cost コスト。
       * @return 微分された損失。
       */
      double TrainPA2(bool desired_output, const LMath::Vec& features,
      double cost) {
        using namespace LMath;

        double y = OutputToNumber(desired_output);
        Vec copy = features;
        copy.push_back(1.0);

        double loss = HingeLoss(y, copy);
        if (loss <= 0.0) return 0.0;
        loss *= y;

        weights_ = weights_
        + ((loss / CalDenominatorPA2(copy, cost)) * copy);

        return -loss;
      }

      /**
       * バックプロパゲーションで学習する。
       * @param differentiated_parent_loss 親の微分された損失のベクトル。
       * @param related_weights_to_me 各親の自分用のウェイトのベクトル。
       * @param children_output 各子の出力。
       * @param rate 学習率。
       * @return 微分された損失。
       */
      double TrainBackPropagation
      (const LMath::Vec& differentiated_parent_loss,
      const LMath::Vec& related_weights_to_me,
      const LMath::Vec& children_output, double rate) {
        using namespace LMath;

        double loss = differentiated_parent_loss * related_weights_to_me;

        Vec copy = children_output;
        copy.push_back(1.0);
        loss *= DDoubleSigmoid(weights_ * copy);

        weights_ = weights_ - ((rate * loss) * copy);

        return loss;
      }

    private:
      /** ウェイトベクトル。 最後尾はバイアス。 */
      LMath::Vec weights_;

      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * シグモイド関数。 (0 < f < 1)
       * @param x 入力。
       * @return 出力。
       */
      static double Sigmoid(double x) {
        return 1.0 / (1.0 + std::exp(-x));
      }
      /**
       * 2倍シグモイド関数。 (-1 < f < 1)
       * @param x 入力。
       * @return 出力。
       */
      static double DoubleSigmoid(double x) {
        return (2.0 * Sigmoid(x)) - 1;
      }
      /**
       * 2倍シグモイドの導関数。
       * @param x 入力。
       * @return 出力。
       */
      static double DDoubleSigmoid(double x) {
        double sig = Sigmoid(x);
        return 2.0 * sig * (1.0 - sig);
      }
      /**
       * ヒンジ損失。
       * @param desired_output 訓練出力。
       * @param features 特徴ベクトル。
       * @return ヒンジ損失。
       */
      double HingeLoss(double desired_output, const LMath::Vec& features)
      const {
        using namespace LMath;

        double loss = 1 - (desired_output * (features * weights_));
        return loss > 0.0 ? loss : 0.0;
      }
      /**
       * 2倍シグモイドによるヒンジ損失。
       * @param desired_output 訓練出力。
       * @param features 特徴ベクトル。
       * @return ヒンジ損失。
       */
      double DSigHingeLoss(double desired_output,
      const LMath::Vec& features) const {
        using namespace LMath;

        return 1 - (desired_output * DoubleSigmoid(features * weights_));
      }

      /**
       * 学習係数の分母を計算する。 (PA1)
       * @param features 特徴ベクトル。
       * @param cost コスト。
       */
      double CalDenominatorPA1(const LMath::Vec& features) const {
        using namespace LMath;

        return features * features;
      }

      /**
       * 学習係数の分母を計算する。 (PA2)
       * @param features 特徴ベクトル。
       * @param cost コスト。
       */
      double CalDenominatorPA2(const LMath::Vec& features, double cost) const {
        using namespace LMath;

        return (features * features) + (1 / (2 * cost));
      }
  };
}  // namespace Sayuri

#endif
