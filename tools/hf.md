## 简介

hf 全称 Hugging Face Hub CLI，主要用于从 hugging Face Hub 下载大模型到本地。

## 安装

```bash
$ sudo dnf install hf
```

## 使用

```bash
$ hf cache list
ID                                 SIZE LAST_ACCESSED  LAST_MODIFIED REFS
-------------------------------- ------ -------------- ------------- ----
model/ggml-org/Qwen3.5-0.8B-GGUF 833.6M 20 minutes ago 5 weeks ago   main
```

查询当前本地有哪些大模型

```bash
$ hf cache rm model/ggml-org/Qwen3.5-0.8B-GGUF
```

删除指定本地大模型

```bash
$ hf models list --sort downloads --apps llama.cpp --search "glm 5.3 flash"
ID                          CREATED_AT DOWNLOADS LIKES PIPELINE_TAG
--------------------------- ---------- --------- ----- ---------------
unsloth/GLM-5.3-Flash-GGUF  2026-08-26    945179   483 text-generation
```

从 Hugging Face 查询符合条件（下载量、推理引擎、模型名等）的大模型

```bash
$ hf models list -h --tree unsloth/GLM-5.3-Flash-GGUF
  9.5 KB  Aug 29 10:43  ├── .gitattributes
                        ├── BF16/
                        ├── Q8_0/
  8.4 KB  Aug 28 14:45  ├── README.md
                        ├── Shard_Rewrite/
                        ├── UD-IQ1_M/
                        ├── UD-IQ1_S/
                        ├── UD-IQ2_XXS/
                        ├── UD-IQ3_XXS/
                        ├── UD-IQ4_XS/
                        ├── UD-Q2_K_XL/
                        ├── UD-Q3_K_XL/
                        ├── UD-Q4_K_XL/
                        ├── UD-Q5_K_XL/
                        ├── UD-Q6_K_XL/
512.7 MB  Aug 28 18:04  ├── imatrix_unsloth.gguf_file
  1.2 GB  Oct 05 22:23  ├── mmproj-BF16.gguf
  1.1 GB  Oct 05 22:23  └── mmproj-F16.gguf
```

查询指定大模型存在哪些量化版本（Q8_0, UD-IQ1_M..UD-Q6_K_XL）

```bash
$ hf download unsloth/GLM-5.3-Flash-GGUF --include "Q8_0/*"
```

下载指定大模型（GLM-5.3-Flash Q8_0 量化版本）到本地。
如果没有指定 `--include`，将下载所有量化版本到本地。
