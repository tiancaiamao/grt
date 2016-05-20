function new_object(method, data)
  local o = data or {}
  method.__index = method
  setmetatable(o, method)
  return o
end
