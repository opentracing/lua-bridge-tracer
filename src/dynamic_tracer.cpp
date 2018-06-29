#include "dynamic_tracer.h"

#include <opentracing/dynamic_load.h>

namespace lua_bridge_tracer {
//------------------------------------------------------------------------------
// DynamicSpan
//------------------------------------------------------------------------------
namespace {
class DynamicSpan : public opentracing::Span {
 public:
  DynamicSpan(std::shared_ptr<opentracing::Tracer> tracer,
              std::unique_ptr<opentracing::Span>&& span) noexcept
      : tracer_{tracer}, span_{std::move(span)} {}

  void FinishWithOptions(const opentracing::FinishSpanOptions&
                             finish_span_options) noexcept final {
    span_->FinishWithOptions(finish_span_options);
  }

  void SetOperationName(opentracing::string_view name) noexcept final {
    span_->SetOperationName(name);
  }

  void SetTag(opentracing::string_view key,
              const opentracing::Value& value) noexcept final {
    span_->SetTag(key, value);
  }

  void SetBaggageItem(opentracing::string_view restricted_key,
                      opentracing::string_view value) noexcept final {
    span_->SetBaggageItem(restricted_key, value);
  }

  std::string BaggageItem(opentracing::string_view restricted_key) const
      noexcept final {
    return span_->BaggageItem(restricted_key);
  }

  void Log(std::initializer_list<
           std::pair<opentracing::string_view, opentracing::Value>>
               fields) noexcept final {
    span_->Log(fields);
  }

  const opentracing::SpanContext& context() const noexcept final {
    return span_->context();
  }

  const opentracing::Tracer& tracer() const noexcept final { return *tracer_; }

 private:
  std::shared_ptr<opentracing::Tracer> tracer_;
  std::unique_ptr<opentracing::Span> span_;
};
}  // namespace

//------------------------------------------------------------------------------
// DynamicTracer
//------------------------------------------------------------------------------
namespace {
class DynamicTracer : public opentracing::Tracer,
                      public std::enable_shared_from_this<DynamicTracer> {
 public:
  DynamicTracer(opentracing::DynamicLibraryHandle&& handle,
                std::shared_ptr<opentracing::Tracer> tracer) noexcept
      : handle_{std::move(handle)}, tracer_{std::move(tracer)} {}

  std::unique_ptr<opentracing::Span> StartSpanWithOptions(
      opentracing::string_view operation_name,
      const opentracing::StartSpanOptions& options) const noexcept final {
    auto span = tracer_->StartSpanWithOptions(operation_name, options);
    if (span == nullptr) return nullptr;
    return std::unique_ptr<opentracing::Span>{
        new (std::nothrow) DynamicSpan(tracer_, std::move(span))};
  }

 private:
  opentracing::DynamicLibraryHandle handle_;
  std::shared_ptr<opentracing::Tracer> tracer_;
};
}  // namespace

//------------------------------------------------------------------------------
// make_dynamic_tracer
//------------------------------------------------------------------------------
std::shared_ptr<opentracing::Tracer> make_dynamic_tracer(
    const char* tracer_library, const char* config) {

  return nullptr;
}
}  // namespace lua_bridge_tracer
