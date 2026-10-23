# NightAuth C SDK — 在 GitHub 上关注并点星 ⭐

官方 **NightAuth** C11 Header-Only SDK 与参考客户端套件（现代原生 **Win32 Pure GDI+** 桌面界面与交互式终端 CLI）。在几分钟内保护您的原生 C 软件免受逆向工程、盗版和篡改。

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 视频教程与系统部署指南
> 官方视频操作教程以及关于如何在 NightAuth 管理后台配置应用、订阅计划、云端变量、Webhook 与密钥分发的分步指南**即将发布**，敬请关注官方网站与频道。

---

## 📸 界面截图

### 桌面应用程序 (原生 Win32 GDI 现代深色主题)

| 登录 / 激活认证界面 | 用户面板与授权监控 |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### 控制台客户端 (交互式 Terminal CLI)

| 终端登录交互 | 控制台交互式仪表盘 |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ 推荐安全实践

* **原生二进制虚拟化保护：** 原生 C 二进制文件易受到 IDA Pro、Ghidra 和 Binary Ninja 等工具的静态逆向分析。在生产发布版本中，务必使用商业代码虚拟化保护壳（如 **VMProtect**、**Themida** 或 **CodeVirtualizer**）对编译后的可执行文件进行加壳保护。
* **编译期字符串加密：** 严禁将敏感字符串或 API 密钥以明文形式遗留在 `.rdata` 节区中。使用编译期 XOR 字符串混淆宏，仅在调用瞬间在栈上解密。
* **内存中安全文件执行：** 通过 `nightauth_download_file(api, ticket, ...)` 串流下载受保护的载荷或 DLL 时，**切勿将字节流写入物理硬盘**。必须直接在内存 RAM 中加载并执行（如反射型 PE 加载或手动映射注入），执行完成后立即调用 `SecureZeroMemory()` 彻底擦除缓冲区。
* **硬件指纹与运行期完整性检测：** 在生产版本中保持反调试、内存完整性校验和时序检测常开，有效阻断运行时 Hook 及调试器挂载。

---

## 📦 系统要求与编译

需要 **C11** 或更高版本，以及 **Visual Studio 2026**（MSVC 工具集 `v145`）。随附的项目均配置为 `v145`：控制台项目无条件固定 `<PlatformToolset>v145</PlatformToolset>`，而桌面项目仅在未指定工具集时默认使用 `v145`。使用 **Visual Studio 2022**（`v143`）编译只有在手动更改 `console/NightAuth-Console.vcxproj` 中的工具集（或为桌面客户端传入 `/p:PlatformToolset=v143`）时才有效。

Windows 依赖项：
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib`（Windows 原生加密与套接字）
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib`（桌面 GUI 渲染）
* 内置的 `libcurl` 和 `LibreSSL`（兼容 OpenSSL，头文件位于 `include/openssl/`）（位于 `deps/curl_win64`）

> [!NOTE]
> SDK 头文件使用了 `<stdatomic.h>`（C11 原子操作）。将 `sdk/nightauth.h` 集成到你自己的 MSVC 项目时，请添加编译器标志 `/experimental:c11atomics`（两个随附的 `.vcxproj` 文件均已设置）。缺少该标志时，MSVC 将无法编译加密会话层所使用的原子操作。

### 使用 MSBuild 编译解决方案

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

### 运行时部署

两个客户端项目都针对 libcurl 的**导入库**（`libcurl.dll.a`）进行链接，因此 `libcurl-x64.dll`（位于 `deps/curl_win64/curl-8.23.0_5-win64-mingw/bin`）**必须与编译后的 `.exe` 文件一起分发**，否则客户端将无法启动。桌面项目还会链接 `shell32.lib`。

---

## ⚙️ 应用程序配置与客户端实例化

在 NightAuth 管理后台的 **Manage Apps** 选项卡中获取应用凭证：

```c
#define NIGHTAUTH_IMPLEMENTATION
#include "sdk/nightauth.h"

// Application Configuration (from NightAuth Dashboard)
#define APP_NAME    ""
#define APP_API_KEY ""
#define APP_VERSION "1.0"
#define APP_TOKEN   NULL

// Initialize application handle
NightAuthApi* api = nightauth_create(
    APP_NAME,
    APP_API_KEY,
    APP_VERSION,
    APP_TOKEN,
    NULL
);
```

### 初始化加密握手 (`init`)

在调用其他任何 SDK 方法之前，必须先调用 `init()`。该方法将完成 Diffie-Hellman (X25519) 密钥交换并建立端到端加密会话：

```c
nightauth_init(api);

NightAuthResponse* resp = nightauth_get_response(api);
if (!resp->success) {
    fprintf(stderr, "Initialization failed: %s\n", resp->message);
    nightauth_destroy(api);
    return 1;
}
```

---

## 🚀 SDK 核心功能使用指南

### 1. 验证会话与黑名单检查

```c
// Validate active session status with the server
if (nightauth_check(api)) {
    printf("Session active.\n");
}

// Check if client HWID or IP address is blacklisted
bool blacklisted = false;
if (nightauth_check_blacklist(api, NULL, &blacklisted) && blacklisted) {
    fprintf(stderr, "Access Denied: Machine or IP is blacklisted.\n");
    return 1;
}
```

### 2. 账号密码登录 (支持 2FA 双重认证)

```c
if (nightauth_login(api, username, password, totp_code)) {
    NightAuthUserData* user = nightauth_get_user(api);
    printf("Welcome back, %s!\n", user->username);
    printf("Hardware ID: %s\n", user->hwid);
    printf("Client IP:   %s\n", user->ip);

    for (int i = 0; i < user->subscription_count; i++) {
        printf("Plan: %s | Expires: %s | Key: %s\n",
               user->subscriptions[i].subscription,
               user->subscriptions[i].expiry,
               user->subscriptions[i].key);
    }
} else {
    fprintf(stderr, "Login failed: %s\n", nightauth_get_response(api)->message);
}
```

### 3. 使用授权密钥注册新用户

```c
if (nightauth_register(api, username, password, license_key, email)) {
    printf("Registered successfully: %s\n", nightauth_get_user(api)->username);
} else {
    fprintf(stderr, "Registration failed: %s\n", nightauth_get_response(api)->message);
}
```

### 4. 仅凭卡密激活登录 (Key Only)

```c
if (nightauth_license(api, license_key)) {
    printf("License verified successfully. Access granted.\n");
} else {
    fprintf(stderr, "Access denied: %s\n", nightauth_get_response(api)->message);
}
```

### 5. 升级账号 / 激活充值新卡密 (Upgrade)

```c
if (nightauth_upgrade(api, username, new_license_key)) {
    printf("Subscription upgraded successfully.\n");
} else {
    fprintf(stderr, "Upgrade failed: %s\n", nightauth_get_response(api)->message);
}
```

### 6. 重置机器码绑定 (HWID Reset)

```c
if (nightauth_reset_hwid(api, username, password, totp_code)) {
    printf("HWID reset successfully. You can now log in on this machine.\n");
} else {
    fprintf(stderr, "HWID reset failed: %s\n", nightauth_get_response(api)->message);
}
```

### 7. 找回密码与重置 (Forgot Password & Reset)

```c
// Request password reset email
if (nightauth_forgot_password(api, email)) {
    printf("Password recovery email sent. Check your inbox.\n");
}

// Complete password reset with token
if (nightauth_reset_password(api, token, new_password)) {
    printf("Password reset successfully.\n");
}
```

### 8. 用户档案与授权信息 (`result.ToUserData()`)

```c
NightAuthUserData* user = nightauth_get_user(api);

printf("Username:    %s\n", user->username);
printf("Client IP:   %s\n", user->ip);
printf("Hardware ID: %s\n", user->hwid);
printf("Registered:  %s\n", user->created_at);
printf("Last Login:  %s\n", user->last_login_at);

for (int i = 0; i < user->subscription_count; i++) {
    printf("Plan: %s | Expires: %s | Key: %s\n",
           user->subscriptions[i].subscription,
           user->subscriptions[i].expiry,
           user->subscriptions[i].key);
}
```

### 9. 云端全局变量 (Cloud Variables)

```c
char motd[256] = {0};
if (nightauth_get_global_var(api, "server_motd", motd, sizeof(motd))) {
    printf("Cloud Variable: %s\n", motd);
}
```

### 10. 用户专属隔离变量 (User Variables)

```c
// Set a user variable
bool set_ok = nightauth_set_user_var(api, "discord_tag", "user#0001");

// Retrieve a user variable
char tag[128] = {0};
if (nightauth_get_user_var(api, "discord_tag", tag, sizeof(tag))) {
    printf("Stored Discord: %s\n", tag);
}

// Retrieve multiple user variables
NightAuthUserVar vars[32];
int total = nightauth_get_user_variables(api, vars, 32);
for (int i = 0; i < total; i++) {
    printf("%s = %s\n", vars[i].key, vars[i].value);
}
```

### 11. 远程安全审计日志

```c
nightauth_log(api, "Suspicious memory modification attempt detected.");
```

### 12. 封禁用户并拉黑当前设备

```c
nightauth_ban(api, "Security tampering detected by Anti-Cheat.");
```

### 13. 服务端安全 Webhook 转发

```c
int status = 0;
const char* payload = "{\"content\":\"Client notification\"}";

if (nightauth_webhook(api, "7kR0UedlVI", payload, NULL, &status)) {
    printf("Webhook Response Status: HTTP %d\n", status);
}
```

### 14. 内存中安全下载文件

文件下载需要使用 `nightauth_access_ticket()` 获取的**带作用域的访问票据**（服务器在提供文件之前会验证该票据）：

```c
// 为目标文件请求带作用域的访问票据（"file" 作用域）
char ticket[128] = {0};
int64_t expires_in = 0;
if (!nightauth_access_ticket(api, "file", "385624", ticket, sizeof(ticket), &expires_in)) {
    fprintf(stderr, "访问票据被拒绝：%s\n", nightauth_get_response(api)->message);
    return 1;
}

char file_name[128] = {0};
uint8_t file_buf[65536];
size_t downloaded_len = 0;

if (nightauth_download_file(api, ticket, file_name, file_buf, sizeof(file_buf), &downloaded_len)) {
    printf("已安全下载 %s（%zu 字节）。\n", file_name, downloaded_len);
    // 直接在内存中处理或执行载荷（Reflective PE / Shellcode）
    // 执行后安全地擦除内存缓冲区
    SecureZeroMemory(file_buf, downloaded_len);
}
```

### 15. 实时聊天频道

```c
// Send a message to a channel
nightauth_send_chat_message(api, "Hello community!", "general");

// Fetch latest messages
NightAuthChatMessage msgs[50];
int msg_count = nightauth_get_chat_messages(api, "general", msgs, 50);
for (int i = 0; i < msg_count; i++) {
    printf("[%s]: %s\n", msgs[i].author, msgs[i].message);
}
```

### 16. 实时服务端推送事件处理 (SSE)

```c
static bool handle_event(const char* event_json, void* user_data) {
    printf("[EVENT] %s\n", event_json);
    return true; // Return true to keep stream alive
}

nightauth_events_run(api, "ticket_value", NULL, handle_event, NULL);
```

### 17. 双重认证管理 (2FA)

```c
// Setup TOTP 2FA
NightAuthTwoFactorSetupData setup;
if (nightauth_setup_2fa(api, &setup)) {
    printf("2FA Secret: %s\n", setup.secret);
    printf("QR URI:     %s\n", setup.uri);
}

// Confirm with 6-digit Authenticator code
NightAuthTwoFactorConfirmData conf;
nightauth_confirm_2fa(api, "123456", &conf);

// Disable 2FA
nightauth_disable_2fa(api, password, "123456");
```

### 18. 会话保活与安全登出

```c
// Heartbeat ping to keep session alive
nightauth_heartbeat(api);

// Revoke and terminate session
nightauth_logout(api);

// Release memory allocations
nightauth_destroy(api);
```

### 19. 客户端高级配置

```c
// 每次请求的超时时间（毫秒），影响之后所有的网络调用
nightauth_set_request_timeout(api, 15000);

// 允许对 loopback/自托管开发端点使用未加密的 HTTP
NightAuthApi* api_dev = nightauth_create_ex(
    APP_NAME, APP_API_KEY, APP_VERSION, APP_TOKEN,
    "http://127.0.0.1:8787/api/v1",   // 自定义 API URL（NULL = 默认值）
    true                               // allow_insecure_loopback
);

// 查看或轮换活跃句柄的身份信息（名称、api key、版本）
char cur_name[128], cur_key[128], cur_ver[32];
nightauth_get_identity(api, cur_name, sizeof(cur_name), cur_key, sizeof(cur_key), cur_ver, sizeof(cur_ver));
nightauth_set_identity(api, "MyApp", "new_api_key", "1.1");
```

### 20. 服务器公钥与默认常量

SDK 固定了服务器的签名公钥与 sealed-box 公钥。默认值以编译期常量形式提供（`sdk/nightauth.h` 中的 `NIGHTAUTH_DEFAULT_API_URL`、`NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY`、`NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY`），实现还导出了可修改的副本（`NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY`、`NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY`），自托管运营者可以按句柄覆盖：

```c
// 将句柄指向使用自有密钥的自托管 NightAuth 实例
nightauth_set_server_keys(api,
    "a1b2c3...",   // 签名公钥（64 个十六进制字符）
    "d4e5f6...");  // sealed-box 公钥（64 个十六进制字符）
```

### 21. 使用 2FA 的许可证登录（`nightauth_license_with_totp`）

对于许可证流程强制要求 TOTP 的账户，请将身份验证器代码与许可证密钥一起传入：

```c
if (nightauth_license_with_totp(api, license_key, "123456")) {
    printf("许可证已通过 2FA 验证。已授予访问权限。\n");
}
```

### 22. 修改账户用户名

需要提供当前密码。**注意：**出于安全原因，服务器在成功改名后会关闭会话——请使用新用户名重新登录。

```c
if (nightauth_change_username(api, "new_username", password)) {
    printf("用户名已更改为 %s。请重新登录。\n",
           nightauth_get_user(api)->username);
}
```

### 23. 验证文件哈希

针对服务器端授权版本注册表，校验你所分发二进制的 SHA-256 校验和：

```c
if (nightauth_verify_file_hash(api, "a1b2c3d4...")) {
    printf("完整性验证通过：校验和与授权版本一致。\n");
} else {
    fprintf(stderr, "验证失败：%s\n", nightauth_get_response(api)->message);
}
```

### 24. 应用遥测与在线用户

```c
// 应用聚合统计信息
NightAuthAppStats stats;
if (nightauth_get_stats(api, &stats)) {
    printf("用户：%lld（活跃 %lld，封禁 %lld）\n",
           (long long)stats.total_users, (long long)stats.active_users, (long long)stats.banned_users);
    printf("许可证：已用 %lld / 未用 %lld | 版本：%s\n",
           (long long)stats.used_licenses, (long long)stats.unused_licenses, stats.version);
}

// 当前在线用户（返回写入的用户名数量）
char online[16][128];
int64_t total_online = 0;
int count = nightauth_get_online_users(api, &total_online, online, 16);
for (int i = 0; i < count; i++) {
    printf("在线：%s\n", online[i]);
}

// 应用静态元数据（名称、版本、总数）
NightAuthAppData* app = nightauth_get_app(api);
printf("App：%s %s | 用户：%lld | 在线：%lld\n",
       app->application, app->version, (long long)app->total_users, (long long)app->online_users);
```

### 25. 硬件 ID（HWID）管理

```c
// 读取 SDK 计算出的机器指纹
char hwid[128] = {0};
nightauth_get_hardware_id(hwid, sizeof(hwid));

// 将当前会话句柄绑定到指定的 HWID
nightauth_set_hardware_id(api, hwid);

// 全局覆盖未来所有句柄上报的指纹
// （适用于测试或自定义指纹方案）
nightauth_set_hardware_id_override("custom-machine-id");
```

### 26. 分页聊天记录（`nightauth_get_chat_messages_ex`）

获取早于指定 ID 的消息（传 `0` 表示最新一页），并可选择在服务器端将它们标记为已读：

```c
NightAuthChatMessage hist[25];
int count = nightauth_get_chat_messages_ex(api, "general", 0, true, hist, 25);
for (int i = 0; i < count; i++) {
    printf("[%s] %s：%s (#%s)\n", hist[i].timestamp, hist[i].author, hist[i].message, hist[i].id);
}
```

### 27. 高级服务器端 Webhook（`nightauth_webhook_ex`）

带有查询参数、可选文件附件和 HTTP 状态输出的扩展 Webhook：

```c
int status = 0;
const char* json = "{\"content\":\"Client notification\"}";
const void* report_data = NULL;   // 可选附件字节
size_t report_len = 0;

if (nightauth_webhook_ex(api, "7kR0UedlVI", "Client notification",
                         "urgent=true", json,
                         report_data, report_len, "report.bin", &status)) {
    printf("Webhook 已触发：HTTP %d\n", status);
}
```

### 28. 高级实时事件流（`events_run_ex`、`events_cancel`）

`nightauth_events_run_ex` 会连同服务器事件 ID 一起交付每个 SSE 事件（可通过 `last_event_id` 恢复）。`nightauth_events_cancel` 可从另一个线程中断正在运行的流（例如在用户输入时）：

```c
static bool handle_event_ex(const char* event_json, const char* event_id, void* user) {
    printf("[EVENT %s] %s\n", event_id ? event_id : "-", event_json);
    return true; // 返回 true 以保持流继续运行
}

// 另一个线程可以调用 nightauth_events_cancel(api) 来停止循环
if (nightauth_events_run_ex(api, ticket, NULL, handle_event_ex, NULL)) {
    printf("事件流已正常关闭。\n");
}
```

### 29. 已签名且经验证的自更新

对照固定的 Ed25519 公钥验证已签名的更新信封、下载更新，或将已验证的 URL 交给服务器辅助的应用流程：

```c
// 1. 验证从服务器收到的已签名更新清单
NightAuthVerifiedUpdate update;
char err[256] = {0};
if (nightauth_signed_update_verify(envelope_json, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                   "MyApp", /*min_sequence=*/1, &update, err, sizeof(err))) {
    printf("更新 v%s -> 序号 %llu（%llu 字节）\n", update.version,
           (unsigned long long)update.sequence, (unsigned long long)update.size);

    // 2. 将已验证的构件下载到磁盘
    if (nightauth_signed_update_download(&update, "update.pkg", err, sizeof(err))) {
        printf("更新已下载且摘要已验证。\n");
    }
}

// 3. 或直接从清单 URL 下载官方安装程序
nightauth_signed_update_installer_download(manifest_url, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                           "MyApp", "installer.exe", err, sizeof(err));

// 4. 服务器辅助应用：将待处理的下载 URL 交给 API 句柄
if (nightauth_apply_update(api, download_url)) {
    printf("所需版本：%s\n", nightauth_get_required_version(api));
}
const char* pending = nightauth_get_pending_download_url(api); // 没有待处理项时为 NULL
```

---

## 🖥️ 运行控制台与桌面客户端

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 授权协议

本项目基于 **Elastic License 2.0** 协议授权。

* 不得以托管或在线托管服务的形式向第三方提供本软件服务。
* 不得移动、修改、禁用或规避许可密钥验证相关功能。
* 不得修改、移除或遮挡任何许可与版权声明。

详情请参阅 [LICENSE](../LICENSE) 文件。
