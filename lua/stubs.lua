-- TODO: pass this into the compile function as a
-- file or string reader
function string_reader(input_name, input)
  local state = { host_reader = nil }

  return {
    setup = function()
      state.host_reader = _G.__compiler_reader_setup(input, #input)
    end,

    teardown = function()
      _G.__compiler_reader_teardown(state.host_reader)
    end,

    next = function()
      return _G.__compiler_reader_next(state.host_reader)
    end,

    input_name = function()
      return input_name
    end,
  }
end

-- TODO implement in C
function tag(name, ...)
  return { tag = name, values = {...} }
end

-- TODO implement in C
function tag_get(obj, index)
  return obj.values[index+1]
end

local tag_inspectors = {}

function tag_key(name, arity) return name .. '@' .. tostring(arity) end

function impl_inspect_tag(name, arity, impl)
  tag_inspectors[tag_key(name, arity)] = impl
end

function inspect_tag(t)
  local dyn_impl = tag_inspectors[tag_key(t.tag, #t.values)]
  if dyn_impl then return dyn_impl(unpack(t.values)) end

  local out = '(.' .. t.tag

  for _,v in pairs(t.values) do
    out = out .. ' ' .. inspect_value(v)
  end

  return out .. ')'
end

-- TODO implement in C
function matches_tag(t, name, arity)
  if not (type(t) == 'table') then return false end
  if not (t.tag == name) then return false end
  if not (#t.values == arity) then return false end
  return true
end

function inspect_table(t)
  local out = '{'
  local first = true
  for k, v in pairs(t) do
    if first then
      first = false
    else
      out = out .. ', '
    end

    out = out .. '[' .. inspect_value(k) .. '] = ' .. inspect_value(v)
  end

  out = out .. '}'

  return out
end

-- TODO implement in C
function inspect_value(t)
  if type(t) == 'table' and t.tag then return inspect_tag(t)
  elseif type(t) == 'table' and t.tokid then return inspect_token(t)
  elseif type(t) == 'string' then return '"' .. t .. '"'
  elseif type(t) == 'table' then return inspect_table(t)
  else
    return tostring(t)
  end
end

function Token(id, value, range)
  if range ~= '<synthetic>' then
    range = tag('range', range.start, range.final)
  end

  return tag('tok', id, value, range)
end

function tokid(tok)
  if not matches_tag(tok, 'tok', 3) then return end

  return tag_get(tok, 0)
end

function tokvalue(tok)
  if not matches_tag(tok, 'tok', 3) then return end

  return tag_get(tok, 1)
end

function tokrange(tok)
  if not matches_tag(tok, 'tok', 3) then return end

  return tag_get(tok, 2)
end

function check_tok(tok, type)
  return tokid(tok) == type
end

function inspect_loc(loc)
  local line = tag_get(loc, 2)
  local column = tag_get(loc, 3)
  return line .. ':' .. column
end

function inspect_range(range)
  if range == '<synthetic>' then return '' end

  local start = tag_get(range, 0)
  local final = tag_get(range, 1)

  local input = tag_get(start, 0)

  return '<' .. input .. ':' ..
                inspect_loc(start) .. '-' ..
                inspect_loc(final) .. '>'
end

function inspect_token(id, value, range)
  local name = token_names[id]

  range = inspect_range(range)

  local raw = nil

  if value then
    raw = name .. '(' .. value .. ')'
  else
    raw = name
  end

  return raw -- .. '@' .. range
end

impl_inspect_tag('tok', 3, inspect_token)

return {
  string_reader = string_reader,
  inspect_skeletons = inspect_skeletons,
  inspect_value = inspect_value,
  matches_tag = matches_tag,
  tag = tag,
  tag_get = tag_get,
  Token = Token,
  impl_inspect_tag = impl_inspect_tag,
}
