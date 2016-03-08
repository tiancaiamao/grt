package.cpath = package.cpath .. ";../luaclib/?.dylib"
local csocket = require "csocket"
local csystem = require "csystem"
local ltask = require "ltask"

require "system"

local M = {
  __command = new_command()
}

local socket_chan = csystem.get(csystem.SOCKET)
if not socket_chan then
  local filename = package.searchpath("socket_service", package.path)
  socket_chan = ltask.channel()
  ltask.task(filename, socket_chan)
  csystem.set(csystem.SOCKET, socket_chan)
end

local sockets = {}
local sockets_event = {}
local sockets_arg = {}
local sockets_closed = {}
local sockets_accept = {}

local socket = {}
local listen_socket = {}

function socket:close()
	local fd = self.__fd
  if sockets_closed[fd] then
    return
  end
	sockets[fd] = nil
	sockets_closed[fd] = true
  local cmd = self.__command
  cmd:call(socket_chan, "disconnect", fd)
end

local socket_meta = {
	__index = socket,
	__gc = socket.close,
	__tostring = function(self)
		return "[socket: " .. self.__fd .. "]"
	end,
}

local listen_meta = {
	__index = listen_socket,
	__gc = close_msg,
	__tostring = function(self)
		return "[socket listen: " .. self.__fd .. "]"
	end,
}


--todo:
function listen_socket:disconnect()
	sockets_accept[self.__fd] = nil
	socket.disconnect(self)
end

function M.connect(addr, port)
  print("in socket.connect")
  local cmd = M.__command
  local ch = ltask.channel()
	local obj = {
    __fd = assert(cmd:call(socket_chan, "connect", ch, addr, port), "Connect failed"),
    __ch = ch,
  }
	return setmetatable(obj, socket_meta)
end

function M.listen(port)
  local cmd = M.__command
  local ch = ltask.channel()
  local obj = {
    __fd = assert(cmd:call(socket_chan, "listen", ch, port), "Listen failed"),
    __ch = ch,
  }
  return setmetatable(obj, listen_meta)
end

function listen_socket:accept()
  while true do
    if ltask.select(self.__ch) then
      local ok, fd, addr = ltask.recv(self.__ch)
      if ok then
        local obj = {
          __fd = fd,
          __ch = ltask.channel(),
          __command = new_command(),
        }
        return setmetatable(obj, socket_meta), addr
      end
    end
    coroutine.yield()
  end
end

function socket:write(msg)
	local fd = self.__fd
  local cmd = self.__command
  cmd:call(socket_chan, "send", fd, csocket.sendpack(msg))
end

local function socket_wait(sock, sep)
  local fd = sock.__fd
  local ch = sock.__ch
  while true do
    if ltask.select(ch) then
      local ok, fd, sz, msg = ltask.recv(ch)
      local buffer, bsz = csocket.push(sockets[fd], msg, sz)
      sockets[fd] = buffer
      break
    end
    coroutine.yield()
  end
end

function socket:readbytes(bytes)
	local fd = self.__fd
	if sockets_closed[fd] then
		sockets[fd] = nil
		return
	end
	if sockets[fd] then
		local data = csocket.pop(sockets[fd], bytes)
		if data then
			return data
		end
	end
	socket_wait(self, bytes)
	if sockets_closed[fd] then
		sockets[fd] = nil
		return
	end
	return csocket.pop(sockets[fd], bytes)
end

function socket:readline(sep)
	local fd = self.__fd
	if sockets_closed[fd] then
		sockets[fd] = nil
		return
	end
	sep = sep or "\n"
	if sockets[fd] then
		local line = csocket.readline(sockets[fd], sep)
		if line then
			return line
		end
	end
	socket_wait(self, sep)
	if sockets_closed[fd] then
		sockets[fd] = nil
		return
	end
	return csocket.readline(sockets[fd], sep)
end

return M
