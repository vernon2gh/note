## 简介

llama.cpp 能够在笔记本电脑、台式机或服务器上本地运行大型语言模型，设置极少，
性能达到业界领先水平。

* 通过包管理器安装

```bash
$ sudo dnf install llama-cpp
```

* 原生安装

```bash
$ curl -LsSf https://llama.app/install.sh | sh
```

## 直接使用

```bash
llama cli -hf ggml-org/gemma-4-e4b-it-GGUF:Q4_0
```

启动一个大模型，在终端中与大模型对话

```bash
llama serve -hf ggml-org/gemma-4-e4b-it-GGUF:Q4_0
```

启动一个兼容 OpenAI 的服务器，并自带 Web 界面。

## 接入到 Pi

```bash
$ llama serve                                    # 启动大模型
$ pi install git:github.com/huggingface/pi-llama # 安装 pi-llama 插件
$ pi                                             # 启动 Pi
```

## 参考

https://llama.app/
