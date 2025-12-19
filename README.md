# WeCom_copy

# 📚简介
本项目为Qt实现企业微信界面项目，包含客户端界面和服务端实现。客户端实现了纯界面逻辑，主要用于学习QWidget的使用，包含绘制，布局，信号，事件，重写，多线程，qss等技术使用。服务端实现了音视频通话和TCP通信功能。
[![star](https://gitee.com/hudejie/wecom-copy/badge/star.svg?theme=dark)](https://gitee.com/hudejie/wecom-copy)

## 项目结构

- **客户端 (Client)**: Qt 图形界面应用 - 企业微信UI实现
- **服务端 (Server)**: TCP 服务器 - 支持音视频通话和消息通信

# 💾体验程序

## 客户端 (Client)
-  [安装包地址](https://gitee.com/hudejie/wecom-copy/raw/master/setup/Setup.exe)

## 服务端 (Server)
查看 [server/README.md](server/README.md) 了解如何编译和运行服务器。

# 📦软件架构

## 客户端
- Qt 5.9 + msvc 2015
- Windows(x32, x64)/Linux(x32, x64)

## 服务端
- Qt 5.9+ (仅使用 Qt Core 和 Qt Network)
- C++11
- 支持 Windows/Linux/macOS 

# 🛠️主要技术

## 客户端技术

| 模块                |     介绍                                                                          |
| -------------------|---------------------------------------------------------------------------------- |
| qss                   |     样式表，本程序所有窗体、控件的样式都由qss设计                                           |
| signal\slot                |     控件、窗体间通信，事件处理                                               |
| QThread              |     异步处理                                                                     |
| QNetworkAccessManager|     网络请求，主要用于聊天机器人及实时天气获取                                               |
| QPainter        |     部分窗口的绘制，例如实时天气界面                                          |
| iconfont      |     阿里巴巴矢量图标库，主要用于按钮及标签上图标等显示                                     |
| webenginewidgets        |     实现嵌入html，主要用于聊天界面                                          |
| webchannel      |     和js进行通信，用于聊天界面交互                                     |

## 服务端技术

| 模块                |     介绍                                                                          |
| -------------------|---------------------------------------------------------------------------------- |
| QTcpServer         |     TCP服务器，处理客户端连接                                                        |
| QTcpSocket         |     TCP套接字，管理客户端通信                                                        |
| QDataStream        |     数据序列化，消息编解码                                                           |
| 音视频通话          |     一对一音视频通话支持                                                             |
| 呼叫信令            |     呼叫请求、接受、拒绝、结束                                                        |
| 媒体中继            |     音视频数据在客户端间转发                                                          |

# 🗺️软件截图

### 导航
![导航](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/导航.gif)

### 基础框架
![基础框架](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/基础框架.gif)

### 用户详情
![用户详情](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/用户详情.png)

### 好友列表
![好友列表](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/好友列表.gif)

### 模拟登录
![模拟登录](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/模拟登录.gif)

### 聊天对话框
![聊天对话框](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/聊天对话框.gif)

### 聊天界面
![聊天界面](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/聊天界面.gif)

### 智能机器人
![智能机器人](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/智能机器人.gif)

### 天气预报
![天气预报](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/天气预报.gif)

### 自绘时钟
![自绘时钟](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/自绘时钟.gif)

### iconfont图标展示
![iconfont](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/iconfont.gif)

### 逗逗猫（小猫眼睛随着鼠标位置转动）
![逗逗猫](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/%E9%80%97%E9%80%97%E7%8C%AB.gif)

### 组件
#### 通知提醒框
![通知提醒框](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/组件_通知提醒框.gif)

#### 气泡确认框
![气泡确认框](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/组件_气泡确认框.gif)

#### 滑动输入条
![滑动输入条](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/组件_滑动输入条.gif)

### 动态主页
![动态主页](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/动态主页.gif)
![动态主页](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/主页.jpg)

### ECharts表格
![ECharts表格](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/Echarts.gif)

### 轮播图
![轮播图](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/轮播图.gif)

### 背景音乐(别撸代码了，听听歌吧)
![背景音乐](https://gitee.com/hudejie/wecom-copy/raw/master/screenshot/背景音乐.gif)

---

# 🖥️ 服务端 (Server)

## 功能特性

WeCompany Server 是一个基于 Qt 的 TCP 服务器，提供以下功能：

- ✅ **TCP 服务器** - 处理多客户端并发连接
- ✅ **音视频通话** - 支持一对一音频/视频通话
- ✅ **呼叫信令** - 完整的呼叫流程管理
- ✅ **媒体中继** - 客户端间音视频数据转发
- ✅ **消息通信** - 文本消息和心跳保活

## 快速开始

### 编译服务器

```bash
cd WeCompany
qmake WeCompanyServer.pro
make
```

### 运行服务器

```bash
./bin/WeCompanyServer -p 8888
```

## 详细文档

- 📖 [服务器 README](server/README.md) - 功能介绍和使用说明
- 🛠️ [构建部署指南](server/BUILD_DEPLOY.md) - 多平台编译和部署
- 📡 [API 文档](server/API.md) - 消息协议和 API 说明
- 💻 [客户端集成](server/CLIENT_INTEGRATION.md) - 客户端接入示例代码

## 架构设计

```
┌─────────────────┐          ┌─────────────────┐
│   Client App 1  │◄────────►│                 │
│  (WeComCopy)    │          │                 │
└─────────────────┘          │  WeCompany      │          ┌─────────────────┐
                             │    Server       │◄────────►│   Client App 2  │
┌─────────────────┐          │                 │          │  (WeComCopy)    │
│   Client App 3  │◄────────►│  - TCP Server   │          └─────────────────┘
│  (WeComCopy)    │          │  - Call Manager │
└─────────────────┘          │  - Media Relay  │
                             └─────────────────┘
```

## 技术栈

- **语言**: C++11
- **框架**: Qt 5.9+ (Core, Network)
- **协议**: TCP, 自定义二进制协议
- **平台**: Windows, Linux, macOS

## 项目文件

- `WeComCopy.pro` - 客户端项目文件
- `WeCompanyServer.pro` - 服务端项目文件
- `server/` - 服务端源代码和文档




