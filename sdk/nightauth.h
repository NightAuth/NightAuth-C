

#if defined(NIGHTAUTH_IMPLEMENTATION) && !defined(_WIN32) && !defined(_POSIX_C_SOURCE) \
    && !defined(_DEFAULT_SOURCE) && !defined(_GNU_SOURCE)
#define _POSIX_C_SOURCE 199309L
#endif

#ifndef NIGHTAUTH_C_H
#define NIGHTAUTH_C_H

#define NIGHTAUTH_DEFAULT_API_URL "https://nightauth.org/api/v1"
#define NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY "cdb2285db481666ce40003aadb874c8751dbeb9a0672a37f740dfe881ecdc742"
#define NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY "b773fc4f6cf1a0938e78e714c3d84cb386584bea24728923008471a6cd203213"
#define NIGHTAUTH_HKDF_INFO "nightauth-v1-e2e"
#define NIGHTAUTH_SEAL_INFO "nightauth-v1-init-seal"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool success;
    char message[512];
    char error[128];

    char url[512];

    int status_code;
} NightAuthResponse;

typedef struct {
    char subscription[128];
    char key[128];
    char expiry[64];
    int64_t time_left;
} NightAuthSubscriptionData;

typedef struct {
    char username[128];
    char email[128];
    char hwid[128];
    char ip[64];
    char status[64];
    char created_at[64];
    char last_login_at[64];
    NightAuthSubscriptionData subscriptions[16];
    int subscription_count;
} NightAuthUserData;

typedef struct {
    int64_t total_users;
    int64_t online_users;
    int64_t total_keys;
    char version[32];
} NightAuthAppData;

typedef struct {
    int64_t total_users;
    int64_t active_users;
    int64_t banned_users;
    int64_t total_licenses;
    int64_t used_licenses;
    int64_t unused_licenses;
    char version[32];
} NightAuthAppStats;

typedef struct {
    char id[64];
    char author[128];
    char message[1024];
    char timestamp[64];
    char status[32];
} NightAuthChatMessage;

typedef struct {
    char secret[128];
    char qr_data_url[16384];
    char uri[512];
} NightAuthTwoFactorSetupData;

typedef struct {
    char backup_codes[16][32];
    int backup_code_count;
} NightAuthTwoFactorConfirmData;

typedef bool (*NightAuthEventCallback)(const char* event_json, void* user);

typedef bool (*NightAuthEventCallbackEx)(const char* event_json, const char* event_id, void* user);

typedef struct NightAuthApi NightAuthApi;

extern const char* const NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY;
extern const char* const NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY;

bool nightauth_set_server_keys(NightAuthApi* api, const char* signing_key_hex, const char* sealed_box_key_hex);

void nightauth_set_request_timeout(NightAuthApi* api, long timeout_ms);

NightAuthApi* nightauth_create(const char* name, const char* api_key, const char* version, const char* token, const char* api_url);

void nightauth_set_identity(NightAuthApi* api, const char* name, const char* api_key, const char* version);
void nightauth_get_identity(const NightAuthApi* api, char* name, size_t name_size, char* api_key, size_t key_size,
                            char* version, size_t version_size);
NightAuthApi* nightauth_create_ex(const char* name, const char* api_key, const char* version, const char* token, const char* api_url, bool allow_insecure_loopback);
void nightauth_destroy(NightAuthApi* api);

void nightauth_init(NightAuthApi* api);
bool nightauth_login(NightAuthApi* api, const char* username, const char* password, const char* totp_code);
bool nightauth_register(NightAuthApi* api, const char* username, const char* password, const char* key, const char* email);
bool nightauth_license(NightAuthApi* api, const char* key);

bool nightauth_license_with_totp(NightAuthApi* api, const char* key, const char* totp_code);
bool nightauth_upgrade(NightAuthApi* api, const char* username, const char* key);
bool nightauth_heartbeat(NightAuthApi* api);
bool nightauth_check(NightAuthApi* api);

bool nightauth_logout(NightAuthApi* api);

bool nightauth_change_username(NightAuthApi* api, const char* new_username, const char* password);
bool nightauth_reset_hwid(NightAuthApi* api, const char* username, const char* password, const char* totp_code);
bool nightauth_ban(NightAuthApi* api, const char* reason);
bool nightauth_check_blacklist(NightAuthApi* api, const char* hwid, bool* out_blacklisted);
bool nightauth_verify_file_hash(NightAuthApi* api, const char* hash);
bool nightauth_forgot_password(NightAuthApi* api, const char* email);

bool nightauth_reset_password(NightAuthApi* api, const char* token, const char* new_password);

bool nightauth_setup_2fa(NightAuthApi* api, NightAuthTwoFactorSetupData* out_data);
bool nightauth_confirm_2fa(NightAuthApi* api, const char* code, NightAuthTwoFactorConfirmData* out_data);
bool nightauth_disable_2fa(NightAuthApi* api, const char* password, const char* code);

bool nightauth_get_global_var(NightAuthApi* api, const char* key, char* out_val, size_t out_val_size);
bool nightauth_get_user_var(NightAuthApi* api, const char* key, char* out_val, size_t out_val_size);

typedef struct { char key[128]; char value[1024]; } NightAuthUserVar;

int nightauth_get_user_variables(NightAuthApi* api, NightAuthUserVar* out, int max_vars);
bool nightauth_set_user_var(NightAuthApi* api, const char* key, const char* value);
bool nightauth_log(NightAuthApi* api, const char* message);

bool nightauth_get_stats(NightAuthApi* api, NightAuthAppStats* out_stats);

int nightauth_get_online_users(NightAuthApi* api, int64_t* out_online, char out_users[][128], int max_users);

int nightauth_get_chat_messages(NightAuthApi* api, const char* channel, NightAuthChatMessage* out_msgs, int max_msgs);
int nightauth_get_chat_messages_ex(NightAuthApi* api, const char* channel, int64_t before_id, bool mark_read,
                                   NightAuthChatMessage* out_msgs, int max_msgs);
bool nightauth_send_chat_message(NightAuthApi* api, const char* message, const char* channel);

bool nightauth_access_ticket(NightAuthApi* api, const char* scope, const char* resource_id,
                             char* out_ticket, size_t out_ticket_size, int64_t* out_expires_in);

bool nightauth_download_file(NightAuthApi* api, const char* ticket, char out_name[128],
                             uint8_t* out_data, size_t out_data_cap, size_t* out_data_len);

bool nightauth_events_run(NightAuthApi* api, const char* ticket, const char* last_event_id,
                          NightAuthEventCallback cb, void* user);

bool nightauth_events_run_ex(NightAuthApi* api, const char* ticket, const char* last_event_id,
                             NightAuthEventCallbackEx cb, void* user);

void nightauth_events_cancel(NightAuthApi* api);

bool nightauth_webhook(NightAuthApi* api, const char* web_id, const char* content,
                       const char* payload_json, int* out_status);

bool nightauth_webhook_ex(NightAuthApi* api, const char* web_id, const char* content, const char* params,
                          const char* payload_json, const void* file_data, size_t file_len,
                          const char* file_name, int* out_status);

NightAuthResponse* nightauth_get_response(NightAuthApi* api);
NightAuthUserData* nightauth_get_user(NightAuthApi* api);
NightAuthAppData* nightauth_get_app(NightAuthApi* api);
void nightauth_get_hardware_id(char* out_hwid, size_t out_size);

void nightauth_set_hardware_id_override(const char* hwid);

void nightauth_set_hardware_id(NightAuthApi* api, const char* hwid);

typedef struct {
    char application[128];
    char version[100];
    uint64_t sequence;
    char url[1024];
    uint64_t size;
    uint64_t expires_at;
    uint8_t digest[32];
} NightAuthVerifiedUpdate;

bool nightauth_signed_update_verify(const char* envelope_json, const char* pinned_pub_key_hex,
                                    const char* expected_app, uint64_t min_sequence,
                                    NightAuthVerifiedUpdate* out_update, char* err_buf, size_t err_len);

bool nightauth_signed_update_download(const NightAuthVerifiedUpdate* update, const char* destination,
                                      char* err_buf, size_t err_len);

bool nightauth_signed_update_installer_download(const char* manifest_url, const char* pinned_pub_key_hex,
                                                const char* application, const char* destination,
                                                char* err_buf, size_t err_len);

bool nightauth_apply_update(NightAuthApi* api, const char* download_url);
const char* nightauth_get_pending_download_url(const NightAuthApi* api);
const char* nightauth_get_required_version(const NightAuthApi* api);

#ifdef NIGHTAUTH_TEST_HOOKS

void nightauth_test_override_next_post(const char* body, const char* content_type);
#endif

#ifdef __cplusplus
}
#endif

#endif

#ifdef NIGHTAUTH_IMPLEMENTATION
#ifndef NIGHTAUTH_C_IMPLEMENTATION_INCLUDED
#define NIGHTAUTH_C_IMPLEMENTATION_INCLUDED

#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdatomic.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <strings.h>
#endif


const char* const NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY = NIGHTAUTH_DEFAULT_SIGNING_PUBLIC_KEY;
const char* const NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY = NIGHTAUTH_DEFAULT_SEALED_BOX_PUBLIC_KEY;

#define NA_MAX_RESPONSE_BYTES (5L * 1024 * 1024)
#define NA_TS_SKEW_MS (90LL * 1000)

typedef struct {
    uint8_t* data;
    size_t len;
    size_t cap;
} NaBuf;

static void na_buf_init(NaBuf* b) { b->data = NULL; b->len = 0; b->cap = 0; }

static bool na_buf_reserve(NaBuf* b, size_t extra) {
    if (b->len + extra <= b->cap) return true;
    size_t newcap = b->cap ? b->cap * 2 : 64;
    while (newcap < b->len + extra) newcap *= 2;
    uint8_t* nd = (uint8_t*)realloc(b->data, newcap);
    if (!nd) return false;
    b->data = nd;
    b->cap = newcap;
    return true;
}

static bool na_buf_append(NaBuf* b, const void* p, size_t n) {
    if (!na_buf_reserve(b, n + 1)) return false;
    memcpy(b->data + b->len, p, n);
    b->len += n;
    b->data[b->len] = '\0';
    return true;
}

static bool na_buf_append_str(NaBuf* b, const char* s) { return na_buf_append(b, s, strlen(s)); }

static void na_buf_free(NaBuf* b) {
    if (b->data) { OPENSSL_cleanse(b->data, b->cap); free(b->data); }
    b->data = NULL; b->len = 0; b->cap = 0;
}

static void na_hex_encode(const uint8_t* data, size_t len, char* out ) {
    static const char* h = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out[i * 2] = h[(data[i] >> 4) & 0xf];
        out[i * 2 + 1] = h[data[i] & 0xf];
    }
    out[len * 2] = '\0';
}

static bool na_hex_decode(const char* hex, uint8_t* out, size_t out_len) {
    size_t hl = strlen(hex);
    if (hl != out_len * 2) return false;
    for (size_t i = 0; i < out_len; i++) {
        int hi = -1, lo = -1;
        char c1 = hex[i * 2], c2 = hex[i * 2 + 1];
        if (c1 >= '0' && c1 <= '9') hi = c1 - '0'; else if (c1 >= 'a' && c1 <= 'f') hi = c1 - 'a' + 10; else if (c1 >= 'A' && c1 <= 'F') hi = c1 - 'A' + 10;
        if (c2 >= '0' && c2 <= '9') lo = c2 - '0'; else if (c2 >= 'a' && c2 <= 'f') lo = c2 - 'a' + 10; else if (c2 >= 'A' && c2 <= 'F') lo = c2 - 'A' + 10;
        if (hi < 0 || lo < 0) return false;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return true;
}

static const char NA_B64_TBL[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char* na_base64_encode(const uint8_t* data, size_t len) {
    char* out = (char*)malloc(((len + 2) / 3) * 4 + 1);
    if (!out) return NULL;
    size_t oi = 0, i = 0;
    for (; i + 2 < len; i += 3) {
        uint32_t n = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8) | data[i + 2];
        out[oi++] = NA_B64_TBL[(n >> 18) & 0x3f];
        out[oi++] = NA_B64_TBL[(n >> 12) & 0x3f];
        out[oi++] = NA_B64_TBL[(n >> 6) & 0x3f];
        out[oi++] = NA_B64_TBL[n & 0x3f];
    }
    if (len - i == 1) {
        uint32_t n = (uint32_t)data[i] << 16;
        out[oi++] = NA_B64_TBL[(n >> 18) & 0x3f];
        out[oi++] = NA_B64_TBL[(n >> 12) & 0x3f];
        out[oi++] = '='; out[oi++] = '=';
    } else if (len - i == 2) {
        uint32_t n = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8);
        out[oi++] = NA_B64_TBL[(n >> 18) & 0x3f];
        out[oi++] = NA_B64_TBL[(n >> 12) & 0x3f];
        out[oi++] = NA_B64_TBL[(n >> 6) & 0x3f];
        out[oi++] = '=';
    }
    out[oi] = '\0';
    return out;
}

static int na_b64_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static uint8_t* na_base64_decode(const char* in, size_t* out_len) {
    size_t inlen = strlen(in);
    uint8_t* out = (uint8_t*)malloc(inlen ? (inlen / 4 + 1) * 3 : 1);
    if (!out) return NULL;
    size_t oi = 0;
    uint32_t buf = 0;
    int bits = 0;
    for (size_t i = 0; i < inlen; i++) {
        char c = in[i];
        if (c == '=' || c == '\n' || c == '\r') continue;
        int v = na_b64_val(c);
        if (v < 0) continue;
        buf = (buf << 6) | v;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out[oi++] = (uint8_t)((buf >> bits) & 0xff);
        }
    }
    *out_len = oi;
    return out;
}

typedef enum { NA_JSON_NULL, NA_JSON_BOOL, NA_JSON_NUM, NA_JSON_STR, NA_JSON_ARR, NA_JSON_OBJ } NaJsonType;

typedef struct NaJson NaJson;
struct NaJson {
    NaJsonType type;
    int boolean;
    double number;
    char* string;
    NaJson** items;
    char** keys;
    size_t count;
};

static NaJson* na_json_new(NaJsonType t) {
    NaJson* v = (NaJson*)calloc(1, sizeof(NaJson));
    if (v) v->type = t;
    return v;
}

static void na_json_free(NaJson* v) {
    if (!v) return;
    if (v->string) free(v->string);
    for (size_t i = 0; i < v->count; i++) {
        if (v->keys) free(v->keys[i]);
        na_json_free(v->items[i]);
    }
    free(v->items);
    free(v->keys);
    free(v);
}

typedef struct { const char* s; size_t pos; size_t len; bool error; } NaJsonParser;

static void na_json_skip_ws(NaJsonParser* p) {
    while (p->pos < p->len && isspace((unsigned char)p->s[p->pos])) p->pos++;
}

static NaJson* na_json_parse_value(NaJsonParser* p);

static char* na_json_parse_string_raw(NaJsonParser* p) {
    if (p->pos >= p->len || p->s[p->pos] != '"') { p->error = true; return NULL; }
    p->pos++;
    NaBuf b; na_buf_init(&b);
    while (true) {
        if (p->pos >= p->len) { p->error = true; na_buf_free(&b); return NULL; }
        char c = p->s[p->pos++];
        if (c == '"') break;
        if (c == '\\') {
            if (p->pos >= p->len) { p->error = true; na_buf_free(&b); return NULL; }
            char e = p->s[p->pos++];
            char out;
            switch (e) {
                case '"': out = '"'; break;
                case '\\': out = '\\'; break;
                case '/': out = '/'; break;
                case 'b': out = '\b'; break;
                case 'f': out = '\f'; break;
                case 'n': out = '\n'; break;
                case 'r': out = '\r'; break;
                case 't': out = '\t'; break;
                case 'u': {
                    if (p->pos + 4 > p->len) { p->error = true; na_buf_free(&b); return NULL; }
                    char hexb[5] = { p->s[p->pos], p->s[p->pos+1], p->s[p->pos+2], p->s[p->pos+3], 0 };
                    p->pos += 4;
                    unsigned int cp = (unsigned int)strtoul(hexb, NULL, 16);

                    if (cp < 0x80) { char cc = (char)cp; na_buf_append(&b, &cc, 1); }
                    else if (cp < 0x800) {
                        char cc[2] = { (char)(0xC0 | (cp >> 6)), (char)(0x80 | (cp & 0x3F)) };
                        na_buf_append(&b, cc, 2);
                    } else {
                        char cc[3] = { (char)(0xE0 | (cp >> 12)), (char)(0x80 | ((cp >> 6) & 0x3F)), (char)(0x80 | (cp & 0x3F)) };
                        na_buf_append(&b, cc, 3);
                    }
                    continue;
                }
                default: p->error = true; na_buf_free(&b); return NULL;
            }
            na_buf_append(&b, &out, 1);
        } else {
            na_buf_append(&b, &c, 1);
        }
    }
    if (!b.data) { char* empty = (char*)malloc(1); if (empty) empty[0] = '\0'; return empty; }
    return (char*)b.data;
}

static NaJson* na_json_parse_object(NaJsonParser* p) {
    NaJson* v = na_json_new(NA_JSON_OBJ);
    p->pos++;
    na_json_skip_ws(p);
    if (p->pos < p->len && p->s[p->pos] == '}') { p->pos++; return v; }
    while (true) {
        na_json_skip_ws(p);
        char* key = na_json_parse_string_raw(p);
        if (p->error) { free(key); na_json_free(v); return NULL; }
        na_json_skip_ws(p);
        if (p->pos >= p->len || p->s[p->pos] != ':') { p->error = true; free(key); na_json_free(v); return NULL; }
        p->pos++;
        na_json_skip_ws(p);
        NaJson* val = na_json_parse_value(p);
        if (p->error) { free(key); na_json_free(val); na_json_free(v); return NULL; }
        NaJson** nitems = (NaJson**)realloc(v->items, (v->count + 1) * sizeof(NaJson*));
        if (!nitems) { p->error = true; free(key); na_json_free(val); na_json_free(v); return NULL; }
        v->items = nitems;
        char** nkeys = (char**)realloc(v->keys, (v->count + 1) * sizeof(char*));
        if (!nkeys) { p->error = true; free(key); na_json_free(val); na_json_free(v); return NULL; }
        v->keys = nkeys;
        v->items[v->count] = val;
        v->keys[v->count] = key;
        v->count++;
        na_json_skip_ws(p);
        if (p->pos >= p->len) { p->error = true; na_json_free(v); return NULL; }
        if (p->s[p->pos] == ',') { p->pos++; continue; }
        if (p->s[p->pos] == '}') { p->pos++; break; }
        p->error = true; na_json_free(v); return NULL;
    }
    return v;
}

static NaJson* na_json_parse_array(NaJsonParser* p) {
    NaJson* v = na_json_new(NA_JSON_ARR);
    p->pos++;
    na_json_skip_ws(p);
    if (p->pos < p->len && p->s[p->pos] == ']') { p->pos++; return v; }
    while (true) {
        na_json_skip_ws(p);
        NaJson* val = na_json_parse_value(p);
        if (p->error) { na_json_free(val); na_json_free(v); return NULL; }
        NaJson** nitems = (NaJson**)realloc(v->items, (v->count + 1) * sizeof(NaJson*));
        if (!nitems) { p->error = true; na_json_free(val); na_json_free(v); return NULL; }
        v->items = nitems;
        v->items[v->count++] = val;
        na_json_skip_ws(p);
        if (p->pos >= p->len) { p->error = true; na_json_free(v); return NULL; }
        if (p->s[p->pos] == ',') { p->pos++; continue; }
        if (p->s[p->pos] == ']') { p->pos++; break; }
        p->error = true; na_json_free(v); return NULL;
    }
    return v;
}

static NaJson* na_json_parse_value(NaJsonParser* p) {
    if (p->pos >= p->len) { p->error = true; return NULL; }
    char c = p->s[p->pos];
    if (c == '{') return na_json_parse_object(p);
    if (c == '[') return na_json_parse_array(p);
    if (c == '"') {
        char* s = na_json_parse_string_raw(p);
        if (p->error) { free(s); return NULL; }
        NaJson* v = na_json_new(NA_JSON_STR);
        v->string = s;
        return v;
    }
    if (!strncmp(p->s + p->pos, "true", 4)) { p->pos += 4; NaJson* v = na_json_new(NA_JSON_BOOL); v->boolean = 1; return v; }
    if (!strncmp(p->s + p->pos, "false", 5)) { p->pos += 5; NaJson* v = na_json_new(NA_JSON_BOOL); v->boolean = 0; return v; }
    if (!strncmp(p->s + p->pos, "null", 4)) { p->pos += 4; return na_json_new(NA_JSON_NULL); }

    size_t start = p->pos;
    if (p->pos < p->len && (p->s[p->pos] == '-' || p->s[p->pos] == '+')) p->pos++;
    while (p->pos < p->len && (isdigit((unsigned char)p->s[p->pos]) || p->s[p->pos] == '.' || p->s[p->pos] == 'e' || p->s[p->pos] == 'E' || p->s[p->pos] == '+' || p->s[p->pos] == '-')) p->pos++;
    if (p->pos == start) { p->error = true; return NULL; }
    char numbuf[64];
    size_t nlen = p->pos - start;
    if (nlen >= sizeof(numbuf)) { p->error = true; return NULL; }
    memcpy(numbuf, p->s + start, nlen);
    numbuf[nlen] = '\0';
    NaJson* v = na_json_new(NA_JSON_NUM);
    v->number = strtod(numbuf, NULL);
    return v;
}

static NaJson* na_json_parse(const char* s, size_t len) {
    NaJsonParser p = { s, 0, len, false };
    na_json_skip_ws(&p);
    NaJson* v = na_json_parse_value(&p);
    if (p.error) { na_json_free(v); return NULL; }
    na_json_skip_ws(&p);
    if (p.pos != p.len) { na_json_free(v); return NULL; }
    return v;
}

static NaJson* na_json_obj_get(const NaJson* obj, const char* key) {
    if (!obj || obj->type != NA_JSON_OBJ) return NULL;
    for (size_t i = 0; i < obj->count; i++) {
        if (strcmp(obj->keys[i], key) == 0) return obj->items[i];
    }
    return NULL;
}

static const char* na_json_get_str(const NaJson* obj, const char* key, const char* dflt) {
    NaJson* v = na_json_obj_get(obj, key);
    return (v && v->type == NA_JSON_STR) ? v->string : dflt;
}

static void na_json_escape_append(NaBuf* b, const char* s) {
    na_buf_append(b, "\"", 1);
    for (const unsigned char* p = (const unsigned char*)s; *p; p++) {
        switch (*p) {
            case '"': na_buf_append_str(b, "\\\""); break;
            case '\\': na_buf_append_str(b, "\\\\"); break;
            case '\n': na_buf_append_str(b, "\\n"); break;
            case '\r': na_buf_append_str(b, "\\r"); break;
            case '\t': na_buf_append_str(b, "\\t"); break;
            default:
                if (*p < 0x20) { char tmp[8]; snprintf(tmp, sizeof(tmp), "\\u%04x", *p); na_buf_append_str(b, tmp); }
                else na_buf_append(b, p, 1);
        }
    }
    na_buf_append(b, "\"", 1);
}

static bool na_openssl_ok(int rc) { return rc == 1; }

static bool na_hkdf_sha256(const uint8_t* ikm, size_t ikm_len, const uint8_t* info, size_t info_len, uint8_t* out, size_t out_len) {
    bool ok = false;
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    if (!pctx) return false;
    if (!na_openssl_ok(EVP_PKEY_derive_init(pctx))) goto done;
    if (!na_openssl_ok(EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()))) goto done;
    if (!na_openssl_ok(EVP_PKEY_CTX_set1_hkdf_salt(pctx, (const unsigned char*)"", 0))) goto done;
    if (!na_openssl_ok(EVP_PKEY_CTX_set1_hkdf_key(pctx, ikm, (int)ikm_len))) goto done;
    if (!na_openssl_ok(EVP_PKEY_CTX_add1_hkdf_info(pctx, info, (int)info_len))) goto done;
    {
        size_t len = out_len;
        if (!na_openssl_ok(EVP_PKEY_derive(pctx, out, &len))) goto done;
        if (len != out_len) goto done;
    }
    ok = true;
done:
    EVP_PKEY_CTX_free(pctx);
    return ok;
}

static EVP_PKEY* na_x25519_generate(void) {
    EVP_PKEY* pkey = NULL;
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, NULL);
    if (!pctx) return NULL;
    if (na_openssl_ok(EVP_PKEY_keygen_init(pctx)) && na_openssl_ok(EVP_PKEY_keygen(pctx, &pkey))) {

    } else {
        pkey = NULL;
    }
    EVP_PKEY_CTX_free(pctx);
    return pkey;
}

static bool na_x25519_public_raw(EVP_PKEY* pkey, uint8_t out[32]) {
    size_t len = 32;
    return na_openssl_ok(EVP_PKEY_get_raw_public_key(pkey, out, &len)) && len == 32;
}

static EVP_PKEY* na_x25519_public_from_raw(const uint8_t raw32[32]) {
    return EVP_PKEY_new_raw_public_key(EVP_PKEY_X25519, NULL, raw32, 32);
}

static bool na_x25519_dh(EVP_PKEY* priv, const uint8_t peer_pub_raw[32], uint8_t out[32]) {
    bool ok = false;
    EVP_PKEY* peer = na_x25519_public_from_raw(peer_pub_raw);
    if (!peer) return false;
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new(priv, NULL);
    if (!pctx) { EVP_PKEY_free(peer); return false; }
    if (!na_openssl_ok(EVP_PKEY_derive_init(pctx))) goto done;
    if (!na_openssl_ok(EVP_PKEY_derive_set_peer(pctx, peer))) goto done;
    {
        size_t len = 32;
        if (!na_openssl_ok(EVP_PKEY_derive(pctx, out, &len))) goto done;
        if (len != 32) goto done;
    }
    ok = true;
done:
    EVP_PKEY_CTX_free(pctx);
    EVP_PKEY_free(peer);
    return ok;
}

static bool na_ed25519_verify(const uint8_t pub_raw[32], const uint8_t* data, size_t data_len, const uint8_t* sig, size_t sig_len) {
    bool ok = false;
    EVP_PKEY* pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, NULL, pub_raw, 32);
    if (!pkey) return false;
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) { EVP_PKEY_free(pkey); return false; }
    if (na_openssl_ok(EVP_DigestVerifyInit(mdctx, NULL, NULL, NULL, pkey))) {
        ok = EVP_DigestVerify(mdctx, sig, sig_len, data, data_len) == 1;
    }
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
    return ok;
}

static bool na_aes_gcm_encrypt(const uint8_t key[32], const uint8_t* pt, size_t pt_len,
                                const uint8_t* aad, size_t aad_len,
                                uint8_t nonce_out[12], uint8_t** out_ct, size_t* out_ct_len) {
    bool ok = false;
    uint8_t* ct = NULL;
    EVP_CIPHER_CTX* ctx = NULL;
    if (!na_openssl_ok(RAND_bytes(nonce_out, 12))) return false;
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    if (!na_openssl_ok(EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))) goto done;
    if (!na_openssl_ok(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL))) goto done;
    if (!na_openssl_ok(EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce_out))) goto done;
    {
        int outl = 0;
        if (aad_len > 0 && !na_openssl_ok(EVP_EncryptUpdate(ctx, NULL, &outl, aad, (int)aad_len))) goto done;
        ct = (uint8_t*)malloc(pt_len + 16);
        if (!ct) goto done;
        int len1 = 0;
        if (pt_len > 0 && !na_openssl_ok(EVP_EncryptUpdate(ctx, ct, &len1, pt, (int)pt_len))) goto done;
        int len2 = 0;
        if (!na_openssl_ok(EVP_EncryptFinal_ex(ctx, ct + len1, &len2))) goto done;
        uint8_t tag[16];
        if (!na_openssl_ok(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))) goto done;
        memcpy(ct + len1 + len2, tag, 16);
        *out_ct = ct;
        *out_ct_len = (size_t)(len1 + len2) + 16;
        ct = NULL;
    }
    ok = true;
done:
    if (ct) free(ct);
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    return ok;
}

static bool na_aes_gcm_decrypt(const uint8_t key[32], const uint8_t nonce[12],
                                const uint8_t* ct_with_tag, size_t ct_with_tag_len,
                                const uint8_t* aad, size_t aad_len,
                                uint8_t** out_pt, size_t* out_pt_len) {
    if (ct_with_tag_len < 16) return false;
    bool ok = false;
    size_t ct_len = ct_with_tag_len - 16;
    const uint8_t* tag = ct_with_tag + ct_len;
    uint8_t* pt = (uint8_t*)malloc(ct_len ? ct_len : 1);
    EVP_CIPHER_CTX* ctx = NULL;
    if (!pt) return false;
    ctx = EVP_CIPHER_CTX_new();
    if (!ctx) { free(pt); return false; }
    if (!na_openssl_ok(EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))) goto done;
    if (!na_openssl_ok(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL))) goto done;
    if (!na_openssl_ok(EVP_DecryptInit_ex(ctx, NULL, NULL, key, nonce))) goto done;
    {
        int outl = 0;
        if (aad_len > 0 && !na_openssl_ok(EVP_DecryptUpdate(ctx, NULL, &outl, aad, (int)aad_len))) goto done;
        int len1 = 0;
        if (ct_len > 0 && !na_openssl_ok(EVP_DecryptUpdate(ctx, pt, &len1, ct_with_tag, (int)ct_len))) goto done;
        if (!na_openssl_ok(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void*)tag))) goto done;
        int len2 = 0;

        if (EVP_DecryptFinal_ex(ctx, pt + len1, &len2) != 1) goto done;
        *out_pt = pt;
        *out_pt_len = (size_t)(len1 + len2);
        pt = NULL;
    }
    ok = true;
done:
    if (pt) free(pt);
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    return ok;
}

static char* na_seal_init(const char* plaintext_json, const uint8_t recipient_pub_raw[32]) {
    char* result = NULL;
    EVP_PKEY* ephemeral = na_x25519_generate();
    if (!ephemeral) return NULL;
    uint8_t ephem_pub[32];
    if (!na_x25519_public_raw(ephemeral, ephem_pub)) { EVP_PKEY_free(ephemeral); return NULL; }
    uint8_t secret[32];
    if (!na_x25519_dh(ephemeral, recipient_pub_raw, secret)) { EVP_PKEY_free(ephemeral); return NULL; }
    uint8_t key[32];
    bool kdf_ok = na_hkdf_sha256(secret, 32, (const uint8_t*)"nightauth-v1-init-seal", strlen("nightauth-v1-init-seal"), key, 32);
    OPENSSL_cleanse(secret, sizeof(secret));
    EVP_PKEY_free(ephemeral);
    if (!kdf_ok) return NULL;

    uint8_t nonce[12];
    uint8_t* ct = NULL;
    size_t ct_len = 0;
    bool enc_ok = na_aes_gcm_encrypt(key, (const uint8_t*)plaintext_json, strlen(plaintext_json),
                                      ephem_pub, 32, nonce, &ct, &ct_len);
    OPENSSL_cleanse(key, sizeof(key));
    if (!enc_ok) return NULL;

    size_t ctonly_len = ct_len - 16;
    size_t total = 32 + 12 + 16 + ctonly_len;
    uint8_t* out = (uint8_t*)malloc(total);
    if (out) {
        memcpy(out, ephem_pub, 32);
        memcpy(out + 32, nonce, 12);
        memcpy(out + 44, ct + ctonly_len, 16);
        memcpy(out + 60, ct, ctonly_len);
        result = na_base64_encode(out, total);
        free(out);
    }
    free(ct);
    return result;
}

typedef struct {
    NaBuf buf;
    long status;
    char location[512];
    char failure[24];
    bool too_large;
} NaHttpResp;

static const char* na_transport_code(const NaHttpResp* r) {
    return (r && r->failure[0]) ? r->failure : "network_error";
}

static size_t na_curl_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    NaHttpResp* r = (NaHttpResp*)userdata;
    size_t n = size * nmemb;
    if (r->buf.len + n > NA_MAX_RESPONSE_BYTES) {

        r->too_large = true;
        return 0;
    }
    if (!na_buf_append(&r->buf, ptr, n)) return 0;
    return n;
}

struct NightAuthApi {
    char name[128];
    char api_key[128];
    char version[32];
    char token[128];
    char api_url[256];
    bool allow_insecure_loopback;

    char signing_pin[65];
    char sealed_pin[65];
    long request_timeout_ms;

    long last_status;

    bool has_hwid;
    char hwid[128];

    NightAuthResponse response;
    NightAuthUserData user;
    NightAuthAppData app;

    EVP_PKEY* client_ephemeral;
    uint8_t shared_key[32];
    char session_id[40];
    bool initialized;

    char pending_download_url[1024];
    char required_version[64];

    atomic_int events_cancel;

    const uint8_t* mp_data;
    size_t mp_len;
    const char* mp_name;
};

static void na_set_error(NightAuthApi* api, const char* msg, const char* err) {
    if (!api) return;
    memset(&api->response, 0, sizeof(api->response));
    api->response.status_code = (int)api->last_status;
    api->response.success = false;
    strncpy(api->response.message, (msg && msg[0]) ? msg : "Operation failed", sizeof(api->response.message) - 1);
    strncpy(api->response.error, (err && err[0]) ? err : "error", sizeof(api->response.error) - 1);
}

static void na_set_success(NightAuthApi* api, const char* msg) {
    if (!api) return;
    memset(&api->response, 0, sizeof(api->response));
    api->response.status_code = (int)api->last_status;
    api->response.success = true;
    if (msg) strncpy(api->response.message, msg, sizeof(api->response.message) - 1);
}

static bool na_ascii_equal_ci(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return false;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

static bool na_parse_base_url(const char* url, bool* out_is_https, bool* out_is_loopback) {
    if (!url || !out_is_https || !out_is_loopback) return false;
    *out_is_https = false;
    *out_is_loopback = false;

    CURLU* parsed = curl_url();
    if (!parsed) return false;
    bool valid = curl_url_set(parsed, CURLUPART_URL, url, 0) == CURLUE_OK;
    char *scheme = NULL, *host = NULL, *user = NULL, *password = NULL;
    char *path = NULL, *query = NULL, *fragment = NULL;
    if (valid) valid = curl_url_get(parsed, CURLUPART_SCHEME, &scheme, 0) == CURLUE_OK;
    if (valid) valid = curl_url_get(parsed, CURLUPART_HOST, &host, 0) == CURLUE_OK;
    if (valid && curl_url_get(parsed, CURLUPART_USER, &user, 0) == CURLUE_OK) valid = false;
    if (valid && curl_url_get(parsed, CURLUPART_PASSWORD, &password, 0) == CURLUE_OK) valid = false;
    if (valid && curl_url_get(parsed, CURLUPART_QUERY, &query, 0) == CURLUE_OK) valid = false;
    if (valid && curl_url_get(parsed, CURLUPART_FRAGMENT, &fragment, 0) == CURLUE_OK) valid = false;
    if (valid && curl_url_get(parsed, CURLUPART_PATH, &path, 0) == CURLUE_OK &&
        path && path[0] && strcmp(path, "/") != 0 && strcmp(path, "/api/v1") != 0) valid = false;

    if (valid) {
        *out_is_https = na_ascii_equal_ci(scheme, "https");
        bool is_http = na_ascii_equal_ci(scheme, "http");
        valid = *out_is_https || is_http;

        *out_is_loopback = na_ascii_equal_ci(host, "localhost") ||
                           na_ascii_equal_ci(host, "127.0.0.1") ||
                           na_ascii_equal_ci(host, "::1") ||
                           na_ascii_equal_ci(host, "[::1]");
    }

    curl_free(scheme);
    curl_free(host);
    curl_free(user);
    curl_free(password);
    curl_free(path);
    curl_free(query);
    curl_free(fragment);
    curl_url_cleanup(parsed);
    return valid;
}

static bool na_base_url_allowed(const char* url, bool allow_insecure_loopback,
                                bool* out_is_https, bool* out_is_loopback) {
    if (!na_parse_base_url(url, out_is_https, out_is_loopback)) return false;
    return *out_is_https || (*out_is_loopback && allow_insecure_loopback);
}

typedef bool (*NaTransportHookFn)(NightAuthApi* api, const char* method, const char* target,
                                  const char* body, NaHttpResp* out);
static NaTransportHookFn na_test_transport_hook = NULL;

#ifdef NIGHTAUTH_TEST_HOOKS

static char* na_override_body;
static char na_override_ct[160];
void nightauth_test_override_next_post(const char* body, const char* content_type) {
    free(na_override_body);
    na_override_body = NULL;
    if (body) {
        size_t n = strlen(body) + 1;
        na_override_body = (char*)malloc(n);
        if (na_override_body) memcpy(na_override_body, body, n);
    }
    snprintf(na_override_ct, sizeof na_override_ct, "%s", content_type ? content_type : "application/json");
}
#endif

static bool na_http_request_network(NightAuthApi* api, const char* method, const char* target,
                                    const char* body, NaHttpResp* out);

static bool na_http_request(NightAuthApi* api, const char* method, const char* target,
                            const char* body, NaHttpResp* out) {
    if (!api || !method || !target || target[0] != '/') return false;
    memset(out, 0, sizeof(*out));
    na_buf_init(&out->buf);
    api->last_status = 0;
    bool ok = na_test_transport_hook ? na_test_transport_hook(api, method, target, body, out)
                                     : na_http_request_network(api, method, target, body, out);

    api->last_status = (out->failure[0] && strcmp(out->failure, "response_too_large") != 0) ? 0 : out->status;
    return ok;
}

static bool na_http_request_network(NightAuthApi* api, const char* method, const char* target,
                                    const char* body, NaHttpResp* out) {

    bool is_https = false, loopback = false;
    if (!na_base_url_allowed(api->api_url, api->allow_insecure_loopback,
                             &is_https, &loopback)) {
        return false;
    }

    char full_url[700];
    if (snprintf(full_url, sizeof(full_url), "%s%s", api->api_url, target) >= (int)sizeof(full_url)) return false;

    CURL* curl = curl_easy_init();
    if (!curl) return false;

    struct curl_slist* headers = NULL;
#ifdef NIGHTAUTH_TEST_HOOKS
    const bool overridden = na_override_body != NULL && strcmp(method, "POST") == 0 && strcmp(target, "/init") != 0;
    if (overridden) {
        char ct_line[200];
        snprintf(ct_line, sizeof ct_line, "Content-Type: %s", na_override_ct);
        headers = curl_slist_append(headers, ct_line);
        body = na_override_body;
    }
#else
    const bool overridden = false;
#endif
    const bool multipart = !overridden && api->mp_data != NULL && strcmp(method, "POST") == 0;
    if (!multipart && !overridden) headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: NightAuth-C-SDK/1.0");
    curl_mime* mime = NULL;

    curl_easy_setopt(curl, CURLOPT_URL, full_url);
    if (strcmp(method, "GET") == 0) {
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    } else if (multipart) {
        mime = curl_mime_init(curl);
        curl_mimepart* part = curl_mime_addpart(mime);
        curl_mime_name(part, "envelope");
        curl_mime_data(part, body ? body : "", CURL_ZERO_TERMINATED);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "file");
        curl_mime_filename(part, api->mp_name ? api->mp_name : "file.bin");
        curl_mime_type(part, "application/octet-stream");
        curl_mime_data(part, (const char*)api->mp_data, api->mp_len);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    } else {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body ? body : "");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)(body ? strlen(body) : 0));
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, na_curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, api->request_timeout_ms > 0 ? api->request_timeout_ms : 20000L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, is_https ? "https" : "http");
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    CURLcode rc = curl_easy_perform(curl);
    bool ok = (rc == CURLE_OK);
    if (!ok) {
        const char* code = rc == CURLE_OPERATION_TIMEDOUT ? "timeout"
                           : (rc == CURLE_WRITE_ERROR && out->too_large) ? "response_too_large" : "network_error";
        snprintf(out->failure, sizeof(out->failure), "%s", code);

        if (out->too_large) {
            long status = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
            out->status = status;
        }
    }
    if (ok) {
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        out->status = status;
        if (status >= 300 && status < 400) {
            char* loc = NULL;
            if (curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &loc) == CURLE_OK && loc)
                snprintf(out->location, sizeof(out->location), "%s", loc);
            ok = false;
        }
    }

    curl_slist_free_all(headers);
#ifdef NIGHTAUTH_TEST_HOOKS
    if (overridden) { free(na_override_body); na_override_body = NULL; }
#endif
    if (mime) curl_mime_free(mime);
    curl_easy_cleanup(curl);
    if (!ok) na_buf_free(&out->buf);
    return ok;
}

static bool na_post_raw(NightAuthApi* api, const char* path, const char* json_body, NaHttpResp* out) {
    return na_http_request(api, "POST", path, json_body, out);
}

static char g_hwid_override[128];

void nightauth_set_hardware_id_override(const char* hwid) {
    if (!hwid) { g_hwid_override[0] = '\0'; return; }
    strncpy(g_hwid_override, hwid, sizeof(g_hwid_override) - 1);
    g_hwid_override[sizeof(g_hwid_override) - 1] = '\0';
}

void nightauth_set_hardware_id(NightAuthApi* api, const char* hwid) {
    if (!api) return;
    api->has_hwid = hwid != NULL;
    snprintf(api->hwid, sizeof(api->hwid), "%s", hwid ? hwid : "");
}

void nightauth_get_hardware_id(char* out_hwid, size_t out_size);

static void na_client_hwid(const NightAuthApi* api, char* out, size_t out_size) {
    if (api->has_hwid) snprintf(out, out_size, "%s", api->hwid);
    else nightauth_get_hardware_id(out, out_size);
}

void nightauth_get_hardware_id(char* out_hwid, size_t out_size) {
    if (!out_hwid || out_size == 0) return;
    if (g_hwid_override[0]) {
        strncpy(out_hwid, g_hwid_override, out_size - 1);
        out_hwid[out_size - 1] = '\0';
        return;
    }
    const char* hostname_env = getenv("COMPUTERNAME");
    if (!hostname_env || !hostname_env[0]) hostname_env = getenv("HOSTNAME");
    if (!hostname_env || !hostname_env[0]) hostname_env = "unknown_host";
    const char* username_env = getenv("USERNAME");
    if (!username_env || !username_env[0]) username_env = getenv("USER");
    if (!username_env || !username_env[0]) username_env = "unknown_user";

    char hostname[256];
    char username[256];
    size_t hi = 0;
    for (; hostname_env[hi] && hi < sizeof(hostname) - 1; ++hi) {
        char c = hostname_env[hi];
        hostname[hi] = (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
    }
    hostname[hi] = '\0';
    size_t ui = 0;
    for (; username_env[ui] && ui < sizeof(username) - 1; ++ui) {
        char c = username_env[ui];
        username[ui] = (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
    }
    username[ui] = '\0';

    char raw[560];
    snprintf(raw, sizeof(raw), "%s-%s-nightauth-hwid", hostname, username);

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (mdctx && EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) &&
        EVP_DigestUpdate(mdctx, raw, strlen(raw)) &&
        EVP_DigestFinal_ex(mdctx, hash, &hash_len)) {
        char hex[2 * EVP_MAX_MD_SIZE + 1];
        na_hex_encode(hash, hash_len, hex);
        strncpy(out_hwid, hex, out_size - 1);
        out_hwid[out_size - 1] = '\0';
    } else {
        strncpy(out_hwid, "default_hwid_hash", out_size - 1);
        out_hwid[out_size - 1] = '\0';
    }
    if (mdctx) EVP_MD_CTX_free(mdctx);
}

void nightauth_set_identity(NightAuthApi* api, const char* name, const char* api_key, const char* version) {
    if (!api) return;
    if (name) { strncpy(api->name, name, sizeof(api->name) - 1); api->name[sizeof(api->name) - 1] = '\0'; }
    if (api_key) { strncpy(api->api_key, api_key, sizeof(api->api_key) - 1); api->api_key[sizeof(api->api_key) - 1] = '\0'; }
    if (version) { strncpy(api->version, version, sizeof(api->version) - 1); api->version[sizeof(api->version) - 1] = '\0'; }
}

void nightauth_get_identity(const NightAuthApi* api, char* name, size_t name_size, char* api_key, size_t key_size,
                            char* version, size_t version_size) {
    if (!api) return;
    if (name && name_size) { strncpy(name, api->name, name_size - 1); name[name_size - 1] = '\0'; }
    if (api_key && key_size) { strncpy(api_key, api->api_key, key_size - 1); api_key[key_size - 1] = '\0'; }
    if (version && version_size) { strncpy(version, api->version, version_size - 1); version[version_size - 1] = '\0'; }
}

NightAuthApi* nightauth_create(const char* name, const char* api_key, const char* version, const char* token, const char* api_url) {
    return nightauth_create_ex(name, api_key, version, token, api_url, false);
}

NightAuthApi* nightauth_create_ex(const char* name, const char* api_key, const char* version, const char* token, const char* api_url, bool allow_insecure_loopback) {
    if (!name || !name[0] || !api_key || !api_key[0]) return NULL;
    if ((api_url && strlen(api_url) >= sizeof(((NightAuthApi*)0)->api_url)) ||
        strlen(name) >= sizeof(((NightAuthApi*)0)->name) ||
        strlen(api_key) >= sizeof(((NightAuthApi*)0)->api_key) ||
        (token && strlen(token) >= sizeof(((NightAuthApi*)0)->token))) return NULL;
    NightAuthApi* api = (NightAuthApi*)calloc(1, sizeof(NightAuthApi));
    if (!api) return NULL;

    strncpy(api->name, name, sizeof(api->name) - 1);
    strncpy(api->api_key, api_key, sizeof(api->api_key) - 1);
    strncpy(api->version, (version && version[0]) ? version : "1.0", sizeof(api->version) - 1);
    if (token) strncpy(api->token, token, sizeof(api->token) - 1);
    strncpy(api->api_url, (api_url && api_url[0]) ? api_url : NIGHTAUTH_DEFAULT_API_URL, sizeof(api->api_url) - 1);
    snprintf(api->signing_pin, sizeof api->signing_pin, "%s", NIGHTAUTH_SERVER_SIGNING_PUBLIC_KEY);
    snprintf(api->sealed_pin, sizeof api->sealed_pin, "%s", NIGHTAUTH_SERVER_SEALED_BOX_PUBLIC_KEY);
    api->request_timeout_ms = 20000;
    size_t ulen = strlen(api->api_url);
    if (ulen > 0 && api->api_url[ulen - 1] == '/') api->api_url[ulen - 1] = '\0';
    api->allow_insecure_loopback = allow_insecure_loopback;

    bool is_https = false, is_loopback = false;
    if (!na_base_url_allowed(api->api_url, allow_insecure_loopback,
                             &is_https, &is_loopback)) {
        OPENSSL_cleanse(api, sizeof(*api));
        free(api);
        return NULL;
    }

    return api;
}

void nightauth_destroy(NightAuthApi* api) {
    if (!api) return;
    if (api->client_ephemeral) EVP_PKEY_free(api->client_ephemeral);
    OPENSSL_cleanse(api->shared_key, sizeof(api->shared_key));
    OPENSSL_cleanse(api, sizeof(*api));
    free(api);
}

static bool na_is_key_hex(const char* s) {
    if (!s || strlen(s) != 64) return false;
    for (const char* p = s; *p; p++)
        if (!isxdigit((unsigned char)*p)) return false;
    return true;
}

bool nightauth_set_server_keys(NightAuthApi* api, const char* signing_key_hex, const char* sealed_box_key_hex) {
    if (!api || !na_is_key_hex(signing_key_hex) || !na_is_key_hex(sealed_box_key_hex)) return false;
    snprintf(api->signing_pin, sizeof api->signing_pin, "%s", signing_key_hex);
    snprintf(api->sealed_pin, sizeof api->sealed_pin, "%s", sealed_box_key_hex);
    return true;
}

void nightauth_set_request_timeout(NightAuthApi* api, long timeout_ms) {
    if (api && timeout_ms > 0) api->request_timeout_ms = timeout_ms;
}

NightAuthResponse* nightauth_get_response(NightAuthApi* api) { return api ? &api->response : NULL; }
NightAuthUserData* nightauth_get_user(NightAuthApi* api) { return api ? &api->user : NULL; }
NightAuthAppData* nightauth_get_app(NightAuthApi* api) { return api ? &api->app : NULL; }

void nightauth_init(NightAuthApi* api) {
    if (!api) return;
    if (api->client_ephemeral) { EVP_PKEY_free(api->client_ephemeral); api->client_ephemeral = NULL; }
    api->initialized = false;
    OPENSSL_cleanse(api->shared_key, sizeof(api->shared_key));
    memset(api->session_id, 0, sizeof(api->session_id));
    memset(&api->user, 0, sizeof(api->user));
    memset(&api->app, 0, sizeof(api->app));

    uint8_t recipient_raw[32];
    if (!na_hex_decode(api->sealed_pin, recipient_raw, 32)) {
        na_set_error(api, "invalid pinned sealed-box key", "init_failed");
        return;
    }

    EVP_PKEY* eph = na_x25519_generate();
    if (!eph) { na_set_error(api, "failed to generate ephemeral keypair", "init_failed"); return; }
    uint8_t client_pub[32];
    if (!na_x25519_public_raw(eph, client_pub)) { EVP_PKEY_free(eph); na_set_error(api, "failed to export ephemeral public key", "init_failed"); return; }
    char k1_hex[65];
    na_hex_encode(client_pub, 32, k1_hex);

    uint8_t id_bytes[16];
    if (RAND_bytes(id_bytes, sizeof(id_bytes)) != 1) {
        EVP_PKEY_free(eph);
        OPENSSL_cleanse(id_bytes, sizeof(id_bytes));
        na_set_error(api, "cryptographic random generator failed", "init_failed");
        return;
    }
    char session_id[33];
    na_hex_encode(id_bytes, 16, session_id);

    NaBuf payload; na_buf_init(&payload);
    na_buf_append_str(&payload, "{\"apikey\":");
    na_json_escape_append(&payload, api->api_key);
    na_buf_append_str(&payload, ",\"name\":");
    na_json_escape_append(&payload, api->name);
    na_buf_append_str(&payload, ",\"version\":");
    na_json_escape_append(&payload, api->version);
    na_buf_append_str(&payload, ",\"id\":");
    na_json_escape_append(&payload, session_id);
    na_buf_append_str(&payload, ",\"k1\":");
    na_json_escape_append(&payload, k1_hex);
    na_buf_append_str(&payload, "}");

    char* sealed_b64 = na_seal_init((const char*)payload.data, recipient_raw);
    na_buf_free(&payload);
    if (!sealed_b64) { EVP_PKEY_free(eph); na_set_error(api, "failed to seal init payload", "init_failed"); return; }

    NaBuf body; na_buf_init(&body);
    na_buf_append_str(&body, "{\"p\":");
    na_json_escape_append(&body, sealed_b64);
    na_buf_append_str(&body, "}");
    free(sealed_b64);

    NaHttpResp raw;
    bool http_ok = na_post_raw(api, "/init", (const char*)body.data, &raw);
    na_buf_free(&body);
    if (!http_ok) { EVP_PKEY_free(eph); na_set_error(api, "network request to " "/init" " failed", na_transport_code(&raw)); return; }

    NaJson* resp = na_json_parse((const char*)raw.buf.data, raw.buf.len);
    na_buf_free(&raw.buf);
    if (!resp || resp->type != NA_JSON_OBJ) {

        char msg[96];
        if (raw.status != 200) snprintf(msg, sizeof(msg), "Initialization failed: HTTP %ld", (long)raw.status);
        else snprintf(msg, sizeof(msg), "malformed init response");

        EVP_PKEY_free(eph); na_json_free(resp); na_set_error(api, msg, "invalid_response"); return;
    }

    NaJson* successVal = na_json_obj_get(resp, "success");
    if (raw.status != 200 || !(successVal && successVal->type == NA_JSON_BOOL && successVal->boolean)) {
        const char* dl = na_json_get_str(resp, "downloadUrl", NULL);
        if (dl) snprintf(api->pending_download_url, sizeof(api->pending_download_url), "%s", dl);
        const char* reqVer = na_json_get_str(resp, "requiredVersion", NULL);
        if (reqVer) snprintf(api->required_version, sizeof(api->required_version), "%s", reqVer);
        na_set_error(api, na_json_get_str(resp, "message", "Initialization failed"), na_json_get_str(resp, "error", "init_failed"));
        na_json_free(resp);
        EVP_PKEY_free(eph);
        return;
    }

    NaJson *k2v = na_json_obj_get(resp, "k2"), *vkv = na_json_obj_get(resp, "vk"), *tsv = na_json_obj_get(resp, "ts"), *sigv = na_json_obj_get(resp, "sig");
    if (!k2v || k2v->type != NA_JSON_STR || !vkv || vkv->type != NA_JSON_STR || !tsv || tsv->type != NA_JSON_NUM || !sigv || sigv->type != NA_JSON_STR) {
        na_set_error(api, "init response missing signature binding fields", "untrusted_response");
        na_json_free(resp);
        EVP_PKEY_free(eph);
        return;
    }

    const char* k2_hex = k2v->string;
    const char* vk_hex = vkv->string;
    int64_t ts = (int64_t)tsv->number;
    const char* sig_b64 = sigv->string;

#ifdef _WIN32
    bool vk_matches = _stricmp(vk_hex, api->signing_pin) == 0;
#else
    bool vk_matches = strcasecmp(vk_hex, api->signing_pin) == 0;
#endif
    if (!vk_matches) {
        na_set_error(api, "init response signing key does not match pinned key -- refusing handshake", "untrusted_response");
        na_json_free(resp);
        EVP_PKEY_free(eph);
        return;
    }

    uint8_t vk_raw[32], k2_raw[32];
    if (strlen(k2_hex) != 64 || !na_hex_decode(k2_hex, k2_raw, 32) || !na_hex_decode(api->signing_pin, vk_raw, 32)) {
        na_set_error(api, "invalid k2/vk length", "untrusted_response");
        na_json_free(resp);
        EVP_PKEY_free(eph);
        return;
    }

    size_t sig_len = 0;
    uint8_t* sig_bytes = na_base64_decode(sig_b64, &sig_len);
    if (!sig_bytes) { na_set_error(api, "invalid signature encoding", "untrusted_response"); na_json_free(resp); EVP_PKEY_free(eph); return; }

    char sig_data[256];
    int sig_data_len = snprintf(sig_data, sizeof(sig_data), "%s.%s.%lld", k2_hex, vk_hex, (long long)ts);
    bool sig_ok = sig_data_len > 0 && (size_t)sig_data_len < sizeof(sig_data) &&
                  na_ed25519_verify(vk_raw, (const uint8_t*)sig_data, (size_t)sig_data_len, sig_bytes, sig_len);
    free(sig_bytes);
    if (!sig_ok) {
        na_set_error(api, "init response signature verification failed -- refusing to trust this handshake", "untrusted_response");
        na_json_free(resp);
        EVP_PKEY_free(eph);
        return;
    }

    time_t now = time(NULL);
    int64_t now_ms = (int64_t)now * 1000;
    int64_t skew = now_ms - ts;
    if (skew < 0) skew = -skew;
    if (skew > NA_TS_SKEW_MS) {
        na_set_error(api, "init response timestamp is stale/future -- possible replay", "untrusted_response");
        na_json_free(resp);
        EVP_PKEY_free(eph);
        return;
    }

    uint8_t secret[32];
    bool dh_ok = na_x25519_dh(eph, k2_raw, secret);
    if (!dh_ok) { na_set_error(api, "X25519 DH with server ephemeral key failed", "init_failed"); na_json_free(resp); EVP_PKEY_free(eph); return; }

    bool kdf_ok = na_hkdf_sha256(secret, 32, (const uint8_t*)"nightauth-v1-e2e", strlen("nightauth-v1-e2e"), api->shared_key, 32);
    OPENSSL_cleanse(secret, sizeof(secret));
    na_json_free(resp);
    EVP_PKEY_free(eph);
    api->client_ephemeral = NULL;
    if (!kdf_ok) { na_set_error(api, "session key derivation failed", "init_failed"); return; }

    snprintf(api->session_id, sizeof(api->session_id), "%s", session_id);
    api->initialized = true;
    na_set_success(api, "Initialized successfully.");
}

static void na_clear_session(NightAuthApi* api);

static void na_resolve_target_path(const char* api_url, const char* path, char* out, size_t out_len) {
    if (!out || out_len == 0) return;
    out[0] = '\0';
    const char* base_path = "";
    if (api_url) {
        const char* p = strstr(api_url, "://");
        if (p) {
            p += 3;
            const char* slash = strchr(p, '/');
            if (slash && *slash) {
                base_path = slash;
            }
        }
    }
    if (base_path[0] != '\0') {
        size_t bplen = strlen(base_path);
        if (strncmp(path, base_path, bplen) == 0 && (path[bplen] == '/' || path[bplen] == '\0')) {
            snprintf(out, out_len, "%s", path);
        } else {
            snprintf(out, out_len, "%s%s", base_path, path);
        }
    } else {
        snprintf(out, out_len, "%s", path ? path : "");
    }
}

static char* na_send_encrypted_once(NightAuthApi* api, const char* path, const char* payload_json) {
    if (!api || !api->initialized) { na_set_error(api, "NightAuth must be initialized before calling API methods.", "not_initialized"); return NULL; }

#if defined(_WIN32)
    time_t now = time(NULL);
    if (now == (time_t)-1) {
        na_set_error(api, "system clock unavailable", "protocol_error");
        return NULL;
    }
    int64_t ts = (int64_t)now * 1000;
#else
    struct timespec tv;
    if (clock_gettime(CLOCK_REALTIME, &tv) != 0) {
        na_set_error(api, "system clock unavailable", "protocol_error");
        return NULL;
    }
    int64_t ts = (int64_t)tv.tv_sec * 1000 + tv.tv_nsec / 1000000;
#endif
    uint8_t rid_rand[8];
    if (RAND_bytes(rid_rand, sizeof(rid_rand)) != 1) {
        OPENSSL_cleanse(rid_rand, sizeof(rid_rand));
        na_set_error(api, "cryptographic random generator failed", "crypto_error");
        return NULL;
    }
    char rid_hex[17];
    na_hex_encode(rid_rand, 8, rid_hex);
    char rid[48];
    snprintf(rid, sizeof(rid), "rid-%lld-%s", (long long)ts, rid_hex);

    char target_path[128];
    na_resolve_target_path(api->api_url, path, target_path, sizeof(target_path));
    char aad[256];
    int aad_len = snprintf(aad, sizeof(aad), "POST:%s:%s:%s:%lld", target_path, api->session_id, rid, (long long)ts);
    if (aad_len <= 0 || (size_t)aad_len >= sizeof(aad)) { na_set_error(api, "aad too long", "protocol_error"); return NULL; }

    uint8_t nonce[12];
    uint8_t* ct = NULL;
    size_t ct_len = 0;
    if (!na_aes_gcm_encrypt(api->shared_key, (const uint8_t*)payload_json, strlen(payload_json), (const uint8_t*)aad, (size_t)aad_len, nonce, &ct, &ct_len)) {
        na_set_error(api, "encryption failed", "protocol_error");
        return NULL;
    }
    char* ct_b64 = na_base64_encode(ct, ct_len);
    char* nonce_b64 = na_base64_encode(nonce, 12);
    free(ct);
    if (!ct_b64 || !nonce_b64) { free(ct_b64); free(nonce_b64); na_set_error(api, "encoding failed", "protocol_error"); return NULL; }

    NaBuf envelope; na_buf_init(&envelope);
    na_buf_append_str(&envelope, "{\"id\":");
    na_json_escape_append(&envelope, api->session_id);
    na_buf_append_str(&envelope, ",\"ct\":");
    na_json_escape_append(&envelope, ct_b64);
    na_buf_append_str(&envelope, ",\"n\":");
    na_json_escape_append(&envelope, nonce_b64);
    na_buf_append_str(&envelope, ",\"rid\":");
    na_json_escape_append(&envelope, rid);
    {
        char tsbuf[32];
        snprintf(tsbuf, sizeof(tsbuf), ",\"ts\":%lld}", (long long)ts);
        na_buf_append_str(&envelope, tsbuf);
    }
    free(ct_b64);
    free(nonce_b64);

    NaHttpResp raw;
    bool http_ok = na_post_raw(api, path, (const char*)envelope.data, &raw);
    na_buf_free(&envelope);
    if (!http_ok) { na_set_error(api, "network request failed", na_transport_code(&raw)); return NULL; }

    long http_status = raw.status;
    NaJson* resp = na_json_parse((const char*)raw.buf.data, raw.buf.len);
    na_buf_free(&raw.buf);
    if (!resp || resp->type != NA_JSON_OBJ) { na_json_free(resp); na_set_error(api, "Response body is not a JSON object.", "invalid_response"); return NULL; }

    NaJson *ctv = na_json_obj_get(resp, "ct"), *nv = na_json_obj_get(resp, "n");
    if ((!ctv || ctv->type != NA_JSON_STR || !nv || nv->type != NA_JSON_STR) && (http_status < 200 || http_status >= 300)) {

        const char* code = na_json_get_str(resp, "error", "");
        if (!code[0] && http_status == 428) code = "handshake_required";
        na_set_error(api, na_json_get_str(resp, "message", "Request refused by the server."), code[0] ? code : "http_error");
        if (http_status == 428 || strcmp(code, "handshake_required") == 0) na_clear_session(api);
        na_json_free(resp);
        return NULL;
    }
    if (!ctv || ctv->type != NA_JSON_STR || !nv || nv->type != NA_JSON_STR) {

        na_set_error(api, "response was not encrypted (plaintext success is never valid)", "downgrade_rejected");
        na_json_free(resp);
        return NULL;
    }
    NaJson *idv = na_json_obj_get(resp, "id"), *ridv = na_json_obj_get(resp, "rid"), *tsv = na_json_obj_get(resp, "ts");
    const char* id_resp = (idv && idv->type == NA_JSON_STR) ? idv->string : NULL;
    const char* rid_resp = (ridv && ridv->type == NA_JSON_STR) ? ridv->string : NULL;
    int64_t ts_resp = (tsv && tsv->type == NA_JSON_NUM) ? (int64_t)tsv->number : INT64_MIN;
    if (!id_resp || strcmp(id_resp, api->session_id) != 0 || !rid_resp || strcmp(rid_resp, rid) != 0 || ts_resp != ts) {
        na_set_error(api, "response id/rid/ts does not match the request - binding violation, discarding", "binding_violation");
        na_json_free(resp);
        return NULL;
    }

    char resp_aad[256];
    int resp_aad_len = snprintf(resp_aad, sizeof(resp_aad), "v1:res:%s:%s:%lld", id_resp, rid_resp, (long long)ts_resp);
    size_t ct2_len = 0, nonce2_len = 0;
    uint8_t* ct2 = na_base64_decode(ctv->string, &ct2_len);
    uint8_t* nonce2 = na_base64_decode(nv->string, &nonce2_len);
    char* out = NULL;
    if (ct2 && nonce2 && nonce2_len == 12 && resp_aad_len > 0 && (size_t)resp_aad_len < sizeof(resp_aad)) {
        uint8_t* pt = NULL;
        size_t pt_len = 0;
        if (na_aes_gcm_decrypt(api->shared_key, nonce2, ct2, ct2_len, (const uint8_t*)resp_aad, (size_t)resp_aad_len, &pt, &pt_len)) {
            out = (char*)malloc(pt_len + 1);
            if (out) { memcpy(out, pt, pt_len); out[pt_len] = '\0'; }
            free(pt);
        }
    }
    free(ct2);
    free(nonce2);
    na_json_free(resp);
    if (!out) { na_set_error(api, "response decryption failed (tampered or invalid)", "decrypt_failed"); return NULL; }
    return out;
}

static bool na_is_handshake_expired(const char* error, const char* message) {
    if (error && (strcmp(error, "handshake_required") == 0 || strcmp(error, "session_invalid") == 0 ||
                  strcmp(error, "unauthenticated") == 0))
        return true;
    if (!message) return false;
    char low[512];
    size_t i = 0;
    for (; message[i] && i < sizeof(low) - 1; i++) low[i] = (char)tolower((unsigned char)message[i]);
    low[i] = '\0';
    return strstr(low, "handshake") != NULL || strstr(low, "initialize app") != NULL;
}

static char* na_send_encrypted_ex(NightAuthApi* api, const char* path, const char* payload_json, bool allow_reinit) {

    if (api && allow_reinit && !api->initialized) {
        nightauth_init(api);
        if (!api->initialized) return NULL;
    }
    char* out = na_send_encrypted_once(api, path, payload_json);
    if (!allow_reinit || !api) return out;
    bool expired = false;
    if (!out) {
        expired = !api->response.success && na_is_handshake_expired(api->response.error, NULL);
    } else {
        NaJson* v = na_json_parse(out, strlen(out));
        if (v && v->type == NA_JSON_OBJ) {
            NaJson* s = na_json_obj_get(v, "success");
            bool ok = s && s->type == NA_JSON_BOOL && s->boolean;
            expired = !ok && na_is_handshake_expired(na_json_get_str(v, "error", ""), na_json_get_str(v, "message", ""));
        }
        na_json_free(v);
    }
    if (!expired) return out;
    NightAuthResponse first = api->response;
    nightauth_init(api);
    if (!api->initialized) {
        api->response = first;
        return out;
    }
    free(out);
    return na_send_encrypted_once(api, path, payload_json);
}

static char* na_send_encrypted(NightAuthApi* api, const char* path, const char* payload_json) {
    return na_send_encrypted_ex(api, path, payload_json, true);
}

static bool na_body_is_success(const char* json, NaJson** out_parsed) {
    NaJson* v = na_json_parse(json, strlen(json));
    if (!v || v->type != NA_JSON_OBJ) { na_json_free(v); return false; }
    NaJson* s = na_json_obj_get(v, "success");
    bool ok = s && s->type == NA_JSON_BOOL && s->boolean;
    if (out_parsed) *out_parsed = v; else na_json_free(v);
    return ok;
}

static void na_format_timestamp_or_str(const NaJson* obj, const char* k1, const char* k2, char* out, size_t out_cap) {
    if (!out || out_cap == 0) return;
    out[0] = '\0';
    NaJson* v = na_json_obj_get(obj, k1);
    if ((!v || v->type == NA_JSON_NULL) && k2) v = na_json_obj_get(obj, k2);
    if (!v) return;
    if (v->type == NA_JSON_STR && v->string && v->string[0]) {
        bool all_digits = true;
        for (const char* p = v->string; *p; ++p) {
            if (!isdigit((unsigned char)*p)) { all_digits = false; break; }
        }
        if (all_digits && strlen(v->string) >= 9) {
            time_t ts = (time_t)atoll(v->string);
            struct tm* tm_info = gmtime(&ts);
            if (tm_info) {
                strftime(out, out_cap, "%Y-%m-%d %H:%M UTC", tm_info);
                return;
            }
        }
        strncpy(out, v->string, out_cap - 1);
        out[out_cap - 1] = '\0';
    } else if (v->type == NA_JSON_NUM && v->number > 0) {
        time_t ts = (time_t)(int64_t)v->number;
        struct tm* tm_info = gmtime(&ts);
        if (tm_info) {
            strftime(out, out_cap, "%Y-%m-%d %H:%M UTC", tm_info);
        } else {
            snprintf(out, out_cap, "%lld", (long long)v->number);
        }
    }
}

static void na_populate_user_from_response(NightAuthApi* api, NaJson* resp) {
    NaJson* u = na_json_obj_get(resp, "user");
    if (!u || u->type != NA_JSON_OBJ) u = na_json_obj_get(resp, "info");
    if (!u || u->type != NA_JSON_OBJ) return;
    memset(&api->user, 0, sizeof(api->user));
    strncpy(api->user.username, na_json_get_str(u, "username", ""), sizeof(api->user.username) - 1);
    strncpy(api->user.email, na_json_get_str(u, "email", ""), sizeof(api->user.email) - 1);
    strncpy(api->user.hwid, na_json_get_str(u, "hwid", ""), sizeof(api->user.hwid) - 1);
    strncpy(api->user.ip, na_json_get_str(u, "ip", ""), sizeof(api->user.ip) - 1);
    strncpy(api->user.status, na_json_get_str(u, "status", "active"), sizeof(api->user.status) - 1);
    na_format_timestamp_or_str(u, "createdate", "createdAt", api->user.created_at, sizeof(api->user.created_at));
    na_format_timestamp_or_str(u, "lastlogin", "lastLoginAt", api->user.last_login_at, sizeof(api->user.last_login_at));
    NaJson* subs = na_json_obj_get(u, "subscriptions");
    if (subs && subs->type == NA_JSON_ARR) {
        int count = 0;
        for (size_t i = 0; i < subs->count && count < 16; i++) {
            NaJson* item = subs->items[i];
            if (!item || item->type != NA_JSON_OBJ) continue;
            NightAuthSubscriptionData* sd = &api->user.subscriptions[count];
            strncpy(sd->subscription, na_json_get_str(item, "subscription", ""), sizeof(sd->subscription) - 1);
            strncpy(sd->key, na_json_get_str(item, "key", ""), sizeof(sd->key) - 1);
            na_format_timestamp_or_str(item, "expiry", "expiresAt", sd->expiry, sizeof(sd->expiry));
            NaJson* tl = na_json_obj_get(item, "timeleft");
            if (!tl) tl = na_json_obj_get(item, "timeLeft");
            sd->time_left = (tl && tl->type == NA_JSON_NUM) ? (int64_t)tl->number : 0;
            count++;
        }
        api->user.subscription_count = count;
    }
}

bool nightauth_login(NightAuthApi* api, const char* username, const char* password, const char* totp_code) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_buf_append_str(&body, "{\"apikey\":"); na_json_escape_append(&body, api->api_key);
    na_buf_append_str(&body, ",\"name\":"); na_json_escape_append(&body, api->name);
    if (api->token[0]) { na_buf_append_str(&body, ",\"token\":"); na_json_escape_append(&body, api->token); }
    na_buf_append_str(&body, ",\"version\":"); na_json_escape_append(&body, api->version);
    na_buf_append_str(&body, ",\"username\":"); na_json_escape_append(&body, username ? username : "");
    na_buf_append_str(&body, ",\"password\":"); na_json_escape_append(&body, password ? password : "");
    char hwid[128]; na_client_hwid(api, hwid, sizeof(hwid));
    na_buf_append_str(&body, ",\"hwid\":"); na_json_escape_append(&body, hwid);
    if (totp_code && totp_code[0]) { na_buf_append_str(&body, ",\"totpCode\":"); na_json_escape_append(&body, totp_code); }
    na_buf_append_str(&body, "}");

    char* resp_json = na_send_encrypted(api, "/login", (const char*)body.data);
    na_buf_free(&body);
    if (!resp_json) return false;

    NaJson* parsed = NULL;
    bool ok = na_body_is_success(resp_json, &parsed);
    if (!ok) {
        na_set_error(api, parsed ? na_json_get_str(parsed, "message", "Login failed") : "Login failed",
                     parsed ? na_json_get_str(parsed, "error", "login_failed") : "login_failed");
        na_json_free(parsed);
        free(resp_json);
        return false;
    }
    na_populate_user_from_response(api, parsed);
    na_json_free(parsed);
    free(resp_json);
    na_set_success(api, "Logged in successfully.");
    return true;
}

bool nightauth_register(NightAuthApi* api, const char* username, const char* password, const char* key, const char* email) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_buf_append_str(&body, "{\"apikey\":"); na_json_escape_append(&body, api->api_key);
    na_buf_append_str(&body, ",\"name\":"); na_json_escape_append(&body, api->name);
    if (api->token[0]) { na_buf_append_str(&body, ",\"token\":"); na_json_escape_append(&body, api->token); }
    na_buf_append_str(&body, ",\"version\":"); na_json_escape_append(&body, api->version);
    na_buf_append_str(&body, ",\"username\":"); na_json_escape_append(&body, username ? username : "");
    na_buf_append_str(&body, ",\"password\":"); na_json_escape_append(&body, password ? password : "");
    na_buf_append_str(&body, ",\"key\":"); na_json_escape_append(&body, key ? key : "");

    if (email && email[0]) { na_buf_append_str(&body, ",\"email\":"); na_json_escape_append(&body, email); }
    char hwid[128]; na_client_hwid(api, hwid, sizeof(hwid));
    na_buf_append_str(&body, ",\"hwid\":"); na_json_escape_append(&body, hwid);
    na_buf_append_str(&body, "}");

    char* resp_json = na_send_encrypted(api, "/register", (const char*)body.data);
    na_buf_free(&body);
    if (!resp_json) return false;

    NaJson* parsed = NULL;
    bool ok = na_body_is_success(resp_json, &parsed);
    if (!ok) {
        na_set_error(api, parsed ? na_json_get_str(parsed, "message", "Registration failed") : "Registration failed",
                     parsed ? na_json_get_str(parsed, "error", "registration_failed") : "registration_failed");
        na_json_free(parsed);
        free(resp_json);
        return false;
    }
    na_populate_user_from_response(api, parsed);
    na_json_free(parsed);
    free(resp_json);
    na_set_success(api, "Registered successfully.");
    return true;
}

bool nightauth_license(NightAuthApi* api, const char* key) {
    return nightauth_license_with_totp(api, key, NULL);
}

bool nightauth_license_with_totp(NightAuthApi* api, const char* key, const char* totp_code) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_buf_append_str(&body, "{\"apikey\":"); na_json_escape_append(&body, api->api_key);
    na_buf_append_str(&body, ",\"name\":"); na_json_escape_append(&body, api->name);
    if (api->token[0]) { na_buf_append_str(&body, ",\"token\":"); na_json_escape_append(&body, api->token); }
    na_buf_append_str(&body, ",\"version\":"); na_json_escape_append(&body, api->version);
    na_buf_append_str(&body, ",\"key\":"); na_json_escape_append(&body, key ? key : "");
    char hwid[128]; na_client_hwid(api, hwid, sizeof(hwid));
    na_buf_append_str(&body, ",\"hwid\":"); na_json_escape_append(&body, hwid);
    if (totp_code && totp_code[0]) { na_buf_append_str(&body, ",\"totpCode\":"); na_json_escape_append(&body, totp_code); }
    na_buf_append_str(&body, "}");

    char* resp_json = na_send_encrypted(api, "/license", (const char*)body.data);
    na_buf_free(&body);
    if (!resp_json) return false;

    NaJson* parsed = NULL;
    bool ok = na_body_is_success(resp_json, &parsed);
    if (!ok) {
        na_set_error(api, parsed ? na_json_get_str(parsed, "message", "License activation failed") : "License activation failed",
                     parsed ? na_json_get_str(parsed, "error", "license_failed") : "license_failed");
        na_json_free(parsed);
        free(resp_json);
        return false;
    }
    na_populate_user_from_response(api, parsed);
    na_json_free(parsed);
    free(resp_json);
    na_set_success(api, "License verified successfully.");
    return true;
}

static void na_clear_session(NightAuthApi* api) {
    OPENSSL_cleanse(api->shared_key, sizeof(api->shared_key));
    memset(api->session_id, 0, sizeof(api->session_id));
    memset(&api->user, 0, sizeof(api->user));
    memset(&api->app, 0, sizeof(api->app));
    api->initialized = false;
}

static bool na_copy_bounded(char* dst, size_t cap, const char* src) {
    size_t n = strlen(src);
    if (!dst || cap == 0 || n >= cap) {
        if (dst && cap) dst[0] = '\0';
        return false;
    }
    memcpy(dst, src, n + 1);
    return true;
}

static int64_t na_json_get_int(const NaJson* obj, const char* key) {
    NaJson* v = na_json_obj_get(obj, key);
    return (v && v->type == NA_JSON_NUM) ? (int64_t)v->number : 0;
}

static void na_begin_body(NaBuf* b, const NightAuthApi* api) {
    na_buf_append_str(b, "{\"apikey\":");
    na_json_escape_append(b, api->api_key);
    na_buf_append_str(b, ",\"name\":");
    na_json_escape_append(b, api->name);
    if (api->token[0]) {
        na_buf_append_str(b, ",\"token\":");
        na_json_escape_append(b, api->token);
    }
}

static void na_add_str(NaBuf* b, const char* key, const char* value) {
    na_buf_append_str(b, ",\"");
    na_buf_append_str(b, key);
    na_buf_append_str(b, "\":");
    na_json_escape_append(b, value ? value : "");
}

static NaJson* na_call_json(NightAuthApi* api, const char* path, NaBuf* body,
                            const char* fail_msg, const char* fail_code) {
    char* resp_json = na_send_encrypted(api, path, (const char*)body->data);
    na_buf_free(body);
    if (!resp_json) return NULL;
    NaJson* parsed = NULL;
    bool ok = na_body_is_success(resp_json, &parsed);
    free(resp_json);
    if (!ok) {
        na_set_error(api,
                     parsed ? na_json_get_str(parsed, "message", fail_msg) : fail_msg,
                     parsed ? na_json_get_str(parsed, "error", fail_code) : fail_code);
        na_json_free(parsed);
        return NULL;
    }
    return parsed;
}

static bool na_ticket_charset_ok(const char* ticket) {
    if (!ticket || !ticket[0]) return false;
    for (const char* p = ticket; *p; p++) {
        if (!(isalnum((unsigned char)*p) || *p == '-' || *p == '_')) return false;
    }
    return true;
}

#define NA_MAXBITS 15

typedef struct {
    const uint8_t* src;
    size_t src_len;
    size_t pos;
    uint32_t bitbuf;
    int bitcnt;
    uint8_t* out;
    size_t out_cap;
    size_t out_len;
    bool too_large;
} NaInf;

typedef struct {
    short count[NA_MAXBITS + 1];
    short symbol[320];
} NaHuff;

static const short NA_LBASE[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
                                   35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static const short NA_LEXT[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                                  3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static const short NA_DBASE[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
                                   257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193,
                                   12289, 16385, 24577};
static const short NA_DEXT[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                                  7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
static const short NA_CL_ORDER[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

static uint32_t na_crc32(const uint8_t* p, size_t n) {
    uint32_t c = 0xffffffffu;
    for (size_t i = 0; i < n; i++) {
        c ^= p[i];
        for (int k = 0; k < 8; k++) c = (c >> 1) ^ (0xedb88320u & (0u - (c & 1u)));
    }
    return c ^ 0xffffffffu;
}

static int na_inf_bits(NaInf* s, int need) {
    uint32_t val = s->bitbuf;
    while (s->bitcnt < need) {
        if (s->pos >= s->src_len) return -1;
        val |= (uint32_t)s->src[s->pos++] << s->bitcnt;
        s->bitcnt += 8;
    }
    s->bitbuf = val >> need;
    s->bitcnt -= need;
    return (int)(val & ((1u << need) - 1u));
}

static int na_huff_build(NaHuff* h, const short* length, int n) {
    for (int i = 0; i <= NA_MAXBITS; i++) h->count[i] = 0;
    for (int sym = 0; sym < n; sym++) h->count[length[sym]]++;
    if (h->count[0] == n) return 0;
    int left = 1;
    for (int len = 1; len <= NA_MAXBITS; len++) {
        left <<= 1;
        left -= h->count[len];
        if (left < 0) return left;
    }
    short offs[NA_MAXBITS + 1];
    offs[1] = 0;
    for (int len = 1; len < NA_MAXBITS; len++) offs[len + 1] = (short)(offs[len] + h->count[len]);
    for (int sym = 0; sym < n; sym++) {
        if (length[sym] != 0) h->symbol[offs[length[sym]]++] = (short)sym;
    }
    return left;
}

static int na_huff_decode(NaInf* s, const NaHuff* h) {
    int code = 0, first = 0, index = 0;
    for (int len = 1; len <= NA_MAXBITS; len++) {
        int bit = na_inf_bits(s, 1);
        if (bit < 0) return -1;
        code |= bit;
        int count = h->count[len];
        if (code - count < first) return h->symbol[index + (code - first)];
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -2;
}

static int na_inf_codes(NaInf* s, const NaHuff* lencode, const NaHuff* distcode) {
    for (;;) {
        int sym = na_huff_decode(s, lencode);
        if (sym < 0) return -1;
        if (sym < 256) {
            if (s->out_len >= s->out_cap) { s->too_large = true; return -1; }
            s->out[s->out_len++] = (uint8_t)sym;
        } else if (sym == 256) {
            return 0;
        } else {
            sym -= 257;
            if (sym >= 29) return -1;
            int extra = na_inf_bits(s, NA_LEXT[sym]);
            if (extra < 0) return -1;
            size_t len = (size_t)NA_LBASE[sym] + (size_t)extra;
            sym = na_huff_decode(s, distcode);
            if (sym < 0 || sym >= 30) return -1;
            extra = na_inf_bits(s, NA_DEXT[sym]);
            if (extra < 0) return -1;
            size_t dist = (size_t)NA_DBASE[sym] + (size_t)extra;
            if (dist > s->out_len) return -1;
            if (len > s->out_cap - s->out_len) { s->too_large = true; return -1; }
            for (size_t i = 0; i < len; i++) {
                s->out[s->out_len] = s->out[s->out_len - dist];
                s->out_len++;
            }
        }
    }
}

static int na_inf_stored(NaInf* s) {

    s->bitbuf = 0;
    s->bitcnt = 0;
    if (s->pos + 4 > s->src_len) return -1;
    unsigned len = s->src[s->pos] | ((unsigned)s->src[s->pos + 1] << 8);
    unsigned nlen = s->src[s->pos + 2] | ((unsigned)s->src[s->pos + 3] << 8);
    s->pos += 4;
    if (len != (~nlen & 0xffffu)) return -1;
    if (len > s->src_len - s->pos) return -1;
    if (len > s->out_cap - s->out_len) { s->too_large = true; return -1; }
    memcpy(s->out + s->out_len, s->src + s->pos, len);
    s->out_len += len;
    s->pos += len;
    return 0;
}

static int na_inf_fixed(NaInf* s) {
    short lengths[320];
    NaHuff lcode, dcode;
    int i = 0;
    for (; i < 144; i++) lengths[i] = 8;
    for (; i < 256; i++) lengths[i] = 9;
    for (; i < 280; i++) lengths[i] = 7;
    for (; i < 288; i++) lengths[i] = 8;
    na_huff_build(&lcode, lengths, 288);
    for (i = 0; i < 30; i++) lengths[i] = 5;
    na_huff_build(&dcode, lengths, 30);
    return na_inf_codes(s, &lcode, &dcode);
}

static int na_inf_dynamic(NaInf* s) {
    int nlen = na_inf_bits(s, 5);
    if (nlen < 0) return -1;
    nlen += 257;
    int ndist = na_inf_bits(s, 5);
    if (ndist < 0) return -1;
    ndist += 1;
    int ncode = na_inf_bits(s, 4);
    if (ncode < 0) return -1;
    ncode += 4;
    if (nlen > 286 || ndist > 30) return -1;

    short lengths[320];
    memset(lengths, 0, sizeof(lengths));
    for (int i = 0; i < ncode; i++) {
        int v = na_inf_bits(s, 3);
        if (v < 0) return -1;
        lengths[NA_CL_ORDER[i]] = (short)v;
    }
    NaHuff lencode;
    if (na_huff_build(&lencode, lengths, 19) != 0) return -1;

    int index = 0;
    while (index < nlen + ndist) {
        int sym = na_huff_decode(s, &lencode);
        if (sym < 0) return -1;
        if (sym < 16) {
            lengths[index++] = (short)sym;
            continue;
        }
        short len = 0;
        int extra, rep;
        if (sym == 16) {
            if (index == 0) return -1;
            len = lengths[index - 1];
            extra = na_inf_bits(s, 2);
            if (extra < 0) return -1;
            rep = 3 + extra;
        } else if (sym == 17) {
            extra = na_inf_bits(s, 3);
            if (extra < 0) return -1;
            rep = 3 + extra;
        } else {
            extra = na_inf_bits(s, 7);
            if (extra < 0) return -1;
            rep = 11 + extra;
        }
        if (index + rep > nlen + ndist) return -1;
        while (rep-- > 0) lengths[index++] = len;
    }
    if (lengths[256] == 0) return -1;

    NaHuff lcode, dcode;
    int err = na_huff_build(&lcode, lengths, nlen);
    if (err < 0 || (err > 0 && nlen - lcode.count[0] != 1)) return -1;
    err = na_huff_build(&dcode, lengths + nlen, ndist);
    if (err < 0 || (err > 0 && ndist - dcode.count[0] != 1)) return -1;
    return na_inf_codes(s, &lcode, &dcode);
}

static int na_inflate_raw(NaInf* s) {
    int last;
    do {
        last = na_inf_bits(s, 1);
        if (last < 0) return -1;
        int type = na_inf_bits(s, 2);
        if (type < 0) return -1;
        int err;
        if (type == 0) err = na_inf_stored(s);
        else if (type == 1) err = na_inf_fixed(s);
        else if (type == 2) err = na_inf_dynamic(s);
        else err = -1;
        if (err) return -1;
    } while (!last);
    return 0;
}

static bool na_gunzip_ex(const uint8_t* in, size_t in_len, uint8_t* out, size_t out_cap,
                         size_t* out_len, bool* too_large) {
    if (too_large) *too_large = false;
    if (!in || !out_len || (!out && out_cap > 0)) return false;
    *out_len = 0;
    if (in_len < 18 || in[0] != 0x1f || in[1] != 0x8b || in[2] != 8) return false;
    uint8_t flg = in[3];
    if (flg & 0xe0) return false;
    size_t pos = 10;
    if (flg & 0x04) {
        if (in_len - pos < 2) return false;
        size_t xlen = in[pos] | ((size_t)in[pos + 1] << 8);
        pos += 2;
        if (in_len - pos < xlen) return false;
        pos += xlen;
    }
    if (flg & 0x08) {
        while (pos < in_len && in[pos] != 0) pos++;
        if (pos >= in_len) return false;
        pos++;
    }
    if (flg & 0x10) {
        while (pos < in_len && in[pos] != 0) pos++;
        if (pos >= in_len) return false;
        pos++;
    }
    if (flg & 0x02) pos += 2;
    if (pos > in_len || in_len - pos < 8) return false;

    NaInf s;
    memset(&s, 0, sizeof(s));
    s.src = in;
    s.src_len = in_len - 8;
    s.pos = pos;
    s.out = out;
    s.out_cap = out_cap;
    if (na_inflate_raw(&s) != 0) {
        if (too_large) *too_large = s.too_large;
        return false;
    }
    const uint8_t* tr = in + in_len - 8;
    uint32_t crc_expected = (uint32_t)tr[0] | ((uint32_t)tr[1] << 8) | ((uint32_t)tr[2] << 16) | ((uint32_t)tr[3] << 24);
    uint32_t isize = (uint32_t)tr[4] | ((uint32_t)tr[5] << 8) | ((uint32_t)tr[6] << 16) | ((uint32_t)tr[7] << 24);
    if (na_crc32(out, s.out_len) != crc_expected) return false;
    if (isize != (uint32_t)s.out_len) return false;
    *out_len = s.out_len;
    return true;
}

#define NA_SSE_MAX_LINE (64 * 1024)
#define NA_SSE_MAX_EVENT (64 * 1024)

typedef struct {
    NaBuf line;
    NaBuf data;
    char last_id[64];
    bool stop;
} NaSse;

static void na_sse_init(NaSse* s) {
    na_buf_init(&s->line);
    na_buf_init(&s->data);
    s->last_id[0] = '\0';
    s->stop = false;
}

static void na_sse_free(NaSse* s) {
    na_buf_free(&s->line);
    na_buf_free(&s->data);
}

static void na_sse_dispatch(NaSse* s, NightAuthEventCallback cb, void* user) {
    if (s->data.len == 0) return;
    s->data.len -= 1;
    s->data.data[s->data.len] = '\0';
    bool keep = cb((const char*)s->data.data, user);
    s->data.len = 0;
    s->data.data[0] = '\0';
    if (!keep) s->stop = true;
}

static bool na_sse_line(NaSse* s, const char* line, size_t len, NightAuthEventCallback cb, void* user) {
    if (len == 0) {
        na_sse_dispatch(s, cb, user);
        return true;
    }
    if (line[0] == ':') return true;
    const char* colon = memchr(line, ':', len);
    size_t name_len = colon ? (size_t)(colon - line) : len;
    const char* value = colon ? colon + 1 : line + len;
    size_t value_len = colon ? len - name_len - 1 : 0;
    if (value_len > 0 && value[0] == ' ') { value++; value_len--; }
    if (name_len == 4 && memcmp(line, "data", 4) == 0) {
        if (s->data.len + value_len + 1 > NA_SSE_MAX_EVENT) return false;
        if (!na_buf_append(&s->data, value, value_len)) return false;
        if (!na_buf_append(&s->data, "\n", 1)) return false;
    } else if (name_len == 2 && memcmp(line, "id", 2) == 0) {
        if (value_len < sizeof(s->last_id)) {
            memcpy(s->last_id, value, value_len);
            s->last_id[value_len] = '\0';
        }
    }
    return true;
}

static bool na_sse_feed(NaSse* s, const char* chunk, size_t n, NightAuthEventCallback cb, void* user) {
    for (size_t i = 0; i < n; i++) {
        if (s->stop) return true;
        char c = chunk[i];
        if (c == '\n') {
            size_t len = s->line.len;
            if (len > 0 && s->line.data[len - 1] == '\r') len--;
            if (!na_sse_line(s, (const char*)s->line.data, len, cb, user)) return false;
            s->line.len = 0;
            if (s->line.data) s->line.data[0] = '\0';
        } else {
            if (s->line.len >= NA_SSE_MAX_LINE) return false;
            if (!na_buf_append(&s->line, &c, 1)) return false;
        }
    }
    return true;
}

void nightauth_events_cancel(NightAuthApi* api) {
    if (api) atomic_store(&api->events_cancel, 1);
}

bool nightauth_logout(NightAuthApi* api) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_buf_append_str(&body, "}");

    char* resp_json = na_send_encrypted_ex(api, "/logout", (const char*)body.data, true);
    na_buf_free(&body);
    bool confirmed = false;
    char code[128] = "";
    char message[512] = "";
    if (resp_json) {
        NaJson* parsed = NULL;
        confirmed = na_body_is_success(resp_json, &parsed);
        if (!confirmed && parsed && parsed->type == NA_JSON_OBJ) {
            snprintf(code, sizeof(code), "%s", na_json_get_str(parsed, "error", ""));
            snprintf(message, sizeof(message), "%s", na_json_get_str(parsed, "message", ""));
        }
        na_json_free(parsed);
        free(resp_json);
    } else {

        snprintf(code, sizeof(code), "%s", api->response.error);
        snprintf(message, sizeof(message), "%s", api->response.message);
    }

    na_clear_session(api);
    if (confirmed) {
        na_set_success(api, "Logged out; the server confirmed the session was revoked.");
        return true;
    }

    static const char* const kept[] = { "network_error", "timeout", "invalid_response", "response_too_large",
                                        "handshake_required", "binding_violation", "untrusted_response", NULL };
    for (int i = 0; kept[i]; i++) {
        if (strcmp(code, kept[i]) == 0) {
            na_set_error(api, message[0] ? message : "Logout failed", code);
            return false;
        }
    }
    na_set_error(api, "Local session cleared, but the server did not confirm logout; the session may still be valid server-side.",
                 "logout_unconfirmed");
    return false;
}

bool nightauth_upgrade(NightAuthApi* api, const char* username, const char* key) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    if (username && username[0]) na_add_str(&body, "username", username);
    na_add_str(&body, "key", key);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/upgrade", &body, "Upgrade failed", "upgrade_failed");
    if (!p) return false;
    na_json_free(p);
    na_set_success(api, "License redeemed; subscription updated.");
    return true;
}

bool nightauth_heartbeat(NightAuthApi* api) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "version", api->version);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/heartbeat", &body, "Heartbeat failed", "heartbeat_failed");
    if (!p) return false;
    na_json_free(p);
    na_set_success(api, "Heartbeat accepted.");
    return true;
}

bool nightauth_check(NightAuthApi* api) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "version", api->version);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/check", &body, "Session check failed", "session_invalid");
    if (!p) return false;
    na_populate_user_from_response(api, p);
    na_json_free(p);
    na_set_success(api, "Session is valid.");
    return true;
}

bool nightauth_check_blacklist(NightAuthApi* api, const char* hwid, bool* out_blacklisted) {
    if (out_blacklisted) *out_blacklisted = false;
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    if (hwid && hwid[0]) na_add_str(&body, "hwid", hwid);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/check-blacklist", &body, "Blacklist check failed", "blacklist_check_failed");
    if (!p) return false;
    NaJson* flag = na_json_obj_get(p, "blacklisted");
    if (!flag || flag->type != NA_JSON_BOOL) {
        na_json_free(p);
        na_set_error(api, "blacklist response is missing the blacklisted flag", "protocol_error");
        return false;
    }
    if (out_blacklisted) *out_blacklisted = flag->boolean != 0;
    na_json_free(p);
    na_set_success(api, "Blacklist check completed.");
    return true;
}

bool nightauth_reset_hwid(NightAuthApi* api, const char* username, const char* password, const char* totp_code) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "username", username);
    na_add_str(&body, "password", password);
    if (totp_code && totp_code[0]) na_add_str(&body, "totpCode", totp_code);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/hwid-reset", &body, "HWID reset failed", "hwid_reset_failed");
    if (!p) return false;
    na_set_success(api, na_json_get_str(p, "message", "Hardware ID reset."));
    na_json_free(p);
    return true;
}

bool nightauth_change_username(NightAuthApi* api, const char* new_username, const char* password) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "newUsername", new_username);
    na_add_str(&body, "password", password);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/change-username", &body, "Username change failed", "username_change_failed");
    if (!p) return false;
    na_json_free(p);

    na_set_success(api, "Username changed; log in again with the new name.");
    return true;
}

bool nightauth_forgot_password(NightAuthApi* api, const char* email) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "email", email);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/forgot-password", &body, "Password reset request failed", "forgot_password_failed");
    if (!p) return false;
    na_json_free(p);
    na_set_success(api, "If the account exists, a reset email was sent.");
    return true;
}

bool nightauth_reset_password(NightAuthApi* api, const char* token, const char* new_password) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_buf_append_str(&body, "{\"token\":");
    na_json_escape_append(&body, token ? token : "");
    na_buf_append_str(&body, ",\"newPassword\":");
    na_json_escape_append(&body, new_password ? new_password : "");
    na_buf_append_str(&body, "}");
    NaHttpResp raw;
    bool http_ok = na_http_request(api, "POST", "/reset-password", (const char*)body.data, &raw);
    na_buf_free(&body);

    if (!http_ok) {
        na_set_error(api, "network request to " "/reset-password" " failed", na_transport_code(&raw));
        return false;
    }
    NaJson* resp = na_json_parse((const char*)raw.buf.data, raw.buf.len);
    na_buf_free(&raw.buf);
    if (!resp || resp->type != NA_JSON_OBJ) {
        na_json_free(resp);
        na_set_error(api, "malformed reset-password response", "invalid_response");
        return false;
    }
    NaJson* s = na_json_obj_get(resp, "success");
    if (raw.status != 200 || !(s && s->type == NA_JSON_BOOL && s->boolean)) {
        na_set_error(api, na_json_get_str(resp, "message", "Password reset failed"),
                     na_json_get_str(resp, "error", "reset_password_failed"));
        na_json_free(resp);
        return false;
    }
    na_json_free(resp);
    na_set_success(api, "Password reset.");
    return true;
}

bool nightauth_verify_file_hash(NightAuthApi* api, const char* hash) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "hash", hash);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/verify-hash", &body, "Hash verification failed", "hash_mismatch");
    if (!p) return false;
    na_json_free(p);
    na_set_success(api, "File hash verified.");
    return true;
}

bool nightauth_ban(NightAuthApi* api, const char* reason) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    if (reason && reason[0]) na_add_str(&body, "reason", reason);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/ban", &body, "Ban report failed", "ban_failed");
    if (!p) return false;
    na_json_free(p);
    na_set_success(api, "Ban reported.");
    return true;
}

int nightauth_get_online_users(NightAuthApi* api, int64_t* out_online, char out_users[][128], int max_users) {
    if (out_online) *out_online = 0;
    if (!api) return -1;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/online-users", &body, "Online users query failed", "online_users_failed");
    if (!p) return -1;
    NaJson* online = na_json_obj_get(p, "online");
    NaJson* users = na_json_obj_get(p, "users");
    if (!online || online->type != NA_JSON_NUM || !users || users->type != NA_JSON_ARR) {
        na_json_free(p);
        na_set_error(api, "malformed online-users response", "protocol_error");
        return -1;
    }
    if (out_online) *out_online = (int64_t)online->number;
    int n = 0;
    for (size_t i = 0; i < users->count && out_users && n < max_users; i++) {
        NaJson* item = users->items[i];
        if (!item || item->type != NA_JSON_STR) continue;
        if (!na_copy_bounded(out_users[n], 128, item->string)) {
            na_json_free(p);
            na_set_error(api, "online user name does not fit the output buffer", "buffer_too_small");
            return -1;
        }
        n++;
    }
    na_json_free(p);
    na_set_success(api, "Online users retrieved.");
    return n;
}

bool nightauth_get_stats(NightAuthApi* api, NightAuthAppStats* out_stats) {
    if (out_stats) memset(out_stats, 0, sizeof(*out_stats));
    if (!api || !out_stats) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/stats", &body, "Stats query failed", "stats_failed");
    if (!p) return false;
    NaJson* users = na_json_obj_get(p, "users");
    NaJson* lic = na_json_obj_get(p, "licenses");
    if (!users || users->type != NA_JSON_OBJ || !lic || lic->type != NA_JSON_OBJ) {
        na_json_free(p);
        na_set_error(api, "malformed stats response", "protocol_error");
        return false;
    }
    out_stats->total_users = na_json_get_int(users, "total");
    out_stats->active_users = na_json_get_int(users, "active");
    out_stats->banned_users = na_json_get_int(users, "banned");
    out_stats->total_licenses = na_json_get_int(lic, "total");
    out_stats->used_licenses = na_json_get_int(lic, "used");
    out_stats->unused_licenses = na_json_get_int(lic, "notUsed");
    snprintf(out_stats->version, sizeof(out_stats->version), "%s", na_json_get_str(p, "version", ""));
    na_json_free(p);
    na_set_success(api, "Stats retrieved.");
    return true;
}

bool nightauth_get_global_var(NightAuthApi* api, const char* key, char* out_val, size_t out_val_size) {
    if (out_val && out_val_size) out_val[0] = '\0';
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "varName", key);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/global-var", &body, "Global variable lookup failed", "not_found");
    if (!p) return false;
    NaJson* v = na_json_obj_get(p, "value");
    if (!v || v->type != NA_JSON_STR) {
        na_json_free(p);
        na_set_error(api, "global variable has no string value", "protocol_error");
        return false;
    }
    if (!na_copy_bounded(out_val, out_val_size, v->string)) {
        na_json_free(p);
        na_set_error(api, "global variable value does not fit the output buffer", "buffer_too_small");
        return false;
    }
    na_json_free(p);
    na_set_success(api, "Global variable retrieved.");
    return true;
}

bool nightauth_get_user_var(NightAuthApi* api, const char* key, char* out_val, size_t out_val_size) {
    if (out_val && out_val_size) out_val[0] = '\0';
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/user-vars/get", &body, "User variables query failed", "user_vars_failed");
    if (!p) return false;
    NaJson* vars = na_json_obj_get(p, "variables");
    if (!vars || vars->type != NA_JSON_ARR) {
        na_json_free(p);
        na_set_error(api, "malformed user-vars response", "protocol_error");
        return false;
    }
    bool found = false, fits = true;
    for (size_t i = 0; i < vars->count && !found; i++) {
        NaJson* item = vars->items[i];
        if (!item || item->type != NA_JSON_OBJ) continue;
        if (strcmp(na_json_get_str(item, "key", ""), key ? key : "") != 0) continue;
        found = true;
        fits = na_copy_bounded(out_val, out_val_size, na_json_get_str(item, "value", ""));
    }
    na_json_free(p);
    if (!found) { na_set_error(api, "user variable not found", "not_found"); return false; }
    if (!fits) { na_set_error(api, "user variable value does not fit the output buffer", "buffer_too_small"); return false; }
    na_set_success(api, "User variable retrieved.");
    return true;
}

int nightauth_get_user_variables(NightAuthApi* api, NightAuthUserVar* out, int max_vars) {
    if (!api) return -1;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/user-vars/get", &body, "User variables query failed", "user_vars_failed");
    if (!p) return -1;
    NaJson* vars = na_json_obj_get(p, "variables");
    if (!vars || vars->type != NA_JSON_ARR) {
        na_json_free(p);
        na_set_error(api, "malformed user-vars response", "protocol_error");
        return -1;
    }
    int n = 0;
    for (size_t i = 0; i < vars->count; i++) {
        NaJson* item = vars->items[i];
        if (!item || item->type != NA_JSON_OBJ) continue;
        if (out && n < max_vars) {
            na_copy_bounded(out[n].key, sizeof out[n].key, na_json_get_str(item, "key", ""));
            na_copy_bounded(out[n].value, sizeof out[n].value, na_json_get_str(item, "value", ""));
        }
        n++;
    }
    na_json_free(p);
    na_set_success(api, "User variables retrieved.");
    return n;
}

bool nightauth_set_user_var(NightAuthApi* api, const char* key, const char* value) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "key", key);
    na_add_str(&body, "value", value);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/user-vars/set", &body, "User variable write failed", "user_vars_failed");
    if (!p) return false;
    na_json_free(p);
    na_set_success(api, "User variable saved.");
    return true;
}

bool nightauth_log(NightAuthApi* api, const char* message) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "message", message);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/log", &body, "Log submission failed", "log_failed");
    if (!p) return false;
    na_json_free(p);
    na_set_success(api, "Log event submitted.");
    return true;
}

int nightauth_get_chat_messages(NightAuthApi* api, const char* channel, NightAuthChatMessage* out_msgs, int max_msgs) {
    return nightauth_get_chat_messages_ex(api, channel, 0, false, out_msgs, max_msgs);
}

int nightauth_get_chat_messages_ex(NightAuthApi* api, const char* channel, int64_t before_id, bool mark_read,
                                   NightAuthChatMessage* out_msgs, int max_msgs) {
    if (!api) return -1;
    if (!out_msgs) max_msgs = 0;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "room", channel);
    if (before_id > 0) {
        char b[32];
        snprintf(b, sizeof(b), "%lld", (long long)before_id);
        na_buf_append_str(&body, ",\"before\":");
        na_buf_append_str(&body, b);
    }
    if (mark_read) na_buf_append_str(&body, ",\"markRead\":true");
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/chat/get", &body, "Chat history query failed", "chat_failed");
    if (!p) return -1;
    NaJson* msgs = na_json_obj_get(p, "messages");
    if (!msgs || msgs->type != NA_JSON_ARR) {
        na_json_free(p);
        na_set_error(api, "malformed chat/get response", "protocol_error");
        return -1;
    }
    int n = 0;
    for (size_t i = 0; i < msgs->count && n < max_msgs; i++) {
        NaJson* item = msgs->items[i];
        if (!item || item->type != NA_JSON_OBJ) continue;
        NightAuthChatMessage* m = &out_msgs[n];
        memset(m, 0, sizeof(*m));
        char idbuf[32];
        snprintf(idbuf, sizeof(idbuf), "%lld", (long long)na_json_get_int(item, "id"));
        bool fits = na_copy_bounded(m->id, sizeof(m->id), idbuf) &&
                    na_copy_bounded(m->author, sizeof(m->author), na_json_get_str(item, "author", "")) &&
                    na_copy_bounded(m->message, sizeof(m->message), na_json_get_str(item, "content", "")) &&
                    na_copy_bounded(m->timestamp, sizeof(m->timestamp), na_json_get_str(item, "createdAt", "")) &&
                    na_copy_bounded(m->status, sizeof(m->status), na_json_get_str(item, "status", ""));
        if (!fits) {
            na_json_free(p);
            na_set_error(api, "chat message does not fit the output buffer", "buffer_too_small");
            return -1;
        }
        n++;
    }
    na_json_free(p);
    na_set_success(api, "Chat messages retrieved.");
    return n;
}

bool nightauth_send_chat_message(NightAuthApi* api, const char* message, const char* channel) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "room", channel);
    na_add_str(&body, "content", message);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/chat/send", &body, "Chat send failed", "chat_failed");
    if (!p) return false;
    na_json_free(p);
    na_set_success(api, "Chat message sent.");
    return true;
}

bool nightauth_setup_2fa(NightAuthApi* api, NightAuthTwoFactorSetupData* out_data) {
    if (!api) return false;
    if (!out_data) { na_set_error(api, "output buffer required", "invalid_argument"); return false; }
    memset(out_data, 0, sizeof(*out_data));
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/2fa/setup", &body, "2FA setup failed", "2fa_setup_failed");
    if (!p) return false;
    const char* secret = na_json_get_str(p, "secret", NULL);
    const char* uri = na_json_get_str(p, "uri", NULL);
    const char* qr = na_json_get_str(p, "qr", NULL);
    if (!secret || !uri || !qr) {
        na_json_free(p);
        na_set_error(api, "2FA setup response is missing fields", "protocol_error");
        return false;
    }
    bool fits = na_copy_bounded(out_data->secret, sizeof(out_data->secret), secret) &&
                na_copy_bounded(out_data->uri, sizeof(out_data->uri), uri) &&
                na_copy_bounded(out_data->qr_data_url, sizeof(out_data->qr_data_url), qr);
    na_json_free(p);
    if (!fits) { na_set_error(api, "2FA setup field does not fit the output buffer", "buffer_too_small"); return false; }
    na_set_success(api, "2FA enrolment started; confirm with a code from the new authenticator.");
    return true;
}

bool nightauth_confirm_2fa(NightAuthApi* api, const char* code, NightAuthTwoFactorConfirmData* out_data) {
    if (!api) return false;
    if (!out_data) { na_set_error(api, "output buffer required", "invalid_argument"); return false; }
    memset(out_data, 0, sizeof(*out_data));
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "code", code);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/2fa/confirm", &body, "2FA confirmation failed", "code_invalid");
    if (!p) return false;
    NaJson* codes = na_json_obj_get(p, "backupCodes");
    if (!codes || codes->type != NA_JSON_ARR) {
        na_json_free(p);
        na_set_error(api, "2FA confirm response is missing backup codes", "protocol_error");
        return false;
    }
    bool fits = true;
    for (size_t i = 0; i < codes->count && fits; i++) {
        NaJson* item = codes->items[i];
        if (!item || item->type != NA_JSON_STR || out_data->backup_code_count >= 16) { fits = false; break; }
        fits = na_copy_bounded(out_data->backup_codes[out_data->backup_code_count], sizeof(out_data->backup_codes[0]), item->string);
        if (fits) out_data->backup_code_count++;
    }
    na_json_free(p);
    if (!fits) { na_set_error(api, "backup code list does not fit the output buffer", "buffer_too_small"); return false; }
    na_set_success(api, "2FA enabled; store the backup codes safely.");
    return true;
}

bool nightauth_disable_2fa(NightAuthApi* api, const char* password, const char* code) {
    if (!api) return false;
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "password", password);
    na_add_str(&body, "code", code);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/2fa/disable", &body, "2FA disable failed", "invalid_code");
    if (!p) return false;
    na_json_free(p);
    na_set_success(api, "2FA disabled.");
    return true;
}

bool nightauth_access_ticket(NightAuthApi* api, const char* scope, const char* resource_id,
                             char* out_ticket, size_t out_ticket_size, int64_t* out_expires_in) {
    if (out_expires_in) *out_expires_in = 0;
    if (out_ticket && out_ticket_size) out_ticket[0] = '\0';
    if (!api) return false;
    bool is_file = scope && strcmp(scope, "file") == 0;
    if (!scope || (!is_file && strcmp(scope, "events") != 0)) {
        na_set_error(api, "scope must be \"events\" or \"file\"", "invalid_argument");
        return false;
    }
    if (is_file && (!resource_id || !resource_id[0])) {
        na_set_error(api, "resource_id is required for file tickets", "invalid_argument");
        return false;
    }
    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "scope", scope);
    if (resource_id && resource_id[0]) na_add_str(&body, "resourceId", resource_id);
    na_buf_append_str(&body, "}");
    NaJson* p = na_call_json(api, "/access-ticket", &body, "Access ticket request failed", "access_ticket_failed");
    if (!p) return false;
    const char* ticket = na_json_get_str(p, "ticket", NULL);
    if (!ticket || !out_ticket || !na_copy_bounded(out_ticket, out_ticket_size, ticket)) {
        na_json_free(p);
        na_set_error(api, "access ticket is missing or does not fit the output buffer", "buffer_too_small");
        return false;
    }
    if (out_expires_in) *out_expires_in = na_json_get_int(p, "expiresIn");
    na_json_free(p);
    na_set_success(api, "Access ticket issued.");
    return true;
}

bool nightauth_download_file(NightAuthApi* api, const char* ticket, char out_name[128],
                             uint8_t* out_data, size_t out_data_cap, size_t* out_data_len) {
    if (out_data_len) *out_data_len = 0;
    if (out_name) out_name[0] = '\0';
    if (!api || !out_data_len) return false;
    if (!na_ticket_charset_ok(ticket)) {
        na_set_error(api, "ticket is missing or malformed", "invalid_argument");
        return false;
    }
    char target[160];
    if (snprintf(target, sizeof(target), "%s?ticket=%s", "/files/download", ticket) >= (int)sizeof(target)) {
        na_set_error(api, "ticket is too long", "invalid_argument");
        return false;
    }
    NaHttpResp raw;
    bool sent = na_http_request(api, "GET", target, NULL, &raw);
    if (raw.status >= 300 && raw.status < 400) {

        na_buf_free(&raw.buf);
        na_set_error(api, "File download redirected to an external location; it was not followed.", "external_download");
        snprintf(api->response.url, sizeof(api->response.url), "%s", raw.location);
        return false;
    }
    if (!sent) {
        na_set_error(api, "file download request failed", "network_error");
        return false;
    }
    NaJson* resp = na_json_parse((const char*)raw.buf.data, raw.buf.len);
    na_buf_free(&raw.buf);
    if (!resp || resp->type != NA_JSON_OBJ) {
        na_json_free(resp);
        na_set_error(api, "malformed file download response", "protocol_error");
        return false;
    }
    NaJson* s = na_json_obj_get(resp, "success");
    if (!(s && s->type == NA_JSON_BOOL && s->boolean)) {
        na_set_error(api, na_json_get_str(resp, "message", "File download failed"),
                     na_json_get_str(resp, "error", "download_failed"));
        na_json_free(resp);
        return false;
    }
    const char* data_b64 = na_json_get_str(resp, "data", NULL);
    if (!data_b64) {
        na_json_free(resp);
        na_set_error(api, "file download response has no data (external URL files are not fetched)", "protocol_error");
        return false;
    }
    if (out_name) snprintf(out_name, 128, "%s", na_json_get_str(resp, "name", ""));
    size_t gz_len = 0;
    uint8_t* gz = na_base64_decode(data_b64, &gz_len);
    bool too_large = false;
    bool ok = gz && na_gunzip_ex(gz, gz_len, out_data, out_data_cap, out_data_len, &too_large);
    free(gz);
    na_json_free(resp);
    if (!ok) {
        na_set_error(api,
                     too_large ? "file does not fit the output buffer" : "file data is not valid gzip",
                     too_large ? "buffer_too_small" : "file_decode_failed");
        return false;
    }
    na_set_success(api, "File downloaded.");
    return true;
}

bool nightauth_webhook(NightAuthApi* api, const char* web_id, const char* content,
                       const char* payload_json, int* out_status) {
    return nightauth_webhook_ex(api, web_id, content, NULL, payload_json, NULL, 0, NULL, out_status);
}

bool nightauth_webhook_ex(NightAuthApi* api, const char* web_id, const char* content, const char* params,
                          const char* payload_json, const void* file_data, size_t file_len,
                          const char* file_name, int* out_status) {
    if (out_status) *out_status = 0;
    if (!api) return false;

    NaBuf body; na_buf_init(&body);
    na_begin_body(&body, api);
    na_add_str(&body, "webId", web_id ? web_id : "");
    if (content) na_add_str(&body, "content", content);
    if (params) na_add_str(&body, "params", params);
    if (payload_json) {
        NaJson* pv = na_json_parse(payload_json, strlen(payload_json));
        bool valid = pv != NULL;
        na_json_free(pv);
        if (!valid) {
            na_buf_free(&body);
            na_set_error(api, "payload_json must be valid JSON", "invalid_argument");
            return false;
        }
        na_buf_append_str(&body, ",\"payload\":");
        na_buf_append_str(&body, payload_json);
    }
    na_buf_append_str(&body, "}");
    if (file_data) {
        api->mp_data = (const uint8_t*)file_data;
        api->mp_len = file_len;
        api->mp_name = file_name;
    }
    char* resp_json = na_send_encrypted(api, "/webhook", (const char*)body.data);
    api->mp_data = NULL;
    api->mp_len = 0;
    api->mp_name = NULL;
    na_buf_free(&body);
    if (!resp_json) return false;
    NaJson* parsed = NULL;
    bool delivered = na_body_is_success(resp_json, &parsed);
    free(resp_json);
    if (!parsed) {
        na_set_error(api, "malformed webhook response", "protocol_error");
        return false;
    }
    if (out_status) *out_status = (int)na_json_get_int(parsed, "status");
    if (!delivered) {
        na_set_error(api, na_json_get_str(parsed, "message", "Webhook delivery failed"),
                     na_json_get_str(parsed, "error", "webhook_failed"));
        na_json_free(parsed);
        return false;
    }
    na_json_free(parsed);
    na_set_success(api, "Webhook delivered.");
    return true;
}

typedef struct {
    NightAuthApi* api;
    CURL* curl;
    NightAuthEventCallback cb;
    NightAuthEventCallbackEx cb_ex;
    void* user;
    NaSse sse;
    long status;
    NaBuf error_body;
    bool failed;
} NaEventsCtx;

static bool na_events_dispatch(const char* json, void* u) {
    NaEventsCtx* c = (NaEventsCtx*)u;
    if (c->cb_ex) return c->cb_ex(json, c->sse.last_id[0] ? c->sse.last_id : NULL, c->user);
    return c->cb(json, c->user);
}

static size_t na_events_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    NaEventsCtx* c = (NaEventsCtx*)userdata;
    size_t n = size * nmemb;
    if (atomic_load(&c->api->events_cancel)) return 0;
    if (c->status == 0) curl_easy_getinfo(c->curl, CURLINFO_RESPONSE_CODE, &c->status);
    if (c->status != 200) {

        if (c->error_body.len + n > 16 * 1024) return 0;
        if (!na_buf_append(&c->error_body, ptr, n)) return 0;
        return n;
    }
    if (!na_sse_feed(&c->sse, ptr, n, na_events_dispatch, c)) {
        c->failed = true;
        return 0;
    }
    if (c->sse.stop) return 0;
    return n;
}

static int na_events_progress_cb(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                                 curl_off_t ultotal, curl_off_t ulnow) {
    (void)dltotal; (void)dlnow; (void)ultotal; (void)ulnow;
    NaEventsCtx* c = (NaEventsCtx*)clientp;
    return atomic_load(&c->api->events_cancel) ? 1 : 0;
}

static bool na_events_run_impl(NightAuthApi* api, const char* ticket, const char* last_event_id,
                               NightAuthEventCallback cb, NightAuthEventCallbackEx cb_ex, void* user) {
    if (!api) return false;
    if (!cb && !cb_ex) { na_set_error(api, "event callback is required", "invalid_argument"); return false; }
    if (!na_ticket_charset_ok(ticket)) { na_set_error(api, "ticket is missing or malformed", "invalid_argument"); return false; }
    bool is_https = false, loopback = false;
    if (!na_base_url_allowed(api->api_url, api->allow_insecure_loopback, &is_https, &loopback)) {
        na_set_error(api, "API URL does not satisfy the transport policy", "transport_refused");
        return false;
    }
    char url[700];
    if (snprintf(url, sizeof(url), "%s%s?ticket=%s", api->api_url, "/events", ticket) >= (int)sizeof(url)) {
        na_set_error(api, "ticket is too long", "invalid_argument");
        return false;
    }
    atomic_store(&api->events_cancel, 0);

    CURL* curl = curl_easy_init();
    if (!curl) { na_set_error(api, "could not initialise libcurl", "network_error"); return false; }
    NaEventsCtx c;
    memset(&c, 0, sizeof(c));
    c.api = api;
    c.curl = curl;
    c.cb = cb;
    c.cb_ex = cb_ex;
    c.user = user;
    na_sse_init(&c.sse);
    na_buf_init(&c.error_body);

    struct curl_slist* headers = curl_slist_append(NULL, "Accept: text/event-stream");
    headers = curl_slist_append(headers, "User-Agent: NightAuth-C-SDK/1.0");
    if (last_event_id && last_event_id[0]) {
        char lei[160];
        if (snprintf(lei, sizeof(lei), "Last-Event-ID: %s", last_event_id) < (int)sizeof(lei)) {
            headers = curl_slist_append(headers, lei);
        }
    }
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, na_events_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &c);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, na_events_progress_cb);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &c);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, is_https ? "https" : "http");
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    CURLcode rc = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    bool cancelled = atomic_load(&api->events_cancel) != 0;
    api->last_status = status;

    if (status != 0 && status != 200) {
        NaJson* err = na_json_parse((const char*)c.error_body.data, c.error_body.len);
        na_set_error(api, na_json_get_str(err, "message", "event stream was refused"),
                     na_json_get_str(err, "error", "events_refused"));
        na_json_free(err);
    } else if (c.failed) {
        na_set_error(api, "event stream violated protocol limits", "protocol_error");
    } else if (c.sse.stop || cancelled || rc == CURLE_ABORTED_BY_CALLBACK || rc == CURLE_OK) {
        na_set_success(api, cancelled ? "Event stream cancelled." : "Event stream closed.");
        rc = CURLE_OK;
    } else {
        na_set_error(api, "event stream transport error", "network_error");
    }
    bool ok = (rc == CURLE_OK) && nightauth_get_response(api)->success;

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    na_sse_free(&c.sse);
    na_buf_free(&c.error_body);
    return ok;
}

bool nightauth_events_run(NightAuthApi* api, const char* ticket, const char* last_event_id,
                          NightAuthEventCallback cb, void* user) {
    return na_events_run_impl(api, ticket, last_event_id, cb, NULL, user);
}

bool nightauth_events_run_ex(NightAuthApi* api, const char* ticket, const char* last_event_id,
                             NightAuthEventCallbackEx cb, void* user) {
    return na_events_run_impl(api, ticket, last_event_id, NULL, cb, user);
}

const char* nightauth_get_pending_download_url(const NightAuthApi* api) {
    return api ? api->pending_download_url : "";
}

const char* nightauth_get_required_version(const NightAuthApi* api) {
    return api ? api->required_version : "";
}

bool nightauth_apply_update(NightAuthApi* api, const char* download_url) {
    (void)download_url;
    if (api) {
        na_set_error(api, "Unsigned auto-update is disabled. Verify a signed release manifest with SignedUpdateInstaller.", "unsigned_update_disabled");
    }
    return false;
}

bool nightauth_signed_update_verify(const char* envelope_json, const char* pinned_pub_key_hex,
                                    const char* expected_app, uint64_t min_sequence,
                                    NightAuthVerifiedUpdate* out_update, char* err_buf, size_t err_len) {
    if (!envelope_json || strlen(envelope_json) > 65536 || !pinned_pub_key_hex || strlen(pinned_pub_key_hex) != 64 ||
        !expected_app || !expected_app[0] || !out_update) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Invalid manifest parameters.");
        return false;
    }

    uint8_t key_bytes[32];
    if (!na_hex_decode(pinned_pub_key_hex, key_bytes, 32)) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Invalid pinned public key hex.");
        return false;
    }

    NaJson* env = na_json_parse(envelope_json, strlen(envelope_json));
    if (!env || env->type != NA_JSON_OBJ) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Manifest envelope is not a valid JSON object.");
        na_json_free(env);
        return false;
    }

    const char* payload_b64 = na_json_get_str(env, "payload", NULL);
    const char* sig_b64 = na_json_get_str(env, "signature", NULL);
    if (!payload_b64 || !sig_b64) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Manifest envelope missing payload or signature.");
        na_json_free(env);
        return false;
    }

    size_t payload_len = 0;
    uint8_t* payload_bytes = na_base64_decode(payload_b64, &payload_len);
    size_t sig_len = 0;
    uint8_t* sig_bytes = na_base64_decode(sig_b64, &sig_len);

    if (!payload_bytes || payload_len > 32768 || !sig_bytes || sig_len != 64) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Invalid manifest payload size or signature length.");
        free(payload_bytes);
        free(sig_bytes);
        na_json_free(env);
        return false;
    }

    if (!na_ed25519_verify(key_bytes, payload_bytes, payload_len, sig_bytes, sig_len)) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Release signature rejected.");
        free(payload_bytes);
        free(sig_bytes);
        na_json_free(env);
        return false;
    }

    free(sig_bytes);
    na_json_free(env);

    NaJson* data = na_json_parse((const char*)payload_bytes, payload_len);
    free(payload_bytes);
    if (!data || data->type != NA_JSON_OBJ) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Manifest payload is not a valid JSON object.");
        na_json_free(data);
        return false;
    }

    int64_t schema = na_json_get_int(data, "schema");
    const char* app = na_json_get_str(data, "application", "");
    const char* ver = na_json_get_str(data, "version", "");
    int64_t seq = na_json_get_int(data, "sequence");
    int64_t size = na_json_get_int(data, "size");
    int64_t expires_at = na_json_get_int(data, "expiresAt");
    const char* url_str = na_json_get_str(data, "url", "");
    const char* sha256_hex = na_json_get_str(data, "sha256", "");

    if (schema != 1 || strcmp(app, expected_app) != 0 || !ver[0] || strlen(ver) > 100) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Release identity or schema rejected.");
        na_json_free(data);
        return false;
    }

    if (seq <= (int64_t)min_sequence || seq > 9007199254740991LL) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Release sequence rejected.");
        na_json_free(data);
        return false;
    }

    if (size < 1 || size > 256 * 1024 * 1024) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Release artifact size rejected.");
        na_json_free(data);
        return false;
    }

    if (expires_at <= (int64_t)time(NULL)) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Release expired.");
        na_json_free(data);
        return false;
    }

    if (strncmp(url_str, "https://", 8) != 0 || strchr(url_str, '#') || strchr(url_str, '@')) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Release URL must be HTTPS without user info or fragment.");
        na_json_free(data);
        return false;
    }

    if (strlen(sha256_hex) != 64 || !na_hex_decode(sha256_hex, out_update->digest, 32)) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Invalid sha256 hex length or characters.");
        na_json_free(data);
        return false;
    }

    strncpy(out_update->application, app, sizeof(out_update->application) - 1);
    out_update->application[sizeof(out_update->application) - 1] = '\0';
    strncpy(out_update->version, ver, sizeof(out_update->version) - 1);
    out_update->version[sizeof(out_update->version) - 1] = '\0';
    out_update->sequence = (uint64_t)seq;
    out_update->size = (uint64_t)size;
    out_update->expires_at = (uint64_t)expires_at;
    strncpy(out_update->url, url_str, sizeof(out_update->url) - 1);
    out_update->url[sizeof(out_update->url) - 1] = '\0';

    na_json_free(data);
    return true;
}

typedef struct {
    FILE* fp;
    EVP_MD_CTX* mdctx;
    uint64_t total_bytes;
    uint64_t max_bytes;
    bool error;
} NaDownloadContext;

static size_t na_download_write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    NaDownloadContext* dc = (NaDownloadContext*)userdata;
    size_t bytes = size * nmemb;
    if (dc->total_bytes + bytes > dc->max_bytes) {
        dc->error = true;
        return 0;
    }
    dc->total_bytes += bytes;
    if (EVP_DigestUpdate(dc->mdctx, ptr, bytes) != 1) {
        dc->error = true;
        return 0;
    }
    size_t written = fwrite(ptr, 1, bytes, dc->fp);
    if (written != bytes) {
        dc->error = true;
        return 0;
    }
    return bytes;
}

bool nightauth_signed_update_download(const NightAuthVerifiedUpdate* update, const char* destination,
                                      char* err_buf, size_t err_len) {
    if (!update || !destination || !destination[0]) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Invalid download parameters.");
        return false;
    }
    if (update->expires_at <= (uint64_t)time(NULL)) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Release expired.");
        return false;
    }

    FILE* check = fopen(destination, "rb");
    if (check) {
        fclose(check);
        if (err_buf && err_len) snprintf(err_buf, err_len, "Destination file already exists.");
        return false;
    }

    char temp_path[1024];
    snprintf(temp_path, sizeof(temp_path), "%s.%lu.partial", destination, (unsigned long)time(NULL));

    FILE* fp = fopen(temp_path, "wb");
    if (!fp) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Failed to create temporary download file.");
        return false;
    }

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx || EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        fclose(fp);
        remove(temp_path);
        if (mdctx) EVP_MD_CTX_free(mdctx);
        if (err_buf && err_len) snprintf(err_buf, err_len, "Failed to initialize SHA-256 context.");
        return false;
    }

    NaDownloadContext dc;
    dc.fp = fp;
    dc.mdctx = mdctx;
    dc.total_bytes = 0;
    dc.max_bytes = update->size;
    dc.error = false;

    CURL* curl = curl_easy_init();
    if (!curl) {
        fclose(fp);
        remove(temp_path);
        EVP_MD_CTX_free(mdctx);
        if (err_buf && err_len) snprintf(err_buf, err_len, "Failed to initialize libcurl.");
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, update->url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, na_download_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &dc);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    struct curl_slist* headers = curl_slist_append(NULL, "User-Agent: NightAuth-Updater/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    fclose(fp);

    if (res != CURLE_OK || http_code != 200 || dc.error) {
        remove(temp_path);
        EVP_MD_CTX_free(mdctx);
        if (err_buf && err_len) {
            snprintf(err_buf, err_len, "Download failed: HTTP %ld (%s)", http_code, curl_easy_strerror(res));
        }
        return false;
    }

    if (dc.total_bytes != update->size) {
        remove(temp_path);
        EVP_MD_CTX_free(mdctx);
        if (err_buf && err_len) snprintf(err_buf, err_len, "Artifact size differs from signed manifest.");
        return false;
    }

    uint8_t computed[32];
    unsigned int comp_len = 0;
    EVP_DigestFinal_ex(mdctx, computed, &comp_len);
    EVP_MD_CTX_free(mdctx);

    if (comp_len != 32 || CRYPTO_memcmp(computed, update->digest, 32) != 0) {
        remove(temp_path);
        if (err_buf && err_len) snprintf(err_buf, err_len, "Artifact hash rejected.");
        return false;
    }

    if (update->expires_at <= (uint64_t)time(NULL)) {
        remove(temp_path);
        if (err_buf && err_len) snprintf(err_buf, err_len, "Release expired during download.");
        return false;
    }

    if (rename(temp_path, destination) != 0) {
        remove(temp_path);
        if (err_buf && err_len) snprintf(err_buf, err_len, "Failed to move downloaded artifact to destination.");
        return false;
    }

    return true;
}

static void na_make_sequence_dir(char* dir_out, size_t dir_len) {
#ifdef _WIN32
    const char* root = getenv("LOCALAPPDATA");
    if (!root || !root[0]) root = "C:\\NightAuth";
    snprintf(dir_out, dir_len, "%s\\NightAuth\\release-sequences", root);
    CreateDirectoryA(root, NULL);
    char parent[512];
    snprintf(parent, sizeof(parent), "%s\\NightAuth", root);
    CreateDirectoryA(parent, NULL);
    CreateDirectoryA(dir_out, NULL);
#else
    const char* root = getenv("XDG_DATA_HOME");
    if (!root || !root[0]) {
        const char* home = getenv("HOME");
        snprintf(dir_out, dir_len, "%s/.local/share/NightAuth/release-sequences", home ? home : "/tmp");
    } else {
        snprintf(dir_out, dir_len, "%s/NightAuth/release-sequences", root);
    }
#endif
}

bool nightauth_signed_update_installer_download(const char* manifest_url, const char* pinned_pub_key_hex,
                                                const char* application, const char* destination,
                                                char* err_buf, size_t err_len) {
    if (!manifest_url || strncmp(manifest_url, "https://", 8) != 0 || strchr(manifest_url, '#') || strchr(manifest_url, '@') ||
        !pinned_pub_key_hex || !application || !application[0] || !destination) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Invalid installer download parameters.");
        return false;
    }

    char dir[512];
    na_make_sequence_dir(dir, sizeof(dir));

    unsigned char app_hash[32];
    unsigned int app_hash_len = 0;
    EVP_MD_CTX* md = EVP_MD_CTX_new();
    if (md) {
        EVP_DigestInit_ex(md, EVP_sha256(), NULL);
        EVP_DigestUpdate(md, application, strlen(application));
        EVP_DigestFinal_ex(md, app_hash, &app_hash_len);
        EVP_MD_CTX_free(md);
    }
    char app_hash_hex[65];
    na_hex_encode(app_hash, 32, app_hash_hex);

    char seq_file[1024];
#ifdef _WIN32
    snprintf(seq_file, sizeof(seq_file), "%s\\%s.seq", dir, app_hash_hex);
#else
    snprintf(seq_file, sizeof(seq_file), "%s/%s.seq", dir, app_hash_hex);
#endif

    uint64_t current_seq = 0;
    FILE* sf = fopen(seq_file, "r");
    if (sf) {
        char buf[64];
        if (fgets(buf, sizeof(buf), sf)) {
            char* end = NULL;
            long long val = strtoll(buf, &end, 10);
            if (val < 0) {
                fclose(sf);
                if (err_buf && err_len) snprintf(err_buf, err_len, "Stored release sequence is corrupt; refusing update.");
                return false;
            }
            current_seq = (uint64_t)val;
        }
        fclose(sf);
    }

    NaHttpResp raw;
    memset(&raw, 0, sizeof(raw));
    na_buf_init(&raw.buf);

    CURL* curl = curl_easy_init();
    if (!curl) {
        if (err_buf && err_len) snprintf(err_buf, err_len, "Failed to initialize libcurl.");
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, manifest_url);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, na_curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &raw);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    struct curl_slist* headers = curl_slist_append(NULL, "User-Agent: NightAuth-Updater/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || http_code != 200 || raw.too_large || raw.buf.len == 0) {
        na_buf_free(&raw.buf);
        if (err_buf && err_len) {
            snprintf(err_buf, err_len, "Failed to fetch release manifest (HTTP %ld): %s", http_code, curl_easy_strerror(res));
        }
        return false;
    }

    NightAuthVerifiedUpdate verified;
    memset(&verified, 0, sizeof(verified));
    bool verify_ok = nightauth_signed_update_verify((const char*)raw.buf.data, pinned_pub_key_hex,
                                                    application, current_seq, &verified, err_buf, err_len);
    na_buf_free(&raw.buf);
    if (!verify_ok) return false;

    if (!nightauth_signed_update_download(&verified, destination, err_buf, err_len)) {
        return false;
    }

    char tmp_seq[1024];
    snprintf(tmp_seq, sizeof(tmp_seq), "%s.%lu.tmp", seq_file, (unsigned long)time(NULL));
    FILE* out_sf = fopen(tmp_seq, "w");
    if (out_sf) {
        fprintf(out_sf, "%llu\n", (unsigned long long)verified.sequence);
        fclose(out_sf);
        remove(seq_file);
        rename(tmp_seq, seq_file);
    }

    return true;
}

#endif
#endif
