/* WCSCLI-compatible grammar for direct BMC access. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ipmitool/ipmi_intf.h>
#include <ipmitool/ipmi_main.h>
#include <ipmitool/ipmi_ocs_console.h>
#include <ipmitool/ipmi_sel.h>
#include <ipmitool/ipmi_wcscli.h>
#include <ipmitool/log.h>

#define WCSCLI_MAX_ARGS 32

static int
run(struct ipmi_intf *intf, char *command, int argc, char **argv)
{
	return ipmi_cmd_run(intf, command, argc, argv);
}

static int
same(int argc, char **argv, int count, const char **words)
{
	int i;

	if (argc < count)
		return 0;
	for (i = 0; i < count; ++i) {
		if (strcmp(argv[i], words[i]) != 0)
			return 0;
	}
	return 1;
}

static void
usage(void)
{
	lprintf(LOG_NOTICE, "usage: wcscli show system <feature> [-i 1-48]");
	lprintf(LOG_NOTICE, "       wcscli set system <action> [options] [-i 1-48]");
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Show: info, health, fru, nvme, state, nextboot, led,");
	lprintf(LOG_NOTICE, "      log read, bios config, bios code, tpm presence");
	lprintf(LOG_NOTICE, "Set:  on, off, reset, nextboot, led, log clear,");
	lprintf(LOG_NOTICE, "      bios config, tpm presence, console bmc|host");
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "-i is accepted for Rack Manager syntax compatibility.");
	lprintf(LOG_NOTICE, "In direct-BMC mode, -H selects the BMC; -i is not routed.");
}

/* Remove RM's instance option before feature parsing.  ipmitool has already
 * parsed global -I, so upper-case -I and lower-case -i never share a parser. */
static int
filter_instance(int argc, char **argv, int *outc, char **outv)
{
	int i;
	char *end;
	long instance;

	*outc = 0;
	for (i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "-i") == 0) {
			if (++i >= argc) {
				lprintf(LOG_ERR, "Missing value after -i");
				return -1;
			}
			instance = strtol(argv[i], &end, 10);
			if (end == argv[i] || *end != '\0' || instance < 1 ||
				instance > 48) {
				lprintf(LOG_ERR, "Invalid instance '%s'; expected 1-48",
					argv[i]);
				return -1;
			}
			continue;
		}
		if (*outc >= WCSCLI_MAX_ARGS) {
			lprintf(LOG_ERR, "Too many WCSCLI arguments");
			return -1;
		}
		outv[(*outc)++] = argv[i];
	}
	return 0;
}

static char *
option_value(int argc, char **argv, const char *option)
{
	int i;

	for (i = 0; i + 1 < argc; ++i) {
		if (strcmp(argv[i], option) == 0)
			return argv[i + 1];
	}
	return NULL;
}

static char *
boot_device(const char *value)
{
	if (strcmp(value, "1") == 0 || strcmp(value, "none") == 0)
		return "none";
	if (strcmp(value, "2") == 0 || strcmp(value, "pxe") == 0)
		return "pxe";
	if (strcmp(value, "3") == 0 || strcmp(value, "disk") == 0)
		return "disk";
	if (strcmp(value, "4") == 0 || strcmp(value, "bios") == 0)
		return "bios";
	if (strcmp(value, "5") == 0 || strcmp(value, "floppy") == 0)
		return "floppy";
	return NULL;
}

static int
show_system(struct ipmi_intf *intf, int argc, char **argv)
{
	char *a1[1];
	const char *info[] = { "info" };
	const char *health[] = { "health" };
	const char *fru[] = { "fru" };
	const char *nvme[] = { "nvme" };
	const char *state[] = { "state" };
	const char *nextboot[] = { "nextboot" };
	const char *led[] = { "led" };
	const char *log[] = { "log" };
	const char *logread[] = { "log", "read" };
	const char *biosconfig[] = { "bios", "config" };
	const char *bioscode[] = { "bios", "code" };
	const char *tpm[] = { "tpm", "presence" };
	char *version;
	int rc;

	if (same(argc, argv, 1, info) && argc == 1) {
		a1[0] = "info";
		rc = run(intf, "mc", 1, a1);
		if (rc != 0)
			return rc;
		return run(intf, "fru", 0, NULL);
	}
	if (same(argc, argv, 1, health) && argc == 1) {
		a1[0] = "elist";
		return run(intf, "sdr", 1, a1);
	}
	if (same(argc, argv, 1, fru) && argc == 1)
		return run(intf, "fru", 0, NULL);
	if (same(argc, argv, 1, nvme) && argc == 1) {
		a1[0] = "nvme";
		return run(intf, "ocsoem", 1, a1);
	}
	if (same(argc, argv, 1, state) && argc == 1) {
		a1[0] = "status";
		return run(intf, "power", 1, a1);
	}
	if (same(argc, argv, 1, nextboot) && argc == 1) {
		char *args[] = { "bootparam", "get", "5" };
		return run(intf, "chassis", 3, args);
	}
	if (same(argc, argv, 1, led) && argc == 1) {
		a1[0] = "getledstatus";
		return run(intf, "ocsoem", 1, a1);
	}
	if ((same(argc, argv, 1, log) && argc == 1) ||
		(same(argc, argv, 2, logread) && argc == 2)) {
		int rc;
		a1[0] = "elist";
		ipmi_sel_set_raw_prefix(1);
		rc = run(intf, "sel", 1, a1);
		ipmi_sel_set_raw_prefix(0);
		return rc;
	}
	if (same(argc, argv, 2, biosconfig) && argc == 2) {
		a1[0] = "biosconfig";
		return run(intf, "ocsoem", 1, a1);
	}
	if (same(argc, argv, 2, bioscode)) {
		char *args[2];
		version = option_value(argc - 2, argv + 2, "-v");
		args[0] = "bioscode";
		args[1] = version == NULL ? "current" : version;
		if (strcmp(args[1], "current") != 0 &&
			strcmp(args[1], "previous") != 0)
			return -1;
		return run(intf, "ocsoem", 2, args);
	}
	if (same(argc, argv, 2, tpm) && argc == 2) {
		a1[0] = "gettpmphypresence";
		return run(intf, "ocsoem", 1, a1);
	}
	return -1;
}

static int
set_system(struct ipmi_intf *intf, int argc, char **argv)
{
	char *a1[1];
	char *value;
	const char *nextboot[] = { "nextboot" };
	const char *led[] = { "led" };
	const char *logclear[] = { "log", "clear" };
	const char *biosconfig[] = { "bios", "config" };
	const char *tpm[] = { "tpm", "presence" };
	const char *console[] = { "console" };

	if (argc == 1 && (strcmp(argv[0], "on") == 0 ||
		strcmp(argv[0], "off") == 0 || strcmp(argv[0], "reset") == 0)) {
		a1[0] = argv[0];
		return run(intf, "power", 1, a1);
	}
	if (same(argc, argv, 1, nextboot)) {
		char *args[2];
		value = option_value(argc - 1, argv + 1, "-t");
		if (value == NULL || (value = boot_device(value)) == NULL)
			return -1;
		args[0] = "bootdev";
		args[1] = value;
		return run(intf, "chassis", 2, args);
	}
	if (same(argc, argv, 1, led) && argc == 2) {
		char *args[2];
		args[0] = "identify";
		if (strcmp(argv[1], "on") == 0)
			args[1] = "force";
		else if (strcmp(argv[1], "off") == 0)
			args[1] = "0";
		else
			return -1;
		return run(intf, "chassis", 2, args);
	}
	if (same(argc, argv, 2, logclear) && argc == 2) {
		a1[0] = "clear";
		return run(intf, "sel", 1, a1);
	}
	if (same(argc, argv, 2, biosconfig)) {
		char *args[3];
		args[1] = option_value(argc - 2, argv + 2, "-j");
		args[2] = option_value(argc - 2, argv + 2, "-n");
		if (args[1] == NULL || args[2] == NULL)
			return -1;
		args[0] = "setbiosconfig";
		return run(intf, "ocsoem", 3, args);
	}
	if (same(argc, argv, 2, tpm)) {
		char *args[2];
		value = option_value(argc - 2, argv + 2, "-p");
		if (value == NULL || (strcmp(value, "0") != 0 &&
			strcmp(value, "1") != 0))
			return -1;
		args[0] = "settpmphypresence";
		args[1] = strcmp(value, "1") == 0 ? "true" : "false";
		return run(intf, "ocsoem", 2, args);
	}
	if (same(argc, argv, 1, console) && argc == 2)
		return ipmi_ocs_console_select(intf, argv[1]);
	return -1;
}

int
ipmi_wcscli_main(struct ipmi_intf *intf, int argc, char **argv)
{
	char *filtered[WCSCLI_MAX_ARGS];
	int count;
	int rc;

	if (argc == 0 || strcmp(argv[0], "help") == 0) {
		usage();
		return 0;
	}
	if (filter_instance(argc, argv, &count, filtered) != 0) {
		usage();
		return -1;
	}
	if (count < 3 || strcmp(filtered[1], "system") != 0) {
		usage();
		return -1;
	}
	if (strcmp(filtered[0], "show") == 0)
		rc = show_system(intf, count - 2, filtered + 2);
	else if (strcmp(filtered[0], "set") == 0)
		rc = set_system(intf, count - 2, filtered + 2);
	else
		rc = -1;
	if (rc != 0) {
		lprintf(LOG_ERR, "Unsupported or invalid WCSCLI command");
		usage();
	}
	return rc;
}
