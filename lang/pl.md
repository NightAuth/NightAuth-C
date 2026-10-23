# NightAuth C SDK — Dodaj gwiazdkę na GitHub ⭐

Oficjalny pakiet **NightAuth** C11 Header-Only SDK i zestaw aplikacji referencyjnych (Nowoczesny interfejs graficzny **Win32 Pure GDI+** oraz interaktywny terminal CLI). Zabezpiecz swoje oprogramowanie w C przed inżynierią wsteczną i piractwem w kilka minut.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Poradniki wideo i instrukcje konfiguracji
> Oficjalne poradniki wideo i przewodniki krok po kroku opisujące konfigurację aplikacji, planów, subskrypcji, zmiennych chmurowych, webhooków i licencji w panelu NightAuth **zostaną wkrótce opublikowane**.

---

## 📸 Zrzuty ekranu

### Aplikacja Pulpitowa (Natywna Win32 GDI Nowoczesny Ciemny Motyw)

| Ekran logowania i autoryzacji | Panel zarządzania i subskrypcji |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Aplikacja konsolowa (Interaktywny terminal CLI)

| Logowanie w terminalu | Panel interaktywny w konsoli |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Zalecane Praktyki Bezpieczeństwa

* **Wirtualizacja Plików Binarnych:** Natywne pliki binarne C podlegają analizie statycznej za pomocą IDA Pro, Ghidra i Binary Ninja. W wersjach produkcyjnych zabezpieczaj skompilowane pliki wykonywalne za pomocą komercyjnych wirtualizatorów, takich jak **VMProtect**, **Themida** lub **CodeVirtualizer**.
* **Szyfrowanie Ciągów Znaków w Czasie Kompilacji:** Nie pozostawiaj poufnych ciągów znaków ani tokenów API jako zwykłego tekstu w sekcji `.rdata`. Używaj makr zaciemniania ciągów XOR, aby odszyfrowywać ciągi tylko w momencie ich użycia.
* **Bezpieczne Wykonywanie Plików w Pamięci:** Podczas pobierania chronionych ładunków lub bibliotek DLL przez `nightauth_download_file(api, ticket, ...)`, **nigdy nie zapisuj bajtów na dysku fizycznym**. Ładuj i wykonuj je bezpośrednio w pamięci RAM (np. reflective PE lub manual mapping) oraz natychmiast wyczyść bufor za pomocą `SecureZeroMemory()`.
* **Weryfikacja Sprzętu i Integralności:** Utrzymuj aktywne mechanizmy anti-debugging, sprawdzanie integralności pamięci i pomiary czasu w wersjach produkcyjnych.

---

## 📦 Wymagania i Kompilacja

Wymaga **C11** lub nowszego oraz **Visual Studio 2026** (zestaw narzędzi MSVC `v145`). Dołączone projekty są skonfigurowane dla `v145`: projekt konsolowy bezwarunkowo ustawia `<PlatformToolset>v145</PlatformToolset>`, natomiast projekt desktopowy domyślnie używa `v145` tylko wtedy, gdy nie określono zestawu narzędzi. Kompilacja z **Visual Studio 2022** (`v143`) działa tylko po ręcznej zmianie zestawu narzędzi w `console/NightAuth-Console.vcxproj` (lub po przekazaniu `/p:PlatformToolset=v143` dla klienta desktopowego).

Zależności w systemie Windows:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Natywna kryptografia i gniazda Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Renderowanie interfejsu graficznego)
* Dołączone biblioteki `libcurl` i `LibreSSL` (zgodne z OpenSSL, nagłówki w `include/openssl/`) (w `deps/curl_win64`)

> [!NOTE]
> Nagłówek SDK używa `<stdatomic.h>` (atomiki C11). Integrując `sdk/nightauth.h` z własnym projektem MSVC, dodaj flagę kompilatora `/experimental:c11atomics` (oba dołączone pliki `.vcxproj` już ją ustawiają). Bez niej MSVC nie skompiluje operacji atomowych używanych przez warstwę zaszyfrowanej sesji.

### Budowanie rozwiązania za pomocą MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

### Wdrożenie w czasie wykonania

Oba projekty klientów linkują z **biblioteką importową** libcurl (`libcurl.dll.a`), więc plik DLL `libcurl-x64.dll` (z `deps/curl_win64/curl-8.23.0_5-win64-mingw/bin`) **musi być dystrybuowany obok skompilowanych plików `.exe`**, w przeciwnym razie klienty się nie uruchomią. Projekt desktopowy dodatkowo linkuje `shell32.lib`.

---

## ⚙️ Konfiguracja i inicjalizacja

Skonfiguruj dane uwierzytelniające aplikacji z panelu NightAuth:

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

### Inicjalizacja uzgadniania kryptograficznego (`init`)

Wywołaj `init()` przed wszelkimi innymi operacjami. Tworzy wymianę kluczy Diffie-Hellmana (X25519) i szyfrowaną sesję:

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

## 🚀 Przewodnik po SDK

### 1. Sprawdzenie sesji i czarna lista

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

### 2. Logowanie loginem i hasłem (oraz 2FA)

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

### 3. Rejestracja z kluczem licencyjnym

```c
if (nightauth_register(api, username, password, license_key, email)) {
    printf("Registered successfully: %s\n", nightauth_get_user(api)->username);
} else {
    fprintf(stderr, "Registration failed: %s\n", nightauth_get_response(api)->message);
}
```

### 4. Dostęp tylko z kluczem (Key Only)

```c
if (nightauth_license(api, license_key)) {
    printf("License verified successfully. Access granted.\n");
} else {
    fprintf(stderr, "Access denied: %s\n", nightauth_get_response(api)->message);
}
```

### 5. Aktualizacja subskrypcji / Użycie klucza (Upgrade)

```c
if (nightauth_upgrade(api, username, new_license_key)) {
    printf("Subscription upgraded successfully.\n");
} else {
    fprintf(stderr, "Upgrade failed: %s\n", nightauth_get_response(api)->message);
}
```

### 6. Reset identyfikatora sprzętowego (HWID Lock)

```c
if (nightauth_reset_hwid(api, username, password, totp_code)) {
    printf("HWID reset successfully. You can now log in on this machine.\n");
} else {
    fprintf(stderr, "HWID reset failed: %s\n", nightauth_get_response(api)->message);
}
```

### 7. Przypomnienie hasła (Forgot Password & Reset)

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

### 8. Dane profilu użytkownika (`result.ToUserData()`)

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

### 9. Zmienne w chmurze (Globalne)

```c
char motd[256] = {0};
if (nightauth_get_global_var(api, "server_motd", motd, sizeof(motd))) {
    printf("Cloud Variable: %s\n", motd);
}
```

### 10. Zmienne użytkownika (User Variables)

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

### 11. Dziennik audytu aplikacji

```c
nightauth_log(api, "Suspicious memory modification attempt detected.");
```

### 12. Blokowanie użytkownika i urządzenia

```c
nightauth_ban(api, "Security tampering detected by Anti-Cheat.");
```

### 13. Webhooki po stronie serwera

```c
int status = 0;
const char* payload = "{\"content\":\"Client notification\"}";

if (nightauth_webhook(api, "7kR0UedlVI", payload, NULL, &status)) {
    printf("Webhook Response Status: HTTP %d\n", status);
}
```

### 14. Bezpieczne pobieranie plików w pamięci

Pobieranie plików wymaga **biletu dostępu z zakresem** uzyskanego przez `nightauth_access_ticket()` (serwer sprawdza bilet przed udostępnieniem pliku):

```c
// Poproś o bilet dostępu z zakresem dla docelowego pliku (zakres "file")
char ticket[128] = {0};
int64_t expires_in = 0;
if (!nightauth_access_ticket(api, "file", "385624", ticket, sizeof(ticket), &expires_in)) {
    fprintf(stderr, "Bilet dostępu odrzucony: %s\n", nightauth_get_response(api)->message);
    return 1;
}

char file_name[128] = {0};
uint8_t file_buf[65536];
size_t downloaded_len = 0;

if (nightauth_download_file(api, ticket, file_name, file_buf, sizeof(file_buf), &downloaded_len)) {
    printf("Pobrano %s (%zu bajtów) w bezpieczny sposób.\n", file_name, downloaded_len);
    // Przetwarzaj lub wykonuj ładunek bezpośrednio w pamięci (Reflective PE / Shellcode)
    // Bezpiecznie wyzeruj bufor pamięci po wykonaniu
    SecureZeroMemory(file_buf, downloaded_len);
}
```

### 15. Kanały czatu na żywo

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

### 16. Obsługa zdarzeń w czasie rzeczywistym (SSE)

```c
static bool handle_event(const char* event_json, void* user_data) {
    printf("[EVENT] %s\n", event_json);
    return true; // Return true to keep stream alive
}

nightauth_events_run(api, "ticket_value", NULL, handle_event, NULL);
```

### 17. Uwierzytelnianie dwuskładnikowe (2FA)

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

### 18. Cykl życia sesji i wylogowanie

```c
// Heartbeat ping to keep session alive
nightauth_heartbeat(api);

// Revoke and terminate session
nightauth_logout(api);

// Release memory allocations
nightauth_destroy(api);
```

### 19. Zaawansowana konfiguracja klienta

```c
// Limit czasu na żądanie w milisekundach (dotyczy wszystkich kolejnych wywołań sieciowych)
nightauth_set_request_timeout(api, 15000);

// Zezwól na nieszyfrowane HTTP dla endpointów loopback/self-hosted w trakcie rozwoju
NightAuthApi* api_dev = nightauth_create_ex(
    APP_NAME, APP_API_KEY, APP_VERSION, APP_TOKEN,
    "http://127.0.0.1:8787/api/v1",   // własny adres API (NULL = domyślny)
    true                               // allow_insecure_loopback
);

// Sprawdź lub rotuj tożsamość (nazwa, klucz api, wersja) aktywnego uchwytu
char cur_name[128], cur_key[128], cur_ver[32];
nightauth_get_identity(api, cur_name, sizeof(cur_name), cur_key, sizeof(cur_key), cur_ver, sizeof(cur_ver));
nightauth_set_identity(api, "MyApp", "new_api_key", "1.1");
```

### 20. Klucze publiczne serwera i stałe domyślne

SDK przypina publiczne klucze podpisu i sealed-box serwera. Wartości domyślne są dostarczane jako stałe kompilacji (`NIGHTAUTH_DEFAULT_API_URL`, `NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY` w `sdk/nightauth.h`), a implementacja eksportuje zmienne kopie (`NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY`), które operatorzy self-hosted mogą nadpisywać dla każdego uchwytu:

```c
// Skieruj uchwyt do instancji NightAuth self-hosted z własnymi kluczami
nightauth_set_server_keys(api,
    "a1b2c3...",   // publiczny klucz podpisu (64 znaki hex)
    "d4e5f6...");  // publiczny klucz sealed-box (64 znaki hex)
```

### 21. Logowanie licencją z 2FA (`nightauth_license_with_totp`)

Dla kont, których przepływ licencji wymusza TOTP, przekaż kod aplikacji uwierzytelniającej razem z kluczem licencyjnym:

```c
if (nightauth_license_with_totp(api, license_key, "123456")) {
    printf("Licencja zweryfikowana z 2FA. Dostęp przyznany.\n");
}
```

### 22. Zmiana nazwy użytkownika

Wymaga aktualnego hasła. **Uwaga:** ze względów bezpieczeństwa serwer zamyka sesję po udanej zmianie nazwy — zaloguj się ponownie z nową nazwą.

```c
if (nightauth_change_username(api, "new_username", password)) {
    printf("Nazwa użytkownika zmieniona na %s. Zaloguj się ponownie.\n",
           nightauth_get_user(api)->username);
}
```

### 23. Weryfikacja skrótu pliku

Sprawdza sumę kontrolną SHA-256 Twojego dystrybuowanego pliku binarnego względem serwerowego rejestru autoryzowanych wydań:

```c
if (nightauth_verify_file_hash(api, "a1b2c3d4...")) {
    printf("Integralność zweryfikowana: suma kontrolna zgadza się z autoryzowanym wydaniem.\n");
} else {
    fprintf(stderr, "Weryfikacja nie powiodła się: %s\n", nightauth_get_response(api)->message);
}
```

### 24. Telemetria aplikacji i użytkownicy online

```c
// Zbiorcze statystyki aplikacji
NightAuthAppStats stats;
if (nightauth_get_stats(api, &stats)) {
    printf("Użytkownicy: %lld (aktywni %lld, zbanowani %lld)\n",
           (long long)stats.total_users, (long long)stats.active_users, (long long)stats.banned_users);
    printf("Licencje: %lld użyte / %lld nieużyte | Wersja: %s\n",
           (long long)stats.used_licenses, (long long)stats.unused_licenses, stats.version);
}

// Aktualnie zalogowani użytkownicy (zwraca liczbę zapisanych nazw)
char online[16][128];
int64_t total_online = 0;
int count = nightauth_get_online_users(api, &total_online, online, 16);
for (int i = 0; i < count; i++) {
    printf("Online: %s\n", online[i]);
}

// Statyczne metadane aplikacji (nazwa, wersja, sumy)
NightAuthAppData* app = nightauth_get_app(api);
printf("App: %s %s | Użytkownicy: %lld | Online: %lld\n",
       app->application, app->version, (long long)app->total_users, (long long)app->online_users);
```

### 25. Zarządzanie Hardware ID (HWID)

```c
// Odczytaj obliczony przez SDK odcisk palca maszyny
char hwid[128] = {0};
nightauth_get_hardware_id(hwid, sizeof(hwid));

// Przypisz bieżący uchwyt sesji do jawnego HWID
nightauth_set_hardware_id(api, hwid);

// Globalnie nadpisz odcisk palca zgłaszany przez wszystkie przyszłe uchwyty
// (przydatne do testów lub własnych schematów identyfikacji)
nightauth_set_hardware_id_override("custom-machine-id");
```

### 26. Stronicowana historia czatu (`nightauth_get_chat_messages_ex`)

Pobiera wiadomości starsze niż podany identyfikator (przekaż `0` dla najnowszej strony) i opcjonalnie oznacza je jako przeczytane po stronie serwera:

```c
NightAuthChatMessage hist[25];
int count = nightauth_get_chat_messages_ex(api, "general", 0, true, hist, 25);
for (int i = 0; i < count; i++) {
    printf("[%s] %s: %s (#%s)\n", hist[i].timestamp, hist[i].author, hist[i].message, hist[i].id);
}
```

### 27. Zaawansowane webhooki po stronie serwera (`nightauth_webhook_ex`)

Rozszerzony webhook z parametrami zapytania, opcjonalnym załącznikiem i wyjściem statusu HTTP:

```c
int status = 0;
const char* json = "{\"content\":\"Client notification\"}";
const void* report_data = NULL;   // bajty opcjonalnego załącznika
size_t report_len = 0;

if (nightauth_webhook_ex(api, "7kR0UedlVI", "Client notification",
                         "urgent=true", json,
                         report_data, report_len, "report.bin", &status)) {
    printf("Webhook wysłany: HTTP %d\n", status);
}
```

### 28. Zaawansowane strumieniowanie zdarzeń (`events_run_ex`, `events_cancel`)

`nightauth_events_run_ex` dostarcza każde zdarzenie SSE wraz z jego serwerowym identyfikatorem (umożliwiając wznowienie przez `last_event_id`). `nightauth_events_cancel` przerywa działający strumień z innego wątku (np. na wejście użytkownika):

```c
static bool handle_event_ex(const char* event_json, const char* event_id, void* user) {
    printf("[EVENT %s] %s\n", event_id ? event_id : "-", event_json);
    return true; // Zwróć true, aby strumień pozostał aktywny
}

// Inny wątek może wywołać nightauth_events_cancel(api), aby zatrzymać pętlę
if (nightauth_events_run_ex(api, ticket, NULL, handle_event_ex, NULL)) {
    printf("Strumień zdarzeń zamknięty poprawnie.\n");
}
```

### 29. Podpisane i zweryfikowane samoaktualizacje

Weryfikuje podpisaną kopertę aktualizacji względem przypiętego klucza publicznego Ed25519, pobiera ją lub przekazuje zweryfikowany adres URL do wspomaganego przez serwer przepływu aplikowania:

```c
// 1. Zweryfikuj podpisany manifest aktualizacji otrzymany z serwera
NightAuthVerifiedUpdate update;
char err[256] = {0};
if (nightauth_signed_update_verify(envelope_json, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                   "MyApp", /*min_sequence=*/1, &update, err, sizeof(err))) {
    printf("Aktualizacja v%s -> seq %llu (%llu bajtów)\n", update.version,
           (unsigned long long)update.sequence, (unsigned long long)update.size);

    // 2. Pobierz zweryfikowany artefakt na dysk
    if (nightauth_signed_update_download(&update, "update.pkg", err, sizeof(err))) {
        printf("Aktualizacja pobrana i skrót zweryfikowany.\n");
    }
}

// 3. Lub pobierz oficjalny instalator bezpośrednio z adresu URL manifestu
nightauth_signed_update_installer_download(manifest_url, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                           "MyApp", "installer.exe", err, sizeof(err));

// 4. Aplikowanie wspomagane przez serwer: przekaż oczekujący adres pobierania do uchwytu API
if (nightauth_apply_update(api, download_url)) {
    printf("Wymagana wersja: %s\n", nightauth_get_required_version(api));
}
const char* pending = nightauth_get_pending_download_url(api); // NULL, gdy brak oczekujących
```

---

## 🖥️ Uruchamianie aplikacji konsoli i pulpitu

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Licencja

Projekt jest licencjonowany na warunkach **Elastic License 2.0**.

* Nie wolno udostępniać oprogramowania osobom trzecim jako usługi hostowanej lub zarządzanej.
* Nie wolno modyfikować, wyłączać ani omijać funkcji weryfikacji licencji.
* Nie wolno usuwać ani modyfikować informacji o prawach autorskich.

Pełny tekst znajduje się w pliku [LICENSE](../LICENSE).
