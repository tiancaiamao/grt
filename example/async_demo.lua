package.cpath = package.cpath .. ';../lib/?.so'
package.path = package.path .. ';../lib/?.lua'

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

