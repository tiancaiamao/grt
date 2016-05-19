local http = require "http"

function index_handler(req)
    return "hello world"
end

function markdown_handler(req)
    return "xxx"
end

local router = {
    index = index_handler,
    md = markdown_handler,
}

function handler(req)
    local xxx = router[req.Head[URL]]
    if xxx then
        return xxx(req)
    end
    print('默认的handle')
    return "main handler\n"
end

http.serve("8080", handler)
