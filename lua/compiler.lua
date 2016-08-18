local function is_tok(skel, id)
  return matches_tag(skel, 'skeleton/token', 1) and check_tok(tag_get(skel, 0), id)
end

local function compiler()
  local compile_term
  local compile_segment
  local compile_expr
  local compile_module

  local state = {
    bound_scope = List.empty
    module_context = List.empty
  }

  local function with_context(name, context, fn)
    state[name] = List.cons(context, state[name])
    fn()
    state[name] = List.tail(state[name])
    return context
  end

  local function current_module()
    return List.head(state.module_context)
  end

  local function register_names(skels)
    local names = current_module().names

    jkjk
  end

  function compile_item(item)
    local body = tag_get(item, 1)
    local module = List.head(state.module_context)
    return compile_expr(body)
  end

  function compile_expr(skels)
    if List.is_nil(skels) then error('empty expression') end

    local segments = List.split(skels, function(s) return is_tok(s, token_ids.GT) end)
    local compiled_segments = List.map(segments, compile_segment)

    return compiled_segments
  end

  function compile_module(name, skels)
    local module = { type = 'module', name = name, members = {}, names = {} }
    with_context('module_context', module, function()
      register_names(skels)
      List.each(skels, compile_item)
    end)
  end

  function compile_segment(segment)
    out = List.map(segment, compile_term)
    if List.is_nil(List.tail(out)) then return List.head(out) end

    return tag('apply', out)
  end

  function compile_term(term)
    if is_tok(term, token_ids.NAME) then
      return tag('name', tag_get(tag_get(term, 0), 1))
    else
      print('unimplemented term', inspect_value(term))
    end
  end

  return {
    compile_expr = compile_expr,
    compile_module = compile_module,
    compile_item = compile_item,
  }
end

return { compiler = compiler }
