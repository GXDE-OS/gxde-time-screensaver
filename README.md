# GXDE 一言屏保
GXDE 一言屏保，可以显示时间和句子（句子来自一言）  


![截图_选择区域_20240413210018.png](https://storage.deepin.org/thread/202404131305187727_截图_选择区域_20240413210018.png)

## 如何编译
在 GXDE（推荐）或 deepin、UOS 下输入：  
```bash
git clone https://gitee.com/GXDE-OS/gxde-time-screensaver
cd gxde-time-screensaver
sudo apt build-dep .
qmake .
make -j4
sudo make install -j4
```

### 构建安装包
```bash
git clone https://gitee.com/GXDE-OS/gxde-time-screensaver
cd gxde-time-screensaver
sudo apt build-dep .
dpkg-buildpackage -b
```

# ©2023～Now gfdgd xi