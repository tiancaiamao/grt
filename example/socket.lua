local net = require "net"
local Thread = net.Thread
local Listener = net.Listener

function main()
	local lconn = Listener:new("8888")
	while true do
		local conn = lconn:accept()
		Thread.new(function()
			conn:add2event(net.g_event)
			while true do
				local data = conn:read()
				conn:write(data)
			end
		end)
	end
end

Thread.new(main)
Thread.big_bang()
