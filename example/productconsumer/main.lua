local ltask = require "include"

ltask.init(4)
ltask.task("hello.lua", "Hello", "world")
local c = ltask.channel()
ltask.task("producer.lua",c)
ltask.task("consumer.lua",c)
ltask.run()
