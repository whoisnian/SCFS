# SCFS
Simple and Crude FileSystem.  

仿Linux下ext2结构的文件系统，提供“挂载到目录，使用已安装软件正常访问读写”和“使用命令行预定义指令进行交互”两种使用方式。

## 要求
* Linux 环境
* libfuse 3

## 相关设计
* root 用户默认密码`toor`；
* root 用户 uid 和 gid 为 0，目录挂载模式下登录前用户 uid 和 gid 为 2000，相当于访客身份，新建用户uid从2001开始递增，新建用户组gid从3001开始递增；
* 目录挂载模式下通过运行 bin 目录下程序进行用户登录，新建用户，修改密码等操作，程序获取用户输入后向`/.run_command`文件写入内容，文件系统在`sc_write()`函数内拦截对该文件的写入操作，并执行相应指令。；
* `test_debug`程序用于查看虚拟磁盘文件源数据，如超级块，索引节点信息，数据块十六进制数据等，输入`h`可查看使用帮助；
* `test_fuse`为目录挂载模式主程序，由于部分函数未实现线程安全，如用到的`strtok()`，运行时需要指定单线程模式；
* `test_screen`为命令行交互主程序，支持指令包括：`clear`，`exit`，`ls`，`cd`，`mkdir`，`cat`，`touch`，`rm`，`cp`，`chmod`，`passwd`，`useradd`，`userdel`，`groupadd`，`groupdel`，`gpasswd`，`vi`，和`scfsviewer`，`vi`是一个十分简单的编辑器，`scfsviewer`是集成进来的`test_debug`功能；
* bitmap 查找时每次读取`unsigned long`长度（64位），假设 1G 大小的虚拟磁盘文件，块大小为 4K，则共`2^30/2^12 = 2^18 = 262144`个块，每个块对应一位，则共相当于`262144/64 = 4096`个无符号长整形，查找速度可以接受；
* inode 下 block 块号存储区预留16个直接，2个间接，1个双间接，一共可以指向`16+2*1024+1*1024*1024 = 1050640`个有效block，即单个文件最大约为4G。

## 使用
* clone仓库源码：  
  `git clone https://github.com/whoisnian/SCFS.git`  
  `cd SCFS`   
  `mkdir out mnt`  
* 在目录挂载模式下运行：  
  * 编译运行并挂载到./mnt目录（-s表示单线程，-d表示debug模式）：  
    `make test_fuse && ./out/test_fuse -s -d ./mnt`  
  * 取消挂载：  
    `fusermount -u ./mnt`  
  * 清空数据：  
    `make clean && rm /tmp/test.img`  
* 在命令行交互模式下运行：
  * 编译运行：  
    `make test_screen && ./out/test_screen`
  * 清空数据：  
    `make clean && rm ./test.img`  

## 结构
* 1G大小的文件模拟磁盘，磁盘上分为262144个4K扇区，其中
  * 0号扇区：superblock 超级块
  * 1-2048号扇区：inode 索引节点
  * 2049-2050号扇区：inode_bitmap 索引位图
  * 2051-2058号扇区：block_bitmap 数据块位图
  * 2059-262143扇区：block 数据块
* 具体结构图：
  ![SCFS.svg](SCFS.svg)

## 参考资料
* [鳥哥的 Linux 私房菜：第七章、Linux 磁碟與檔案系統管理](http://linux.vbird.org/linux_basic/0230filesystem.php)
* [Linux系统ext2相关源码](https://github.com/torvalds/linux/blob/master/fs/ext2/ext2.h)
* [libfuse接口说明](http://libfuse.github.io/doxygen/structfuse__operations.html)
