function binding_identifier(keymap, location, premods)
  stdmods = string.format("%02x", premods.stdmods)
  anymods = string.format("%02x", premods.anymods)
  return (keymap.name .. '_' .. location .. '_' .. stdmods .. anymods)
end

character_replacements = {}
character_replacements["!"]   =  '_EXCLAMATION'
character_replacements["%?"]  =  '_QUESTION'
character_replacements["@"]   =  '_ATSIGN'
character_replacements["#"]   =  '_POUND'
character_replacements["%$"]  =  '_DOLLAR'
character_replacements["%%"]   =  '_PERCENT'
character_replacements["%^"]  =  '_CARET'
character_replacements["&"]   =  '_AMPERSAND'
character_replacements["%*"]  =  '_ASTERISK'
character_replacements["%."]  =  '_PERIOD'
character_replacements["\,"]  =  '_COMMA'
character_replacements[";"]   =  '_SEMICOLON'
character_replacements["\:"] =  '_COLON'
character_replacements["="]   =  '_EQUALS'
character_replacements["%+"]  =  '_PLUS'
character_replacements["%-"]  =  '_MINUS'
character_replacements["%("]  =  '_LPAREN'
character_replacements["%)"]  =  '_RPAREN'
character_replacements["%["]  =  '_LSQUAREBRACKET'
character_replacements["%]"]  =  '_RSQUAREBRACKET'
character_replacements["%{"]  =  '_LCURLYBRACE'
character_replacements["%}"]  =  '_RCURLYBRACE'
character_replacements["\>"] =  '_GREATERTHAN'
character_replacements["\<"] =  '_LESSTHAN'
character_replacements["/"]  =  '_SLASH'
character_replacements["\~"]  =  '_TILDE'
character_replacements["\|"]  =  '_PIPE'
character_replacements["`"]   =  '_BACKTICK'
character_replacements["\\'"] =  '_APOSTROPHE'
character_replacements['\\"'] =  '_QUOTE'
character_replacements[' ']   =  '_'
character_replacements["\\"]  =  '_BACKSLASH'

function normalize_identifier(identifier)
  normalized = string.upper(identifier)
  for char,repl in pairs(character_replacements) do
    if string.find(normalized, char) ~= nil then
      normalized = string.gsub(normalized, char, repl)
    end
  end
  return normalized
end

function lookup_key(keymap, location)
  key = nil
  while key == nil do
    key = keymap.keys[location]
    if key ~= nil then break end
    if keymap.base == nil then break end
    keymap = kb.keymaps[keymap.base]
    if keymap == nil then break end
  end
  return keymap, key
end

function convert_anymods_to_stdmods(mods)
  return bit.bor(bit.band(mods, 0x00FF), bit.rshift(bit.band(mods,0x0F00), 4));
end
