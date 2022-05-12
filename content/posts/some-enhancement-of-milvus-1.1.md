---
title: "Milvus 1.1上的一些增强"
date: 2022-05-09T15:18:00+08:00
draft: false
tags:
  - milvus
categories:
  - Development
  - Milvus
---

## 简述

最近，在社区发现还是有很多的用户仍然在使用Milvus 1.1.1的版本，其实Milvus 2.0目前已经GA，在这种情况下，使用1.1.1的用户，我想不外乎下面几个理由（其实我们也还在使用1.1.1）：

- 现有的产品已经基于1.1.1开发集成了，并且也能够满足需求，没有升级动力
- 目前Milvus 2.0尚没有提供GPU的版本，但是业务对性能有比较苛刻的需求
- 觉得2.0的架构太复杂了（目前即使单机运行至少还需要Etcd和Minio作为依赖）

但是1.1.1真的太老了（其实这么说也不太对，毕竟1.1的Release了也就一年），在使用的时候，会有多种的限制，但是同时官方并没有Release镜像的计划了，因为我们在自己使用的过程中就会需要自己增强部分功能并进行编译。同时最近在社区的交流中也帮助社区的小伙伴解决了一些1.1.1版本中软硬件环境的兼容问题，因此就有了分享一个非官方的镜像的想法，并且给一个简单的介绍。

这些增强的代码改动目前都可以从这儿获取: [https://github.com/matrixji/milvus/tree/1.1-enhanced](https://github.com/matrixji/milvus/tree/1.1-enhanced)

而针对基于CentOS7的编译好的镜像则可以通过以下方式获取（CPU和GPU版本）
- `docker pull matrixji/milvus:1.1-enhanced-cpu-9408a4e`
- `docker pull matrixji/milvus:1.1-enhanced-gpu-9408a4e`

## 增强点

由于官方的镜像停留在了：330cc61，实际在这之后，官方1.1分支上的改进和修复根据Commit Message来看应该就包含了：
- search result of the restful api contains -1 (#6127)
- fix insert to or delete from the default partition (#6269)
- Fix GetEntityById in the default partition (#6285)
- improve getEntityById performance
- mysql ssl (#6670)
- Flat binary search over APU (#6676)
- avoid potential random crash (#6710)
- Introducing s3 region parameter for aws s3 service. (#7419)
- Update MemTable.cpp (#14224) // 改进了删除性能
- Fix #12498 crashed when do specific actions (#14698)
- Support aliyun oss storage (#16002)

在此基础之上，针对社区中遇到的对于仅支持SSE4.2的CPU无法运行的问题，以及一些非常老的或者非常新的显卡无法运行GPU版本的情况（RTX 3060, Tesla P80等），做了一些编译层面的适配：
- Support cpu only support SSE42
- Adaptation for cuda11 and legacy gpu

下面会对几个我提供的增强做一些简单的介绍。

### 支持了阿里云OSS存储

Milvus 1.1.1 已经支持了S3作为存储后端，这在读写分离的集群部署中是可能用到的。但是考虑到国内阿里云的用户的存在，实际上阿里云并无法完全通过S3的API来完成访问（通过特定Minio的版本是可以完成适配，但这增加了部署复杂度和单点故障），所以基于一个实际项目的需求，完成了Milvus使用OSS存储作为存储后端的实现。

具体的配置，可以参考如下：

```yaml

# milvus_server.yaml

storage:
  path: /var/lib/milvus
  auto_flush_interval: 1
  oss_enabled: true
  # 接入的OSS Endpoint，通常可以在OSS的控制台查询到
  oss_endpoint: https://oss-cn-hangzhou.aliyuncs.com
  oss_access_key: access_key
  oss_secret_key: secret_key
  oss_bucket: milvus_bucket
```

### 支持了更多的显卡

编译的时候使用了Cuda 11, 并且尝试开启了更多的显卡硬件支持。所以理论上对于Nvidia显卡，从Pascal（典型产品 Tesla P40/80）到Ampere（典型产品RTX 3060/Tesla A2/10/30）架构应该基本都支持了。

### 支持了更老的CPU

一些非常老的CPU，以及一些特殊环境的虚拟CPU，可能仅支持SSE4.2指令集（没有AVX等更高的高级指令集支持），这在之前版本的Milvus中由于一些实现的细微差别，并没有办法运行。因此在这个增强分支中，这些老CPU获得了在本地运行测试的可能。当然这里面也包括了Apple的M1的 X86_64的模拟层。当然Apple M1的模拟层效率会很低，所以不纠结1.1版本的小伙伴，如果想在Apple Silicon上玩耍的话，还是尽量在2.0上愉快地玩耍吧（目前2.0应该已经支持了原生的M1）。

### 更小的镜像体积

Milvus 1.1 官方最后的GPU镜像的体积达到了 1.6GB（压缩后），鉴于 Milvus实际上主要依赖CuBLAS，所以基于Cuda的Base镜像对Dockerfile进行了优化。所以在上面提到的增强分支上的GPU的镜像 `milvus:1.1-enhanced-gpu-9408a4e` 在体积上优化到了700MB左右。

### 一些依赖

- 依赖更新，对于GPU版本，由于使用了Cuda 11.6，所以在运行的主机上需要安装 Nvidia的驱动版本至少 >= 450.80.02

## 后记

Milvus 1.1.1 这个版本时至今日仍然有着不小的用户群体，说实话，起初我是有些吃惊的。但是后来我们自己在重构内部的软件的时候，曾经在1.1和2.0上做选择，但是最终起到决定性作用的是：2.0无法支持GPU，最后短期内我们只能考虑使用1.1完成第一个版本。所以也才有了上面的这些增强改进，基于Upstream First的原则，后续我应该会尝试先把代码合并到官方的1.1分支上。

相信在Milvus 2.2迎来 GPU Support的时候，社区中会有更多的小伙伴去拥抱2.0的分支。