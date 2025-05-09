// Compiler for PHP (aka KPHP)
// Copyright (c) 2023 LLC «V Kontakte»
// Distributed under the GPL v3 License, see LICENSE.notice.txt

#include "server/php-runner.h"
#include "server/workers-control.h"

// The size of buffer for alternative signal stack
constexpr auto signal_stack_buffer_size = 65536;

void perform_error_if_running(const char *msg, script_error_t error_type, const std::optional<int> &triggered_by_signal);

void init_handlers();
void worker_global_init_handlers(WorkerType worker_type);
