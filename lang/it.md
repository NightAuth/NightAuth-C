# NightAuth C SDK — Lascia una stella su GitHub ⭐

SDK Header-Only ufficiale di **NightAuth** per C11 e suite di client di riferimento (GUI desktop moderna nativa **Win32 Pure GDI+** e CLI da terminale interattiva). Proteggi il tuo software nativo in C contro reverse engineering e pirateria in pochi minuti.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Video Tutorial & Guide di Configurazione
> I video tutorial ufficiali e le guide dettagliate passo-passo per configurare applicazioni, piani, abbonamenti, variabili cloud, webhook e licenze nel dashboard di NightAuth **saranno pubblicati a breve** sul nostro portale e canale ufficiale.

---

## 📸 Schermate

### Applicazione Desktop (Nativa Win32 GDI Modern Dark Theme)

| Schermata di Accesso / Autenticazione | Dashboard & Gestione Abbonamento |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Applicazione Console (CLI Interattiva)

| Accesso da Terminale | Dashboard Console Interattiva |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Pratiche di Sicurezza Consigliate

* **Virtualizzazione Binaria Nativa:** I binari C nativi sono soggetti ad analisi statica tramite IDA Pro, Ghidra e Binary Ninja. Per le versioni di produzione, proteggi gli eseguibili compilati utilizzando virtualizzatori e protettori commerciali come **VMProtect**, **Themida** o **CodeVirtualizer**.
* **Crittografia delle Stringhe a Tempo di Compilazione:** Non lasciare stringhe sensibili o token API in testo normale nella sezione `.rdata`. Usa macro di offuscamento XOR per decrittografare le stringhe solo quando necessario.
* **Esecuzione Sicura dei File in Memoria:** Durante lo streaming di payload o DLL protetti tramite `nightauth_download_file(api, ticket, ...)`, **non scrivere mai i byte sul disco fisico**. Carica ed esegui i payload direttamente dalla memoria RAM (es. reflective PE loading o manual map injection) e ripulisci immediatamente il buffer con `SecureZeroMemory()`.
* **Verifica di Hardware e Integrità:** Mantieni attivi i controlli anti-debugging, di integrità della memoria e di tempistica nelle build di produzione per mitigare hooking a runtime e debugger.

---

## 📦 Requisiti e Compilazione

Richiede **C11** o successivo e **Visual Studio 2026** (MSVC toolset `v145`). I progetti inclusi sono configurati per `v145`: il progetto console fissa `<PlatformToolset>v145</PlatformToolset>` incondizionatamente, mentre il progetto desktop usa `v145` come predefinito solo quando non viene specificato alcun toolset. Compilare con **Visual Studio 2022** (`v143`) funziona solo cambiando manualmente il toolset in `console/NightAuth-Console.vcxproj` (o passando `/p:PlatformToolset=v143` per il client desktop).

Dipendenze Windows:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Crittografia e socket nativi di Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Rendering GUI desktop)
* Pacchetto integrato di `libcurl` e `LibreSSL` (compatibile con OpenSSL, header in `include/openssl/`) (in `deps/curl_win64`)

> [!NOTE]
> L'header del SDK usa `<stdatomic.h>` (atomiche C11). Quando integri `sdk/nightauth.h` nel tuo progetto MSVC, aggiungi il flag del compilatore `/experimental:c11atomics` (entrambi i file `.vcxproj` inclusi lo impostano già). Senza di esso, MSVC non riesce a compilare le operazioni atomiche usate dal livello di sessione cifrata.

### Compila la soluzione con MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

### Distribuzione a runtime

Entrambi i progetti client si collegano alla **libreria di importazione** di libcurl (`libcurl.dll.a`), quindi la DLL `libcurl-x64.dll` (da `deps/curl_win64/curl-8.23.0_5-win64-mingw/bin`) **deve essere distribuita accanto agli `.exe` compilati**, altrimenti i client non si avviano. Il progetto desktop collega inoltre `shell32.lib`.

---

## ⚙️ Configurazione & Inizializzazione

Configura le credenziali della tua applicazione dalla dashboard di NightAuth:

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

### Handshake Crittografico (`init`)

Chiama `init()` prima di eseguire qualsiasi altra funzione. Stabilisce lo scambio di chiavi Diffie-Hellman (X25519) e la sessione cifrata:

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

## 🚀 Guida all'Uso dell'SDK

### 1. Controllo Sessione & Blacklist

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

### 2. Login con Nome Utente & Password (e 2FA)

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

### 3. Registrazione con Chiave di Licenza

```c
if (nightauth_register(api, username, password, license_key, email)) {
    printf("Registered successfully: %s\n", nightauth_get_user(api)->username);
} else {
    fprintf(stderr, "Registration failed: %s\n", nightauth_get_response(api)->message);
}
```

### 4. Autenticazione Solo Licenza (Key Only)

```c
if (nightauth_license(api, license_key)) {
    printf("License verified successfully. Access granted.\n");
} else {
    fprintf(stderr, "Access denied: %s\n", nightauth_get_response(api)->message);
}
```

### 5. Riscatta Licenza / Aggiorna Abbonamento (Upgrade)

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

### 7. Recupero Password (Forgot Password & Reset)

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

### 8. Dati del Profilo Utente (`result.ToUserData()`)

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

### 9. Variabili Cloud (Globali)

```c
char motd[256] = {0};
if (nightauth_get_global_var(api, "server_motd", motd, sizeof(motd))) {
    printf("Cloud Variable: %s\n", motd);
}
```

### 10. Variabili Utente (User Variables)

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

### 11. Registri di Audit e Telemetria

```c
nightauth_log(api, "Suspicious memory modification attempt detected.");
```

### 12. Banna Utente e Blocca Macchina

```c
nightauth_ban(api, "Security tampering detected by Anti-Cheat.");
```

### 13. Webhook Lato Server

```c
int status = 0;
const char* payload = "{\"content\":\"Client notification\"}";

if (nightauth_webhook(api, "7kR0UedlVI", payload, NULL, &status)) {
    printf("Webhook Response Status: HTTP %d\n", status);
}
```

### 14. Download Sicuro in Memoria

Il download dei file richiede un **ticket di accesso con ambito** ottenuto con `nightauth_access_ticket()` (il server valida il ticket prima di servire il file):

```c
// Richiedere un ticket di accesso con ambito per il file di destinazione (ambito "file")
char ticket[128] = {0};
int64_t expires_in = 0;
if (!nightauth_access_ticket(api, "file", "385624", ticket, sizeof(ticket), &expires_in)) {
    fprintf(stderr, "Ticket di accesso negato: %s\n", nightauth_get_response(api)->message);
    return 1;
}

char file_name[128] = {0};
uint8_t file_buf[65536];
size_t downloaded_len = 0;

if (nightauth_download_file(api, ticket, file_name, file_buf, sizeof(file_buf), &downloaded_len)) {
    printf("Scaricato %s (%zu byte) in modo sicuro.\n", file_name, downloaded_len);
    // Elabora o esegui il payload direttamente in memoria (Reflective PE / Shellcode)
    // Cancella il buffer in memoria in modo sicuro dopo l'esecuzione
    SecureZeroMemory(file_buf, downloaded_len);
}
```

### 15. Canali Chat in Tempo Reale

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

### 16. Gestori di Eventi in Tempo Reale (SSE)

```c
static bool handle_event(const char* event_json, void* user_data) {
    printf("[EVENT] %s\n", event_json);
    return true; // Return true to keep stream alive
}

nightauth_events_run(api, "ticket_value", NULL, handle_event, NULL);
```

### 17. Autenticazione a Due Fattori (2FA)

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

### 18. Ciclo di Vita della Sessione e Logout

```c
// Heartbeat ping to keep session alive
nightauth_heartbeat(api);

// Revoke and terminate session
nightauth_logout(api);

// Release memory allocations
nightauth_destroy(api);
```

### 19. Configurazione avanzata del client

```c
// Timeout per richiesta in millisecondi (influisce su tutte le chiamate di rete successive)
nightauth_set_request_timeout(api, 15000);

// Consentire HTTP non cifrato per endpoint di sviluppo loopback/self-hosted
NightAuthApi* api_dev = nightauth_create_ex(
    APP_NAME, APP_API_KEY, APP_VERSION, APP_TOKEN,
    "http://127.0.0.1:8787/api/v1",   // URL API personalizzato (NULL = predefinito)
    true                               // allow_insecure_loopback
);

// Ispezionare o ruotare l'identità (nome, api key, versione) di un handle attivo
char cur_name[128], cur_key[128], cur_ver[32];
nightauth_get_identity(api, cur_name, sizeof(cur_name), cur_key, sizeof(cur_key), cur_ver, sizeof(cur_ver));
nightauth_set_identity(api, "MyApp", "new_api_key", "1.1");
```

### 20. Chiavi pubbliche del server & costanti predefinite

L'SDK fissa (pinning) le chiavi pubbliche di firma e sealed-box del server. I valori predefiniti sono distribuiti come costanti di compilazione (`NIGHTAUTH_DEFAULT_API_URL`, `NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY` in `sdk/nightauth.h`), e l'implementazione esporta copie modificabili (`NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY`) che gli operatori self-hosted possono sovrascrivere per handle:

```c
// Puntare un handle a un'istanza NightAuth self-hosted con le proprie chiavi
nightauth_set_server_keys(api,
    "a1b2c3...",   // chiave pubblica di firma (64 caratteri esadecimali)
    "d4e5f6...");  // chiave pubblica sealed-box (64 caratteri esadecimali)
```

### 21. Accesso con licenza e 2FA (`nightauth_license_with_totp`)

Per gli account il cui flusso di licenza impone TOTP, passa il codice dell'app di autenticazione insieme alla chiave di licenza:

```c
if (nightauth_license_with_totp(api, license_key, "123456")) {
    printf("Licenza verificata con 2FA. Accesso consentito.\n");
}
```

### 22. Cambiare il nome utente

Richiede la password attuale. **Nota:** per sicurezza, il server chiude la sessione dopo una ridenominazione riuscita — effettua di nuovo il login con il nuovo nome.

```c
if (nightauth_change_username(api, "new_username", password)) {
    printf("Nome utente cambiato in %s. Effettua di nuovo il login.\n",
           nightauth_get_user(api)->username);
}
```

### 23. Verificare l'hash di un file

Convalida un checksum SHA-256 del tuo binario distribuito rispetto al registro lato server delle versioni autorizzate:

```c
if (nightauth_verify_file_hash(api, "a1b2c3d4...")) {
    printf("Integrità verificata: il checksum corrisponde a una versione autorizzata.\n");
} else {
    fprintf(stderr, "Verifica fallita: %s\n", nightauth_get_response(api)->message);
}
```

### 24. Telemetria dell'applicazione & utenti online

```c
// Statistiche aggregate dell'applicazione
NightAuthAppStats stats;
if (nightauth_get_stats(api, &stats)) {
    printf("Utenti: %lld (attivi %lld, bannati %lld)\n",
           (long long)stats.total_users, (long long)stats.active_users, (long long)stats.banned_users);
    printf("Licenze: %lld usate / %lld inutilizzate | Versione: %s\n",
           (long long)stats.used_licenses, (long long)stats.unused_licenses, stats.version);
}

// Utenti attualmente online (restituisce il numero di nomi scritti)
char online[16][128];
int64_t total_online = 0;
int count = nightauth_get_online_users(api, &total_online, online, 16);
for (int i = 0; i < count; i++) {
    printf("Online: %s\n", online[i]);
}

// Metadati statici dell'applicazione (nome, versione, totali)
NightAuthAppData* app = nightauth_get_app(api);
printf("App: %s %s | Utenti: %lld | Online: %lld\n",
       app->application, app->version, (long long)app->total_users, (long long)app->online_users);
```

### 25. Gestione dell'Hardware ID (HWID)

```c
// Leggere l'impronta della macchina calcolata dall'SDK
char hwid[128] = {0};
nightauth_get_hardware_id(hwid, sizeof(hwid));

// Legare l'handle di sessione corrente a un HWID esplicito
nightauth_set_hardware_id(api, hwid);

// Sovrascrivere globalmente l'impronta riportata da tutti gli handle futuri
// (utile per test o schemi di fingerprint personalizzati)
nightauth_set_hardware_id_override("custom-machine-id");
```

### 26. Cronologia chat paginata (`nightauth_get_chat_messages_ex`)

Recupera i messaggi più vecchi di un dato ID (passa `0` per la pagina più recente) e li marca opzionalmente come letti lato server:

```c
NightAuthChatMessage hist[25];
int count = nightauth_get_chat_messages_ex(api, "general", 0, true, hist, 25);
for (int i = 0; i < count; i++) {
    printf("[%s] %s: %s (#%s)\n", hist[i].timestamp, hist[i].author, hist[i].message, hist[i].id);
}
```

### 27. Webhook server avanzati (`nightauth_webhook_ex`)

Webhook esteso con parametri di query, allegato opzionale e uscita dello stato HTTP:

```c
int status = 0;
const char* json = "{\"content\":\"Client notification\"}";
const void* report_data = NULL;   // byte dell'allegato opzionale
size_t report_len = 0;

if (nightauth_webhook_ex(api, "7kR0UedlVI", "Client notification",
                         "urgent=true", json,
                         report_data, report_len, "report.bin", &status)) {
    printf("Webhook inviato: HTTP %d\n", status);
}
```

### 28. Streaming di eventi avanzato (`events_run_ex`, `events_cancel`)

`nightauth_events_run_ex` consegna ogni evento SSE insieme al suo ID evento del server (consentendo la ripresa tramite `last_event_id`). `nightauth_events_cancel` interrompe uno stream in corso da un altro thread (ad esempio su input dell'utente):

```c
static bool handle_event_ex(const char* event_json, const char* event_id, void* user) {
    printf("[EVENT %s] %s\n", event_id ? event_id : "-", event_json);
    return true; // Restituire true per mantenere attivo lo stream
}

// Un altro thread può chiamare nightauth_events_cancel(api) per fermare il ciclo
if (nightauth_events_run_ex(api, ticket, NULL, handle_event_ex, NULL)) {
    printf("Stream di eventi chiuso correttamente.\n");
}
```

### 29. Auto-aggiornamenti firmati e verificati

Verifica una busta di aggiornamento firmata rispetto a una chiave pubblica Ed25519 fissata, la scarica, oppure passa l'URL verificato al flusso di applicazione assistito dal server:

```c
// 1. Verificare un manifest di aggiornamento firmato ricevuto dal server
NightAuthVerifiedUpdate update;
char err[256] = {0};
if (nightauth_signed_update_verify(envelope_json, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                   "MyApp", /*min_sequence=*/1, &update, err, sizeof(err))) {
    printf("Aggiornamento v%s -> seq %llu (%llu byte)\n", update.version,
           (unsigned long long)update.sequence, (unsigned long long)update.size);

    // 2. Scaricare l'artefatto verificato su disco
    if (nightauth_signed_update_download(&update, "update.pkg", err, sizeof(err))) {
        printf("Aggiornamento scaricato e digest verificato.\n");
    }
}

// 3. Oppure scaricare l'installer ufficiale direttamente da un URL del manifest
nightauth_signed_update_installer_download(manifest_url, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                           "MyApp", "installer.exe", err, sizeof(err));

// 4. Applicazione assistita dal server: passare l'URL di download in sospeso all'handle API
if (nightauth_apply_update(api, download_url)) {
    printf("Versione richiesta: %s\n", nightauth_get_required_version(api));
}
const char* pending = nightauth_get_pending_download_url(api); // NULL quando non ce n'è nessuno
```

---

## 🖥️ Esecuzione dei client Console e Desktop

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Licenza

Questo progetto è rilasciato sotto licenza **Elastic License 2.0**.

* Non è consentito fornire il software a terzi come servizio gestito o in hosting.
* Non è consentito alterare, disattivare o aggirare le funzioni di licenza.
* Non è consentito rimuovere o modificare gli avvisi di copyright.

Consulta il file [LICENSE](../LICENSE) per il testo completo.
