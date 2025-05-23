// Compiler for PHP (aka KPHP)
// Copyright (c) 2020 LLC «V Kontakte»
// Distributed under the GPL v3 License, see LICENSE.notice.txt

#pragma once

#include "runtime-common/core/runtime-core.h"
#include "runtime/allocator.h"
#include "runtime/exception.h"
#include "runtime/storage.h"

extern bool resumable_finished;
extern int64_t first_forked_resumable_id;

extern const char* last_wait_error;

#define WAIT return false;
#define RETURN(x)                                                                                                                                              \
  output_->save<ReturnT>(x);                                                                                                                                   \
  return true;
#define RETURN_VOID()                                                                                                                                          \
  output_->save_void();                                                                                                                                        \
  return true;

#define TRY_WAIT(labelName, a, T)                                                                                                                              \
  if (!resumable_finished) {                                                                                                                                   \
    pos__ = &&labelName;                                                                                                                                       \
    WAIT;                                                                                                                                                      \
  labelName:                                                                                                                                                   \
    php_assert(input_ != nullptr);                                                                                                                             \
    a = input_->load<T>();                                                                                                                                     \
  }

#define TRY_WAIT_DROP_RESULT(labelName, T)                                                                                                                     \
  if (!resumable_finished) {                                                                                                                                   \
    pos__ = &&labelName;                                                                                                                                       \
    WAIT;                                                                                                                                                      \
  labelName:                                                                                                                                                   \
    php_assert(input_ != nullptr);                                                                                                                             \
    input_->load<T>();                                                                                                                                         \
  }

#define RESUMABLE_BEGIN                                                                                                                                        \
  if (pos__ != nullptr) {                                                                                                                                      \
    goto* pos__;                                                                                                                                               \
  }                                                                                                                                                            \
  do {

#define RESUMABLE_END                                                                                                                                          \
  }                                                                                                                                                            \
  while (0)                                                                                                                                                    \
    ;                                                                                                                                                          \
  pos__ = &&unreachable_end_label;                                                                                                                             \
  unreachable_end_label:                                                                                                                                       \
  php_assert(0);                                                                                                                                               \
  return false;

class Resumable : public ManagedThroughDlAllocator {
protected:
  static Storage* input_;
  static Storage* output_;
  void* pos__{nullptr};

  virtual bool run() = 0;

public:
  Resumable() = default;
  virtual ~Resumable() = default;

  bool resume(int64_t resumable_id, Storage* input);
  void* get_stack_ptr() {
    return pos__;
  }

  virtual bool is_internal_resumable() const noexcept {
    return false;
  }

  static void update_output();
};

int32_t get_resumable_stack(void** buffer, int32_t limit);

int64_t fork_resumable(Resumable* resumable) noexcept;
Storage* start_resumable_impl(Resumable* resumable) noexcept;

template<class T>
T start_resumable(Resumable* resumable) noexcept {
  auto* storage = start_resumable_impl(resumable);
  return storage ? storage->load<T>() : T();
}

void resumable_run_ready(int64_t resumable_id);

bool wait_without_result(int64_t resumable_id, double timeout = -1.0);
void wait_without_result_synchronously(int64_t resumable_id);
bool wait_without_result_synchronously_safe(int64_t resumable_id);

int64_t register_forked_resumable(Resumable* resumable);
Resumable* get_forked_resumable(int64_t resumable_id);
Storage* get_forked_storage(int64_t resumable_id);

int64_t wait_queue_create(const array<int64_t>& resumable_ids);
void unregister_wait_queue(int64_t queue_id);
int64_t wait_queue_push_unsafe(int64_t queue_id, int64_t resumable_id);
Optional<int64_t> wait_queue_next_synchronously(int64_t queue_id);

void global_init_resumable_lib();
void init_resumable_lib();

bool f$wait_concurrently(int64_t resumable_id);
inline bool f$wait_concurrently(Optional<int64_t> resumable_id) {
  return f$wait_concurrently(resumable_id.val());
}
inline bool f$wait_concurrently(const mixed& resumable_id) {
  return f$wait_concurrently(resumable_id.to_int());
}

void f$sched_yield();
void f$sched_yield_sleep(double timeout);

int64_t f$wait_queue_create();
int64_t f$wait_queue_create(const mixed& resumable_ids);
int64_t f$wait_queue_push(int64_t queue_id, const mixed& resumable_ids);
bool f$wait_queue_empty(int64_t queue_id);
Optional<int64_t> f$wait_queue_next(int64_t queue_id, double timeout = -1.0);
Optional<int64_t> f$wait_queue_next_synchronously(int64_t queue_id);

int64_t f$get_running_fork_id();
Optional<array<mixed>> f$get_fork_stat(int64_t fork_id);

template<typename T, bool use_autogenerated_loader = true>
class wait_result_resumable final : public Resumable {
public:
  wait_result_resumable(int64_t resumable_id, double timeout) noexcept
      : resumable_id_(resumable_id),
        timeout_(timeout) {}

private:
  using ReturnT = T;

  bool run() final {
    RESUMABLE_BEGIN
    ready_ = wait_without_result(resumable_id_, timeout_);
    TRY_WAIT(wait_result_resumable_label_1, ready_, bool);
    if (!ready_) {
      last_wait_error = last_wait_error ?: "Timeout in wait";
      return on_return(nullptr);
    }

    Storage* output = get_forked_storage(resumable_id_);
    if (output->tag == 0) {
      last_wait_error = "Result already was gotten";
      return on_return(nullptr);
    }

    return on_return(output);
    RESUMABLE_END
  }

  bool on_return(Storage* output) noexcept {
    if constexpr (std::is_same_v<T, void>) {
      if (output) {
        output->load_as<void>();
      }
      RETURN_VOID();
    } else {
      if constexpr (use_autogenerated_loader) {
        if (output) {
          RETURN(output->load_as<T>());
        }
        RETURN(Optional<bool>{});
      } else {
        if (output) {
          RETURN(output->load<T>());
        }
        RETURN(T{});
      }
    }
  }

  int64_t resumable_id_;
  double timeout_;
  bool ready_{false};
};

template<typename T, bool is_php_runtime_type = true>
T f$wait(int64_t resumable_id, double timeout = -1.0) {
  return start_resumable<T>(new wait_result_resumable<T, is_php_runtime_type>(resumable_id, timeout));
}

template<typename T>
T f$wait(Optional<int64_t> resumable_id, double timeout = -1.0) {
  return f$wait<T>(resumable_id.val(), timeout);
}

template<typename T>
T f$wait_synchronously(int64_t resumable_id) {
  wait_without_result_synchronously(resumable_id);
  return start_resumable<T>(new wait_result_resumable<T>(resumable_id, -1));
}

template<typename T>
T f$wait_synchronously(Optional<int64_t> resumable_id) {
  return f$wait_synchronously<T>(resumable_id.val());
}

template<typename T>
class wait_multi_resumable final : public Resumable {
public:
  explicit wait_multi_resumable(const array<int64_t>& resumable_ids) noexcept
      : resumable_ids_(resumable_ids),
        result_(resumable_ids.size()) {}

private:
  using ReturnT = T;

  bool run() final {
    RESUMABLE_BEGIN
    for (resumable_it_ = const_begin(resumable_ids_); resumable_it_ != const_end(resumable_ids_); ++resumable_it_) {
      next_waited_result_ = f$wait<typename T::value_type>(resumable_it_.get_value());

      TRY_WAIT(wait_multi_label, next_waited_result_, typename T::value_type);
      CHECK_EXCEPTION(RETURN(T()));

      result_.set_value(resumable_it_.get_key(), next_waited_result_);
    }

    RETURN(result_);
    RESUMABLE_END
  }

  array<int64_t> resumable_ids_;
  T result_;

  array<int64_t>::const_iterator resumable_it_;
  typename T::value_type next_waited_result_;
};

template<typename T>
T f$wait_multi(const array<Optional<int64_t>>& resumable_ids) {
  auto ids = array<int64_t>::convert_from(resumable_ids);
  return f$wait_multi<T>(ids);
}

template<typename T>
T f$wait_multi(const array<int64_t>& resumable_ids) {
  return start_resumable<T>(new wait_multi_resumable<T>(resumable_ids));
}

void forcibly_stop_all_running_resumables();
