<p align="center"><img width="210" src="https://github.com/Water-Melon/Melon/blob/master/docs/logo.png?raw=true" alt="Melon logo"></p>
<p align="center"><img src="https://img.shields.io/github/license/Water-Melon/Melang" /></p>
<h1 align="center">Melon</h1>



**[中文说明](http://doc.melonc.io/cn/)**

Melon is a generic cross-platform C library. It contains many algorithms, data structures, functional components, scripting languages and practical frameworks, which can facilitate developers to quickly develop applications and avoid the dilemma of repeated wheel building.

- Components
  - Library Initialization
  - Configuration
  - Log
  - Error Code Management
  - Memory Pool
  - Thread Pool
  - I/O Thread
  - TCP Encapsulation
  - Event Mechanism
  - File Set
  - HTTP Handling
  - Scripting Language
  - Lexical Analyzer
  - Parser Generator
  - Websocket
  - String
  - Regular Expression
  - Big Number Calculation
  - FEC
  - JSON
  - Matrix Operations
  - Reed Solomon Coding
  - Cron Format Parser
  - Spin Lock
  - Prime Generator
- Data Structures
  - Doubly Linked List
  - Fibonacci Heap
  - Hash Table
  - Queue
  - Red-black Tree
  - Stack
  - Array
- Algorithms
  - AES
  - DES/3DES
  - RC4
  - RSA
  - MD5
  - SHA
  - Base64
- Scripting Language Development
- Frameworks
  - Multi-Process Model
  - Multi-Thread Model
  - Trace Mode
  - IPC



### Platform Support

Melon was originally written for UNIX systems, so it is suitable for UNIX-like systems such as Linux and MacOS. And there are a few optimizations for Intel CPUs.

At present, Melon has also completed the preliminary porting to Windows, so it can be used on Windows. However, because Windows differs greatly from the UNIX system in the creation process, some functions of the above `framework` are temporarily not supported in Windows.



### Installation

On Windows, please install `mingw`, `git bash` and `make` at first. Install [MingW-W64-builds](https://www.mingw-w64.org/downloads/#mingw-builds) with the installation settings:

- `Version`: `8.1.0`

- `Architecture`: `i686`

- `Threads`: `posix`

- `Exception`: `dwarf`

- `Build revision`: `0`

Then execute these shell commands on git bash or terminal (on UNIX).

```
git clone https://github.com/Water-Melon/Melon.git
cd Melon
./configure [--prefix=LIB_INSTALL_PATH]
make
make install
```



### License

[BSD-3-Clause License](https://github.com/Water-Melon/Melang/blob/master/LICENSE)

Copyright (c) 2014-present, Niklaus F. Schen



### Documentation

Please refer to [Official Documentation Website](http://doc.melonc.io/) for more details.

中文文档请参考：[中文指南](http://doc.melonc.io/cn/)



### Docker

You can pull the built container image to deploy the running environment.

**Docker image will not be maintained temporarily because I cannot sign in docker hub from my country.** 

```shell
docker pull melonc/melon
```



### Webassembly

You should install `emsdk` at first, make sure `emcc` and `emar` have been installed. Then execute:

```shell
./configure --enable-wasm
make && make install
```

There is only one static library `libmelon.a` to be created.



### Contact

Twitter: @MelangInc

QQ: 756582294

