/*
 * WCSCLI-compatible command dispatcher for direct BMC access.
 */

#ifndef IPMI_WCSCLI_H
#define IPMI_WCSCLI_H

#include <ipmitool/ipmi_intf.h>

int ipmi_wcscli_main(struct ipmi_intf *intf, int argc, char **argv);

#endif /* IPMI_WCSCLI_H */
