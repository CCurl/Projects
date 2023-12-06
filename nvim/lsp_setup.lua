-- **********************************************
-- Configure language servers ...
-- **********************************************

require("lsp")
lspconfig = require("lspconfig")
--util = require("lspconfig.util")

lspconfig.clangd.setup {}

lspconfig.lua_ls.setup {}

lspconfig.angularls.setup {}
lspconfig.tsserver.setup {}
