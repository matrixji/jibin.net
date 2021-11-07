---
title: "在 Windows上运行Milvus 2.0"
date: 2021-10-18T16:22:00+08:00
draft: false
tags:
  - milvus
  - windows
categories:
  - Development
  - Milvus
---

![Milvus On Windows](/images/milvus-on-windows.png)

## 简述

最近，一直在断断续续地尝试把 Milvus 2.0 Porting到Windows上，目前基本上已经可以以Standalone模式运行了。

目前看支持Windows运行的这部分改动合并到主线，看起来可能会是一个漫长的过程，所以写一篇笔记记录一下。

如果你想要从Windows 开始熟悉Milvus 2.0，或者因为环境的限制，需要在纯粹的Windows上运行Milvus的话，那么本文将是一个不错的参考。

文末的阅读原文可以获取最新版本。

## 获取Windows上的Milvus 2.0

目前，可以有两种方式来获取。

第一种是从源代码编译，你可以从下面这个个人仓库/分支来获取代码完成编译：[matrixji/milvus at windows-dev](https://github.com/matrixji/milvus/tree/windows-dev)

第二种方式就是直接下载已经编译好的二进制包了，从下面的Release页面可以找到：[Releases - matrixji/milvus](https://github.com/matrixji/milvus/releases)

### 从源码编译的步骤

#### 安装 MSYS

目前使用 MinGW64/MSYS 作为工具链来完成Milvus 2.0在Windows 上的编译，你可以在 [MSYS2](https://www.msys2.org/) 的官网下载并安装。

安装完MSYS之后，接下来所有的编译步骤，你都需要在 **MSYS2 MinGW 64-bit** 的Shell中完成。

首次安装后，可以进行更新，并且安装git 方便后面clone 代码。

```shell
$ pacman -Su
$ pacman -S git
```

#### Clone 代码

```shell
$ git clone git@github.com:matrixji/milvus.git -b windows-dev
```

当前，在非官方的仓库，使用 `windows-dev` 这个分支来跟踪用来支持Windows 上编译的更改。这个分支每周会从上游的主线分支 [milvus-io/milvus](https://github.com/milvus-io/milvus) 进行 rebase。


#### 编译并打包

先安装需要的依赖和工具链：

```shell
$ cd milvus
$ scripts/install_deps_msys.sh
```

**注意：第一次安装依赖之后，需要重启一下MinGW/MSYS的Shell，以确保一些工具的配置生效**

接下来就可以编译和打包 Milvus 了:

```shell
$ make
$ sh scripts/package_windows.sh
```

如果一切顺利的话，最后你能够在 `windows_package` 的子目录里面找到一个zip包。里面包含了运行milvus.exe的所有的文件，同时包括 minio.exe 和 etcd.exe。

**注意**

- 因为有一些第三方依赖存放在Github上，所以为了顺利编译，可能科学上网是需要的。
- Milvus 2 是使用golang开发的，为了更好地下载第三方modules，可能goproxy是需要设置的，参考：[goproxy.cn](https://goproxy.cn)

### 直接下载编译好的二进制包

如果不想自己编译，也可以从 [Releases - matrixji/milvus](https://github.com/matrixji/milvus/releases) 下载已经编译好的二进制包。

当前会使用 `windows-test-` 作为Release的前缀，比如 `windows-test-v8` 这样的版本，找到对应的zip包下载就可以了。


## 启动 Milvus

把zip包解压后，你就能找到 `milvus` 的目录，所有的内容都在里面了。

### 启动步骤
- `run_minio.bat` 双击/运行他将启动一个 minio 的默认配置的服务，他将使用 `s3data` 这个子目录来存放数据。
- `run_etcd.bat` 将启动一个默认配置下的 etcd 的服务。
- 上面两个服务都启动成功后，你现在就可以通过 `run_milvus.bat` 来启动 milvus 了。

### hello_milvus.py

在Windows 上成功启动 Milvus 之后，可以尝试使用 hello_milvus.py 来测试一下，这个可以参考下面的官方指导，这里就不赘述了。

官方的Hello Milvus：https://milvus.io/docs/v2.0.0/example_code.md

因为 pymilvus 是一个纯 python 的库，所以可以方便地在Windows下运行起来。

