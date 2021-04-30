// Compiler for PHP (aka KPHP)
// Copyright (c) 2021 LLC «V Kontakte»
// Distributed under the GPL v3 License, see LICENSE.notice.txt

#include "compiler/pipes/null-check-elim.h"

namespace {

bool can_be_null(FunctionPtr f, VertexPtr expr) {
  if (expr->type() == op_var && expr->get_string() == "this" && f->modifiers.is_instance()) {
    return false;
  }
  return true;
}

VertexPtr walk_instance_prop(FunctionPtr f, VertexAdaptor<op_instance_prop> instance_prop) {
  if (!can_be_null(f, instance_prop->instance())) {
    instance_prop->is_null_safe = true;
    G->stats.cnt_null_elim += 1;
    return instance_prop;
  }

  return instance_prop;
}

} // namespace

VertexPtr NullCheckElimPass::on_enter_vertex(VertexPtr root) {
  if (auto instance_prop = root.try_as<op_instance_prop>()) {
    return walk_instance_prop(current_function, instance_prop);
  }

  return root;
}
