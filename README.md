# NightAuth C SDK — Star us on GitHub ⭐

Official **NightAuth** C11 Header-Only SDK & Reference Client Suite (Modern Desktop UI in **Win32 Pure GDI** and Interactive Terminal CLI). Protect your native C software against reverse engineering, piracy, and tampering with cloud licensing, HWID locking, realtime encrypted chat, and secure in-memory streaming in minutes.

Official Portal: [https://nightauth.org](https://nightauth.org)  
Documentation: [https://nightauth.org/docs](https://nightauth.org/docs)  
Technical Architecture Benchmark: [NightAuth vs KeyAuth vs Auth.gg](https://nightauth.org/blog/nightauth-vs-keyauth-authgg-comparison)

---

**Languages / Idiomas / Langues / Sprachen:**  
[English](README.md) • [Español](lang/es.md) • [Français](lang/fr.md) • [Deutsch](lang/de.md) • [Italiano](lang/it.md) • [Português](lang/pt.md) • [Русский](lang/ru.md) • [Türkçe](lang/tr.md) • [Polski](lang/pl.md) • [中文](lang/zh.md)

---

> [!NOTE]
> ### 📺 Video Tutorials & System Setup Guides
> Official video tutorials and detailed step-by-step guides explaining how to configure applications, plans, subscriptions, cloud variables, webhooks, and licenses in the NightAuth Dashboard **will be published soon** on our official portal and YouTube channel.

---

## 📸 Screenshots

### Desktop Application (Native Win32 GDI Modern Dark Theme)

| Start / Authentication Screen | Dashboard & Subscription Management |
| :---: | :---: |
| ![NightAuth Desktop Login](image/desktop.png) | ![NightAuth Desktop Dashboard](image/desktop-dashboard.png) |

### Console Application (Interactive Terminal CLI)

| Terminal Login Screen | Interactive Console Dashboard |
| :---: | :---: |
| ![NightAuth Console Login](image/console.png) | ![NightAuth Console Dashboard](image/console-dashboard.png) |

---

## 🛡️ Recommended Security Practices

* **Native Binary Virtualization:** Native C binaries are subject to static analysis via IDA Pro, Ghidra, and Binary Ninja. For production releases, protect compiled executables using commercial code virtualizers and protectors such as **VMProtect**, **Themida**, or **CodeVirtualizer**.
* **Compile-Time String Encryption:** Do not leave sensitive strings or API tokens in plain text in the `.rdata` section. Use XOR string obfuscation macros to decrypt strings only at the instant they are required.
* **In-Memory Secure File Execution:** When streaming protected payloads or DLLs via `nightauth_download_file(api, ticket, ...)`, **never write bytes to physical storage or disk**. Load and execute payloads directly from memory (e.g., reflective PE loading or manual map injection) and immediately wipe the buffer with `SecureZeroMemory()`.
* **Hardware & Integrity Verification:** Keep anti-debugging, memory integrity, and timing checks active in production releases to mitigate runtime hooking and debugger attachment.

---

## 📦 Requirements & Building

Requires **C11** or later and **Visual Studio 2026** (MSVC toolset `v145`). The bundled projects are configured for `v145`: the console project pins `<PlatformToolset>v145</PlatformToolset>` unconditionally, while the desktop project defaults to `v145` only when no toolset is specified. Building with **Visual Studio 2022** (`v143`) works only if you manually change the toolset in `console/NightAuth-Console.vcxproj` (or pass `/p:PlatformToolset=v143` for the desktop client).

Windows dependencies:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Native Windows Crypto & Sockets)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Desktop GUI Rendering)
* Bundled `libcurl` and `LibreSSL` (OpenSSL-compatible, headers under `include/openssl/`) (in `deps/curl_win64`)

> [!NOTE]
> The SDK header uses `<stdatomic.h>` (C11 atomics). When integrating `sdk/nightauth.h` into your own MSVC project, add the `/experimental:c11atomics` compiler flag (both bundled `.vcxproj` files already set it via `<AdditionalOptions>`). Without it, MSVC fails to compile the atomic operations used by the encrypted session layer.

### Build Solution with MSBuild

```bash
# Build both Console & Desktop clients in 64-bit Release
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

### Runtime Deployment

Both client projects link against the bundled libcurl **import library** (`libcurl.dll.a`), so the DLL `libcurl-x64.dll` (from `deps/curl_win64/curl-8.23.0_5-win64-mingw/bin`) **must be distributed next to the built `.exe` files**, otherwise the clients fail to start. The desktop GUI project additionally links `shell32.lib`.

---

## ⚙️ Configuration & Initialization

Include the header in one C translation unit with `#define NIGHTAUTH_IMPLEMENTATION`:

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

### Initialize Cryptographic Handshake (`init`)

You must call `nightauth_init()` prior to executing any other SDK functions. This establishes an ephemeral Diffie-Hellman (X25519) key exchange and an encrypted session:

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

## 🚀 SDK Usage Guide

### 1. Check Session Validation & Blacklist Status

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

### 2. Login with Username & Password (and 2FA)

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

### 3. Register with License Key

```c
if (nightauth_register(api, username, password, license_key, email)) {
    printf("Registered successfully: %s\n", nightauth_get_user(api)->username);
} else {
    fprintf(stderr, "Registration failed: %s\n", nightauth_get_response(api)->message);
}
```

### 4. Authenticate with License Key Only

```c
if (nightauth_license(api, license_key)) {
    printf("License verified successfully. Access granted.\n");
} else {
    fprintf(stderr, "Access denied: %s\n", nightauth_get_response(api)->message);
}
```

### 5. Upgrade Account / Claim License Key

```c
if (nightauth_upgrade(api, username, new_license_key)) {
    printf("Subscription upgraded successfully.\n");
} else {
    fprintf(stderr, "Upgrade failed: %s\n", nightauth_get_response(api)->message);
}
```

### 6. Reset Hardware ID (HWID Lock)

```c
if (nightauth_reset_hwid(api, username, password, totp_code)) {
    printf("HWID reset successfully. You can now log in on this machine.\n");
} else {
    fprintf(stderr, "HWID reset failed: %s\n", nightauth_get_response(api)->message);
}
```

### 7. Forgot Password / Account Recovery

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

### 8. User Profile Data (`nightauth_get_user`)

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

### 9. Cloud Variables (Global)

```c
char motd[256] = {0};
if (nightauth_get_global_var(api, "server_motd", motd, sizeof(motd))) {
    printf("Cloud Variable: %s\n", motd);
}
```

### 10. User Variables

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

### 11. Application Audit Logs

```c
nightauth_log(api, "Suspicious memory modification attempt detected.");
```

### 12. Ban User & Blacklist Machine

```c
nightauth_ban(api, "Security tampering detected by Anti-Cheat.");
```

### 13. Server-Sided Webhooks

```c
int status = 0;
const char* payload = "{\"content\":\"Client notification\"}";

if (nightauth_webhook(api, "7kR0UedlVI", payload, NULL, &status)) {
    printf("Webhook Response Status: HTTP %d\n", status);
}
```

### 14. Secure In-Memory File Download

File downloads require a **scoped access ticket** obtained with `nightauth_access_ticket()` (the server validates the ticket before serving the file):

```c
// Request a scoped access ticket for the target file ("file" scope)
char ticket[128] = {0};
int64_t expires_in = 0;
if (!nightauth_access_ticket(api, "file", "385624", ticket, sizeof(ticket), &expires_in)) {
    fprintf(stderr, "Access ticket denied: %s\n", nightauth_get_response(api)->message);
    return 1;
}

char file_name[128] = {0};
uint8_t file_buf[65536];
size_t downloaded_len = 0;

if (nightauth_download_file(api, ticket, file_name, file_buf, sizeof(file_buf), &downloaded_len)) {
    printf("Downloaded %s (%zu bytes) securely.\n", file_name, downloaded_len);
    // Process or execute payload directly in memory (Reflective PE / Shellcode)
    // Securely wipe memory buffer after execution
    SecureZeroMemory(file_buf, downloaded_len);
}
```

### 15. Realtime Chat Channels

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

### 16. Realtime Event Handlers (Server-Sent Events)

```c
static bool handle_event(const char* event_json, void* user_data) {
    printf("[EVENT] %s\n", event_json);
    return true; // Return true to keep stream alive
}

nightauth_events_run(api, "ticket_value", NULL, handle_event, NULL);
```

### 17. Two-Factor Authentication (2FA)

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

### 18. Session Lifecycle & Logout

```c
// Heartbeat ping to keep session alive
nightauth_heartbeat(api);

// Revoke and terminate session
nightauth_logout(api);

// Release memory allocations
nightauth_destroy(api);
```

### 19. Advanced Client Configuration

```c
// Per-request timeout in milliseconds (affects all subsequent network calls)
nightauth_set_request_timeout(api, 15000);

// Allow plain HTTP for loopback/self-hosted development endpoints
NightAuthApi* api_dev = nightauth_create_ex(
    APP_NAME, APP_API_KEY, APP_VERSION, APP_TOKEN,
    "http://127.0.0.1:8787/api/v1",   // custom API URL (NULL = default)
    true                               // allow_insecure_loopback
);

// Inspect or rotate the identity (name, api key, version) of a live handle
char cur_name[128], cur_key[128], cur_ver[32];
nightauth_get_identity(api, cur_name, sizeof(cur_name), cur_key, sizeof(cur_key), cur_ver, sizeof(cur_ver));
nightauth_set_identity(api, "MyApp", "new_api_key", "1.1");
```

### 20. Server Public Keys & Default Constants

The SDK pins the server's signing and sealed-box public keys. Defaults ship as compile-time constants (`NIGHTAUTH_DEFAULT_API_URL`, `NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY` in `sdk/nightauth.h`), and the implementation exports mutable copies (`NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY`) that self-hosted operators can override per handle:

```c
// Point a handle at a self-hosted NightAuth instance with its own keys
nightauth_set_server_keys(api,
    "a1b2c3...",   // signing public key (64 hex chars)
    "d4e5f6...");  // sealed-box public key (64 hex chars)
```

### 21. License Login with 2FA (`nightauth_license_with_totp`)

For accounts whose license flow enforces TOTP, pass the authenticator code together with the license key:

```c
if (nightauth_license_with_totp(api, license_key, "123456")) {
    printf("License verified with 2FA. Access granted.\n");
}
```

### 22. Change Account Username

Requires the current password. **Note:** for security the server closes the session after a successful rename — re-login with the new username.

```c
if (nightauth_change_username(api, "new_username", password)) {
    printf("Username changed to %s. Please sign in again.\n",
           nightauth_get_user(api)->username);
}
```

### 23. Verify Binary File Hash

Validates a SHA-256 checksum of your distributed binary against the server-side registry of authorized releases:

```c
if (nightauth_verify_file_hash(api, "a1b2c3d4...")) {
    printf("Integrity verified: checksum matches an authorized release.\n");
} else {
    fprintf(stderr, "Verification failed: %s\n", nightauth_get_response(api)->message);
}
```

### 24. Application Telemetry & Online Users

```c
// Aggregated application statistics
NightAuthAppStats stats;
if (nightauth_get_stats(api, &stats)) {
    printf("Users: %lld (active %lld, banned %lld)\n",
           (long long)stats.total_users, (long long)stats.active_users, (long long)stats.banned_users);
    printf("Licenses: %lld used / %lld unused | Version: %s\n",
           (long long)stats.used_licenses, (long long)stats.unused_licenses, stats.version);
}

// Currently online users (returns the number of names written)
char online[16][128];
int64_t total_online = 0;
int count = nightauth_get_online_users(api, &total_online, online, 16);
for (int i = 0; i < count; i++) {
    printf("Online: %s\n", online[i]);
}

// Static application metadata (name, version, totals)
NightAuthAppData* app = nightauth_get_app(api);
printf("App: %s %s | Users: %lld | Online: %lld\n",
       app->application, app->version, (long long)app->total_users, (long long)app->online_users);
```

### 25. Hardware ID (HWID) Management

```c
// Read the machine fingerprint computed by the SDK
char hwid[128] = {0};
nightauth_get_hardware_id(hwid, sizeof(hwid));

// Bind the current session handle to an explicit HWID
nightauth_set_hardware_id(api, hwid);

// Globally override the fingerprint reported by all future handles
// (useful for testing or custom fingerprinting schemes)
nightauth_set_hardware_id_override("custom-machine-id");
```

### 26. Paginated Chat History (`nightauth_get_chat_messages_ex`)

Fetches messages older than a given ID (pass `0` for the latest page) and optionally marks them as read server-side:

```c
NightAuthChatMessage hist[25];
int count = nightauth_get_chat_messages_ex(api, "general", 0, true, hist, 25);
for (int i = 0; i < count; i++) {
    printf("[%s] %s: %s (#%s)\n", hist[i].timestamp, hist[i].author, hist[i].message, hist[i].id);
}
```

### 27. Advanced Server-Sided Webhooks (`nightauth_webhook_ex`)

Extended webhook with query parameters, optional file attachment, and HTTP status output:

```c
int status = 0;
const char* json = "{\"content\":\"Client notification\"}";
const void* report_data = NULL;   // optional attachment bytes
size_t report_len = 0;

if (nightauth_webhook_ex(api, "7kR0UedlVI", "Client notification",
                         "urgent=true", json,
                         report_data, report_len, "report.bin", &status)) {
    printf("Webhook dispatched: HTTP %d\n", status);
}
```

### 28. Advanced Realtime Event Streaming (`events_run_ex`, `events_cancel`)

`nightauth_events_run_ex` delivers each SSE event together with its server event ID (enabling resume via `last_event_id`). `nightauth_events_cancel` interrupts a running stream from another thread (e.g. on user input):

```c
static bool handle_event_ex(const char* event_json, const char* event_id, void* user) {
    printf("[EVENT %s] %s\n", event_id ? event_id : "-", event_json);
    return true; // Return true to keep the stream alive
}

// Another thread can call nightauth_events_cancel(api) to stop the loop
if (nightauth_events_run_ex(api, ticket, NULL, handle_event_ex, NULL)) {
    printf("Event stream closed cleanly.\n");
}
```

### 29. Signed & Verified Self-Updates

Verify a signed update envelope against a pinned Ed25519 public key, download it, or hand the verified URL to the server-assisted apply flow:

```c
// 1. Verify a signed update manifest received from the server
NightAuthVerifiedUpdate update;
char err[256] = {0};
if (nightauth_signed_update_verify(envelope_json, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                   "MyApp", /*min_sequence=*/1, &update, err, sizeof(err))) {
    printf("Update v%s -> seq %llu (%llu bytes)\n", update.version,
           (unsigned long long)update.sequence, (unsigned long long)update.size);

    // 2. Download the verified artifact to disk
    if (nightauth_signed_update_download(&update, "update.pkg", err, sizeof(err))) {
        printf("Update downloaded and digest-verified.\n");
    }
}

// 3. Or download the official installer directly from a manifest URL
nightauth_signed_update_installer_download(manifest_url, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                           "MyApp", "installer.exe", err, sizeof(err));

// 4. Server-assisted apply: hand the pending download URL to the API handle
if (nightauth_apply_update(api, download_url)) {
    printf("Required version: %s\n", nightauth_get_required_version(api));
}
const char* pending = nightauth_get_pending_download_url(api); // NULL when none
```

---

## 🖥️ Run the Console and Desktop Clients

Launch either reference client from the `source` directory or build with MSBuild:

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

---

## 🆚 Why NightAuth? (KeyAuth & Auth.gg Comparison)

| Feature | NightAuth | KeyAuth | Auth.gg |
| :--- | :---: | :---: | :---: |
| **Cryptography** | **End-to-End Curve25519 (X25519) + ChaCha20-Poly1305** | Static AES-CBC (Vulnerable to MITM & Proxy Interception) | Legacy HTTPS / Plain JSON |
| **Hardware ID (HWID)** | **Multi-Component Cryptographic Hash (Anti-Spoofing)** | Raw WMI Strings (Trivially Spoofed) | Simple Registry / MAC |
| **Edge Network Latency** | **< 35ms Global Anycast Edge Network** | Single-Origin VPS (Frequent Network Outages) | Unmaintained / High Latency |
| **Reseller System & Bots** | **Built-in Multi-Tier Credit Balances + Discord & Telegram Bots** | Manual API / Third-Party Scripts | Incomplete / Deprecated |
| **In-Memory Streaming** | **Zero-Disk Decryption with Direct Memory Execution & Buffer Wiping** | Disk Writes Required | Not Supported |
| **Internationalization** | **10 Languages Supported Out of the Box across Documentation & SDKs** | English Only | English Only |

Read the full architectural benchmark: [NightAuth vs KeyAuth vs Auth.gg](https://nightauth.org/blog/nightauth-vs-keyauth-authgg-comparison)

---

## 📄 License

This project is licensed under the **Elastic License 2.0**.

* You may not provide the software to third parties as a hosted or managed service.
* You may not move, change, disable, or circumvent the license key functionality.
* You may not alter, remove, or obscure any licensing or copyright notices.

See the [LICENSE](LICENSE) file for the full legal text.
