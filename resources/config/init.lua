-- base setting
vim.o.number = true
vim.o.mouse = ""
vim.wo.colorcolumn = "80"
vim.cmd [[ autocmd BufWritePre * %s/\s\+$//e ]]
vim.keymap.set({'n', 'v'}, 'i', '<up>', opts)
vim.keymap.set({'n', 'v'}, 'k', '<down>', opts)
vim.keymap.set({'n', 'v'}, 'j', '<left>', opts)
vim.keymap.set({'n', 'v'}, 'l', '<right>', opts)
vim.keymap.set({'n', 'v'}, 'h', '<insert>', opts)
vim.keymap.set({'n', 'v'}, ';i', '30<up>', opts)
vim.keymap.set({'n', 'v'}, ';k', '30<down>', opts)
vim.keymap.set({'n', 'v'}, '<C-i>', '<pageup>', opts)
vim.keymap.set({'n', 'v'}, '<C-k>', '<pagedown>', opts)
vim.keymap.set({'n', 'v'}, '<C-j>', 'b', opts)
vim.keymap.set({'n', 'v'}, '<C-l>', 'w', opts)

-- plugins setting
require('nvim-treesitter.configs').setup {
	highlight = {
		enable = true,
		disable = { "c" },
		additional_vim_regex_highlighting = false,
	},
}

require('lspconfig').clangd.setup { }
vim.diagnostic.disable()
vim.keymap.set('n', ';d', vim.lsp.buf.definition, opts)
vim.keymap.set('n', ';r', vim.lsp.buf.references, opts)
vim.keymap.set('n', ';c', vim.lsp.buf.incoming_calls, opts)
vim.keymap.set('n', ';gg', ':!git grep -w <C-R>=expand("<cword>")<CR><CR>', opts)

require('gitsigns').setup {
	current_line_blame = true,
	current_line_blame_formatter = '<abbrev_sha>, <author>, <author_time:%Y/%m/%d> - <summary>',
}

require('diffview').setup {
	use_icons = false,
	view = {
		merge_tool = {
			layout = "diff4_mixed",
		},
	},
	file_panel = {
		listing_style = "tree",
		win_config = {
			width = 0,
		},
	},
}
vim.keymap.set('n', ';gb', ':.,.DiffviewFileHistory<CR>', opts)
vim.keymap.set('n', ';gm', ':DiffviewOpen<CR>', opts)

require('render-markdown').setup {
	heading = {
		enabled = true,
		position = 'inline',
		width = 'block',
	},
}

require("codecompanion").setup {
	strategies = {
		chat = {
			adapter = "ollama",
		},
		inline = {
			adapter = "ollama",
		},
	},
}
vim.keymap.set({'n', 'v'}, ';w', ':CodeCompanionChat Toggle<CR>', { silent = true })
vim.keymap.set({'n', 'v'}, ';e', ':CodeCompanion /explain<CR>', {silent = true})
vim.keymap.set({'n', 'v'}, ';a', ':CodeCompanion ', opts)

vim.cmd [[ set background=dark ]]
vim.cmd [[ colorscheme sonokai ]]

-- plugins install
vim.cmd [[packadd packer.nvim]]
return require('packer').startup(function(use)
	use 'wbthomason/packer.nvim'
	use {
		'nvim-treesitter/nvim-treesitter',
		run = function()
			local ts_update = require('nvim-treesitter.install').update({ with_sync = true })
			ts_update()
		end,
	}
	use 'neovim/nvim-lspconfig'
	use 'lewis6991/gitsigns.nvim'
	use 'sindrets/diffview.nvim'
	use 'MeanderingProgrammer/render-markdown.nvim'
	use {
		"olimorris/codecompanion.nvim",
		requires = {
			"nvim-lua/plenary.nvim",
			"nvim-treesitter/nvim-treesitter",
		}
	}
	use 'sainnhe/sonokai'
end)
