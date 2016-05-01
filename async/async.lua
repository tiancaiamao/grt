local socket = require "c.socket"
local epoll = require "c.epoll"

local M = {}

local EventLoop = {}
function EventLoop:new()
	local obj = {
		handlers = {},
		epollfd = epoll.create(),
	}
	self.__index = self
	return setmetatable(obj, self)
end

function EventLoop:add(handler)
	self.handlers[handler.fd] = handler
	epoll.ctl(self.epollfd, epoll.CTL_ADD, handler.fd)
end

function EventLoop:del(handler)
	self.handlers[handler.fd] = nil
	epoll.ctl(self.epollfd, epoll.CTL_DEL, handler.fd)
end

function EventLoop:run() 
	local result = {}
	while true do
		for i = 1, epoll.wait(self.epollfd, result) do
			local v = result[i]
			local handler = self.handlers[v.fd]
			local mask = v.mask
			if mask & epoll.MASK_READ then
				handler:on_readable(self)
			end
			if mask & epoll.MASK_WRITE then
				handler:on_writeable(self)
			end
		end
	end
end

local Listener = {}
function Listener:new(addr)
	fd, err = socket.listen(addr)
	if err then
		print(err)
		return nil
	end
	local obj = {
		fd = fd,
		addr = addr,
	}
	self.__index = self
	return setmetatable(obj, self)
end
function Listener:on_writeable(eventloop) end
function Listener:on_error(eventloop) end
function Listener:on_readable(eventloop)
	local fd, err = socket.accept(self.fd)
	if not err then 
		self:on_conn(fd, eventloop)
	end
end
-- Listener instance should overload it
function Listener:on_conn(fd, event) 
	print('accept a new connection fd=', fd)
end

local BufferQueue = {}

function BufferQueue:new()
	local list = {
		head = 1,
		tail = 1,
		-- available data in [head, tail) 
		deque = {},
		enque = {},
	}
	self.__index = self
	return setmetatable(list, self)
end

function BufferQueue:push(x)
	table.insert(self.enque, x)
end

function BufferQueue:pop()
	if self.head == self.tail then
		self.head = 1
		self.deque, self.enque = self.enque, self.deque
		self.tail = #self.deque+1 
	end

	if self.head < self.tail then
		local ret = self.deque[self.head]
		self.deque[self.head] = nil
		self.head = self.head + 1
		return ret
	end
end

function BufferQueue:push_back(x)
	self.head = self.head - 1
	self.deque[self.head] = x
end

local Conn = {}
function Conn:new(conf)
	conf.buffer = BufferQueue:new()
	self.__index = self
	return setmetatable(conf, self)
end
function Conn:add2event(el)
	self.eventloop = el
	el:add(self)
end
function Conn:on_error(eventloop) 
--	eventloop.del(self)
--	socket.close(self.fd) 
end
function Conn:on_readable(eventloop) 
	local data = socket.read(self.fd)
	self:on_read(data)
end
function Conn:write(data)
	self.buffer:push(data)
	local epollfd = self.eventloop.epollfd
	epoll.ctl(epollfd, epoll.CTL_WRITE, self.fd, true)
end
function Conn:on_writeable(eventloop)
	local data = self.buffer:pop()
	while data do
		local len = socket.write(self.fd, data)
		if len < string.len(data) then
			local rem = string.sub(data)
			self.buffer:push_back(rem)
			return
		end

		data = self.buffer:pop()
	end
	epoll.ctl(eventloop.epollfd, epoll.CTL_WRITE, self.fd, false)
end

M.EventLoop = EventLoop
M.Listener = Listener
M.Connection = Conn

return M
