bridge_tracer = require 'opentracing_bridge_tracer'
json = require 'JSON'

function new_mocktracer(json_file)
  mocktracer_path = os.getenv("MOCKTRACER")
  if mocktracer_path == nil then
    error("MOCKTRACER environmental variable must be set")
  end
  return bridge_tracer:new(
            mocktracer_path, 
            '{ "output_file":"' .. json_file .. '" }')
end

function read_json(file)
    local f = assert(io.open(file, "rb"))
    local content = f:read("*all")
    f:close()
    return json:decode(content)
end

describe("in bridge_tracer", function()
  describe("the start_span method", function()
    it("creates spans", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      span = tracer:start_span("abc")
      span:finish()
      tracer:close()
			json = read_json(json_file)
			assert.are.equal(#json, 1)
			assert.are.equal(json[1]["operation_name"], "abc")
    end)

    it("supports customizing the time points", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      span = tracer:start_span("abc", {["start_time"] = 1531434895308545})
      span:finish(1531434896813719)
      tracer:close()
			json = read_json(json_file)
			assert.are.equal(#json, 1)
      assert.is_true(json[1]["duration"] > 1.0e6)
    end)

    it("errors when passed an incorrect operation name", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      local errorfn = function()
        span = tracer:start_span({})
      end
      assert.has_error(errorfn)
    end)
  end)

  describe("a tracer", function()
    it("returns nil when extracting from an empty table", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      -- text map
      context1 = tracer:text_map_extract({})
      assert.are.equal(context1, nil)

      -- http headers
      context2 = tracer:http_headers_extract({})
      assert.are.equal(context2, nil)

      -- binary
      context3 = tracer:binary_extract("")
      assert.are.equal(context3, nil)
    end)
  end)

  describe("a span", function()
    it("supports context propagation", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      span = tracer:start_span("abc")
      span:finish()

      -- text map
      carrier1 = {}
      tracer:text_map_inject(span:context(), carrier1)
      context1 = tracer:text_map_extract(carrier1)
      assert.are_not_equals(context1, nil)

      -- http headers
      carrier2 = {}
      tracer:http_headers_inject(span:context(), carrier2)
      context2 = tracer:http_headers_extract(carrier2)
      assert.are_not_equals(context2, nil)

      -- binary
      carrier3 = tracer:binary_inject(span:context())
      context3 = tracer:binary_extract(carrier3)
      assert.are_not_equals(context3, nil)
    end)

    it("is correctly garbage collected", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      span = tracer:start_span("abc")
      context = span:context()
      tracer = nil
      span = nil
      context = nil

      -- free functions should be called for the tracer, span, and context
      -- 
      -- when run with address sanitizer, leaks should be detected if they
      -- aren't freed
      collectgarbage()
    end)

  end)
end)
