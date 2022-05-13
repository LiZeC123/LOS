LOS操作系统
----------------



### Ubuntu编译安装bochs

虽然官网提供了安装包，但为了启用调试功能，还是需要从源码进行编译。由于bochs使用了X11相关的功能，因此需要先安装如下的依赖

```
sudo apt install libxrandr-dev
sudo apt install libx11-dev
```

之后执行

```
./configure --enable-debugger --with-x --with-x11
make -j6
sudo make install 
```


