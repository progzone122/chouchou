#include "include/arm.h"
#include "include/printf.h"
#include "include/string.h"
#include "include/fastboot.h"
#include "include/common.h"
#include "include/commands.h"

void original_flash(const char *arg, void *data, unsigned sz) {
    ((void (*)(const char *arg, void *data, unsigned sz))(0x4c436a18 | 1))(arg, data, sz);
}

void original_erase(const char *arg, void *data, unsigned sz) {
    ((void (*)(const char *arg, void *data, unsigned sz))(0x4c436695 | 1))(arg, data, sz);
}

void cmd_hexdump(const char *arg, void *data, unsigned sz) {
    if (!arg || !*arg) {
        fastboot_fail("Usage: hexdump <addr> <size>");
        return;
    }

    // TODO: Do better memory management
    char args[128];
    strncpy(args, arg, sizeof(args) - 1);
    args[sizeof(args) - 1] = '\0';

    char *a = strtok(args, " ");
    char *s = strtok(NULL, " ");

    if (a && s) {
        long al = strtol(a, NULL, 0);
        long sl = strtol(s, NULL, 0);

        if (al == 0 || sl <= 0) {
            fastboot_fail("Invalid address or size");
            return;
        }

        video_hexdump((void *)al, sl);
    } else {
        fastboot_fail("Usage: hexdump <addr> <size>");
    }

    fastboot_okay("");
}

void cmd_help(const char *arg, void *data, unsigned sz) {
    struct fastboot_cmd *cmd = NULL;

    if (!cmd) {
        fastboot_fail("No commands found!");
        return;
    }

    fastboot_info("\nAvailable oem commands:");
    while (cmd) {
        if (cmd->prefix) {
            if (strncmp(cmd->prefix, "oem", 3) == 0) {
                fastboot_info(cmd->prefix);
            }
        }
        cmd = cmd->next;
    }
    fastboot_okay("");
}

void cmd_flash(const char *arg, void *data, unsigned sz) {
    if (!arg || *arg == '\0') {
        fastboot_fail("Invalid argument!");
        return;
    }

    if (fastboot_is_protected_partition(arg)) {
        fastboot_fail("Partition is protected");
        return;
    }

    original_flash(arg, data, sz);
}

void cmd_erase(const char *arg, void *data, unsigned sz) {
    if (!arg || *arg == '\0') {
        fastboot_fail("Invalid argument!");
        return;
    }

    if (fastboot_is_protected_partition(arg)) {
        fastboot_fail("Partition is protected");
        return;
    }

    original_erase(arg, data, sz);
}

void cmd_flashing_lock(const char *arg, void *data, unsigned sz) {
    fastboot_info("");
    fastboot_info("To lock the bootloader, you need to flash");
    fastboot_info("stock firmare through SP Flash Tool first");
    fastboot_info("");
    fastboot_fail(":(");
}

void register_commands() {
    uint32_t vbar = READ_VBAR() & ~0xFFF;
    static char membase_str[11];
    int_to_hex_str(vbar, membase_str);

    fastboot_publish("membase", membase_str);
    fastboot_publish("chouchou-version", VERSION);

    fastboot_register("oem hexdump", cmd_hexdump, 1);
    fastboot_register("oem help", cmd_help, 1);
    fastboot_register("flash:", cmd_flash, 1);
    fastboot_register("erase:", cmd_erase, 1);
    fastboot_register("flashing lock", cmd_flashing_lock, 1);

    LOGD("Successfully registered fastboot commands!\n");
}