local chttp = require "chttp"
local async = require "async"
local EventLoop = async.EventLoop
local Listener = async.Listener
local Connection = async.Connection

local Parser = {}
function Parser:new()
  local o = { parser = chttp.new_parser() }
  setmetatable(o, self)
  self.__index = self
  return o
end

function Parser:parse(data, result)
  chttp.parse(self.parser, data, result)
end

local GET = 1
local POST = 2
local HEAD = 3

local HttpConn = {}
setmetatable(HttpConn, {__index = Connection})

function new_http_conn(fd)
  local o = HttpConn:new(fd)
  o.parser = Parser:new()
  o.request = {}
  return o
end

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

function serve(port, handler)
    local server = Listener:new(port)
    function server:on_conn(fd, event)
        local c = new_http_conn(fd)
        c.request = {}
        c.handler = handler
        c:add2event(event)
    end

    local event = EventLoop:new()
    event:add(server)
    event:run()
end

local M = {
  Parser = Parser,
  serve = serve,
}

return M
