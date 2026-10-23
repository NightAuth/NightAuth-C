# NightAuth C SDK — GitHub'da Bize Yıldız Verin ⭐

Resmi **NightAuth** C11 Header-Only SDK ve Referans İstemci Paketi (Modern **Win32 Pure GDI+** Masaüstü Arayüzü ve Etkileşimli Terminal CLI). C yazılımlarınızı tersine mühendislik ve korsanlığa karşı dakikalar içinde koruyun.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Video Eğitimleri & Kurulum Kılavuzları
> NightAuth Yönetim Panelinde uygulama, plan, abonelik, bulut değişkenleri, webhook ve lisansların nasıl yapılandırılacağını anlatan resmi video rehberleri ve adım adım kılavuzlar **yakında yayınlanacaktır**.

---

## 📸 Ekran Görüntüleri

### Masaüstü Uygulaması (Yerel Win32 GDI Modern Koyu Tema)

| Giriş ve Kimlik Doğrulama Ekranı | Kullanıcı Paneli ve Abonelik Yönetimi |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Konsol Uygulaması (Etkileşimli Terminal CLI)

| Terminal Giriş Ekranı | Etkileşimli Konsol Paneli |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Önerilen Güvenlik Uygulamaları

* **Yerel İkili Dosya Sanallaştırma:** Yerel C ikili dosyaları IDA Pro, Ghidra ve Binary Ninja ile statik analize açıktır. Üretim sürümleri için derlenmiş yürütülebilir dosyaları **VMProtect**, **Themida** veya **CodeVirtualizer** gibi ticari kod sanallaştırıcılar ile koruyun.
* **Derleme Zamanı Dize Şifreleme:** `.rdata` bölümünde hassas dizeleri veya API belirteçlerini düz metin olarak bırakmayın. Dizeleri yalnızca gerektiği anda çözmek için derleme zamanı XOR dize gizleme makroları kullanın.
* **Bellekte Güvenli Dosya Yürütme:** Korunan yükleri veya DLL'leri `nightauth_download_file(api, ticket, ...)` aracılığıyla indirirken, **baytları asla fiziksel diske yazmayın**. Yükleri doğrudan RAM'den yükleyin ve çalıştırın (örn. yansıtıcı PE yükleme veya manuel haritalama) ve ardından arabelleği hemen `SecureZeroMemory()` ile temizleyin.
* **Donanım ve Bütünlük Doğrulaması:** Çalışma zamanı kancalamasını ve hata ayıklayıcı bağlanmasını azaltmak için üretim sürümlerinde anti-debugging ve bellek bütünlüğü denetimlerini etkin tutun.

---

## 📦 Gereksinimler ve Derleme

**C11** veya üstü ve **Visual Studio 2026** (MSVC araç seti `v145`) gerektirir. Birlikte verilen projeler `v145` için yapılandırılmıştır: konsol projesi `<PlatformToolset>v145</PlatformToolset>` değerini koşulsuz sabitler, masaüstü projesi ise yalnızca araç seti belirtilmediğinde varsayılan olarak `v145` kullanır. **Visual Studio 2022** (`v143`) ile derlemek yalnızca `console/NightAuth-Console.vcxproj` içindeki araç setini elle değiştirirseniz çalışır (veya masaüstü istemcisi için `/p:PlatformToolset=v143` geçerseniz).

Windows bağımlılıkları:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Yerel Windows Kriptografi ve Soketler)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Masaüstü Grafik Arayüz Çizimi)
* Paket içeriğindeki `libcurl` ve `LibreSSL` (OpenSSL uyumlu, başlıklar `include/openssl/` altında) (`deps/curl_win64`)

> [!NOTE]
> SDK başlığı `<stdatomic.h>` kullanır (C11 atomikleri). `sdk/nightauth.h` dosyasını kendi MSVC projenize entegre ederken derleyici bayrağı olarak `/experimental:c11atomics` ekleyin (birlikte verilen her iki `.vcxproj` dosyası da bunu zaten ayarlar). Bu bayrak olmadan MSVC, şifreli oturum katmanının kullandığı atomik işlemleri derleyemez.

### Çözümü MSBuild ile Derleme

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

### Çalışma Zamanında Dağıtım

Her iki istemci projesi de libcurl **import kütüphanesine** (`libcurl.dll.a`) bağlanır; bu nedenle `libcurl-x64.dll` dosyası (`deps/curl_win64/curl-8.23.0_5-win64-mingw/bin` içinden) **derlenen `.exe` dosyalarının yanında dağıtılmalıdır**, aksi halde istemciler başlamaz. Masaüstü projesi ayrıca `shell32.lib` bağlar.

---

## ⚙️ Yapılandırma & Başlatma

NightAuth panelindeki uygulama bilgilerinizi tanımlayın:

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

### Kriptografik El Sıkışmayı Başlatma (`init`)

Tüm işlemlerden önce `init()` çağrılmalıdır. Diffie-Hellman (X25519) anahtar değişimi ve şifreli oturumu kurar:

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

## 🚀 SDK Kullanım Kılavuzu

### 1. Oturum Kontrolü & Kara Liste Durumu

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

### 2. Kullanıcı Adı & Parola ile Giriş (ve 2FA)

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

### 3. Lisans Anahtarı ile Kayıt

```c
if (nightauth_register(api, username, password, license_key, email)) {
    printf("Registered successfully: %s\n", nightauth_get_user(api)->username);
} else {
    fprintf(stderr, "Registration failed: %s\n", nightauth_get_response(api)->message);
}
```

### 4. Sadece Lisans Anahtarı ile Giriş (Key Only)

```c
if (nightauth_license(api, license_key)) {
    printf("License verified successfully. Access granted.\n");
} else {
    fprintf(stderr, "Access denied: %s\n", nightauth_get_response(api)->message);
}
```

### 5. Abonelik Yükseltme / Anahtar Kullanma (Upgrade)

```c
if (nightauth_upgrade(api, username, new_license_key)) {
    printf("Subscription upgraded successfully.\n");
} else {
    fprintf(stderr, "Upgrade failed: %s\n", nightauth_get_response(api)->message);
}
```

### 6. Donanım Kimliği Sıfırlama (HWID Reset)

```c
if (nightauth_reset_hwid(api, username, password, totp_code)) {
    printf("HWID reset successfully. You can now log in on this machine.\n");
} else {
    fprintf(stderr, "HWID reset failed: %s\n", nightauth_get_response(api)->message);
}
```

### 7. Şifremi Unuttum (Forgot Password & Reset)

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

### 8. Kullanıcı Profil Bilgileri (`result.ToUserData()`)

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

### 9. Bulut Değişkenleri (Genel)

```c
char motd[256] = {0};
if (nightauth_get_global_var(api, "server_motd", motd, sizeof(motd))) {
    printf("Cloud Variable: %s\n", motd);
}
```

### 10. Kullanıcı Değişkenleri (User Variables)

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

### 11. Uygulama Denetim Günlükleri

```c
nightauth_log(api, "Suspicious memory modification attempt detected.");
```

### 12. Kullanıcı Engelleme & Cihazı Yasaklama

```c
nightauth_ban(api, "Security tampering detected by Anti-Cheat.");
```

### 13. Sunucu Taraflı Webhook'lar

```c
int status = 0;
const char* payload = "{\"content\":\"Client notification\"}";

if (nightauth_webhook(api, "7kR0UedlVI", payload, NULL, &status)) {
    printf("Webhook Response Status: HTTP %d\n", status);
}
```

### 14. Bellekte Güvenli Dosya İndirme

Dosya indirmeleri, `nightauth_access_ticket()` ile alınan **kapsamlı bir erişim bileti** gerektirir (sunucu, dosyayı sunmadan önce bileti doğrular):

```c
// Hedef dosya için kapsamlı erişim bileti iste ("file" kapsamı)
char ticket[128] = {0};
int64_t expires_in = 0;
if (!nightauth_access_ticket(api, "file", "385624", ticket, sizeof(ticket), &expires_in)) {
    fprintf(stderr, "Erişim bileti reddedildi: %s\n", nightauth_get_response(api)->message);
    return 1;
}

char file_name[128] = {0};
uint8_t file_buf[65536];
size_t downloaded_len = 0;

if (nightauth_download_file(api, ticket, file_name, file_buf, sizeof(file_buf), &downloaded_len)) {
    printf("%s (%zu bayt) güvenle indirildi.\n", file_name, downloaded_len);
    // Yükü doğrudan bellekte işleyin veya çalıştırın (Reflective PE / Shellcode)
    // Yürütmeden sonra bellek arabelleğini güvenle silin
    SecureZeroMemory(file_buf, downloaded_len);
}
```

### 15. Canlı Sohbet Kanalları

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

### 16. Gerçek Zamanlı Olay Dinleyicileri (SSE)

```c
static bool handle_event(const char* event_json, void* user_data) {
    printf("[EVENT] %s\n", event_json);
    return true; // Return true to keep stream alive
}

nightauth_events_run(api, "ticket_value", NULL, handle_event, NULL);
```

### 17. İki Faktörlü Kimlik Doğrulama (2FA)

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

### 18. Oturum Yaşam Döngüsü & Çıkış

```c
// Heartbeat ping to keep session alive
nightauth_heartbeat(api);

// Revoke and terminate session
nightauth_logout(api);

// Release memory allocations
nightauth_destroy(api);
```

### 19. Gelişmiş İstemci Yapılandırması

```c
// İstek başına milisaniye cinsinden zaman aşımı (sonraki tüm ağ çağrılarını etkiler)
nightauth_set_request_timeout(api, 15000);

// Loopback/self-hosted geliştirme uç noktaları için şifresiz HTTP'e izin ver
NightAuthApi* api_dev = nightauth_create_ex(
    APP_NAME, APP_API_KEY, APP_VERSION, APP_TOKEN,
    "http://127.0.0.1:8787/api/v1",   // özel API URL'si (NULL = varsayılan)
    true                               // allow_insecure_loopback
);

// Aktif bir tanıtıcının kimliğini (ad, api key, sürüm) inceleyin veya değiştirin
char cur_name[128], cur_key[128], cur_ver[32];
nightauth_get_identity(api, cur_name, sizeof(cur_name), cur_key, sizeof(cur_key), cur_ver, sizeof(cur_ver));
nightauth_set_identity(api, "MyApp", "new_api_key", "1.1");
```

### 20. Sunucu Ortak Anahtarları ve Varsayılan Sabitler

SDK, sunucunun imza ve sealed-box ortak anahtarlarını sabitler. Varsayılan değerler derleme zamanı sabitleri olarak gelir (`sdk/nightauth.h` içinde `NIGHTAUTH_DEFAULT_API_URL`, `NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY`); uygulama ayrıca self-hosted operatörlerin tanıtıcı başına geçersiz kılabileceği değiştirilebilir kopyalar dışa aktarır (`NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY`):

```c
// Bir tanıtıcıyı kendi anahtarları olan self-hosted bir NightAuth örneğine yönlendir
nightauth_set_server_keys(api,
    "a1b2c3...",   // imza ortak anahtarı (64 hex karakter)
    "d4e5f6...");  // sealed-box ortak anahtarı (64 hex karakter)
```

### 21. 2FA ile Lisans Girişi (`nightauth_license_with_totp`)

Lisans akışı TOTP gerektiren hesaplar için kimlik doğrulama uygulaması kodunu lisans anahtarıyla birlikte geçirin:

```c
if (nightauth_license_with_totp(api, license_key, "123456")) {
    printf("Lisans 2FA ile doğrulandı. Erişim izni verildi.\n");
}
```

### 22. Kullanıcı Adını Değiştirme

Geçerli parola gerektirir. **Not:** güvenlik nedeniyle sunucu, başarılı bir yeniden adlandırmanın ardından oturumu kapatır — yeni kullanıcı adıyla tekrar giriş yapın.

```c
if (nightauth_change_username(api, "new_username", password)) {
    printf("Kullanıcı adı %s olarak değiştirildi. Lütfen tekrar giriş yapın.\n",
           nightauth_get_user(api)->username);
}
```

### 23. Dosya Hash Doğrulama

Dağıtılan ikili dosyanızın SHA-256 sağlama toplamını, sunucu tarafı yetkili sürüm kaydına karşı doğrular:

```c
if (nightauth_verify_file_hash(api, "a1b2c3d4...")) {
    printf("Bütünlük doğrulandı: sağlama toplamı yetkili bir sürümle eşleşiyor.\n");
} else {
    fprintf(stderr, "Doğrulama başarısız: %s\n", nightauth_get_response(api)->message);
}
```

### 24. Uygulama Telemetrisi ve Çevrimiçi Kullanıcılar

```c
// Toplu uygulama istatistikleri
NightAuthAppStats stats;
if (nightauth_get_stats(api, &stats)) {
    printf("Kullanıcılar: %lld (aktif %lld, yasaklı %lld)\n",
           (long long)stats.total_users, (long long)stats.active_users, (long long)stats.banned_users);
    printf("Lisanslar: %lld kullanılan / %lld kullanılmayan | Sürüm: %s\n",
           (long long)stats.used_licenses, (long long)stats.unused_licenses, stats.version);
}

// Şu anda çevrimiçi olan kullanıcılar (yazılan isim sayısını döndürür)
char online[16][128];
int64_t total_online = 0;
int count = nightauth_get_online_users(api, &total_online, online, 16);
for (int i = 0; i < count; i++) {
    printf("Çevrimiçi: %s\n", online[i]);
}

// Statik uygulama meta verileri (ad, sürüm, toplamlar)
NightAuthAppData* app = nightauth_get_app(api);
printf("App: %s %s | Kullanıcılar: %lld | Çevrimiçi: %lld\n",
       app->application, app->version, (long long)app->total_users, (long long)app->online_users);
```

### 25. Hardware ID (HWID) Yönetimi

```c
// SDK'nın hesapladığı makine parmak izini oku
char hwid[128] = {0};
nightauth_get_hardware_id(hwid, sizeof(hwid));

// Mevcut oturum tanıtıcısını açık bir HWID'ye bağla
nightauth_set_hardware_id(api, hwid);

// Gelecekteki tüm tanıtıcıların bildirdiği parmak izini global olarak geçersiz kıl
// (testler veya özel parmak izi şemaları için kullanışlıdır)
nightauth_set_hardware_id_override("custom-machine-id");
```

### 26. Sayfalanmış Sohbet Geçmişi (`nightauth_get_chat_messages_ex`)

Verilen bir ID'den eski mesajları getirir (en son sayfa için `0` geçin) ve bunları isteğe bağlı olarak sunucuda okundu olarak işaretler:

```c
NightAuthChatMessage hist[25];
int count = nightauth_get_chat_messages_ex(api, "general", 0, true, hist, 25);
for (int i = 0; i < count; i++) {
    printf("[%s] %s: %s (#%s)\n", hist[i].timestamp, hist[i].author, hist[i].message, hist[i].id);
}
```

### 27. Gelişmiş Sunucu Taraflı Webhook'lar (`nightauth_webhook_ex`)

Sorgu parametreleri, isteğe bağlı dosya eki ve HTTP durumu çıkışıyla genişletilmiş webhook:

```c
int status = 0;
const char* json = "{\"content\":\"Client notification\"}";
const void* report_data = NULL;   // isteğe bağlı ek dosya baytları
size_t report_len = 0;

if (nightauth_webhook_ex(api, "7kR0UedlVI", "Client notification",
                         "urgent=true", json,
                         report_data, report_len, "report.bin", &status)) {
    printf("Webhook gönderildi: HTTP %d\n", status);
}
```

### 28. Gelişmiş Gerçek Zamanlı Olay Akışı (`events_run_ex`, `events_cancel`)

`nightauth_events_run_ex`, her SSE olayını sunucu olay kimliğiyle birlikte teslim eder (`last_event_id` ile devam etmeyi sağlar). `nightauth_events_cancel`, çalışan bir akışı başka bir iş parçacığından keser (örneğin kullanıcı girdisinde):

```c
static bool handle_event_ex(const char* event_json, const char* event_id, void* user) {
    printf("[EVENT %s] %s\n", event_id ? event_id : "-", event_json);
    return true; // Akışı canlı tutmak için true döndürün
}

// Başka bir iş parçacığı, döngüyü durdurmak için nightauth_events_cancel(api) çağırabilir
if (nightauth_events_run_ex(api, ticket, NULL, handle_event_ex, NULL)) {
    printf("Olay akışı temiz şekilde kapatıldı.\n");
}
```

### 29. İmzalı ve Doğrulanmış Kendi Kendine Güncellemeler

İmzalı bir güncelleme zarfını sabitlenmiş bir Ed25519 ortak anahtarına karşı doğrular, indirir veya doğrulanmış URL'yi sunucu destekli uygulama akışına teslim eder:

```c
// 1. Sunucudan alınan imzalı güncelleme manifestini doğrula
NightAuthVerifiedUpdate update;
char err[256] = {0};
if (nightauth_signed_update_verify(envelope_json, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                   "MyApp", /*min_sequence=*/1, &update, err, sizeof(err))) {
    printf("Güncelleme v%s -> seq %llu (%llu bayt)\n", update.version,
           (unsigned long long)update.sequence, (unsigned long long)update.size);

    // 2. Doğrulanmış yapıtı diske indir
    if (nightauth_signed_update_download(&update, "update.pkg", err, sizeof(err))) {
        printf("Güncelleme indirildi ve özeti doğrulandı.\n");
    }
}

// 3. Veya resmi yükleyiciyi doğrudan bir manifest URL'sinden indir
nightauth_signed_update_installer_download(manifest_url, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                           "MyApp", "installer.exe", err, sizeof(err));

// 4. Sunucu destekli uygulama: bekleyen indirme URL'sini API tanıtıcısına ver
if (nightauth_apply_update(api, download_url)) {
    printf("Gerekli sürüm: %s\n", nightauth_get_required_version(api));
}
const char* pending = nightauth_get_pending_download_url(api); // Bekleyen yoksa NULL
```

---

## 🖥️ Konsol ve Masaüstü İstemcilerini Çalıştırma

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Lisans

Bu proje **Elastic License 2.0** kapsamında lisanslanmıştır.

* Yazılımı üçüncü taraflara barındırılan veya yönetilen bir hizmet olarak sağlayamazsınız.
* Lisans anahtarı işlevselliğini değiştiremez, devre dışı bırakamaz veya atlayamazsınız.
* Lisans veya telif hakkı bildirimlerini kaldıramazsınız.

Tam metin için [LICENSE](../LICENSE) dosyasına bakın.
