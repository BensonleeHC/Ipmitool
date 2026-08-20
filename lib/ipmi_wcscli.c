/*
 * WCSCLI-compatible command dispatcher for direct BMC access.
 *
 * This module owns only the WCSCLI grammar.  Protocol and response decoding
 * remain in their feature modules so that the compatibility syntax does not
 * duplicate OEM implementations.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ipmitool/ipmi.h>
#include <ipmitool/ipmi_intf.h>
#include <ipmitool/ipmi_ocsoem.h>
#include <ipmitool/ipmi_wcscli.h>
#include <ipmitool/log.h>

#define WCSCLI_MIN_INSTANCE 1
#define WCSCLI_MAX_INSTANCE 48

typedef int (*wcscli_handler)(struct ipmi_intf *intf);

struct wcscli_command {
	const char *verb;
	const char *target;
	const char *feature;
	wcscli_handler handler;
	const char *description;
};

static int
wcscli_show_system_nvme(struct ipmi_intf *intf)
{
	char *args[] = { "nvme" };

	return ipmi_msftoem_main(intf, 1, args);
}

static const struct wcscli_command wcscli_commands[] = {
	{ "show", "system", "nvme", wcscli_show_system_nvme,
	  "Show system NVMe status" },
	{ NULL, NULL, NULL, NULL, NULL }
};

static void
wcscli_usage(void)
{
	const struct wcscli_command *command;

	lprintf(LOG_NOTICE, "usage: wcscli <verb> <target> <feature> [options]");
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Commands:");
	for (command = wcscli_commands; command->handler != NULL; ++command) {
		lprintf(LOG_NOTICE, "  %-6s %-8s %-12s %s", command->verb,
			command->target, command->feature, command->description);
	}
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Compatibility options:");
	lprintf(LOG_NOTICE, "  -i <1-48>   Accept a Rack Manager server instance ID.");
	lprintf(LOG_NOTICE, "              In direct-BMC mode -H remains the target.");
}

static int
wcscli_parse_instance(int argc, char **argv, int *instance)
{
	char *end = NULL;
	long value;

	*instance = 0;
	if (argc == 0) {
		return 0;
	}
	if (argc != 2 || strcmp(argv[0], "-i") != 0) {
		lprintf(LOG_ERR, "Invalid WCSCLI option");
		return -1;
	}

	value = strtol(argv[1], &end, 10);
	if (end == argv[1] || *end != '\0' ||
		value < WCSCLI_MIN_INSTANCE || value > WCSCLI_MAX_INSTANCE) {
		lprintf(LOG_ERR, "Invalid server instance '%s'; expected 1-48",
			argv[1]);
		return -1;
	}

	*instance = (int)value;
	return 0;
}

int
ipmi_wcscli_main(struct ipmi_intf *intf, int argc, char **argv)
{
	const struct wcscli_command *command;
	int instance;

	if (argc == 0 || strcmp(argv[0], "help") == 0) {
		wcscli_usage();
		return 0;
	}
	if (argc < 3) {
		lprintf(LOG_ERR, "Incomplete WCSCLI command");
		wcscli_usage();
		return -1;
	}

	if (wcscli_parse_instance(argc - 3, argv + 3, &instance) != 0) {
		wcscli_usage();
		return -1;
	}

	for (command = wcscli_commands; command->handler != NULL; ++command) {
		if (strcmp(argv[0], command->verb) == 0 &&
			strcmp(argv[1], command->target) == 0 &&
			strcmp(argv[2], command->feature) == 0) {
			if (instance != 0 && verbose > 0) {
				lprintf(LOG_INFO,
					"WCSCLI instance %d accepted for compatibility; "
					"direct-BMC target is selected by -H", instance);
			}
			return command->handler(intf);
		}
	}

	lprintf(LOG_ERR, "Unsupported WCSCLI command: %s %s %s",
		argv[0], argv[1], argv[2]);
	wcscli_usage();
	return -1;
}
