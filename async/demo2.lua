local net = require "net"
local Thread = net.Thread
local Listener = net.Listener

print('event =', net.g_event, net)
function main()
	-- print('main线程')
	local lconn = Listener:new("8888")
	while true do
		local conn = lconn:accept()
		-- print('accept居然真的返回了？', conn)
		Thread.new(function()
			conn:add2event(net.g_event)
			-- print('启动协程服务连接')
			while true do
				-- print('xxx运行read之前', conn)
				local data = conn:read()
				-- print('从client读了一条数据')
				conn:write(data)
				-- print('再写回给client')
			end
		end)
	end
	-- print '靠，居然退出了?'
end

Thread.new(main)
--[[
local Listener = net.Listener

function main()
	local lconn = Listener:new("8888")
	print('听8888端口')
	while true do
		local conn = lconn:accept()
		Thread.new(function()
			conn:add2event(net.event)
			while true do
				local data = conn:read()
				conn:write(data)
			end
		end)
	end
end

Thread.new(main)
--]]
Thread.big_bang()
