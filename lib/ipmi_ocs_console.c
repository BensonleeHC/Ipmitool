/* Olympus BMC/host SOL mux selector. */

#include <stdio.h>
#include <string.h>

#include <ipmitool/helper.h>
#include <ipmitool/ipmi.h>
#include <ipmitool/ipmi_intf.h>
#include <ipmitool/ipmi_ocs_console.h>
#include <ipmitool/log.h>

#define OCS_CONSOLE_NETFN 0x34
#define OCS_CONSOLE_CMD   0x93
#define OCS_CONSOLE_HOST  0x00
#define OCS_CONSOLE_BMC   0x04

int
ipmi_ocs_console_select(struct ipmi_intf *intf, const char *target)
{
	struct ipmi_rq request;
	struct ipmi_rs *response;
	unsigned char data[2];

	if (strcmp(target, "bmc") == 0)
		data[1] = OCS_CONSOLE_BMC;
	else if (strcmp(target, "host") == 0)
		data[1] = OCS_CONSOLE_HOST;
	else {
		lprintf(LOG_ERR, "Console target must be 'bmc' or 'host'");
		return -1;
	}
	data[0] = 0x01;
	memset(&request, 0, sizeof(request));
	request.msg.netfn = OCS_CONSOLE_NETFN;
	request.msg.cmd = OCS_CONSOLE_CMD;
	request.msg.data = data;
	request.msg.data_len = sizeof(data);

	response = intf->sendrecv(intf, &request);
	if (response == NULL) {
		lprintf(LOG_ERR, "No response selecting %s console", target);
		return -1;
	}
	if (response->ccode != 0) {
		lprintf(LOG_ERR, "Console select failed: %s",
			val2str(response->ccode, completion_code_vals));
		return -1;
	}
	printf("SOL console selected: %s\n", target);
	return 0;
}
