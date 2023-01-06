This isn't really a guide on how to set up nvim ... it's more of a record of what I did, so that I can hopefully replicate it on my other computers.

nvim config stuff is here on Windows: %LOCALAPPDATA%\nvim 
Note that on my system that is: C:\Users\<user>\AppData\Local\nvim

Here is how mine is setup:

lua/
  - init.lua
  - config/
    - init.lua
    - options.lua
    - packer.lua
    - telescope.lua


contents of init.lua
```
require("config")
```

contents of config/init.lua
```
require("config.packer")
require("config.telescope")
require("config.options")
require('github-theme').setup({
	theme_style = "dark_default",
	-- other config
})
```

contents of packer.lua
```
vim.cmd [[packadd packer.nvim]]

return require('packer').startup(function(use)
  -- Packer can manage itself
  use('wbthomason/packer.nvim')
  use {
    'nvim-telescope/telescope.nvim', tag = '0.1.0',
    requires = { {'nvim-lua/plenary.nvim'} }
  }
  use ({ 'projekt0n/github-nvim-theme' })
end)
```


contents of options.lua
```
local opt = vim.opt

opt.number = true
opt.relativenumber = true
opt.ruler = false
opt.scrolloff = 8
```


contents of telescope.lua
```
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
```
