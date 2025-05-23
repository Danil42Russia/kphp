// Compiler for PHP (aka KPHP)
// Copyright (c) 2021 LLC «V Kontakte»
// Distributed under the GPL v3 License, see LICENSE.notice.txt

#pragma once

#include <cstdint>
#include <string_view>

#include "common/algorithms/hashes.h"
#include "runtime-common/core/class-instance/refcountable-php-classes.h"
#include "runtime-common/core/runtime-core.h"
#include "runtime/instance-copy-processor.h"
#include "runtime/to-array-processor.h"

class InstanceReferencesCountingVisitor;

class InstanceDeepCopyVisitor;

class InstanceDeepDestroyVisitor;

class ToArrayVisitor;

class CommonMemoryEstimateVisitor;

namespace job_workers {

enum {
  server_php_script_error_offset = -100,
  client_timeout_error = -102, // same as script timeout
  client_oom_error = -1001,
  server_nothing_replied_error = -2001,
  store_response_incorrect_call_error = -3000,
  store_response_not_enough_shared_messages_error = -3001,
  store_response_too_big_error = -3002,
  store_response_cant_send_error = -3003,
};

struct SendingInstanceBase : virtual abstract_refcountable_php_interface {
  virtual const char* get_class() const noexcept = 0;
  virtual int get_hash() const noexcept = 0;

  virtual void accept(InstanceReferencesCountingVisitor&) noexcept = 0;
  virtual void accept(InstanceDeepCopyVisitor&) noexcept = 0;
  virtual void accept(InstanceDeepDestroyVisitor&) noexcept = 0;

  virtual void accept(ToArrayVisitor&) noexcept {}

  virtual void accept(CommonMemoryEstimateVisitor&) noexcept {}

  virtual size_t virtual_builtin_sizeof() const noexcept = 0;
  virtual SendingInstanceBase* virtual_builtin_clone() const noexcept = 0;

  virtual ~SendingInstanceBase() = default;
};

struct FinishedJob;

struct JobSharedMessage;

} // namespace job_workers

struct C$KphpJobWorkerSharedMemoryPiece : job_workers::SendingInstanceBase {
  C$KphpJobWorkerSharedMemoryPiece* virtual_builtin_clone() const noexcept override = 0;
};

struct C$KphpJobWorkerRequest : job_workers::SendingInstanceBase {
  C$KphpJobWorkerRequest* virtual_builtin_clone() const noexcept override = 0;

  virtual class_instance<C$KphpJobWorkerSharedMemoryPiece> get_shared_memory_piece() const noexcept = 0;
  virtual void set_shared_memory_piece(const class_instance<C$KphpJobWorkerSharedMemoryPiece>&) noexcept = 0;
};

struct C$KphpJobWorkerResponse : job_workers::SendingInstanceBase {
  C$KphpJobWorkerResponse* virtual_builtin_clone() const noexcept override = 0;
};

struct C$KphpJobWorkerResponseError : public refcountable_polymorphic_php_classes<C$KphpJobWorkerResponse> {
  string error;
  int64_t error_code;

  const char* get_class() const noexcept override {
    return R"(KphpJobWorkerResponseError)";
  }

  int32_t get_hash() const noexcept override {
    std::string_view name_view{C$KphpJobWorkerResponseError::get_class()};
    return static_cast<int32_t>(vk::murmur_hash<uint32_t>(name_view.data(), name_view.size()));
  }

  template<class Visitor>
  void generic_accept(Visitor&& visitor) noexcept {
    visitor("error", error);
    visitor("error_code", error_code);
  }

  void accept(InstanceReferencesCountingVisitor& visitor) noexcept override {
    return generic_accept(visitor);
  }

  void accept(InstanceDeepCopyVisitor& visitor) noexcept override {
    return generic_accept(visitor);
  }

  void accept(InstanceDeepDestroyVisitor& visitor) noexcept override {
    return generic_accept(visitor);
  }

  void accept(ToArrayVisitor& visitor) noexcept override {
    return generic_accept(visitor);
  }

  void accept(CommonMemoryEstimateVisitor& visitor) noexcept override;

  size_t virtual_builtin_sizeof() const noexcept override {
    return sizeof(*this);
  }

  C$KphpJobWorkerResponseError* virtual_builtin_clone() const noexcept override {
    return new C$KphpJobWorkerResponseError{*this};
  }
};

class_instance<C$KphpJobWorkerResponseError> f$KphpJobWorkerResponseError$$__construct(class_instance<C$KphpJobWorkerResponseError> const& v$this) noexcept;
string f$KphpJobWorkerResponseError$$getError(class_instance<C$KphpJobWorkerResponseError> const& v$this) noexcept;
int64_t f$KphpJobWorkerResponseError$$getErrorCode(class_instance<C$KphpJobWorkerResponseError> const& v$this) noexcept;

class_instance<C$KphpJobWorkerResponseError> create_error_on_other_memory(int32_t error_code, const char* error_msg,
                                                                          memory_resource::unsynchronized_pool_resource& resource) noexcept;

bool f$is_kphp_job_workers_enabled() noexcept;

int64_t f$get_job_workers_number() noexcept;

void global_init_job_workers_lib() noexcept;
void clear_shared_job_messages() noexcept;

int get_job_timeout_wakeup_id();

void process_job_answer(int job_id, job_workers::FinishedJob* job_result) noexcept;

void process_job_timeout(int job_id) noexcept;
