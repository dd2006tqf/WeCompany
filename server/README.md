# WeCompany Server

## 简介 (Introduction)

WeCompany Server 是企业微信服务端实现，提供音视频通话和TCP通信功能。

WeCompany Server is the server-side implementation for the WeCompany application, providing audio/video calling and TCP communication features.

## 功能特性 (Features)

- **TCP 服务器** - 处理客户端连接和消息通信
  - TCP Server - Handles client connections and message communication
  
- **音视频通话** - 支持一对一音频和视频通话
  - Audio/Video Calling - Supports one-to-one audio and video calls
  
- **呼叫信令** - 完整的呼叫流程（发起、接受、拒绝、结束）
  - Call Signaling - Complete call flow (initiate, accept, reject, end)
  
- **媒体中继** - 音视频数据在客户端之间转发
  - Media Relay - Audio/video data forwarding between clients

## 架构 (Architecture)

### 核心组件 (Core Components)

1. **TcpServer** - TCP服务器基础类
   - 管理客户端连接
   - 处理消息收发
   - 支持广播和点对点通信

2. **VideoCallServer** - 音视频通话服务器
   - 呼叫会话管理
   - 呼叫信令处理
   - 媒体数据转发

### 消息协议 (Message Protocol)

消息格式：`[消息类型(1字节)][用户ID长度(2字节)][用户ID][数据]`

Message Format: `[MessageType(1 byte)][UserId length(2 bytes)][UserId][Data]`

#### 消息类型 (Message Types)

- `MSG_TEXT (0)` - 文本消息
- `MSG_FILE (1)` - 文件传输
- `MSG_CALL_REQUEST (2)` - 音视频通话请求
- `MSG_CALL_ACCEPT (3)` - 接受通话
- `MSG_CALL_REJECT (4)` - 拒绝通话
- `MSG_CALL_END (5)` - 结束通话
- `MSG_MEDIA_DATA (6)` - 音视频媒体数据
- `MSG_HEARTBEAT (7)` - 心跳消息

## 编译与运行 (Build and Run)

### 编译 (Build)

```bash
qmake WeCompanyServer.pro
make
```

### 运行 (Run)

```bash
# 使用默认端口 8888
./bin/WeCompanyServer

# 指定端口
./bin/WeCompanyServer -p 9999

# 查看帮助
./bin/WeCompanyServer --help
```

## 使用示例 (Usage Example)

### 启动服务器 (Start Server)

```bash
./bin/WeCompanyServer -p 8888
```

服务器将在端口 8888 上监听客户端连接。

The server will listen for client connections on port 8888.

### 客户端连接流程 (Client Connection Flow)

1. 客户端连接到服务器
2. 发送注册消息（包含用户ID）
3. 开始发送/接收消息
4. 发起或接收音视频通话

### 音视频通话流程 (Audio/Video Call Flow)

1. 主叫方发送 `MSG_CALL_REQUEST` 到服务器
2. 服务器转发通话请求给被叫方
3. 被叫方回应 `MSG_CALL_ACCEPT` 或 `MSG_CALL_REJECT`
4. 通话建立后，双方通过 `MSG_MEDIA_DATA` 传输音视频数据
5. 任一方可发送 `MSG_CALL_END` 结束通话

## API 文档 (API Documentation)

### TcpServer

#### 主要方法 (Main Methods)

- `bool startServer(quint16 port)` - 启动服务器
- `void stopServer()` - 停止服务器
- `bool sendMessage(const QString &userId, const QByteArray &data)` - 发送消息给指定用户
- `void broadcastMessage(const QByteArray &data)` - 广播消息给所有在线用户
- `QList<QString> getOnlineUsers()` - 获取在线用户列表

### VideoCallServer

#### 主要方法 (Main Methods)

- `bool initiateCall(const QString &caller, const QString &callee, bool isVideo)` - 发起通话
- `bool acceptCall(const QString &callId)` - 接受通话
- `bool rejectCall(const QString &callId, const QString &reason)` - 拒绝通话
- `bool endCall(const QString &callId)` - 结束通话
- `void relayMediaData(const QString &callId, const QString &fromUser, const QByteArray &mediaData)` - 转发媒体数据

## 系统要求 (System Requirements)

- Qt 5.9 或更高版本
- C++11 支持
- 支持的操作系统：Windows, Linux, macOS

## 许可证 (License)

查看根目录下的 LICENSE 文件。

See LICENSE file in the root directory.

## 贡献 (Contributing)

欢迎提交问题和拉取请求！

Issues and pull requests are welcome!
