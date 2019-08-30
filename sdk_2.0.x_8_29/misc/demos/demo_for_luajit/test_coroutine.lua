
local function main1()
  print("a\n")
  coroutine.yield()
  print("b\n")
  coroutine.yield()
  print("c\n")
  coroutine.yield()
end

local function main2()
  print("a1\n")
  coroutine.yield()
  print("b2\n")
  coroutine.yield()
  print("c3\n")
  coroutine.yield()
end


local function test_coroutine()
  local co1 = coroutine.create(main1)
  local co2 = coroutine.create(main2)
  coroutine.resume(co1)
  coroutine.resume(co2)
  coroutine.resume(co1)
  coroutine.resume(co2)
end


test_coroutine()