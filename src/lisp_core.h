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
      LPointer SelectSymbol(const std::string& symbol) const {
        // 手前のスコープから順番に調べる。
        LScopeChain::const_reverse_iterator citr = crbegin();
        for (; citr != crend(); ++citr) {
          if ((*citr)->find(symbol) != (*citr)->end()) {
            return (*citr)->at(symbol);
          }
        }

        return LPointer(nullptr);
      }
      /**
       * シンボルを追加。
       * @param symbol 追加するシンボル。 一番近くのスコープに追加される。
       * @param ptr シンボルにバインドするオブジェクトのポインタ。
       */
      void InsertSymbol(const std::string& symbol, const LPointer& ptr) {
        back()->emplace(symbol, ptr);
      }
      /**
       * シンボルを更新。
       * @param symbol 更新するシンボル。
       * @param ptr シンボルにバインドするオブジェクトのポインタ。
       */
      void UpdateSymbol(const std::string& symbol, const LPointer& ptr) {
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
      void DeleteSymbol(const std::string& symbol) {
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

  /** 関数の引数名ベクトル。 */
  using LArgNames = std::vector<std::string>;

  /** C言語関数オブジェクト。 <結果(自分自身, 呼び出し元, 引数リスト)> */
  using LC_Function =
  std::function<LPointer(LPointer, LObject*, const LObject&)>;

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
      virtual LPointer Evaluate(const LObject& target) final;

      /**
       * 自分がリストかどうか。
       * @return リストならtrue。
       */
      virtual bool IsList() const {return false;}

      /**
       * アクセサ - Car。
       * @return Car。
       */
      virtual LPointer car() const {return LPointer(nullptr);}
      /**
       * アクセサ - Cdr。
       * @return Cdr。
       */
      virtual LPointer cdr() const {return LPointer(nullptr);}
      /**
       * アクセサ - シンボル。
       * @return シンボル。
       */
      virtual std::string symbol() const {return std::string("");}
      /**
       * アクセサ - 数字。
       * @return 数字。
       */
      virtual double number() const {return 0.0;}
      /**
       * アクセサ - 真偽値。
       * @return 真偽値。
       */
      virtual bool boolean() const {return false;}
      /**
       * アクセサ - 文字列。
       * @return 文字列。
       */
      virtual std::string string() const {return std::string("");}
      /**
       * アクセサ - 関数の引数名ベクトル。
       * @return 関数の引数名ベクトル。
       */
      virtual LArgNames arg_names() const {return LArgNames();}
      /**
       * アクセサ - 関数の式。
       * @return 関数の式。
       */
      virtual LPointerVec expression() const {return LPointerVec();}
      /**
       * アクセサ - ネイティブ関数の実体。
       * @return ネイティブ関数の実体。
       */
      virtual LC_Function c_function() const {return LC_Function();}
      /**
       * アクセサ - ネイティブ関数用識別文字列。
       * @return ネイティブ関数用識別文字列。
       */
      virtual std::string func_id() const {return std::string("");}
      /**
       * アクセサ - スコープチェーン。
       * @return スコープチェーン。
       */
      virtual LScopeChain scope_chain() const {return LScopeChain();}

      /**
       * ミューテータ - Car。
       * @param ptr Car。
       */
      virtual void car(const LPointer& ptr) {}
      /**
       * ミューテータ - Cdr。
       * @param ptr Cdr。
       */
      virtual void cdr(const LPointer& ptr) {}
      /**
       * ミューテータ - シンボル。
       * @param symbol シンボル。
       */
      virtual void symbol(const std::string& symbol) {}
      /**
       * ミューテータ - 数字。
       * @param number 数字。
       */
      virtual void number(double number) {}
      /**
       * ミューテータ - 真偽値。
       * @param boolean 真偽値。
       */
      virtual void boolean(bool boolean) {}
      /**
       * ミューテータ - 文字列。
       * @param string 文字列。
       */
      virtual void string(const std::string& string) {}
      /**
       * ミューテータ - 関数の引数名ベクトル。
       * @param arg_names 関数の引数名ベクトル。
       */
      virtual void arg_names(const LArgNames& arg_name) {}
      /**
       * ミューテータ - 関数の式。
       * @param expr 関数の式。
       */
      virtual void expression(const LPointerVec& expression) {}
      /**
       * ミューテータ - ネイティブ関数の実体。
       * @param c_function ネイティブ関数の実体。
       */
      virtual void c_function(const LC_Function& c_function) {}
      /**
       * アクセサ - ネイティブ関数用識別文字列。
       * @return ネイティブ関数用識別文字列。
       */
      virtual void func_id(const std::string& id) {}
      /**
       * ミューテータ - スコープチェーン。
       * @param scope_chain スコープチェーン。
       */
      virtual void scope_chain(const LScopeChain& scope_chain) {}

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
        return LPointer(new LNil());
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
      LPair() : car_(new LNil()), cdr_(new LNil()) {}
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
        return LPointer(new LPair(car_->Clone(), cdr_->Clone()));
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
      virtual LPointer car() const override {return car_;}
      /**
       * アクセサ - Cdr。
       * @return Cdr。
       */
      virtual LPointer cdr() const override {return cdr_;}

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
        return LPointer(new LSymbol(*this));
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
      virtual std::string symbol() const override {
        return symbol_;
      }
      /**
       * ミューテータ - シンボル。
       * @param symbol シンボル。
       */
      virtual void symbol(const std::string& symbol) override {
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
        return LPointer(new LNumber(*this));
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
        return LPointer(new LBoolean(*this));
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
        return LPointer(new LString(*this));
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
      virtual std::string string() const override {
        return string_;
      }

      /**
       * ミューテータ - 文字列。
       * @param string 文字列。
       */
      virtual void string(const std::string& string) override {
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
        return LPointer(new LFunction(*this));
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
        oss << "Function: (lambda (";
        for (auto& name : arg_names_) {
          oss << name << " ";
        }
        oss.seekp(oss.str().size() - 1);
        oss << ") ";
        for (auto& expr : expression_) {
          oss << expr->ToString() << " ";
        }
        oss.seekp(oss.str().size() - 1);
        oss << ")";
        return oss.str();
      }

      /**
       * アクセサ - 関数の引数名ベクトル。
       * @return 関数の引数名ベクトル。
       */
      virtual LArgNames arg_names() const override {
        return arg_names_;
      }
      /**
       * アクセサ - 関数の式。
       * @return 関数の式。
       */
      virtual LPointerVec expression() const override {
        return expression_;
      }
      /**
       * アクセサ - スコープチェーン。
       * @return スコープチェーン。
       */
      virtual LScopeChain scope_chain() const override {
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
        return LPointer(new LN_Function(*this));
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
        //oss << "Native Function: " << c_function_.target<LC_Function>();
        oss << "Native Function";
        return oss.str();
      }

      /**
       * アクセサ - ネイティブ関数の実体。
       * @return ネイティブ関数の実体。
       */
      virtual LC_Function c_function() const override {
        return c_function_;
      }
      /**
       * アクセサ - ネイティブ関数用識別文字列。
       * @return ネイティブ関数用識別文字列。
       */
      virtual std::string func_id() const override {
        return func_id_;
      }
      /**
       * アクセサ - スコープチェーン。
       * @return スコープチェーン。
       */
      virtual LScopeChain scope_chain() const override {
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

  /** ヘルプ辞書。 */
  using LHelpDict = std::map<std::string, std::string>;

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
        return LPointer(new LNil());
      }
      /**
       * ペアを作る。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewPair() {
        return LPointer(new LPair());
      }
      /**
       * ペアを作る。
       * @param car Car。
       * @param cdr Cdr。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewPair(const LPointer& car, const LPointer& cdr) {
        return LPointer(new LPair(car, cdr));
      }
      /**
       * シンボルを作る。
       * @param symbol シンボル。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewSymbol(const std::string& symbol) {
        return LPointer(new LSymbol(symbol));
      }
      /**
       * 数字を作る。
       * @param number 数字。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewNumber(double number) {
        return LPointer(new LNumber(number));
      }
      /**
       * 真偽値を作る。
       * @param boolean 真偽値。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewBoolean(bool boolean) {
        return LPointer(new LBoolean(boolean));
      }
      /**
       * 文字列を作る。
       * @param string 文字列。
       * @return オブジェクトのポインタ。
       */
      static LPointer NewString(const std::string& string) {
        return LPointer(new LString(string));
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
        return LPointer(new LFunction(arg_names, expression, scope_chain));
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
        return LPointer(new LN_Function(c_function, func_id, scope_chain));
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
        LPointer ret = NewPair();
        LObject* ptr = ret.get();
        --length;
        for (; length > 0; --length, Next(&ptr)) {
          ptr->cdr(NewPair());
        }
        return ret;
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
      static LType CheckType(const LObject& obj, LType required_type) {
        static const std::string not_type_symbol[] {
          "@not-nil", "@not-pair", "@not-symbol", "@not-number",
          "@not-boolean", "@not-string", "@not-procedure", "@not-procedure"
        };

        LType ret = obj.type();
        if (ret != required_type) {
          throw GenError(not_type_symbol[static_cast<int>(ret)],
          "'" + obj.ToString() + "' is not " + TypeToName(required_type)
          + ". It's " + TypeToName(ret) + ".");
        }
        return ret;
      }

      /**
       * リストかどうかをチェックする。 ダメなら例外を発生。
       * @param obj チェックするオブジェクト。
       * @return リストならtrue。
       */
      static bool CheckList(const LObject& obj) {
        if (obj.IsList()) {
          return true;
        }

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
       * @return 引数の数。
       */
      static int GetReadyForFunction(const LObject& args,
      int required_args, LObject** args_ptr_ptr) {
        // 引数の数をカウント。
        int ret = CountList(args) - 1;

        // 第一引数のポインタを得る。
        if (ret >= 1) *args_ptr_ptr = args.cdr().get();

        // 引数の数をチェック。
        if (ret < required_args) {
          throw GenError("@insufficient-arguments", "'"
          + args.car()->ToString() + "' requires "
          + std::to_string(required_args) + " arguments and more. Not "
          + std::to_string(ret) + ".");
        }

        return ret;
      }

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
       * 関数オブジェクト。 (LC_Function)
       */
      LPointer operator()(LPointer self, LObject* caller,
      const LObject& abs) {
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

      /**
       * ヘルプ辞書に追加する。
       * @param key ヘルプのキー。
       * @param content ヘルプの内容。
       */
      void AddHelp(const std::string& key, const std::string& content) {
        help_dict_.emplace(key, content);
      }

      // ======== //
      // アクセサ //
      // ======== //
      /**
       * アクセサ - カッコの整合性。
       * @return カッコの整合性。
       */
      int pareth_counter() const {return parenth_counter_;}
      /**
       * アクセサ - 文字列中かどうか。
       * @return 文字列中かどうか。
       */
      bool in_string() const {return in_string_;}
      /**
       * アクセサ - 解析中のトークンのキュー。
       * @return 解析中のトークンのキュー。
       */
      const std::queue<std::string> token_quene() const {
        return token_queue_;
      }

      // ============== //
      // ネイティブ関数 //
      // ============== //
      /** ネイティブ関数 - help */
      LPointer Help(LPointer self, LObject* caller, const LObject& args);

      // %%% eval
      /** ネイティブ関数 - eval */
      LPointer Eval(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        return caller->Evaluate(*(caller->Evaluate(*(args_ptr->car()))));
      }

      /** ネイティブ関数 - parse */
      LPointer ParseFunc(LPointer self, LObject* caller, const LObject& args);

      // %%% parval
      /** ネイティブ関数 - parval */
      LPointer Parval(LPointer self, LObject* caller, const LObject& args) {
        LPointer result = ParseFunc(self, caller, args);
        return caller->Evaluate(*result);
      }

      // %%% to-string
      /** ネイティブ関数 - to-string */
      LPointer ToStringFunc(LPointer self, LObject* caller,
      const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        return NewString(caller->Evaluate(*(args_ptr->car()))->ToString());
      }

      /** ネイティブ関数 - try */
      LPointer Try(LPointer self, LObject* caller, const LObject& args);

      // %%% throw
      /** ネイティブ関数 - throw */
      LPointer Throw(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        throw caller->Evaluate(*(args_ptr->cdr()));
      }

      // %%% car
      /** ネイティブ関数 - car */
      LPointer CarFunc(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // ペアを得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::PAIR);

        // Carを返す。
        return result->car();
      }

      // %%% cdr
      /** ネイティブ関数 - cdr */
      LPointer CdrFunc(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // ペアを得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::PAIR);

        // Cdrを返す。
        return result->cdr();
      }

      // %%% cons
      /** ネイティブ関数 - cons */
      LPointer Cons(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        return NewPair(caller->Evaluate(*(args_ptr->car())),
        caller->Evaluate(*(args_ptr->cdr()->car())));
      }

      // %%% apply
      /** ネイティブ関数 - apply */
      LPointer Apply(LPointer self, LObject* caller, const LObject& args) {
        // consしてeval。
        return caller->Evaluate(*(Cons(self, caller, args)));
      }

      // %%% quote
      /** ネイティブ関数 - quote */
      LPointer Quote(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        return args_ptr->car()->Clone();
      }

      /**
       * !関数用シンボル上書き関数。
       * @param chain 対象のチェーン。
       * @param symbol シンボル。
       * @param ptr 上書きするポインタ。
       * @return 前の値。
       */
      LPointer SetCore(LScopeChain& chain, const std::string& symbol,
      const LPointer& ptr) {
        // 前の値を得る。
        LPointer prev = chain.SelectSymbol(symbol);
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
      LPointer Set(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // シンボルを得る。
        LPointer symbol_ptr = args_ptr->car();
        CheckType(*symbol_ptr, LType::SYMBOL);
        std::string symbol =  symbol_ptr->symbol();

        // スコープチェーンを得る。
        LScopeChain chain = caller->scope_chain();

        return SetCore(chain, symbol_ptr->symbol(),
        caller->Evaluate(*(args_ptr->cdr()->car())));
      }

      /**
       * (<func>! ...) 関連の関数を(set! ...)に変換して評価する関数。
       * 引数ありバージョン。
       * @param required_args 必要引数。
       * @param func_symbol ファンクションのシンボル。
       */
      LPointer ExclamToSet(int required_args, const std::string& func_symbol,
      LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, required_args, &args_ptr);

        // 第1引数はシンボル。
        LPointer symbol_ptr = args_ptr->car();

        // (set! ...)に変えて評価する。
        return caller->Evaluate(LPair(NewSymbol("set!"), NewPair(symbol_ptr,
        NewPair(NewPair(NewSymbol(func_symbol),
        NewPair(symbol_ptr, args_ptr->cdr())), NewNil()))));
      }

      /** ネイティブ関数 - define */
      LPointer Define(LPointer self, LObject* caller, const LObject& args);

      /** ネイティブ関数 - lambda */
      LPointer Lambda(LPointer self, LObject* caller, const LObject& args);

      /** ネイティブ関数 - let */
      LPointer Let(LPointer self, LObject* caller, const LObject& args);

      /** ネイティブ関数 - while */
      LPointer While(LPointer self, LObject* caller, const LObject& args);

      /** ネイティブ関数 - for */
      LPointer For(LPointer self, LObject* caller, const LObject& args);

      // %%% if
      /** ネイティブ関数 - if */
      LPointer If(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 3, &args_ptr);

        // 第1引数、条件を評価。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::BOOLEAN);

        // #tなら第2引数を評価して返す。
        if (result->boolean()) {
          return caller->Evaluate(*(args_ptr->cdr()->car()));
        }

        // #tではなかったので第3引数を評価して返す。
        return caller->Evaluate(*(args_ptr->cdr()->cdr()->car()));
      }

      /** ネイティブ関数 - cond */
      LPointer Cond(LPointer self, LObject* caller, const LObject& args);

      // %%% begin
      /** ネイティブ関数 - begin */
      LPointer Begin(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 順次実行。
        LPointer ret_ptr = NewNil();
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          ret_ptr = caller->Evaluate(*(args_ptr->car()));
        }

        return ret_ptr;
      }

      /** ネイティブ関数 - display */
      LPointer Display(LPointer self, LObject* caller, const LObject& args);

      /** ネイティブ関数 - stdin */
      LPointer Stdin(LPointer self, LObject* caller, const LObject& args);

      /** ネイティブ関数 - stdout */
      LPointer Stdout(LPointer self, LObject* caller, const LObject& args);

      /** ネイティブ関数 - stderr */
      LPointer Stderr(LPointer self, LObject* caller, const LObject& args);

      /** ネイティブ関数 - import */
      LPointer Import(LPointer self, LObject* caller, const LObject& args);

      // %%% equal?
      /** ネイティブ関数 - equal? */
      LPointer EqualQ(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer first = caller->Evaluate(*(args_ptr->car()));

        // 比較していく。
        LPointer result;
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(*(args_ptr->car()));
          if (first != result) return NewBoolean(false);
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
      LPointer QFunc(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 調べていく。
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          if (caller->Evaluate(*(args_ptr->car()))->type() != LTYPE) {
            return NewBoolean(false);
          }
        }

        return NewBoolean(true);
      }

      // %%% procedure?
      LPointer ProcedureQ(LPointer self, LObject* caller,
      const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 調べていく。
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(*(args_ptr->car()));
          if (!(result->IsFunction() || result->IsN_Function())) {
            return NewBoolean(false);
          }
        }

        return NewBoolean(true);
      }

      /** ネイティブ関数 - output-stream */
      LPointer OutputStream(LPointer self, LObject* caller,
      const LObject& args);

      /** ネイティブ関数 - input-stream */
      LPointer InputStream(LPointer self, LObject* caller,
      const LObject& args);

      // %%% system
      /** ネイティブ関数 - system */
      LPointer System(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::STRING);

        return NewNumber(std::system(result->string().c_str()));
      }

      /** ネイティブ関数 - append */
      LPointer Append(LPointer self, LObject* caller, const LObject& args);

      /** ネイティブ関数 - ref */
      LPointer Ref(LPointer self, LObject* caller, const LObject& args);

      // %%% list
      /** ネイティブ関数 - list */
      LPointer List(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = args.cdr().get();
        LPointer ret_ptr = NewList(CountList(*args_ptr));

        for (LObject* ptr = ret_ptr.get(); args_ptr->IsPair();
        Next(&args_ptr), Next(&ptr)) {
          ptr->car(caller->Evaluate(*(args_ptr->car())));
        }

        return ret_ptr;
      }

      /** ネイティブ関数 - list-replace */
      LPointer ListReplace(LPointer self, LObject* caller,
      const LObject& args);

      /** ネイティブ関数 - list-remove */
      LPointer ListRemove(LPointer self, LObject* caller,
      const LObject& args);

      /** ネイティブ関数 - list-search */
      LPointer ListSearch(LPointer self, LObject* caller,
      const LObject& args);

      /** ネイティブ関数 - map */
      LPointer Map(LPointer self, LObject* caller, const LObject& args);

      // %%% range
      /** ネイティブ関数 - range */
      LPointer Range(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 個数を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
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

      // %%% length
      /** ネイティブ関数 - length */
      LPointer LengthFunc(LPointer self, LObject* caller,
      const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));

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
      LPointer NumEqual(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 比較。
          if (result->number() != value) return NewBoolean(false);
        }

        return NewBoolean(true);
      }

      // %%% ~=
      /** ネイティブ関数 - ~= */
      LPointer NumNotEqual(LPointer self, LObject* caller,
      const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 比較。
          if (result->number() != value) return NewBoolean(true);
        }

        return NewBoolean(false);
      }

      // %%% >
      /** ネイティブ関数 - > */
      LPointer NumGT(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

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
      LPointer NumGE(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

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
      LPointer NumLT(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

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
      LPointer NumLE(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 比較していく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

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

      // %%% not 
      /** ネイティブ関数 - not */
      LPointer Not(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 元になる値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::BOOLEAN);

        return NewBoolean(!(result->boolean()));
      }

      // %%% and 
      /** ネイティブ関数 - and */
      LPointer And(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 1つでもfalseがあればfalse。
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(*(args_ptr->car()));
          CheckType(*result, LType::BOOLEAN);

          if (!(result->boolean())) return NewBoolean(false);
        }

        return NewBoolean(true);
      }

      // %%% or 
      /** ネイティブ関数 - or */
      LPointer Or(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 1つでもtrueがあればtrue。
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(*(args_ptr->car()));
          CheckType(*result, LType::BOOLEAN);

          if (result->boolean()) return NewBoolean(true);
        }

        return NewBoolean(false);
      }

      // %%% +
      /** ネイティブ関数 - + */
      LPointer Addition(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 全部足す。
        double value = 0.0;
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 足す。
          value += result->number();
        }

        return NewNumber(value);
      }

      // %%% -
      /** ネイティブ関数 - - */
      LPointer Subtraction(LPointer self, LObject* caller,
      const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 引き算の元になる値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 引いていく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 引く。
          value -= result->number();
        }

        return NewNumber(value);
      }

      // %%% *
      /** ネイティブ関数 - * */
      LPointer Multiplication(LPointer self, LObject* caller,
      const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        double value = 1.0;

        // 掛けていく。
        LPointer result;
        for (; args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 掛ける。
          value *= result->number();
        }

        return NewNumber(value);
      }

      // %%% /
      /** ネイティブ関数 - / */
      LPointer Division(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 割り算の元になる値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double value = result->number();

        // 割っていく。
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          // 評価する。
          result = caller->Evaluate(*(args_ptr->car()));

          // タイプをチェック。
          CheckType(*result, LType::NUMBER);

          // 割る。
          value /= result->number();
        }

        return NewNumber(value);
      }

      // %%% ++
      /** ネイティブ関数 - ++ */
      LPointer Inc(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(result->number() + 1.0);
      }

      // %%% --
      /** ネイティブ関数 - -- */
      LPointer Dec(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(result->number() - 1.0);
      }

      /** ネイティブ関数 - string-split */
      LPointer StringSplit(LPointer self, LObject* caller,
      const LObject& args); 

      // %%% front
      /** ネイティブ関数 - front */
      LPointer Front(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckList(*result);

        if (result->IsNil()) return result;

        return result->car();
      }

      // %%% back
      /** ネイティブ関数 - back */
      LPointer Back(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 値を得る。
        LPointer result = caller->Evaluate(*(args_ptr->car()));
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
      LPointer PushFront(LPointer self, LObject* caller,
      const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 第1引数。 リスト。
        LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckList(*target_ptr);

        // プッシュする。
        return NewPair(caller->Evaluate(*(args_ptr->cdr()->car())),
        target_ptr);
      }

      // %%% pop-front
      /** ネイティブ関数 - pop-front */
      LPointer PopFront(LPointer self, LObject* caller,
      const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 第1引数。 リスト。
        LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckList(*target_ptr);

        if (target_ptr->IsNil()) return NewNil();
        return target_ptr->cdr();
      }

      // %%% push-back
      /** ネイティブ関数 - push-front */
      LPointer PushBack(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 2, &args_ptr);

        // 第1引数。 リスト。
        LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckList(*target_ptr);

        if (target_ptr->IsNil()) {
          return NewPair(caller->Evaluate(*(args_ptr->cdr()->car())),
          target_ptr);
        }
        
        LPointer next;
        for (LObject* ptr = target_ptr.get(); ptr->IsPair(); Next(&ptr)) {
          if (ptr->cdr()->IsNil()) {
            ptr->cdr(NewPair(caller->Evaluate(*(args_ptr->cdr()->car())),
            NewNil()));
            return target_ptr;
          }
        }

        return NewNil();
      }

      // %%% pop-back
      /** ネイティブ関数 - pop-back */
      LPointer PopBack(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        // 第1引数。 リスト。
        LPointer target_ptr = caller->Evaluate(*(args_ptr->car()));
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

      // %%% sin
      /** ネイティブ関数 - sin */
      LPointer Sin(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::sin(result->number()));
      }

      // %%% cos
      /** ネイティブ関数 - cos */
      LPointer Cos(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::cos(result->number()));
      }

      // %%% tan
      /** ネイティブ関数 - tan */
      LPointer Tan(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::tan(result->number()));
      }

      // %%% asin
      /** ネイティブ関数 - asin */
      LPointer ASin(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::asin(result->number()));
      }

      // %%% acos
      /** ネイティブ関数 - acos */
      LPointer ACos(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::acos(result->number()));
      }

      // %%% atan
      /** ネイティブ関数 - atan */
      LPointer ATan(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::atan(result->number()));
      }

      // %%% sqrt
      /** ネイティブ関数 - sqrt */
      LPointer Sqrt(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::sqrt(result->number()));
      }

      // %%% abs
      /** ネイティブ関数 - abs */
      LPointer Abs(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::fabs(result->number()));
      }

      // %%% ceil
      /** ネイティブ関数 - ceil */
      LPointer Ceil(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::ceil(result->number()));
      }

      // %%% floor
      /** ネイティブ関数 - floor */
      LPointer Floor(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::floor(result->number()));
      }

      // %%% round
      /** ネイティブ関数 - round */
      LPointer Round(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::round(result->number()));
      }

      // %%% trunc
      /** ネイティブ関数 - trunc */
      LPointer Trunc(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::trunc(result->number()));
      }

      // %%% exp
      /** ネイティブ関数 - exp */
      LPointer Exp(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::exp(result->number()));
      }

      // %%% expt
      /** ネイティブ関数 - expt */
      LPointer Expt(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer first_ptr = caller->Evaluate(*(args_ptr->car()));
        CheckType(*first_ptr, LType::NUMBER);

        LPointer second_ptr = caller->Evaluate(*(args_ptr->cdr()->car()));
        CheckType(*second_ptr, LType::NUMBER);

        return NewNumber(std::pow(first_ptr->number(), second_ptr->number()));
      }

      // %%% log
      /** ネイティブ関数 - log */
      LPointer Log(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::log(result->number()));
      }

      // %%% log2
      /** ネイティブ関数 - log2 */
      LPointer Log2(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        return NewNumber(std::log2(result->number()));
      }

      // %%% log10
      /** ネイティブ関数 - log10 */
      LPointer Log10(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
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

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);

        std::uniform_real_distribution<> dist(0, result->number());
        return NewNumber(dist(engine));
      }

      // %%% max
      /** ネイティブ関数 - max */
      LPointer Max(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double max = result->number();
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(*(args_ptr->car()));
          CheckType(*result, LType::NUMBER);

          double current = result->number();
          if (current > max) max = current;
        }

        return NewNumber(max);
      }

      // %%% min
      /** ネイティブ関数 - min */
      LPointer Min(LPointer self, LObject* caller, const LObject& args) {
        // 準備。
        LObject* args_ptr = nullptr;
        GetReadyForFunction(args, 1, &args_ptr);

        LPointer result = caller->Evaluate(*(args_ptr->car()));
        CheckType(*result, LType::NUMBER);
        double min = result->number();
        for (Next(&args_ptr); args_ptr->IsPair(); Next(&args_ptr)) {
          result = caller->Evaluate(*(args_ptr->car()));
          CheckType(*result, LType::NUMBER);

          double current = result->number();
          if (current < min) min = current;
        }

        return NewNumber(min);
      }

    protected:
      /** ヘルプ辞書。 */
      LHelpDict help_dict_;

    private:
      // ================ //
      // プライベート関数 //
      // ================ //
      /**
       * 再帰コール用パーサ。
       * @return パース結果のポインタ。
       */
      LPointer ParseCore();
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
      // --- 字句解析器 --- //
      /** カッコの数合わせ。 */
      int parenth_counter_;
      /** 文字列解析中かどうか。 */
      bool in_string_;
      /** トークンのベクトル。 */
      std::queue<std::string> token_queue_;
  };
}  // namespace Sayuri

#endif
