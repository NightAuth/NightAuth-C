# NightAuth C SDK — Danos una estrella en GitHub ⭐

SDK Header-Only oficial de **NightAuth** para C11 y suite de clientes de referencia (interfaz gráfica moderna nativa de **Win32 GDI** y CLI interactivo para terminal). Protege tu software en C contra ingeniería inversa, piratería y manipulaciones en minutos.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Tutoriales en Video y Configuración del Sistema
> Los video-tutoriales oficiales y las guías paso a paso de cómo configurar aplicaciones, planes, suscripciones, variables en la nube, webhooks y licencias en el panel de control de NightAuth **serán publicados próximamente** en nuestro portal y canal oficial.

---

## 📸 Capturas de Pantalla

### Aplicación de Escritorio (Nativa Win32 GDI Tema Oscuro Moderno)

| Pantalla de Inicio / Autenticación | Panel de Control & Gestión de Suscripciones |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Aplicación de Consola (CLI Interactiva)

| Inicio de Sesión en Terminal | Panel Interactivo de Consola |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Prácticas de Seguridad Recomendadas

* **Virtualización de Binarios Nativos:** Los binarios nativos en C están sujetos a análisis estático mediante IDA Pro, Ghidra y Binary Ninja. Para versiones de producción, protege los ejecutables compilados utilizando virtualizadores y protectores comerciales como **VMProtect**, **Themida** o **CodeVirtualizer**.
* **Cifrado de Cadenas en Tiempo de Compilación:** No dejes cadenas sensibles o tokens de API en texto plano en la sección `.rdata`. Utiliza macros de ofuscación de cadenas XOR para descifrar cadenas solo en el instante en que se necesitan.
* **Ejecución Segura de Archivos en Memoria:** Al transmitir payloads protegidos o DLLs mediante `nightauth_download_file(api, ticket, ...)`, **nunca escribas los bytes en el almacenamiento físico o disco**. Carga y ejecuta las cargas útiles directamente desde la memoria (p. ej., carga reflectiva de PE o inyección manual map) y limpia inmediatamente el búfer con `SecureZeroMemory()`.
* **Verificación de Hardware e Integridad:** Mantén activas las comprobaciones anti-debugging, integridad de memoria y temporización en versiones de producción para mitigar el enganche en tiempo de ejecución y la conexión de depuradores.

---

## 📦 Requisitos y Compilación

Requiere **C11** o posterior y **Visual Studio 2026** (conjunto de herramientas MSVC `v145`). Los proyectos incluidos están configurados para `v145`: el proyecto de consola fija `<PlatformToolset>v145</PlatformToolset>` de forma incondicional, mientras que el proyecto de escritorio usa `v145` por defecto solo cuando no se especifica ningún toolset. Compilar con **Visual Studio 2022** (`v143`) solo funciona si cambias manualmente el toolset en `console/NightAuth-Console.vcxproj` (o pasas `/p:PlatformToolset=v143` para el cliente de escritorio).

Dependencias en Windows:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Criptografía y sockets nativos de Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Renderizado de GUI nativa)
* Distribución incluida de `libcurl` y `LibreSSL` (compatible con OpenSSL, cabeceras en `include/openssl/`) (en `deps/curl_win64`)

> [!NOTE]
> La cabecera del SDK utiliza `<stdatomic.h>` (atómicos de C11). Al integrar `sdk/nightauth.h` en tu propio proyecto de MSVC, añade el flag del compilador `/experimental:c11atomics` (ambos `.vcxproj` incluidos ya lo configuran). Sin él, MSVC no puede compilar las operaciones atómicas usadas por la capa de sesión cifrada.

### Compilar la Solución con MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

### Despliegue en Tiempo de Ejecución

Ambos proyectos de cliente enlazan contra la **librería de importación** de libcurl (`libcurl.dll.a`), por lo que el DLL `libcurl-x64.dll` (de `deps/curl_win64/curl-8.23.0_5-win64-mingw/bin`) **debe distribuirse junto a los `.exe` compilados**; de lo contrario, los clientes no arrancan. El proyecto de escritorio además enlaza `shell32.lib`.

---

## ⚙️ Configuración e Inicialización

Configura las credenciales de tu aplicación obtenidas en el panel de NightAuth:

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

### Inicializar Handshake Criptográfico (`init`)

Debes invocar `init()` antes de cualquier otra llamada. Establece el intercambio de claves efímeras Diffie-Hellman (X25519) y la sesión cifrada:

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

## 🚀 Guía de Uso del SDK

### 1. Comprobar Validación de Sesión y Blacklist

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

### 2. Inicio de Sesión (Login con Usuario, Contraseña y 2FA)

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

### 3. Registro con Clave de Licencia

```c
if (nightauth_register(api, username, password, license_key, email)) {
    printf("Registered successfully: %s\n", nightauth_get_user(api)->username);
} else {
    fprintf(stderr, "Registration failed: %s\n", nightauth_get_response(api)->message);
}
```

### 4. Acceso con Solo Clave de Licencia (Key Only)

```c
if (nightauth_license(api, license_key)) {
    printf("License verified successfully. Access granted.\n");
} else {
    fprintf(stderr, "Access denied: %s\n", nightauth_get_response(api)->message);
}
```

### 5. Actualizar Suscripción / Canjear Clave (Upgrade)

```c
if (nightauth_upgrade(api, username, new_license_key)) {
    printf("Subscription upgraded successfully.\n");
} else {
    fprintf(stderr, "Upgrade failed: %s\n", nightauth_get_response(api)->message);
}
```

### 6. Restablecer HWID (Hardware ID Lock)

```c
if (nightauth_reset_hwid(api, username, password, totp_code)) {
    printf("HWID reset successfully. You can now log in on this machine.\n");
} else {
    fprintf(stderr, "HWID reset failed: %s\n", nightauth_get_response(api)->message);
}
```

### 7. Recuperación de Contraseña (Forgot Password & Reset)

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

### 8. Datos del Usuario Autenticado (`result.ToUserData()`)

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

### 9. Variables en la Nube (Globales)

```c
char motd[256] = {0};
if (nightauth_get_global_var(api, "server_motd", motd, sizeof(motd))) {
    printf("Cloud Variable: %s\n", motd);
}
```

### 10. Variables por Usuario (User Variables)

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

### 11. Logs de Aplicación y Auditoría

```c
nightauth_log(api, "Suspicious memory modification attempt detected.");
```

### 12. Baneo de Usuarios y Lista Negra

```c
nightauth_ban(api, "Security tampering detected by Anti-Cheat.");
```

### 13. Webhooks Seguros del Servidor

```c
int status = 0;
const char* payload = "{\"content\":\"Client notification\"}";

if (nightauth_webhook(api, "7kR0UedlVI", payload, NULL, &status)) {
    printf("Webhook Response Status: HTTP %d\n", status);
}
```

### 14. Descarga Segura de Archivos en Memoria

La descarga de archivos requiere un **ticket de acceso con ámbito** obtenido con `nightauth_access_ticket()` (el servidor valida el ticket antes de servir el archivo):

```c
// Solicitar un ticket de acceso con ámbito para el archivo destino (ámbito "file")
char ticket[128] = {0};
int64_t expires_in = 0;
if (!nightauth_access_ticket(api, "file", "385624", ticket, sizeof(ticket), &expires_in)) {
    fprintf(stderr, "Ticket de acceso denegado: %s\n", nightauth_get_response(api)->message);
    return 1;
}

char file_name[128] = {0};
uint8_t file_buf[65536];
size_t downloaded_len = 0;

if (nightauth_download_file(api, ticket, file_name, file_buf, sizeof(file_buf), &downloaded_len)) {
    printf("Descargado %s (%zu bytes) de forma segura.\n", file_name, downloaded_len);
    // Procesa o ejecuta el payload directamente en memoria (Reflective PE / Shellcode)
    // Borra el búfer de memoria de forma segura tras la ejecución
    SecureZeroMemory(file_buf, downloaded_len);
}
```

### 15. Canales de Chat en Vivo

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

### 16. Controladores de Eventos en Tiempo Real (SSE)

```c
static bool handle_event(const char* event_json, void* user_data) {
    printf("[EVENT] %s\n", event_json);
    return true; // Return true to keep stream alive
}

nightauth_events_run(api, "ticket_value", NULL, handle_event, NULL);
```

### 17. Autenticación de Dos Factores (2FA)

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

### 18. Ciclo de Vida de Sesión y Logout

```c
// Heartbeat ping to keep session alive
nightauth_heartbeat(api);

// Revoke and terminate session
nightauth_logout(api);

// Release memory allocations
nightauth_destroy(api);
```

### 19. Configuración Avanzada del Cliente

```c
// Tiempo de espera por petición en milisegundos (afecta a todas las llamadas de red posteriores)
nightauth_set_request_timeout(api, 15000);

// Permitir HTTP sin cifrar para endpoints de desarrollo locales / self-hosted
NightAuthApi* api_dev = nightauth_create_ex(
    APP_NAME, APP_API_KEY, APP_VERSION, APP_TOKEN,
    "http://127.0.0.1:8787/api/v1",   // URL de API personalizada (NULL = predeterminada)
    true                               // allow_insecure_loopback
);

// Inspeccionar o rotar la identidad (nombre, api key, versión) de un handle activo
char cur_name[128], cur_key[128], cur_ver[32];
nightauth_get_identity(api, cur_name, sizeof(cur_name), cur_key, sizeof(cur_key), cur_ver, sizeof(cur_ver));
nightauth_set_identity(api, "MyApp", "new_api_key", "1.1");
```

### 20. Claves Públicas del Servidor y Constantes Predeterminadas

El SDK fija las claves públicas de firma y sealed-box del servidor. Los valores predeterminados se distribuyen como constantes de compilación (`NIGHTAUTH_DEFAULT_API_URL`, `NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY` en `sdk/nightauth.h`), y la implementación exporta copias mutables (`NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY`) que los operadores self-hosted pueden sobrescribir por handle:

```c
// Dirigir un handle a una instancia NightAuth self-hosted con sus propias claves
nightauth_set_server_keys(api,
    "a1b2c3...",   // clave pública de firma (64 caracteres hex)
    "d4e5f6...");  // clave pública de sealed-box (64 caracteres hex)
```

### 21. Acceso con Licencia y 2FA (`nightauth_license_with_totp`)

Para cuentas cuyo flujo de licencia exige TOTP, pasa el código del autenticador junto con la clave de licencia:

```c
if (nightauth_license_with_totp(api, license_key, "123456")) {
    printf("Licencia verificada con 2FA. Acceso concedido.\n");
}
```

### 22. Cambiar el Nombre de Usuario

Requiere la contraseña actual. **Nota:** por seguridad, el servidor cierra la sesión tras un cambio exitoso — vuelve a iniciar sesión con el nuevo nombre.

```c
if (nightauth_change_username(api, "new_username", password)) {
    printf("Nombre de usuario cambiado a %s. Inicia sesión de nuevo.\n",
           nightauth_get_user(api)->username);
}
```

### 23. Verificar el Hash de un Archivo

Valida una suma de verificación SHA-256 de tu binario distribuido contra el registro del servidor de versiones autorizadas:

```c
if (nightauth_verify_file_hash(api, "a1b2c3d4...")) {
    printf("Integridad verificada: el hash coincide con una versión autorizada.\n");
} else {
    fprintf(stderr, "Verificación fallida: %s\n", nightauth_get_response(api)->message);
}
```

### 24. Telemetría de la Aplicación y Usuarios en Línea

```c
// Estadísticas agregadas de la aplicación
NightAuthAppStats stats;
if (nightauth_get_stats(api, &stats)) {
    printf("Usuarios: %lld (activos %lld, baneados %lld)\n",
           (long long)stats.total_users, (long long)stats.active_users, (long long)stats.banned_users);
    printf("Licencias: %lld usadas / %lld sin usar | Versión: %s\n",
           (long long)stats.used_licenses, (long long)stats.unused_licenses, stats.version);
}

// Usuarios actualmente en línea (devuelve el número de nombres escritos)
char online[16][128];
int64_t total_online = 0;
int count = nightauth_get_online_users(api, &total_online, online, 16);
for (int i = 0; i < count; i++) {
    printf("En línea: %s\n", online[i]);
}

// Metadatos estáticos de la aplicación (nombre, versión, totales)
NightAuthAppData* app = nightauth_get_app(api);
printf("App: %s %s | Usuarios: %lld | En línea: %lld\n",
       app->application, app->version, (long long)app->total_users, (long long)app->online_users);
```

### 25. Gestión del Hardware ID (HWID)

```c
// Leer la huella de la máquina calculada por el SDK
char hwid[128] = {0};
nightauth_get_hardware_id(hwid, sizeof(hwid));

// Vincular el handle de sesión actual a un HWID explícito
nightauth_set_hardware_id(api, hwid);

// Sobrescribir globalmente la huella que reportan todos los handles futuros
// (útil para pruebas o esquemas de huella personalizados)
nightauth_set_hardware_id_override("custom-machine-id");
```

### 26. Historial de Chat Paginado (`nightauth_get_chat_messages_ex`)

Obtiene mensajes anteriores a un ID dado (pasa `0` para la página más reciente) y opcionalmente los marca como leídos en el servidor:

```c
NightAuthChatMessage hist[25];
int count = nightauth_get_chat_messages_ex(api, "general", 0, true, hist, 25);
for (int i = 0; i < count; i++) {
    printf("[%s] %s: %s (#%s)\n", hist[i].timestamp, hist[i].author, hist[i].message, hist[i].id);
}
```

### 27. Webhooks Avanzados del Lado del Servidor (`nightauth_webhook_ex`)

Webhook extendido con parámetros de consulta, archivo adjunto opcional y salida de estado HTTP:

```c
int status = 0;
const char* json = "{\"content\":\"Client notification\"}";
const void* report_data = NULL;   // bytes del adjunto opcional
size_t report_len = 0;

if (nightauth_webhook_ex(api, "7kR0UedlVI", "Client notification",
                         "urgent=true", json,
                         report_data, report_len, "report.bin", &status)) {
    printf("Webhook enviado: HTTP %d\n", status);
}
```

### 28. Streaming de Eventos Avanzado (`events_run_ex`, `events_cancel`)

`nightauth_events_run_ex` entrega cada evento SSE junto con su ID de evento del servidor (permitiendo reanudar mediante `last_event_id`). `nightauth_events_cancel` interrumpe un stream en curso desde otro hilo (por ejemplo, ante una entrada del usuario):

```c
static bool handle_event_ex(const char* event_json, const char* event_id, void* user) {
    printf("[EVENT %s] %s\n", event_id ? event_id : "-", event_json);
    return true; // Devuelve true para mantener el stream activo
}

// Otro hilo puede llamar a nightauth_events_cancel(api) para detener el bucle
if (nightauth_events_run_ex(api, ticket, NULL, handle_event_ex, NULL)) {
    printf("Stream de eventos cerrado correctamente.\n");
}
```

### 29. Auto-actualizaciones Firmadas y Verificadas

Verifica un sobre de actualización firmada contra una clave pública Ed25519 fijada, descárgala o entrega la URL verificada al flujo de aplicación asistido por el servidor:

```c
// 1. Verificar un manifiesto de actualización firmado recibido del servidor
NightAuthVerifiedUpdate update;
char err[256] = {0};
if (nightauth_signed_update_verify(envelope_json, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                   "MyApp", /*min_sequence=*/1, &update, err, sizeof(err))) {
    printf("Actualización v%s -> seq %llu (%llu bytes)\n", update.version,
           (unsigned long long)update.sequence, (unsigned long long)update.size);

    // 2. Descargar el artefacto verificado a disco
    if (nightauth_signed_update_download(&update, "update.pkg", err, sizeof(err))) {
        printf("Actualización descargada y digest verificado.\n");
    }
}

// 3. O descargar el instalador oficial directamente desde una URL de manifiesto
nightauth_signed_update_installer_download(manifest_url, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                           "MyApp", "installer.exe", err, sizeof(err));

// 4. Aplicación asistida por el servidor: entregar la URL de descarga pendiente al handle de la API
if (nightauth_apply_update(api, download_url)) {
    printf("Versión requerida: %s\n", nightauth_get_required_version(api));
}
const char* pending = nightauth_get_pending_download_url(api); // NULL cuando no hay ninguna
```

---

## 🖥️ Ejecutar los Clientes de Consola y Escritorio

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Licencia

Este proyecto está bajo la licencia **Elastic License 2.0**.

* No está permitido ofrecer el software a terceros como servicio gestionado o alojado.
* No está permitido modificar, deshabilitar o eludir el sistema de licencias.
* No está permitido alterar ni eliminar avisos de copyright o de licencia.

Consulta el archivo [LICENSE](../LICENSE) para el texto legal completo.
