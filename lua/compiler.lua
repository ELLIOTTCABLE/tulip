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
    local val = fn()
    state.impl = function()
      return val
    end
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
    local p, b = List.split_once(_source, function(e) return is_tok(e, token_ids.EQ) end)
    return { patterns = p, body = b }
  end)

  self.patterns = function() return self.patterns_and_body().patterns end
  self.body     = function() return self.patterns_and_body().body end

  self.compile = function()
    local body = self.compile_body()
    local patterns = self.patterns()

    if List.is_nil(patterns) then
      return tag('assign', _name, body)
    else
      return error('todo')
    end
  end

  self.compile_body = function()
    if self.body() == nil then
      return error(_source, 'invalid assignment')
    end

    return Expr.new(self.module, self.body()).compile()
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
    if select('#', ...) == 0 then
      return fn
    else
      return tag('apply', fn, List.list({...}))
    end
  end

  function compile_term(skel)
    if is_tok(skel, token_ids.NAME) then
      -- NOTE: name values get checked and fully
      -- module-qualified in post-processing
      return tag('unresolved-name', skel)
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

  function qualify_names(compiled_expr)
    return Scope.for_module(_module).qualify(compiled_expr)
  end

  self.compile = function()
    return qualify_names(apply(unpack(compile_segment(true, _source))))
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
        return error('malformed module member')
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

  self.qualify = function(name, name_skel)
    if self.has_own_name(name) then
      return self.qualpath() .. '/' .. name
    elseif _parent then
      return _parent.qualify(name, name_skel)
    else
      return error(name_skel, 'unbound name `' .. name .. '`')
    end
  end

  self.compile = function()
    return {
      name = self.qualpath(),
      members = List.map(self.members(), function(x) return x.compile() end)
    }
  end

  return self
end

Module.root = Module.new('', nil, List.empty)

function Scope.new(_parent, _module)
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

  self.qualify = function(expr)
    if matches_tag(expr, 'unresolved-name', 1) then
      local name_skel = tag_get(expr, 0)
      local name = skel_val(name_skel)

      if self.has_name(name) then
        return expr
      else
        return tag('name', _module.qualify(name, name_skel))
      end
    elseif matches_tag(expr, 'apply', 2) then
      local fn = tag_get(expr, 0)
      local args = tag_get(expr, 1)
      return tag('apply', self.qualify(fn), List.map(args, self.qualify))
    elseif matches_tag(expr, 'lambda', 2) then
      local arg_list = tag_get(expr, 0)
      local body = tag_get(expr, 1)

      local new_scope = self.extend()
      List.each(arg_list, function(n) new_scope.bind(n) end)
      return tag('lambda', arg_list, new_scope.qualify(body))
    elseif matches_tag(expr, 'block', 1) then
      local elements = tag_get(expr, 0)
      local new_scope = self.extend()
      List.each(elements, function(el)
        if not matches_tag(el, 'assign', 2) then return end

        new_scope.bind(tag_get(el, 0))
      end)

      return tag('block', List.map(elements, function(el)
        if matches_tag(el, 'assign', 2) then
          return tag('assign', tag_get(el, 0), new_scope.qualify(tag_get(el, 1)))
        else
          error(el, 'can\'t expand!')
        end
      end))
    else
      return expr
    end
  end

  return self
end

function Scope.for_module(module) return Scope.new(nil, module) end

local function compiler()
  function compile_item(item)
    local body = tag_get(item, 1)
    local module = List.head(state.module_context)
    return compile_expr(body)
  end

  function compile_expr(skels, module)
    return Expr.new(module, skels).compile()
  end

  function compile_root_expr(skels)
    return compile_expr(skels, Module.root)
  end

  function compile_module(name, skels)
    local module = Module.new(name, Module.root, skels)
    return module.compile()
  end

  return {
    compile_expr = compile_expr,
    compile_root_expr = compile_root_expr,
    compile_module = compile_module,
    compile_item = compile_item,
  }
end

return { compiler = compiler }
