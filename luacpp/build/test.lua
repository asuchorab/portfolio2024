local a, b
local c, d = -4*2, -4^2
a = function(...)
  if ({...})[0] > 1 then
    print("asdf" .. 1 + 2)
  end
end
b = {['p'] = 3, r = 4, 1; 2, 3}
b.a = {}
function b.a:a()
  print(type(self) == 'table')
end
b.a:a()
b["a"]:a()
while true do
  print(d)
  if d < 2 then
    break
  end
end

::asdf::

goto asdf
