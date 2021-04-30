// Compiler for PHP (aka KPHP)
// Copyright (c) 2021 LLC «V Kontakte»
// Distributed under the GPL v3 License, see LICENSE.notice.txt

#pragma once

#include "compiler/compiler-core.h"
#include "compiler/function-pass.h"

class NullCheckElimPass final : public FunctionPassBase {
public:
  string get_description() override {
    return "Null checks elimination";
  }

  VertexPtr on_enter_vertex(VertexPtr root) override;
};
