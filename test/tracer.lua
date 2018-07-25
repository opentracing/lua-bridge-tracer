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
  describe("tracer construction", function()
    it("fails when passed an invalid library", function()
      local errorfn = function()
        local tracer = bridge_tracer:new("invalid_library", "invalid_config")
      end
      assert.has_error(errorfn)
    end)

    it("fails when passed an invalid tracer config", function()
      local mocktracer_path = os.getenv("MOCKTRACER")
      if mocktracer_path == nil then
        error("MOCKTRACER environmental variable must be set")
      end
      local errorfn = function()
        local tracer = bridge_tracer:new(mocktracer_path, "invalid_config")
      end
      assert.has_error(errorfn)
    end)

    it("supports construction from a valid library and config", function()
      local json_file = os.tmpname()
      local tracer = new_mocktracer(json_file)
      assert.are_not_equals(tracer, nil)
    end)

    it("supports construction from the C++ global tracer", function()
      local tracer = bridge_tracer:new_from_global()
      assert.are_not_equals(tracer, nil)
    end)
  end)

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

    it("ignore nil references", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      span = tracer:start_span("abc", {["references"] = {{"child_of", nil}}})
      span:finish()
      tracer:close()
			json = read_json(json_file)
			assert.are.equal(#json, 1)
      references = json[1]["references"]
			assert.are.equal(#references, 0)
    end);
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

      -- itnores non-string key-value pairs
      carrier4 = {[1] = "abc", ["abc"] = 2.0}
      tracer:text_map_inject(span:context(), carrier4)
      context4 = tracer:text_map_extract(carrier4)
      assert.are_not_equals(context4, nil)
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

    it("supports attaching tags", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      span = tracer:start_span("abc")
      span:set_tag("s", "abc")
      span:set_tag("i", 123)
      span:finish()
      tracer:close()
			json = read_json(json_file)
			assert.are.equal(#json, 1)
			assert.are.equal(json[1]["tags"]["s"], "abc")
			assert.are.equal(json[1]["tags"]["i"], 123)
    end)

    it("supports logging", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      span = tracer:start_span("abc")
      span:log_kv({["x"] = 123})
      span:finish()
      tracer:close()
			json = read_json(json_file)
			assert.are.equal(#json, 1)
      logs = json[1]["logs"]
      assert.are.equal(#logs, 1)
      records = logs[1]["fields"]
      assert.are.equal(#records, 1)
      assert.are.equal(records[1]["key"], "x")
      assert.are.equal(records[1]["value"], 123)
    end)

    it("supports attaching and querying baggage", function()
      json_file = os.tmpname()
      tracer = new_mocktracer(json_file)
      span = tracer:start_span("abc")
      span:set_baggage_item("abc", "123")
      assert.are_equal(span:get_baggage_item("abc"), "123")
    end)

  end)
end)
