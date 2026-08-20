#ifndef IPMI_OCS_CONSOLE_H
#define IPMI_OCS_CONSOLE_H

#include <ipmitool/ipmi_intf.h>

int ipmi_ocs_console_select(struct ipmi_intf *intf, const char *target);

#endif
