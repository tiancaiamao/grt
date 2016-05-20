package.cpath = package.cpath .. ';../lib/?.so'
package.path = package.path .. ';../lib/?.lua'

local async = require "async"

local server = async.new_listener("8888")
function server:on_conn(fd, event)
	local c = async.new_connection(fd)
	function c:on_read(data)
		self:write(data)
	end
	c:add2event(event)
end

local event = async.new_event_loop()
event:add(server)
event:run()

