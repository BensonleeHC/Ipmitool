#include <syslog.h>
#include <ipmitool/ipmi_intf.h>
#include <ipmitool/ipmi_fru.h>
#include <ipmitool/log.h>

int ipmi_rf_server(struct ipmi_intf *intf, int argc, char ** argv) {
	int rc = 0;

	// Firmware BIOS version
	printf("BIOS Version :");
	char *bios_args[] = { "firmwareversion", "bios" };
	rc = ipmi_cmd_run(intf, "ocsoem", 2, bios_args);
	if (rc != 0) {
		printf("Failure");
	}

	// Firmware BMC version
	printf("\n");
	printf("BMC Version :");
	char *bmc_args[] = { "firmwareversion", "bmc" };
	rc = ipmi_cmd_run(intf, "ocsoem", 2, bmc_args);
	if (rc != 0) {
		printf("Failure");
	}

	// Firmware CPLD version
	printf("\n");
	printf("CPLD Version :");
	char *cpld_args[] = { "firmwareversion", "cpld" };
	rc = ipmi_cmd_run(intf, "ocsoem", 2, cpld_args);
	if (rc != 0) {
		printf("Failure");
	}
	printf("\n");
	// Mac Address
	printf("MAC1:");
	char *nicargs[] = { "nicinfo", "1" };
	rc = ipmi_cmd_run(intf, "ocsoem", 2, nicargs);
	if (rc != 0) {
		printf("Failure");
	}

	// Fru details
	printf("\n");
	rc = ipmi_cmd_run(intf, "fru", 0, NULL);
	if (rc != 0) {
		printf("Fru error : Failure");
		printf("Completion code: Failure");
	}

	printf("\n");
	printf("GUID:");
	char *guidargs[] = { "guid" };
	rc = ipmi_cmd_run(intf, "ocsoem", 1, guidargs);
	if (rc != 0) {
		printf("Failure");
	}

	printf("\n");
	printf("Power State: ");
	char *chassisstatus[] = { "power", "status" };
	rc = ipmi_cmd_run(intf, "chassis", 2, chassisstatus);
	if (rc != 0) {
		printf("Failure");
	}

	printf("\n");
	char *ledstatus[] = { "getledstatus" };
	rc = ipmi_cmd_run(intf, "ocsoem", 1, ledstatus);
	if (rc != 0) {
		printf("LED Status: Failure");
	}

	char *cpu_info[] = { "redfish", "cpu" };
	rc = ipmi_cmd_run(intf, "ocsoem", 2, cpu_info);
	if (rc != 0) {
		printf("Completion code: Failure");
	}

	// Get total system memory
	printf("\n");
	char *totalmemory[] = { "totalmemory" };
	rc = ipmi_cmd_run(intf, "ocsoem", 1, totalmemory);
	if (rc != 0) {
		printf("TotalSystemMemoryGiB: Failure");
	}

	return rc;
}

int ipmi_rf_fan(struct ipmi_intf *intf, int argc, char ** argv)
{
	int rc = 0;
	printf("\n");
	printf("----- Fan Information -----\n");
	for (int i = 0; i <= 5; i++) { // Fan sensors 0 to 5
			char fanstr[] = "Fan_";
			char fan_id[8];
			sprintf(fan_id, "%s%d",fanstr,i);
			char *faninfo[] = { "get", fan_id };
			rc = ipmi_cmd_run(intf, "sdr", 2, faninfo);
			if (rc != 0) {
				printf("\n");
			}
	}
	return rc;
}

int ipmi_rf_cpu(struct ipmi_intf *intf, int argc, char ** argv) {
	int rc = 0;
	printf("----- CPU Information -----\n");
	char *processor1[] = { "processorinfo", "0x00" };
	char *processor2[] = { "processorinfo", "0x01" };

	rc = ipmi_cmd_run(intf, "ocsoem", 2, processor1);
	if (rc != 0) {
		printf("Processor0 Info: Failure");
		printf("Completion Code: Failure");
	}
	rc = ipmi_cmd_run(intf, "ocsoem", 2, processor2);
	if (rc != 0) {
		printf("Processor1 Info: Failure");
		printf("Completion Code: Failure");
	}
	return rc;
}

int ipmi_rf_server_data(struct ipmi_intf *intf, int argc, char ** argv) {
	int rc = 0;
	printf("----- Server Information -----\n");
	char *slotid[] = { "getslotid", "0xE", "4" };
	char *state[] = { "systemstate" };

	rc = ipmi_cmd_run(intf, "ocsoem", 3, slotid);
	if (rc != 0) {
		printf("Slotid: Failure");
	}
	rc = ipmi_cmd_run(intf, "ocsoem", 1, state);
	if (rc != 0) {
		printf("System state: Failure");
	}

	printf("GUID:");
	char *guidargs[] = { "guid" };
	rc = ipmi_cmd_run(intf, "ocsoem", 1, guidargs);
	if (rc != 0) {
		printf("Failure");
	}
	printf("MAC1:");
	char *nicargs[] = { "nicinfo", "1" };
	rc = ipmi_cmd_run(intf, "ocsoem", 2, nicargs);
	if (rc != 0) {
		printf("Failure");
	}
	return rc;
}

int ipmi_rf_thermal(struct ipmi_intf *intf, int argc, char ** argv)
{
	int rc = 0;

	// Fan Information
	printf("\n");
	char *fan_info[] = {"redfish", "fan"};
	rc = ipmi_cmd_run(intf, "ocsoem", 2, fan_info);
	if (rc != 0) {
		printf("Completion code: Failure");
	}

	printf("\n");
	printf("----- Temperature Information -----\n");
	char *temp_info[] = {"type", "temperature"};
	rc = ipmi_cmd_run(intf, "sdr", 2, temp_info);

	if (rc != 0) {
		printf("Completion Code: Failure");
	}

	return rc;
}

int ipmi_rf_server_health(struct ipmi_intf *intf, int argc, char ** argv) {
	int rc = 0;

	//Get Blade Information
	printf("\n");
	printf("----- Server Information -----\n");
	char *slotid[] = { "getslotid", "0xE", "4" };
	char *state[] = { "systemstate" };

	rc = ipmi_cmd_run(intf, "ocsoem", 3, slotid);
	if (rc != 0) {
		printf("Slotid: Failure\n");
		printf("Completion Code: Failure\n");
	}
	rc = ipmi_cmd_run(intf, "ocsoem", 1, state);
	if (rc != 0) {
		printf("System state: Failure");
		printf("Completion Code: Failure\n");
	}

	// Using this printf to split the data and parse in python scripts
	printf("\n$$$$$$$$$$$$$$$$$$$$$\n");
	// Get Processors Info -----
	printf("\n");
	char *cpu_info[] = { "redfish", "cpu" };
	rc = ipmi_cmd_run(intf, "ocsoem", 2, cpu_info);
	if (rc != 0) {
		printf("Completion code: Failure\n");
	}
	printf("\n$$$$$$$$$$$$$$$$$$$$$\n");
	printf("----- Memory Information -----\n");
	char *memory_info[] = { "dimminfo" };
	rc = ipmi_cmd_run(intf, "ocsoem", 1, memory_info);

	if (rc != 0) {
		printf("Completion code: Failure\n");
	}
	printf("\n$$$$$$$$$$$$$$$$$$$$$\n");
	printf("----- PCIE Information -----\n");
	char *pcie_info[] = { "getpcie" };
	rc = ipmi_cmd_run(intf, "ocsoem", 1, pcie_info);

	if (rc != 0) {
		printf("Completion code: Failure\n");
	}
	printf("\n$$$$$$$$$$$$$$$$$$$$$\n");
	printf("----- Fan Information -----\n");
	// Fan Information
	char *fan_info[] = { "type", "fan" };
	rc = ipmi_cmd_run(intf, "sdr", 2, fan_info);
	if (rc != 0) {
		printf("Completion code: Failure\n");
	}
	printf("\n$$$$$$$$$$$$$$$$$$$$$\n");
	printf("----- Fru Information -----\n");

	char *fru_print[] = { "print" };
	rc = ipmi_cmd_run(intf, "fru", 1, fru_print);
	if (rc != 0) {
		printf("Completion Code: Failure\n");
	}

	printf("\n$$$$$$$$$$$$$$$$$$$$$\n");
	printf("----- Temperature Information -----\n");
	char *temperature[] = { "type", "temperature" };
	rc = ipmi_cmd_run(intf, "sdr", 2, temperature);
	if (rc != 0) {
		printf("Completion Code: Failure\n");
	}

	printf("\n$$$$$$$$$$$$$$$$$$$$$\n");
	printf("----- Sensor Information -----\n");
	char *sensor[] = { "elist" };
	rc = ipmi_cmd_run(intf, "sdr", 1, sensor);
	if (rc != 0) {
		printf("Completion Code: Failure\n");
	}

	return rc;
}

/*
 * Function Name:     ipmi_ocsoem_rf_main_usage
 * Description:       This function prints Open Compute - Open Cloud Server commands help for redfish commands
 * Return:
 */
static void ipmi_ocsoem_rf_main_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "usage: ocsoem redfish <command> [option...]");
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "commands:");
    lprintf(LOG_NOTICE, "    systeminfo");
    lprintf(LOG_NOTICE, "    fan");
    lprintf(LOG_NOTICE, "    cpu");
    lprintf(LOG_NOTICE, "    server");
    lprintf(LOG_NOTICE, "    health");
	lprintf(LOG_NOTICE, "\nFor help on individual commands type:");
	lprintf(LOG_NOTICE, "ocsoem redfish <command> help");
}


int ipmi_ocsoem_rf_server_main(struct ipmi_intf * intf, int argc, char **argv) {
	int rc = 0;
	unsigned int arg_idx = 1;

	if ((argc == 1) || (strncmp(argv[arg_idx], "help\0", 5) == 0)) {
		ipmi_ocsoem_rf_main_usage();
		return 0;
	}
	if ((0 == strncmp(argv[arg_idx], "systeminfo\0", 8))) {
		/* get systems info*/
		rc = ipmi_rf_server(intf, argc, argv);
	} else if ((0 == strncmp(argv[arg_idx], "fan\0", 4))) {
		/* get fan info*/
		rc = ipmi_rf_fan(intf, argc, argv);
	} else if ((0 == strncmp(argv[arg_idx], "cpu\0", 4))) {
		/* get processor info*/
		rc = ipmi_rf_cpu(intf, argc, argv);
	} else if ((0 == strncmp(argv[arg_idx], "server\0", 7))) {
		/* get server info*/
		rc = ipmi_rf_server_data(intf, argc, argv);
	} else if ((0 == strncmp(argv[arg_idx], "health\0", 7))) {
		/* get system health info*/
		rc = ipmi_rf_server_health(intf, argc, argv);
	} else if ((0 == strncmp(argv[arg_idx], "thermal\0", 8))) {
		/* get system Thermal info*/
		rc = ipmi_rf_thermal(intf, argc, argv);
	} else {
		ipmi_ocsoem_rf_main_usage();
		return (-1);
	}
	return rc;
}
