# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Language Policy

**IMPORTANT**: Always respond in Chinese, regardless of whether the user asks questions in Chinese or English.

## Repository Overview

This is the Linux Kernel source code repository.

## Code Analysis Guidelines

**IMPORTANT**: When analyzing kernel functions (finding definitions, references, call hierarchies, etc.), always use the LSP tool instead of grep/search commands. LSP provides accurate symbol information through the language server.

Examples of when to use LSP:
- Finding where a function is defined (goToDefinition)
- Finding all references to a function/variable (findReferences)
- Getting function documentation or type information (hover)
- Analyzing call relationships (prepareCallHierarchy, incomingCalls, outgoingCalls)
- Listing all symbols in a file (documentSymbol)
