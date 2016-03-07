package.cpath = package.cpath .. ";../luaclib/?.dylib"
local system = require "csystem"
local ltask = require "ltask"

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

function command:call(ch, command, ...)
  local retch = self.__ch
  ltask.send(ch, retch, command, table.unpack({...}))

  while true do
    if ltask.select(retch) then
      return select(2, ltask.recv(retch))
    end
    coroutine.yield()
  end
end
