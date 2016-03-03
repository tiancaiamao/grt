package.cpath = package.cpath .. ";../../lib/?.dylib"

local ltask = require "ltask"

socket_chan = ltask.channel()
ltask.task("socket_task.lua", socket_chan)

local services = {}
services["socket"] = socket_chan

local function close_command(self)
  ltask.close(self.__ch)
end
local command = {}
local command_meta = {
  __index = command,
  __gc = close_command,
}
function new_command()
  local obj = {__ch = ltask.channel()}
  return setmetatable(obj, command_meta)
end

function command:call(service_name, command, ...)
  c = services[service_name]
  assert(c)

  local retch = self.__ch
  ltask.send(c, retch, command, table.unpack({...}))

  while true do
    if ltask.select(retch) then
      return select(2, ltask.recv(retch))
    end
    coroutine.yield()
  end
end
