local socket = require "socket"

print("before call socket.listen")

local ln = socket.listen("0.0.0.0:8888")

print("hello world!")

while true do
  conn = ln.accept()
  if conn then
    print("accept a connection")
    conn.write("hello world")
    conn.close()
  end
end
