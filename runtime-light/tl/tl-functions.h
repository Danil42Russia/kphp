// Compiler for PHP (aka KPHP)
// Copyright (c) 2024 LLC «V Kontakte»
// Distributed under the GPL v3 License, see LICENSE.notice.txt

#pragma once

#include <cstdint>

#include "runtime-core/runtime-core.h"
#include "runtime-light/tl/tl-core.h"
#include "runtime-light/tl/tl-types.h"

namespace tl {

// ===== JOB WORKERS =====

inline constexpr uint32_t K2_INVOKE_HTTP_MAGIC = 0xd909'efe8;
inline constexpr uint32_t K2_INVOKE_JOB_WORKER_MAGIC = 0x437d'7312;

struct K2InvokeJobWorker final {
  uint64_t image_id{};
  int64_t job_id{};
  bool ignore_answer{};
  uint64_t timeout_ns{};
  string body;

  bool fetch(TLBuffer &tlb) noexcept;

  void store(TLBuffer &tlb) const noexcept;
};

// ===== CRYPTO =====

inline constexpr uint32_t GET_CRYPTOSECURE_PSEUDORANDOM_BYTES_MAGIC = 0x2491'b81d;
inline constexpr uint32_t GET_PEM_CERT_INFO_MAGIC = 0xa50c'fd6c;
inline constexpr uint32_t DIGEST_SIGN_MAGIC = 0xd345'f658;
inline constexpr uint32_t DIGEST_VERIFY_MAGIC = 0x5760'bd0e;


struct GetCryptosecurePseudorandomBytes final {
  int32_t size{};

  void store(TLBuffer &tlb) const noexcept;
};

struct GetPemCertInfo final {
  bool is_short{true};
  string bytes;

  void store(TLBuffer &tlb) const noexcept;
};

struct DigestSign final {
  string data;
  string private_key;
  DigestAlgorithm algorithm;

  void store(TLBuffer &tlb) const noexcept;
};

struct DigestVerify final {
  string data;
  string public_key;
  DigestAlgorithm algorithm;
  string signature;

  void store(TLBuffer &tlb) const noexcept;
};

} // namespace tl
