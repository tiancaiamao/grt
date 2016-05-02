Thread = require "Thread"

local save

Thread.new(function() 
	for i = 1,10 do
		print(i)
		if i == 10 then
			save = nil
		else
			save = Thread.running()
		end
		Thread.block_switch()
	end
end)

Thread.new(function()
	repeat do
		print('notify ready')
		Thread.notify_ready(save)
	end until save==nil
end)

Thread.big_bang()
