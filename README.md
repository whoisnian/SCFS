# SCFS
Simple and Crude FileSystem.

## 要求
* Linux 环境
* libfuse

## 结构
* 1G大小的文件模拟磁盘，磁盘上分为262144个4K扇区，其中
  * 0号扇区：superblock
  * 1-2048号扇区：inode
  * 2049-2050号扇区：inode位图
  * 2051-2058号扇区：block位图
  * 2059-262143扇区：block
* 具体结构图：
  ![SCFS.svg](SCFS.svg)
