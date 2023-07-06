# NVIM setup guide

This isn't really a guide on how to set up nvim ... it's more of a record of what I did, so that I can hopefully replicate it on my other computers.

The nvim initialization file is this:
```
-- **********************************************
-- options ...
-- **********************************************
local opt = vim.opt

opt.number = true
opt.relativenumber = true
opt.ruler = false
opt.scrolloff = 8

-- custom key mappings
vim.keymap.set('n', '<F4>', ':cn<CR>')
vim.keymap.set('n', '<C-F4>', ':bd<CR>')
vim.keymap.set('v', '<C-F5>', '"+y')
vim.keymap.set('v', '<C-F6>', '"+p')
vim.keymap.set('n', '<C-F5>', '"+yy')
vim.keymap.set('n', '<C-F6>', '"+p')
```

## Locations

### Windows
On Windows, the nvim configuration goes here: %LOCALAPPDATA%\nvim\init.lua
- On my system, that is: C:\Users\CCurl\AppData\Local\nvim\init.lua

### Linux
On Linux, it is here: ~/.config/nvim/init.lua
