local async = require "async"
local EventLoop = async.EventLoop
local Listener = async.Listener
local Connection = async.Connection

local server = Listener:new("8888")
function server:on_conn(fd, event) 
	local c = Connection:new(fd)
	function c:on_read(data)
		self:write(data)
	end
	c:add2event(event)
end

local event = EventLoop:new()
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
