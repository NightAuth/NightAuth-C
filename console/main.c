#define _CRT_SECURE_NO_WARNINGS
#define NIGHTAUTH_IMPLEMENTATION
#include "nightauth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <sys/stat.h>
#ifdef __MINGW32__
#undef fstat64
__attribute__((used)) int fstat64(int fd, struct _stat64 *buf) {
    return _fstat64(fd, buf);
}
#endif
#endif

/* Application Configuration */
#define APP_NAME    ""
#define APP_API_KEY ""
#define APP_VERSION "1.0"
#define APP_TOKEN   NULL

/* ANSI TrueColor & 256-Color Design Tokens */
#define C_RESET      "\x1b[0m"
#define C_BOLD       "\x1b[1m"
#define C_DIM        "\x1b[38;2;100;116;139m"
#define C_MUTED      "\x1b[38;2;148;163;184m"
#define C_TEXT       "\x1b[38;2;226;232;240m"
#define C_WHITE      "\x1b[38;2;248;250;252m"
#define C_BLUE       "\x1b[38;2;59;130;246m"
#define C_GLOW       "\x1b[38;2;96;165;250m"
#define C_CYAN       "\x1b[38;2;56;189;248m"
#define C_GREEN      "\x1b[38;2;52;211;153m"
#define C_AMBER      "\x1b[38;2;251;191;36m"
#define C_RED        "\x1b[38;2;248;113;113m"
#define C_BORDER     "\x1b[38;2;42;50;70m"
#define C_BADGE_BG   "\x1b[48;2;23;37;68m"
#define C_GREEN_BG   "\x1b[48;2;14;46;38m"
#define C_RED_BG     "\x1b[48;2;56;20;26m"

static void clear_screen(void) {
    printf("\x1b[2J\x1b[H");
    fflush(stdout);
}

static void print_hr(void) {
    printf("  " C_BORDER "────────────────────────────────────────────────────────────────────────────" C_RESET "\n");
}

static void print_section_header(const char* title, const char* subtitle) {
    printf("\n  " C_BLUE "▌" C_RESET " " C_WHITE C_BOLD "%s" C_RESET, title);
    if (subtitle && subtitle[0]) {
        printf("  " C_DIM "·  %s" C_RESET, subtitle);
    }
    printf("\n");
    print_hr();
}

static void print_ok(const char* title, const char* detail) {
    printf("\n  " C_GREEN_BG C_GREEN C_BOLD " ✓ SUCCESS " C_RESET " " C_WHITE C_BOLD "%s" C_RESET, title);
    if (detail && detail[0]) {
        printf(" " C_MUTED "— %s" C_RESET, detail);
    }
    printf("\n\n");
}

static void print_err(const char* title, const char* detail) {
    printf("\n  " C_RED_BG C_RED C_BOLD " ✕ ERROR " C_RESET " " C_WHITE C_BOLD "%s" C_RESET, title);
    if (detail && detail[0]) {
        printf(" " C_RED "— %s" C_RESET, detail);
    }
    printf("\n\n");
}

static void print_info(const char* label, const char* value) {
    printf("  " C_BLUE "•" C_RESET " " C_MUTED "%-18s" C_RESET " " C_CYAN "%s" C_RESET "\n", label, value);
}

static void ask(const char* prompt, char* out_buf, size_t buf_size) {
    printf("  " C_GLOW "❯" C_RESET " " C_TEXT "%s" C_RESET, prompt);
    fflush(stdout);
    if (fgets(out_buf, (int)buf_size, stdin)) {
        size_t len = strlen(out_buf);
        while (len > 0 && (out_buf[len - 1] == '\n' || out_buf[len - 1] == '\r')) {
            out_buf[--len] = '\0';
        }
    } else {
        exit(0);
    }
}

static void render_banner(const char* app_name, const char* hwid, bool connected) {
    printf("\n");
    printf("  " C_BLUE C_BOLD "  _   _ _       _     _      _         _   _     \n" C_RESET);
    printf("  " C_BLUE C_BOLD " | \\ | (_) __ _| |__ | |_   / \\  _   _| |_| |__  \n" C_RESET);
    printf("  " C_GLOW C_BOLD " |  \\| | |/ _` | '_ \\| __| / _ \\| | | | __| '_ \\ \n" C_RESET);
    printf("  " C_GLOW C_BOLD " | |\\  | | (_| | | | | |_ / ___ \\ |_| | |_| | | |\n" C_RESET);
    printf("  " C_CYAN C_BOLD " |_| \\_|_|\\__, |_| |_|\\__/_/   \\_\\__,_|\\__|_| |_|\n" C_RESET);
    printf("  " C_CYAN C_BOLD "          |___/                                  " C_RESET "  " C_BADGE_BG C_GLOW C_BOLD " NATIVE C CLI " C_RESET "\n\n");

    printf("  " C_BORDER "╭──────────────────────────────────────────────────────────────────────────╮" C_RESET "\n");
    printf("  " C_BORDER "│" C_RESET "  " C_DIM "APPLICATION" C_RESET "  " C_WHITE C_BOLD "%-26.26s" C_RESET "  " C_DIM "STATUS" C_RESET "  %s%-16s" C_RESET " " C_BORDER "│" C_RESET "\n",
           (app_name && app_name[0]) ? app_name : "Unconfigured",
           connected ? C_GREEN "● " : C_AMBER "◌ ",
           connected ? "Connected" : "Initializing");
    if (hwid && hwid[0]) {
        printf("  " C_BORDER "│" C_RESET "  " C_DIM "DEVICE HWID" C_RESET "  " C_CYAN "%-58.58s" C_RESET " " C_BORDER "│" C_RESET "\n", hwid);
    }
    printf("  " C_BORDER "╰──────────────────────────────────────────────────────────────────────────╯" C_RESET "\n");
}

static void perform_handshake(NightAuthApi* api, const char* app_name) {
    printf("\n  " C_MUTED "◌ Establishing encrypted session with NightAuth Cloud..." C_RESET "\r");
    fflush(stdout);
    nightauth_init(api);
    printf("                                                                      \r");
    fflush(stdout);

    NightAuthResponse* resp = nightauth_get_response(api);
    if (!resp->success) {
        render_banner(app_name, NULL, false);
        print_err("Handshake Failed", resp->message);
        printf("  " C_DIM "Please verify your App Name, API Key, and Server URL." C_RESET "\n\n");
        char dummy[16];
        ask("Press Enter to exit...", dummy, sizeof(dummy));
        exit(1);
    }

    char hwid[128] = {0};
    nightauth_get_hardware_id(hwid, sizeof(hwid));
    clear_screen();
    render_banner(app_name, hwid, true);
}

static bool handle_login(NightAuthApi* api) {
    char user[128] = {0};
    char pass[128] = {0};
    char totp[32] = {0};

    print_section_header("ACCOUNT SIGN IN", "Authenticate with your username and password");
    ask("Username: ", user, sizeof(user));
    ask("Password: ", pass, sizeof(pass));
    ask("2FA Code " C_DIM "(Optional, Enter to skip)" C_TEXT ": ", totp, sizeof(totp));

    if (nightauth_login(api, user, pass, totp[0] ? totp : NULL)) {
        print_ok("Session Authenticated", "Welcome back to NightAuth.");
        return true;
    }

    print_err("Login Failed", nightauth_get_response(api)->message);
    return false;
}

static bool handle_register(NightAuthApi* api) {
    char user[128] = {0};
    char pass[128] = {0};
    char key[128] = {0};
    char email[128] = {0};

    print_section_header("CREATE NEW ACCOUNT", "Activate a license key and bind your credentials");
    ask("Username: ", user, sizeof(user));
    ask("Password: ", pass, sizeof(pass));
    ask("License Key: ", key, sizeof(key));
    ask("Email Address " C_DIM "(Optional, Enter to skip)" C_TEXT ": ", email, sizeof(email));

    if (nightauth_register(api, user, pass, key, email)) {
        print_ok("Account Registered & Activated", "Session started automatically.");
        return true;
    }

    char detail[320];
    snprintf(detail, sizeof(detail), "%s%s%s%s",
             nightauth_get_response(api)->message,
             nightauth_get_response(api)->error[0] ? " [" : "",
             nightauth_get_response(api)->error,
             nightauth_get_response(api)->error[0] ? "]" : "");
    print_err("Registration Failed", detail);
    return false;
}

static bool handle_license_only(NightAuthApi* api) {
    char key[128] = {0};
    print_section_header("LICENSE KEY AUTHENTICATION", "Instant passwordless login bound to your HWID");
    ask("Enter License Key: ", key, sizeof(key));

    if (nightauth_license(api, key)) {
        print_ok("License Validated", "Direct hardware-bound session active.");
        return true;
    }

    print_err("License Validation Failed", nightauth_get_response(api)->message);
    return false;
}

static bool handle_upgrade(NightAuthApi* api, const char* known_user) {
    char user[128] = {0};
    char key[128] = {0};

    print_section_header("UPGRADE ACCOUNT SUBSCRIPTION", "Attach a new license key tier to an account");
    if (known_user && known_user[0]) {
        strncpy(user, known_user, sizeof(user) - 1);
        print_info("Target Account:", user);
    } else {
        ask("Target Username: ", user, sizeof(user));
    }
    ask("Upgrade License Key: ", key, sizeof(key));

    if (nightauth_upgrade(api, user, key)) {
        print_ok("Upgrade Complete", nightauth_get_response(api)->message);
        return true;
    } else {
        print_err("Upgrade Failed", nightauth_get_response(api)->message);
        return false;
    }
}

static void handle_forgot_password(NightAuthApi* api) {
    char email[128] = {0};
    print_section_header("PASSWORD RECOVERY", "Request a reset link to your registered email");
    ask("Registered Email: ", email, sizeof(email));

    if (nightauth_forgot_password(api, email)) {
        print_ok("Recovery Dispatched", "If an account matches, a password reset link was sent to your email.");
    } else {
        print_err("Request Failed", nightauth_get_response(api)->message);
    }
}

static void handle_reset_password(NightAuthApi* api) {
    char token[128] = {0};
    char new_pass[128] = {0};
    print_section_header("COMPLETE PASSWORD RESET", "Confirm recovery token and set a new password");
    ask("Reset Token: ", token, sizeof(token));
    ask("New Password: ", new_pass, sizeof(new_pass));

    if (nightauth_reset_password(api, token, new_pass)) {
        print_ok("Password Updated", "Your password has been reset successfully.");
    } else {
        print_err("Reset Failed", nightauth_get_response(api)->message);
    }
}

static void handle_reset_hwid(NightAuthApi* api) {
    char user[128] = {0};
    char pass[128] = {0};
    char totp[32] = {0};

    print_section_header("RESET HARDWARE FINGERPRINT (HWID)", "Unbind previous device to authorize a new machine");
    ask("Username: ", user, sizeof(user));
    ask("Password: ", pass, sizeof(pass));
    ask("2FA Code " C_DIM "(Optional, Enter to skip)" C_TEXT ": ", totp, sizeof(totp));

    if (nightauth_reset_hwid(api, user, pass, totp[0] ? totp : NULL)) {
        print_ok("HWID Reset", nightauth_get_response(api)->message);
    } else {
        print_err("HWID Reset Failed", nightauth_get_response(api)->message);
    }
}

static void handle_check_blacklist(NightAuthApi* api) {
    char target_hwid[128] = {0};
    print_section_header("HARDWARE BLACKLIST CHECK", "Verify if a machine fingerprint is restricted");
    ask("Hardware ID " C_DIM "(Press Enter for local machine HWID)" C_TEXT ": ", target_hwid, sizeof(target_hwid));
    if (!target_hwid[0]) {
        nightauth_get_hardware_id(target_hwid, sizeof(target_hwid));
    }

    bool is_blacklisted = false;
    if (nightauth_check_blacklist(api, target_hwid, &is_blacklisted)) {
        if (is_blacklisted) {
            print_err("Blacklisted", "This Hardware ID is currently restricted.");
        } else {
            print_ok("Clean Fingerprint", "Hardware ID is not on the blacklist.");
        }
    } else {
        print_err("Blacklist Query Failed", nightauth_get_response(api)->message);
    }
}

static void handle_verify_hash(NightAuthApi* api) {
    char hash[128] = {0};
    print_section_header("BINARY INTEGRITY CHECK", "Validate SHA-256 checksum against server registry");
    ask("SHA-256 File Hash: ", hash, sizeof(hash));
    if (nightauth_verify_file_hash(api, hash)) {
        print_ok("Integrity Verified", "Checksum matches authorized server release.");
    } else {
        print_err("Verification Failed", nightauth_get_response(api)->message);
    }
}

static bool handle_authentication_flow(NightAuthApi* api) {
    while (true) {
        print_section_header("AUTHENTICATION & PRE-SESSION GATEWAY", "Select an operation to continue");

        printf("  " C_BADGE_BG C_GLOW C_BOLD " 01 " C_RESET " " C_WHITE "Sign In (Username & Password)" C_RESET
               "          " C_BADGE_BG C_GLOW C_BOLD " 06 " C_RESET " " C_WHITE "Complete Password Reset (Token)" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 02 " C_RESET " " C_WHITE "Register New Account + License" C_RESET
               "         " C_BADGE_BG C_GLOW C_BOLD " 07 " C_RESET " " C_WHITE "Reset Hardware Binding (HWID)" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 03 " C_RESET " " C_WHITE "Instant License Key Login" C_RESET
               "              " C_BADGE_BG C_GLOW C_BOLD " 08 " C_RESET " " C_WHITE "Check HWID Blacklist Status" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 04 " C_RESET " " C_WHITE "Upgrade Existing Account Plan" C_RESET
               "          " C_BADGE_BG C_GLOW C_BOLD " 09 " C_RESET " " C_WHITE "Verify Binary SHA-256 Hash" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 05 " C_RESET " " C_WHITE "Forgot Password (Email Link)" C_RESET
               "           " C_RED_BG C_RED C_BOLD " 10 " C_RESET " " C_MUTED "Exit Application" C_RESET "\n\n");

        char sel[16] = {0};
        ask("Select option [1-10]: ", sel, sizeof(sel));

        if (strcmp(sel, "1") == 0 || strcmp(sel, "01") == 0) {
            if (handle_login(api)) return true;
        } else if (strcmp(sel, "2") == 0 || strcmp(sel, "02") == 0) {
            if (handle_register(api)) return true;
        } else if (strcmp(sel, "3") == 0 || strcmp(sel, "03") == 0) {
            if (handle_license_only(api)) return true;
        } else if (strcmp(sel, "4") == 0 || strcmp(sel, "04") == 0) {
            handle_upgrade(api, NULL);
        } else if (strcmp(sel, "5") == 0 || strcmp(sel, "05") == 0) {
            handle_forgot_password(api);
        } else if (strcmp(sel, "6") == 0 || strcmp(sel, "06") == 0) {
            handle_reset_password(api);
        } else if (strcmp(sel, "7") == 0 || strcmp(sel, "07") == 0) {
            handle_reset_hwid(api);
        } else if (strcmp(sel, "8") == 0 || strcmp(sel, "08") == 0) {
            handle_check_blacklist(api);
        } else if (strcmp(sel, "9") == 0 || strcmp(sel, "09") == 0) {
            handle_verify_hash(api);
        } else if (strcmp(sel, "10") == 0 || strcmp(sel, "0") == 0) {
            return false;
        }
    }
}

static void render_user_profile(NightAuthApi* api) {
    NightAuthUserData* u = nightauth_get_user(api);
    printf("\n");
    printf("  " C_BORDER "╭─ " C_GLOW C_BOLD "AUTHENTICATED IDENTITY & ENTITLEMENTS" C_BORDER " ──────────────────────────────────╮" C_RESET "\n");
    printf("  " C_BORDER "│" C_RESET "  " C_DIM "Username :" C_RESET " " C_WHITE C_BOLD "%-24.24s" C_RESET "  " C_DIM "Status     :" C_RESET " " C_GREEN C_BOLD "%-20.20s" C_RESET " " C_BORDER "│" C_RESET "\n",
           u->username[0] ? u->username : "Authenticated",
           u->status[0] ? u->status : "ACTIVE");
    printf("  " C_BORDER "│" C_RESET "  " C_DIM "Email    :" C_RESET " " C_TEXT "%-24.24s" C_RESET "  " C_DIM "Client IP  :" C_RESET " " C_CYAN "%-20.20s" C_RESET " " C_BORDER "│" C_RESET "\n",
           u->email[0] ? u->email : "Not linked",
           u->ip[0] ? u->ip : "Protected / TLS");
    printf("  " C_BORDER "│" C_RESET "  " C_DIM "Created  :" C_RESET " " C_MUTED "%-24.24s" C_RESET "  " C_DIM "Last Login :" C_RESET " " C_MUTED "%-20.20s" C_RESET " " C_BORDER "│" C_RESET "\n",
           u->created_at[0] ? u->created_at : "-",
           u->last_login_at[0] ? u->last_login_at : "Just now");
    printf("  " C_BORDER "├──────────────────────────────────────────────────────────────────────────┤" C_RESET "\n");

    if (u->subscription_count > 0) {
        for (int i = 0; i < u->subscription_count; i++) {
            printf("  " C_BORDER "│" C_RESET "  " C_GREEN "●" C_RESET " " C_WHITE C_BOLD "%-16.16s" C_RESET " " C_DIM "Key:" C_RESET " " C_CYAN "%-22.22s" C_RESET " " C_DIM "Exp:" C_RESET " " C_AMBER "%-16.16s" C_RESET " " C_BORDER "│" C_RESET "\n",
                   u->subscriptions[i].subscription,
                   u->subscriptions[i].key[0] ? u->subscriptions[i].key : "Direct Entitlement",
                   u->subscriptions[i].expiry[0] ? u->subscriptions[i].expiry : "Lifetime");
        }
    } else {
        printf("  " C_BORDER "│" C_RESET "  " C_DIM "No active subscription tiers attached to this account." C_RESET "                   " C_BORDER "│" C_RESET "\n");
    }
    printf("  " C_BORDER "╰──────────────────────────────────────────────────────────────────────────╯" C_RESET "\n");
}

static void handle_chat_room(NightAuthApi* api) {
    char room[64] = {0};
    print_section_header("LIVE CHANNEL CHAT", "Type /exit at any time to leave the channel");
    ask("Channel name (e.g. general): ", room, sizeof(room));
    if (!room[0]) strcpy(room, "general");

    NightAuthChatMessage msgs[10];
    int count = nightauth_get_chat_messages(api, room, msgs, 10);
    printf("\n  " C_BORDER "╭─ " C_GLOW C_BOLD "#%-20s" C_BORDER " ──────────────────────────────────────────────────╮" C_RESET "\n", room);
    if (count <= 0) {
        printf("  " C_BORDER "│" C_RESET "  " C_DIM "No messages in this channel yet. Start the conversation below." C_RESET "          " C_BORDER "│" C_RESET "\n");
    } else {
        for (int i = 0; i < count; i++) {
            printf("  " C_BORDER "│" C_RESET " " C_DIM "[%s]" C_RESET " " C_CYAN C_BOLD "%s" C_RESET ": " C_TEXT "%s" C_RESET "\n",
                   msgs[i].timestamp, msgs[i].author, msgs[i].message);
        }
    }
    printf("  " C_BORDER "╰──────────────────────────────────────────────────────────────────────────╯" C_RESET "\n\n");

    while (true) {
        char prompt[128];
        snprintf(prompt, sizeof(prompt), C_CYAN "#%s" C_RESET " message: ", room);
        char input[256] = {0};
        ask(prompt, input, sizeof(input));
        if (
#ifdef _WIN32
            _stricmp(input, "/exit") == 0
#else
            strcasecmp(input, "/exit") == 0
#endif
        ) {
            printf("  " C_DIM "Left channel #%s." C_RESET "\n\n", room);
            return;
        }
        if (!input[0]) continue;
        if (!nightauth_send_chat_message(api, input, room)) {
            print_err("Send Failed", nightauth_get_response(api)->message);
        }
    }
}

static void render_stats_table(NightAuthApi* api) {
    NightAuthAppStats stats;
    if (nightauth_get_stats(api, &stats)) {
        printf("\n");
        printf("  " C_BORDER "╭─ " C_GLOW C_BOLD "APPLICATION LIVE TELEMETRY" C_BORDER " ─────────────────────────────────────────────╮" C_RESET "\n");
        printf("  " C_BORDER "│" C_RESET "  " C_DIM "Total Users     :" C_RESET " " C_WHITE C_BOLD "%-14lld" C_RESET "  " C_DIM "Total Licenses  :" C_RESET " " C_WHITE C_BOLD "%-18lld" C_RESET " " C_BORDER "│" C_RESET "\n",
               (long long)stats.total_users, (long long)stats.total_licenses);
        printf("  " C_BORDER "│" C_RESET "  " C_DIM "Active Users    :" C_RESET " " C_GREEN C_BOLD "%-14lld" C_RESET "  " C_DIM "Used Licenses   :" C_RESET " " C_CYAN C_BOLD "%-18lld" C_RESET " " C_BORDER "│" C_RESET "\n",
               (long long)stats.active_users, (long long)stats.used_licenses);
        printf("  " C_BORDER "│" C_RESET "  " C_DIM "Banned Users    :" C_RESET " " C_RED C_BOLD "%-14lld" C_RESET "  " C_DIM "Unused Licenses :" C_RESET " " C_AMBER C_BOLD "%-18lld" C_RESET " " C_BORDER "│" C_RESET "\n",
               (long long)stats.banned_users, (long long)stats.unused_licenses);
        printf("  " C_BORDER "│" C_RESET "  " C_DIM "Target Version  :" C_RESET " " C_GLOW "%-52s" C_RESET " " C_BORDER "│" C_RESET "\n",
               stats.version[0] ? stats.version : APP_VERSION);
        printf("  " C_BORDER "╰──────────────────────────────────────────────────────────────────────────╯" C_RESET "\n\n");
    } else {
        print_err("Telemetry Error", nightauth_get_response(api)->message);
    }
}

static void handle_setup_2fa(NightAuthApi* api) {
    print_section_header("TWO-FACTOR AUTHENTICATION (TOTP)", "Enroll an authenticator app (RFC 6238)");
    NightAuthTwoFactorSetupData setup;
    if (nightauth_setup_2fa(api, &setup)) {
        print_info("TOTP Secret Key:", setup.secret);
        print_info("Provisioning URI:", setup.uri);
        char code[32] = {0};
        ask("Enter 6-digit Authenticator code: ", code, sizeof(code));
        NightAuthTwoFactorConfirmData conf;
        if (nightauth_confirm_2fa(api, code, &conf)) {
            print_ok("2FA Activated", "Store these recovery backup codes safely:");
            for (int i = 0; i < conf.backup_code_count; i++) {
                printf("    " C_AMBER "• %s" C_RESET "\n", conf.backup_codes[i]);
            }
            printf("\n");
        } else {
            print_err("Verification Failed", nightauth_get_response(api)->message);
        }
    } else {
        print_err("2FA Setup Failed", nightauth_get_response(api)->message);
    }
}

static void handle_download_file(NightAuthApi* api) {
    char file_id[64] = {0};
    print_section_header("PROTECTED ASSET DOWNLOADER", "Download encrypted files via scoped access ticket");
    ask("Enter File ID or Asset Name: ", file_id, sizeof(file_id));

    char ticket[128] = {0};
    int64_t expires_in = 0;
    if (!nightauth_access_ticket(api, "file", file_id, ticket, sizeof(ticket), &expires_in)) {
        print_err("Access Ticket Denied", nightauth_get_response(api)->message);
        return;
    }

    char out_name[128] = {0};
    size_t cap = 4 * 1024 * 1024;
    uint8_t* buffer = (uint8_t*)malloc(cap);
    if (!buffer) {
        print_err("Memory Error", "Could not allocate download buffer.");
        return;
    }

    size_t data_len = 0;
    if (nightauth_download_file(api, ticket, out_name, buffer, cap, &data_len)) {
        const char* saved_as = out_name[0] ? out_name : "downloaded_file.bin";
        FILE* fp = fopen(saved_as, "wb");
        if (fp) {
            fwrite(buffer, 1, data_len, fp);
            fclose(fp);
            char msg[256];
            snprintf(msg, sizeof(msg), "Saved '%s' (%zu bytes) to disk.", saved_as, data_len);
            print_ok("Download Complete", msg);
        }
    } else {
        print_err("Download Failed", nightauth_get_response(api)->message);
    }
    free(buffer);
}

static void handle_list_all_user_vars(NightAuthApi* api) {
    NightAuthUserVar vars[32];
    int count = nightauth_get_user_variables(api, vars, 32);
    if (count < 0) {
        print_err("Fetch Failed", nightauth_get_response(api)->message);
        return;
    }
    print_section_header("USER CLOUD VARIABLES", "Per-user encrypted key-value pairs");
    if (count == 0) {
        printf("  " C_DIM "No user variables stored for this account yet." C_RESET "\n\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("  " C_BLUE "•" C_RESET " " C_CYAN C_BOLD "%-22s" C_RESET " = " C_WHITE "%s" C_RESET "\n", vars[i].key, vars[i].value);
    }
    printf("\n");
}

static void handle_trigger_webhook(NightAuthApi* api) {
    char web_id[64] = {0};
    char message[256] = {0};
    print_section_header("SERVER-SIDE WEBHOOK TRIGGER", "Execute an authenticated webhook endpoint");
    ask("Webhook ID: ", web_id, sizeof(web_id));
    ask("JSON / Message Payload: ", message, sizeof(message));

    int status = 0;
    if (nightauth_webhook(api, web_id, message, NULL, &status)) {
        char detail[128];
        snprintf(detail, sizeof(detail), "HTTP %d OK", status);
        print_ok("Webhook Dispatched", detail);
    } else {
        print_err("Webhook Failed", nightauth_get_response(api)->message);
    }
}

static bool console_event_cb(const char* event_json, void* user) {
    (void)user;
    printf("  " C_BADGE_BG C_GLOW " EVENT " C_RESET " " C_CYAN "%s" C_RESET "\n", event_json);
    return true;
}

#ifdef _WIN32
static DWORD WINAPI console_cancel_thread(LPVOID param) {
    char dummy[16] = {0};
    fgets(dummy, sizeof(dummy), stdin);
    nightauth_events_cancel((NightAuthApi*)param);
    return 0;
}
#endif

static void handle_events_feed(NightAuthApi* api) {
    char ticket[128] = {0};
    int64_t expires_in = 0;
    if (!nightauth_access_ticket(api, "events", NULL, ticket, sizeof(ticket), &expires_in)) {
        print_err("Events Ticket Denied", nightauth_get_response(api)->message);
        return;
    }

    print_section_header("REAL-TIME APPLICATION EVENTS (SSE)", "Press Enter at any time to stop listening");

#ifdef _WIN32
    DWORD tid = 0;
    HANDLE h = CreateThread(NULL, 0, console_cancel_thread, api, 0, &tid);
    if (h) CloseHandle(h);
#endif

    if (nightauth_events_run(api, ticket, NULL, console_event_cb, NULL)) {
        print_ok("Stream Closed", "Disconnected from real-time event feed.");
    } else {
        print_err("Stream Interrupted", nightauth_get_response(api)->message);
    }
}

static void handle_self_ban(NightAuthApi* api) {
    char reason[256] = {0};
    print_section_header("EMERGENCY KILLSWITCH / BAN", "Report security violation and lock this session");
    ask("Ban Reason " C_DIM "(Optional)" C_TEXT ": ", reason, sizeof(reason));

    if (nightauth_ban(api, reason[0] ? reason : NULL)) {
        print_ok("Killswitch Executed", nightauth_get_response(api)->message);
    } else {
        print_err("Ban Report Failed", nightauth_get_response(api)->message);
    }
}

static void handle_heartbeat(NightAuthApi* api) {
    if (nightauth_heartbeat(api)) {
        print_ok("Heartbeat Acknowledged", nightauth_get_response(api)->message);
    } else {
        print_err("Heartbeat Failed", nightauth_get_response(api)->message);
    }
}

static void handle_kill_session_demo(NightAuthApi* api) {
    print_section_header("SESSION REVOCATION & INVALIDATION VERIFICATION", "3-Step Live Security Audit");
    printf("  " C_MUTED "1. Verifying active session before revocation..." C_RESET "\n");
    if (nightauth_check(api)) {
        print_ok("Step 1 Passed", "Session is active and authenticated on server.");
    } else {
        print_err("Step 1 Failed", nightauth_get_response(api)->message);
        return;
    }

    printf("  " C_MUTED "2. Revoking and terminating session on server (/logout)..." C_RESET "\n");
    if (nightauth_logout(api)) {
        print_ok("Step 2 Passed", "Server confirmed session revocation.");
    } else {
        print_err("Step 2 Failed", nightauth_get_response(api)->message);
    }

    printf("  " C_MUTED "3. Verifying that subsequent session check is rejected..." C_RESET "\n");
    if (nightauth_check(api)) {
        print_err("Step 3 Unexpected", "Session was still accepted after revocation.");
    } else {
        print_ok("Step 3 Passed", "Server rejected revoked session as expected.");
    }
}

static void run_application_features_menu(NightAuthApi* api) {
    while (true) {
        print_section_header("WORKSPACE MODULES & ACTIONS", "Select a command [1-21]");

        printf("  " C_DIM "CLOUD VARIABLES & CHAT" C_RESET "                    " C_DIM "SECURITY & TELEMETRY" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 01 " C_RESET " " C_WHITE "Read Global Variable" C_RESET
               "                 " C_BADGE_BG C_GLOW C_BOLD " 10 " C_RESET " " C_WHITE "Setup Two-Factor Auth (2FA)" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 02 " C_RESET " " C_WHITE "Get User Cloud Variable" C_RESET
               "              " C_BADGE_BG C_GLOW C_BOLD " 11 " C_RESET " " C_WHITE "Disable Two-Factor Auth (2FA)" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 03 " C_RESET " " C_WHITE "Set User Cloud Variable" C_RESET
               "              " C_BADGE_BG C_GLOW C_BOLD " 12 " C_RESET " " C_WHITE "Download Protected Asset" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 04 " C_RESET " " C_WHITE "List All User Variables" C_RESET
               "              " C_BADGE_BG C_GLOW C_BOLD " 13 " C_RESET " " C_WHITE "Trigger Server Webhook" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 05 " C_RESET " " C_WHITE "Enter Live Chat Channel" C_RESET
               "              " C_BADGE_BG C_GLOW C_BOLD " 14 " C_RESET " " C_WHITE "Send Audit Log Event" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 19 " C_RESET " " C_WHITE "Fetch Paginated Chat History" C_RESET
               "         " C_BADGE_BG C_GLOW C_BOLD " 15 " C_RESET " " C_WHITE "Verify Session Entitlement" C_RESET "\n\n");

        printf("  " C_DIM "ACCOUNT & INSIGHTS" C_RESET "                        " C_DIM "SESSION LIFECYCLE" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 06 " C_RESET " " C_WHITE "Application Live Statistics" C_RESET
               "          " C_BADGE_BG C_GLOW C_BOLD " 16 " C_RESET " " C_WHITE "Send Heartbeat Ping" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 07 " C_RESET " " C_WHITE "View Online Active Users" C_RESET
               "             " C_BADGE_BG C_GLOW C_BOLD " 17 " C_RESET " " C_WHITE "Emergency Killswitch / Ban" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 08 " C_RESET " " C_WHITE "Change Account Username" C_RESET
               "              " C_BADGE_BG C_GLOW C_BOLD " 18 " C_RESET " " C_WHITE "Stream Live Events (SSE)" C_RESET "\n");
        printf("  " C_BADGE_BG C_GLOW C_BOLD " 09 " C_RESET " " C_WHITE "Upgrade Plan with License Key" C_RESET
               "        " C_BADGE_BG C_GLOW C_BOLD " 20 " C_RESET " " C_WHITE "Session Revocation Audit Demo" C_RESET "\n");
        printf("                                            " C_RED_BG C_RED C_BOLD " 21 " C_RESET " " C_MUTED "Sign Out & Terminate Session" C_RESET "\n\n");

        char choice[16] = {0};
        ask("Action [1-21]: ", choice, sizeof(choice));
        int num = atoi(choice);

        if (num == 1) {
            char gKey[64] = {0};
            char gVal[256] = {0};
            ask("Global Variable Key: ", gKey, sizeof(gKey));
            if (nightauth_get_global_var(api, gKey, gVal, sizeof(gVal))) {
                print_ok(gKey, gVal);
            } else {
                print_err("Fetch Failed", nightauth_get_response(api)->message);
            }
        } else if (num == 2) {
            char uKey[64] = {0};
            char uVal[256] = {0};
            ask("User Variable Key: ", uKey, sizeof(uKey));
            if (nightauth_get_user_var(api, uKey, uVal, sizeof(uVal))) {
                print_ok(uKey, uVal);
            } else {
                print_err("Fetch Failed", nightauth_get_response(api)->message);
            }
        } else if (num == 3) {
            char sKey[64] = {0};
            char sVal[256] = {0};
            ask("User Variable Key: ", sKey, sizeof(sKey));
            ask("Value to Store: ", sVal, sizeof(sVal));
            if (nightauth_set_user_var(api, sKey, sVal)) {
                print_ok("Variable Saved", sKey);
            } else {
                print_err("Save Failed", nightauth_get_response(api)->message);
            }
        } else if (num == 4) {
            handle_list_all_user_vars(api);
        } else if (num == 5) {
            handle_chat_room(api);
        } else if (num == 6) {
            render_stats_table(api);
        } else if (num == 7) {
            char users[16][128];
            int64_t total_online = 0;
            int count = nightauth_get_online_users(api, &total_online, users, 16);
            char hdr[64];
            snprintf(hdr, sizeof(hdr), "%lld active session(s)", (long long)total_online);
            print_section_header("ONLINE ACTIVE SESSIONS", hdr);
            for (int i = 0; i < count; i++) {
                printf("  " C_GREEN "●" C_RESET " " C_WHITE "%s" C_RESET "\n", users[i]);
            }
            printf("\n");
        } else if (num == 8) {
            char newU[128] = {0};
            char p1[128] = {0};
            ask("New Username: ", newU, sizeof(newU));
            ask("Confirm Password: ", p1, sizeof(p1));
            if (nightauth_change_username(api, newU, p1)) {
                print_ok("Username Updated", nightauth_get_user(api)->username);
                printf("  " C_AMBER "For security, your session has been closed. Please sign in with your new username." C_RESET "\n\n");
                return;
            } else {
                print_err("Update Failed", nightauth_get_response(api)->message);
            }
        } else if (num == 9) {
            if (handle_upgrade(api, nightauth_get_user(api)->username)) {
                nightauth_check(api);
                render_user_profile(api);
            }
        } else if (num == 10) {
            handle_setup_2fa(api);
        } else if (num == 11) {
            char p3[128] = {0};
            char code3[32] = {0};
            ask("Account Password: ", p3, sizeof(p3));
            ask("6-digit Authenticator Code: ", code3, sizeof(code3));
            if (nightauth_disable_2fa(api, p3, code3)) {
                print_ok("2FA Disabled", "Two-factor protection has been removed from this account.");
            } else {
                print_err("Disable Failed", nightauth_get_response(api)->message);
            }
        } else if (num == 12) {
            handle_download_file(api);
        } else if (num == 13) {
            handle_trigger_webhook(api);
        } else if (num == 14) {
            char logMsg[256] = {0};
            ask("Audit Log Message: ", logMsg, sizeof(logMsg));
            if (nightauth_log(api, logMsg)) {
                print_ok("Audit Log Recorded", "Delivered to application telemetry feed.");
            } else {
                print_err("Log Failed", nightauth_get_response(api)->message);
            }
        } else if (num == 15) {
            if (nightauth_check(api)) {
                print_ok("Session Verified", "Active and cryptographically valid.");
            } else {
                print_err("Session Invalid", nightauth_get_response(api)->message);
            }
        } else if (num == 16) {
            handle_heartbeat(api);
        } else if (num == 17) {
            handle_self_ban(api);
        } else if (num == 18) {
            handle_events_feed(api);
        } else if (num == 19) {
            char hist_room[64] = {0};
            char hist_before[32] = {0};
            char hist_mr[8] = {0};
            ask("Channel Name: ", hist_room, sizeof(hist_room));
            ask("Before Message ID " C_DIM "(Optional, Enter for latest)" C_TEXT ": ", hist_before, sizeof(hist_before));
            ask("Mark messages as read? (y/n): ", hist_mr, sizeof(hist_mr));

            NightAuthChatMessage hist[25];
            int64_t before_id = hist_before[0] ? atoll(hist_before) : 0;
            int hist_count = nightauth_get_chat_messages_ex(api, hist_room, before_id,
                                                            hist_mr[0] == 'y' || hist_mr[0] == 'Y',
                                                            hist, 25);
            if (hist_count < 0) {
                print_err("History Query Failed", nightauth_get_response(api)->message);
            } else {
                char sub[64];
                snprintf(sub, sizeof(sub), "%d message(s) retrieved", hist_count);
                print_section_header(hist_room, sub);
                for (int i = 0; i < hist_count; i++) {
                    printf("  " C_DIM "[%s]" C_RESET " " C_CYAN C_BOLD "%s" C_RESET ": " C_TEXT "%s" C_RESET " " C_DIM "(#%s · %s)" C_RESET "\n",
                           hist[i].timestamp, hist[i].author, hist[i].message,
                           hist[i].id, hist[i].status[0] ? hist[i].status : "-");
                }
                printf("\n");
            }
        } else if (num == 20) {
            handle_kill_session_demo(api);
            return;
        } else if (num == 21) {
            nightauth_logout(api);
            print_ok("Signed Out", "Session revoked cleanly. Goodbye!");
            return;
        }
    }
}

int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitleA("NightAuth — Native C Security Console");
#endif

    const char* env_name = getenv("NIGHTAUTH_APP_NAME");
    const char* env_key = getenv("NIGHTAUTH_API_KEY");
    const char* env_ver = getenv("NIGHTAUTH_APP_VERSION");
    const char* env_url = getenv("NIGHTAUTH_API_URL");
    const char* app_name = (env_name && env_name[0]) ? env_name : APP_NAME;
    const char* app_key = (env_key && env_key[0]) ? env_key : APP_API_KEY;
    const char* app_ver = (env_ver && env_ver[0]) ? env_ver : APP_VERSION;

    clear_screen();
    render_banner(app_name, NULL, false);

    char prompted_name[128] = {0};
    char prompted_key[128] = {0};
    if (!app_name[0]) {
        ask("Enter App Name: ", prompted_name, sizeof(prompted_name));
        app_name = prompted_name;
    }
    if (!app_key[0]) {
        ask("Enter API Key: ", prompted_key, sizeof(prompted_key));
        app_key = prompted_key;
    }

    NightAuthApi* api = nightauth_create(app_name, app_key, app_ver, APP_TOKEN, env_url);
    if (!api) {
        print_err("Initialization Error", "Failed to configure NightAuth client.");
        return 1;
    }
    perform_handshake(api, app_name);

    if (!handle_authentication_flow(api)) {
        nightauth_destroy(api);
        return 0;
    }

    char hwid[128] = {0};
    nightauth_get_hardware_id(hwid, sizeof(hwid));
    clear_screen();
    render_banner(app_name, hwid, true);
    render_user_profile(api);

    run_application_features_menu(api);
    nightauth_destroy(api);
    return 0;
}
