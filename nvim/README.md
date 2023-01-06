# NVIM setup guide

This isn't really a guide on how to set up nvim ... it's more of a record of what I did, so that I can hopefully replicate it on my other computers.

nvim config stuff is here on Windows: %LOCALAPPDATA%\nvim 
Note that on my system that is: C:\Users\<user>\AppData\Local\nvim

Here is how mine is setup:

## There are 2 files:

- lua/init.lua
- lua/config/init.lua

### contents of lua/init.lua:
```
require("config")
```

### contents of lua/config/init.lua:
```
-- **********************************************
-- packer ...
-- **********************************************
vim.cmd [[packadd packer.nvim]]

require('packer').startup(function(use)
  -- Packer can manage itself
  use('wbthomason/packer.nvim')
  use ({ 'projekt0n/github-nvim-theme' })
  use {
    'nvim-telescope/telescope.nvim', tag = '0.1.0',
    requires = { {'nvim-lua/plenary.nvim'} }
  }
end)


-- **********************************************
-- color scheme/theme ...
-- **********************************************
require('github-theme').setup({
	theme_style = "dark_default",
	-- other config
})


-- **********************************************
-- telescope ...
-- **********************************************
local builtin = require('telescope.builtin')
vim.g.mapleader = ' '
vim.keymap.set('n', '<leader>ff', builtin.find_files, {})
vim.keymap.set('n', '<leader>fg', builtin.live_grep, {})
vim.keymap.set('n', '<leader>fb', builtin.buffers, {})
vim.keymap.set('n', '<leader>fh', builtin.help_tags, {})

require('telescope').setup{
  defaults = {
    -- Default configuration for telescope goes here:
    -- config_key = value,
    mappings = {
      i = {
        -- map actions.which_key to <C-h> (default: <C-/>)
        -- actions.which_key shows the mappings for your picker,
        -- e.g. git_{create, delete, ...}_branch for the git_branches picker
        ["<C-h>"] = "which_key"
      }
    }
  },
  pickers = {
    -- Default configuration for builtin pickers goes here:
    -- picker_name = {
    --   picker_config_key = value,
    --   ...
    -- }
    -- Now the picker_config_key will be applied every time you call this
    -- builtin picker
  },
  extensions = {
    -- Your extension configuration goes here:
    -- extension_name = {
    --   extension_config_key = value,
    -- }
    -- please take a look at the readme of the extension you want to configure
  }
}


-- **********************************************
-- other options ...
-- **********************************************
local opt = vim.opt

opt.number = true
opt.relativenumber = true
opt.ruler = false
opt.scrolloff = 8
```
