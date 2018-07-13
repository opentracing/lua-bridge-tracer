bridge_tracer = require 'opentracing_bridge_tracer'

assert(#arg == 2)

-- initialize a tracer
tracer_library = arg[1]
config_file = arg[2]

f = assert(io.open(config_file, "rb"))
config = f:read("*all")
f:close()

tracer = bridge_tracer:new(tracer_library, config)

parent_span = tracer:start_span("parent")

-- create a child span
child_span = tracer:start_span(
                    "ChildA", 
                    {["references"] = {{"child_of", parent_span:context()}}})

child_span:set_tag("simple tag", 123)
child_span:log_kv({["event"] = "simple log", ["abc"] = 123})
child_span:finish()

-- propagate the span context
carrier = {}
tracer:text_map_inject(parent_span:context(), carrier)
span_context = tracer:text_map_extract(carrier)
assert(span_context ~= nil)

propagation_span = tracer:start_span(
                      "PropagationSpan", 
                      {["references"] = {{"follows_from", span_context}}})
propagation_span:finish()

-- close the tracer to ensure that spans are flushed
parent_span:finish()
tracer:close()
