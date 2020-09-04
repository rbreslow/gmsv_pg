#ifndef _GLOO_LUA_VALUE_H_
#define _GLOO_LUA_VALUE_H_

#include <map>
#include <string>
#include <memory>
#include <cassert>
#include <algorithm>
#include "GarrysMod/Lua/Interface.h"

// C++ 17 std::variant pollyfill
// https://github.com/mpark/variant
#include "variant.hpp"

namespace GarrysMod {
namespace Lua {

  class LuaValue
  {
  public:
    typedef bool                         bool_t;
    typedef std::map<LuaValue, LuaValue> table_t;
    typedef double                       number_t;
    typedef std::string                  string_t;
    typedef CFunc                        function_t;
    typedef void*                        userdata_t;
    typedef mpark::variant<
      bool_t,
      table_t,
      number_t,
      string_t,
      function_t,
      userdata_t
    > value_t;
  private:
    int     _type;
    value_t _value;
  public:
    int type() const { return _type; }
  public:
    LuaValue() { _type = Type::Nil; }

    LuaValue(bool_t value) { _type = Type::Bool; _value = value; }
    LuaValue(table_t value) { _type = Type::Table; _value = value; }
    LuaValue(number_t value) { _type = Type::Number; _value = value; }
    LuaValue(string_t value) { _type = Type::String; _value = value; }
    LuaValue(function_t value) { _type = Type::Function; _value = value; }
    LuaValue(userdata_t value) { _type = Type::UserData; _value = value; }
    LuaValue(int type, userdata_t value) { _type = type; _value = value; }

    LuaValue(const LuaValue &value) { Copy(value); }

    LuaValue(int value) { _type = Type::Number; _value = (number_t)value; }
    LuaValue(unsigned int value) { _type = Type::Number; _value = (number_t)value; }
    LuaValue(const char *value) { _type = Type::String; _value = std::string(value); }
  public:
    /**
     * @brief copy lua value from that
     * @param that - Lua value
     */
    void Copy(const LuaValue &that)
    {
      _type = that._type;
      _value = that._value;
    }

    /**
     * @brief Checks if type is equal to supplied type
     * @param type - Type to check
     * @throw std::runtime_error
     */
    void AssertType(int type) const
    {
      if (_type != type)
        throw std::runtime_error("Expected type '" + std::string("") + "' not type '" + std::string("") + "'");
    }

    /**
     * @brief Push table value to lua stack
     * @param state - Lua state
     */
    void PushTable(lua_State *state) const
    {
      if (_type != Type::Table)
        throw new std::runtime_error("Unable to push type '" + std::string(LUA->GetTypeName(_type)) + "' as table");

      // Create table
      LUA->CreateTable();

      // Iterate over table value
      for (const auto &pair : mpark::get<table_t>(_value))
      {
        // Push key and value to stack
        pair.first.Push(state);
        pair.second.Push(state);
        // Assign pair to table
        LUA->SetTable(-3);
      }
    }

    /**
     * @brief Push value to lua stack 
     * @param state - Lua state
     * @return number of items pushed to stack
     */
    int Push(lua_State *state) const
    {
      switch (_type)
      {
        case Type::Nil: LUA->PushNil(); break;
        case Type::Table: PushTable(state); break;
        case Type::Number: LUA->PushNumber(mpark::get<number_t>(_value)); break;
        case Type::String: LUA->PushString(mpark::get<string_t>(_value).c_str()); break;
        case Type::Bool: LUA->PushBool(mpark::get<bool_t>(_value)); break;
        case Type::Function: LUA->PushCFunction(mpark::get<function_t>(_value)); break;
        default:
          LUA->PushUserdata(mpark::get<userdata_t>(_value));
          break;
      }

      return 1;
    }
  public:
    inline LuaValue& operator= (const LuaValue& rhs)
    {
      Copy(rhs);
      return *this;
    }

    inline bool operator< (const LuaValue& rhs) const
    {
      if (_type != rhs._type) return _type < rhs._type;

      switch (_type)
      {
        case Type::Nil: return false;
        case Type::Bool: return mpark::get<bool_t>(_value) < mpark::get<bool_t>(rhs._value);
        case Type::Number: return mpark::get<number_t>(_value) < mpark::get<number_t>(rhs._value);
        case Type::String: return mpark::get<string_t>(_value) < mpark::get<string_t>(rhs._value);
        case Type::Function: return mpark::get<function_t>(_value) < mpark::get<function_t>(rhs._value);
        default: return mpark::get<userdata_t>(_value) < mpark::get<userdata_t>(rhs._value);
      }
    }
    inline bool operator> (const LuaValue& rhs) const { return rhs < *this; }
    inline bool operator<=(const LuaValue& rhs) const { return !(*this < rhs); }
    inline bool operator>=(const LuaValue& rhs) const { return !(rhs < *this); }
    inline bool operator==(const LuaValue& rhs) const
    {
      if (_type != rhs._type)
        return false;

      switch (_type)
      {
        case Type::Nil: return true;
        case Type::Bool: return mpark::get<bool_t>(_value) == mpark::get<bool_t>(rhs._value);
        case Type::Table: return mpark::get<table_t>(_value) == mpark::get<table_t>(rhs._value);
        case Type::Number: return mpark::get<number_t>(_value) == mpark::get<number_t>(rhs._value);
        case Type::String: return mpark::get<string_t>(_value) == mpark::get<string_t>(rhs._value);
        case Type::Function: return mpark::get<function_t>(_value) == mpark::get<function_t>(rhs._value);
        default: return mpark::get<userdata_t>(_value) == mpark::get<userdata_t>(rhs._value);
      }
    }
    inline bool operator!=(const LuaValue& rhs) const { return !(*this == rhs); }
    inline LuaValue& operator[](LuaValue idx)
    {
      AssertType(Type::Table);
      return mpark::get<table_t>(_value)[idx];
    }

    template<typename T>
    inline LuaValue& operator= (T rhs) { return *this = LuaValue(rhs); }
    template<typename T>
    inline bool operator< (T rhs) const { return *this < LuaValue(rhs); }
    template<typename T>
    inline bool operator> (T rhs) const { return *this > LuaValue(rhs); }
    template<typename T>
    inline bool operator<=(T rhs) const { return *this <= LuaValue(rhs); }
    template<typename T>
    inline bool operator>=(T rhs) const { return *this >= LuaValue(rhs); }
    template<typename T>
    inline bool operator==(T rhs) const { return *this == LuaValue(rhs); }
    template<typename T>
    inline bool operator!=(T rhs) const { return *this != LuaValue(rhs); }

    operator const bool_t() const { return mpark::get<bool_t>(_value); }
    operator const table_t() const { return mpark::get<table_t>(_value); }
    operator const number_t() const { return mpark::get<number_t>(_value); }
    operator const string_t() const { return mpark::get<string_t>(_value); }
    operator const function_t() const { return mpark::get<function_t>(_value); }
    operator const userdata_t() const { return mpark::get<userdata_t>(_value); }

    operator int() const { return (int)mpark::get<number_t>(_value); }
  public:
    /**
     * @brief pop lua table from stack
     * @param state    - Lua state
     * @param position - Lua stack position
     * @return new lua table value
     */
    static inline LuaValue PopTable(lua_State *state, int position = 1)
    {
      int     table_ref;
      auto    table_value = LuaValue(table_t());
      int     type = LUA->GetType(position);

      if (type != Type::Table)
        throw std::runtime_error("Unable to pop type '" + std::string(LUA->GetTypeName(type)) + "' as table");

      // Create table ref and push ref to stack
      LUA->Push(position);
      table_ref = LUA->ReferenceCreate();
      LUA->ReferencePush(table_ref);
      // Push nil as first key
      LUA->PushNil();

      // Increment lua iterator
      while (LUA->Next(-2))
      {
        LuaValue key;
        LuaValue value;

        // Push key and table ref
        LUA->Push(-2);
        LUA->ReferencePush(table_ref);

        // Ensure key is not equal to table ref
        if (LUA->Equal(-1, -2))
          throw std::runtime_error("Unable to pop table with cyclic reference");
        else
          key = LuaValue::Pop(state, -2);

        // Ensure value is not equal to table ref
        if (LUA->Equal(-1, -3))
          throw std::runtime_error("Unable to pop table with cyclic reference");
        else
          value = LuaValue::Pop(state, -3);

        // Store key/value pair
        mpark::get<table_t>(table_value._value)[key] = value;

        // Pop key copy, table ref, and value
        LUA->Pop(3);
      }

      // Pop table and free table ref
      LUA->Pop();
      LUA->ReferenceFree(table_ref);

      return table_value;
    }

    /**
     * @brief pop lua value from stack
     * @param state    - Lua state
     * @param position - Lua stack position
     * @returns new lua value
     */
    static inline LuaValue Pop(lua_State *state, int position = 1)
    {
      int type = LUA->GetType(position);

      switch (type)
      {
        case Type::Nil:
          return LuaValue();
        case Type::Bool:
          return LuaValue(LUA->GetBool(position));
        case Type::Table:
          return PopTable(state, position);
        case Type::Number:
          return LuaValue(LUA->GetNumber(position));
        case Type::String:
          return LuaValue(std::string(LUA->GetString(position)));
        case Type::Function:
          return LuaValue(LUA->GetCFunction(position));
        case Type::UserData:
          return LuaValue(LUA->GetUserdata(position));
      }

      return LuaValue();
    }

    /**
     * @brief creates empty LuaValue
     * @param type - Lua type
     */
    static inline LuaValue Make(int type)
    {
      switch (type)
      {
        case Type::Nil:
          return LuaValue();
        case Type::Bool:
          return LuaValue(false);
        case Type::Table:
          return LuaValue(table_t());
        case Type::Number:
          return LuaValue(0.0);
        case Type::Function:
          return LuaValue(__empty);
        case Type::UserData:
          return LuaValue((void*)nullptr);
        default:
          return LuaValue(type, nullptr);
      }
    }

    /**
     * @brief push value to lua stack
     * @param state - lua state
     * @param value - lua value
     * @return number of items pushed to stack
     */
    template<typename T>
    static int Push(lua_State *state, T value)
    {
      return LuaValue(value).Push(state);
    }
  private:
    static int __empty(lua_State *state) { return 0; }
  }; // LuaValue

}} // GarrysMod::Lua

#endif//_GLOO_LUA_VALUE_H_
