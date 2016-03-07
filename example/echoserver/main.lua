package.cpath = package.cpath .. ";../../luaclib/?.dylib"
local ltask = require "ltask"

ltask.init(4)
ltask.task("server.lua", c)
ltask.run()
