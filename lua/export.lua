local Stubs = require 'lua/stubs'
local Lexer = require 'lua/lexer'
local Skeleton = require 'lua/skeleton'
local Errors = require 'lua/errors'
local Macros = require 'lua/macros'

local function compile(reader)
  local lexer = Lexer.new(reader)
  local out = {}
  local errors, _ = Errors.error_scope(function()
    out.skel = parse_skeleton(lexer)
    out.expanded = Macros.macro_expand(out.skel)
  end)

  return errors, out
end

local function repl()
  local state = { line = 0 }
  while true do
    state.line = state.line + 1
    io.stdout:write('> ')
    local input = io.stdin:read()

    if not input then break end

    local reader = Stubs.string_reader('<repl:' .. state.line .. '>', input)

    local errors, out = compile(reader)

    if #errors == 0 then
      print('parsed: ' .. Stubs.inspect_value(out.skel))
      print('expanded: ' .. Stubs.inspect_value(out.expanded))
    else
      for _,e in pairs(errors) do
        print('error: ' .. Stubs.inspect_value(e))
      end
    end
  end
end

_G.init = function()
  repl()
end

return {
  compile = compile
}
