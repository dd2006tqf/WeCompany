# WeCompany Server v2.0 - Enhanced Features

## 概述 (Overview)

WeCompany Server v2.0 在原有 TCP 服务器和音视频通话基础上，新增了以下功能：

1. **用户认证系统** - 登录验证、Token 生成和分发
2. **MySQL 数据库集成** - 用户账号、好友关系、消息持久化
3. **声网 SDK 集成** - 专业音视频通话支持

## 新增功能 (New Features)

### 1. 用户认证系统 (Authentication System)

#### 功能特性

- 用户注册和登录
- JWT 风格的 Token 生成
- Token 验证和管理
- 密码加密（SHA-256 + Salt）
- Token 自动过期和清理

#### API 使用

**注册用户**

```cpp
AuthManager authManager;
QString userId;
bool success = authManager.registerUser("username", "password", userId);
```

**用户登录**

```cpp
QString token = authManager.authenticateUser("username", "password");
if (!token.isEmpty()) {
    // Login successful, use token for subsequent requests
}
```

**验证 Token**

```cpp
if (authManager.validateToken(token)) {
    QString userId = authManager.getUserIdFromToken(token);
    // Token is valid
}
```

### 2. MySQL 数据库集成 (MySQL Database Integration)

#### 数据库表结构 (Database Schema)

**users 表** - 用户账号信息

```sql
CREATE TABLE users (
    user_id VARCHAR(36) PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash VARCHAR(64) NOT NULL,
    salt VARCHAR(32) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_username (username)
);
```

**user_profiles 表** - 用户详细信息

```sql
CREATE TABLE user_profiles (
    user_id VARCHAR(36) PRIMARY KEY,
    nickname VARCHAR(100),
    avatar VARCHAR(255),
    email VARCHAR(100),
    phone VARCHAR(20),
    status VARCHAR(20) DEFAULT 'offline',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);
```

**friend_relations 表** - 好友关系

```sql
CREATE TABLE friend_relations (
    relation_id VARCHAR(36) PRIMARY KEY,
    user_id VARCHAR(36) NOT NULL,
    friend_id VARCHAR(36) NOT NULL,
    remark VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    UNIQUE KEY unique_friendship (user_id, friend_id),
    INDEX idx_user_id (user_id),
    INDEX idx_friend_id (friend_id),
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (friend_id) REFERENCES users(user_id) ON DELETE CASCADE
);
```

**offline_messages 表** - 离线消息

```sql
CREATE TABLE offline_messages (
    message_id VARCHAR(36) PRIMARY KEY,
    from_user_id VARCHAR(36) NOT NULL,
    to_user_id VARCHAR(36) NOT NULL,
    content TEXT NOT NULL,
    message_type VARCHAR(20) DEFAULT 'text',
    sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    delivered BOOLEAN DEFAULT FALSE,
    INDEX idx_to_user (to_user_id, delivered),
    INDEX idx_sent_at (sent_at),
    FOREIGN KEY (from_user_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (to_user_id) REFERENCES users(user_id) ON DELETE CASCADE
);
```

#### 分段加载策略 (Segmented Loading Strategy)

好友列表采用分页加载，优化大量好友时的响应速度：

```cpp
DatabaseManager dbManager;

// 获取第一页好友（0-50）
QList<FriendRelation> friends = dbManager.getFriendList(userId, 0, 50);

// 获取第二页好友（50-100）
QList<FriendRelation> moreFriends = dbManager.getFriendList(userId, 50, 50);

// 获取好友总数
int totalFriends = dbManager.getFriendCount(userId);
```

#### API 使用

**连接数据库**

```cpp
DatabaseManager dbManager;
bool connected = dbManager.connectToDatabase(
    "localhost",  // host
    3306,         // port
    "wecompany",  // database name
    "root",       // username
    "password"    // password
);

if (connected) {
    dbManager.initializeDatabase();
}
```

**用户管理**

```cpp
// 创建用户
dbManager.createUser(userId, username, passwordHash, salt);

// 获取用户资料
UserProfile profile = dbManager.getUserProfile(userId);

// 更新用户状态
dbManager.updateUserStatus(userId, "online");
```

**好友管理**

```cpp
// 添加好友关系
dbManager.addFriendRelation(userId, friendId);

// 获取好友列表
QList<FriendRelation> friends = dbManager.getFriendList(userId);

// 检查好友关系
bool isFriend = dbManager.areFriends(userId, friendId);
```

**消息管理**

```cpp
// 保存离线消息
OfflineMessage msg;
msg.messageId = generateId();
msg.fromUserId = senderId;
msg.toUserId = receiverId;
msg.content = "Hello!";
msg.messageType = "text";
dbManager.saveOfflineMessage(msg);

// 获取离线消息
QList<OfflineMessage> messages = dbManager.getOfflineMessages(userId);

// 标记消息已送达
dbManager.markMessagesAsDelivered(userId);
```

### 3. 声网 SDK 集成 (Agora SDK Integration)

#### 功能特性

- Agora RTC Token 生成
- 频道创建和管理
- 1v1 低延迟音视频通话
- PCM 音频文件存储
- TCP 音频数据转发

#### 环境配置

1. 注册声网账号：https://console.agora.io/
2. 创建项目获取 App ID 和 App Certificate
3. （可选）下载并安装 Agora C++ SDK

#### API 使用

**初始化 Agora Manager**

```cpp
AgoraManager agoraManager;

AgoraConfig config;
config.appId = "your_agora_app_id";
config.appCertificate = "your_agora_certificate";
config.tokenExpirationSeconds = 3600;
config.enableAudio = true;
config.enableVideo = true;

agoraManager.initialize(config);
```

**生成 Token**

```cpp
// 生成 RTC Token（用于音视频通话）
QString rtcToken = agoraManager.generateRtcToken(channelName, userId, 3600);

// 生成 RTM Token（用于实时消息）
QString rtmToken = agoraManager.generateRtmToken(userId, 3600);
```

**频道管理**

```cpp
// 创建频道
QString channelId = agoraManager.createChannel(hostUserId, guestUserId);

// 用户加入频道
agoraManager.joinChannel(channelId, userId);

// 用户离开频道
agoraManager.leaveChannel(channelId, userId);

// 关闭频道
agoraManager.closeChannel(channelId);
```

**音频文件管理**

```cpp
// 保存 PCM 音频文件
QByteArray pcmData = ...; // PCM audio data
agoraManager.saveAudioPCM(userId, pcmData, "recording_001.pcm");

// 加载 PCM 音频文件
QByteArray audioData = agoraManager.loadAudioPCM("userId_recording_001.pcm");
```

**音频数据转发**

```cpp
// 转发音频数据
agoraManager.forwardAudioData(channelId, fromUserId, audioData);

// 接收转发的音频数据
connect(&agoraManager, &AgoraManager::audioDataReceived,
        [](const QString &channelId, const QString &userId, const QByteArray &data) {
    // 处理接收到的音频数据
    playAudio(data);
});
```

## 服务器启动 (Server Startup)

### 基本启动

```bash
./bin/WeCompanyServer -p 8888
```

### 完整配置启动

```bash
./bin/WeCompanyServer \
  -p 8888 \
  --db-host localhost \
  --db-port 3306 \
  --db-name wecompany \
  --db-user root \
  --db-pass mypassword \
  --agora-appid your_app_id \
  --agora-cert your_certificate
```

### 命令行参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-p, --port` | 服务器端口 | 8888 |
| `--db-host` | MySQL 主机地址 | localhost |
| `--db-port` | MySQL 端口 | 3306 |
| `--db-name` | 数据库名称 | wecompany |
| `--db-user` | 数据库用户名 | root |
| `--db-pass` | 数据库密码 | (空) |
| `--agora-appid` | Agora App ID | (空) |
| `--agora-cert` | Agora App Certificate | (空) |

## 数据库配置 (Database Setup)

### 1. 安装 MySQL

**Ubuntu/Debian:**

```bash
sudo apt-get update
sudo apt-get install mysql-server
sudo mysql_secure_installation
```

**CentOS/RHEL:**

```bash
sudo yum install mysql-server
sudo systemctl start mysqld
sudo mysql_secure_installation
```

### 2. 创建数据库

```sql
CREATE DATABASE wecompany CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER 'wecompany_user'@'localhost' IDENTIFIED BY 'your_password';
GRANT ALL PRIVILEGES ON wecompany.* TO 'wecompany_user'@'localhost';
FLUSH PRIVILEGES;
```

### 3. 安装 Qt MySQL 驱动

**Ubuntu/Debian:**

```bash
sudo apt-get install libqt5sql5-mysql
```

**macOS:**

```bash
brew install qt5
# MySQL driver is usually included
```

## 性能优化 (Performance Optimization)

### 数据库优化

1. **索引优化** - 已在表结构中添加关键索引
2. **连接池** - 使用 Qt 的连接池管理
3. **查询优化** - 使用 prepared statements
4. **分页加载** - 好友列表分页，默认每页 50 条

### 内存优化

1. **定期清理** - 自动清理过期 Token 和旧消息
2. **资源释放** - 正确使用 Qt 的父子对象机制
3. **缓存策略** - 热点数据缓存

### 网络优化

1. **异步 I/O** - Qt 事件循环
2. **消息队列** - 离线消息队列
3. **数据压缩** - 可选的数据压缩传输

## 安全建议 (Security Recommendations)

1. **密码安全**
   - 使用 SHA-256 + 随机 Salt
   - Salt 长度 16 字节
   - 不存储明文密码

2. **Token 安全**
   - Token 有效期 24 小时
   - 定期清理过期 Token
   - 使用 HTTPS 传输 Token

3. **数据库安全**
   - 使用专用数据库用户
   - 限制数据库访问权限
   - 定期备份数据

4. **网络安全**
   - 建议使用 SSL/TLS
   - 实施 IP 白名单
   - 限流防止 DDoS

## 监控和维护 (Monitoring and Maintenance)

### 日志记录

服务器会记录以下信息：

- 用户连接/断开
- 认证成功/失败
- 数据库操作错误
- 通话创建/结束
- Token 生成和验证

### 定期维护任务

服务器自动执行以下维护任务：

- 每小时清理过期 Token
- 每小时删除 30 天前的已送达消息
- 数据库连接检查

## 故障排查 (Troubleshooting)

### 数据库连接失败

```
Error: Failed to connect to database: Access denied
```

**解决方案：**
- 检查数据库用户名和密码
- 确认数据库服务运行中
- 检查防火墙设置
- 验证 MySQL 驱动已安装

### Agora 初始化失败

```
Warning: Failed to initialize Agora SDK
```

**解决方案：**
- 验证 App ID 和 Certificate 正确
- 检查网络连接
- 确认 Agora SDK 库已安装（如使用完整 SDK）

### Token 验证失败

```
Warning: Invalid password for user: xxx
```

**解决方案：**
- 确认用户名和密码正确
- 检查数据库中用户数据
- 验证密码加密算法一致

## 扩展开发 (Extension Development)

### 添加新消息类型

1. 在消息协议中定义新类型
2. 更新 TcpServer 的消息处理
3. 更新数据库表结构（如需持久化）

### 集成其他服务

1. 创建新的 Manager 类
2. 在 servermain.cpp 中初始化
3. 连接必要的信号和槽

### 实现集群部署

1. 使用 Redis 共享会话
2. 消息队列（RabbitMQ/Kafka）
3. 负载均衡器（Nginx/HAProxy）

## 版本历史 (Version History)

### v2.0 (Current)

- ✅ 用户认证系统
- ✅ MySQL 数据库集成
- ✅ 声网 SDK 支持
- ✅ Token 生成和验证
- ✅ 离线消息存储
- ✅ 好友关系管理
- ✅ PCM 音频存储

### v1.0

- ✅ 基础 TCP 服务器
- ✅ 一对一音视频通话
- ✅ 基本消息转发

## 许可证 (License)

遵循项目根目录的 LICENSE 文件。

## 技术支持 (Support)

如有问题或建议，请在 GitHub 提交 Issue。
