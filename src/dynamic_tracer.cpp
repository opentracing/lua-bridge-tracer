#include "dynamic_tracer.h"

#include <opentracing/dynamic_load.h>

#include <stdexcept>

namespace lua_bridge_tracer {
//--------------------------------------------------------------------------------------------------
// DynamicSpanContext
//--------------------------------------------------------------------------------------------------
namespace {
class DynamicSpanContext final : public opentracing::SpanContext {
 public:
  DynamicSpanContext(
      std::shared_ptr<const opentracing::Tracer> tracer,
      std::unique_ptr<opentracing::SpanContext>&& span_context) noexcept
      : tracer_{std::move(tracer)}, span_context_{std::move(span_context)} {}

  void ForeachBaggageItem(
      std::function<bool(const std::string&, const std::string&)> callback)
      const override {
    span_context_->ForeachBaggageItem(callback);
  }

  const opentracing::SpanContext& context() const noexcept { 
    return *span_context_;
  }

 private:
  std::shared_ptr<const opentracing::Tracer> tracer_;
  std::unique_ptr<opentracing::SpanContext> span_context_;

  friend class DynamicTracer;
};
}  // namespace

//------------------------------------------------------------------------------
// DynamicSpan
//------------------------------------------------------------------------------
namespace {
class DynamicSpan final : public opentracing::Span {
 public:
  DynamicSpan(std::shared_ptr<const opentracing::Tracer> tracer,
              std::unique_ptr<opentracing::Span>&& span) noexcept
      : tracer_{std::move(tracer)}, span_{std::move(span)} {}

 private:
  std::shared_ptr<const opentracing::Tracer> tracer_;
  std::unique_ptr<opentracing::Span> span_;

  void FinishWithOptions(const opentracing::FinishSpanOptions&
                             finish_span_options) noexcept override {
    span_->FinishWithOptions(finish_span_options);
  }

  void SetOperationName(opentracing::string_view name) noexcept override {
    span_->SetOperationName(name);
  }

  void SetTag(opentracing::string_view key,
              const opentracing::Value& value) noexcept override {
    span_->SetTag(key, value);
  }

  void SetBaggageItem(opentracing::string_view restricted_key,
                      opentracing::string_view value) noexcept override {
    span_->SetBaggageItem(restricted_key, value);
  }

  std::string BaggageItem(opentracing::string_view restricted_key) const
      noexcept override {
    return span_->BaggageItem(restricted_key);
  }

  void Log(std::initializer_list<
           std::pair<opentracing::string_view, opentracing::Value>>
               fields) noexcept override {
    span_->Log(fields);
  }

  const opentracing::SpanContext& context() const noexcept override {
    return span_->context();
  }

  const opentracing::Tracer& tracer() const noexcept override {
    return *tracer_;
  }
};
}  // namespace

//------------------------------------------------------------------------------
// DynamicTracer
//------------------------------------------------------------------------------
namespace {
class DynamicTracer final : public opentracing::Tracer,
                            public std::enable_shared_from_this<DynamicTracer> {
 public:
  DynamicTracer(opentracing::DynamicTracingLibraryHandle&& handle,
                std::shared_ptr<opentracing::Tracer>&& tracer) noexcept
      : handle_{std::move(handle)}, tracer_{std::move(tracer)} {}

 private:
  opentracing::DynamicTracingLibraryHandle handle_;
  std::shared_ptr<opentracing::Tracer> tracer_;

  std::unique_ptr<opentracing::Span> StartSpanWithOptions(
      opentracing::string_view operation_name,
      const opentracing::StartSpanOptions& options) const noexcept override {
    auto options_prime = options;
    for (auto& reference : options_prime.references) {
      auto span_context =
          dynamic_cast<const DynamicSpanContext*>(reference.second);
      if (span_context != nullptr) {
        reference.second = span_context->span_context_.get();
      }
    }
    auto span = tracer_->StartSpanWithOptions(operation_name, options);
    if (span == nullptr) {
      return nullptr;
    }
    return std::unique_ptr<opentracing::Span>{
        new (std::nothrow) DynamicSpan(shared_from_this(), std::move(span))};
  }

  opentracing::expected<void> Inject(const opentracing::SpanContext& sc,
                                     std::ostream& writer) const override {
    auto span_context = dynamic_cast<const DynamicSpanContext*>(&sc);
    if (span_context != nullptr) {
      return tracer_->Inject(span_context->context(), writer);
    }
    return tracer_->Inject(sc, writer);
  }

  opentracing::expected<void> Inject(
      const opentracing::SpanContext& sc,
      const opentracing::TextMapWriter& writer) const override {
    auto span_context = dynamic_cast<const DynamicSpanContext*>(&sc);
    if (span_context != nullptr) {
      return tracer_->Inject(span_context->context(), writer);
    }
    return tracer_->Inject(sc, writer);
  }

  opentracing::expected<void> Inject(
      const opentracing::SpanContext& sc,
      const opentracing::HTTPHeadersWriter& writer) const override {
    auto span_context = dynamic_cast<const DynamicSpanContext*>(&sc);
    if (span_context != nullptr) {
      return tracer_->Inject(span_context->context(), writer);
    }
    return tracer_->Inject(sc, writer);
  }

  opentracing::expected<void> Inject(
      const opentracing::SpanContext& sc,
      const opentracing::CustomCarrierWriter& writer) const override {
    auto span_context = dynamic_cast<const DynamicSpanContext*>(&sc);
    if (span_context != nullptr) {
      return tracer_->Inject(span_context->context(), writer);
    }
    return tracer_->Inject(sc, writer);
  }

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      std::istream& reader) const override {
    return this->ExtractImpl(reader);
  }

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      const opentracing::TextMapReader& reader) const override {
    return this->ExtractImpl(reader);
  }

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      const opentracing::HTTPHeadersReader& reader) const override {
    return this->ExtractImpl(reader);
  }

  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> Extract(
      const opentracing::CustomCarrierReader& reader) const override {
    return reader.Extract(*tracer_);
  }

  template <class Reader>
  opentracing::expected<std::unique_ptr<opentracing::SpanContext>> ExtractImpl(
      Reader& reader) const {
    auto span_context_maybe = tracer_->Extract(reader);
    if (!span_context_maybe) {
      return span_context_maybe;
    }
    if (*span_context_maybe == nullptr) {
      return std::unique_ptr<opentracing::SpanContext>{};
    }
    return std::unique_ptr<opentracing::SpanContext>{new DynamicSpanContext{
        shared_from_this(), std::move(*span_context_maybe)}};
  }

  void Close() noexcept final { tracer_->Close(); }
};
}  // namespace

//------------------------------------------------------------------------------
// make_dynamic_tracer
//------------------------------------------------------------------------------
// Dynamically loads a C++ OpenTracing plugin and constructs a tracer with the
// given configuration.
//
// The opentracing::DynamicTracingLibraryHandle returned can't be freed until
// the opentracing::Tracer is freed. To accomplish this, we build a new
// opentracing::Tracer that wraps the plugin's tracer and owns the
// opentracing::DynamicTracingLibraryHandle.
std::shared_ptr<opentracing::Tracer> load_tracer(const char* library_name,
                                                 const char* config) {
  std::string error_message;
  auto handle_maybe =
      opentracing::DynamicallyLoadTracingLibrary(library_name, error_message);
  if (!handle_maybe) {
    throw std::runtime_error{error_message};
  }
  auto& handle = *handle_maybe;
  auto tracer_maybe = handle.tracer_factory().MakeTracer(config, error_message);
  if (!tracer_maybe) {
    throw std::runtime_error{error_message};
  }
  return std::make_shared<DynamicTracer>(std::move(handle),
                                         std::move(*tracer_maybe));
}
}  // namespace lua_bridge_tracer
