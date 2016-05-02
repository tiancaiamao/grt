local running = nil
-- q1 q2 is runnable queue
local q1 = {}
local q2 = {}

local Thread = {}
function Thread.new(f)
	local obj = {
		co = coroutine.create(f),
		data = nil
	}
	table.insert(q2, obj)
end

function Thread.block_switch()
	assert(running.co == coroutine.running())
	local co = running.co
	local x, y, z = coroutine.yield()
	print('block_switch yield returned:', x, y, z)
	return y
end

function Thread.notify_ready(t, ...)
	t.data = table.pack(...)
	table.insert(q2, t)
	Thread.yield()
end

function Thread.yield()
	assert(running.co == coroutine.running())
	table.insert(q2, running)
	coroutine.yield()
end

function Thread.running()
	assert(coroutine.running() == running.co)
	return running
end

Thread.new(function()
	print('epoll线程')
	event:run()
end)

function Thread.big_bang()
	while true do
		q1, q2 = q2, q1
		if #q1 == 0 then 
			coroutine.resume(co_epoll)
		end
		for i = 1, #q1 do
			running = q1[i]
			local data = running.data
			local x, y, z
			if data then
				x, y, z = coroutine.resume(running.co, table.unpack(data))
			else
				x, y, z = coroutine.resume(running.co)
			end
			q1[i] = nil
			print('返回到调度器里', x, y, z)
		end
	end
	print('all finished')
end

return Thread 
