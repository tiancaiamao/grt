local async = require "async"
local AsyncListener = async.Listener
local AsyncConn = async.Connection
local EventLoop = async.EventLoop

-- q1 q2 is runnable queue
local q1 = {}
local q2 = {}

local tls = {}

local Thread = {}
function Thread.new(f)
	local co = coroutine.create(f),
	print('Thread.new', q2, #q2)
	table.insert(q2, co)
end

function Thread.notify_ready(co, ...)
	tls[co] = table.pack(...)
	table.insert(q2, co)
	print("len of q2 = ", #q2)
	coroutine.yield()
end

function Thread.yield()
	table.insert(q2, coroutine.running())
	coroutine.yield()
end

local event = EventLoop:new()

local co_epoll = coroutine.create(function()
	print('epoll线程')
	event:run()
end) 

local Conn = AsyncConn:new()
function Conn:on_read(data)
	print('Conn:on_read回调，得到数据', data, event, self.co)
	Thread.notify_ready(self.co, data, event)
end
function Conn:read()
	self.co = coroutine.running()
	print('调用Conn:read,再切加eventloop', self.co)
	local x, y, z = coroutine.yield()
	print("conn read:", x, y, z)
	return x, y, z
end

local Listener = {}
setmetatable(Listener, {__index=AsyncListener})

function Listener:on_conn(fd, event)
	print('会回调到conn', fd, event)
	local conn = Conn:new(fd)
	--conn:add2event(event)
	local x, y, z = Thread.notify_ready(self.co, conn)
	print('回调返回了,又回到epoll', x, y, z)
end

function Listener:accept()
	print('accept被调用了,加到event，切换', self.on_readable)
	event:add(self)
	self.co = coroutine.running()
	local x, y, z =  coroutine.yield()
	print('accept return:', x, y, z)
	return x, y, z
end

function main()
	print('main线程')
	local lconn = Listener:new("8888")
	while true do
		local conn = lconn:accept()
		print('accept居然真的返回了？', conn)
		Thread.new(function()
			conn:add2event(event)
			print('启动协程服务连接')
			while true do
				print('xxx运行read之前', conn)
				local data = conn:read()
				print('从client读了一条数据')
				conn:write(data)
				print('再写回给client')
			end
		end)
	end
	print '靠，居然退出了?'
end

Thread.new(main)
print("begin...", #q1, #q2)
while true do
	q1, q2 = q2, q1
	if #q1 == 0 then 
		print('没有可运行了...调回co_epoll')
		local x, y, z = coroutine.resume(co_epoll)
		print('co_epoll返回', x, y, z)
	end
	for i = 1, #q1 do
		local current = q1[i]
		local data = tls[current]
		local x, y, z
		if data then
			x, y, z = coroutine.resume(current, table.unpack(data))
		else
			x, y, z = coroutine.resume(current)
		end
		q1[i] = nil
		print('返回到调度器里', x, y, z)
	end
end
print('all finished')
