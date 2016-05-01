local async = require "async"
local eventloop = async.eventloop
local listener = async.listener
local connection = async.connection

function on_new_connection(fd, event)
	local conn = {
		fd = fd,
		on_read = function (c, p)
			c:write(p)
		end
	}
	local c = connection:new(conn)
	c:add2event(event)
end

local event = eventloop:new()
local server = listener:new({
	port = 8888, 
	on_conn = on_new_connection})

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
