local Stubs = require('lua/stubs')

local state = {}

local function error_scope(fn)
  local errors = {}
  local orig_errors = state.errors
  state.errors = errors
  local out = fn()
  state.errors = orig_errors

  return errors, out
end

local function any()
  return #state.errors > 0
end

local function ok()
  return (not any())
end

local function error(error_tag, ...)
  print('custom error!')
  error_obj = tag(error_tag, ...)
  table.insert(state.errors, error_obj)
  return error_obj
end

return {
  error_scope = error_scope,
  error = error,
  any = any,
  ok = ok
}
