local grt = require("grt")
local event = grt.event

local connector = grt:connector("127.0.0.1", 8088)

function on_connect(conn, err)
  if err ~= nil then
    print(err)
    connector = nil
    return
  end

  conn:add2event(event, function (p, err)
                   print("fuck")
  end)

  conn:send("hello world", function (err)
  end)

end

connector:add2event(event, on_connect)

event.loop()
