-- Bootstrap lazy.nvim
local lazypath = vim.fn.stdpath("data") .. "/lazy/lazy.nvim"
if not (vim.uv or vim.loop).fs_stat(lazypath) then
	local lazyrepo = "https://github.com/folke/lazy.nvim.git"
	local out = vim.fn.system({ "git", "clone", "--filter=blob:none", "--branch=stable", lazyrepo, lazypath })
	if vim.v.shell_error ~= 0 then
		vim.api.nvim_echo({
			{ "Failed to clone lazy.nvim:\n", "ErrorMsg" },
			{ out, "WarningMsg" },
			{ "\nPress any key to exit..." },
		}, true, {})
		vim.fn.getchar()
		os.exit(1)
	end
end
vim.opt.rtp:prepend(lazypath)

-- Setup lazy.nvim
require("lazy").setup({
	{
		"nvim-treesitter/nvim-treesitter",
		branch = 'master',
		lazy = false,
		build = ":TSUpdate"
	},
	{
		"nvim-telescope/telescope.nvim",
		tag = '0.1.8',
		dependencies = {
			"nvim-lua/plenary.nvim",
			"BurntSushi/ripgrep"
		}
	},
	{
		"MeanderingProgrammer/render-markdown.nvim",
		dependencies = {
			"nvim-treesitter/nvim-treesitter",
			"echasnovski/mini.nvim"
		},
		ft = {
			"markdown",
			"codecompanion"
		}
	},
	{
		"olimorris/codecompanion.nvim",
		dependencies = {
			"nvim-lua/plenary.nvim",
			"nvim-treesitter/nvim-treesitter"
		}
	},
	"neovim/nvim-lspconfig",
	"jmacadie/telescope-hierarchy.nvim",
	"lewis6991/gitsigns.nvim",
	"sindrets/diffview.nvim",
	"cpea2506/one_monokai.nvim",
})

-- base setting
vim.o.number = true
vim.o.relativenumber = true
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
	ensure_installed = { "c", "lua", "vim", "vimdoc", "query", "markdown", "markdown_inline" },
	sync_install = false,
	auto_install = true,
	highlight = {
		enable = true,
		disable = { "c" },
		additional_vim_regex_highlighting = false,
	},
}

local builtin = require('telescope.builtin')
vim.keymap.set('n', ';ff', builtin.find_files, { desc = 'Telescope find files' })
vim.keymap.set('n', ';fg', builtin.live_grep, { desc = 'Telescope live grep' })

require('lspconfig').clangd.setup { }
vim.diagnostic.enable(false)
vim.keymap.set('n', ';d', vim.lsp.buf.definition, opts)
vim.keymap.set('n', ';r', vim.lsp.buf.references, opts)
vim.keymap.set('n', ';c', vim.lsp.buf.incoming_calls, opts)

vim.keymap.set('n', ';cc', ':Telescope hierarchy incoming_calls<CR>', opts)

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
	adapters = {
		deepseek = function()
			return require("codecompanion.adapters").extend("deepseek", {
				env = { api_key = "xxx" },
			})
		end,
	},
	strategies = {
		-- chat = { adapter = "ollama" },
		-- inline = { adapter = "ollama" },
		-- agent = { adapter = "ollama" },
		chat = { adapter = "deepseek" },
		inline = { adapter = "deepseek" },
		agent = { adapter = "deepseek" },
	},
	opts = {
		language = "Chinese",
	},
}
vim.keymap.set({'n', 'v'}, ';w', ':CodeCompanionChat Toggle<CR>', { silent = true })
vim.keymap.set({'n', 'v'}, ';e', ':CodeCompanion /explain<CR>', {silent = true})
vim.keymap.set({'n', 'v'}, ';a', ':CodeCompanion ', opts)

require("one_monokai").setup { }
vim.api.nvim_set_hl(0, "Identifier", { link = "Normal" })
