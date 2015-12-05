local grt = require("grt")
local event = grt.event

function on_new_client(conn)
  print("on new client")
  conn:add2event(event)
  conn:read(function (p, err)
      print("packet send finish")
      if err ~= nil then
        conn:close()
        conn = nil
        return
      end

      conn:send(p, function ()
                  print("packet send finish")
                  conn:close()
                  conn = nil
      end)
  end)
end

local acceptor = grt.acceptor(8088)
acceptor:add2event(event, on_new_client)

event.loop()
