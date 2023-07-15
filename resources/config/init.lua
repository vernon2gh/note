-- base setting
vim.o.number = true
vim.o.mouse = ""
vim.cmd([[autocmd BufWritePre * %s/\s\+$//e]])

-- plugins setting
vim.diagnostic.disable()
require'lspconfig'.clangd.setup{}

-- plugins install
vim.cmd [[packadd packer.nvim]]

return require('packer').startup(function(use)
	use 'wbthomason/packer.nvim'
	use 'neovim/nvim-lspconfig'
end)
