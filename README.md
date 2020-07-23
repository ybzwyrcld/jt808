# JT808

中华人民共和国交通运输行业标准JT/T808协议的命令生成和解析库;


## 快速使用
本工程使用`CMake`构建工具, 编译工具为`G++`(`Windows`系统下为`MinGW G++`).

### 编译准备

#### Ubuntu 系统
```bash
$ sudo apt-get install cmake cmake-curses-gui
```

#### Windows 系统
到官网下载[MinGW](http://www.mingw.org/), 安装并设置好环境变量.
到官网下载[CMake](https://cmake.org/download/), 安装并设置好环境变量.

### 编译
```bash
$ cmake .. && make
```

### 编译示例程序
```bash
$ cmake .. -DJT808_BUILD_EXAMPLES=ON && make
```
编译输出文件位于`build/examples`目录下.


### 生成 debug 版和 release 版的程序
```bash
$ ccmake ..
```
编辑`CMAKE_BUILD_TYPE`行, 填写`Debug`或`Release`.
