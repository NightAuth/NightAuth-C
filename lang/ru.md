# NightAuth C SDK — Поставьте звезду на GitHub ⭐

Официальный **NightAuth** C11 Header-Only SDK и набор эталонных клиентов (Современный графический интерфейс **Win32 Pure GDI+** и интерактивный терминал CLI). Защитите нативное ПО на C от реверс-инжиниринга и пиратства за считанные минуты.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Видеоинструкции и руководства
> Официальные видеоуроки и пошаговые руководства по настройке приложений, тарифов, подписок, переменных, вебхуков и ключей в панели NightAuth **будут опубликованы в ближайшее время**.

---

## 📸 Скриншоты

### Десктопное приложение (Нативный Win32 GDI современная темная тема)

| Окно входа и авторизации | Панель управления и подписки |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Консольное приложение (Интерактивный CLI)

| Вход через терминал | Панель управления в консоли |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Рекомендуемые практики безопасности

* **Виртуализация нативных бинарных файлов:** Нативные C исполняемые файлы подвержены статическому анализу через IDA Pro, Ghidra и Binary Ninja. Для релизов защищайте скомпилированные файлы коммерческими виртуализаторами кода, такими как **VMProtect**, **Themida** или **CodeVirtualizer**.
* **Шифрование строк во время компиляции:** Не оставляйте важные строки или токены API в открытом виде в секции `.rdata`. Используйте макросы XOR-обфускации строк, чтобы расшифровывать строки только в момент их вызова.
* **Безопасное выполнение файлов в оперативной памяти:** При потоковой загрузке защищенных полезных нагрузок или DLL через `nightauth_download_file(api, ticket, ...)`, **никогда не записывайте байты на диск**. Загружайте и выполняйте полезную нагрузку напрямую из RAM (например, reflective PE loading или manual mapping) и сразу очищайте буфер с помощью `SecureZeroMemory()`.
* **Проверка аппаратного обеспечения и целостности:** Держите активными проверки anti-debugging, целостности памяти и таймингов в продакшене.

---

## 📦 Требования и Сборка

Требуется **C11** или новее и **Visual Studio 2026** (набор инструментов MSVC `v145`). Включённые проекты настроены на `v145`: проект консоли безусловно фиксирует `<PlatformToolset>v145</PlatformToolset>`, а проект десктопа использует `v145` по умолчанию только когда набор инструментов не задан. Сборка с **Visual Studio 2022** (`v143`) работает только при ручной смене набора инструментов в `console/NightAuth-Console.vcxproj` (или передаче `/p:PlatformToolset=v143` для десктопного клиента).

Зависимости для Windows:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Нативная криптография и сокеты Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Отрисовка GUI)
* Включенные библиотеки `libcurl` и `LibreSSL` (совместим с OpenSSL, заголовки в `include/openssl/`) (в `deps/curl_win64`)

> [!NOTE]
> Заголовок SDK использует `<stdatomic.h>` (атомики C11). При интеграции `sdk/nightauth.h` в собственный проект MSVC добавьте флаг компилятора `/experimental:c11atomics` (оба включённых `.vcxproj` уже задают его). Без него MSVC не сможет скомпилировать атомарные операции, используемые слоем зашифрованной сессии.

### Сборка решения с помощью MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

### Развёртывание при выполнении

Оба клиентских проекта компонуются с **библиотекой импорта** libcurl (`libcurl.dll.a`), поэтому DLL `libcurl-x64.dll` (из `deps/curl_win64/curl-8.23.0_5-win64-mingw/bin`) **должна распространяться вместе с собранными `.exe`**, иначе клиенты не запустятся. Десктопный проект дополнительно компонуется с `shell32.lib`.

---

## ⚙️ Конфигурация и инициализация

Укажите учетные данные вашего приложения из панели управления NightAuth:

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

### Криптографическое рукопожатие (`init`)

Вызовите `init()` перед вызовом других методов. Устанавливает сеанс с обменом ключами Diffie-Hellman (X25519) и сквозным шифрованием:

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

## 🚀 Руководство по использованию SDK

### 1. Проверка статуса сессии и черный список

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

### 2. Вход по логину и паролю (и 2FA)

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

### 3. Регистрация с лицензионным ключом

```c
if (nightauth_register(api, username, password, license_key, email)) {
    printf("Registered successfully: %s\n", nightauth_get_user(api)->username);
} else {
    fprintf(stderr, "Registration failed: %s\n", nightauth_get_response(api)->message);
}
```

### 4. Вход только по ключу лицензии (Key Only)

```c
if (nightauth_license(api, license_key)) {
    printf("License verified successfully. Access granted.\n");
} else {
    fprintf(stderr, "Access denied: %s\n", nightauth_get_response(api)->message);
}
```

### 5. Обновление подписки / Активация ключа (Upgrade)

```c
if (nightauth_upgrade(api, username, new_license_key)) {
    printf("Subscription upgraded successfully.\n");
} else {
    fprintf(stderr, "Upgrade failed: %s\n", nightauth_get_response(api)->message);
}
```

### 6. Сброс привязки железа (HWID Lock)

```c
if (nightauth_reset_hwid(api, username, password, totp_code)) {
    printf("HWID reset successfully. You can now log in on this machine.\n");
} else {
    fprintf(stderr, "HWID reset failed: %s\n", nightauth_get_response(api)->message);
}
```

### 7. Восстановление пароля (Forgot Password & Reset)

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

### 8. Профиль пользователя (`result.ToUserData()`)

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

### 9. Облачные переменные (Глобальные)

```c
char motd[256] = {0};
if (nightauth_get_global_var(api, "server_motd", motd, sizeof(motd))) {
    printf("Cloud Variable: %s\n", motd);
}
```

### 10. Пользовательские переменные

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

### 11. Журнал аудита и логи

```c
nightauth_log(api, "Suspicious memory modification attempt detected.");
```

### 12. Бан пользователя и блокировка устройства

```c
nightauth_ban(api, "Security tampering detected by Anti-Cheat.");
```

### 13. Серверные вебхуки

```c
int status = 0;
const char* payload = "{\"content\":\"Client notification\"}";

if (nightauth_webhook(api, "7kR0UedlVI", payload, NULL, &status)) {
    printf("Webhook Response Status: HTTP %d\n", status);
}
```

### 14. Безопасная загрузка файлов в RAM

Загрузка файлов требует **тикета доступа с областью действия**, получаемого через `nightauth_access_ticket()` (сервер проверяет тикет перед выдачей файла):

```c
// Запросить тикет доступа с областью действия для целевого файла (область "file")
char ticket[128] = {0};
int64_t expires_in = 0;
if (!nightauth_access_ticket(api, "file", "385624", ticket, sizeof(ticket), &expires_in)) {
    fprintf(stderr, "Тикет доступа отклонён: %s\n", nightauth_get_response(api)->message);
    return 1;
}

char file_name[128] = {0};
uint8_t file_buf[65536];
size_t downloaded_len = 0;

if (nightauth_download_file(api, ticket, file_name, file_buf, sizeof(file_buf), &downloaded_len)) {
    printf("Загружен %s (%zu байт) безопасно.\n", file_name, downloaded_len);
    // Обработайте или выполните payload прямо в памяти (Reflective PE / Shellcode)
    // Надёжно сотрите буфер в памяти после выполнения
    SecureZeroMemory(file_buf, downloaded_len);
}
```

### 15. Онлайн-чат

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

### 16. Обработчики событий реального времени (SSE)

```c
static bool handle_event(const char* event_json, void* user_data) {
    printf("[EVENT] %s\n", event_json);
    return true; // Return true to keep stream alive
}

nightauth_events_run(api, "ticket_value", NULL, handle_event, NULL);
```

### 17. Двухфакторная аутентификация (2FA)

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

### 18. Жизненный цикл сессии и выход

```c
// Heartbeat ping to keep session alive
nightauth_heartbeat(api);

// Revoke and terminate session
nightauth_logout(api);

// Release memory allocations
nightauth_destroy(api);
```

### 19. Расширенная настройка клиента

```c
// Таймаут на запрос в миллисекундах (влияет на все последующие сетевые вызовы)
nightauth_set_request_timeout(api, 15000);

// Разрешить незашифрованный HTTP для loopback/self-hosted эндпоинтов разработки
NightAuthApi* api_dev = nightauth_create_ex(
    APP_NAME, APP_API_KEY, APP_VERSION, APP_TOKEN,
    "http://127.0.0.1:8787/api/v1",   // пользовательский URL API (NULL = по умолчанию)
    true                               // allow_insecure_loopback
);

// Просмотреть или сменить идентичность (имя, api key, версию) активного хендла
char cur_name[128], cur_key[128], cur_ver[32];
nightauth_get_identity(api, cur_name, sizeof(cur_name), cur_key, sizeof(cur_key), cur_ver, sizeof(cur_ver));
nightauth_set_identity(api, "MyApp", "new_api_key", "1.1");
```

### 20. Публичные ключи сервера и константы по умолчанию

SDK закрепляет публичные ключи подписи и sealed-box сервера. Значения по умолчанию поставляются как константы времени компиляции (`NIGHTAUTH_DEFAULT_API_URL`, `NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY` в `sdk/nightauth.h`), а реализация экспортирует изменяемые копии (`NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY`), которые операторы self-hosted могут переопределять для каждого хендла:

```c
// Направить хендл на self-hosted инстанс NightAuth с собственными ключами
nightauth_set_server_keys(api,
    "a1b2c3...",   // публичный ключ подписи (64 hex-символа)
    "d4e5f6...");  // публичный ключ sealed-box (64 hex-символа)
```

### 21. Вход по лицензии с 2FA (`nightauth_license_with_totp`)

Для аккаунтов, чей лицензионный поток требует TOTP, передайте код аутентификатора вместе с лицензионным ключом:

```c
if (nightauth_license_with_totp(api, license_key, "123456")) {
    printf("Лицензия проверена с 2FA. Доступ разрешён.\n");
}
```

### 22. Смена имени пользователя

Требуется текущий пароль. **Примечание:** в целях безопасности сервер закрывает сессию после успешной смены имени — войдите снова с новым именем.

```c
if (nightauth_change_username(api, "new_username", password)) {
    printf("Имя пользователя изменено на %s. Пожалуйста, войдите снова.\n",
           nightauth_get_user(api)->username);
}
```

### 23. Проверка хеша файла

Проверяет SHA-256 контрольную сумму вашего распространяемого бинарника по серверному реестру авторизованных релизов:

```c
if (nightauth_verify_file_hash(api, "a1b2c3d4...")) {
    printf("Целостность проверена: контрольная сумма совпадает с авторизованным релизом.\n");
} else {
    fprintf(stderr, "Проверка не удалась: %s\n", nightauth_get_response(api)->message);
}
```

### 24. Телеметрия приложения и пользователи онлайн

```c
// Сводная статистика приложения
NightAuthAppStats stats;
if (nightauth_get_stats(api, &stats)) {
    printf("Пользователи: %lld (активны %lld, забанены %lld)\n",
           (long long)stats.total_users, (long long)stats.active_users, (long long)stats.banned_users);
    printf("Лицензии: %lld использовано / %lld не использовано | Версия: %s\n",
           (long long)stats.used_licenses, (long long)stats.unused_licenses, stats.version);
}

// Пользователи, сейчас находящиеся онлайн (возвращает количество записанных имён)
char online[16][128];
int64_t total_online = 0;
int count = nightauth_get_online_users(api, &total_online, online, 16);
for (int i = 0; i < count; i++) {
    printf("Онлайн: %s\n", online[i]);
}

// Статические метаданные приложения (имя, версия, суммы)
NightAuthAppData* app = nightauth_get_app(api);
printf("App: %s %s | Пользователи: %lld | Онлайн: %lld\n",
       app->application, app->version, (long long)app->total_users, (long long)app->online_users);
```

### 25. Управление Hardware ID (HWID)

```c
// Прочитать вычисленный SDK отпечаток машины
char hwid[128] = {0};
nightauth_get_hardware_id(hwid, sizeof(hwid));

// Привязать текущий хендл сессии к явному HWID
nightauth_set_hardware_id(api, hwid);

// Глобально переопределить отпечаток, сообщаемый всеми будущими хендлами
// (полезно для тестов или собственных схем фингерпринта)
nightauth_set_hardware_id_override("custom-machine-id");
```

### 26. Пагинированная история чата (`nightauth_get_chat_messages_ex`)

Получает сообщения старше заданного ID (передайте `0` для последней страницы) и опционально помечает их прочитанными на сервере:

```c
NightAuthChatMessage hist[25];
int count = nightauth_get_chat_messages_ex(api, "general", 0, true, hist, 25);
for (int i = 0; i < count; i++) {
    printf("[%s] %s: %s (#%s)\n", hist[i].timestamp, hist[i].author, hist[i].message, hist[i].id);
}
```

### 27. Расширенные серверные вебхуки (`nightauth_webhook_ex`)

Расширенный вебхук с параметрами запроса, опциональным вложением и выводом HTTP-статуса:

```c
int status = 0;
const char* json = "{\"content\":\"Client notification\"}";
const void* report_data = NULL;   // байты опционального вложения
size_t report_len = 0;

if (nightauth_webhook_ex(api, "7kR0UedlVI", "Client notification",
                         "urgent=true", json,
                         report_data, report_len, "report.bin", &status)) {
    printf("Вебхук отправлен: HTTP %d\n", status);
}
```

### 28. Расширенная потоковая передача событий (`events_run_ex`, `events_cancel`)

`nightauth_events_run_ex` доставляет каждое SSE-событие вместе с его серверным ID события (что позволяет возобновлять поток через `last_event_id`). `nightauth_events_cancel` прерывает работающий поток из другого потока (например, по вводу пользователя):

```c
static bool handle_event_ex(const char* event_json, const char* event_id, void* user) {
    printf("[EVENT %s] %s\n", event_id ? event_id : "-", event_json);
    return true; // Верните true, чтобы поток продолжал работу
}

// Другой поток может вызвать nightauth_events_cancel(api), чтобы остановить цикл
if (nightauth_events_run_ex(api, ticket, NULL, handle_event_ex, NULL)) {
    printf("Поток событий корректно закрыт.\n");
}
```

### 29. Подписанные и проверенные самообновления

Проверяет подписанный конверт обновления по закреплённому публичному ключу Ed25519, загружает его или передаёт проверенный URL в серверно-ассистируемый поток применения:

```c
// 1. Проверить подписанный манифест обновления, полученный от сервера
NightAuthVerifiedUpdate update;
char err[256] = {0};
if (nightauth_signed_update_verify(envelope_json, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                   "MyApp", /*min_sequence=*/1, &update, err, sizeof(err))) {
    printf("Обновление v%s -> seq %llu (%llu байт)\n", update.version,
           (unsigned long long)update.sequence, (unsigned long long)update.size);

    // 2. Загрузить проверенный артефакт на диск
    if (nightauth_signed_update_download(&update, "update.pkg", err, sizeof(err))) {
        printf("Обновление загружено, дайджест проверен.\n");
    }
}

// 3. Или загрузить официальный установщик напрямую по URL манифеста
nightauth_signed_update_installer_download(manifest_url, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                           "MyApp", "installer.exe", err, sizeof(err));

// 4. Серверно-ассистируемое применение: передать ожидающий URL загрузки хендлу API
if (nightauth_apply_update(api, download_url)) {
    printf("Требуемая версия: %s\n", nightauth_get_required_version(api));
}
const char* pending = nightauth_get_pending_download_url(api); // NULL, если ничего не ожидает
```

---

## 🖥️ Запуск приложений консоли и рабочего стола

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Лицензия

Проект распространяется под лицензией **Elastic License 2.0**.

* Запрещено предоставлять ПО третьим лицам в качестве хостинг-сервиса.
* Запрещено изменять или обходить систему лицензирования.
* Запрещено удалять уведомления об авторских правах.

Полный текст лицензии доступен в файле [LICENSE](../LICENSE).
