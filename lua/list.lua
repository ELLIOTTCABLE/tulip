local Stubs = require 'lua/stubs'

local empty = tag('nil')
local tag = Stubs.tag

local function cons(head, tail)
  return tag('cons', head, tail)
end

local function is_cons(list)
  return Stubs.matches_tag(list, 'cons', 2)
end

local function is_nil(list)
  return Stubs.matches_tag(list, 'nil', 0)
end

local function head(list)
  return tag_get(list, 0)
end

local function tail(list)
  return tag_get(list, 1)
end

local function is_singleton(list)
  return is_cons(list) and is_nil(tail(list))
end

local function list(tbl)
  local out = empty

  for i = #tbl,1,-1 do
    out = cons(tbl[i], out)
  end

  return out
end

local function each(list, fn)
  while matches_tag(list, 'cons', 2) do
    fn(head(list))
    list = tail(list)
  end
end

local function map_reverse(list, fn)
  local out = empty

  each(list, function(el)
    out = cons(fn(el), out)
  end)

  return out
end

local function foldl(list, accum, fn)
  if is_nil(list) then return accum end

  return foldl(tail(list), fn(accum, head(list)), fn)
end

local function foldr(list, init, fn)
  if is_nil(list) then return init end

  return fn(head(list), foldr(tail(list), init, fn))
end

local function reverse(list)
  return map_reverse(list, function(x) return x end)
end

local function map(list, fn)
  return reverse(map_reverse(list, fn))
end

local function join(list, join_str)
  if is_nil(list) then return '' end

  local out = head(list)

  each(tail(list), function(el)
    out = out .. join_str .. el
  end)

  return out
end

local function each_slice(list, pred, fn)
  local last = empty

  each(list, function(el)
    if pred(el) then
      fn(reverse(last))
      last = empty
    else
      last = cons(el, last)
    end
  end)

  fn(last)
end

local function split(list, pred)
  local out = empty

  each_slice(list, pred, function(slice)
    out = cons(slice, out)
  end)

  return reverse(out)
end

function split_once(list, pred)
  local out = empty

  while is_cons(list) do
    local h, t = head(list), tail(list)
    if pred(h) then
      return reverse(out), list
    else
      out = cons(h, out)
      list = t
    end
  end

  return nil, nil
end

local function size(list)
  local count = 0
  each(list, function(_) count = count + 1 end)
  return count
end

local function find_map(list, pred)
  while not is_nil(list) do
    local out = pred(head(list))
    if out then return out end
    list = tail(list)
  end
end

local function find(list, pred)
  return find_map(list, function(e) if pred(e) then return e end end)
end

Stubs.impl_inspect_tag('nil', 0, function() return '\\list()' end)
Stubs.impl_inspect_tag('cons', 2, function(head, tail)
  local inspects = map(cons(head, tail), Stubs.inspect_value)

  return '\\list(' .. join(inspects, ' ') .. ')'
end)

_G.List = {
  list = list,
  map = map,
  find_map = find_map,
  reverse = reverse,
  empty = empty,
  head = head,
  tail = tail,
  each = each,
  is_cons = is_cons,
  is_nil = is_nil,
  join = join,
  each_slice = each_slice,
  split = split,
  size = size,
  foldl = foldl,
  foldr = foldr,
  is_singleton = is_singleton,
}

return List
