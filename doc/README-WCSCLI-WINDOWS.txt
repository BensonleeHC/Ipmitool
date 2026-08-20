WCSCLI-compatible ipmitool for Windows x64
===========================================

Version: ipmitool 1.8.16-cvs
Interfaces: lan, lanplus
Commands: standard ipmitool, ocsoem, wcscli

Initial WCSCLI command:

  ipmitool.exe -I lanplus -H <BMC-IP> -U <user> -E \
      wcscli show system nvme [-i 1..48]

PowerShell password example:

  $env:IPMI_PASSWORD = "your-password"
  .\ipmitool.exe -I lanplus -H <BMC-IP> -U <user> -E `
      wcscli show system nvme -i 12
  Remove-Item Env:IPMI_PASSWORD

The uppercase -I is an ipmitool global option.  The lowercase -i belongs to
the WCSCLI command.  All global connection options must appear before
"wcscli".

Direct-BMC target behavior:

  -H selects the BMC that receives the command.  The optional -i value is
  accepted for Rack Manager command-line compatibility but does not change
  the direct-BMC target.

Legacy equivalent:

  ipmitool.exe -I lanplus -H <BMC-IP> -U <user> -E ocsoem nvme

Runtime files:

  ipmitool.exe
  msys-2.0.dll
  msys-crypto-3.dll

Live-BMC validation is still required.
