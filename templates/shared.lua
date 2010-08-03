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

function modifier_symbol_from_name(mod_name, convertanymods)
  convertanymods = convertanymods or false
  mod_name = string.lower(mod_name)
  if mod_name == "" then               return 0 end
  if mod_name == "left_alt" then       return "L_ALT" end
  if mod_name == "left_control" then   return "L_CTL" end
  if mod_name == "left_shift" then     return "L_SHF" end
  if mod_name == "left_gui" then       return "L_GUI" end
  if mod_name == "right_alt" then      return "R_ALT" end
  if mod_name == "right_control" then  return "R_CTL" end
  if mod_name == "right_shift" then    return "R_SHF" end
  if mod_name == "right_gui" then      return "R_GUI" end
  if mod_name == "alt" or 
     mod_name == "control" or 
     mod_name == "shift" or 
     mod_name == "gui" then
    if convertanymods then
      if mod_name == "alt" then        return "L_ALT" end
      if mod_name == "control" then    return "L_CTL" end
      if mod_name == "shift" then      return "L_SHF" end
      if mod_name == "gui" then        return "L_GUI" end
    else
      if mod_name == "alt" then        return "A_ALT" end
      if mod_name == "control" then    return "A_CTL" end
      if mod_name == "shift" then      return "A_SHF" end
      if mod_name == "gui" then        return "A_GUI" end
    end
  end
  error("Unknown modifier " ..mod_name)
end
