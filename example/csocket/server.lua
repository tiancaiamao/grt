--a demo use csocket and callback
package.cpath = package.cpath .. ";../../lib/?.dylib"

csocket = require "csocket"
csocket.init()

function Sleep(n)
   os.execute("sleep " .. n)
end

local listenfd = csocket.listen("0.0.0.0:8888")

--[[
  local fd = csocket.connect("")
  local sz, msg = csocket.sendpack("hello world")
  csocket.send(fd, sz, msg)
--]]

local result = {}

while true do
    for i = 1, csocket.poll(result) do
      local v = result[i]
        if type(v[3]) == "string" then
          -- accept: listen fd, new fd , ip
          print("incomming connection from:", v[3])
          csocket.send(v[2], csocket.sendpack("hello world"))
          csocket.close(v[2])
        elseif type(c) == "table" then
          -- connect
        else
          -- forward: fd , size , message
        end
    end
    Sleep(1)
end

-- test:   curl 127.0.0.1:8888
