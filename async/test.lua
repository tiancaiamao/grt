local async = require "async"
local EventLoop = async.EventLoop
local Listener = async.Listener
local Connection = async.Connection

local event = EventLoop:new()
local server = Listener:new("8888")
function server:on_conn(fd, event) 
	local conn = {
		fd = fd,
		on_read = function (c, p)
			c:write(p)
		end
	}
	local c = Connection:new(conn)
	c:add2event(event)
end

event:add(server)
event:run()

-- 同步API，基于coroutine

--[[
local lconn = net.listen(":8808")
while true {
	conn, ok = lconn.accept()
	task.new(function()
		data = conn:read()
		conn:write(data)
	end)
}
--]]
