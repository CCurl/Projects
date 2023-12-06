-- **********************************************
-- Configure packer ...
-- **********************************************

local ensure_packer = function()
  local fn = vim.fn
  local install_path = fn.stdpath('data')..'/site/pack/packer/start/packer.nvim'
  if fn.empty(fn.glob(install_path)) > 0 then
    fn.system({'git', 'clone', '--depth', '1', 'https://github.com/wbthomason/packer.nvim', install_path})
    vim.cmd [[packadd packer.nvim]]
    return true
  end
  return false
end

local packer_bootstrap = ensure_packer()

require('packer').startup(function(use)
	-- Packer can manage itself
	-- Execute ":PackerSync" to sync packages
	use('wbthomason/packer.nvim')
	use('williamboman/mason.nvim')
	use('williamboman/mason-lspconfig.nvim')
	use('neovim/nvim-lspconfig')
	use('folke/tokyonight.nvim')
	use {'folke/which-key.nvim',
		config = function()
			vim.o.timeout = true
			vim.o.timeoutlen = 300
			require("which-key").setup {
			}
		end
	}
	use('hrsh7th/cmp-nvim-lsp')
	use('hrsh7th/cmp-buffer')
	use('hrsh7th/cmp-path')
	use('hrsh7th/cmp-cmdline')
	use('hrsh7th/nvim-cmp')
	-- Telescope ...
	use {
		'nvim-telescope/telescope.nvim', tag = '0.1.2',
		requires = { {'nvim-lua/plenary.nvim'} }
	}

	-- Tree-sitter
	use {
		'nvim-treesitter/nvim-treesitter',
		run = ':TSUpdate'
	}
	-- language services ...
	use('typescript-language-server/typescript-language-server')

	-- Some examples ...
	-- use ({ 'projekt0n/github-nvim-theme' })
	end
)
