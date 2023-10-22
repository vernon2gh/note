-- base setting
vim.o.number = true
vim.o.mouse = ""
vim.wo.colorcolumn = "80"
vim.cmd [[ autocmd BufWritePre * %s/\s\+$//e ]]
vim.keymap.set('n', 'i', '<up>', opts)
vim.keymap.set('n', 'k', '<down>', opts)
vim.keymap.set('n', 'j', '<left>', opts)
vim.keymap.set('n', 'l', '<right>', opts)
vim.keymap.set('n', 'h', '<insert>', opts)
vim.keymap.set('v', 'i', '<up>', opts)
vim.keymap.set('v', 'k', '<down>', opts)
vim.keymap.set('v', 'j', '<left>', opts)
vim.keymap.set('v', 'l', '<right>', opts)
vim.keymap.set('v', 'h', '<insert>', opts)

-- plugins setting
require'lspconfig'.clangd.setup { }
vim.diagnostic.disable()
vim.keymap.set('n', 'gr', vim.lsp.buf.references, opts)
vim.keymap.set('n', 'fg', ':!git grep <C-R>=expand("<cword>")<CR><CR>', opts)
require('gitsigns').setup {
	current_line_blame = true,
}
vim.cmd [[ set background=dark ]]
vim.cmd [[ colorscheme sonokai ]]

-- plugins install
vim.cmd [[packadd packer.nvim]]
return require('packer').startup(function(use)
	use 'wbthomason/packer.nvim'
	use 'neovim/nvim-lspconfig'
	use 'lewis6991/gitsigns.nvim'
	use 'sainnhe/sonokai'
end)
