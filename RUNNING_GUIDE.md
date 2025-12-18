# WeCompany 项目完整运行指南

## Complete Running Guide for WeCompany Project

本文档提供 WeCompany 项目的完整运行步骤，包括服务器和客户端的安装、配置和启动。

This document provides complete instructions for running the WeCompany project, including server and client installation, configuration, and startup.

---

## 目录 (Table of Contents)

1. [系统要求](#系统要求-system-requirements)
2. [环境准备](#环境准备-environment-setup)
3. [数据库配置](#数据库配置-database-setup)
4. [编译项目](#编译项目-building-the-project)
5. [运行服务器](#运行服务器-running-the-server)
6. [运行客户端](#运行客户端-running-the-client)
7. [测试功能](#测试功能-testing-features)
8. [常见问题](#常见问题-troubleshooting)

---

## 系统要求 (System Requirements)

### 硬件要求
- CPU: 双核 2.0GHz 或更高
- 内存: 4GB RAM 或更多
- 硬盘: 至少 1GB 可用空间

### 软件要求

#### 必需组件
- **Qt 5.9 或更高版本**
  - Qt Core
  - Qt GUI
  - Qt Widgets
  - Qt Network
  - Qt SQL (用于数据库功能)
  - Qt WebEngineWidgets
  - Qt WebChannel
  - Qt Multimedia
  
- **C++ 编译器**
  - Linux: GCC 5.0+ 或 Clang 3.8+
  - Windows: MSVC 2015+ 或 MinGW
  - macOS: Xcode 8.0+ (Clang)

- **MySQL 5.7+ 或 MariaDB 10.2+** (可选，用于数据持久化)

#### 可选组件
- **声网 Agora SDK** (可选，用于专业音视频功能)
- **Git** (用于克隆代码)

---

## 环境准备 (Environment Setup)

### 1. 安装 Qt

#### Linux (Ubuntu/Debian)

```bash
# 安装 Qt 5 开发工具
sudo apt-get update
sudo apt-get install qt5-default qtbase5-dev qtwebengine5-dev \
  libqt5webchannel5-dev libqt5multimedia5 libqt5sql5-mysql

# 验证安装
qmake --version
```

#### Linux (CentOS/RHEL)

```bash
# 安装 Qt 5
sudo yum install qt5-qtbase-devel qt5-qtwebengine-devel \
  qt5-qtwebchannel-devel qt5-qtmultimedia-devel

# 设置环境变量
export PATH=/usr/lib64/qt5/bin:$PATH
```

#### macOS

```bash
# 使用 Homebrew 安装
brew install qt@5

# 设置环境变量
export PATH="/usr/local/opt/qt@5/bin:$PATH"
export LDFLAGS="-L/usr/local/opt/qt@5/lib"
export CPPFLAGS="-I/usr/local/opt/qt@5/include"
```

#### Windows

1. 从 [Qt 官网](https://www.qt.io/download) 下载 Qt 安装器
2. 运行安装器，选择以下组件：
   - Qt 5.9 或更高版本
   - MSVC 2015 或 MinGW
   - Qt Creator (推荐)
3. 将 Qt bin 目录添加到 PATH 环境变量

### 2. 安装 MySQL (可选但推荐)

#### Linux (Ubuntu/Debian)

```bash
# 安装 MySQL Server
sudo apt-get install mysql-server

# 启动 MySQL 服务
sudo systemctl start mysql
sudo systemctl enable mysql

# 安全配置
sudo mysql_secure_installation
```

#### macOS

```bash
# 使用 Homebrew 安装
brew install mysql

# 启动 MySQL
brew services start mysql

# 安全配置
mysql_secure_installation
```

#### Windows

1. 从 [MySQL 官网](https://dev.mysql.com/downloads/mysql/) 下载安装器
2. 运行安装器，选择 "Developer Default"
3. 设置 root 密码
4. 确保 MySQL 服务正在运行

### 3. 克隆项目代码

```bash
# 克隆仓库
git clone https://github.com/dd2006tqf/WeCompany.git
cd WeCompany

# 切换到开发分支
git checkout copilot/add-video-audio-communication
```

---

## 数据库配置 (Database Setup)

### 1. 创建数据库

```bash
# 登录 MySQL
mysql -u root -p

# 在 MySQL 命令行中执行：
```

```sql
-- 创建数据库
CREATE DATABASE wecompany CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

-- 创建专用用户 (推荐)
CREATE USER 'wecompany_user'@'localhost' IDENTIFIED BY 'your_password';
GRANT ALL PRIVILEGES ON wecompany.* TO 'wecompany_user'@'localhost';
FLUSH PRIVILEGES;

-- 退出 MySQL
EXIT;
```

### 2. 初始化数据库表

```bash
# 使用提供的 SQL 脚本初始化表结构
mysql -u wecompany_user -p wecompany < server/database_setup.sql

# 或者使用 root 用户
mysql -u root -p wecompany < server/database_setup.sql
```

### 3. 验证数据库

```bash
# 登录并检查表
mysql -u wecompany_user -p wecompany

# 在 MySQL 中执行：
SHOW TABLES;
# 应该看到: users, user_profiles, friend_relations, offline_messages

DESCRIBE users;
# 查看表结构

EXIT;
```

---

## 编译项目 (Building the Project)

### 方法 1: 使用命令行编译

#### 编译服务器

```bash
# 进入项目目录
cd /path/to/WeCompany

# 生成 Makefile
qmake WeCompanyServer.pro

# 编译
make

# 或者在 Windows 上使用 nmake (MSVC) 或 mingw32-make (MinGW)
# nmake
# 或
# mingw32-make
```

#### 编译客户端

```bash
# 在同一目录中
qmake WeComCopy.pro

# 编译
make

# Windows 用户同样使用对应的 make 命令
```

### 方法 2: 使用 Qt Creator (推荐)

1. **打开 Qt Creator**
2. **打开项目文件**:
   - File → Open File or Project
   - 选择 `WeCompanyServer.pro` (服务器) 或 `WeComCopy.pro` (客户端)
3. **配置构建**:
   - 选择 Qt 版本和编译器
   - 选择 Debug 或 Release 模式
4. **编译**:
   - Build → Build Project (Ctrl+B)
5. **运行**:
   - Build → Run (Ctrl+R)

### 验证编译结果

```bash
# 检查生成的可执行文件
ls -la bin/

# 应该看到:
# - WeCompanyServer (或 WeCompanyServer.exe)
# - WeComCopy (或 WeComCopy.exe)
```

---

## 运行服务器 (Running the Server)

### 基本启动 (无数据库)

```bash
cd bin
./WeCompanyServer -p 8888
```

### 完整配置启动 (推荐)

```bash
./WeCompanyServer \
  -p 8888 \
  --db-host localhost \
  --db-port 3306 \
  --db-name wecompany \
  --db-user wecompany_user \
  --db-pass your_password
```

### 使用 Agora SDK (可选)

```bash
./WeCompanyServer \
  -p 8888 \
  --db-host localhost \
  --db-port 3306 \
  --db-name wecompany \
  --db-user wecompany_user \
  --db-pass your_password \
  --agora-appid YOUR_AGORA_APP_ID \
  --agora-cert YOUR_AGORA_CERTIFICATE
```

### 服务器命令行参数

| 参数 | 说明 | 默认值 | 示例 |
|------|------|--------|------|
| `-p, --port` | 服务器监听端口 | 8888 | `-p 9999` |
| `--db-host` | MySQL 主机地址 | localhost | `--db-host 192.168.1.100` |
| `--db-port` | MySQL 端口 | 3306 | `--db-port 3307` |
| `--db-name` | 数据库名称 | wecompany | `--db-name mydb` |
| `--db-user` | 数据库用户名 | root | `--db-user admin` |
| `--db-pass` | 数据库密码 | (空) | `--db-pass mypassword` |
| `--agora-appid` | Agora App ID | (空) | `--agora-appid abc123...` |
| `--agora-cert` | Agora Certificate | (空) | `--agora-cert def456...` |

### 服务器启动成功提示

```
========================================
WeCompany Server v2.0 started
Port: 8888
Database: Enabled
Agora SDK: Disabled
========================================
Press Ctrl+C to stop the server
```

### 后台运行服务器 (Linux/macOS)

```bash
# 使用 nohup
nohup ./WeCompanyServer -p 8888 \
  --db-host localhost \
  --db-name wecompany \
  --db-user wecompany_user \
  --db-pass your_password > server.log 2>&1 &

# 查看日志
tail -f server.log

# 停止服务器
ps aux | grep WeCompanyServer
kill <PID>
```

---

## 运行客户端 (Running the Client)

### 启动客户端

```bash
cd bin
./WeComCopy

# Windows 用户
# WeComCopy.exe
```

### 首次使用 - 注册账号

1. **启动客户端**后会自动显示登录界面
2. **注册新账号**:
   - 点击 "注册新账号" 按钮
   - 输入用户名 (例如: `test_user`)
   - 输入密码 (至少6位，例如: `password123`)
   - 点击 "注册新账号" 完成注册
3. **注册成功**后会自动登录

### 登录已有账号

1. 输入用户名
2. 输入密码
3. (可选) 勾选 "记住密码"
4. 点击 "登录" 按钮

### 客户端配置

客户端默认连接到 `127.0.0.1:8888`。如需修改：

```cpp
// 在 source/serverlogindlg.cpp 中修改
m_serverHost = "your_server_ip";  // 第17行
m_serverPort = 8888;               // 第18行
```

重新编译后运行。

---

## 测试功能 (Testing Features)

### 测试 1: 用户注册和登录

#### 步骤
1. 启动服务器
2. 启动第一个客户端
3. 注册账号: `user1` / `password123`
4. 观察服务器日志，应显示:
   ```
   Client connected: user1
   User authenticated: user1 Token issued
   ```
5. 启动第二个客户端
6. 注册账号: `user2` / `password123`

#### 预期结果
- ✅ 注册成功提示
- ✅ 自动登录到主界面
- ✅ 服务器显示两个客户端连接

### 测试 2: 拨打电话

#### 步骤
1. 在 `user1` 的客户端，找到联系人 `user2`
2. 点击"通话"按钮（音频或视频）
3. `user1` 看到 **拨打电话界面** (OutgoingCallDialog)
4. `user2` 看到 **来电界面** (IncomingCallDialog)
5. `user2` 点击绿色"接听"按钮
6. 双方进入 **通话中界面** (ActiveCallDialog)

#### 预期结果
- ✅ 界面正确显示
- ✅ 通话时长计时器工作
- ✅ 服务器日志显示呼叫流程

### 测试 3: 通话功能

#### 步骤
1. 在通话中界面:
   - 点击"静音"按钮 → 图标变化
   - 点击"视频"按钮 → 图标变化
   - 观察通话时长计时
2. 点击"挂断"按钮结束通话

#### 预期结果
- ✅ 按钮状态正确切换
- ✅ 时长准确计时
- ✅ 挂断后正确返回主界面

### 测试 4: 数据库持久化

#### 步骤
1. 注册多个用户
2. 停止服务器
3. 登录 MySQL 查看数据:
   ```sql
   USE wecompany;
   SELECT * FROM users;
   SELECT * FROM user_profiles;
   ```
4. 重启服务器
5. 客户端可以用之前的账号登录

#### 预期结果
- ✅ 数据保存在数据库
- ✅ 服务器重启后数据仍然存在
- ✅ 可以用旧账号登录

---

## 常见问题 (Troubleshooting)

### 问题 1: 编译错误 - Qt 模块找不到

**错误信息:**
```
Project ERROR: Unknown module(s) in QT: webenginewidgets
```

**解决方案:**
```bash
# Linux
sudo apt-get install qtwebengine5-dev

# macOS
brew install qt5

# 或者在 .pro 文件中注释掉不需要的模块
```

### 问题 2: 服务器无法启动 - 端口被占用

**错误信息:**
```
Failed to start server on port 8888
```

**解决方案:**
```bash
# 检查端口占用
netstat -tuln | grep 8888
# 或
lsof -i :8888

# 杀死占用进程
kill <PID>

# 或使用不同端口
./WeCompanyServer -p 9999
```

### 问题 3: 客户端无法连接服务器

**错误信息:**
```
无法连接到服务器，请检查服务器是否运行
```

**检查清单:**
1. ✅ 服务器是否正在运行？
   ```bash
   ps aux | grep WeCompanyServer
   ```
2. ✅ 防火墙是否开放端口？
   ```bash
   # Linux
   sudo ufw allow 8888
   # 或
   sudo iptables -A INPUT -p tcp --dport 8888 -j ACCEPT
   ```
3. ✅ 客户端配置的服务器地址是否正确？
4. ✅ 使用 telnet 测试连接:
   ```bash
   telnet localhost 8888
   ```

### 问题 4: MySQL 连接失败

**错误信息:**
```
Failed to connect to database: Access denied
```

**解决方案:**
1. 验证数据库凭据:
   ```bash
   mysql -u wecompany_user -p wecompany
   ```
2. 检查用户权限:
   ```sql
   SHOW GRANTS FOR 'wecompany_user'@'localhost';
   ```
3. 重新授权:
   ```sql
   GRANT ALL PRIVILEGES ON wecompany.* TO 'wecompany_user'@'localhost';
   FLUSH PRIVILEGES;
   ```

### 问题 5: Qt SQL 驱动找不到

**错误信息:**
```
QSqlDatabase: QMYSQL driver not loaded
```

**解决方案:**
```bash
# Linux (Ubuntu/Debian)
sudo apt-get install libqt5sql5-mysql

# macOS
# Qt 通常包含 MySQL 驱动

# 验证驱动
./WeCompanyServer --help  # 如果能运行说明基本环境OK
```

### 问题 6: 界面显示异常

**可能原因:**
- 图标字体未正确加载
- 资源文件缺失

**解决方案:**
1. 确认资源文件存在:
   ```bash
   ls -la res/
   # 应该有 WeComCopy.png, usricon.jpeg 等
   ```
2. 重新编译 qrc 资源:
   ```bash
   qmake WeComCopy.pro
   make clean
   make
   ```

---

## 快速开始命令 (Quick Start Commands)

### Linux/macOS 一键启动

```bash
#!/bin/bash
# 文件名: start_wecompany.sh

# 启动服务器 (后台)
cd bin
./WeCompanyServer -p 8888 \
  --db-host localhost \
  --db-name wecompany \
  --db-user root \
  --db-pass your_password > server.log 2>&1 &

echo "服务器已启动，PID: $!"
echo "日志文件: server.log"

# 等待2秒让服务器启动
sleep 2

# 启动客户端
./WeComCopy &

echo "客户端已启动"
echo ""
echo "要停止服务器，运行: kill $(pgrep WeCompanyServer)"
```

使用方法:
```bash
chmod +x start_wecompany.sh
./start_wecompany.sh
```

### Windows 批处理脚本

```batch
@echo off
REM 文件名: start_wecompany.bat

echo 启动 WeCompany 服务器...
cd bin
start "WeCompany Server" WeCompanyServer.exe -p 8888 --db-host localhost --db-name wecompany --db-user root --db-pass your_password

timeout /t 3

echo 启动 WeCompany 客户端...
start "WeCompany Client" WeComCopy.exe

echo.
echo WeCompany 已启动！
echo.
pause
```

---

## 开发调试模式 (Development Debug Mode)

### 启用详细日志

在服务器或客户端代码中添加:

```cpp
// 在 main.cpp 开头
#include <QDebug>
qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss}] [%{type}] %{message}");
```

### 使用调试器

#### Qt Creator
1. 设置断点 (点击行号左侧)
2. Debug → Start Debugging (F5)
3. 使用调试控制台查看变量

#### GDB (Linux/macOS)
```bash
gdb ./bin/WeCompanyServer
(gdb) run -p 8888
(gdb) bt  # 查看堆栈
```

---

## 性能优化建议

### 服务器优化
1. 使用 Release 模式编译
2. 增加系统文件描述符限制:
   ```bash
   ulimit -n 65535
   ```
3. 优化 MySQL 配置
4. 使用连接池

### 客户端优化
1. 减少不必要的界面刷新
2. 使用图片缓存
3. 异步加载好友列表

---

## 更多资源

### 文档
- [服务器 API 文档](server/API.md)
- [客户端 UI 指南](CLIENT_UI_GUIDE.md)
- [构建部署指南](server/BUILD_DEPLOY.md)
- [增强功能文档](server/ENHANCED_FEATURES.md)

### 社区
- GitHub Issues: https://github.com/dd2006tqf/WeCompany/issues
- 项目 Wiki (如有)

### 声网 SDK 资源
- 声网官网: https://www.agora.io/cn/
- 声网文档: https://docs.agora.io/cn/

---

## 许可证

查看项目根目录的 LICENSE 文件。

---

**祝您使用愉快！如有问题，请提交 Issue。**
