/*
 * ipmi_ocsoem.h
 *
 *  Created on: Apr 14, 2016
 *      Author: admin_user
 */

#ifndef IPMI_OCSOEM_H_
#define IPMI_OCSOEM_H_

// net functions
#define OEM_GP1_NETFN								(uint8_t)(0x2E)
#define OEM_GP2_NETFN								(uint8_t)(0x30)
#define OEM_GP3_NETFN								(uint8_t)(0x32)
#define OEM_GP4_NETFN								(uint8_t)(0x38)
#define OEM_GP5_NETFN								(uint8_t)(0x34)

// commands
#define CMD_GET_NIC_INFO		    				(uint8_t)(0x19)
#define CMD_SET_ENERGY_STORAGE						(uint8_t)(0x22)
#define CMD_GET_ENERGY_STORAGE						(uint8_t)(0x23)
#define CMD_ACTIVATE_PSU_ALERT 						(uint8_t)(0x24)
#define CMD_SET_DEFAULT_POWER_LIMIT					(uint8_t)(0x25)
#define CMD_GET_DEFAULT_POWER_LIMIT					(uint8_t)(0x26)
#define CMD_SET_PSU_ALERT		    				(uint8_t)(0x27)
#define CMD_GET_PSU_ALERT		    				(uint8_t)(0x28)
#define CMD_GET_BIOS_CODE							(uint8_t)(0x73)
#define CMD_SET_BIOS_CONFIG							(uint8_t)(0x74)
#define CMD_GET_BIOS_CONFIG							(uint8_t)(0x75)
#define CMD_SET_JTAG_PROGRAM_STATUS					(uint8_t)(0x80)
#define CMD_GET_JTAG_PROGRAM_STATUS					(uint8_t)(0x81)
#define CMD_GET_TPM_PHYSICAL_PRESENCE 				(uint8_t)(0x82)
#define CMD_SET_TPM_PHYSICAL_PRESENCE 				(uint8_t)(0x83)
#define CMD_GET_PCIE_INFO		    				(uint8_t)(0x1A)
#define	CMD_MASTER_WRITE_READ						(uint8_t)(0x52)
#define CMD_MASTER_MUX_WRITE_READ  					(uint8_t)(0x53)
#define CMD_MASTER_PHASE_WRITE_READ					(uint8_t)(0x54)
#define CMD_GET_FIRMWARE_VERSION					(uint8_t)(0x0B)
#define CMD_GET_MEMORY_INFO							(uint8_t)(0x1D)
#define CMD_GET_GPIO								(uint8_t)(0xE1)
#define CMD_SET_GPIO								(uint8_t)(0xE0)
#define CMD_GET_NV_DIMM_TRIGGER						(uint8_t)(0x2A)
#define CMD_SET_NV_DIMM_TRIGGER						(uint8_t)(0x29)
#define CMD_GET_PROCESSOR_INFO						(uint8_t)(0x1B)
#define CMD_BMC_DEBUG			    				(uint8_t)(0xF9)
#define CMD_GET_THERMAL_CONTROL	    				(uint8_t)(0x2C)
#define CMD_GET_GUID			    				(uint8_t)(0x37)
#define	CMD_CONFIG_FW_UPDATE						(uint8_t)(0x84)
#define CMD_GET_CHANNEL_AUTH_CAP    				(uint8_t)(0x38)
#define CMD_GET_STORAGE_MAP							(uint8_t)(0x86)
#define CMD_GET_ACTIVE_SESSIONS						(uint8_t)(0xB0)
#define CMD_CLOSE_ACTIVE_SESSION					(uint8_t)(0xB1)
#define CMD_SET_SERVICE_CONFIG						(uint8_t)(0x87)

#define SET_RMEDIA_SERVICE							(uint8_t)(0x9F)
#define GET_RMEDIA_SERVICE							(uint8_t)(0x9E)
#define MOUNT_RMEDIA_SERVICE						(uint8_t)(0xA0)

// Master Mux RW Request Register Masks, Shifts, Values
#define MMRWREQ_PCIe_BUS_ID							(uint8_t)(0x01)
#define MMRWREQ_BUS_ID_SHIFT						(uint8_t)(1)
#define MMRWREQ_PRIVATE_BUS_TYPE					(uint8_t)(0x01)
#define MMRWREQ_MUX_SLAVEADDRESS_SHIFT				(uint8_t)(1)
#define MMRWREQ_2NDLVL_MUX_NOT_REQ					(uint8_t)(0x01)
#define MMRWREQ_FORCE_BYPASS_SHIFT					(0)
#define MMRWREQ_FORCE_BYPASS_MASK					(uint8_t)(1 << MMRWREQ_FORCE_BYPASS_SHIFT)
#define MMRWREQ_USER_LOGIC_SHIFT					(1)
#define MMRWREQ_USER_LOGIC_MASK						(uint8_t)(1 << MMRWREQ_USER_LOGIC_SHIFT)
#define MMRWREQ_MAX_READ_COUNT						32

// Channel Authentication Capabilities
#define CHAUTH_SERVER_TYPE_MASK						(uint8_t)(0xFC)
#define CHAUTH_SERVER_TYPE_SHIFT					(2)

// FPGA FRU Product Info Area Register Masks, Shifts, Values
#define PRODINFO_TYPE_MASK							(uint8_t)(0xC0)
#define PRODINFO_TYPE_SHIFT							(6)
#define PRODINFO_LEN_MASK   						(uint8_t)(0x1F)
#define PRODINFO_SECTION_TYPE_ASCII  				(3)
#define PRODINFO_TERM_FIELD							(0xC1)

// M.2 SSD Register Masks, Shifts, Values
#define M2_SSD_DEVICE_ADDR							(uint8_t)(0xD4)
#define M2_MUX_ADDR									(uint8_t)(0xE2)
#define PCIE_MUX_1_ADDR								(uint8_t)(0xE0)
#define PCIE_MUX_2_ADDR								(uint8_t)(0xE6)

// completion codes
#define CC_SUCCESS									(uint8_t)(0x00)
#define CC_NODE_BUSY								(uint8_t)(0xc0)
#define CC_INVALID_COMMAND							(uint8_t)(0xc1)
#define CC_PARAMETER_OUTOF_RANGE					(uint8_t)(0xc9)
#define CC_INVALID_DATA_FIELD_IN_REQUEST			(uint8_t)(0xcc)
#define CC_NO_RESPONSE								(uint8_t)(0xd0)
#define CC_BMC_INIT									(uint8_t)(0xd1)
#define CC_DESTINATION_UNAVAIL						(uint8_t)(0xd2)
#define CC_FRU_VER_NO_SUPPORT						(uint8_t)(0xd3)
#define CC_INVALID_RESPONSE_LEN						(uint8_t)(0xd4)
#define CC_COMMAND_NOT_SUPPORTED					(uint8_t)(0xd5)
#define CC_SEND_RECEIVE_FAILED						(uint8_t)(0xd6)
#define CC_UNSPECIFIED_ERROR						(uint8_t)(0xff)


#pragma pack(push, 1)

typedef struct _ipmi_default_powerlimit
{
	uint16_t    dpc;
	uint16_t	delay;
	uint8_t	   	action;
	uint8_t	   	autoremove;
} __attribute__ ((packed)) IPMI_DEFAULT_POWERLIMIT;

typedef struct _ipmi_set_default_powerlimit
{
	uint16_t    dpc;
	uint16_t	delay;
	uint8_t	   	autoremoveDelayInMilliSec;
} __attribute__ ((packed)) IPMI_SET_DEFAULT_POWERLIMIT;


typedef struct _ipmi_bios_config
{
    uint8_t     currentconfg;
	uint8_t		chosenconfig;
	uint8_t		availconfig[237];
} __attribute__ ((packed)) IPMI_BIOS_CONFIG;

typedef struct _ipmi_energy_storage
{
	uint16_t    presence:2;
	uint16_t	state:3;
	uint16_t	reserved:11;
	uint8_t		scalefactor;
	uint16_t	systemenergy;
	uint8_t		nvdimmenergy;
	uint16_t    rollingcounter;
} __attribute__ ((packed)) IPMI_ENERGY_STORAGE;

typedef struct _ipmi_set_energy_storage
{
	uint8_t     presence_state;
	uint8_t     reserved;
	uint8_t		scalefactor;
	uint16_t	systemenergy;
	uint8_t		nvdimmenergy;
	uint16_t    rollingcounter;
} __attribute__ ((packed)) IPMI_SET_ENERGY_STORAGE;

typedef struct _ipmi_nic_info
{
	uint8_t macaddress;

} __attribute__ ((packed)) IPMI_NIC_INFO;

typedef struct _ipmi_gpio
{
	uint8_t	   	pindirection;
	uint8_t	   	pinstate;
} __attribute__ ((packed)) IPMI_GPIO;

typedef struct _ipmi_set_gpio
{
	uint8_t     pinnumber;
	uint8_t	   	pindirection;
	uint8_t	   	pinstate;
} __attribute__ ((packed)) IPMI_SET_GPIO;

typedef struct _ipmi_jtag_program_status
{
	uint8_t	   	programmingstatus;
} __attribute__ ((packed)) IPMI_JTAG_PROGRAM_STATUS;

typedef struct _ipmi_nv_dimm_trigger
{
	uint8_t    	adrTrigger;
	uint8_t		adrCompletedelay;
	uint8_t		nvDimmPresentPowerOffDelay;
	uint8_t		adrComplete;
	uint8_t		adrCompleteTimeRemaining;
	uint8_t		nvDimmPresentTimeRemaining;
} __attribute__ ((packed)) IPMI_NV_DIMM_TRIGGER;

typedef struct _ipmi_processor_info
{
	uint8_t    	type;
	uint16_t	frequency;
	uint8_t		state;
} __attribute__ ((packed)) IPMI_PROCESSOR_INFO;

typedef struct _ipmi_psu_alert
{
	uint8_t    	alertstatus;
} __attribute__ ((packed)) IPMI_PSU_ALERT;

typedef struct _ipmi_pcie_info_response_map
{
	uint8_t    	slot8to1;
	uint8_t    	slot16to9;
	uint8_t    	slot21to17:4;
} __attribute__ ((packed)) IPMI_PCIE_INFO_RESPONSE_MAP;

typedef struct _ipmi_pcie_info_response
{
	uint16_t    	vendorId;
	uint16_t    	deviceId;
	uint16_t    	subSystemId;
	uint16_t    	subSystemVendorId;
} __attribute__ ((packed)) IPMI_PCIE_INFO_RESPONSE;

typedef struct _ipmi_receive_serial_data
{
	uint16_t    payLoadLength;
	uint8_t    	payLoad[8];
} __attribute__ ((packed)) IPMI_RECEIVE_SERIAL_DATA;

typedef struct _ipmi_activate_psu_alert
{
	uint8_t    	autoprochot;
	uint8_t    	removedpc;
} __attribute__ ((packed)) IPMI_ACTIVATE_PSU_ALERT;

typedef struct _ipmi_set_nv_dimm_trigger
{
	uint8_t    	adrTrigger;
	uint8_t		adrCompletepoweroffdelay;
	uint8_t		nvDimmPresentPowerOffDelay;
} __attribute__ ((packed)) IPMI_SET_NV_DIMM_TRIGGER;

typedef struct _ipmi_get_memory_info
{
	uint8_t    	type;
	uint16_t	dimmspeed;
	uint32_t	dimmsize;
	uint8_t 	dimmstatus;
} __attribute__ ((packed)) IPMI_GET_MEMORY_INFO;

typedef struct _ipmi_get_memory_index
{
	uint8_t		map1to8; 		//dimm presence info map 1 to 8
	uint8_t		map9to16; 		//dimm presence info map 9 to 16
	uint8_t 	map17to24;		//dimm presence info map 17 to 24
} __attribute__ ((packed)) IPMI_GET_MEMORY_INDEX;

typedef struct _ipmi_get_channel_auth_cap
{
	uint8_t		channelnumber;
	uint8_t		authenticationtype;
	uint8_t 	reserved;
	uint8_t 	servertype;
	uint8_t 	oem_id[3];    /* IANA enterprise number for auth type */
	uint8_t 	oem_aux_data; /* Additional OEM specific data for oem auths */
} __attribute__ ((packed)) _ipmi_get_channel_auth_cap;

#define	PSU_FW_NAME_MAX_LEN		61
typedef struct _ipmi_psu_fw_update
{
	uint8_t		component; // Component type and index
	uint8_t		type; // Image type
	uint8_t		operation; // Update operation
	char		image[PSU_FW_NAME_MAX_LEN]; // Image file name
} __attribute__ ((packed)) IMPI_PSU_FW_UPDATE;

#define I2C_WRITE_MAX_LEN 		240
typedef struct _ipmi_master_write_read 
{
	uint8_t type:1;		// Bus type
	uint8_t bus:3;		// Bus ID
	uint8_t channel:4;	// Channel number
	uint8_t zero:1;		// Reserved, zero.
	uint8_t address:7;	// Device address
	uint8_t rlen;		// Number of bytes to read.
	uint8_t wdata[I2C_WRITE_MAX_LEN];	// Data buffer to write
} __attribute__ ((packed)) IPMI_MASTER_WRITE_READ;
#define	MASTER_WRITE_READ_MIN_LEN	(sizeof (struct _ipmi_master_write_read) - I2C_WRITE_MAX_LEN)

typedef struct _ipmi_master_mux_rw
{
	uint8_t busid;
	uint8_t mux1address;
	uint8_t mux1channel;
	uint8_t mux2address;
	uint8_t mux2channel;
	uint8_t deviceaddress;
	uint8_t readcount;
	uint8_t wdata[I2C_WRITE_MAX_LEN];
} __attribute__ ((packed)) IPMI_MASTER_MUX_RW;
#define MASTER_MUX_RW_MIN_LEN (sizeof(IPMI_MASTER_MUX_RW) - I2C_WRITE_MAX_LEN)

typedef struct _ipmi_master_phase_write_read
{
	uint8_t type:1;		// Bus type
	uint8_t bus:3;		// Bus ID
	uint8_t channel:4;	// Channel number
	uint8_t zero:1;		// Reserved, zero.
	uint8_t address:7;	// Device address
	uint8_t phase;		// PMBUS phase selector
	uint8_t rlen;		// Number of bytes to read.
	uint8_t wdata[I2C_WRITE_MAX_LEN];	// Data buffer to write
} __attribute__ ((packed)) IPMI_MASTER_PHASE_WRITE_READ;
#define	MASTER_PHASE_WRITE_READ_MIN_LEN	(sizeof (struct _ipmi_master_phase_write_read) - I2C_WRITE_MAX_LEN)

typedef struct _ipmi_fpga_health
{
	uint8_t PCIe_HIP0_UP:1;
	uint8_t PCIe_HIP1_UP:1;
	uint8_t LINK0_40G_UP:1;
	uint8_t LINK0_40G_TX_ACT:1;
	uint8_t LINK0_40G_RX_ACT:1;
	uint8_t LINK1_40G_UP:1;
	uint8_t LINK1_40G_TX_ACT:1;
	uint8_t LINK1_40G_RX_ACT:1;
} __attribute__ ((packed)) IPMI_FPGA_HEALTH;

typedef struct _ipmi_fpga_fru_common_header
{
	uint8_t formatversion;
	uint8_t internaluseoffset;
	uint8_t chassisinfooffset;
	uint8_t boardoffset;
	uint8_t productinfooffset;
	uint8_t multirecordoffset;
	uint8_t pad;
	uint8_t checksum;
} __attribute__ ((packed)) IPMI_FPGA_FRU_COMMON_HEADER;

typedef struct _nvme_storage_status
{
	uint8_t length;
	uint8_t status;
	uint8_t smart_wrn;
	uint8_t temp;
	uint8_t pdlu;
	uint16_t vendor;
	uint8_t serial[20];
} __attribute__((packed)) NVME_STORAGE_STATUS;

typedef struct ipmi_active_sessions
{
	uint8_t present;
	uint8_t Firstrecord_sessionid[4];
	uint8_t Secondrecord_sessionid[4];
	uint8_t *consecutive_session;
} __attribute__((packed)) IPMI_ACTIVE_SESSIONS;

enum {
	SERVICE_TYPE_CPU_DEBUG = 1,
	SERVICE_TYPE_FPGA_DEBUG = 2
};

enum {
	SERVICE_CONFIG_START = 0,
	SERVICE_CONFIG_STOP = 1
};
typedef struct ipmi_service_config
{
	uint8_t type;
	uint8_t config;
} __attribute__((packed)) IPMI_SERVICE_CONFIG;

#pragma pack(pop)

int ipmi_msftoem_main(struct ipmi_intf *intf, int argc, char **argv);

#endif /* IPMI_MSFTOEM_H_ */
