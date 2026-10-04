## 简介

llama.cpp 能够在笔记本电脑、台式机或服务器上本地运行大型语言模型，设置极少，
性能达到业界领先水平。

## 安装

```bash
$ curl -LsSf https://llama.app/install.sh | sh
```

## 直接使用

```bash
llama cli -hf ggml-org/Qwen3.5-0.8B-GGUF
```

从 Hugging Face 下载/启动指定大模型，然后在终端中与大模型对话

```bash
llama serve
```

启动一个兼容 OpenAI 的服务器，并自带 Web 界面。
可以与 Pi 配合使用。

```bash
llama bench -hf ggml-org/Qwen3.5-0.8B-GGUF
```

对 prompt 处理和文本生成速度（tokens/s）进行基准测试

## 参数（可选）

* `--offline`     使用本地大模型，禁止通过网络下载大模型
* `--device none` 使用 CPU，禁止使用 GPU
* `--no-repack`   禁止 repack 成 SIMD 布局的 malloc 副本（匿名页）

## 接入到 Pi

```bash
$ pi
> /login  ## 指定URL，（可选）使用 `/llama` 进行load/unload/download大模型
> /model  ## 选择大模型
```

## 参考

https://llama.app/
