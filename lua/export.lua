local Stubs = require 'lua/stubs'
local Lexer = require 'lua/lexer'
local Skeleton = require 'lua/skeleton'
local Errors = require 'lua/errors'
local Macros = require 'lua/macros'
local Compiler = require 'lua/compiler'

function compile(reader)
  local lexer = Lexer.new(reader)
  local out = {}
  local compiler = Compiler.compiler()

  local errors, _ = Errors.error_scope(function()
    out.skel = parse_skeleton(lexer)
    if Errors.ok() then out.expanded = Macros.macro_expand(out.skel) end
    if Errors.ok() then out.compiled = compiler.compile_root_expr(List.head(out.expanded)) end
  end)

  return errors, out
end

local function compile_module(reader)
  local lexer = Lexer.new(reader)
  local out = {}
  local compiler = Compiler.compiler()

  local errors, _ = Errors.error_scope(function()
    out.skel = parse_skeleton(lexer)
    if Errors.ok() then out.expanded = Macros.macro_expand(out.skel) end
    if Errors.ok() then out.compiled = compiler.compile_module('testy', out.expanded) end
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
      print('compiled: ' .. Stubs.inspect_value(out.compiled))
    else
      for _,e in pairs(errors) do
        print('error: ' .. Stubs.inspect_value(e))
      end
    end
  end
end

local function test_file()
  -- these should all compile correctly
  -- local input = "@module Foo [ bar = 1; baz = 2; zot = bar > baz ]"
  local input = "foo = 1; @module Bar [ baz = foo ]"

  local reader = Stubs.string_reader('input.tlp', input)

  print('compiling: ', inspect_value(input))
  local errors, out = compile_module(reader)

  if #errors == 0 then
    print('parsed: ' .. Stubs.inspect_value(out.skel))
    print('expanded: ' .. Stubs.inspect_value(out.expanded))
    print('compiled: ' .. Stubs.inspect_value(out.compiled.members))
  else
    for _,e in pairs(errors) do
      print('error: ' .. Stubs.inspect_value(e))
    end
  end
end

_G.init = function()
  test_file()
end

return {
  compile = compile
}
