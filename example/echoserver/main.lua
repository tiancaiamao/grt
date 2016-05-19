package.cpath = package.cpath .. ";../../lib/?.so"
local ltask = require "ltask"

ltask.init(4)
ltask.task("server.lua")
ltask.run()
