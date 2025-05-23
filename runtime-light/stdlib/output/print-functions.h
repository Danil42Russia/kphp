// Compiler for PHP (aka KPHP)
// Copyright (c) 2024 LLC «V Kontakte»
// Distributed under the GPL v3 License, see LICENSE.notice.txt

#pragma once

#include <cstdint>

#include "runtime-common/core/runtime-core.h"
#include "runtime-common/stdlib/string/string-functions.h"
#include "runtime-light/state/instance-state.h"
#include "runtime-light/utils/logs.h"

// === print ======================================================================================

inline void print(const char* s, size_t len) noexcept {
  Response& response{InstanceState::get().response};
  response.output_buffers[response.current_buffer].append(s, len);
}

inline void print(const char* s) noexcept {
  print(s, strlen(s));
}

inline void print(const string_buffer& sb) noexcept {
  print(sb.buffer(), sb.size());
}

inline void print(const string& s) noexcept {
  print(s.c_str(), s.size());
}

inline int64_t f$print(const string& s) noexcept {
  print(s);
  return 1;
}

// === print_r ====================================================================================

string f$print_r(const mixed& v, bool buffered = false) noexcept;

template<class T>
string f$print_r(const class_instance<T>& v, bool buffered = false) noexcept {
  kphp::log::warning("print_r used on object");
  return f$print_r(string{v.get_class(), static_cast<string::size_type>(strlen(v.get_class()))}, buffered);
}

// === var_export =================================================================================

string f$var_export(const mixed& v, bool buffered = false) noexcept;

template<class T>
string f$var_export(const class_instance<T>& v, bool buffered = false) noexcept {
  kphp::log::warning("print_r used on object");
  return f$var_export(string{v.get_class(), static_cast<string::size_type>(strlen(v.get_class()))}, buffered);
}

// === var_dump ===================================================================================

void f$var_dump(const mixed& v) noexcept;

template<class T>
void f$var_dump(const class_instance<T>& v) noexcept {
  kphp::log::warning("print_r used on object");
  return f$var_dump(string{v.get_class(), static_cast<string::size_type>(strlen(v.get_class()))});
}

// ================================================================================================

inline void f$echo(const string& s) noexcept {
  print(s);
}

inline int64_t f$printf(const string& format, const array<mixed>& a) noexcept {
  const string to_print{f$sprintf(format, a)};
  print(to_print);
  return to_print.size();
}

inline Optional<int64_t> f$fprintf(const mixed&, const string&, const array<mixed>&) {
  kphp::log::error("call to unsupported function");
}

inline Optional<int64_t> f$fputcsv(const mixed&, const array<mixed>&, string = string(",", 1), string = string("\"", 1), string = string("\\", 1)) {
  kphp::log::error("call to unsupported function");
}
