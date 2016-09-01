local Errors = require('lua/errors')

local function is_tok(skel, id)
  return matches_tag(skel, 'skeleton/token', 1) and check_tok(tag_get(skel, 0), id)
end

local function skel_body(skel)
  return tag_get(skel, 2)
end

local function is_nest(skel, id)
  return matches_tag(skel, 'skeleton/nested', 3) and check_tok(tag_get(skel, 0), id)
end

local function nest_body(skel) return tag_get(skel, 2) end

local function error(skel, message)
  return Errors.error('parse/compiler', skel, message)
end

local Assign = {}
local Module = {}
local Expr = {}
local Block = {}
local Scope = {}

function cache(fn)
  local state = {}

  state.impl = function()
    return val
  end

  return function() return state.impl() end
end

local function skel_val(skel)
  if not matches_tag(skel, 'skeleton/token', 1) then return nil end
  local token = tag_get(skel, 0)
  return tag_get(token, 1)
end

function Assign.new(_name, _module, _source)
  local self = {}
  self.name = _name
  self.source = _source
  self.module = _module

  self.patterns_and_body = cache(function()
    return List.split_once(_source, function(e) return is_tok(e, token_ids.EQ) end)
  end)

  self.patterns = function() local p, _ = self.patterns_and_body(); return p end
  self.body     = function() local _, b = self.patterns_and_body(); return b end

  self.compile = function()
    local body = self.compile_body()
    local patterns = self.patterns()

    if List.is_nil(patterns) then
      return body
    else
      return error('todo')
    end
  end

  self.compile_body = function()
    return Expr.new(self.module, Scope.root, self.body()).compile()
  end

  return self
end

function Expr.new(_module, _source)
  local self = {}
  self.module = module
  self.source = source

  local compile_expr
  local compile_lambda
  local compile_pattern
  local compile_term
  local compile_segment

  function apply(fn, ...)
    return tag('apply', fn, List.list({...}))
  end

  function compile_term(skel)
    if is_tok(skel, token_ids.NAME) then
      -- NOTE: name values get checked and fully
      -- module-qualified in post-processing
      return tag('name', skel_val(skel))
    elseif is_tok(skel, token_ids.INT) then
      return tag('constant', tag('int', skel_val(skel)))
    elseif is_tok(skel, token_ids.STRING) then
      return tag('constant', tag('string', skel_val(skel)))
    elseif is_tok(skel, token_ids.TAGGED) then
      return tag('tag', skel_val(skel))
    elseif is_tok(skel, token_ids.FLAG) then
      return tag('flag', skel_val(skel))
    elseif is_tok(skel, token_ids.BANG) then
      return error(skel, 'improper `!` in term position')
    elseif is_nest(skel, token_ids.LPAREN) then
      return compile_expr(skel_body(skel))
    elseif is_nest(skel, token_ids.LBRACE) then
      return compile_block(skel_body(skel))
    elseif is_nest(skel, token_ids.MACRO) then
      return error(skel, 'unexpanded macro!')
    elseif is_nest(skel, token_ids.LBRACK) then
      return compile_lambda(skel_body(skel))
    elseif matches_tag(skel, 'skeleton/item', 2) then
      -- TODO: check annotations here
      return compile_expr(tag_get(skel, 1))
    else
      return error(skel, 'unsupported form')
    end
  end

  function compile_lambda(source)
    return error('todo')
  end

  function compile_segment(is_first, segment)
    assert(List.is_cons(segment), 'TODO: gracefully handle >>')

    -- add_dash(is_first, segment)

    local code_segment = {}
    function yield(thing)
      table.insert(code_segment, thing)
    end

    while List.is_cons(segment) do
      local head = List.head(segment)
      if is_tok(head, token_ids.BANG) then
        if #code_segment == 0 then
          yield(error(head, '`!` must appear only in argument position'))
        elseif matches_tag(code_segment[1], 'tag', 1) then
          yield(error(head, '`!` can\'t be passed to a tag constructor'))
        else
          code_segment[#code_segment] = apply(code_segment[#code_segment],
                                              tag('constant', tag('bang')))
        end
      elseif is_tok(head, token_ids.FLAGKEY) then
        yield(error('TODO: flagkeys'))
      elseif is_tok(head, token_ids.DASH) then
        yield(error('TODO: dash'))
      else
        yield(compile_term(head))
      end

      segment = List.tail(segment)
    end

    return code_segment
  end

  function compile_expr(expr)
    -- TODO: implement chaining
    local terms = compile_segment(false, expr)
    if #terms == 1 then
      return terms[1]
    else
      return apply(unpack(terms))
    end
  end

  self.compile = function()
    return compile_term(_source)
  end

  return self
end


function Module.new(_name, _parent, _source)
  local self = {}

  self.name = _name
  self.parent = _parent
  self.source = _source

  self.members = cache(function()
    return List.map(_source, function(item)
      local body = tag_get(item, 1)
      local head = List.head(body)

      if is_tok(head, token_ids.ATMACRO) and skel_val(head) == 'module' then
        local new_name = skel_val(List.head(List.tail(body)))
        local module_body = tag_get(List.head(List.tail(List.tail(body))), 2)

        return Module.new(new_name, self, module_body)
      elseif is_tok(head, token_ids.NAME) then
        return Assign.new(skel_val(head), self, List.tail(body))
      else
        error('malformed module member')
      end
    end)
  end)

  self.names_set = cache(function()
    local out = {}
    List.each(self.members(), function(member)
      out['<name>'..member.name] = true
    end)
    return out
  end)

  self.has_own_name = function(name)
    return self.names_set()['<name>'..name]
  end

  self.qualpath = cache(function()
    if _parent == nil then
      return ''
    else
      return _parent.qualpath() .. '/' .. _name
    end
  end)

  self.qualify = function(name)
    if self.has_own_name(name) then
      return self.qualpath() .. '/' .. name
    elseif _parent then
      return _parent.qualify(name)
    else
      error('unbound name `' .. name .. '`')
    end
  end

  self.compile = function()
  end

  return self
end

Module.root = Module.new('', nil, List.empty)

function Scope.new(_parent)
  local self = {}
  self.names = {}
  self.bind = function(name) self.names['<name>'..name] = true end
  self.has_own_name = function(name) return self.names['<name>'..name] end
  self.has_name = function(name)
    if self.has_own_name(name) then return true
    elseif _parent then return _parent.has_own_name(name)
    else return false
    end
  end

  self.extend = function() return Scope.new(self) end
end

Scope.root = Scope.new(nil)

local function compiler()
  function compile_item(item)
    local body = tag_get(item, 1)
    local module = List.head(state.module_context)
    return compile_expr(body)
  end

  function compile_expr(skels, module)
    return Expr.new(module, skels).compile()
  end

  function compile_module(name, skels)
    local module = Module.new(name, Module.root, skels)
    return module.compile()
  end

  return {
    compile_expr = compile_expr,
    compile_module = compile_module,
    compile_item = compile_item,
  }
end

return { compiler = compiler }
