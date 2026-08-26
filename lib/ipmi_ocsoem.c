/*
 * ipmi_ocsoem.c
 *
 *  Created on: Apr 14, 2016
 *      Author: admin_user
 */

#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <ipmitool/log.h>
#include <ipmitool/helper.h>
#include <ipmitool/ipmi.h>
#include <ipmitool/ipmi_intf.h>
#include <ipmitool/ipmi_ocsoem.h>

#define NUM_PRODINFO_LABELS 9
#define NUM_PRODINFO_MFG    16

const char* ProductInfo_Labels[NUM_PRODINFO_LABELS] = 
{
	"Product Manufacturer",
    "Product Name",
	"Product Model Number",
	"Product Version",
	"Product Serial Number",
	"Reserved",
	"Product FRU File ID",
	"Product Custom Field 1",
	"Product Custom Field 2"
};

const char* ProductInfo_MfgCode[NUM_PRODINFO_MFG] = 
{
	"Unassigned",
    "Unassigned",
	"Wiwynn",
	"Dell",
	"HP",
	"ZT Systems",
	"Unassigned",
	"Unassigned",
	"Foxconn Longhua",
	"Unassigned",
	"Quanta",
	"Foxconn Tianjin",
	"Ingraysis Tianjin",
	"Flextronics China",
	"Lenovo",
	"Unassigned"
};

const struct valstr ocs_completion_code[] = {
	{ 0x00, "Command Success." },
	{ 0x81, "Lost arbitration." },
	{ 0x82, "Bus error." },
	{ 0x83, "NAK on write." },
	{ 0x84, "Truncated read." },
	{ 0x90, "Rmedia Support is not enabled." },
	{ 0xc0, "Node busy." },
	{ 0xc1, "Invalid command." },
	{ 0xcc, "Invalid data field in request." },
	{ 0xc9, "Parameter out of range." },
	{ 0xd0, "Response not provided." },
	{ 0xd1, "BMC initialization." },
	{ 0xd2, "Request destination unavailable." },
	{ 0xd3, "Fru version not supported." },
	{ 0xd4, "Response length invalid." },
	{ 0xd5, "Command not supported."},
	{ 0xd6, "Send/Receive failed."},
	{ 0xff, "Unspecified error."},
	{ 0x00, NULL }
};

char energy_storage_presence [4] [12] = {
	"Unknown",
	"Charging",
	"Discharging",
	"Floating"
};

char default_power_limit_action [3] [17] = {
	"NoAction",
	"ThrottleAndLimit",
	"PowerLimitOnly"
};

char processor_type [8] [15] = {
		"Sandy Bridge",
		"Ivy Bridge" ,
		"Centerton" ,
		"Haswell" ,
		"Broadwell",
		"Skylake" ,
		"No CPU Present",
		"Unknown"
};

char adr_trigger_type [4] [10] = {
	"Disabled",  //0x00
	"uvadrtrigger",//"PchAdrGpi", //0x01
	"PchSmiGpi", //0x02
	"fmuv_pchsmi", //0x03
	"cpldgpo", //0x04
	"Unknown"    //0xFF Unknown trigger response type only
};

/*
 * Function Name:     ipmi_ocs_usage
 *
 * Description:       This function prints Open Compute - Open Cloud Server commands
 * Input:
 * Output:
 *
 * Return:
 *
 */
static void ipmi_ocs_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "usage: ocsoem <command> [option...]");
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "commands:");
    lprintf(LOG_NOTICE, "    activatepsualert");
    lprintf(LOG_NOTICE, "    bioscode");
    lprintf(LOG_NOTICE, "    biosconfig");
    lprintf(LOG_NOTICE, "    dpc");
    lprintf(LOG_NOTICE, "    firmwareversion");
    lprintf(LOG_NOTICE, "    getmemoryinfo");
    lprintf(LOG_NOTICE, "    gettotalmemory");
    lprintf(LOG_NOTICE, "    dimminfo");
    lprintf(LOG_NOTICE, "    getledstatus");
    lprintf(LOG_NOTICE, "    systemstate");
    lprintf(LOG_NOTICE, "    gpio");
    lprintf(LOG_NOTICE, "    bmcfwport");
    lprintf(LOG_NOTICE, "    guid");
    lprintf(LOG_NOTICE, "    getslotid");
    lprintf(LOG_NOTICE, "    jtagprogramstatus");
    lprintf(LOG_NOTICE, "    localenergy");
    lprintf(LOG_NOTICE, "    nicinfo");
    lprintf(LOG_NOTICE, "    nvdimmtrigger");
    lprintf(LOG_NOTICE, "    pcieinfo");
    lprintf(LOG_NOTICE, "    getpcie");
    lprintf(LOG_NOTICE, "    processorinfo");
    lprintf(LOG_NOTICE, "    psualert");
    lprintf(LOG_NOTICE, "    psufwupdate");
    lprintf(LOG_NOTICE, "    psuread");
    lprintf(LOG_NOTICE, "    psuaction");
    lprintf(LOG_NOTICE, "    setbiosconfig");
    lprintf(LOG_NOTICE, "    setdpc");
    lprintf(LOG_NOTICE, "    setenergy");
    lprintf(LOG_NOTICE, "    setgpio");
    lprintf(LOG_NOTICE, "    setjtagprogramstatus");
    lprintf(LOG_NOTICE, "    setnvdimmtrigger");
    lprintf(LOG_NOTICE, "    setpsualert");
    lprintf(LOG_NOTICE, "    gettpmphypresence");
    lprintf(LOG_NOTICE, "    settpmphypresence");
    lprintf(LOG_NOTICE, "    mastermuxrw");
	lprintf(LOG_NOTICE, "    nvme");
	lprintf(LOG_NOTICE, "    mount");
	lprintf(LOG_NOTICE, "    unmount");
	lprintf(LOG_NOTICE, "    getsessions");
	lprintf(LOG_NOTICE, "    closesession");
    lprintf(LOG_NOTICE, "    redfish");
    lprintf(LOG_NOTICE, "    fpgaaction");
    lprintf(LOG_NOTICE, "    fpgaread");
	//lprintf(LOG_NOTICE, "For help on individual commands type:");
	//lprintf(LOG_NOTICE, "ocsoem <command> help");
}
/*
 * Function Name:   getbios_code_usage
 *
 * Description:   This function prints help for the get bios help message
 * Input:
 * Output:
 *
 * Return:
 */
static void get_bios_code_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Get Bios Code Help:");
	lprintf(LOG_NOTICE, "      bioscode {version}	| values: current || Previous");
	lprintf(LOG_NOTICE, "");
}

/*
 * Function Name:   get_nic_info_usage
 *
 * Description:   This function prints help for the get bios help message
 * Input: nic (0-based index)
 * Output: device type and status
 *
 * Return: MacAddress
 */
static void get_nic_info_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Get Nic Info Help:");
	lprintf(LOG_NOTICE, "   			nicinfo {nic}	| nic values: {0-3}");
	lprintf(LOG_NOTICE, "");
}

/*
 * Function Name:   get_firmware_version_usage
 *
 * Description:   This function prints help for the get firmware version help message
 * Input: component name
 * bios - 0x00
 * cpld - 0x01
 * bmc  - 0x02
 * Output: firmware version for the Specific component
 *
 */
static void get_firmware_version_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Get Firmware Version Help:");
	lprintf(LOG_NOTICE, "   	firmwareversion {component} | component values: {bios|cpld|bmc} ");
	lprintf(LOG_NOTICE, "");
}

/*
 * Function Name:   get_gpio
 *
 * Description:   This function prints help for the get gpio command
 * Input: pin number
 * Output: Pin direction & Pin state
 *
 */
static void get_gpio_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Get Gpio Help:");
	lprintf(LOG_NOTICE, "   	gpio  | values: {pinNumber} ");
	lprintf(LOG_NOTICE, "");
}

static void get_group_gpio_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Get group_gpio Help:");
	lprintf(LOG_NOTICE, "   	bmcfwport  | values: {pinNumber} {length}");
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE,"pinnumber  : 184");
	lprintf(LOG_NOTICE,"  length : 6");
	lprintf(LOG_NOTICE, "");
}

/*
 * Function Name:   get_processor_info_usage
 *
 * Description:   This function prints help for the get processor help message
 * Input: processor (0-based index)
 * Output: processor type; processor frequency and processor state
 *
 */
static void get_processor_info_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Get processor Info Help:");
	lprintf(LOG_NOTICE, "   			processorinfo | values: {processor}");
	lprintf(LOG_NOTICE, "");
}

/*
 * Function Name:   get_pcie_info_usage
 *
 * Description:   This function prints help for the get pcie help message
 * Input: pcieindex (1-based index)
 * Output: Returns : VendorId, DeviceId, Subsystem Id and Subsystem vendor Id
 *
 * Or
 *
 * Input: pcieIndex (0)
 * Output: Returns: pciePresenceLsb, pciePresenceMsb
 *
 */
static void get_pcie_info_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Get get_pcie_info Info Help:");
	lprintf(LOG_NOTICE, "   			pcieinfo {pcieIndex} | pcieindex values: {1-5}");
	lprintf(LOG_NOTICE, "Get get_pcie_info response map Help:");
	lprintf(LOG_NOTICE, "   			pcieinfo  | pcieindex value: {0}");
	lprintf(LOG_NOTICE, "");
}

/* Function Name:   set_bios_config_usage
 *
 * Description:   This function prints help for the set bios config usage help message
 * Input: Configuration, majorflavor, minorflavor
 * * Configuration: Current BIOS Configuration*
 * Output: Completion Code zero
 *
 */
static void set_bios_config_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Set Bios config Usage:");
	lprintf(LOG_NOTICE,	"    setbiosconfig  | values: {majorflavor} {minorflavor}");
	lprintf(LOG_NOTICE, "");
}
/* Function Name:   set_default_power_limit_usage
 *
 * Description:   This function prints help for the set default power limit usage help message
 * Input: dpc (default power limit in watts), delay, autoremovedpcdelayinmillisecs
 * Output: Completion Code zero
 *
 */
static void set_default_power_limit_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Set default power limit Usage:");
	lprintf(LOG_NOTICE,  "   	 setdpc | values: {dpc} {delay} {autoremoveDelayInMilliSec}");
	lprintf(LOG_NOTICE, "");
}
/* Function Name:   set_jtag_program_status_usage
 *
 * Description:   This function prints help for the set jtag program status usage help message
 * Input: jtagdevice
 * Output: Completion Code zero
 *
 */
static void set_jtag_program_status_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Set jtag program status Usage:");
	lprintf(LOG_NOTICE, "   		setjtagprogramstatus | values: {jtagDevice}");
	lprintf(LOG_NOTICE, "");
}

/* Function Name:   set_energy_storage_usage
 *
 * Description:   This function prints help for the set energy storage usage help message
 * Input: batterypresence, state, scalingfactor,bladeenergy,nvdimmenergy
 * Output: Completion Code zero
 *
 */
static void set_energy_storage_usage() {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Set energy storage Usage:");
	lprintf(LOG_NOTICE, "   		setenergy | values:{batterypresence} {state} {scalingfactor} {bladeenergy} {nvdimmenergy}");
	lprintf(LOG_NOTICE, "");
}

/* Function Name:   set_gpio_usage
 *
 * Description:   This function prints help for the set gpio usage help message
 * Input: pinnumber(GPIO pin number), pindirection (IN/OUT Direction of the pin), pinstate (LO/HI direction of the pin)
 * Output: Completion Code zero
 *
 */
static void set_gpio_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Set gpio Usage:");
	lprintf(LOG_NOTICE, "   	setgpio | values: {pinnumber} {pindirection} {pinstate}");
	lprintf(LOG_NOTICE, "");
}

static void auth_cap_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "get slot_id Usage:");
	lprintf(LOG_NOTICE, "   	getslotid | values: {channelnumber} {privilege}");
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE,"ChannelNumbers  are:");
	lprintf(LOG_NOTICE,"   1 || 2 || 0xE");
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE,"Possible privilege levels are:");
	lprintf(LOG_NOTICE,"   1   Callback level");
	lprintf(LOG_NOTICE,"   2   User level");
	lprintf(LOG_NOTICE,"   3   Operator level");
	lprintf(LOG_NOTICE,"   4   Administrator level");
	lprintf(LOG_NOTICE,"   5   OEM Proprietary level");
	lprintf(LOG_NOTICE,"  15   No access");
}

/* Function Name:   set_psu_alert_usage
 *
 * Description:   This function prints help for the set psu alert usage help message
 * Input: enable (True || False)
 * Output: Completion Code zero
 *
 */
static void set_psu_alert_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Set psu alert Usage:");
	lprintf(LOG_NOTICE, "   	setpsualert {enablestate} | enable state values: true || false");
	lprintf(LOG_NOTICE, "");
}

/* Function Name:   activate_psu_alert_usage
 *
 * Description:   This function prints help for the activate psu alert usage help message
 * Input: bmcpsualertaction(0||4||8), bmcremocecapaction(0||1||2)
 * Output: Completion Code zero
 *
 */
static void activate_psu_alert_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Activate psu alert Usage:");
	lprintf(LOG_NOTICE, "   	activatepsualert {bmcaction} {removecapaction} | "
					"values: {0 || 4 || 8} {0 || 1 || 2}");
	lprintf(LOG_NOTICE, "");
}

/* Function Name:   set_nvdimm_trigger_usage
 *
 * Description:   This function prints help for the set nvdimm trigger usage help message
 * Input:
 * AdrTriggerType ((disabled)0 || (pchadrgpi)1 || (pchsmigpi)2 || (unknown) 0xFF)
 * AdrDelay {True || False}
 * NvdimmDelay {True || False}
 * Output: Completion Code zero
 *
 */
static void set_nvdimm_trigger_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Set nv dimm trigger Usage:");
	lprintf(LOG_NOTICE, "  setnvdimmtrigger {AdrTriggerTpe} {AdrDelay} {NvDimmDelay} | "
			"values: {0 || 1 || 2 || 0xFF} {seonds} {seconds}");
	lprintf(LOG_NOTICE, "");
}

static void get_memory_info_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Get memory info Help:");
	lprintf(LOG_NOTICE, "  getmemoryinfo | values: {dimmIndex_Non zero}");
	lprintf(LOG_NOTICE, "Get memory info prsence Map Help:");
		lprintf(LOG_NOTICE, "   	getmemoryinfo  | value: 0");
	lprintf(LOG_NOTICE, "");
}

static void psu_fw_update_usage (void) {
	lprintf (LOG_NOTICE, "");
	lprintf (LOG_NOTICE, "PSU Firmware Update Usage:");
	lprintf (LOG_NOTICE, "    psufwupdate {compindex} {imgtype} {op} {image} | "
		"values: {1 (PSU0) | 2 (PSU1)}, {0 (bootloader) | 1 (image A) | 2 (image B)}, "
		"{1 (start) | 2 (abort) | 3 (query)}, {file name}");
	lprintf (LOG_NOTICE, "");
}

static void ocs_psu_read_usage (void) {
	lprintf (LOG_NOTICE, "");
	lprintf (LOG_NOTICE, "PSU Status Read Usage:");
	lprintf (LOG_NOTICE, "    psuread {command} {phase|image} | "
		"values: {all | caps | status | vin | iin | pin | iout | pout | mfr | device | model | serial | feed | battery | fw}, "
		"{0 (bootloader, phase 1) | 1 (phase 2) | 2 (phase 3) | 0xA (fw A) | 0xB (fw B) | 0xFF (all phases, active fw)}");
	lprintf (LOG_NOTICE, "");
}

static void ocs_psu_action_usage (void) {
	lprintf (LOG_NOTICE, "");
	lprintf (LOG_NOTICE, "PSU Status Action Usage:");
	lprintf (LOG_NOTICE, "    psuaction {command} {phase} | "
		"values: {clear | battery}, "
		"{0 (phase 1) | 1 (phase 2) | 2 (phase 3) | 0xFF (all phases)}");
	lprintf (LOG_NOTICE, "");
}

static void ocs_master_mux_rw_usage()
{
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Master Mux-Write-Read Help Usage:");
	lprintf(LOG_NOTICE,	"   	busid | mux1address | mux1channel | mux2address | mux2channel | deviceslaveaddress | readcount | writedata ");
	lprintf(LOG_NOTICE, "");
}

static void ocs_fpga_read_usage()
{
	lprintf (LOG_NOTICE, "");
	lprintf (LOG_NOTICE, "FPGA Read Usage:");
	lprintf (LOG_NOTICE, "    fpgaread {command} | "
		"values: {mode | temp | health | i2cversion | i2ctest | assetinfo}");
	lprintf (LOG_NOTICE, "");
}

static void ocs_fpga_action_usage()
{
	lprintf (LOG_NOTICE, "");
	lprintf (LOG_NOTICE, "FPGA Action Usage:");
	lprintf (LOG_NOTICE, "    fpgaaction {command} | "
		"values: {setbypass | clearbypass}");
	lprintf (LOG_NOTICE, "");
}

static void mount_remote_media_usage(void) {
	lprintf(LOG_NOTICE, "\n");
	lprintf(LOG_NOTICE, "Remote Media Usage:\n");
	lprintf(LOG_NOTICE, "   	mount | values: {ip address} {image name} {image nfs path}\n");
	lprintf(LOG_NOTICE, "\n");
}

static void unmountmount_remote_media_usage(void) {
	lprintf(LOG_NOTICE, "\n");
	lprintf(LOG_NOTICE, "Remote Media Usage:\n");
	lprintf(LOG_NOTICE, "   	unmount \n");
	lprintf(LOG_NOTICE, "\n");
}

static void active_sessions_usage(void) {
	lprintf(LOG_NOTICE, "\n");
	lprintf(LOG_NOTICE, "Get All Active Session Usage:\n");
	lprintf(LOG_NOTICE, "   	getsessions | values: {Session type}\n");
	lprintf(LOG_NOTICE, "	Session Type:\n	00h - WEBUI\n	01h - KVM\n	02h - CD\n	03h - FD\n	04h - HD\n	05h - SSH\n	06h - TELNET\n");
	lprintf(LOG_NOTICE, "\n");
}

static void close_active_sessions_usage(void) {
	lprintf(LOG_NOTICE, "\n");
	lprintf(LOG_NOTICE, "Close active session:\n");
	lprintf(LOG_NOTICE, "   	closesession | values: {4 bytes Session Id}\n");
	lprintf(LOG_NOTICE, "	session id example 3 2 6 5\n");
	lprintf(LOG_NOTICE, "\n");
}

static void set_service_config_usage(void) {
	lprintf(LOG_NOTICE, "");
	lprintf(LOG_NOTICE, "Set service configuration:");
	lprintf(LOG_NOTICE, "    service {type} {config} | values: {cpudbg | fpgadbg}, {start | stop}");
	lprintf(LOG_NOTICE, "");
}


int ipmi_ocs_get_bios_code(struct ipmi_intf * intf, char version);
int ipmi_ocs_get_firmware_version(struct ipmi_intf *intf, char component);

int ipmi_msftoem_main(struct ipmi_intf * intf, int argc, char **argv) {
	int rc = 0;
	unsigned int arg_idx = 0;

	if (argc == 0 || strncmp(argv[0], "help\0", 5) == 0) {
		ipmi_ocs_usage();
		return 0;
	}
	if (0 == strncmp(argv[arg_idx], "bioscode\0", 9)) {
		/* get bios code */
		rc = ipmi_ocs_get_bios_code_main(intf, argv);
	} else if (strncmp(argv[arg_idx], "biosconfig\0", 11) == 0) {
		/* get bios config */
		rc = ipmi_ocs_get_bios_config(intf);
	} else if (strncmp(argv[arg_idx], "setbiosconfig\0", 14) == 0) {
		/* set bios config */
		rc = ipmi_ocs_set_bios_config(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "dpc\0", 4) == 0) {
		/* default power limit */
		rc = ipmi_ocs_get_default_power_limit(intf);
	} else if (strncmp(argv[arg_idx], "setdpc\0", 7) == 0) {
		/* default power limit */
		rc = ipmi_ocs_set_default_power_limit(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "localenergy\0", 12) == 0) {
		/* get energy storage*/
		rc = ipmi_ocs_get_energy_storage(intf);
	} else if (strncmp(argv[arg_idx], "setenergy\0", 10) == 0) {
		/* set energy storage*/
		rc = ipmi_ocs_set_energy_storage(intf,argc,argv);
	} else if (strncmp(argv[arg_idx], "nicinfo\0", 8) == 0) {
		/* get nicinfo*/
		rc = ipmi_ocs_get_nic_info(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "firmwareversion\0", 16) == 0) {
		/* get firmwareversion*/
		rc = ipmi_ocs_get_firmware_version_main(intf, argv);
	} else if (strncmp(argv[arg_idx], "gpio\0", 5) == 0) {
		/* get gpio*/
		rc = ipmi_ocs_get_gpio(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "setgpio\0", 8) == 0) {
		/* set gpio*/
		rc = ipmi_ocs_set_gpio(intf,argc, argv);
	} else if (strncmp(argv[arg_idx], "jtagprogramstatus\0", 18) == 0) {
		/* get jtagprogram status*/
		rc = ipmi_ocs_get_jtag_program_status(intf);
	} else if (strncmp(argv[arg_idx], "setjtagprogramstatus\0", 21) == 0) {
		/* set jtagprogram status*/
		rc = ipmi_ocs_set_jtag_program_status(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "nvdimmtrigger\0", 14) == 0) {
		/* get nv dimm trigger*/
		rc = ipmi_ocs_nv_dimm_trigger(intf);
	}  else if (strncmp(argv[arg_idx], "setnvdimmtrigger\0", 17) == 0) {
		/* set nv dimm trigger*/
		rc = ipmi_ocs_set_nvdimm_trigger(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "processorinfo\0", 14) == 0) {
		/* get processor info*/
		rc = ipmi_ocs_get_processor_info(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "psualert\0", 9) == 0) {
		/* get psu alert*/
		rc = ipmi_ocs_get_psu_alert(intf);
	} else if (strncmp(argv[arg_idx], "setpsualert\0", 12) == 0) {
		/* set psu alert*/
		rc = ipmi_ocs_set_psu_alert(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "activatepsualert\0", 17) == 0) {
		/* activate psu alert*/
		rc = ipmi_ocs_activate_psu_alert(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "pcieinfo\0", 9) == 0) {
		/* get pcie info*/
		rc = ipmi_ocs_get_pcie_info_response(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "getpcie\0", 8) == 0) {
		/* get all pcie info*/
		rc = ipmi_ocs_pcie_info(intf);
	}else if (strncmp(argv[arg_idx], "getmemoryinfo\0", 14) == 0) {
				rc = ipmi_ocs_get_memory_info(intf, argc, argv);
	} else if ((strncmp(argv[arg_idx], "dimminfo\0", 9) == 0) || (strncmp(argv[arg_idx], "totalmemory\0", 11) == 0)) {
		rc = ipmi_ocs_memory_info(intf,argv[arg_idx]);
	}else if (strncmp(argv[arg_idx], "gettotalmemory\0", 15) == 0) {
		rc = ipmi_ocs_get_total_memory(intf);
	} else if (strncmp(argv[arg_idx], "guid\0", 5) == 0) {
	    rc = ipmi_ocs_get_guid(intf);
	} else if (strncmp(argv[arg_idx], "gettpmphypresence\0", 18) == 0) {
	    rc = ipmi_ocs_get_tpm_physical_presence(intf);
	} else if (strncmp(argv[arg_idx], "settpmphypresence\0", 18) == 0) {
	    rc = ipmi_ocs_set_tpm_physical_prsence(intf,argc,argv);
	} else if (strncmp(argv[arg_idx], "getledstatus\0", 13) == 0) {
	    rc = ipmi_ocs_get_led_status(intf);
	} else if (strncmp(argv[arg_idx], "redfish\0", 8) == 0) {
		rc=ipmi_ocsoem_rf_server_main(intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "psufwupdate", 12) == 0) {
		rc=ipmi_ocs_psu_fw_update (intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "psuread", 7) == 0) {
		rc=ipmi_ocs_psu_read (intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "psuaction", 9) == 0) {
		rc=ipmi_ocs_psu_action (intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "fpgaread", 8) == 0) {
		rc = ipmi_ocs_fpga_read (intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "fpgaaction", 10) == 0) {
		rc = ipmi_ocs_fpga_action (intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "mastermuxrw", 11) == 0) {
		rc = ipmi_ocs_master_mux_rw (intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "getslotid", 10) == 0) {
		rc = ipmi_ocs_channel_auth_cap (intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "bmcfwport", 10) == 0) {
		rc = ipmi_ocs_group_gpio_read (intf, argc, argv);
	} else if (strncmp(argv[arg_idx], "systemstate", 12) == 0) {
		rc = ipmi_system_status(intf);
	} else if (strncmp(argv[arg_idx], "nvme\0", 5) == 0) {
		rc = get_nvme_status(intf);
	} else if (strncmp(argv[arg_idx], "mount\0", 6) == 0) {
		rc = mount_rm_media(intf, argc, argv);
	}else if (strncmp(argv[arg_idx], "unmount\0", 8) == 0) {
		rc = unmount_rm_media(intf, argc, argv);
	} else if(strncmp(argv[arg_idx], "getsessions\0",12) == 0) {
		rc = ipmi_ocs_get_all_active_session(intf,argc, argv);
	} else if(strncmp(argv[arg_idx], "closesession\0",13) == 0) {
		rc = close_active_session(intf,argc, argv);
	} else if(strncmp(argv[arg_idx], "service", 7) == 0) {
		rc = set_service_configuration (intf, argc, argv);
	} else {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
						val2str(CC_INVALID_COMMAND, ocs_completion_code), CC_INVALID_COMMAND);
		printf("List of available commands\n");
		ipmi_ocs_usage();
		return -1;
	}
	return rc;
}

int ipmi_ocs_get_bios_code_main(struct ipmi_intf *intf, char **argv) {
	int rc = 0;
	unsigned int arg_idx = 1;

	if (argv[arg_idx] == NULL) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		get_bios_code_usage();
		return -1;
	}

	if (strncmp(argv[arg_idx], "current\0", 8) == 0) {
		rc = ipmi_ocs_get_bios_code(intf, 0x00);
	} else if (strncmp(argv[arg_idx], "previous\0", 9) == 0) {
		rc = ipmi_ocs_get_bios_code(intf, 0x01);
	} else {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
					val2str(CC_INVALID_DATA_FIELD_IN_REQUEST, ocs_completion_code),	CC_INVALID_DATA_FIELD_IN_REQUEST);
		get_bios_code_usage();
		return -1;
	}
	return rc;
}

int ipmi_ocs_get_bios_code(struct ipmi_intf *intf, char version) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	memset(&req, 0, sizeof(req));

	req.msg.netfn = OEM_GP3_NETFN;
	req.msg.cmd = CMD_GET_BIOS_CODE;
	req.msg.lun = 0;
	req.msg.data = &version;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: (%s) Completion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: (%s) Completion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {

		for (int i = 0; i < rsp->data_len; i++) {
			printf(" %2x", rsp->data[i]);
		}
		printf("\n");
	}
	return 0;
}

int ipmi_ocs_get_bios_config(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	uint8_t currentconfig;
	uint8_t chosenconfig;
	int i;

	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_GET_BIOS_CONFIG;
	req.msg.lun = 0;
	req.msg.data_len = 0;

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s\nCompletion Code: %X \n",
				 val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %2X\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (rsp->data_len >= 2) {
			currentconfig = rsp->data[0];
			chosenconfig = rsp->data[1];
			//bit [6:4] minor BIOS flavor
			uint8_t minorconfig = (currentconfig >> 4) & 0x07;
			//bit [3:0] major BIOS flavor
			uint8_t majorconfig = currentconfig & 0x0F;
			printf("Current BIOS Configuration: %u.%u\n",majorconfig,minorconfig);

			//bit [6:4] minor BIOS flavor
			uint8_t chosenminorconfig = (chosenconfig >> 4 ) & 0x07;
			//bit [3:0] major BIOS flavor
			uint8_t chosenmajorconfig = chosenconfig & 0x0F;
			printf("Chosen BIOS Configuration: %u.%u \n",chosenmajorconfig,chosenminorconfig);

			int buflen = rsp->data_len - 2;

			printf("\n\n------Available Configurations\n");
			if (buflen == 0)
			{
				//printf("zero\n");
				return 0;
			}

			for (i = 0 ; i + 3 < buflen ; i+=4)
			{
					char cfgname[4] = { 0, 0, 0, 0 };
					memcpy(cfgname, rsp->data + 2 + i, 3);
					if (cfgname[0] == 0 && cfgname[1] == 0 && cfgname[2] == 0)
						break;
					printf("ConfigName: %s\n", cfgname);

					uint8_t cfgvalue = rsp->data[2 + i + 3]; // get the 4th byte
					//bit [6:4] minor BIOS flavor
					uint8_t avcfgminorconfig = (cfgvalue >> 4) & 0x07;
					//bit [3:0] major BIOS flavor
					uint8_t avcfgmajorconfig = cfgvalue & 0x0F;
					printf("ConfigValue: %u.%u\n",avcfgmajorconfig,avcfgminorconfig);
					printf("*******************************\n");  //using this string to split data in the middle tier script(python)
			}
			return 0;
		} else {
			lprintf(LOG_ERR, "Error: %s \nCompletion Code: %2X\n",
					val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
					return (-1);
		}
	}
	return 0;
}

int ipmi_ocs_get_default_power_limit(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_default_powerlimit powerlimit;

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_DEFAULT_POWER_LIMIT;
	req.msg.lun = 0;
	req.msg.data_len = 0;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (rsp->data_len >= sizeof(struct _ipmi_default_powerlimit)) {
			memcpy(&powerlimit, rsp->data,
					sizeof(struct _ipmi_default_powerlimit));
			printf("Default Power Limit: %u Watts\n ", powerlimit.dpc);
			printf("Action: %s \n",
					(powerlimit.action <= 3 ?
							default_power_limit_action[powerlimit.action] :
							default_power_limit_action[0]));
			printf("Auto Remove: %u sec\n ", powerlimit.autoremove);
			printf("Delay Time: %u ms\n", powerlimit.delay);
			return 0;
		} else {
			printf("Error: (%s) %d\n",
					val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code),
					CC_INVALID_RESPONSE_LEN);
			return (-1);
		}

	}
	return 0;
}

int ipmi_ocs_get_energy_storage(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_energy_storage energystorage;

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_ENERGY_STORAGE;
	req.msg.lun = 0;
	req.msg.data_len = 0;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (rsp->data_len >= sizeof(struct _ipmi_energy_storage)) {
			memcpy(&energystorage, rsp->data,
					sizeof(struct _ipmi_energy_storage));
			printf("Energy Storage:\n");
			printf("	Battery Presence: %s \n", (energystorage.presence == 1 ? "True" : "False"));
			printf("	Battery State: %s \n", (energystorage.state <= 4 ?
							energy_storage_presence[energystorage.state] : energy_storage_presence[0]));
			printf("	System Energy: %u Joule\n", (energystorage.systemenergy * energystorage.scalefactor));
			printf("	NVDIMM Energy: %u Joule\n", (energystorage.nvdimmenergy * energystorage.scalefactor));
			printf("	Rolling Counter: %u \n", (energystorage.rollingcounter));
			return 0;
		} else {
			lprintf(LOG_ERR,"Error: %s \nCompletion Code: %d\n",
					val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
			return (-1);
		}

	}
	return 0;
}

int ipmi_ocs_get_nic_info(struct ipmi_intf *intf, int argc, char **argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	char macaddress[6];
	unsigned int arg_idx = 1;

	if (argc != 2) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		get_nic_info_usage();
		return -1;
	}
	uint8_t nicid;

   if(str2uchar(argv[arg_idx], &nicid) != 0)
	{
	   lprintf(LOG_ERR, "Error: Failed to convert nic index to byte : %s \n",argv[arg_idx]);
		return -1;
	}

	memset(&req, 0, sizeof(req));

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_NIC_INFO;
	req.msg.lun = 0;
	req.msg.data = &nicid;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (rsp->data_len >= sizeof(macaddress)) {
			memcpy(&macaddress, rsp->data, sizeof(macaddress));
			for(int i = 0 ; i < rsp->data_len; i++)
			{
				if (((i%16) == 0) && (i != 0))
					printf("\n");
					printf("%2.2X:",rsp->data[i]);
			}
			printf("\n");
			return 0;
		} else {
			lprintf(LOG_ERR, "Error: %s \nCompletion Code %d\n",
					val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
			return (-1);
		}
	}
	return 0;
}

int ipmi_ocs_get_firmware_version_main(struct ipmi_intf *intf, char **argv) {
	int rc = 0;
	unsigned int arg_idx = 1;

	if (argv[arg_idx] == NULL) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		get_firmware_version_usage();
		return -1;
	}

	if (strncmp(argv[arg_idx], "bios\0", 5) == 0) {
		rc = ipmi_ocs_get_firmware_version(intf, 0x00);
	} else if (strncmp(argv[arg_idx], "cpld\0", 5) == 0) {
		rc = ipmi_ocs_get_firmware_version(intf, 0x01);
	} else if (strncmp(argv[arg_idx], "bmc\0", 4) == 0) {
		rc = ipmi_ocs_get_firmware_version(intf, 0x02);
	} else {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
					val2str(CC_INVALID_DATA_FIELD_IN_REQUEST, ocs_completion_code),	CC_INVALID_DATA_FIELD_IN_REQUEST);
		get_firmware_version_usage();
		return -1;
	}
	return rc;
}

int ipmi_ocs_get_firmware_version(struct ipmi_intf *intf, char component) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	memset(&req, 0, sizeof(req));

	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_GET_FIRMWARE_VERSION;
	req.msg.lun = 0;
	req.msg.data = &component;

	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {

		//successful response, then print the value in ASCII format
		printf(rsp->data);
		printf("\n");
	}
	return 0;
}

int ipmi_ocs_get_guid(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	memset(&req, 0, sizeof(req));

	req.msg.netfn = IPMI_NETFN_APP;
	req.msg.cmd = CMD_GET_GUID;
	req.msg.lun = 0;
	req.msg.data_len = 0;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		//successful response, then print the value in UUID format
		for(int i = 0; i < rsp->data_len ; i++)
		{
			if((i == 4 || i ==10) || (i>4 && i<10 && i%2 == 0))
				printf("-");
			printf("%02X", rsp->data[i]);

		}
		printf("\n");
	}
	return 0;
}

int ipmi_ocs_get_gpio(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_gpio gpio;

	unsigned int arg_idx = 1;

	if (argc != 2) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		get_gpio_usage();
		return -1;
	}
	uint8_t pinnum;
	if (str2uchar(argv[arg_idx], &pinnum) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert pin number to byte : %s \n", argv[arg_idx]);
		return -1;
	}

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_GPIO;
	req.msg.lun = 0;
	req.msg.data = &pinnum;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else { //successful response, then print pin direction & pin state
		if (rsp->data_len >= sizeof(struct _ipmi_gpio)) {
			memcpy(&gpio, rsp->data, sizeof(struct _ipmi_gpio));

			printf("Pin Direction: %u \n", gpio.pindirection);
			printf("Pin State: %u \n", gpio.pinstate);
			return 0;
		} else {
			lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
					val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
			return (-1);
		}
	}
	return 0;
}

static int get_server_type(uint8_t server_type, char* server_type_str)
{
	if(server_type_str == NULL)
	{
		return -1;
	}
	
	server_type = (((uint8_t) (server_type & CHAUTH_SERVER_TYPE_MASK)) >> CHAUTH_SERVER_TYPE_SHIFT);
	
	switch(server_type)
	{
		case 0x00:
		{
			strncpy(server_type_str, "C2010\0", 6);
			break;
		}
		case 0x01:
		{
			strncpy(server_type_str, "J2010\0", 6);
			break;
		}
		case 0x02:
		{
			strncpy(server_type_str, "F2010\0", 6);
			break;		
		}
		case 0x03:
		{
			strncpy(server_type_str, "T2010\0", 6);
			break;
		}
		case 0x04:
		{
			strncpy(server_type_str, "C2020\0", 6);
			break;
		}
		case 0x05:
		{
			strncpy(server_type_str, "OEM\0", 4);
			break;		
		}
		default:
		{
			return -1;
		}
	}
	
	return 0;
}

static int rack_slot(int slot_id);
int ipmi_ocs_channel_auth_cap(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_get_channel_auth_cap authcap;
	char server_type_str[6];

	uint8_t msg_data[2];

	if (argc != 3) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		auth_cap_usage();
		return -1;
	}
	uint8_t channel;
	uint8_t privilege;

	if (str2uchar(argv[1], &channel) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the channel to char type: %s \n",argv[1]);
		return -1;
	}
	if (str2uchar(argv[2], &privilege) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the privilege to char type: %s \n",
				argv[2]);
		return -1;
	}

	msg_data[0] = channel;
	msg_data[1] = privilege;

	req.msg.netfn = IPMI_NETFN_APP;
	req.msg.cmd = CMD_GET_CHANNEL_AUTH_CAP;
	req.msg.lun = 0;
	req.msg.data = msg_data;
	req.msg.data_len = 2;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else { //successful response, then get oem data
		if (rsp->data_len >= sizeof(struct _ipmi_get_channel_auth_cap)) {
			memcpy(&authcap, rsp->data, sizeof(struct _ipmi_get_channel_auth_cap));

			printf("OEM Auxiliary Data         : 0x%x\n",
						   authcap.oem_aux_data);

			char *type;
		    type = ((authcap.oem_aux_data>>6) == 0?"IPMI":
					(authcap.oem_aux_data>>6) == 1?"REST":"Unknown");

	    	printf("Server communication type  : %s\n",type);

			printf("Slot Id                    : %d\n",rack_slot((int)(authcap.oem_aux_data&0x3f)));
			
			if(get_server_type(authcap.servertype, server_type_str) == 0)
			{
				printf("Server Type                : %s\n", server_type_str);
			}

			return 0;
		} else {
			lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
					val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
			return (-1);
		}
	}
	return 0;
}

// 0x38 = 0011 1000, So there are 8 combinations corresponding to the 3 one bit positions
static int rack_slot(int slot_id){

	switch(slot_id & 0x38)
	{
	case 0x00:
		return (slot_id+1);
	case 0x08:
			return (slot_id-1);
	case 0x10:
			return (slot_id-3);
	case 0x18:
			return (slot_id-5);
	case 0x20:
			return (slot_id-7);
	case 0x28:
			return (slot_id-9);
	case 0x30:
			return (slot_id-11);
	case 0x38:
			return (slot_id-13);
	}
	return 0;
}

/*
 read group of gpio and returns a single bit map. */

int ipmi_ocs_group_gpio_read(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	uint8_t slot_id = 0;

	memset(&req, 0, sizeof(req));
	unsigned int arg_idx = 1;

	if (argc != 3) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		get_group_gpio_usage();
		return -1;
	}

	uint8_t pinnum;

	if (str2uchar(argv[arg_idx], &pinnum) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert pin number to byte : %s \n",
				argv[arg_idx]);
		return -1;
	}
	arg_idx++;
	uint8_t read_len;

	if (str2uchar(argv[arg_idx], &read_len) != 0) {
		lprintf(LOG_ERR,
				"Error: Failed to convert gpio read length number to byte : %s \n",
				argv[arg_idx]);
		return -1;
	}
	if((read_len < 1 || read_len > (sizeof(uint8_t)*8) || pinnum + read_len > 215)) {
		lprintf(LOG_ERR,
				"Error: gpio out of range: gpio number: %d read pins: %d \n", pinnum, read_len);
		return -1;
	}
	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_GPIO;
	req.msg.lun = 0;
	req.msg.data_len = 1;

	int idx = 0;
	for(; idx <= read_len; idx++) {
		pinnum = pinnum+idx;

		req.msg.data = &pinnum;
		rsp = intf->sendrecv(intf, &req);

		if (rsp == NULL) {
			lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
					val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code),
					CC_SEND_RECEIVE_FAILED);
			return (-1);
		} else if (rsp->ccode > 0) {
			lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
					val2str(rsp->ccode, completion_code_vals), rsp->ccode);
			return (-1);
		} else {
			if (rsp->data_len >= 2) {
				slot_id = (uint8_t)(
						slot_id | ((uint8_t)(rsp->data[1] & 0x01) << idx));
			} else {
				lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
						val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code),
						CC_INVALID_RESPONSE_LEN);
				return (-1);
			}
		}
	}
	printf("Slot_Id: %d\n", rack_slot(slot_id));
	return 0;
}

int ipmi_ocs_get_jtag_program_status(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_jtag_program_status jtag;

	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_GET_JTAG_PROGRAM_STATUS;
	req.msg.lun = 0;
	req.msg.data_len = 0;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (rsp->data_len >= sizeof(struct _ipmi_jtag_program_status)) {
			memcpy(&jtag, rsp->data, sizeof(struct _ipmi_jtag_program_status));

			printf("Jtag Program Status : %u \n ", jtag.programmingstatus);

			return 0;
		} else {
			lprintf(LOG_ERR, "Error: (%s) %d\n",
				val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
			return (-1);
		}
	}
	return 0;
}

void Display_AdrTriggerType(uint8_t adrTrigger);

int ipmi_ocs_nv_dimm_trigger(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_nv_dimm_trigger nvDimmTrigger;

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_NV_DIMM_TRIGGER;
	req.msg.lun = 0;
	req.msg.data_len = 0;

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (rsp->data_len >= sizeof(struct _ipmi_nv_dimm_trigger)) {
			memcpy(&nvDimmTrigger, rsp->data, sizeof(struct _ipmi_nv_dimm_trigger));

			printf("ADR trigger: %u \n ", nvDimmTrigger.adrTrigger);
			printf("ADR Complete Power Off Delay: %u \n ", nvDimmTrigger.adrCompletedelay);
			printf("NVDIMM Present Power Off Delay: %u \n ", nvDimmTrigger.nvDimmPresentPowerOffDelay);
			printf("ADR Complete: %u \n ", nvDimmTrigger.adrComplete);
			printf("ADR Complete Time Remaining: %u \n ", nvDimmTrigger.adrCompleteTimeRemaining);
			printf("NVDIMM Present Time Remaining: %u \n", nvDimmTrigger.nvDimmPresentTimeRemaining);

			Display_AdrTriggerType(nvDimmTrigger.adrTrigger);

			return 0;
		} else {
			lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				 val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code),	CC_INVALID_RESPONSE_LEN);
			return (-1);
		}

	}
	return 0;
}

void Display_AdrTriggerType(uint8_t adrTrigger) {
	printf("ADR Trigger type is: ");

	switch (adrTrigger) {
	case 0:
		printf("%s \n", adr_trigger_type[0]);
		break;

	case 1:
		printf("%s \n", adr_trigger_type[1]);
		break;

	case 2:
		printf("%s \n", adr_trigger_type[2]);
		break;

	case 3:
		printf("%s \n", adr_trigger_type[3]);
		break;

	default:
		printf("%s \n", adr_trigger_type[4]);
		break;
	}
}

void display_processortype(uint8_t processortype, uint8_t processorId) {
	printf("Processor%d Type : ", processorId);

	switch (processortype) {
	case 21:
		printf("%s \n", processor_type[0]);
		break;

	case 22:
		printf("%s \n", processor_type[1]);
		break;

	case 23:
		printf("%s \n", processor_type[2]);
		break;

	case 24:
		printf("%s \n", processor_type[3]);
		break;

	case 25:
		printf("%s \n", processor_type[4]);
		break;

	case 32:
		printf("%s \n", processor_type[5]);
		break;

	case 255:
		printf("%s \n", processor_type[6]);
		break;

	default:
		printf("%s \n", processor_type[7]);
		//break;
	}
}

int ipmi_ocs_get_processor_info(struct ipmi_intf *intf, int argc, char **argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_processor_info processorinfo;
	unsigned int arg_idx = 1;

	if (argc != 2) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		get_processor_info_usage();
		return -1;
	}
	uint8_t processorid;

	if (str2uchar(argv[arg_idx], &processorid) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert processor index to byte : %s \n", argv[arg_idx]);
		return -1;
	}

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_PROCESSOR_INFO;
	req.msg.lun = 0;
	req.msg.data = &processorid;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
			 val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (rsp->data_len >= sizeof(struct _ipmi_processor_info)) {
			memcpy(&processorinfo, rsp->data, sizeof(struct _ipmi_processor_info));

			//printf("Processor Type: %0x%02x \n", processorinfo.type);
			display_processortype(processorinfo.type, processorid);
			printf("Processor%d Frequency: %u \n",processorid, processorinfo.frequency);

			if(processorinfo.state == 1)
			{
				printf("Processor%d State: %s \n", processorid, "Present");
			}
			else if(processorinfo.state == 0xFF)
			{
				printf("Processor%d State: %s \n",processorid, "Not Present");
			}
			else {
				printf("Processor%d State: %s \n", processorid, "Unknown");
			}
			return 0;
		} else {
			lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				 val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
			return (-1);
		}
	}
	return 0;
}

int ipmi_ocs_get_psu_alert(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_psu_alert psualert;

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_PSU_ALERT;
	req.msg.lun = 0;
	req.msg.data_len = 0;

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
			val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
			val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (rsp->data_len >= sizeof(struct _ipmi_psu_alert)) {
			memcpy(&psualert, rsp->data, sizeof(struct _ipmi_psu_alert));
			printf("Psu Alert:\n");
			printf("	Psu Alert Status: %u \n", psualert.alertstatus);
			printf("	Psu Alertgpi: %s \n",
					((psualert.alertstatus & 0x40) == 0x40) ? "True" : "False");
			printf("	Auto Prochot Enabled: %s \n",
					((psualert.alertstatus & 0x10) == 0x10) ? "True" : "False");
			printf("	Bmc Prochot Enabled: %s \n",
					((psualert.alertstatus & 0x01) == 0x01) ? "True" : "False");
			return 0;
	} else {
			lprintf(LOG_ERR, "Error: (%s) %d\n",
				val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
			return (-1);
		}
	}
	return 0;
}

int ipmi_ocs_pcie_info(struct ipmi_intf *intf) {

	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	memset(&req, 0, sizeof(req));
	uint8_t pcieid = 0;

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_PCIE_INFO;
	req.msg.lun = 0;
	req.msg.data = &pcieid;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	}

	//keep a local copy of the rsp because the rsp content may change when we invoke ocs_pcie_info()
	struct ipmi_rs rsp_copy;
	memcpy(&rsp_copy, rsp, sizeof(struct ipmi_rs));

	for(int idx = 0; idx < rsp_copy.data_len; idx++){
		for(int bit = 0; bit < 8; bit++){
			if((rsp_copy.data[idx]  >> bit) & 0x01 == 0x01) {
				uint8_t val = ((idx == 0)? idx:(idx==1)?8:16) + (bit+1);
				ocs_pcie_info(intf, val);
			}
			else {
					printf("PCIe Id: %d\n", ((idx == 0)? idx:(idx==1)?8:16) + (bit+1));
					printf("PCIe Status: Not Present\n");
			}
			printf("*******************************\n"); // Using this printf to parse the data in python middle tier scripts
		}
	}
	return 0;
}

int ocs_pcie_info(struct ipmi_intf *intf, int val){
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_pcie_info_response_map pcieRespMap;
	struct _ipmi_pcie_info_response pcieResp;
	uint8_t pcieid= val;

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_PCIE_INFO;
	req.msg.lun = 0;
	req.msg.data = &pcieid;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		printf("Null block");
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		printf("Completion Code is : %d\n", rsp->ccode);
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
			if (rsp->data_len >= sizeof(struct _ipmi_pcie_info_response)) {
				memcpy(&pcieResp, rsp->data, sizeof(struct _ipmi_pcie_info_response));

				printf("PCIe Id: %d\n", val);
				printf("PCIe Status: Present\n");
				printf("Device Id: %u \n", pcieResp.deviceId);
				printf("Vendor Id: %u \n", pcieResp.vendorId);
				printf("SubSystem Id: %u \n", pcieResp.subSystemId);
				printf("SubSystem vendor Id: %u \n", pcieResp.subSystemVendorId);
				return 0;
			} else {
				lprintf(LOG_ERR,"Error: %s \nCompletion Code: %d\n",
						val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
				return (-1);
			}
		}

	return 0;
}

int ipmi_ocs_get_pcie_info_response(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_pcie_info_response_map pcieRespMap;
	struct _ipmi_pcie_info_response pcieResp;
	unsigned int arg_idx = 1;

	if (argc != 2) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		get_pcie_info_usage();
		return -1;
	}
	uint8_t pcieid;

	if(str2uchar(argv[arg_idx], &pcieid) != 0) {
		lprintf(LOG_ERR,
				"Error: Failed to convert pcie slot index to byte : %s \n", argv[arg_idx]);
		return -1;
	}

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_PCIE_INFO;
	req.msg.lun = 0;
	req.msg.data = &pcieid;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		printf("Null block");
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		printf("Completion Code is : %d\n", rsp->ccode);
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (pcieid == 0) { //Response map
			if (rsp->data_len >= sizeof(struct _ipmi_pcie_info_response_map)) {
			    memcpy(&pcieRespMap, rsp->data, sizeof(struct _ipmi_pcie_info_response_map));

				printf("PCIe Info response Map:\n");
				printf("	pcie Presence info bitmap for slots 8 to 1: %u \n", pcieRespMap.slot8to1);
				printf("	pcie Presence info bitmap for slots 16 to 9: %u \n", pcieRespMap.slot16to9);
				printf("	pcie Presence info bitmap for slots 21 to 17: %u \n", pcieRespMap.slot21to17);
				return 0;
			} else {
				lprintf(LOG_ERR,"Error: (%s) %d\n",
						val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
				return (-1);
			}

		} else if (pcieid != 0) {
			if (rsp->data_len >= sizeof(struct _ipmi_pcie_info_response)) {
				memcpy(&pcieResp, rsp->data, sizeof(struct _ipmi_pcie_info_response));

				printf("Device Id: %u \n", pcieResp.deviceId);
				printf("Vendor Id: %u \n", pcieResp.vendorId);
				printf("SubSystem Id: %u \n", pcieResp.subSystemId);
				printf("SubSystem vendor Id: %u \n", pcieResp.subSystemVendorId);
				return 0;
			} else {
				lprintf(LOG_ERR,"Error: %s \nCompletion Code: %d\n",
						val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
				return (-1);
			}
		} else {
			lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
					val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
			return (-1);
		}

	}
	return 0;
}

int sethelpermethod(struct ipmi_rs *rsp) {
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else if (rsp->ccode == 0) {
		printf("Completion Status: Success\n");
		return 0;
	} else {
		lprintf("Error: %s \nCompletion Code %d\n",
				val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
		return (-1);
	}
}

int ipmi_ocs_set_default_power_limit(struct ipmi_intf *intf, int argc,
		char ** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	struct _ipmi_set_default_powerlimit powerlimit;

	if (argc != 4) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		set_default_power_limit_usage();
		return -1;
	}

	if (str2short(argv[1], &powerlimit.dpc) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert dpc string to short: %s \n", argv[1]);
		return -1;
	}
	if (str2short(argv[2], &powerlimit.delay) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert delay string to short: %s \n", argv[2]);
		return -1;
	}
	if (str2uchar(argv[3], &powerlimit.autoremoveDelayInMilliSec) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert autoremoveDelayInMilliSec string to byte: %s \n",argv[3]);
		return -1;
	}
	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_SET_DEFAULT_POWER_LIMIT;
	req.msg.lun = 0;
	req.msg.data = &powerlimit;
	req.msg.data_len = sizeof(struct _ipmi_set_default_powerlimit);

	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

int ipmi_ocs_set_bios_config(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	uint8_t configuration;
	uint8_t minorconfiguration;
	uint8_t majorconfiguration;

	if (argc != 3) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		set_bios_config_usage();
		return -1;
	}

	if (str2uchar(argv[1], &majorconfiguration) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the majorConfiguration to char type: %s \n",argv[1]);
		return -1;
	}
	if (str2uchar(argv[2], &minorconfiguration) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the minorConfiguration to char type: %s \n",
				argv[2]);
		return -1;
	}

	//Minor Bios Flavor
	configuration = (minorconfiguration & 0x07) << 4;

	//Major Bios Flavor
	configuration = (configuration | (majorconfiguration & 0x0F));

	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_SET_BIOS_CONFIG;
	req.msg.lun = 0;
	req.msg.data = &configuration;
	req.msg.data_len = sizeof(char);

	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

int ipmi_ocs_set_jtag_program_status(struct ipmi_intf *intf, int argc, char ** argv) {

	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	uint8_t jtagdevice;

	if (argc != 2) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		set_jtag_program_status_usage();
		return -1;
	}

	if (str2uchar(argv[1], &jtagdevice) != 0) {
			printf("Failed to convert the jtag device argument to byte: %s \n ", argv[1]);
			return -1;
		}

	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_SET_JTAG_PROGRAM_STATUS;
	req.msg.data = &jtagdevice;
	req.msg.lun = 0;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

int ipmi_ocs_set_gpio(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_set_gpio setgpio;

	if (argc != 4) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		set_gpio_usage();
		return -1;
	}

	if (str2uchar(argv[1], &setgpio.pinnumber) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the pinNumber argument to byte: %s \n ", argv[1]);
		return -1;
	}
	if (str2uchar(argv[2], &setgpio.pindirection) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the pinDirection argument to byte: %s \n ", argv[2]);
		return -1;
	}
	if (str2uchar(argv[3], &setgpio.pinstate) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the pinNumber argument to byte: %s \n ", argv[3]);
		return -1;
	}

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_SET_GPIO;
	req.msg.lun = 0;
	req.msg.data = &setgpio;
	req.msg.data_len = sizeof(struct _ipmi_set_gpio);

	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

int ipmi_ocs_set_psu_alert(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	if (argc != 2) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		set_psu_alert_usage();
		return -1;
	}
	uint8_t enable;

	if(strcmp(argv[1],"true") == 0)
	{
		enable = 0x01;
	}
	else
	{
		enable  = 0x00;
	}

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_SET_PSU_ALERT;
	req.msg.lun = 0;
	req.msg.data = &enable;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

int ipmi_ocs_set_energy_storage(struct ipmi_intf *intf, int argc, char ** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	struct _ipmi_set_energy_storage setenergystorage;

	if (argc != 6) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		set_energy_storage_usage();
		return -1;
	}
	uint8_t initialstate;

	// if Battery presence is true then set the state to 1
	if (strcmp(argv[1], "true") == 0) {
		initialstate = 0x01;
	}
	else {
		initialstate = 0x00;
	}

	// Battery State
	if(str2uchar(argv[2],&setenergystorage.presence_state) != 0)
	{
		lprintf(LOG_ERR, "Error: Failed to convert state string val to char: %s \n",argv[2]);
				return -1;
	}

	setenergystorage.presence_state = (initialstate | (setenergystorage.presence_state << 2));

	// Reserved
	setenergystorage.reserved = 0x00;

	// Energy scaling factor
	if (str2uchar(argv[3], &setenergystorage.scalefactor) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert scaling factor string to char: %s \n", argv[3]);
		return -1;
	}
	// Blade Energy
	if (str2ushort(argv[4], &setenergystorage.systemenergy) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert blade energy string to ushort: %s \n", argv[4]);
		return -1;
	}
	// NVDIMM energy
	if (str2uchar(argv[5], &setenergystorage.nvdimmenergy) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert nvdimm energy string to char: %s \n", argv[5]);
		return -1;
	}
	// RollingCounter
	struct timeval tv;

	gettimeofday(&tv, NULL);

	unsigned long long secondssinceepoch = (unsigned long long)(tv.tv_sec) + (unsigned long long)(tv.tv_usec) / (1000*1000);

	//Confirmed that the long long value is converted to ushort automatically when we assign. So not casting explicitly.
	setenergystorage.rollingcounter = secondssinceepoch;

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_SET_ENERGY_STORAGE;
	req.msg.lun = 0;
	req.msg.data = &setenergystorage;
	req.msg.data_len = 8;

	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

int ipmi_ocs_activate_psu_alert(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct _ipmi_activate_psu_alert activatepsualert;

	if (argc != 3) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		activate_psu_alert_usage();
		return -1;
	}

	uint8_t bmcaction;

	if(str2uchar(argv[1], &bmcaction) != 0)
	{
	 	lprintf(LOG_ERR, "Error: Failed to convert the bmcaction argument to byte: %s \n ", argv[1]);
 		return -1;
	}

	activatepsualert.autoprochot = bmcaction;

	if(str2uchar(argv[2], &activatepsualert.removedpc) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the remove dpc argument to byte: %s \n ", argv[2]);
		return -1;
	}

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_ACTIVATE_PSU_ALERT;
	req.msg.lun = 0;
	req.msg.data = &activatepsualert;
	req.msg.data_len = sizeof(struct _ipmi_activate_psu_alert);

	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

int ipmi_ocs_set_nvdimm_trigger(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
    struct _ipmi_set_nv_dimm_trigger setnvdimmtrigger;

	if (argc != 4) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		set_nvdimm_trigger_usage();
		return -1;
	}

	if (strcmp(argv[1], "unknown") == 0) {
		setnvdimmtrigger.adrTrigger = 0x00; //If user sets trigger type to unknown it will be disabled.
	}
	else
	{
		if (str2uchar(argv[1], &setnvdimmtrigger.adrTrigger) != 0) {
			lprintf(LOG_ERR, "Error: Failed to convert the adr triggerargument to byte: %s \n ",argv[1]);
				return -1;
			}
	}

	if (str2uchar(argv[2], &setnvdimmtrigger.adrCompletepoweroffdelay) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the adr complete poweroff delay argument to byte: %s \n ", argv[2]);
		return -1;
	}
	if (str2uchar(argv[3], &setnvdimmtrigger.nvDimmPresentPowerOffDelay) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert the nv dimm present power off delay argument to byte: %s \n ", argv[3]);
		return -1;
	}

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_SET_NV_DIMM_TRIGGER;
	req.msg.lun = 0;
	req.msg.data = &setnvdimmtrigger;
	req.msg.data_len = sizeof(setnvdimmtrigger);

	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

static void dimm_status(uint8_t status){
	printf("Dimm Status: ");
	switch (status) {
		case 0x0:
			printf("Reserved\n");
			break;
		case 0x1:
			printf("Unknown DIMM type\n");
			break;
		case 0x2:
			printf("Present\n");
			break;
		case 0x3:
			printf("Not present\n");
			break;
		case 0x5:
			printf("Single bit error\n");
			break;
		case 0x7:
			printf("Multi bit error\n");
			break;
		default:
			printf("Failure\n");
	}
}

static void dimm_type(uint8_t type){
	printf("Dimm Type: ");
	switch (type) {
		case 0:
			printf("SDRAM\n");
			break;
		case 1:
			printf("DDR-1 RAM\n");
			break;
		case 2:
			printf("Rambus\n");
			break;
		case 3:
			printf("DDR-2 RAM\n");
			break;
		case 4:
			printf("FBDIMM\n");
			break;
		case 5:
			printf("DDR-3 RAM\n");
			break;
		case 6:
			printf("DDR-4 RAM\n");
			break;
		case 7:
			printf("DDR-3 NVDIMM\n");
			break;
		case 8:
			printf("DDR-4 NVDIMM\n");
			break;
		case 9:
			printf("DDR-3 NVDIMM with supercap\n");
			break;
		case 10:
			printf("DDR-4 NVDIMM with supercap\n");
			break;
		case 63:
			printf("No DIMM Present\n");
			break;
		default:
			printf("Failure\n");
	}
}

int ipmi_ocs_memory_info(struct ipmi_intf *intf, char* cmdname) {

	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	memset(&req, 0, sizeof(req));
	uint8_t dimmid = 0;

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_MEMORY_INFO;
	req.msg.lun = 0;
	req.msg.data = &dimmid;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	}

	//keep a local copy of the rsp because the rsp content may change when we invoke ipmi_ocs_info
	struct ipmi_rs rsp_copy;
	memcpy(&rsp_copy, rsp, sizeof(struct ipmi_rs));
	unsigned long memory_size = 0;

	for(int idx = 0; idx < rsp_copy.data_len; idx++){
		for(int bit = 0; bit < 8; bit++){
			if((rsp_copy.data[idx]  >> bit) & 0x01 == 0x01) {
				uint8_t val = ((idx == 0)? idx:(idx==1)?8:16) + (bit+1);
				if(strcmp(cmdname, "dimminfo\0") == 0){
					ipmi_ocs_info(intf, val);
				}
				else if (strcmp(cmdname,"totalmemory\0") == 0){
					memory_size += ipmi_ocs_get_total_memory(intf,val);
				}
			}
			else {
				if(strcmp(cmdname, "dimminfo\0") == 0){
					printf("DimmId: %d\n", ((idx == 0)? idx:(idx==1)?8:16) + (bit+1));
					printf("Dimm Status: Not Present\n");
				}
			}
			printf("*******************************\n");
		}
	}
	if (strcmp(cmdname,"totalmemory\0") == 0){
		printf("TotalSystemMemoryGiB: %lu\n",memory_size/1024);
	}
	return 0;
}

int ipmi_ocs_info(struct ipmi_intf *intf, int val) {

	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	struct _ipmi_get_memory_info meminfo;
	struct _ipmi_get_memory_index memindex;

	uint8_t dimmid = val;

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_MEMORY_INFO;
	req.msg.lun = 0;
	req.msg.data = &dimmid;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
			if (rsp->data_len >= sizeof(struct _ipmi_get_memory_info)) {
				memcpy(&meminfo, rsp->data,	sizeof(struct _ipmi_get_memory_info));

				printf("DimmId: %u\n", dimmid);

				uint8_t memtype = (uint8_t)(meminfo.type & 0x3F); ////type = bit[5:0] of type byte
				dimm_type(memtype);

				uint8_t runnningspeed = (uint8_t)((meminfo.type & 0x40) >> 6); //Bit[6] indicates running speed
				if (runnningspeed == 1) {
					printf("Running Speed: %s{%u} \n", "Running at rated speed",runnningspeed);
				} else if (runnningspeed == 0) {
					printf("Running Speed: %s{%u} \n", "Not running at rated speed",runnningspeed);
				} else {
					printf("Running Speed: %s{%u} \n", "Failure",runnningspeed);
				}

				uint8_t voltage = (uint8_t)((meminfo.type & 0x80) >> 7); //Bit[7] indicates Voltage
				if (voltage == 1) {
					printf("Voltage: %s{%u} \n", "Low Voltage",voltage);
				} else if (voltage == 0) {
					printf("Voltage: %s{%u} \n", "Normal Voltage",voltage);
				} else {
					printf("Voltage: %s{%u} \n", "Failure",voltage);
				}

				printf("Dimm speed: %u \n", meminfo.dimmspeed);
				printf("Dimm size: %u \n", meminfo.dimmsize);

				dimm_status(meminfo.dimmstatus);
				return 0;
			} else {
				lprintf(LOG_ERR, "Error: (%s) %d\n",
						val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
				return (-1);
			}
		}

	return 0;
}

int ipmi_ocs_get_memory_info(struct ipmi_intf *intf, int argc, char** argv) {

	if (argc != 2) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		get_memory_info_usage();
		return -1;
	}

	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	struct _ipmi_get_memory_info meminfo;
	struct _ipmi_get_memory_index memindex;

	unsigned int arg_idx = 1;
	uint8_t dimmid;


	if (str2uchar(argv[arg_idx], &dimmid) != 0) {
		lprintf(LOG_ERR, "Error: Failed to convert dimmid to byte : %s \n", argv[arg_idx]);
		return -1;
	}
	if(dimmid > 24)
	{
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_PARAMETER_OUTOF_RANGE, ocs_completion_code), CC_PARAMETER_OUTOF_RANGE);
		return (-1);
	}

	req.msg.netfn = OEM_GP2_NETFN;
	req.msg.cmd = CMD_GET_MEMORY_INFO;
	req.msg.lun = 0;
	req.msg.data = &dimmid;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (dimmid == 0) { //Memory Index
			if (rsp->data_len >= sizeof(struct _ipmi_get_memory_index)) {
				memcpy(&memindex, rsp->data, sizeof(struct _ipmi_get_memory_index));

				printf("Get Memory Index map for the dimmId: %u :\n", dimmid);

				printf("Dimm Presence map 1 to 8: %u \n",	memindex.map1to8);
				printf("Dimm Presence map 9 to 16: %u \n",	memindex.map9to16);
				printf("Dimm Presence map 17 to 24: %u \n",	memindex.map17to24);

				return 0;
			} else {
				lprintf(LOG_ERR, "Error: (%s) %d\n",
						val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
				return (-1);
			}

		} else if (dimmid != 0) {
			if (rsp->data_len >= sizeof(struct _ipmi_get_memory_info)) {
				memcpy(&meminfo, rsp->data,	sizeof(struct _ipmi_get_memory_info));

				printf("DimmId: %u\n", dimmid);

				uint8_t memtype = (uint8_t)(meminfo.type & 0x3F); ////type = bit[5:0] of type byte
				dimm_type(memtype);

				uint8_t runnningspeed = (uint8_t)((meminfo.type & 0x40) >> 6); //Bit[6] indicates running speed
				if (runnningspeed == 1) {
					printf("Running Speed: %s{%u} \n", "Running at rated speed",runnningspeed);
				} else if (runnningspeed == 0) {
					printf("Running Speed: %s{%u} \n", "Not running at rated speed",runnningspeed);
				} else {
					printf("Running Speed: %s{%u} \n", "Failure",runnningspeed);
				}

				uint8_t voltage = (uint8_t)((meminfo.type & 0x80) >> 7); //Bit[7] indicates Voltage
				if (voltage == 1) {
					printf("Voltage: %s{%u} \n", "Low Voltage",voltage);
				} else if (voltage == 0) {
					printf("Voltage: %s{%u} \n", "Normal Voltage",voltage);
				} else {
					printf("Voltage: %s{%u} \n", "Failure",voltage);
				}

				printf("Dimm speed: %u \n", meminfo.dimmspeed);
				printf("Dimm size: %u \n", meminfo.dimmsize);

				dimm_status(meminfo.dimmstatus);
				return 0;
			} else {
				lprintf(LOG_ERR, "Error: (%s) %d\n",
						val2str(CC_INVALID_RESPONSE_LEN, ocs_completion_code), CC_INVALID_RESPONSE_LEN);
				return (-1);
			}
		} else {
			lprintf(LOG_ERR, "Error: %s %d\n",
					val2str(CC_UNSPECIFIED_ERROR, ocs_completion_code), CC_UNSPECIFIED_ERROR);
			return (-1);
		}

	}
	return 0;
}

int ipmi_ocs_get_total_memory(struct ipmi_intf *intf, int dimm_id)
{
	struct ipmi_rs *rsp;
		struct ipmi_rq req;

		struct _ipmi_get_memory_info meminfo;
		struct _ipmi_get_memory_index memindex;

		unsigned long totalmemorygib;
		uint8_t dimmid = dimm_id;

		req.msg.netfn = OEM_GP2_NETFN;
		req.msg.cmd = CMD_GET_MEMORY_INFO;
		req.msg.lun = 0;
		req.msg.data = &dimmid;
		req.msg.data_len = 1;

		rsp = intf->sendrecv(intf, &req);

		if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code),
				CC_SEND_RECEIVE_FAILED);
		return (-1);
		} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
		} else {
		if (rsp->data_len >= sizeof(struct _ipmi_get_memory_info)) {
			memcpy(&meminfo, rsp->data, sizeof(struct _ipmi_get_memory_info));
			return meminfo.dimmsize;
		}
		}
}

int ipmi_ocs_get_tpm_physical_presence(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_GET_TPM_PHYSICAL_PRESENCE;
	req.msg.lun = 0;
	req.msg.data_len = 0;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {

		printf("Physical Presence Flag: %s\n", (*rsp->data == 1 ? "True" : "False"));
	}
	return 0;
}

int ipmi_ocs_set_tpm_physical_prsence(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	if (argc != 2) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.");
		set_psu_alert_usage();
		return -1;
	}
	uint8_t presence;

	if(strcmp(argv[1],"true") == 0)
	{
		presence = 0x01;
	}
	else if(strcmp(argv[1],"false") == 0)
	{
		presence  = 0x00;
	}
	else {
	 lprintf(LOG_ERR, "Error: %s \nCompletion Code: %d\n",
		val2str(CC_INVALID_DATA_FIELD_IN_REQUEST, ocs_completion_code), CC_INVALID_DATA_FIELD_IN_REQUEST);
				return (-1);
	}

	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_SET_TPM_PHYSICAL_PRESENCE;
	req.msg.lun = 0;
	req.msg.data = &presence;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

int ipmi_ocs_get_led_status(struct ipmi_intf *intf)
{
		struct ipmi_rs * rsp;
		struct ipmi_rq req;

		memset(&req, 0, sizeof(req));
		req.msg.netfn = 0x0;
		req.msg.cmd = 0x1;

		rsp = intf->sendrecv(intf, &req);
		if (rsp == NULL) {
			lprintf(LOG_ERR, "Error sending Chassis Status command");
			return -1;
		}
		if (rsp->ccode > 0) {
			lprintf(LOG_ERR, "Error sending Chassis Status command: %s",
					val2str(rsp->ccode, completion_code_vals));
			return -1;
		}

	/* byte 3 */
		printf("LED Status : ");
			switch ((rsp->data[2] & 0x30) >> 4) {
			case 0x0:
				printf("Off\n");
				break;
			case 0x1:
				printf("Temporary (timed) On\n");
				break;
			case 0x2:
				printf("On\n");
				break;
			case 0x3:
			default:
				printf("unknown\n");
			}
	return 0;
}


int ipmi_system_status(struct ipmi_intf * intf)
{
	struct ipmi_rs * rsp;
	struct ipmi_rq req;

	memset(&req, 0, sizeof(req));
	req.msg.netfn = IPMI_NETFN_CHASSIS;
	req.msg.cmd = 0x1;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error sending Chassis Status command");
		return -1;
	}
	if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error sending Chassis Status command: %s",
				val2str(rsp->ccode, completion_code_vals));
		return -1;
	}

	/* byte 1 */
	printf("System Power State: %s\n", (rsp->data[0] & 0x1) ? "On" : "Off");
	return 0;
}

int ipmi_ocs_psu_fw_update (struct ipmi_intf *intf, int argc, char **argv) {
	struct ipmi_rs *resp;
	struct ipmi_rq req;
	struct _ipmi_psu_fw_update fw;

	if (argc < 4) {
		lprintf (LOG_ERR, "Error: Not enough parameters given.");
		psu_fw_update_usage ();
		return -1;
	}

	if (str2uchar (argv[1], &fw.component) != 0) {
		lprintf (LOG_ERR, "Failed to convert comptype to byte: %s\n", argv[1]);
		return -1;
	}

	if (str2uchar (argv[2], &fw.type) != 0) {
		lprintf (LOG_ERR, "Failed to convert imgtype to byte: %s\n", argv[2]);
		return -1;
	}

	if (str2uchar (argv[3], &fw.operation) != 0) {
		lprintf (LOG_ERR, "Failed to convert op to byte: %s\n", argv[3]);
		return -1;
	}

	if (fw.operation == 1) {
		if (argc < 5) {
			lprintf (LOG_ERR, "Error: Not enough parameters given.");
			psu_fw_update_usage ();
			return -1;
		}

		strncpy (fw.image, argv[4], PSU_FW_NAME_MAX_LEN);
		if (fw.image[PSU_FW_NAME_MAX_LEN - 1] != '\0') {
			lprintf (LOG_ERR, "PSU firmware image file name is too long: %s\n", argv[4]);
			return -1;
		}
	}

	memset (&req, 0, sizeof (req));
	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_CONFIG_FW_UPDATE;
	req.msg.lun = 0;
	req.msg.data = (uint8_t*) &fw;
	req.msg.data_len = sizeof (fw);

	fw.component |= (1 << 4);

	resp = intf->sendrecv (intf, &req);
	if (resp == NULL) {
		lprintf (LOG_ERR, "Error sending PSU FW Update command");
		return -1;
	}
	if (resp->ccode != 0) {
		lprintf (LOG_ERR, "Error sending PSU FW Update command: %s",
			val2str (resp->ccode, ocs_completion_code));
		return -1;
	}

	printf ("FW Update Status: ");
	switch (resp->data[0]) {
		case 1:
			printf ("FW image not found\n");
			break;

		case 2:
			printf ("FW update not started\n");
			break;

		case 3:
			printf ("FW update in progress\n");
			break;

		case 4:
			printf ("FW update completed\n");
			break;

		case 5:
			printf ("FW update aborted\n");
			break;

		case 6:
			printf ("FW image corrupted\n");
			break;

		default:
			printf ("unknown\n");
			break;
	}

	return 0;
}

enum {
	PMBUS_CLEAR_FAULT = 0x03,
	PMBUS_CAPABILITY = 0x19,
	PMBUS_STATUS_WORD = 0x79,
	PMBUS_READ_VIN = 0x88,
	PMBUS_READ_IIN = 0x89,
	PMBUS_READ_IOUT = 0x8C,
	PMBUS_READ_POUT = 0x96,
	PMBUS_READ_PIN = 0x97,
	PMBUS_MFR_ID = 0x99,
	PMBUS_MFR_MODEL = 0x9A,
	PMBUS_MFR_SERIAL = 0x9E,
	PMBUS_IC_DEVICE_ID = 0xAD,
	PMBUS_PSU_FEED = 0xD1,
	PMBUS_BATT_STATUS = 0xD6,
	PMBUS_BATT_HEALTH_TEST = 0xD8,
	PMBUS_READ_FW_INFO = 0xEF,
};

#define	OCS_PSU_BUS_NUM		1
#define	OCS_PSU_BUS_ADDR	0x58

#define	PMBUS_GET_WORD(x)	((((uint16_t) x[1]) << 8) | x[0])

static int ipmi_ocs_master_phase_write_read (struct ipmi_intf *intf,
	struct _ipmi_master_phase_write_read *xfer, uint16_t wlength, uint8_t *rdata)
{
	struct ipmi_rq req;
	struct ipmi_rs *resp;

	memset (&req, 0, sizeof (req));
	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_MASTER_PHASE_WRITE_READ;
	req.msg.lun = 0;
	req.msg.data = (uint8_t*) xfer;
	req.msg.data_len = MASTER_PHASE_WRITE_READ_MIN_LEN + wlength;

	resp = intf->sendrecv (intf, &req);
	if (resp == NULL) {
		return -1;
	}
	if (resp->ccode != 0) {
		return resp->ccode;
	}

	if (rdata != NULL) {
		memcpy (rdata, resp->data, xfer->rlen);
	}
	return 0;
}

static double ipmi_ocs_convert_pmbus_linear_data (uint16_t data)
{
	int8_t exponent = (int16_t) data >> 11;
	int16_t mantissa = data & 0x7ff;
	return mantissa * pow (2, exponent);
}

static int ipmi_ocs_psu_init (int argc, char *argv[], struct _ipmi_master_phase_write_read *xfer,
	uint8_t *phase)
{
	memset (xfer, 0, sizeof (*xfer));
	xfer->bus = OCS_PSU_BUS_NUM;
	xfer->type = 1;
	xfer->address = OCS_PSU_BUS_ADDR;
	xfer->phase = 0xff;

	if (argc < 2) {
		lprintf (LOG_ERR, "Error: Not enough parameters given.");
		return -2;
	}

	*phase = 0xff;
	if (argc >= 3) {
		if (str2uchar (argv[2], phase) != 0) {
			lprintf (LOG_ERR, "Failed to convert phase select to byte: %s\n", argv[2]);
			return -1;
		}
	}

	return 0;
}

const char *PSU_STATUS_ALL[] = {"caps", "status", "vin", "iin", "pin", "iout", "pout", "mfr",
	"device", "model", "serial", "feed", "battery", ""};
const char *PSU_PHASE_STATUS_ALL[] = {"status", "vin", "iin", "pin", ""};

int ipmi_ocs_psu_read (struct ipmi_intf *intf, int argc, char *argv[])
{
	uint8_t phase;
	uint16_t wlen;
	uint8_t rdata[21];
	int status;
	struct _ipmi_master_phase_write_read xfer;
	const char **list;
	const char *cmd;
	int i;
	int skip_battery = 0;
	int active_fw = 0;

	status = ipmi_ocs_psu_init (argc, argv, &xfer, &phase);
	if (status != 0) {
		if (status == -2) {
			ocs_psu_read_usage ();
		}
		return -1;
	}

	if (strcmp (argv[1], "all") == 0) {
		i = 0;
		if (phase == 0xff) {
			list = PSU_STATUS_ALL;
		}
		else {
			list = PSU_PHASE_STATUS_ALL;
		}
		cmd = list[i];
	}
	else {
		list = NULL;
		cmd = argv[1];
	}

	do {
		wlen = 1;

		if (strcmp (cmd, "caps") == 0) {
			xfer.wdata[0] = PMBUS_CAPABILITY;
			xfer.rlen = 1;
		}
		else if (strcmp (cmd, "status") == 0) {
			xfer.wdata[0] = PMBUS_STATUS_WORD;
			xfer.phase = phase;
			xfer.rlen = 2;
		}
		else if (strcmp (cmd, "vin") == 0) {
			xfer.wdata[0] = PMBUS_READ_VIN;
			xfer.phase = phase;
			xfer.rlen = 2;
		}
		else if (strcmp (cmd, "iin") == 0) {
			xfer.wdata[0] = PMBUS_READ_IIN;
			xfer.phase = phase;
			xfer.rlen = 2;
		}
		else if (strcmp (cmd, "pin") == 0) {
			xfer.wdata[0] = PMBUS_READ_PIN;
			xfer.phase = phase;
			xfer.rlen = 2;
		}
		else if (strcmp (cmd, "iout") == 0) {
			xfer.wdata[0] = PMBUS_READ_IOUT;
			xfer.rlen = 2;
		}
		else if (strcmp (cmd, "pout") == 0) {
			xfer.wdata[0] = PMBUS_READ_POUT;
			xfer.rlen = 2;
		}
		else if (strcmp (cmd, "mfr") == 0) {
			xfer.wdata[0] = PMBUS_MFR_ID;
			xfer.rlen = 16;
		}
		else if (strcmp (cmd, "device") == 0) {
			xfer.wdata[0] = PMBUS_IC_DEVICE_ID;
			xfer.rlen = 21;
		}
		else if (strcmp (cmd, "model") == 0) {
			xfer.wdata[0] = PMBUS_MFR_MODEL;
			xfer.rlen = 16;
		}
		else if (strcmp (cmd, "serial") == 0) {
			xfer.wdata[0] = PMBUS_MFR_SERIAL;
			xfer.rlen = 16;
		}
		else if (strcmp (cmd, "feed") == 0) {
			xfer.wdata[0] = PMBUS_PSU_FEED;
			xfer.rlen = 1;
		}
		else if (strcmp (cmd, "battery") == 0) {
			xfer.wdata[0] = PMBUS_BATT_STATUS;
			xfer.rlen = 2;
		}
		else if (strcmp (cmd, "fw") == 0) {
			if ((phase != 0xff) && (phase != 0xA) && (phase != 0xB) && (phase != 0)) {
				lprintf (LOG_ERR, "Error: Invalid FW identifier 0x%x.", phase);
				return -1;
			}

			xfer.wdata[0] = PMBUS_READ_FW_INFO;
			xfer.wdata[1] = 1;
			xfer.wdata[2] = (phase == 0xff) ? 0xA : phase;
			wlen = 3;
			xfer.rlen = 10;
		}
		else {
			lprintf (LOG_ERR, "Error: Unknown request.");
			ocs_psu_read_usage ();
			return -1;
		}

		status = ipmi_ocs_master_phase_write_read (intf, &xfer, wlen, rdata);
		if (status != 0) {
			lprintf (LOG_ERR, "Error sending PSU read request: %s\n",
				val2str (status, ocs_completion_code));
			return -1;
		}

		switch (xfer.wdata[0]) {
			case PMBUS_CAPABILITY:
				skip_battery = (rdata[0] & 0x8) ? 0 : 1;
				printf ("Capability: 0x%02x\n", rdata[0]);
				printf ("Battery: %s\n", (skip_battery) ? "No" : "Yes");
				break;

			case PMBUS_STATUS_WORD:
			{
				uint16_t word = PMBUS_GET_WORD (rdata);
				printf ("Status Word: 0x%04x\n", word);
				printf ("Faults: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
					(word & 0x80) ? "BUSY," : "",
					(word & 0x40) ? "OFF," : "",
					(word & 0x20) ? "VOUT OVERVOLTAGE," : "",
					(word & 0x10) ? "IOUT OVERCURRENT," : "",
					(word & 0x08) ? "VIN UNDERVOLTAGE," : "",
					(word & 0x04) ? "TEMPERATURE," : "",
					(word & 0x02) ? "COMM/MEM/LOGIC," : "",
					(word & 0x01) ? "MISC," : "",
					(word & 0x8000) ? "VOUT," : "",
					(word & 0x4000) ? "I/P OUT," : "",
					(word & 0x2000) ? "INPUT," : "",
					(word & 0x1000) ? "MFR SPECIFIC," : "",
					(word & 0x0800) ? "POWER GOOD," : "",
					(word & 0x0400) ? "FANS," : "",
					(word & 0x0200) ? "OTHER," : "",
					(word & 0x0100) ? "UNKNOWN" : "");
				break;
			}

			case PMBUS_READ_VIN:
				printf ("Vin: %fV\n", ipmi_ocs_convert_pmbus_linear_data (PMBUS_GET_WORD (rdata)));
				break;

			case PMBUS_READ_IIN:
				printf ("Iin: %fA\n", ipmi_ocs_convert_pmbus_linear_data (PMBUS_GET_WORD (rdata)));
				break;

			case PMBUS_READ_PIN:
				printf ("Pin: %fW\n", ipmi_ocs_convert_pmbus_linear_data (PMBUS_GET_WORD (rdata)));
				break;

			case PMBUS_READ_IOUT:
				printf ("Iout: %fA\n", ipmi_ocs_convert_pmbus_linear_data (PMBUS_GET_WORD (rdata)));
				break;

			case PMBUS_READ_POUT:
				printf ("Pout: %fW\n", ipmi_ocs_convert_pmbus_linear_data (PMBUS_GET_WORD (rdata)));
				break;

			case PMBUS_MFR_ID:
				if (rdata[0] == (xfer.rlen - 1)) {
					rdata[xfer.rlen] = '\0';
				}
				else {
					rdata[1] = '\0';
				}
				printf ("Manufacturer: %s\n", &rdata[1]);
				break;

			case PMBUS_IC_DEVICE_ID:
				if (rdata[0] == (xfer.rlen - 1)) {
					rdata[xfer.rlen] = '\0';
				}
				else {
					rdata[1] = '\0';
				}
				printf ("Device ID: %s\n", &rdata[1]);
				break;

			case PMBUS_MFR_MODEL:
				if (rdata[0] == (xfer.rlen - 1)) {
					rdata[xfer.rlen] = '\0';
				}
				else {
					rdata[1] = '\0';
				}
				printf ("Model: %s\n", &rdata[1]);
				break;

			case PMBUS_MFR_SERIAL:
				if (rdata[0] == (xfer.rlen - 1)) {
					rdata[xfer.rlen] = '\0';
				}
				else {
					rdata[1] = '\0';
				}
				printf ("Serial: %s\n", &rdata[1]);
				break;

			case PMBUS_PSU_FEED:
				printf ("PSU Feed: %s\n",
					(rdata[0] == 0xAA) ? "A" : ((rdata[0] == 0xBB) ? "B" : "Unknown"));
				break;

			case PMBUS_BATT_STATUS:
			{
				uint16_t word = PMBUS_GET_WORD (rdata);
				printf ("Battery Status: 0x%04x\n", word);
				printf ("Balancing: %s\n", (word & 0x80) ? "Active" : "Not Active");
				printf ("External Power: %s\n", (word & 0x40) ? "Present" : "Not Present");
				printf ("Pre-charge Circuit: %s\n", (word & 0x20) ? "Active" : "Not Active");
				printf ("Discharge: %s\n", (word & 0x10) ? "Allowed" : "Not Allowed");
				printf ("Charge: %s\n", (word & 0x08) ? "Allowed" : "Not Allowed");
				printf ("Charging: %s\n", (word & 0x04) ? "Yes" : "No");
				printf ("Discharging: %s\n", (word & 0x02) ? "Yes" : "No");
				printf ("Initialized: %s\n", (word & 0x01) ? "Yes" : "No");
				break;
			}

			case PMBUS_READ_FW_INFO:
				rdata[10] = '\0';
				if (rdata[1] || (phase != 0xff)) {
					printf ("FW Image: %s\n", ((phase == 0xA) || (phase == 0xff)) ? "A" :
						((phase == 0xB) ? "B" : "Bootloader"));
					printf ("FW Active: %s\n", (rdata[1]) ? "Yes" : "No");
					printf ("FW Revision: %s\n", &rdata[2]);
					active_fw = 0;
				}
				else {
					phase = 0xB;
					active_fw = 1;
				}
				break;

			default:
				printf ("Unknown request sent.\n");
				status = -1;
				break;
		}

		if (list != NULL) {
			cmd = list[++i];
			if (skip_battery && (strcmp ("battery", cmd) == 0)) {
				cmd = list[++i];
			}
		}
		else if (!active_fw) {
			cmd = "";
		}
	} while (strcmp (cmd, "") != 0);

	return status;
}

int ipmi_ocs_psu_action (struct ipmi_intf *intf, int argc, char *argv[])
{
	uint8_t phase;
	uint16_t wlen = 1;
	int status;
	struct _ipmi_master_phase_write_read xfer;

	status = ipmi_ocs_psu_init (argc, argv, &xfer, &phase);
	if (status != 0) {
		if (status == -2) {
			ocs_psu_action_usage ();
		}
		return -1;
	}

	xfer.rlen = 0;

	if (strcmp (argv[1], "clear") == 0) {
		xfer.wdata[0] = PMBUS_CLEAR_FAULT;
		xfer.phase = phase;
	}
	else if (strcmp (argv[1], "battery") == 0) {
		xfer.wdata[0] = PMBUS_BATT_HEALTH_TEST;
		xfer.wdata[1] = 0xcd;
		xfer.wdata[2] = 0xab;
		wlen = 3;
	}
	else {
		lprintf (LOG_ERR, "Error: Unknown action.");
		ocs_psu_action_usage ();
		return -1;
	}

	status = ipmi_ocs_master_phase_write_read (intf, &xfer, wlen, NULL);
	if (status != 0) {
		lprintf (LOG_ERR, "Error sending PSU action request: %s\n",
			val2str (status, ocs_completion_code));
		return -1;
	}

	return 0;
}

//
// Function to populate an I2C RW through MUX request
//
int ocs_master_mux_rw_parse_params(int argc, char** argv, IPMI_MASTER_MUX_RW* data, uint16_t* wlength)
{
	uint8_t* wdata;
	int  result = CC_UNSPECIFIED_ERROR;

	// Test inputs
	if(argc < 8 || data == NULL || argv == NULL)
	{
		lprintf(LOG_ERR, "Error: invalid parameters");
		ocs_master_mux_rw_usage();

		return -1;
	}

 	if(str2uchar(argv[1], &data->busid) != CC_SUCCESS)
 	{
 		lprintf(LOG_ERR, "Error: failed to convert Bus ID (%s) to uchar", argv[1]);

 		return -1;
 	}

 	if(str2uchar(argv[2], &data->mux1address) != CC_SUCCESS)
 	{
 		lprintf(LOG_ERR, "Error: failed to convert Mux 1 Address (%s) to uchar", argv[2]);

 		return -1;
 	}

 	if(str2uchar(argv[3], &data->mux1channel) != CC_SUCCESS)
 	{
 		lprintf(LOG_ERR, "Error: failed to convert Mux 1 Channel (%s) to uchar", argv[3]);

 		return -1;
 	}

 	if(str2uchar(argv[4], &data->mux2address) != CC_SUCCESS)
 	{
 		lprintf(LOG_ERR, "Error: failed to convert Mux 2 Address (%s) to uchar", argv[4]);

 		return -1;
 	}

 	if(str2uchar(argv[5], &data->mux2channel) != CC_SUCCESS)
 	{
 		lprintf(LOG_ERR, "Error: failed to convert Mux 2 Channel (%s) to uchar", argv[5]);

 		return -1;
 	}

 	if(str2uchar(argv[6], &data->deviceaddress) != CC_SUCCESS)
 	{
 		lprintf(LOG_ERR, "Error: failed to convert Device Address (%s) to uchar", argv[6]);

 		return -1;
 	}

 	if(str2uchar(argv[7], &data->readcount) != CC_SUCCESS)
 	{
 		lprintf(LOG_ERR, "Error: failed to convert readcount (%s) to uchar", argv[7]);

 		return -1;
 	}

 	*wlength = strlen(argv[8]);

 	if(*wlength > I2C_WRITE_MAX_LEN)
 	{
 		lprintf(LOG_ERR, "Error: write data buffer too large, maximum is %u", I2C_WRITE_MAX_LEN);

 		return -1;
 	}

 	wdata = hex2byte(argv[8], &result);

 	if (wdata == NULL || result != CC_SUCCESS)
 	{
 		lprintf(LOG_ERR, "Error: failed to convert write data to bytes");

 		return -1;
 	}

 	memcpy(data->wdata, wdata, *wlength);

 	return CC_SUCCESS;
}

//
// Function to send an I2C RW command through a MUX
//
int ocs_master_mux_rw(struct ipmi_intf *intf, IPMI_MASTER_MUX_RW* data, uint16_t wlength, uint8_t* rdata)
{
	struct ipmi_rq  req;
	struct ipmi_rs* rsp;

	memset(&req, 0, sizeof(req));

	// Test inputs
	if (intf == NULL || data == NULL || rdata == NULL)
	{
		lprintf(LOG_ERR, "Error: invalid parameters");

		return -1;
	}

	// Create request
	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_MASTER_MUX_WRITE_READ;
	req.msg.lun = 0;
	req.msg.data = (uint8_t*) data;
	req.msg.data_len = MASTER_MUX_RW_MIN_LEN + wlength;

	// Send request
	rsp = intf->sendrecv(intf, &req);

	// Check result
	if(rsp == NULL)
	{
		lprintf(LOG_ERR, "Error: (%s)\nCompletion Code: %d\n", val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);

		return -1;
	}
	else if(rsp->ccode != 0)
	{
		lprintf(LOG_ERR, "Error: (%s)\nCompletion Code: 0x%x\n", val2str(rsp->ccode, ocs_completion_code), rsp->ccode);

		return rsp->ccode;
	}
	else if(rsp->data != NULL && data->readcount > 0 && data->readcount <= rsp->data_len)
	{
		memcpy(rdata, rsp->data, data->readcount);
	}

	return CC_SUCCESS;
}

//
// Function to parse user input for an I2C RW command though a MUX
//
int ipmi_ocs_master_mux_rw(struct ipmi_intf *intf, int argc, char** argv) 
{
	uint16_t wlength = 0;
	uint8_t  result  = CC_UNSPECIFIED_ERROR;
	uint8_t rdata[MMRWREQ_MAX_READ_COUNT] = {0};

	IPMI_MASTER_MUX_RW data;

	// Test inputs
	if (intf == NULL || argv == NULL || argc < 9)
	{
		lprintf(LOG_ERR, "Error: invalid parameters");
		ocs_master_mux_rw_usage();

		return -1;
	}

	// Parse inputs
	result = ocs_master_mux_rw_parse_params(argc, argv, &data, &wlength);

	if(result != CC_SUCCESS)
	{
		lprintf(LOG_ERR, "Error: Failed to parse input parameters");

		return -1;
	}

	if(data.readcount > MMRWREQ_MAX_READ_COUNT)
	{
		lprintf(LOG_ERR, "Error: Requested readcount too large");

		return -1;
	}

	// Submit request	
	result = ocs_master_mux_rw(intf, &data, wlength, rdata);

	if(result != CC_SUCCESS)
	{
		lprintf(LOG_ERR, "Error: Failed to submit I2C request");

		return -1;
	}

	// Print read data
	printf("%u bytes read from device address 0x%X\n", data.readcount, data.deviceaddress);

	for(uint8_t i = 0; i < data.readcount; ++i)
	{
		printf("0x%X ", rdata[i]);
	}

	printf("\nCompletion Code: Success\n");

	return CC_SUCCESS;
}

//
// Function to process FPGA product info entries
// - Checksum is the modulo 256 sum of the product info area, so arithmetic overflow used instead of 
//   modulo operation
//
int16_t ipmi_process_prodinfo_entry(uint8_t* prodinfo, int16_t currindex, const char* label, uint8_t* checksum)
{
	int index;
	uint8_t sectiontype;
	uint16_t i = 0;
	uint16_t sectionlength;
	bool printoutput = true;

	if (prodinfo == NULL || label == NULL || checksum == NULL)
	{
		lprintf(LOG_ERR, "Error: Failed to process product info entry");

		return -1;
	}

	// No output if reserved section
	if(strncmp(label, ProductInfo_Labels[5], 8) == 0)
	{
		printoutput = false;
	}

	// Check if anymore fields remaining
	if (prodinfo[currindex] == PRODINFO_TERM_FIELD)
	{
		// Terminating field
		*checksum += prodinfo[currindex++];

		// Pad value
		*checksum += prodinfo[currindex++];

		// Checksum
		*checksum += prodinfo[currindex];

		return CC_SUCCESS;
	}

	// Print product info contents
	*checksum += prodinfo[currindex];
	sectionlength = prodinfo[currindex] & PRODINFO_LEN_MASK;
	sectiontype = (prodinfo[currindex] & PRODINFO_TYPE_MASK) >> PRODINFO_TYPE_SHIFT;

	++currindex;

	if(sectionlength > 0)
	{
		if(printoutput)
		{
			printf("%s: ", label);
		}

		for(; i < sectionlength; ++i, ++currindex)
		{
			*checksum += prodinfo[currindex];

			if(printoutput && strncmp(label, ProductInfo_Labels[7], 22) == 0)
			{
				index = (int)strtol((char*)&prodinfo[currindex], NULL, 16);

				if(index < NUM_PRODINFO_MFG)
				{
					printf("%s\n", ProductInfo_MfgCode[index]);
				}
				else
				{
					printf("%c\n", (char)prodinfo[currindex]);
				}

				printoutput = false;
			}
			else if(printoutput)
			{
				if(sectiontype == PRODINFO_SECTION_TYPE_ASCII)
				{
					printf("%c", (char)prodinfo[currindex]);
				}
				else
				{
					printf("%d", prodinfo[currindex]);
				}
			}
		}
	}

	if(printoutput)
	{
		printf("\n");
	}

	// Check if anymore fields remaining
	if (prodinfo[currindex] == PRODINFO_TERM_FIELD)
	{
		// Terminating field
		*checksum += prodinfo[currindex++];

		// Pad value
		*checksum += prodinfo[currindex++];

		// Checksum
		*checksum += prodinfo[currindex];

		return CC_SUCCESS;
	}

	return currindex;
}

//
// Function to process FPGA product info area
//
int ipmi_ocs_fpga_prodinfo(uint8_t* prodinfo, uint8_t formatversion, uint8_t length, uint8_t languagecode)
{
	uint16_t i = 0;
	int16_t result = 0;
	uint8_t checksum = 0;

	// Check if format version is supported
	if(formatversion != 0x01)
	{
		lprintf(LOG_ERR, "Error: Unsupported format version for FPGA product info");

		return -1;
	}

	// Check if language code is for English
	if(languagecode != 0x19)
	{
		lprintf(LOG_ERR, "Error: Unsupported language code for FPGA product info");

		return -1;
	}

	checksum += formatversion;
	checksum += length;
	checksum += languagecode;

	for (i = 0; i < NUM_PRODINFO_LABELS; ++i)
	{
		result = ipmi_process_prodinfo_entry(prodinfo, result, ProductInfo_Labels[i], &checksum);

		if(result == CC_SUCCESS && checksum == 0)
		{
			return CC_SUCCESS;
		}
		else if (result == CC_SUCCESS)
		{
			lprintf(LOG_ERR, "Error: Failed checksum check for FPGA product info");

			return -1;
		}
		else if(result == -1)
		{
			return -1;
		}
	}

	lprintf(LOG_NOTICE, "Warning: Terminating field not reached in FPGA product info");

	return CC_SUCCESS;
}

//
// Function to parse FPGA read commands
//
int ipmi_ocs_fpga_read(struct ipmi_intf *intf, int argc, char** argv)
{
	int result = CC_UNSPECIFIED_ERROR;
	uint8_t writecount = 1;
	uint8_t length;
	uint8_t prodoffset;
	uint8_t formatversion;
	uint8_t languagecode;	
	uint8_t rdata[MMRWREQ_MAX_READ_COUNT] = {0};

	IPMI_MASTER_MUX_RW data;
	IPMI_FPGA_HEALTH* health;
	IPMI_FPGA_FRU_COMMON_HEADER* commonheader;

	// Test inputs
	if (intf == NULL || argv == NULL || argc < 2)
	{
		lprintf(LOG_ERR, "Error: invalid parameters");
		ocs_fpga_read_usage();

		return -1;
	}

	// Setup I2C read request to FPGA
	data.busid = (MMRWREQ_PCIe_BUS_ID << MMRWREQ_BUS_ID_SHIFT) | MMRWREQ_PRIVATE_BUS_TYPE;
	data.mux1address = 0xE0;
	data.mux1channel = 0x05;
	data.mux2address = (0x7F << MMRWREQ_MUX_SLAVEADDRESS_SHIFT) | MMRWREQ_2NDLVL_MUX_NOT_REQ;
	data.mux2channel = 0x00;
	data.deviceaddress = 0xEE;
	data.readcount = 1;

	// Parse command
	if(strncmp(argv[1], "temp", 4) == 0)
	{
		data.wdata[0] = 0x01;
	}
	else if(strncmp(argv[1], "mode", 4) == 0)
	{
		data.wdata[0] = 0x02;
	}
	else if(strncmp(argv[1], "health", 6) == 0)
	{
		data.wdata[0] = 0x03;
	}
	else if(strncmp(argv[1], "i2cversion", 10) == 0)
	{
		data.wdata[0] = 0x04;
	}
	else if(strncmp(argv[1], "i2ctest", 7) == 0)
	{
		data.wdata[0] = 0x00;
	}
	else if(strncmp(argv[1], "assetinfo", 9) == 0)
	{
		data.wdata[0] = 0x00;
		data.wdata[1] = 0x00;
		data.deviceaddress = 0xA2;
		data.readcount = 8;
		writecount = 2;
	}
	else
	{
		lprintf(LOG_ERR, "Error: unknown FPGA read command (%s)", argv[1]);
		ocs_fpga_read_usage();

		return -1;
	}

	// Submit request	
	result = ocs_master_mux_rw(intf, &data, writecount, rdata);

	if(result != CC_SUCCESS)
	{
		lprintf(LOG_ERR, "Error: Failed to submit I2C request");

		return -1;
	}

	// Interpret read back data
	if (data.deviceaddress == 0xEE)
	{
		switch(data.wdata[0])
		{
			case 0x00:
			{
				printf("Zero Reg: 0x%X\n", rdata[0]);

				if(rdata[0] == 0x00)
				{
					printf("I2C test: Succeeded\n");
				}
				else
				{
					printf("I2C test: Failed\n");
				}

				break;
			}

			case 0x01:
			{
				printf("FPGA Temperature in Celsius: %u\n", rdata[0]);

				break;
			}

			case 0x02:
			{
				printf("Network Bypass Status and Control Byte: 0x%X\n", rdata[0]);

				if(rdata[0] & MMRWREQ_FORCE_BYPASS_MASK)
				{
					printf("Bypass Mode: On\n");
				}
				else
				{
					printf("Bypass Mode: Off\n");
				}

				if(rdata[0] & MMRWREQ_USER_LOGIC_MASK)
				{
					printf("User logic network: Enabled\n");
				}
				else
				{
					printf("User logic network: Disabled\n");
				}

				break;
			}

			case 0x03:
			{
				health = (IPMI_FPGA_HEALTH*) &rdata[0];

				printf("FPGA Health Status Byte: 0x%X\n", rdata[0]);
				printf("PCIe HIP 0 Up: %u\n", health->PCIe_HIP0_UP);
				printf("PCIe HIP 1 Up: %u\n", health->PCIe_HIP1_UP);
				printf("40G Link 0 Up: %u\n", health->LINK0_40G_UP);
				printf("40G Link 0 Tx Activity: %u\n", health->LINK0_40G_TX_ACT);
				printf("40G Link 0 Rx Activity: %u\n", health->LINK0_40G_RX_ACT);
				printf("40G Link 1 Up: %u\n", health->LINK1_40G_UP);
				printf("40G Link 1 Tx Activity: %u\n", health->LINK1_40G_TX_ACT);
				printf("40G Link 1 Rx Activity: %u\n", health->LINK1_40G_RX_ACT);

				break;
			}

			case 0x04:
			{
				printf("FPGA I2C Version: %u\n", rdata[0]);

				break;
			}

			default:
			{
				lprintf(LOG_ERR, "Error: unknown FPGA read command (%s)", data.wdata[0]);
				ocs_fpga_read_usage();

				return -1;
			}
		}
	}
	else if (data.deviceaddress == 0xA2)
	{
		// Create new request for board info using offset from common header
		commonheader = (IPMI_FPGA_FRU_COMMON_HEADER*) &rdata[0];
		prodoffset = (commonheader->productinfooffset) * 8;
		data.readcount = 3;
		data.wdata[0] = 0;
		data.wdata[1] = prodoffset;

		if(prodoffset == 0)
		{
			lprintf(LOG_ERR, "Error: Empty product info area");

			return -1;
		}

		// Submit request to read back product info size and language code
		result = ocs_master_mux_rw(intf, &data, 2, rdata);

		if(result != CC_SUCCESS)
		{
			lprintf(LOG_ERR, "Error: Failed to submit I2C request");

			return -1;
		}

		// Submit request to read back rest of product info area
		formatversion = rdata[0];
		length = rdata[1];
		languagecode = rdata[2];
		
		data.readcount = length * 8;
		data.wdata[0] = 0;
		data.wdata[1] = prodoffset + 3;

		if(length == 0)
		{
			lprintf(LOG_ERR, "Error: Empty product info area");

			return -1;
		}

		// Submit request to read back rest of product info area
		result = ocs_master_mux_rw(intf, &data, 2, rdata);

		if(result != CC_SUCCESS)
		{
			lprintf(LOG_ERR, "Error: Failed to submit I2C request");

			return -1;
		}
	
		// Process product info area
		result = ipmi_ocs_fpga_prodinfo(rdata, formatversion, length, languagecode);

		if(result != CC_SUCCESS)
		{
			lprintf(LOG_ERR, "Error: Failed to process FPGA prodinfo");

			return -1;
		}
	}

	printf("Completion Code: Success\n");

	return CC_SUCCESS;
}

//
// Function to parse FPGA action commands
//
int ipmi_ocs_fpga_action(struct ipmi_intf *intf, int argc, char** argv)
{
	IPMI_MASTER_MUX_RW data;
	int result = CC_UNSPECIFIED_ERROR;	
	uint8_t rdata[MMRWREQ_MAX_READ_COUNT] = {0};

	data.busid = (MMRWREQ_PCIe_BUS_ID << MMRWREQ_BUS_ID_SHIFT) | MMRWREQ_PRIVATE_BUS_TYPE;
	data.mux1address = 0xE0;
	data.mux1channel = 0x05;
	data.mux2address = (0x7F << MMRWREQ_MUX_SLAVEADDRESS_SHIFT) | MMRWREQ_2NDLVL_MUX_NOT_REQ;
	data.mux2channel = 0x00;
	data.deviceaddress = 0xEE;

	// Test inputs
	if (intf == NULL || argv == NULL || argc < 2)
	{
		lprintf(LOG_ERR, "Error: invalid parameters");
		ocs_fpga_action_usage();

		return -1;
	}

	// Parse command
	if(strncmp(argv[1], "setbypass", 9) == 0 || strncmp(argv[1], "clearbypass", 11) == 0)
	{
		data.readcount = 1;
		data.wdata[0] = 0x02;
	}
	else
	{
		lprintf(LOG_ERR, "Error: unknown FPGA action command (%s)", argv[1]);
		ocs_fpga_action_usage();

		return -1;
	}

	// Submit request	
	result = ocs_master_mux_rw(intf, &data, 1, rdata);

	if(result != CC_SUCCESS)
	{
		lprintf(LOG_ERR, "Error: Failed to submit I2C request");

		return -1;
	}

	// Submit action request if previous was a read
	if (data.readcount > 0)
	{
		if(strncmp(argv[1], "setbypass", 9) == 0)
		{
			data.readcount = 0;
			data.wdata[1] = rdata[0] | MMRWREQ_FORCE_BYPASS_MASK;
		}
		else if(strncmp(argv[1], "clearbypass", 11) == 0)
		{
			data.readcount = 0;
			data.wdata[1] = rdata[0] & ~(MMRWREQ_FORCE_BYPASS_MASK);
		}
		else
		{
			lprintf(LOG_ERR, "Error: unimplemented FPGA action command (%s)", argv[1]);

			return -1;
		}

		result = ocs_master_mux_rw(intf, &data, 2, rdata);

		if(result != CC_SUCCESS)
		{
			lprintf(LOG_ERR, "Error: Failed to submit I2C request");

			return -1;
		}

		printf("Completion Code: Success\n");
	}

	return CC_SUCCESS;
}

static int storage_device_status(struct ipmi_intf *intf, uint8_t device_addr,
	uint8_t mux_addr, uint8_t mux_channel, uint8_t mux2_addr, uint8_t mux2_channel,
	struct _nvme_storage_status *status) {

	IPMI_MASTER_MUX_RW data;
	int result = CC_UNSPECIFIED_ERROR;
	uint8_t response[MMRWREQ_MAX_READ_COUNT];

	data.busid = (MMRWREQ_PCIe_BUS_ID << MMRWREQ_BUS_ID_SHIFT) | MMRWREQ_PRIVATE_BUS_TYPE;
	data.mux1address = mux_addr;
	data.mux1channel = mux_channel;
	/* No second level mux: [7:1] = 7Fh [0]=1h */
	data.mux2address = (mux2_addr > 0) ? mux2_addr : ((MMRWREQ_PCIe_BUS_ID << MMRWREQ_BUS_ID_SHIFT) | MMRWREQ_PRIVATE_BUS_TYPE);
	data.mux2channel = mux2_channel;
	data.deviceaddress = device_addr;
	data.readcount = 5;
	memset(&data.wdata, 0, sizeof(data.wdata));

	result = ocs_master_mux_rw(intf, &data, 1, response);
	if (result != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error: Failed I2C request");
		return -1;
	}

	status->length = response[0];
	status->status = response[1];
	status->smart_wrn = response[2];
	status->temp = response[3];
	status->pdlu = response[4];

	memset(&response, 0, sizeof(response));

	data.readcount = 23;
	data.wdata[0] = 8;

	result = ocs_master_mux_rw(intf, &data, 1, response);
	if (result != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error: Failed I2C request");
		return -1;
	}

	memcpy(&response[1], &status->vendor, sizeof(uint16_t));
	memcpy(&response[3], status->serial, 20);


	return CC_SUCCESS;
}

static void print_storage_status(const char *port_type, const uint8_t port_id, const uint8_t id, const uint8_t presence,
	struct _nvme_storage_status *drv_state) {
	if (presence == 1) {
		printf("Port Type: %s\nSlot: %d\nDevice: %d\nVendor: %d\nTemp: %d\n", port_type, port_id, id,
			drv_state->vendor, drv_state->temp);
		printf("Drive Status:\nDrive Presence: True\n");
		printf("SMBUS Arbitration: %s\n", ((drv_state->status & 0x80) == 0x80 ? "True" : "False"));
		printf("Drive Not Read:    %s\n", ((drv_state->status & 0x40) == 0x40 ? "True" : "False"));
		printf("Drive Functional:  %s\n", ((drv_state->status & 0x20) == 0x20 ? "True" : "False"));
		printf("No Reset Required: %s\n", ((drv_state->status & 0x10) == 0x10 ? "True" : "False"));
		printf("Port2 Link Active: %s\n", ((drv_state->status & 0x08) == 0x08 ? "True" : "False"));
		printf("Port1 Link Active: %s\n", ((drv_state->status & 0x04) == 0x04 ? "True" : "False"));
		printf("SMART Warn: %02X\n", drv_state->smart_wrn);
		printf("Serial No: %s\n", drv_state->serial);
		printf("\n");
	}
	else {
		printf("Port Type: %s\nSlot: %d\nDevice: %d\nVendor: -\nTemp: -\n", port_type, port_id, id);
		printf("Drive Status:\nDrive Presence: False\n");
		printf("\n");
	}
}

/*
* Get Storage Device Map for i2c status
*/
int get_nvme_status(struct ipmi_intf *intf) {

	struct ipmi_rq  req;
	struct ipmi_rs* rsp;
	char storage_map[5];

	memset(&req, 0, sizeof(req));
	memset(&storage_map, 0, sizeof(storage_map));

	// Create request
	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_GET_STORAGE_MAP;
	req.msg.lun = 0;
	req.msg.data = NULL;
	req.msg.data_len = 0;

	// Send request
	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: (%s)\nCompletion Code: %d\n",
			val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code),
			CC_SEND_RECEIVE_FAILED);
		return CC_SEND_RECEIVE_FAILED;
	}
	else if (rsp->ccode != 0) {
		lprintf(LOG_ERR, "Error: (%s)\nCompletion Code: %d\n",
			val2str(rsp->ccode, completion_code_vals),
			rsp->ccode);
		return rsp->ccode;
	}
	else if (rsp->data != NULL && rsp->data_len >= (int)sizeof(storage_map)) {
		memcpy(storage_map, rsp->data, sizeof(storage_map));
	}

	uint8_t slot = 0;
	uint8_t bit_flag = 1;
	uint8_t presence = 0;
	char *pcie_port = "PCIe\0";
	char *m2_port = "M.2\0";

	NVME_STORAGE_STATUS drive_status;
	NVME_STORAGE_STATUS *prt_status = &drive_status;
	memset(&drive_status, 0, sizeof(NVME_STORAGE_STATUS));
	
	for (; slot < sizeof(storage_map); ++slot) {

		switch (slot)
		{
			// Onboard M.2 Slots
			case 0x00:
			{
				for (uint8_t i = 0; i < 4; ++i) {
					presence = (storage_map[slot] & (bit_flag << i)) == (bit_flag << i) ? 1 : 0;

					if (presence == 1) {
						if (storage_device_status(intf, M2_SSD_DEVICE_ADDR, M2_MUX_ADDR, i, 0, 0, prt_status) != CC_SUCCESS) {
							printf("Port Type: %s\nSlot: %d\nDevice: %d\nVendor: -\nTemp: -\n", m2_port, i + 1, i);
							printf("Drive Status:\nDrive Presence: Unknown\n");
							printf("Error gathering data\n");
							printf("\n");
							continue;
						}
					}

					/* prints if present or not */
					print_storage_status(m2_port, i + 1, i, presence, prt_status);

					memset(prt_status, 0, sizeof(NVME_STORAGE_STATUS));
				}
				break;
			}
			// PCIe Slots 1 & 2
			case 0x01:
			{
				for (uint8_t i = 0; i < 4; ++i) {
					presence = (storage_map[slot] & (bit_flag << ((i >= 2) ? (i + 2) : i))) == (bit_flag << ((i >= 2) ? (i + 2) : i)) ? 1 : 0;

					if (presence == 1) {
						if (storage_device_status(intf, M2_SSD_DEVICE_ADDR, PCIE_MUX_1_ADDR, i, 0, 0, prt_status) != CC_SUCCESS) {
							printf("Port Type: %s\nSlot: %d\nDevice: %d\nVendor: -\nTemp: -\n", m2_port, i + 1, i);
							printf("Drive Status:\nDrive Presence: Unknown\n");
							printf("Error gathering data\n");
							printf("\n");
							continue;
						}
					}

					/* prints if present or not */
					print_storage_status(pcie_port, (i < 2) ? 1 : 2, i, presence, prt_status);

					memset(prt_status, 0, sizeof(NVME_STORAGE_STATUS));

				}
				break;
			}
			// PCIe Slots 3,4,5
			case 0x02:
			case 0x03:
			case 0x04:
			{
				for (uint8_t i = 0; i < 8; i++) {
					presence = (storage_map[slot] & (bit_flag << i)) == (bit_flag << i) ? 1 : 0;

					if (presence == 1) {
						if (storage_device_status(intf, M2_SSD_DEVICE_ADDR, PCIE_MUX_1_ADDR, (slot + 2), PCIE_MUX_2_ADDR, i, prt_status) != CC_SUCCESS) {
							printf("Port Type: %s\nSlot: %d\nDevice: %d\nVendor: -\nTemp: -\n", m2_port, i + 1, i);
							printf("Drive Status:\nDrive Presence: Unknown\n");
							printf("Error gathering data\n");
							printf("\n");
							continue;
						}
					}

					/* prints if present or not */
					print_storage_status(pcie_port, slot + 1, i, presence, prt_status);
					memset(prt_status, 0, sizeof(NVME_STORAGE_STATUS));
				}
				break;
			}

		}

	}

	return CC_SUCCESS;
}

/* 
 * Private method: Enable RMedia Service 
*/
int ipmi_enable_rm_service(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	memset(&req, 0, sizeof(req));

	uint8_t check_rm[2] = { 0x08, 0x0A };
	uint8_t enable_rm[4] = { 0x08, 0x0A, 0x00, 0x01 };

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = GET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &check_rm;
	req.msg.data_len = sizeof(check_rm);

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error checking remote media\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error checking remote media: %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	if (rsp->data[0] == 0x00) {
		memset(&req, 0, sizeof(req));

		req.msg.netfn = OEM_GP5_NETFN;
		req.msg.cmd = SET_RMEDIA_SERVICE;
		req.msg.lun = 0;
		req.msg.data = &enable_rm;
		req.msg.data_len = sizeof(enable_rm);

		rsp = intf->sendrecv(intf, &req);

		if (rsp == NULL) {
			lprintf(LOG_ERR, "Error enabling remote media\n");
			return -1;
		}

		if (rsp->ccode != CC_SUCCESS) {
			lprintf(LOG_ERR, "Error enabling remote media: %s\n",
				val2str(rsp->ccode, ocs_completion_code));
			return -1;
		}
	}

	return CC_SUCCESS;
}

/* 
 * Private method: Disable RMedia Service 
*/
int ipmi_disable_rm_service(struct ipmi_intf *intf) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	memset(&req, 0, sizeof(req));

	uint8_t check_rm[2] = { 0x08, 0x0A };
	uint8_t disable_rm[4] = { 0x08, 0x0A, 0x00, 0x00 };

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = GET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &check_rm;
	req.msg.data_len = sizeof(check_rm);

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error checking remote media\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error checking remote media: %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	if (rsp->data[0] == 0x01) {
		memset(&req, 0, sizeof(req));

		req.msg.netfn = OEM_GP5_NETFN;
		req.msg.cmd = SET_RMEDIA_SERVICE;
		req.msg.lun = 0;
		req.msg.data = &disable_rm;
		req.msg.data_len = sizeof(disable_rm);

		rsp = intf->sendrecv(intf, &req);

		if (rsp == NULL) {
			lprintf(LOG_ERR, "Error disabling remote media\n");
			return -1;
		}

		if (rsp->ccode != CC_SUCCESS) {
			lprintf(LOG_ERR, "Error disabling remote media: %s\n",
				val2str(rsp->ccode, ocs_completion_code));
			return -1;
		}
	}

	return CC_SUCCESS;
}

/*
 * Mount Remote media via NFS path
 * Param1: IP address
 * Param2: Image name
 * Note: Image path is hard coded
 */
int mount_rm_media(struct ipmi_intf *intf, int argc, char** argv) {

	if (argc != 4) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.\n");
		mount_remote_media_usage();
		return -1;
	}

	int addr_len = strlen(argv[1]);
	if (addr_len < 8 || addr_len > 16) {
		lprintf(LOG_ERR, "Error: invalid ip address for parameter: %s \n ", argv[1]);
		return -1;
	}

	int img_len = strlen(argv[2]);
	if (img_len < 1 || img_len > 40) {
		lprintf(LOG_ERR, "Error: invalid image name, must be greater than 1 and less than 40 characters: %s \n ", argv[2]);
		return -1;
	}

	int img_path_len = strlen(argv[3]);
	if (img_len < 1 || img_len > 64) {
		lprintf(LOG_ERR, "Error: invalid image path, must be greater than 1 and less than 64 characters: %s \n ", argv[3]);
		return -1;
	}
	
	uint8_t data_buffer[60];
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	memset(&req, 0, sizeof(req));
	
	/* 1. Enable RMedia Service */
	if (ipmi_enable_rm_service(intf) != CC_SUCCESS) {
		return -1;
	}

	/* 2. Send server (RM) ip address */
	memset(&data_buffer, 0, sizeof(data_buffer));
	
	data_buffer[0] = 0x01;
	data_buffer[1] = 0x02;
	data_buffer[2] = 0x00;
	memcpy(&data_buffer[3], argv[1], addr_len);

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = SET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = (3 + addr_len);

	rsp = intf->sendrecv(intf, &req);

	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error on interface sending: Remote Media Address\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error setting remote media ip address: %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}
	
	/* 3. Set NFS Share Type */
	memset(&data_buffer, 0, sizeof(data_buffer));
	
	data_buffer[0] = 0x01;
	data_buffer[1] = 0x05; 
	data_buffer[2] = 0x00; 
	data_buffer[3] = 0x6e; 
	data_buffer[4] = 0x66; 
	data_buffer[5] = 0x73; 
	data_buffer[6] = 0x00;
	
	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = SET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = 7;

	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error setting NFS share type (response null) : Remote Media\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error setting NFS share type : %s\n", val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	/* 4. Set progress bit (before setting image path) */
	memset(&data_buffer, 0, sizeof(data_buffer));
	
	data_buffer[0] = 0x01;
	data_buffer[1] = 0x01;
	data_buffer[2] = 0x00;
	data_buffer[3] = 0x01;

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = SET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = 4;

	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error on interface setting progress bit (before setting image path): Remote Media\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error setting progress bit (before setting image path): %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	/* 5. Set server image path */
	memset(&data_buffer, 0, sizeof(data_buffer));
	
	data_buffer[0] = 0x01;
	data_buffer[1] = 0x01;
	data_buffer[2] = 0x01;
	memcpy(&data_buffer[3], argv[3], img_path_len);

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = SET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = (3 + img_path_len);
	
	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error on interface setting image path: Remote Media\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error setting image path: %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	/* 6. Save Nfs Configuration (after setting image path) */
	memset(&data_buffer, 0, sizeof(data_buffer));
	
	data_buffer[0] = 0x01;
	data_buffer[1] = 0x01;
	data_buffer[2] = 0x00;
	data_buffer[3] = 0x00;

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = SET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = 4;

	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error saving nfs configuration (after setting image path): Remote Media\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error saving nfs configuration (after setting image path): %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	/* 7. Set progress bit (before setting image name) */
	memset(&data_buffer, 0, sizeof(data_buffer));
	
	data_buffer[0] = 0x01;
	data_buffer[1] = 0x00;
	data_buffer[2] = 0x00;
	data_buffer[3] = 0x01;

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = SET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = 4;

	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error on interface setting progress bit (before setting image name): Remote Media\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error setting progress bit (before setting image name): %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	/* 8. Set mount image name */
	memset(&data_buffer, 0, sizeof(data_buffer));

	data_buffer[0] = 0x01;
	data_buffer[1] = 0x00;
	data_buffer[2] = 0x01;
	memcpy(&data_buffer[3], argv[2], img_len);

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = SET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = 3 + img_len;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error setting image name: %s\n", argv[2]);
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error setting image name: %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	/* 9. Save Nfs Configuration (after setting image name) */
	memset(&data_buffer, 0, sizeof(data_buffer));
	
	data_buffer[0] = 0x01;
	data_buffer[1] = 0x00;
	data_buffer[2] = 0x00;
	data_buffer[3] = 0x00;

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = SET_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = 4;

	rsp = intf->sendrecv(intf, &req);
	
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error saving nfs configuration (after setting image name): Remote Media\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error saving nfs configuration (after setting image name): %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	/* 10. Start rmedia redirection (Mount) */
	memset(&data_buffer, 0, sizeof(data_buffer));
	
	data_buffer[0] = 0x01;
	data_buffer[1] = 0x01;

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = MOUNT_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = 2;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error mounting media: Remote Media\n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error mounting rmedia: %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	return CC_SUCCESS;
}

int unmount_rm_media(struct ipmi_intf *intf) {

	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	memset(&req, 0, sizeof(req));

	uint8_t data_buffer[60];
	memset(&data_buffer, 0, sizeof(data_buffer));

	/* unmount rmedia */
	data_buffer[0] = 0x01;
	data_buffer[1] = 0x00;
	
	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = MOUNT_RMEDIA_SERVICE;
	req.msg.lun = 0;
	req.msg.data = &data_buffer;
	req.msg.data_len = 2;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error on interface unmount (response is null) \n");
		return -1;
	}

	if (rsp->ccode != CC_SUCCESS) {
		lprintf(LOG_ERR, "Error unmounting media: %s\n",
			val2str(rsp->ccode, ocs_completion_code));
		return -1;
	}

	/* Stop rmedia service */
	if (ipmi_disable_rm_service(intf) != CC_SUCCESS)
		return -1;
	else
		return CC_SUCCESS;
}

int is_session_present(const uint8_t* active_session) {
	for (int i = 0; i < sizeof(active_session); i++) {
		if (active_session[i] || 0 != 0) {
			return 1;
		}
	}
	return 0;
}

int ipmi_ocs_get_all_active_session(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;
	struct ipmi_active_sessions active_sessions;

	if (argc != 2) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.\n");
		active_sessions_usage();
		return -1;
	}
	uint8_t sessiontype;

	if (str2uchar(argv[1], &sessiontype) != 0) {
			lprintf(LOG_ERR, "Error: Failed to convert sessiontype to byte : %s \n", argv[1]);
			return -1;
	}
	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = CMD_GET_ACTIVE_SESSIONS;
	req.msg.lun = 0;
	req.msg.data = &sessiontype;
	req.msg.data_len = 1;

	rsp = intf->sendrecv(intf, &req);
	if (rsp == NULL) {
		lprintf(LOG_ERR, "Error: (%s) Completion Code: %d\n",
				val2str(CC_SEND_RECEIVE_FAILED, ocs_completion_code), CC_SEND_RECEIVE_FAILED);
		return (-1);
	} else if (rsp->ccode > 0) {
		lprintf(LOG_ERR, "Error: (%s) Completion Code: %d\n",
				val2str(rsp->ccode, completion_code_vals), rsp->ccode);
		return (-1);
	} else {
		if (rsp->data_len > 1) {
			memcpy(&active_sessions, rsp->data, sizeof(struct ipmi_active_sessions));

			printf("Number of active sessions present: %d\n", active_sessions.present);
			printf("Available sessionIds:\n");

			if(is_session_present(active_sessions.Firstrecord_sessionid)) {
				printf("First record session ID:");
				for (int i = 0; i < sizeof(active_sessions.Firstrecord_sessionid); i++) {
					printf("%u%s", active_sessions.Firstrecord_sessionid[i], "-");
				}
				printf("\n");
			}

			if (is_session_present(active_sessions.Secondrecord_sessionid)) {
				printf("Second record session ID:");
				for (int i = 0; i < sizeof(active_sessions.Secondrecord_sessionid); i++) {
					printf("%u%s", active_sessions.Secondrecord_sessionid[i], "-");
				}
				printf("\n");
			} else {
				return 0;
			}

			int con_sessions = strlen(active_sessions.consecutive_session);
			if (con_sessions <= 0){
				return 0;
			}
			else if(con_sessions > 0 && con_sessions < 4)
			{
				printf("Consecutive session ID:");
				for (int i = 0; i <= con_sessions; i++){
					printf("%u%s", active_sessions.consecutive_session[i], "-");
				}
				printf("\n");
			}
			else {
				printf("consecutive session ids: \n");
				for (int i = 0 ; i < con_sessions ; i+=4) {
					if(con_sessions-i >= 4) {
						uint8_t session[4];
						memcpy(session, active_sessions.consecutive_session, 4);
						if (is_session_present(session)){
							for (int i = 0; i < 4; i++){
								printf("%u%s",session[i],"-");
							}
							printf("\n");
						}
						else {
							return 0;
						}
					}
				}
			}
		}
		else {
			printf("No active sessions present\n");
		}
	}
	return 0;
}

int close_active_session(struct ipmi_intf *intf, int argc, char** argv) {
	struct ipmi_rs *rsp;
	struct ipmi_rq req;

	if (argc != 5) {
		lprintf(LOG_ERR, "Error: Not enough parameters given.\n");
		close_active_sessions_usage();
		return -1;
	}

	uint8_t firstbyte;
	uint8_t secondbyte;
	uint8_t thirdbyte;
	uint8_t fourthbyte;

	if (str2uchar(argv[1], &firstbyte) != 0) {
				lprintf(LOG_ERR, "Error: Failed to convert first argument to byte : %s \n", argv[1]);
				return -1;
	}
	if (str2uchar(argv[2], &secondbyte) != 0) {
				lprintf(LOG_ERR, "Error: Failed to convert second argument to byte : %s \n", argv[2]);
				return -1;
	}
	if (str2uchar(argv[3], &thirdbyte) != 0) {
				lprintf(LOG_ERR, "Error: Failed to convert third argument to byte : %s \n", argv[3]);
				return -1;
	}
	if (str2uchar(argv[4], &fourthbyte) != 0) {
				lprintf(LOG_ERR, "Error: Failed to convert fourth argument to byte : %s \n", argv[4]);
				return -1;
	}

	uint8_t sessionid[4];
	sessionid[0] = firstbyte;
	sessionid[1] = secondbyte;
	sessionid[2] = thirdbyte;
	sessionid[3] = fourthbyte;

	req.msg.netfn = OEM_GP5_NETFN;
	req.msg.cmd = CMD_CLOSE_ACTIVE_SESSION;
	req.msg.lun = 0;
	req.msg.data = (uint8_t*) &sessionid;
	req.msg.data_len = 4;
	rsp = intf->sendrecv(intf, &req);

	return sethelpermethod(rsp);
}

int set_service_configuration (struct ipmi_intf *intf, int argc, char **argv)
{
	struct ipmi_service_config service;
	struct ipmi_rq req;
	struct ipmi_rs *resp;

	if (argc != 3) {
		lprintf (LOG_ERR, "Error: Not enough parameters given.\n");
		set_service_config_usage ();
		return -1;
	}

	if (strcmp (argv[1], "cpudbg") == 0) {
		service.type = SERVICE_TYPE_CPU_DEBUG;
	}
	else if (strcmp (argv[1], "fpgadbg") == 0) {
		service.type = SERVICE_TYPE_FPGA_DEBUG;
	}
	else {
		lprintf (LOG_ERR, "Error: Unknown service type %s.\n", argv[1]);
		set_service_config_usage ();
		return -1;
	}

	if (strcmp (argv[2], "start") == 0) {
		service.config = SERVICE_CONFIG_START;
	}
	else if (strcmp (argv[2], "stop") == 0) {
		service.config = SERVICE_CONFIG_STOP;
	}
	else {
		lprintf (LOG_ERR, "Error: Unknown service configuration %s.\n", argv[2]);
		set_service_config_usage ();
		return -1;
	}

	req.msg.netfn = OEM_GP4_NETFN;
	req.msg.cmd = CMD_SET_SERVICE_CONFIG;
	req.msg.lun = 0;
	req.msg.data = (uint8_t*) &service;
	req.msg.data_len = sizeof (service);

	resp = intf->sendrecv (intf, &req);
	if (resp == NULL) {
		lprintf (LOG_ERR, "Error sending Set Service Configuration command");
		return -1;
	}
	if (resp->ccode != 0) {
		if (resp->ccode == 0x81) {
			lprintf (LOG_ERR, "Remote Debug Protocol Not Ready");
			return -1;
		}
		else if (resp->ccode == 0x82) {
			lprintf (LOG_ERR, "JTAG Bus Busy");
			return -1;
		}
		else {
			lprintf (LOG_ERR, "Error sending Set Service Configuration command: %s",
				val2str (resp->ccode, ocs_completion_code));
			return -1;
		}
	}

	printf ("Completion Status: Success\n");
	return 0;
}
