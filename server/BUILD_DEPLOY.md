# WeCompany Server Build & Deployment Guide

## 编译指南 (Build Guide)

### 前置要求 (Prerequisites)

- Qt 5.9 或更高版本 (Qt 5.9 or higher)
- C++11 编译器 (C++11 compiler)
- qmake (Qt 构建工具)

### Windows 平台

#### 1. 安装 Qt

从 [Qt官网](https://www.qt.io/download) 下载并安装 Qt 5.9 或更高版本。

建议安装组件：
- Qt 5.9+ (MSVC 2015 或 MinGW)
- Qt Creator (可选)

#### 2. 编译步骤

使用命令行：

```cmd
cd WeCompany
qmake WeCompanyServer.pro
nmake        # 如果使用 MSVC
# 或
mingw32-make # 如果使用 MinGW
```

使用 Qt Creator：

1. 打开 Qt Creator
2. 文件 -> 打开文件或项目
3. 选择 `WeCompanyServer.pro`
4. 点击构建 (Build)

#### 3. 运行

```cmd
cd bin
WeCompanyServer.exe -p 8888
```

### Linux 平台

#### 1. 安装 Qt

Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install qt5-default qtbase5-dev
```

CentOS/RHEL:
```bash
sudo yum install qt5-qtbase-devel
```

Arch Linux:
```bash
sudo pacman -S qt5-base
```

#### 2. 编译步骤

```bash
cd WeCompany
qmake WeCompanyServer.pro
make -j$(nproc)
```

#### 3. 运行

```bash
cd bin
./WeCompanyServer -p 8888
```

#### 4. 后台运行 (可选)

```bash
# 使用 nohup
nohup ./WeCompanyServer -p 8888 > server.log 2>&1 &

# 或使用 systemd (推荐)
# 创建服务文件 /etc/systemd/system/wecompany-server.service
```

### macOS 平台

#### 1. 安装 Qt

使用 Homebrew:
```bash
brew install qt@5
```

#### 2. 设置环境变量

```bash
export PATH="/usr/local/opt/qt@5/bin:$PATH"
```

#### 3. 编译步骤

```bash
cd WeCompany
qmake WeCompanyServer.pro
make
```

#### 4. 运行

```bash
cd bin
./WeCompanyServer -p 8888
```

## 部署指南 (Deployment Guide)

### 服务器部署

#### 1. 防火墙配置

确保服务器端口开放：

Linux (iptables):
```bash
sudo iptables -A INPUT -p tcp --dport 8888 -j ACCEPT
sudo iptables-save > /etc/iptables/rules.v4
```

Linux (firewalld):
```bash
sudo firewall-cmd --permanent --add-port=8888/tcp
sudo firewall-cmd --reload
```

Windows 防火墙:
```cmd
netsh advfirewall firewall add rule name="WeCompany Server" dir=in action=allow protocol=TCP localport=8888
```

#### 2. Systemd 服务配置 (Linux)

创建服务文件 `/etc/systemd/system/wecompany-server.service`:

```ini
[Unit]
Description=WeCompany Server
After=network.target

[Service]
Type=simple
User=wecompany
WorkingDirectory=/opt/wecompany
ExecStart=/opt/wecompany/bin/WeCompanyServer -p 8888
Restart=on-failure
RestartSec=10
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
```

启用并启动服务：

```bash
sudo systemctl daemon-reload
sudo systemctl enable wecompany-server
sudo systemctl start wecompany-server
sudo systemctl status wecompany-server
```

查看日志：

```bash
sudo journalctl -u wecompany-server -f
```

#### 3. 使用 Docker 部署 (可选)

创建 Dockerfile:

```dockerfile
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    qt5-default \
    qtbase5-dev \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY server/ ./server/
COPY WeCompanyServer.pro .

RUN qmake WeCompanyServer.pro && make

EXPOSE 8888

CMD ["./bin/WeCompanyServer", "-p", "8888"]
```

构建和运行：

```bash
docker build -t wecompany-server .
docker run -d -p 8888:8888 --name wecompany wecompany-server
```

### 性能优化

#### 1. 系统参数调优 (Linux)

编辑 `/etc/sysctl.conf`:

```bash
# 增加最大文件描述符
fs.file-max = 65535

# TCP 优化
net.ipv4.tcp_max_syn_backlog = 8192
net.core.somaxconn = 1024
net.core.netdev_max_backlog = 5000
```

应用设置：
```bash
sudo sysctl -p
```

#### 2. 资源限制

编辑 `/etc/security/limits.conf`:

```
wecompany soft nofile 65535
wecompany hard nofile 65535
```

### 监控和维护

#### 1. 日志管理

- 使用 `logrotate` 管理日志文件
- 设置日志级别
- 定期清理旧日志

#### 2. 性能监控

监控指标：
- CPU 使用率
- 内存使用
- 网络流量
- 连接数
- 活跃通话数

可用工具：
- `top` / `htop`
- `netstat` / `ss`
- `iotop`
- Prometheus + Grafana

#### 3. 备份

定期备份：
- 配置文件
- 日志文件
- 统计数据

### 安全建议

1. **使用防火墙** - 只开放必要的端口
2. **启用 SSL/TLS** - 加密网络通信（需要额外实现）
3. **认证机制** - 实现用户认证和授权
4. **限流** - 防止DDoS攻击
5. **定期更新** - 保持系统和依赖包更新
6. **监控日志** - 检测异常活动

### 故障排查

#### 服务器无法启动

1. 检查端口是否被占用：
   ```bash
   netstat -tuln | grep 8888
   ```

2. 检查权限：
   ```bash
   ls -l bin/WeCompanyServer
   ```

3. 查看详细错误信息

#### 客户端无法连接

1. 检查防火墙设置
2. 验证服务器IP和端口
3. 使用 telnet 测试连接：
   ```bash
   telnet <server-ip> 8888
   ```

#### 性能问题

1. 检查系统资源使用情况
2. 优化系统参数
3. 增加服务器资源
4. 实现负载均衡（多服务器部署）

### 扩展部署

#### 负载均衡

使用 Nginx 或 HAProxy 实现负载均衡：

Nginx TCP 负载均衡配置示例：

```nginx
stream {
    upstream wecompany_backend {
        server 192.168.1.10:8888;
        server 192.168.1.11:8888;
        server 192.168.1.12:8888;
    }

    server {
        listen 8888;
        proxy_pass wecompany_backend;
    }
}
```

## 常见问题 (FAQ)

### Q: 支持多少并发连接？

A: 取决于服务器硬件和配置。默认配置可支持数百个并发连接。通过优化可支持数千个连接。

### Q: 如何实现SSL/TLS加密？

A: 需要使用 QSslSocket 替换 QTcpSocket，并配置SSL证书。

### Q: 是否支持集群部署？

A: 当前版本为单机版本。集群部署需要实现分布式会话管理和负载均衡。

### Q: 如何监控服务器状态？

A: 可以实现管理接口暴露服务器统计信息，或集成到监控系统（如Prometheus）。

## 支持和联系

如有问题，请在 GitHub 提交 Issue。
