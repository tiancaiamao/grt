local M = require "chttp"
package.path = package.path .. ';../async/?.lua'
package.cpath = package.cpath .. ';../async/?.so'
local async = require "async"
local EventLoop = async.EventLoop
local Listener = async.Listener
local Connection = async.Connection


local GET = 1
local POST = 2
local HEAD = 3

local HttpConn = {}
setmetatable(HttpConn, {__index = Connection})
function HttpConn:on_read(data)
    if not data then
        return
    end
    M.parse(self.request, data)
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
        local c = HttpConn:new(fd)
        c.request = {}
        c.handler = handler
        c:add2event(event)
    end

    local event = EventLoop:new()
    event:add(server)
    event:run()
end

M.serve = serve

return M
