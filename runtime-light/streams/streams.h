// Compiler for PHP (aka KPHP)
// Copyright (c) 2024 LLC «V Kontakte»
// Distributed under the GPL v3 License, see LICENSE.notice.txt

#pragma once

#include <cstdint>
#include <memory>
#include <utility>

#include "runtime-common/core/allocator/script-malloc-interface.h"
#include "runtime-light/coroutine/task.h"

/**
 * **Oneshot component only**
 * Wait for initial stream and process it. There can be 2 types of initial (or starter) streams: 1. http; 2. job worker.
 */
kphp::coro::task<uint64_t> accept_initial_stream() noexcept;

// === read =======================================================================================

kphp::coro::task<std::pair<std::unique_ptr<char, decltype(std::addressof(kphp::memory::script::free))>, size_t>>
read_all_from_stream(uint64_t stream_d) noexcept;

std::pair<std::unique_ptr<char, decltype(std::addressof(kphp::memory::script::free))>, int64_t> read_nonblock_from_stream(uint64_t stream_d) noexcept;

kphp::coro::task<int32_t> read_exact_from_stream(uint64_t stream_d, char* buffer, int32_t len) noexcept;

// === write ======================================================================================

kphp::coro::task<int32_t> write_all_to_stream(uint64_t stream_d, const char* buffer, int32_t len) noexcept;

int32_t write_nonblock_to_stream(uint64_t stream_d, const char* buffer, int32_t len) noexcept;

kphp::coro::task<int32_t> write_exact_to_stream(uint64_t stream_d, const char* buffer, int32_t len) noexcept;
