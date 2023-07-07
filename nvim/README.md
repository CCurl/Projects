# NVIM setup guide

This isn't really a guide on how to set up nvim ... it's more of a record of what I did, so that I can hopefully replicate it on my other computers.


## Locations

- Windows: the nvim configuration file name is "init.lua"
  - On my system, it is here: C:\Users\<me>\AppData\Local\nvim\init.lua
- Linux: it is here: ~/.config/nvim/init.lua

## Install Packer. This isn't used yet, but it will be used in the future.
- Create the folder for the packer git repo:
  - Windows: "C:\Users\<me>\AppData\Local\nvim-data\site\pack\packer\start"
  - Linux: "~/.local/share/nvim/site/pack/packer/start"
- Clone https://github.com/wbthomason/packer.nvim to that folder.

The nvim initialization file (init.lua) is this:
```
--- **********************************************
-- packer ...
-- **********************************************
vim.cmd [[packadd packer.nvim]]

require('packer').startup(function(use)
  -- Packer can manage itself
  -- Execute ":PackerSync" to sync packages
  use('wbthomason/packer.nvim')

  -- Some examples ...
  -- use ({ 'projekt0n/github-nvim-theme' })
  -- use {
  --   'nvim-telescope/telescope.nvim', tag = '0.1.0',
  --   requires = { {'nvim-lua/plenary.nvim'} }
  -- }
  end
)

- **********************************************
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
vim.keymap.set('v', '<F5>', '"+y')
vim.keymap.set('v', '<F6>', '"+p')
vim.keymap.set('n', '<F5>', '"+yy')
vim.keymap.set('n', '<F6>', '"+p')
vim.keymap.set('i', '<F6>', '<C-R>+')
```
