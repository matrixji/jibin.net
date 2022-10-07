---
title: "Milvus 1.x 版本支持S3存储的部署"
date: 2021-08-17T14:42:32+08:00
tags:
  - milvus
  - s3
  - helm
categories:
  - Development
  - Milvus
---

## 背景
Milvus 1.1.1 版本开始比较完整地支持了S3作为存储后端，在此之前，我们在1.1及之前版本部署分布式方案的时候，只能采用NFS的方案。相比来说S3有着更加接地气的云原生表现。

本文简单记录了使用Milvus 1.x版本，通过官方的Helm Chart在Kubernetes环境中部署基于S3 存储的Milvus 集群。

### 测试环境

- Kubernetes: 1.22.0
- S3: Ceph Radosgw (Ceph 16.2.5)

## 部署准备

我们需要首先搭建好一个可用S3集群，这可以是使用Minio搭建的 Server或者 Gateway，也可以是Ceph Radosgw，目前由于实现的限制，对于HTTPS的 S3 在Milvus里面是不支持。

这儿我们使用已经搭建好的Ceph Radosgw提供的 S3 服务作为测试环境。

测试环境相关的参数如下：

- S3服务的 IP地址 `192.168.18.13`，这个参数也支持主机名/域名的方式的
- S3服务的 端口 `80`
- S3服务使用的 `access_key` `WQDU43IQ8YEPS2UDRT4S`
- S3服务使用的 `secret_key` `3tS9vSKY4gGOEeRmVQGzRcxKAWAAHq2zuwMI9d9l`
- 预先创建好给Milvus使用的 Bucket，这里我们使用 `milvus-test`
  ```bash
  s3cmd mb s3://milvus-test
  ```

如果你正在参考此文搭建，需要注意在示例代码中替换这些替换这些参数。

## 部署过程
### 添加Helm

添加Milvus官方的Helm Repo

```bash
helm repo add milvus https://milvus-io.github.io/milvus-helm/
helm repo update
```

当然也可以从官方的Helm Chart的代码仓库下载 1.1分支的Helm Chart，地址： https://github.com/milvus-io/milvus-helm

### 需要配置的参数

#### 开启集群

`cluster.enabled` 需要设置为 `true`

#### S3 相关的参数
```yaml
storage:
  s3:
    enabled: true
    address: 192.168.18.13
    port: 80
    access_key: WQDU43IQ8YEPS2UDRT4S
    secret_key: 3tS9vSKY4gGOEeRmVQGzRcxKAWAAHq2zuwMI9d9l
    bucket: milvus-test
```

#### MySQL存储的设置
在我们的测试中，因为 MySQL 还是使用的是非共享持久化存储(通过PV提供)，并且 Kubernetes 集群预先没有配置默认的 StorageClass, 因此也需要指定 `mysql.persistence.storageClass`。
当然如果仅是测试，你也可以关闭MySQL的数据持久化，配置 `mysql.persistence.enabled` 为 `false`


### 安装部署 
确认好参数之后，我们就可以直接基于Helm 进行部署了

```bash
helm install milvus-test milvus/milvus \
    --version 1.1.6 \
    --set cluster.enabled=true \
    --set storage.s3.enabled=true \
    --set storage.s3.address=192.168.18.13 \
    --set storage.s3.port=80 \
    --set storage.s3.access_key=WQDU43IQ8YEPS2UDRT4S \
    --set storage.s3.secret_key=3tS9vSKY4gGOEeRmVQGzRcxKAWAAHq2zuwMI9d9l \
    --set storage.s3.bucket=milvus-test \
    --set mysql.persistence.storageClass=csi-rbd-sc
```

如果是基于官方代码仓库进行部署的，可以在代码仓目录下执行：

```bash
helm install milvus-test charts/milvus \
    --set cluster.enabled=true \
    --set storage.s3.enabled=true \
    --set storage.s3.address=192.168.18.13 \
    --set storage.s3.port=80 \
    --set storage.s3.access_key=WQDU43IQ8YEPS2UDRT4S \
    --set storage.s3.secret_key=3tS9vSKY4gGOEeRmVQGzRcxKAWAAHq2zuwMI9d9l \
    --set storage.s3.bucket=milvus-test \
    --set mysql.persistence.storageClass=csi-rbd-sc
```

Chart创建后应该会返回大体如下的输出：

```text
NAME: milvus-test
LAST DEPLOYED: Tue Aug 17 06:17:47 2021
NAMESPACE: default
STATUS: deployed
REVISION: 1
TEST SUITE: None
NOTES:

......

For more information on running Milvus, visit:
https://milvus.io/
```

观察POD的状态，等所有milvus-test 相关的POD 都Ready之后，那么就说明部署成功了，如果长时间失败，那么可以通过Describe POD去定位相关的错误原因，当然这超出本文的范围了。

```text
[root@node01 test_milvus]# kubectl get pod  | grep ^milvus-test
milvus-test-mishards-7c44fb887d-98r5r   1/1     Running   0          8h
milvus-test-mysql-7fbd777c9b-6fvdc      1/1     Running   0          8h
milvus-test-readonly-cc46b9f7c-cxqqx    1/1     Running   0          8h
milvus-test-writable-5f67957f94-6sg66   1/1     Running   0          8h
[root@node01 test_milvus]#
```

### Hello Milvus
接下来我们用官方 Hello Milvus来测试一下集群。

```
pip3 install pymilvus==1.1.2 --user
wget https://raw.githubusercontent.com/milvus-io/pymilvus/v1.1.2/examples/example.py
```

并修改example.py中的 Milvus 服务的地址：
```py {hl_lines=[3]}
# Milvus server IP address and port.
# You may need to change _HOST and _PORT accordingly.
_HOST = '10.254.228.63'
_PORT = '19530'  # default value
# _PORT = '19121'  # default http value
```

`10.254.228.63` 是Milvus 服务所在的IP，默认安装时这个可以通过`kubectl get service` 来查询，在正式的生成环境，你应该想法设法通过域名、服务名来访问他。

测试一下，能得到类似的结果应该就是成功了

```text
[root@node01 test_milvus]# python3 example.py
CollectionSchema(collection_name='example_collection_', dimension=8, index_file_size=32, metric_type=<MetricType: L2>)
[[0.5276305713713695, 0.12260932441933903, 0.3171067816154105, 0.13174567988561292, 0.561218915370599, 0.9768724314879139, 0.24527548957936463, 0.14069703982325565], [0.8693438293577815, 0.12859046522689555, 0.9297356851442876, 0.8802749386768061, 0.17213435969718438, 0.2792529446501495, 0.19399770066059763, 0.33760474662703477], [0.5285542325087451, 0.38652674443502677, 0.14204221506972037, 0.4446153658300557, 0.2147865132793082, 0.9802573685721011, 0.3490536785441605, 0.34690242339603694], [0.7640980395490863, 0.23851191563969232, 0.9974789898983394, 0.722209615974155, 0.5624709350402459, 0.9847888630664351, 0.8612679336524167, 0.41734632322181175], [0.7004859469397275, 0.15074329515642382, 0.5012549694880217, 0.6076530709776871, 0.5767756898619726, 0.8492594735963868, 0.5949238938992756, 0.27046992486561117], [0.4883219211158971, 0.6560899610463157, 0.17088775896262798, 0.3163316053192182, 0.4884560175347151, 0.9763345942373309, 0.3704132095239927, 0.14451999941961524], [0.420306114063945, 0.8691462987382358, 0.7681884205204504, 0.953297827028537, 0.3576528295276099, 0.10474646884992755, 0.611149746544882, 0.2603478699362083], [0.8603200422829845, 0.9761167618027261, 0.41316487821482517, 0.7872035159773235, 0.6945912973115762, 0.12198972896625171, 0.8797358763991232, 0.06431771849999612], [0.9815822851563419, 0.8218869618904384, 0.2972918857560406, 0.8344590620834544, 0.6506061203194943, 0.9055832448140761, 0.06218259919367797, 0.5531204533227327], [0.09243830916736795, 0.36326131399430095, 0.053323967514035564, 0.3587912438051374, 0.9962743116612562, 0.8274464481216758, 0.2328478805255606, 0.49902733854966996]]
{'partitions': [{'row_count': 10, 'segments': [{'data_size': 400, 'index_name': 'IDMAP', 'name': '1629250329513627000', 'row_count': 10}], 'tag': '_default'}], 'row_count': 10}
Creating index: {'nlist': 2048}
(collection_name='example_collection_', index_type=<IndexType: IVF_FLAT>, params={'nlist': 2048})
Searching ...
Query result is correct
[
 [ (id:1629250329511543000, distance:0.0) ]
 [ (id:1629250329511543001, distance:0.0) ]
 [ (id:1629250329511543002, distance:0.0) ]
        ......
            ......
]
```