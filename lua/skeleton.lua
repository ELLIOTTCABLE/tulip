local Stubs = require('lua/stubs')
local Lexer = require('lua/lexer')
local Errors = require('lua/errors')
local List = require('lua/list')

token_ids   = Lexer.token_ids
token_names = Lexer.token_names

eats_preceding_newline = {
  [token_ids.GT]       = true,
  [token_ids.RARROW]   = true,
  [token_ids.EQ]       = true,
  [token_ids.COMMA]    = true,
  [token_ids.PIPE]     = true,
  [token_ids.QUESTION] = true,
  [token_ids.RBRACK]   = true,
  [token_ids.RBRACE]   = true,
}

function parse_skeleton(lexer)
  lexer.setup()
  local parsed = _parse_sequence(lexer, nil, 0)
  lexer.teardown()
  return parsed
end

function is_closing(tok)
  return check_tok(tok, token_ids.RBRACK) or
         check_tok(tok, token_ids.RBRACE) or
         check_tok(tok, token_ids.RPAREN)
end

function unexpected(token, matching, message)
  print('unexpected', token, message)
  return tag('error', Errors.error('parse/unexpected', token, matching, message))
end

function unmatched(token, message)
  print('unmatched', token, message)
  return tag('error', Errors.error('parse/unmatched', token, message))
end

local function inspect_items(items)
  local inners = List.map(items, inspect_value)
  return List.join(inners, ' ')
end

Stubs.impl_inspect_tag('skeleton/nested', 3, function(open, close, body)
  local body_ = inspect_items(body)

  return '\\skel[' .. inspect_value(open) .. ': ' .. body_ .. ' :' .. inspect_value(close) .. ']'
end)

Stubs.impl_inspect_tag('skeleton/token', 1, function(tok)
  return inspect_value(tok)
end)

Stubs.impl_inspect_tag('skeleton/item', 2, function(annotations, body)
  local anns_ = List.join(List.map(annotations, inspect_value), '; ')
  local body_ = List.join(List.map(body, inspect_value), ' ')
  return '(' .. anns_ .. ':' .. body_ .. ':)'
end)

function _parse_sequence(lexer, open_tok, expected_close_id)
  local elements = {}
  local items = {}
  local current_annotation = nil
  local annotations = {}

  local function flush_item()
    if expected_close_id == token_ids.RPAREN then return end
    if #elements == 0 then return end

    table.insert(items, tag('skeleton/item', List.list(annotations), List.list(elements)))
    elements = {}
    annotations = {}
  end

  local function flush_annotation()
    if current_annotation then
      table.insert(annotations, tag('skeleton/annotation', List.list(elements)))

      elements = {}
    end
  end

  local function final_items()
    if expected_close_id == token_ids.RPAREN then
      return List.list(elements)
    else
      flush_item()

      return List.list(items)
    end
  end

  while true do
    local tok = lexer.next()

    if not tok then
      return -- the lexer had an error
    elseif check_tok(tok, token_ids.EOF) then
      if open_tok then
        return unmatched(open_tok, token_names[expected_close_id])
      else
        return final_items()
      end
    elseif open_tok and check_tok(tok, expected_close_id) then
      return tag('skeleton/nested', open_tok, tok, final_items())
    elseif is_closing(tok) then
      if open_tok then
        return unexpected(tok, tag('some', open_tok), 'invalid nesting')
      else
        return unexpected(tok, tag('none'), 'invalid nesting')
      end
    elseif check_tok(tok, token_ids.LT) then
      table.insert(elements, _parse_sequence(lexer, tok, token_ids.GT))
    elseif check_tok(tok, token_ids.LPAREN) then
      table.insert(elements, _parse_sequence(lexer, tok, token_ids.RPAREN))
    elseif check_tok(tok, token_ids.LBRACK) then
      table.insert(elements, _parse_sequence(lexer, tok, token_ids.RBRACK))
    elseif check_tok(tok, token_ids.MACRO) then
      local open = lexer.next()
      if check_tok(open, token_ids.LPAREN) then
        table.insert(elements, _parse_sequence(lexer, tok, token_ids.RPAREN))
      elseif check_tok(open, token_ids.LBRACK) then
        table.insert(elements, _parse_sequence(lexer, tok, token_ids.RBRACK))
      else
        return unexpected(open, tag('some', tok), 'invalid macro opening')
      end
    elseif check_tok(tok, token_ids.LBRACE) then
      table.insert(elements, _parse_sequence(lexer, tok, token_ids.RBRACE))
    elseif check_tok(tok, token_ids.ANNOT) or check_tok(tok, token_ids.PANNOT) then
      flush_annotation()

      current_annotation = elements
      table.insert(current_annotation, tag('skeleton/token', tok))
    elseif check_tok(tok, token_ids.NL) then
      flush_annotation()
      flush_item()
    elseif check_tok(tok, token_ids.NL) and eats_preceding_newline[tokid(lexer.peek())] then
      -- pass
    else
      table.insert(elements, tag('skeleton/token', tok))

      if check_tok(tok, token_ids.GT) then
        -- manually skip NL tokens here, since the lexer can't for <...>
        while check_tok(lexer.peek(), token_ids.NL) do lexer.next() end
      end
    end
  end
end

return {
  parse_skeleton = parse_skeleton,
  inspect = inspect_skeletons
}
