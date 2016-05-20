local chttp = require "chttp"
local async = require "async"
local new_event_loop = async.new_event_loop
local new_listener = async.new_listener
local Connection = async.Connection

local Parser = {}

function Parser:parse(data, result)
  chttp.parse(self.parser, data, result)
end

function new_parser()
  return new_object(Parser,{
      parser = chttp.new_parser()
  })
end

local GET = 1
local POST = 2
local HEAD = 3

function new_object(method, data)
  local o = data or {}
  o.__index = method
  setmetatable(o, o)
  return o
end

local HttpConn = new_object(Connection)

function HttpConn:on_read(data)
    if not data then
        return
    end
    self.parser:parse(data, self.request)
    print('日，on_read中')
    if self.request.complete then
        for k,v in pairs(self.request.Head) do
            print(k, v)
        end

        local resp = self.handler(self.request)
        self:write(resp)
        print('cao...write没生效么？什么鬼')
    else
        print('处理了一部分数据:', data)
    end
end

function new_http_conn(fd)
  local o = new_object(HttpConn, new_conn(fd))
  o.parser = new_parser()
  return o
end

function serve(port, handler)
    local server = new_listener(port)
    function server:on_conn(fd, event)
        local c = new_http_conn(fd)
        c.request = {}
        c.handler = handler
        c:add2event(event)
    end

    local event = new_event_loop()
    event:add(server)
    event:run()
end

local M = {
  Parser = Parser,
  serve = serve,
}

return M
