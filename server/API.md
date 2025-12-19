# WeCompany Server API Documentation

## 概述 (Overview)

本文档描述 WeCompany 服务器的消息协议和 API。

This document describes the message protocol and API of the WeCompany Server.

## 消息协议 (Message Protocol)

### 基本格式 (Basic Format)

所有消息遵循以下格式：

```
[MessageType (1 byte)][Payload]
```

- **MessageType**: 消息类型，占 1 字节
- **Payload**: 消息内容，格式取决于消息类型

### 数据编码 (Data Encoding)

使用 Qt 的 QDataStream (版本 Qt_5_9) 进行序列化：

- 整数: Big-endian 字节序
- 字符串: UTF-8 编码，前缀长度（2字节）
- 二进制数据: 直接写入

## 消息类型 (Message Types)

### 0. MSG_TEXT - 文本消息

用于发送文本消息给其他用户。

**客户端 -> 服务器:**

```
[0x00][RecipientId length (2 bytes)][RecipientId][Text length (2 bytes)][Text]
```

**服务器 -> 客户端:**

```
[0x00][SenderId length (2 bytes)][SenderId][Text length (2 bytes)][Text]
```

### 1. MSG_FILE - 文件传输

用于文件传输（当前版本未完全实现）。

### 2. MSG_CALL_REQUEST - 通话请求

发起音频或视频通话。

**客户端 -> 服务器:**

```
[0x02][CalleeId length (2 bytes)][CalleeId][IsVideo (1 byte)]
```

- **CalleeId**: 被叫方用户ID
- **IsVideo**: 1 = 视频通话, 0 = 音频通话

**服务器 -> 被叫方:**

```
[0x02][CallId length (2 bytes)][CallId][CallerId length (2 bytes)][CallerId][IsVideo (1 byte)]
```

- **CallId**: 通话会话ID（UUID）
- **CallerId**: 主叫方用户ID
- **IsVideo**: 1 = 视频通话, 0 = 音频通话

### 3. MSG_CALL_ACCEPT - 接受通话

接受来电。

**客户端 -> 服务器:**

```
[0x03][CallId length (2 bytes)][CallId]
```

**服务器 -> 双方:**

```
[0x03][CallId length (2 bytes)][CallId]
```

### 4. MSG_CALL_REJECT - 拒绝通话

拒绝来电。

**客户端 -> 服务器:**

```
[0x04][CallId length (2 bytes)][CallId][Reason length (2 bytes)][Reason]
```

- **Reason**: 拒绝原因（可选）

**服务器 -> 双方:**

```
[0x04][CallId length (2 bytes)][CallId][Reason length (2 bytes)][Reason]
```

### 5. MSG_CALL_END - 结束通话

结束当前通话。

**客户端 -> 服务器:**

```
[0x05][CallId length (2 bytes)][CallId]
```

**服务器 -> 双方:**

```
[0x05][CallId length (2 bytes)][CallId]
```

### 6. MSG_MEDIA_DATA - 媒体数据

传输音频或视频数据。

**客户端 -> 服务器:**

```
[0x06][MediaData]
```

**服务器 -> 对方:**

```
[0x06][CallId length (2 bytes)][CallId][MediaData]
```

- **MediaData**: 音频/视频编码数据

### 7. MSG_HEARTBEAT - 心跳消息

保持连接活跃。

**客户端 -> 服务器:**

```
[0x07]
```

**服务器 -> 客户端:**

```
[0x07]
```

## 连接流程 (Connection Flow)

### 1. 客户端注册

客户端连接到服务器后，第一个消息必须包含用户ID：

```
[MessageType (any)][UserId length (2 bytes)][UserId][Additional data...]
```

服务器接收到此消息后会注册该客户端。

### 2. 保持连接

建议每 30 秒发送一次心跳消息：

```
[0x07]
```

## 通话流程 (Call Flow)

### 音频/视频通话流程

```
主叫方                    服务器                    被叫方
  |                         |                         |
  |--MSG_CALL_REQUEST------>|                         |
  |                         |--MSG_CALL_REQUEST------>|
  |                         |                         |
  |                         |<--MSG_CALL_ACCEPT-------|
  |<--MSG_CALL_ACCEPT-------|--MSG_CALL_ACCEPT------->|
  |                         |                         |
  |--MSG_MEDIA_DATA-------->|--MSG_MEDIA_DATA-------->|
  |<--MSG_MEDIA_DATA--------|<--MSG_MEDIA_DATA--------|
  |                         |                         |
  |--MSG_CALL_END---------->|                         |
  |                         |--MSG_CALL_END---------->|
  |<--MSG_CALL_END----------|<--MSG_CALL_END----------|
```

## API 示例 (API Examples)

### 注册客户端

```cpp
QByteArray registerMessage;
QDataStream stream(&registerMessage, QIODevice::WriteOnly);
stream.setVersion(QDataStream::Qt_5_9);

stream << static_cast<quint8>(MSG_HEARTBEAT);
QString userId = "user123";
QByteArray userIdData = userId.toUtf8();
stream << static_cast<quint16>(userIdData.size());
stream.writeRawData(userIdData.data(), userIdData.size());

socket->write(registerMessage);
```

### 发起视频通话

```cpp
QByteArray callRequest;
QDataStream stream(&callRequest, QIODevice::WriteOnly);
stream.setVersion(QDataStream::Qt_5_9);

stream << static_cast<quint8>(MSG_CALL_REQUEST);
stream << QString("targetUser");
stream << static_cast<quint8>(1); // 1 for video, 0 for audio

socket->write(callRequest);
```

### 接受通话

```cpp
QByteArray acceptMessage;
QDataStream stream(&acceptMessage, QIODevice::WriteOnly);
stream.setVersion(QDataStream::Qt_5_9);

stream << static_cast<quint8>(MSG_CALL_ACCEPT);
stream << callId; // callId from MSG_CALL_REQUEST

socket->write(acceptMessage);
```

### 发送媒体数据

```cpp
QByteArray mediaMessage;
QDataStream stream(&mediaMessage, QIODevice::WriteOnly);
stream.setVersion(QDataStream::Qt_5_9);

stream << static_cast<quint8>(MSG_MEDIA_DATA);
stream.writeRawData(audioVideoData.data(), audioVideoData.size());

socket->write(mediaMessage);
```

## 错误处理 (Error Handling)

### 常见错误

1. **用户不在线** - 目标用户未连接到服务器
2. **用户忙碌** - 目标用户正在通话中
3. **通话ID无效** - 指定的通话会话不存在
4. **连接断开** - 网络连接中断

### 错误响应

当前版本服务器主要通过日志记录错误，未来版本可能会添加错误响应消息。

## 限制和注意事项 (Limitations and Notes)

1. **单点对单点** - 当前仅支持一对一通话
2. **无加密** - 消息未加密传输（建议在生产环境使用 SSL/TLS）
3. **无认证** - 简单的用户ID注册，无密码验证
4. **媒体格式** - 服务器不处理媒体编解码，仅转发数据
5. **连接管理** - 建议实现重连机制处理网络中断

## 扩展建议 (Extension Suggestions)

1. **认证系统** - 添加用户名/密码或 token 认证
2. **加密通信** - 实现 SSL/TLS 支持
3. **群组通话** - 支持多方通话
4. **消息队列** - 离线消息存储和推送
5. **文件传输** - 完整的文件传输实现
6. **状态同步** - 用户在线状态、通话状态等
7. **管理接口** - 服务器监控和管理 API

## 性能指标 (Performance Metrics)

### 推荐配置

- **最大并发连接**: 1000 (可配置)
- **心跳间隔**: 30 秒
- **通话超时**: 30 秒（无响应）
- **媒体包大小**: 建议 < 64KB

### 带宽估算

- **音频通话**: ~32 Kbps (4 KB/s)
- **视频通话**: ~512 Kbps - 2 Mbps (64-256 KB/s)
- **文本消息**: 可忽略

## 版本历史 (Version History)

### v1.0 (Current)

- TCP 服务器基础功能
- 一对一音视频通话支持
- 基本消息转发
- 呼叫信令实现

## 技术支持 (Technical Support)

如有问题或建议，请在 GitHub 提交 Issue。

For questions or suggestions, please submit an Issue on GitHub.
