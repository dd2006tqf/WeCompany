# 客户端接入示例 (Client Integration Example)

## C++/Qt 客户端示例

### 1. 连接到服务器

```cpp
#include <QTcpSocket>
#include <QDataStream>

QTcpSocket *socket = new QTcpSocket(this);
socket->connectToHost("127.0.0.1", 8888);

connect(socket, &QTcpSocket::connected, []() {
    qDebug() << "Connected to server";
});
```

### 2. 注册客户端

连接成功后，发送注册消息：

```cpp
void registerClient(const QString &userId) {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    // 消息类型（心跳或任意类型）
    stream << static_cast<quint8>(MSG_HEARTBEAT);
    
    // 用户ID
    QByteArray userIdData = userId.toUtf8();
    stream << static_cast<quint16>(userIdData.size());
    stream.writeRawData(userIdData.data(), userIdData.size());
    
    socket->write(message);
    socket->flush();
}
```

### 3. 发起音视频通话

```cpp
void initiateCall(const QString &callee, bool isVideoCall) {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(MSG_CALL_REQUEST);
    stream << callee;
    stream << static_cast<quint8>(isVideoCall ? 1 : 0);
    
    socket->write(message);
    socket->flush();
}
```

### 4. 接受通话

```cpp
void acceptCall(const QString &callId) {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(MSG_CALL_ACCEPT);
    stream << callId;
    
    socket->write(message);
    socket->flush();
}
```

### 5. 拒绝通话

```cpp
void rejectCall(const QString &callId, const QString &reason) {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(MSG_CALL_REJECT);
    stream << callId;
    stream << reason;
    
    socket->write(message);
    socket->flush();
}
```

### 6. 结束通话

```cpp
void endCall(const QString &callId) {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(MSG_CALL_END);
    stream << callId;
    
    socket->write(message);
    socket->flush();
}
```

### 7. 发送媒体数据

```cpp
void sendMediaData(const QByteArray &mediaData) {
    QByteArray message;
    QDataStream stream(&message, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_9);
    
    stream << static_cast<quint8>(MSG_MEDIA_DATA);
    stream.writeRawData(mediaData.data(), mediaData.size());
    
    socket->write(message);
    socket->flush();
}
```

### 8. 接收服务器消息

```cpp
connect(socket, &QTcpSocket::readyRead, [this]() {
    QByteArray data = socket->readAll();
    handleServerMessage(data);
});

void handleServerMessage(const QByteArray &data) {
    QDataStream stream(data);
    stream.setVersion(QDataStream::Qt_5_9);
    
    quint8 msgType;
    stream >> msgType;
    
    switch (static_cast<MessageType>(msgType)) {
        case MSG_CALL_REQUEST: {
            QString callId, caller;
            quint8 isVideo;
            stream >> callId >> caller >> isVideo;
            
            // 显示来电界面
            showIncomingCall(callId, caller, isVideo > 0);
            break;
        }
        
        case MSG_CALL_ACCEPT: {
            QString callId;
            stream >> callId;
            
            // 通话已被接受，开始媒体传输
            startMediaTransmission(callId);
            break;
        }
        
        case MSG_CALL_REJECT: {
            QString callId, reason;
            stream >> callId >> reason;
            
            // 通话被拒绝
            showCallRejected(reason);
            break;
        }
        
        case MSG_CALL_END: {
            QString callId;
            stream >> callId;
            
            // 通话结束
            endCurrentCall();
            break;
        }
        
        case MSG_MEDIA_DATA: {
            // 提取媒体数据并播放
            QByteArray mediaData;
            while (!stream.atEnd()) {
                char byte;
                stream.readRawData(&byte, 1);
                mediaData.append(byte);
            }
            
            playMediaData(mediaData);
            break;
        }
        
        default:
            break;
    }
}
```

## 消息类型定义

在客户端代码中也需要定义相同的消息类型：

```cpp
enum MessageType {
    MSG_TEXT = 0,
    MSG_FILE = 1,
    MSG_CALL_REQUEST = 2,
    MSG_CALL_ACCEPT = 3,
    MSG_CALL_REJECT = 4,
    MSG_CALL_END = 5,
    MSG_MEDIA_DATA = 6,
    MSG_HEARTBEAT = 7
};
```

## 注意事项

1. 所有消息使用 `QDataStream::Qt_5_9` 版本以确保兼容性
2. 第一次连接后必须发送包含用户ID的消息进行注册
3. 媒体数据需要在通话建立后才能发送
4. 建议实现心跳机制来检测连接状态
5. 处理网络异常和重连逻辑

## 完整流程示例

### 发起通话方

1. 连接服务器
2. 注册用户ID
3. 调用 `initiateCall()` 发起通话
4. 等待 `MSG_CALL_ACCEPT` 响应
5. 收到接受后，开始发送媒体数据
6. 通话结束时调用 `endCall()`

### 接收通话方

1. 连接服务器
2. 注册用户ID
3. 接收 `MSG_CALL_REQUEST` 消息
4. 用户选择接受或拒绝
5. 发送 `MSG_CALL_ACCEPT` 或 `MSG_CALL_REJECT`
6. 如果接受，开始接收和发送媒体数据
7. 通话结束时调用 `endCall()`
