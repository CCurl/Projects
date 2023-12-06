require('packer-config')
require("mason-config")
require("lsp-config")
-- require("lsp")
require("telescope-config")

vim.cmd [[colorscheme koehler]]

-- **********************************************
-- options ...
-- **********************************************
local opt = vim.opt

opt.number = true
opt.relativenumber = true
opt.ruler = false
opt.scrolloff = 8
opt.tabstop = 4
opt.softtabstop = 4
opt.shiftwidth = 4
opt.ignorecase = true

-- custom key mappings
vim.keymap.set('n', '<C-F3>', '*')
vim.keymap.set('n', '<C-S-F3>', '#')
vim.keymap.set('n', '<F3>', 'n')
vim.keymap.set('n', '<S-F3>', 'N')
vim.keymap.set('n', '<F4>', ':cn<CR>')
vim.keymap.set('n', '<S-F4>', ':cp<CR>')
vim.keymap.set('n', '<C-F4>', ':bd<CR>')
vim.keymap.set('v', '<F5>', '"+y')
vim.keymap.set('n', '<F5>', '"+yy')
vim.keymap.set('n', '<F6>', '"+p')
vim.keymap.set('i', '<F6>', '<C-R>+')
vim.keymap.set('t', '<F12>', '<C-\\><C-n>') -- TERMINAL -> Normal mode

wk = require("which-key")

wk.register({
  ["<leader>t"] = { "<cmd>Telescope<cr>", "Telescope" },
  ["<leader>f"] = { name = "+file" },
  ["<leader>fb"] = { "<cmd>Telescope buffers<cr>", "Buffers" },
  ["<leader>ff"] = { "<cmd>Telescope find_files<cr>", "Find File" },
  ["<leader>fg"] = { "<cmd>Telescope live_grep<cr>", "Live grep" },
  ["<leader>fh"] = { "<cmd>Telescope help_tags<cr>", "Help tags" },
  ["<leader>fn"] = { "<cmd>enew<cr>", "New File" },
  ["<leader>fo"] = { "<cmd>Telescope oldfiles<cr>", "Open Recent File" },
  ["<leader>fs"] = { "<cmd>Telescope grep_string<cr>", "Grep string" },
  ["vc"] = { "vawy", "Yank entire word" },
  ["vv"] = { "vaw\"0p", "Replace word" },
  ["m"] = { name = "+mine" },
  ["mn"] = { "<cmd>tabnext<cr>", "Next tab" },
  ["mr"] = { "<cmd>Telescope registers<cr>", "Registers" },
})
