#pragma once

#include "lua_class_description.h"

#include <opentracing/span.h>

namespace lua_bridge_tracer {
class LuaSpanContext {
 public:
  explicit LuaSpanContext(const std::shared_ptr<const opentracing::Span>& span)
      : span_{span} {}

  explicit LuaSpanContext(
      std::unique_ptr<const opentracing::SpanContext>&& span_context)
      : span_context_{std::move(span_context)} {}

  static const LuaClassDescription description;

  const opentracing::SpanContext& span_context() const noexcept {
    if (span_ != nullptr) return span_->context();
    return *span_context_;
  }

 private:
  std::shared_ptr<const opentracing::Span> span_;
  std::unique_ptr<const opentracing::SpanContext> span_context_;

  static int free(lua_State* L) noexcept;
};
}  // namespace lua_bridge_tracer
