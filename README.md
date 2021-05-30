# Zero copy
Zero copy can avoid redundant memory copy (CPU copy) and context switches between user space and kernel space. But the "truly zero copy" requires hardware DMA (Direct Memory Access) with scatter-gather capability. In the process of ordinary DMA transferring data, the source physical address and the target physical address must be continuous. Scatter-gather DMA is different, it can transfer the data from a discrete physical address to a continuous physical address.

# Compilation and usage
### Compilation
```
$ git git@github.com:Chen-Kuan-Chung/zero-copy.git
$ cd zero-copy
$ make
```

### Usage
* File copy from disk to disk
```
./file_copy -h
Usage
  file_copy -m <mode> -b <buffer size>

Mode:
  1:        read/write
  2:        mmap/write
  3:        sendfile

Buffer size: bytes, default is 64
```

Example: use mmap/write with a buffer size of 128 bytes to copy file
```
$ ./file_copy -m 2 -b 128
```

* File copy from disk to network
```
[Server]
./file_copy_server -h
Usage
  file_copy_server -b <buffer size>

Buffer size: bytes, default is 64

-------------------------------------------------
[Client]
./file_copy_client -h
Usage
  file_copy -i <ip> -m <mode> -b <buffer size>

IP: Server IP address, default is 127.0.0.1

Mode:
  1:        read/write
  2:        mmap/write
  3:        sendfile

Buffer size: bytes, default is 64
```

Example: use mmap/write with a buffer size of 128 bytes to copy file from client to server (192.168.127.256)
```
[Server]
$ ./file_copy_server -b 128

-------------------------------------------------
[Client]
$ file_copy -i 192.168.127.256 -m 2 -b 128
```

# Zero copy performance
### 1. read/write - two CPU copy
<img src="https://github.com/Chen-Kuan-Chung/zero-copy/blob/master/png/read_write.png" width="480" height="360">

### 2. mmap/write - one CPU copy
<img src="https://github.com/Chen-Kuan-Chung/zero-copy/blob/master/png/mmap_write.png" width="480" height="360">

### 3. sendfile (Ordinary DMA) - one CPU copy
<img src="https://github.com/Chen-Kuan-Chung/zero-copy/blob/master/png/sendfile_ordinary_dma.png" width="480" height="360">

### 4. sendfile (Scatter-gather DMA) - zero CPU copy
<img src="https://github.com/Chen-Kuan-Chung/zero-copy/blob/master/png/sendfile_scatter_gather_dma.png" width="480" height="360">

# Experiment
### Configuration
* File copy from disk to disk
* Buffer size: 32, 64, 128, 256, 512
* Test iteration: 10
```
$ cd zero-copy
$ ./analysis.sh
```

### Result
<img src="https://github.com/Chen-Kuan-Chung/zero-copy/blob/master/png/file_copy_analysis.png" width="480" height="360">

*Note: sendfile has no buffer parameter*
