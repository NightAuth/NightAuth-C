# NightAuth C SDK — Star uns auf GitHub ⭐

Offizielles **NightAuth** C11 Header-Only SDK & Referenz-Client-Suite (Moderne Desktop-GUI mit **Win32 Pure GDI+** und interaktiver Terminal-CLI). Schützen Sie Ihre native C-Software in Minuten vor Reverse Engineering, Piraterie und Manipulation.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Video-Tutorials & Einrichtungsanleitungen
> Offizielle Video-Tutorials und Schritt-für-Schritt-Anleitungen zur Konfiguration von Anwendungen, Tarifen, Abonnements, Cloud-Variablen, Webhooks und Lizenzen im NightAuth-Dashboard **werden in Kürze veröffentlicht**.

---

## 📸 Screenshots

### Desktop-Anwendung (Natives Win32 GDI Modern Dark Theme)

| Anmelde- / Authentifizierungsfenster | Dashboard & Abonnementverwaltung |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Konsolenanwendung (Interaktives CLI-Terminal)

| Terminal-Anmeldung | Interaktives Konsolen-Dashboard |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Empfohlene Sicherheitspraktiken

* **Native Binärvirtualisierung:** Native C-Binärdateien unterliegen statischer Analyse über IDA Pro, Ghidra und Binary Ninja. Schützen Sie kompilierte Executables für Produktions-Releases mit kommerziellen Code-Virtualisierern und Protektoren wie **VMProtect**, **Themida** oder **CodeVirtualizer**.
* **String-Verschlüsselung zur Compile-Zeit:** Hinterlassen Sie keine sensiblen Strings oder API-Token im Klartext im `.rdata`-Bereich. Verwenden Sie XOR-String-Verschleierungs-Makros, um Strings erst im Bedarfsfall zu entschlüsseln.
* **Sichere Dateiausführung im Arbeitsspeicher:** Wenn Sie geschützte Payloads oder DLLs über `nightauth_download_file(api, ticket, ...)` streamen, **schreiben Sie niemals Bytes auf den physischen Speicher oder die Festplatte**. Laden und führen Sie Payloads direkt aus dem RAM aus (z. B. reflective PE-Loading oder manuelles Map-Injection) und überschreiben Sie den Puffer sofort mit `SecureZeroMemory()`.
* **Hardware- & Integritätsprüfung:** Halten Sie Anti-Debugging-, Speicherintegritäts- und Timing-Prüfungen in Produktions-Releases aktiv, um Runtime-Hooking und Debugger-Anhänge zu erschweren.

---

## 📦 Anforderungen und Erstellung

Erfordert **C11** oder neuer und **Visual Studio 2026** (MSVC-Toolset `v145`). Die mitgelieferten Projekte sind auf `v145` konfiguriert: Das Konsolenprojekt setzt `<PlatformToolset>v145</PlatformToolset>` bedingungslos, während das Desktop-Projekt `v145` nur dann als Standard verwendet, wenn kein Toolset angegeben ist. Ein Build mit **Visual Studio 2022** (`v143`) funktioniert nur, wenn das Toolset in `console/NightAuth-Console.vcxproj` manuell geändert wird (oder `/p:PlatformToolset=v143` für den Desktop-Client übergeben wird).

Windows-Abhängigkeiten:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Native Windows-Krypto und Sockets)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Desktop-GUI-Rendering)
* Integriertes `libcurl` und `LibreSSL` (OpenSSL-kompatibel, Header unter `include/openssl/`) (in `deps/curl_win64`)

> [!NOTE]
> Der SDK-Header verwendet `<stdatomic.h>` (C11-Atomics). Wenn du `sdk/nightauth.h` in dein eigenes MSVC-Projekt integrierst, füge den Compiler-Schalter `/experimental:c11atomics` hinzu (beide mitgelieferten `.vcxproj`-Dateien setzen ihn bereits). Ohne diesen Schalter kann MSVC die von der verschlüsselten Sitzungsschicht genutzten Atom-Operationen nicht kompilieren.

### Lösung mit MSBuild kompilieren

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

### Bereitstellung zur Laufzeit

Beide Client-Projekte binden gegen die **Importbibliothek** von libcurl (`libcurl.dll.a`), daher muss die DLL `libcurl-x64.dll` (aus `deps/curl_win64/curl-8.23.0_5-win64-mingw/bin`) **zusammen mit den kompilierten `.exe`-Dateien verteilt werden**, sonst starten die Clients nicht. Das Desktop-GUI-Projekt bindet zusätzlich `shell32.lib`.

---

## ⚙️ Konfiguration & Initialisierung

Definieren Sie Ihre Zugangsdaten aus dem NightAuth-Dashboard:

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

### Kryptografischen Handshake initialisieren (`init`)

Rufen Sie `init()` vor allen anderen Aufrufen auf. Dies baut den Schlüsselaustausch (X25519) und die verschlüsselte Sitzung auf:

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

## 🚀 SDK-Nutzungsleitfaden

### 1. Sitzungsprüfung & Blacklist-Status

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

### 2. Anmeldung mit Benutzername & Passwort (und 2FA)

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

### 3. Registrierung mit Lizenzschlüssel

```c
if (nightauth_register(api, username, password, license_key, email)) {
    printf("Registered successfully: %s\n", nightauth_get_user(api)->username);
} else {
    fprintf(stderr, "Registration failed: %s\n", nightauth_get_response(api)->message);
}
```

### 4. Nur mit Lizenzschlüssel anmelden (Key Only)

```c
if (nightauth_license(api, license_key)) {
    printf("License verified successfully. Access granted.\n");
} else {
    fprintf(stderr, "Access denied: %s\n", nightauth_get_response(api)->message);
}
```

### 5. Abonnement erweitern / Schlüssel einlösen (Upgrade)

```c
if (nightauth_upgrade(api, username, new_license_key)) {
    printf("Subscription upgraded successfully.\n");
} else {
    fprintf(stderr, "Upgrade failed: %s\n", nightauth_get_response(api)->message);
}
```

### 6. Hardware-ID zurücksetzen (HWID Lock)

```c
if (nightauth_reset_hwid(api, username, password, totp_code)) {
    printf("HWID reset successfully. You can now log in on this machine.\n");
} else {
    fprintf(stderr, "HWID reset failed: %s\n", nightauth_get_response(api)->message);
}
```

### 7. Passwort vergessen (Forgot Password & Reset)

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

### 8. Benutzerprofildaten (`result.ToUserData()`)

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

### 9. Cloud-Variablen (Global)

```c
char motd[256] = {0};
if (nightauth_get_global_var(api, "server_motd", motd, sizeof(motd))) {
    printf("Cloud Variable: %s\n", motd);
}
```

### 10. Benutzervariablen (User Variables)

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

### 11. Anwendungs-Audit-Logs

```c
nightauth_log(api, "Suspicious memory modification attempt detected.");
```

### 12. Benutzer sperren & Maschine sperren

```c
nightauth_ban(api, "Security tampering detected by Anti-Cheat.");
```

### 13. Server-Webhooks

```c
int status = 0;
const char* payload = "{\"content\":\"Client notification\"}";

if (nightauth_webhook(api, "7kR0UedlVI", payload, NULL, &status)) {
    printf("Webhook Response Status: HTTP %d\n", status);
}
```

### 14. Sicheres Herunterladen im Arbeitsspeicher

Datei-Downloads erfordern ein **bereichsbezogenes Zugriffsticket**, das mit `nightauth_access_ticket()` angefordert wird (der Server validiert das Ticket, bevor die Datei ausgeliefert wird):

```c
// Bereichsbezogenes Zugriffsticket für die Zieldatei anfordern (Scope "file")
char ticket[128] = {0};
int64_t expires_in = 0;
if (!nightauth_access_ticket(api, "file", "385624", ticket, sizeof(ticket), &expires_in)) {
    fprintf(stderr, "Zugriffsticket verweigert: %s\n", nightauth_get_response(api)->message);
    return 1;
}

char file_name[128] = {0};
uint8_t file_buf[65536];
size_t downloaded_len = 0;

if (nightauth_download_file(api, ticket, file_name, file_buf, sizeof(file_buf), &downloaded_len)) {
    printf("%s (%zu Bytes) sicher heruntergeladen.\n", file_name, downloaded_len);
    // Payload direkt im Speicher verarbeiten oder ausführen (Reflective PE / Shellcode)
    // Puffer nach der Ausführung sicher überschreiben
    SecureZeroMemory(file_buf, downloaded_len);
}
```

### 15. Echtzeit-Chatkanäle

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

### 16. Echtzeit-Ereignishandler (SSE)

```c
static bool handle_event(const char* event_json, void* user_data) {
    printf("[EVENT] %s\n", event_json);
    return true; // Return true to keep stream alive
}

nightauth_events_run(api, "ticket_value", NULL, handle_event, NULL);
```

### 17. Zwei-Faktor-Authentifizierung (2FA)

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

### 18. Sitzungslebenszyklus & Abmeldung

```c
// Heartbeat ping to keep session alive
nightauth_heartbeat(api);

// Revoke and terminate session
nightauth_logout(api);

// Release memory allocations
nightauth_destroy(api);
```

### 19. Erweiterte Client-Konfiguration

```c
// Timeout pro Anfrage in Millisekunden (wirkt sich auf alle folgenden Netzwerkaufrufe aus)
nightauth_set_request_timeout(api, 15000);

// Unverschlüsseltes HTTP für Loopback-/Self-hosted-Entwicklungsendpunkte erlauben
NightAuthApi* api_dev = nightauth_create_ex(
    APP_NAME, APP_API_KEY, APP_VERSION, APP_TOKEN,
    "http://127.0.0.1:8787/api/v1",   // benutzerdefinierte API-URL (NULL = Standard)
    true                               // allow_insecure_loopback
);

// Identität (Name, API-Key, Version) eines aktiven Handles auslesen oder rotieren
char cur_name[128], cur_key[128], cur_ver[32];
nightauth_get_identity(api, cur_name, sizeof(cur_name), cur_key, sizeof(cur_key), cur_ver, sizeof(cur_ver));
nightauth_set_identity(api, "MyApp", "new_api_key", "1.1");
```

### 20. Öffentliche Serverschlüssel & Standardkonstanten

Der SDK pinnt die Signier- und Sealed-Box-Öffentlichschlüssel des Servers. Standardwerte liegen als Compile-Time-Konstanten vor (`NIGHTAUTH_DEFAULT_API_URL`, `NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY` in `sdk/nightauth.h`); die Implementierung exportiert zusätzlich veränderbare Kopien (`NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY`), die Self-hosted-Betreiber pro Handle überschreiben können:

```c
// Einen Handle auf eine Self-hosted-NightAuth-Instanz mit eigenen Schlüsseln ausrichten
nightauth_set_server_keys(api,
    "a1b2c3...",   // Signier-Öffentlichschlüssel (64 Hex-Zeichen)
    "d4e5f6...");  // Sealed-Box-Öffentlichschlüssel (64 Hex-Zeichen)
```

### 21. Lizenz-Login mit 2FA (`nightauth_license_with_totp`)

Für Konten, deren Lizenzfluss TOTP erzwingt, wird der Authenticator-Code zusammen mit dem Lizenzschlüssel übergeben:

```c
if (nightauth_license_with_totp(api, license_key, "123456")) {
    printf("Lizenz mit 2FA verifiziert. Zugriff gewährt.\n");
}
```

### 22. Benutzernamen ändern

Erfordert das aktuelle Passwort. **Hinweis:** Aus Sicherheitsgründen beendet der Server die Sitzung nach erfolgreicher Umbenennung — melde dich mit dem neuen Benutzernamen erneut an.

```c
if (nightauth_change_username(api, "new_username", password)) {
    printf("Benutzername geändert zu %s. Bitte erneut anmelden.\n",
           nightauth_get_user(api)->username);
}
```

### 23. Datei-Hash verifizieren

Prüft eine SHA-256-Prüfsumme deiner verteilten Binärdatei gegen die serverseitige Registrierung autorisierter Releases:

```c
if (nightauth_verify_file_hash(api, "a1b2c3d4...")) {
    printf("Integrität verifiziert: Prüfsumme stimmt mit einem autorisierten Release überein.\n");
} else {
    fprintf(stderr, "Verifizierung fehlgeschlagen: %s\n", nightauth_get_response(api)->message);
}
```

### 24. Anwendungstelemetrie & Online-Benutzer

```c
// Aggregierte Anwendungsstatistiken
NightAuthAppStats stats;
if (nightauth_get_stats(api, &stats)) {
    printf("Benutzer: %lld (aktiv %lld, gebannt %lld)\n",
           (long long)stats.total_users, (long long)stats.active_users, (long long)stats.banned_users);
    printf("Lizenzen: %lld verwendet / %lld ungenutzt | Version: %s\n",
           (long long)stats.used_licenses, (long long)stats.unused_licenses, stats.version);
}

// Aktuell online befindliche Benutzer (gibt die Anzahl geschriebener Namen zurück)
char online[16][128];
int64_t total_online = 0;
int count = nightauth_get_online_users(api, &total_online, online, 16);
for (int i = 0; i < count; i++) {
    printf("Online: %s\n", online[i]);
}

// Statische Anwendungsmetadaten (Name, Version, Summen)
NightAuthAppData* app = nightauth_get_app(api);
printf("App: %s %s | Benutzer: %lld | Online: %lld\n",
       app->application, app->version, (long long)app->total_users, (long long)app->online_users);
```

### 25. Verwaltung der Hardware-ID (HWID)

```c
// Vom SDK berechnete Maschinenkennung auslesen
char hwid[128] = {0};
nightauth_get_hardware_id(hwid, sizeof(hwid));

// Aktuellen Session-Handle an eine explizite HWID binden
nightauth_set_hardware_id(api, hwid);

// Die von allen zukünftigen Handles gemeldete Kennung global überschreiben
// (nützlich für Tests oder eigene Fingerprint-Schemata)
nightauth_set_hardware_id_override("custom-machine-id");
```

### 26. Paginierter Chat-Verlauf (`nightauth_get_chat_messages_ex`)

Ruft Nachrichten ab, die älter als eine gegebene ID sind (`0` für die neueste Seite übergeben), und markiert sie optional serverseitig als gelesen:

```c
NightAuthChatMessage hist[25];
int count = nightauth_get_chat_messages_ex(api, "general", 0, true, hist, 25);
for (int i = 0; i < count; i++) {
    printf("[%s] %s: %s (#%s)\n", hist[i].timestamp, hist[i].author, hist[i].message, hist[i].id);
}
```

### 27. Erweiterte serverseitige Webhooks (`nightauth_webhook_ex`)

Erweiterter Webhook mit Abfrageparametern, optionalem Dateianhang und HTTP-Statusausgabe:

```c
int status = 0;
const char* json = "{\"content\":\"Client notification\"}";
const void* report_data = NULL;   // optionale Anhangs-Bytes
size_t report_len = 0;

if (nightauth_webhook_ex(api, "7kR0UedlVI", "Client notification",
                         "urgent=true", json,
                         report_data, report_len, "report.bin", &status)) {
    printf("Webhook ausgelöst: HTTP %d\n", status);
}
```

### 28. Erweitertes Ereignis-Streaming (`events_run_ex`, `events_cancel`)

`nightauth_events_run_ex` liefert jedes SSE-Ereignis zusammen mit seiner Server-Ereignis-ID (ermöglicht Wiederaufnahme über `last_event_id`). `nightauth_events_cancel` bricht einen laufenden Stream aus einem anderen Thread ab (z. B. bei Benutzereingabe):

```c
static bool handle_event_ex(const char* event_json, const char* event_id, void* user) {
    printf("[EVENT %s] %s\n", event_id ? event_id : "-", event_json);
    return true; // true zurückgeben, um den Stream am Laufen zu halten
}

// Ein anderer Thread kann nightauth_events_cancel(api) aufrufen, um die Schleife zu stoppen
if (nightauth_events_run_ex(api, ticket, NULL, handle_event_ex, NULL)) {
    printf("Ereignisstream sauber geschlossen.\n");
}
```

### 29. Signierte & verifizierte Selbst-Updates

Verifiziert ein signiertes Update-Envelope gegen einen gepinnten Ed25519-Öffentlichschlüssel, lädt es herunter oder übergibt die verifizierte URL an den servergestützten Anwendefluss:

```c
// 1. Ein vom Server empfangenes signiertes Update-Manifest verifizieren
NightAuthVerifiedUpdate update;
char err[256] = {0};
if (nightauth_signed_update_verify(envelope_json, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                   "MyApp", /*min_sequence=*/1, &update, err, sizeof(err))) {
    printf("Update v%s -> Seq %llu (%llu Bytes)\n", update.version,
           (unsigned long long)update.sequence, (unsigned long long)update.size);

    // 2. Verifiziertes Artefakt auf die Festplatte herunterladen
    if (nightauth_signed_update_download(&update, "update.pkg", err, sizeof(err))) {
        printf("Update heruntergeladen und Digest verifiziert.\n");
    }
}

// 3. Oder den offiziellen Installer direkt von einer Manifest-URL herunterladen
nightauth_signed_update_installer_download(manifest_url, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                           "MyApp", "installer.exe", err, sizeof(err));

// 4. Servergestütztes Anwenden: ausstehende Download-URL an den API-Handle übergeben
if (nightauth_apply_update(api, download_url)) {
    printf("Erforderliche Version: %s\n", nightauth_get_required_version(api));
}
const char* pending = nightauth_get_pending_download_url(api); // NULL, wenn keines aussteht
```

---

## 🖥️ Konsolen- und Desktop-Clients ausführen

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Lizenz

Dieses Projekt ist unter der **Elastic License 2.0** lizenziert.

* Die Software darf Dritten nicht als gehosteter Dienst zur Verfügung gestellt werden.
* Das Lizenzierungssystem darf weder verändert noch umgangen werden.
* Copyright- und Lizenzhinweise dürfen nicht entfernt werden.

Den vollständigen Text finden Sie in der [LICENSE](../LICENSE)-Datei.
