package.cpath = package.cpath .. ";../../luaclib/?.dylib"

local ltask = require "ltask"
local csocket = require "csocket"

csocket.init()

local input = ...
assert(type(input) == "number")

local command = {}
local message = {}

local sockets = {}

function command.connect(source, addr, port)
	local fd = csocket.connect(addr, port)
	if fd then
		sockets[fd] = source
		return fd
	end
end

function command.listen(source, port)
  print("llisten called", source, port)
	local fd = csocket.listen(port)
	if fd then
		sockets[fd] = source
		return fd
	end
end

function command.forward(fd, addr)
	local data = sockets[fd]
	sockets[fd] = addr
	if type(data) == "table" then
		for i=1,#data do
			local v = data[i]
			if not pcall(cell.rawsend, addr, 6, v[1], v[2], v[3]) then
				csocket.freepack(v[3])
				message.disconnect(v[1])
			end
		end
	end
end

command.send = csocket.lsend

function command.disconnect(fd)
	if sockets[fd] then
		sockets[fd] = nil
		csocket.close(fd)
	end
end

-- dispatch(ok, retch, command, args...)
local function dispatch(...)
  local arg = {...}
  if arg[1] then
    local cmdfn = command[arg[3]]
    ltask.send(arg[2], cmdfn(select(4, ...)))
  end
  return arg[1]
end

local result = {}
while true do
  for i = 1, csocket.poll(result) do
    local v = result[i]
    local c = sockets[v[1]]
    if c then
      if type(v[3]) == "string" then
        -- accept: listen fd, new fd , ip
        ltask.send(c, v[2], v[3])
      else
        -- read: fd , size , message
        ltask.send(c, v[1], v[2], v[3])
      end
    else
      csocket.freepack(v[3])
    end
  end

  repeat
    ok = dispatch(ltask.recv(input))
  until not ok
  coroutine.yield()
end
