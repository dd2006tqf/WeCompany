# WeCompany Client UI Components

## 客户端界面组件 (Client UI Components)

本文档描述 WeCompany 客户端的登录和通话界面组件。

This document describes the login and call UI components for the WeCompany client.

## 新增界面组件 (New UI Components)

### 1. ServerLoginDlg - 服务器登录对话框

完整的登录界面，支持用户注册和登录，连接到 WeCompany 服务器。

**功能特性 (Features):**
- 用户名和密码输入
- 登录验证
- 新用户注册
- 记住密码选项
- 服务器连接状态显示
- 美观的现代化UI设计

**使用示例 (Usage Example):**

```cpp
#include "serverlogindlg.h"

ServerLoginDlg *loginDlg = new ServerLoginDlg(this);

connect(loginDlg, &ServerLoginDlg::loginSuccessful, 
        [](const QString &token, const QString &userId, const QString &username) {
    qInfo() << "Login successful!";
    qInfo() << "Token:" << token;
    qInfo() << "User ID:" << userId;
    qInfo() << "Username:" << username;
    
    // Store credentials and proceed to main window
});

connect(loginDlg, &ServerLoginDlg::loginFailed,
        [](const QString &error) {
    qWarning() << "Login failed:" << error;
});

loginDlg->exec();
```

**界面截图 (Screenshot):**
- 蓝色现代化设计
- 清晰的输入框和按钮
- 实时状态反馈

### 2. OutgoingCallDialog - 拨打电话对话框

发起音频或视频通话时显示的界面。

**功能特性 (Features):**
- 显示被叫方头像和姓名
- 呼叫状态显示（拨打中、通话中）
- 通话时长计时器
- 挂断按钮
- 支持音频和视频通话

**使用示例 (Usage Example):**

```cpp
#include "calldialog.h"

OutgoingCallDialog *callDlg = new OutgoingCallDialog(
    "user123",      // callee user ID
    "张三",         // callee name
    true,           // is video call
    this
);

connect(callDlg, &OutgoingCallDialog::callCancelled, []() {
    qInfo() << "User cancelled the call";
});

connect(callDlg, &OutgoingCallDialog::callEnded, []() {
    qInfo() << "Call ended";
});

callDlg->show();

// When call is accepted by remote party:
// callDlg->startCall();
```

### 3. IncomingCallDialog - 来电对话框

接收到来电时显示的界面。

**功能特性 (Features):**
- 显示主叫方头像和姓名
- 呼叫类型显示（音频/视频）
- 接听按钮（绿色）
- 拒接按钮（红色）
- 美观的铃声提示界面

**使用示例 (Usage Example):**

```cpp
#include "calldialog.h"

IncomingCallDialog *incomingDlg = new IncomingCallDialog(
    "user456",      // caller user ID
    "李四",         // caller name
    false,          // is video call (false = audio only)
    this
);

connect(incomingDlg, &IncomingCallDialog::callAccepted, []() {
    qInfo() << "Call accepted";
    // Transition to ActiveCallDialog
});

connect(incomingDlg, &IncomingCallDialog::callRejected, []() {
    qInfo() << "Call rejected";
    // Send rejection to server
});

incomingDlg->exec();
```

### 4. ActiveCallDialog - 通话中对话框

通话进行中显示的界面。

**功能特性 (Features):**
- 视频预览窗口（视频通话）
- 对方头像显示（音频通话）
- 实时通话时长显示
- 静音按钮
- 视频开关按钮（视频通话）
- 挂断按钮
- 深色主题（视频通话）

**使用示例 (Usage Example):**

```cpp
#include "calldialog.h"

ActiveCallDialog *activeDlg = new ActiveCallDialog(
    "user789",      // peer user ID
    "王五",         // peer name
    true,           // is video call
    this
);

connect(activeDlg, &ActiveCallDialog::callEnded, []() {
    qInfo() << "Call ended by user";
    // Notify server
});

connect(activeDlg, &ActiveCallDialog::muteToggled, [](bool muted) {
    qInfo() << "Mute toggled:" << muted;
    // Control audio stream
});

connect(activeDlg, &ActiveCallDialog::videoToggled, [](bool enabled) {
    qInfo() << "Video toggled:" << enabled;
    // Control video stream
});

activeDlg->exec();
```

## 界面设计特点 (UI Design Features)

### 1. 现代化设计

- 圆角边框和按钮
- 渐变色和阴影效果
- 扁平化图标
- 响应式交互

### 2. 颜色方案

- **主色调**: #1485EE (蓝色)
- **成功/接听**: #00CC44 (绿色)
- **危险/拒接**: #FF4444 (红色)
- **文字**: #333333 (深灰)
- **提示**: #999999 (浅灰)

### 3. 用户体验

- 清晰的视觉层级
- 直观的按钮布局
- 实时状态反馈
- 流畅的动画过渡

## 集成到主窗口 (Integration with Main Window)

### 修改 main.cpp

```cpp
#include "serverlogindlg.h"
#include "wecomwnd.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Show server login dialog first
    ServerLoginDlg loginDlg;
    
    if (loginDlg.exec() == QDialog::Accepted) {
        // Login successful, show main window
        WeComWnd mainWnd;
        mainWnd.setUserInfo(
            loginDlg.getToken(),
            loginDlg.getUserId(),
            loginDlg.getUsername()
        );
        mainWnd.show();
        
        return app.exec();
    }
    
    return 0;
}
```

### 在主窗口中使用通话对话框

```cpp
// In your main window or chat window class

void MainWindow::onMakeCallClicked()
{
    QString calleeId = getCurrentChatUserId();
    QString calleeName = getCurrentChatUserName();
    bool isVideo = askUserForCallType(); // Ask if audio or video
    
    OutgoingCallDialog *callDlg = new OutgoingCallDialog(
        calleeId, calleeName, isVideo, this
    );
    
    connect(callDlg, &OutgoingCallDialog::callCancelled, this, [=]() {
        // Send cancel message to server
        sendCancelCallToServer(calleeId);
    });
    
    connect(callDlg, &OutgoingCallDialog::callEnded, this, [=]() {
        // Send end call message to server
        sendEndCallToServer(calleeId);
    });
    
    callDlg->show();
}

void MainWindow::onIncomingCall(const QString &callerId, const QString &callerName, bool isVideo)
{
    IncomingCallDialog *incomingDlg = new IncomingCallDialog(
        callerId, callerName, isVideo, this
    );
    
    connect(incomingDlg, &IncomingCallDialog::callAccepted, this, [=]() {
        // Send accept to server
        sendAcceptCallToServer(callerId);
        
        // Show active call dialog
        showActiveCall(callerId, callerName, isVideo);
    });
    
    connect(incomingDlg, &IncomingCallDialog::callRejected, this, [=]() {
        // Send reject to server
        sendRejectCallToServer(callerId);
    });
    
    incomingDlg->exec();
}

void MainWindow::showActiveCall(const QString &peerId, const QString &peerName, bool isVideo)
{
    ActiveCallDialog *activeDlg = new ActiveCallDialog(
        peerId, peerName, isVideo, this
    );
    
    connect(activeDlg, &ActiveCallDialog::callEnded, this, [=]() {
        sendEndCallToServer(peerId);
    });
    
    activeDlg->exec();
}
```

## 消息协议扩展 (Message Protocol Extension)

为支持新的登录和通话UI，需要在消息协议中添加以下消息类型：

```cpp
enum MessageType {
    MSG_TEXT = 0,
    MSG_FILE = 1,
    MSG_CALL_REQUEST = 2,
    MSG_CALL_ACCEPT = 3,
    MSG_CALL_REJECT = 4,
    MSG_CALL_END = 5,
    MSG_MEDIA_DATA = 6,
    MSG_HEARTBEAT = 7,
    MSG_LOGIN_REQUEST = 8,     // 新增：登录请求
    MSG_REGISTER_REQUEST = 9,  // 新增：注册请求
    MSG_AUTH_RESPONSE = 10     // 新增：认证响应
};
```

## 编译说明 (Build Instructions)

### 1. 更新项目文件

已更新 `WeComCopy.pro` 文件，包含新的源文件和头文件：
- `source/serverlogindlg.cpp`
- `source/calldialog.cpp`
- `include/serverlogindlg.h`
- `include/calldialog.h`

同时添加了 `network` 模块支持。

### 2. 编译

```bash
qmake WeComCopy.pro
make
```

### 3. 运行

```bash
./bin/WeComCopy
```

## 截图展示 (Screenshots)

### 登录界面
- 蓝色主题，现代化设计
- 清晰的输入框和按钮
- 状态提示区域

### 拨打电话界面
- 圆形头像显示
- 呼叫状态
- 红色挂断按钮

### 来电界面
- 圆形头像居中
- 绿色接听按钮（左）
- 红色拒接按钮（右）

### 通话中界面
#### 音频通话
- 深色背景
- 头像显示
- 通话时长计时
- 控制按钮（静音、挂断）

#### 视频通话
- 黑色视频预览区域
- 通话时长显示
- 控制按钮（静音、视频、挂断）

## 注意事项 (Notes)

1. **图标字体**: 需要确保 `iconhelper.h` 中定义了所需的图标字符
2. **资源文件**: 确保头像等图片资源存在于 `res/` 目录
3. **服务器连接**: 默认连接到 `127.0.0.1:8888`，可在代码中修改
4. **样式自定义**: 所有样式都使用内联 QSS，便于自定义

## 后续扩展 (Future Enhancements)

- 添加视频流显示
- 实现音频播放和录制
- 添加通话历史记录
- 支持群组通话
- 添加更多通话控制选项（免提、录音等）

## 技术支持 (Support)

如有问题或建议，请在 GitHub 提交 Issue。
