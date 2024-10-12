---
title: "RAFT在Knowhere上的一些评估测试[2]"
date: 2023-04-04T12:30:00+08:00
lastmod: 2023-04-05T1:30:00+08:00
draft: false
tags:
  - vector-database
  - milvus
  - knowhere
categories:
  - Development
  - Vector Database
---

## 背景

上次 [RAFT在Knowhere上的一些评估测试-1]({{< ref "/posts/rapids-raft-benchmark-with-knowhere-s1" >}})中，简单测试了RAFT和Faiss索引在Knowhere上的表现。但是其实从GTC 2023的内容上看，都在表明：RAFT索引其实比较适合做实时向量数据库查询系统，因此：小的NQ，高并发应该对RAFT来说会更加友好。另外在RAFT构建索引的时候，和Faiss是有差别的，这点目前很少有Benchmark的工具关注（索引构建的效率）。

## Milvus is GPU Ready
在GTC 2023同期，Milvus也发布的支持GPU版本的 [2.3.0 Beta](https://github.com/milvus-io/milvus/releases/tag/v2.3.0-beta)，当然也包含了RAFT索引（GPU版本），因此我们可以基于Milvus对RAFT的索引进行测试了，这将更加系统地来评估这一索引的使用情况。

其实Milvus 2.3.0 Beta 的Release Notes中已经提到了Benchmark的结果，但是说的比较简单，而且没有我关注的和Faiss的对比，主要是对比了HNSW的情况，因此我倾向于自己来做一些测试。

## Benchmark 工具
在向量数据库，或者近似搜索ANN领域最有名的当属 [Ann Benchmarks](http://ann-benchmarks.com/)，但是他貌似并不关注并发的能力。

所幸 Zilliz 开源了自己的Benchmark工具 [zilliztech/vectordb-benchmark](https://github.com/zilliztech/vectordb-benchmark)，当然他应该来自于[qdrant/vector-db-benchmark](https://github.com/qdrant/vector-db-benchmark)，但是已经分岔地有些远了，所以我就不再继续追溯了。直接拿来用就是了。

vectordb-benchmark 支持两类测试： Recall 和 Concurrency，其中 Recall 主要关注索引的召回情况，这个和Ann Benchmarks是一致的；而 Concurrency 则是关注查询并发的情况。

## 测试环境
测试硬件大致如下：
- CPU 10C
- 内存 32G
- 显卡 RTX 3080 Ti

软件环境：
- Milvus 2.3.0 Beta Standalone 模式
- Nvidia Driver Version: 525.89.02

测试数据集和配置：
主要以 sift-128-euclidean 为主。大约100万的数据集，128维。使用IVF_FLAT索引，nlist=1024
测试对比：IVF_FLAT（Faiss CPU）、GPU_IVF_FLAT（Faiss GPU）以及 RAFT_IVF_FLAT 三种索引的并发查询情况。（且测且珍惜，以后在一个版本上能同时跑三种类型的统一索引的机会不多了，主线Milvus的代码目前已经关闭了Faiss GPU索引的选项了），查询的时候 nprobe=16, 并发30，这可以认为是一个比较典型的场景了吧。

## 测试过程

本次主要测试Concurrent的情况。vectordb-benchmark 是需要做一些修改以支持新的Milvus的，主要是替换镜像，另外也需要自己准备测试的配置文件。这些就不赘述了，参考 [zilliztech/vectordb-benchmark](https://github.com/zilliztech/vectordb-benchmark) 的文档，都可以基本摸清楚。

## 测试结果

下面是两组不同NQ和TopK下的测试的结果：

### NQ=10, TopK=1

| Index           | RPS        | AVG(Time) | MIN(Time) | MAX(Time) | TP99   |
| --------------- | ---------- | --------- | --------- | --------- | ------ |
| IVF_FLAT        | 779.16     | 37.89     | 2.27      | 100.75    | 50.04  |
| GPU_IVF_FLAT    | 2551.62    | 11.37     | 0.92      | 30.04     | 17.12  |
| RAFT_IVF_FLAT   | 6393.25    | 4.42v     | 0.82      | 22.07     | 8.50   |

### NQ=100, TopK=100

| Index           | RPS        | AVG(Time) | MIN(Time) | MAX(Time) | TP99   |
| --------------- | ---------- | --------- | --------- | --------- | ------ |
| IVF_FLAT        | 77.79      | 383.38    | 33.13     | 421.85    | 407.42 |
| GPU_IVF_FLAT    | 432.23     | 68.68     | 10.09     | 84.44     | 77.72  |
| RAFT_IVF_FLAT   | 408.27     | 72.74     | 13.42     | 93.83     | 83.77  |

说明：
- RPS：每秒查询响应的次数
- AVG(Time)：平均查询时间，单位毫秒
- MIN(Time)：最小查询时间
- MAX(Time)：最大查询时间
- TP99：%99较好查询的最大时间

显然，RAFT的并发能力是优秀的，尤其当NQ=较小的时候，他的TP99也是最低的。
而当NQ=100，TopK=100的时候，RAFT的效率并没有很大的优势，甚至比Faiss的效率还要低一些，希望后续随着RAFT索引自身的优化，能够改善了。

当然测试过程中RAFT索引的显存开销是比较大的，当然这次的测试规模并没有导致出现显存不足的情况，但对于更大规模的数据集，这个问题还是需要考虑的。

所以你如果计划使用Milvus 2.3 GPU版本的话，你知道如何选用索引了吧？

## 后续
后续随着Milvus GPU版本的演进，可能会继续做一些测试，主要的方向：
- 目前Benchmark工具并没有专门提供索引构建的时间的评估，这个也是一个重要的指标，我会在后续的文章中来试图补充这个指标。
- 目前只是测试了IVF_FLAT索引，后续也会考虑测试IVF_PQ。以及IVF_FLAT和IVF_PQ，包括IVF_SQ的横向对比。
- Milvus配置参数对GPU索引的影响。
