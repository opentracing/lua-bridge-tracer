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
end)
