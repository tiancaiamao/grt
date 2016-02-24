local ltask = require "include"
local c = ...
assert(type(c) == "number")
local taskid = ltask.taskid()

for i=1,10 do
	ltask.send(c, i)
	print(string.format("task %d send %d", taskid, i))
	coroutine.yield()
end
