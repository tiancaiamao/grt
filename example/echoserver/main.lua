package.cpath = package.cpath .. ";../../lib/?.dylib"

local ltask = require "ltask"

ltask.init(4)
ltask.task("server.lua",c)
ltask.run()
