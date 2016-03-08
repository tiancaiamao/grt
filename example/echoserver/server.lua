package.cpath = package.cpath .. ";../../luaclib/?.dylib"
package.path = package.path .. ";../../lualib/?.lua;../../service/?.lua"

local socket = require "socket"

print("before call socket.listen")

local ln = socket.listen("0.0.0.0:8888")

print("channel为空么？", ln.__ch)

print("hello world!")

while true do
  conn, addr = ln:accept()
  if conn then
    print("accept a connection", addr)
    conn:write("hello world")
    conn:close()
  end
end
