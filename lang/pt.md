# NightAuth C SDK — Adicione uma estrela no GitHub ⭐

SDK Header-Only oficial do **NightAuth** para C11 e suíte de clientes de referência (Interface gráfica moderna nativa **Win32 Pure GDI+** e terminal interativo CLI). Proteja seu software nativo em C contra engenharia reversa e pirataria em minutos.

Official Portal: [https://nightauth.org](https://nightauth.org)
---

**Languages / Idiomas / Langues / Sprachen:**  
[English](../README.md) • [Español](es.md) • [Français](fr.md) • [Deutsch](de.md) • [Italiano](it.md) • [Português](pt.md) • [Русский](ru.md) • [Türkçe](tr.md) • [Polski](pl.md) • [中文](zh.md)

---

> [!NOTE]
> ### 📺 Tutoriais em Vídeo & Guias de Configuração
> Tutoriais oficiais em vídeo e guias passo a passo explicando como configurar aplicativos, planos, assinaturas, variáveis em nuvem, webhooks e licenças no painel do NightAuth **serão publicados em breve**.

---

## 📸 Capturas de Tela

### Aplicação Desktop (Nativa Win32 GDI Tema Escuro Moderno)

| Tela de Início / Autenticação | Painel de Controle & Assinaturas |
| :---: | :---: |
| ![NightAuth Desktop Login](../image/desktop.png) | ![NightAuth Desktop Dashboard](../image/desktop-dashboard.png) |

### Aplicativo de Console (CLI Interativa)

| Login pelo Terminal | Painel Interativo de Console |
| :---: | :---: |
| ![NightAuth Console Login](../image/console.png) | ![NightAuth Console Dashboard](../image/console-dashboard.png) |

---

## 🛡️ Práticas de Segurança Recomendadas

* **Virtualização de Binários Nativos:** Binários nativos em C estão sujeitos à análise estática via IDA Pro, Ghidra e Binary Ninja. Para versões de produção, proteja os executáveis compilados utilizando virtualizadores e protetores comerciais como **VMProtect**, **Themida** ou **CodeVirtualizer**.
* **Criptografia de Strings em Tempo de Compilação:** Não deixe strings sensíveis ou tokens de API em texto puro na seção `.rdata`. Utilize macros de ofuscação XOR para descriptografar strings apenas no instante em que forem necessárias.
* **Execução Segura de Arquivos em Memória:** Ao transmitir payloads protegidos ou DLLs via `nightauth_download_file(api, ticket, ...)`, **nunca grave bytes no armazenamento físico ou disco**. Carregue e execute os executáveis diretamente da memória RAM (por exemplo, carga refletiva de PE ou injeção manual map) e limpe imediatamente o buffer com `SecureZeroMemory()`.
* **Verificação de Hardware e Integridade:** Mantenha ativas as verificações anti-depuração, integridade de memória e temporização em versões de produção.

---

## 📦 Requisitos e Compilação

Requer **C11** ou posterior e **Visual Studio 2026** (conjunto de ferramentas MSVC `v145`). Os projetos incluídos estão configurados para `v145`: o projeto de consola fixa `<PlatformToolset>v145</PlatformToolset>` incondicionalmente, enquanto o projeto de desktop usa `v145` por padrão apenas quando nenhum toolset é especificado. Compilar com **Visual Studio 2022** (`v143`) só funciona se você alterar manualmente o toolset em `console/NightAuth-Console.vcxproj` (ou passar `/p:PlatformToolset=v143` para o cliente desktop).

Dependências do Windows:
* `ws2_32.lib`, `crypt32.lib`, `bcrypt.lib`, `advapi32.lib` (Criptografia e soquetes nativos do Windows)
* `gdi32.lib`, `msimg32.lib`, `comctl32.lib`, `user32.lib`, `shell32.lib` (Renderização de GUI nativa)
* Distribuição incluída de `libcurl` e `LibreSSL` (compatível com OpenSSL, cabeçalhos em `include/openssl/`) (em `deps/curl_win64`)

> [!NOTE]
> O cabeçalho do SDK usa `<stdatomic.h>` (atômicos de C11). Ao integrar `sdk/nightauth.h` no seu próprio projeto MSVC, adicione a opção de compilador `/experimental:c11atomics` (ambos os arquivos `.vcxproj` incluídos já a definem). Sem ela, o MSVC não consegue compilar as operações atômicas usadas pela camada de sessão criptografada.

### Compilar a Solução com MSBuild

```bash
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64
```

### Implantação em Tempo de Execução

Ambos os projetos de cliente ligam-se à **biblioteca de importação** do libcurl (`libcurl.dll.a`), portanto a DLL `libcurl-x64.dll` (de `deps/curl_win64/curl-8.23.0_5-win64-mingw/bin`) **deve ser distribuída junto aos `.exe` compilados**; caso contrário, os clientes não iniciam. O projeto desktop também liga `shell32.lib`.

---

## ⚙️ Configuração & Inicialização

Configure as credenciais do seu aplicativo obtidas no painel do NightAuth:

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

Chame `init()` antes de qualquer outra chamada para estabelecer a troca de chaves Diffie-Hellman (X25519) e a sessão criptografada:

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

## 🚀 Guia de Uso do SDK

### 1. Verificação de Sessão & Blacklist

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

### 2. Login com Usuário & Senha (e 2FA)

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

### 3. Registro com Chave de Licença

```c
if (nightauth_register(api, username, password, license_key, email)) {
    printf("Registered successfully: %s\n", nightauth_get_user(api)->username);
} else {
    fprintf(stderr, "Registration failed: %s\n", nightauth_get_response(api)->message);
}
```

### 4. Acesso Apenas com Licença (Key Only)

```c
if (nightauth_license(api, license_key)) {
    printf("License verified successfully. Access granted.\n");
} else {
    fprintf(stderr, "Access denied: %s\n", nightauth_get_response(api)->message);
}
```

### 5. Atualizar Assinatura / Resgatar Chave (Upgrade)

```c
if (nightauth_upgrade(api, username, new_license_key)) {
    printf("Subscription upgraded successfully.\n");
} else {
    fprintf(stderr, "Upgrade failed: %s\n", nightauth_get_response(api)->message);
}
```

### 6. Redefinir Hardware ID (HWID Lock)

```c
if (nightauth_reset_hwid(api, username, password, totp_code)) {
    printf("HWID reset successfully. You can now log in on this machine.\n");
} else {
    fprintf(stderr, "HWID reset failed: %s\n", nightauth_get_response(api)->message);
}
```

### 7. Recuperação de Senha (Forgot Password & Reset)

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

### 8. Dados do Perfil do Usuário (`result.ToUserData()`)

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

### 9. Variáveis na Nuvem (Globais)

```c
char motd[256] = {0};
if (nightauth_get_global_var(api, "server_motd", motd, sizeof(motd))) {
    printf("Cloud Variable: %s\n", motd);
}
```

### 10. Variáveis do Usuário (User Variables)

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

### 11. Registros de Auditoria

```c
nightauth_log(api, "Suspicious memory modification attempt detected.");
```

### 12. Banir Usuário & Bloquear Máquina

```c
nightauth_ban(api, "Security tampering detected by Anti-Cheat.");
```

### 13. Webhooks no Servidor

```c
int status = 0;
const char* payload = "{\"content\":\"Client notification\"}";

if (nightauth_webhook(api, "7kR0UedlVI", payload, NULL, &status)) {
    printf("Webhook Response Status: HTTP %d\n", status);
}
```

### 14. Download Seguro na Memória

Os downloads de arquivos exigem um **ticket de acesso com escopo** obtido com `nightauth_access_ticket()` (o servidor valida o ticket antes de servir o arquivo):

```c
// Solicitar um ticket de acesso com escopo para o arquivo de destino (escopo "file")
char ticket[128] = {0};
int64_t expires_in = 0;
if (!nightauth_access_ticket(api, "file", "385624", ticket, sizeof(ticket), &expires_in)) {
    fprintf(stderr, "Ticket de acesso negado: %s\n", nightauth_get_response(api)->message);
    return 1;
}

char file_name[128] = {0};
uint8_t file_buf[65536];
size_t downloaded_len = 0;

if (nightauth_download_file(api, ticket, file_name, file_buf, sizeof(file_buf), &downloaded_len)) {
    printf("Baixado %s (%zu bytes) com segurança.\n", file_name, downloaded_len);
    // Processe ou execute o payload diretamente na memória (Reflective PE / Shellcode)
    // Apague o buffer de memória com segurança após a execução
    SecureZeroMemory(file_buf, downloaded_len);
}
```

### 15. Canais de Chat em Tempo Real

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

### 16. Eventos em Tempo Real (SSE)

```c
static bool handle_event(const char* event_json, void* user_data) {
    printf("[EVENT] %s\n", event_json);
    return true; // Return true to keep stream alive
}

nightauth_events_run(api, "ticket_value", NULL, handle_event, NULL);
```

### 17. Autenticação de Dois Fatores (2FA)

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

### 18. Ciclo de Vida da Sessão e Logout

```c
// Heartbeat ping to keep session alive
nightauth_heartbeat(api);

// Revoke and terminate session
nightauth_logout(api);

// Release memory allocations
nightauth_destroy(api);
```

### 19. Configuração Avançada do Cliente

```c
// Tempo limite por requisição em milissegundos (afeta todas as chamadas de rede subsequentes)
nightauth_set_request_timeout(api, 15000);

// Permitir HTTP sem criptografia para endpoints de desenvolvimento loopback/self-hosted
NightAuthApi* api_dev = nightauth_create_ex(
    APP_NAME, APP_API_KEY, APP_VERSION, APP_TOKEN,
    "http://127.0.0.1:8787/api/v1",   // URL de API personalizada (NULL = padrão)
    true                               // allow_insecure_loopback
);

// Inspecionar ou rotacionar a identidade (nome, api key, versão) de um handle ativo
char cur_name[128], cur_key[128], cur_ver[32];
nightauth_get_identity(api, cur_name, sizeof(cur_name), cur_key, sizeof(cur_key), cur_ver, sizeof(cur_ver));
nightauth_set_identity(api, "MyApp", "new_api_key", "1.1");
```

### 20. Chaves Públicas do Servidor & Constantes Padrão

O SDK fixa as chaves públicas de assinatura e sealed-box do servidor. Os valores padrão são distribuídos como constantes de compilação (`NIGHTAUTH_DEFAULT_API_URL`, `NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY` em `sdk/nightauth.h`), e a implementação exporta cópias mutáveis (`NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY`, `NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY`) que operadores self-hosted podem substituir por handle:

```c
// Direcionar um handle para uma instância NightAuth self-hosted com suas próprias chaves
nightauth_set_server_keys(api,
    "a1b2c3...",   // chave pública de assinatura (64 caracteres hex)
    "d4e5f6...");  // chave pública de sealed-box (64 caracteres hex)
```

### 21. Login com Licença e 2FA (`nightauth_license_with_totp`)

Para contas cujo fluxo de licença exige TOTP, passe o código do autenticador junto com a chave de licença:

```c
if (nightauth_license_with_totp(api, license_key, "123456")) {
    printf("Licença verificada com 2FA. Acesso concedido.\n");
}
```

### 22. Alterar o Nome de Usuário

Requer a senha atual. **Observação:** por segurança, o servidor encerra a sessão após uma alteração bem-sucedida — faça login novamente com o novo nome.

```c
if (nightauth_change_username(api, "new_username", password)) {
    printf("Nome de usuário alterado para %s. Faça login novamente.\n",
           nightauth_get_user(api)->username);
}
```

### 23. Verificar o Hash de um Arquivo

Valida uma soma de verificação SHA-256 do seu binário distribuído contra o registro do servidor de versões autorizadas:

```c
if (nightauth_verify_file_hash(api, "a1b2c3d4...")) {
    printf("Integridade verificada: o checksum corresponde a uma versão autorizada.\n");
} else {
    fprintf(stderr, "Verificação falhou: %s\n", nightauth_get_response(api)->message);
}
```

### 24. Telemetria da Aplicação & Usuários Online

```c
// Estatísticas agregadas da aplicação
NightAuthAppStats stats;
if (nightauth_get_stats(api, &stats)) {
    printf("Usuários: %lld (ativos %lld, banidos %lld)\n",
           (long long)stats.total_users, (long long)stats.active_users, (long long)stats.banned_users);
    printf("Licenças: %lld usadas / %lld não usadas | Versão: %s\n",
           (long long)stats.used_licenses, (long long)stats.unused_licenses, stats.version);
}

// Usuários atualmente online (retorna o número de nomes escritos)
char online[16][128];
int64_t total_online = 0;
int count = nightauth_get_online_users(api, &total_online, online, 16);
for (int i = 0; i < count; i++) {
    printf("Online: %s\n", online[i]);
}

// Metadados estáticos da aplicação (nome, versão, totais)
NightAuthAppData* app = nightauth_get_app(api);
printf("App: %s %s | Usuários: %lld | Online: %lld\n",
       app->application, app->version, (long long)app->total_users, (long long)app->online_users);
```

### 25. Gerenciamento do Hardware ID (HWID)

```c
// Ler a impressão digital da máquina calculada pelo SDK
char hwid[128] = {0};
nightauth_get_hardware_id(hwid, sizeof(hwid));

// Vincular o handle de sessão atual a um HWID explícito
nightauth_set_hardware_id(api, hwid);

// Substituir globalmente a impressão digital reportada por todos os handles futuros
// (útil para testes ou esquemas de fingerprint personalizados)
nightauth_set_hardware_id_override("custom-machine-id");
```

### 26. Histórico de Chat Paginado (`nightauth_get_chat_messages_ex`)

Obtém mensagens mais antigas que um ID dado (passe `0` para a página mais recente) e opcionalmente as marca como lidas no servidor:

```c
NightAuthChatMessage hist[25];
int count = nightauth_get_chat_messages_ex(api, "general", 0, true, hist, 25);
for (int i = 0; i < count; i++) {
    printf("[%s] %s: %s (#%s)\n", hist[i].timestamp, hist[i].author, hist[i].message, hist[i].id);
}
```

### 27. Webhooks Avançados do Lado do Servidor (`nightauth_webhook_ex`)

Webhook estendido com parâmetros de consulta, anexo opcional e saída do status HTTP:

```c
int status = 0;
const char* json = "{\"content\":\"Client notification\"}";
const void* report_data = NULL;   // bytes do anexo opcional
size_t report_len = 0;

if (nightauth_webhook_ex(api, "7kR0UedlVI", "Client notification",
                         "urgent=true", json,
                         report_data, report_len, "report.bin", &status)) {
    printf("Webhook disparado: HTTP %d\n", status);
}
```

### 28. Streaming de Eventos Avançado (`events_run_ex`, `events_cancel`)

`nightauth_events_run_ex` entrega cada evento SSE junto com seu ID de evento do servidor (permitindo retomar via `last_event_id`). `nightauth_events_cancel` interrompe um stream em andamento de outra thread (por exemplo, mediante entrada do usuário):

```c
static bool handle_event_ex(const char* event_json, const char* event_id, void* user) {
    printf("[EVENT %s] %s\n", event_id ? event_id : "-", event_json);
    return true; // Retorne true para manter o stream ativo
}

// Outra thread pode chamar nightauth_events_cancel(api) para parar o loop
if (nightauth_events_run_ex(api, ticket, NULL, handle_event_ex, NULL)) {
    printf("Stream de eventos encerrado corretamente.\n");
}
```

### 29. Auto-atualizações Assinadas e Verificadas

Verifica um envelope de atualização assinado contra uma chave pública Ed25519 fixada, baixa-o, ou entrega a URL verificada ao fluxo de aplicação assistido pelo servidor:

```c
// 1. Verificar um manifesto de atualização assinado recebido do servidor
NightAuthVerifiedUpdate update;
char err[256] = {0};
if (nightauth_signed_update_verify(envelope_json, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                   "MyApp", /*min_sequence=*/1, &update, err, sizeof(err))) {
    printf("Atualização v%s -> seq %llu (%llu bytes)\n", update.version,
           (unsigned long long)update.sequence, (unsigned long long)update.size);

    // 2. Baixar o artefato verificado para o disco
    if (nightauth_signed_update_download(&update, "update.pkg", err, sizeof(err))) {
        printf("Atualização baixada e digest verificado.\n");
    }
}

// 3. Ou baixar o instalador oficial diretamente de uma URL de manifesto
nightauth_signed_update_installer_download(manifest_url, NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY,
                                           "MyApp", "installer.exe", err, sizeof(err));

// 4. Aplicação assistida pelo servidor: passar a URL de download pendente ao handle da API
if (nightauth_apply_update(api, download_url)) {
    printf("Versão necessária: %s\n", nightauth_get_required_version(api));
}
const char* pending = nightauth_get_pending_download_url(api); // NULL quando não houver nenhuma
```

---

## 🖥️ Executar os Clientes de Console e Desktop

```bash
# Build Solution
MSBuild.exe NightAuth.sln /p:Configuration=Release /p:Platform=x64

# Interactive Terminal Console Client
./console/bin/Release/NightAuth-Console.exe

# Native Modern Win32 Desktop GUI Client
./desktop/bin/Release/NightAuth-Desktop.exe
```

---

## 📄 Licença

Este projeto é licenciado sob a **Elastic License 2.0**.

* Não é permitido fornecer o software como serviço hospedado ou gerenciado.
* Não é permitido alterar, desativar ou burlar o sistema de licenças.
* Não é permitido remover avisos de direitos autorais.

Consulte o arquivo [LICENSE](../LICENSE) para o texto legal completo.
