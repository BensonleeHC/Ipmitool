WCSCLI-compatible ipmitool for Windows x64
===========================================

Version: ipmitool 1.8.16-cvs
Interfaces: lan, lanplus
Commands: standard ipmitool, ocsoem, wcscli

PowerShell example:

  $env:IPMI_PASSWORD = "your-password"
  .\ipmitool.exe -I lanplus -H <BMC-IP> -U <user> -E `
      wcscli show system nvme -i 12
  Remove-Item Env:IPMI_PASSWORD

-I is parsed globally as the IPMI interface. Lower-case -i is parsed only by
wcscli and accepts 1..48 for Rack Manager syntax compatibility. In direct-BMC
mode -H is the actual destination; -i does not perform RM address translation.

Implemented show commands:
  info, health, fru, nvme, state, nextboot, led, log, log read,
  bios config, bios code, tpm presence

Implemented set commands:
  on, off, reset, nextboot, led, log clear, bios config, tpm presence,
  console bmc, console host

Console example:
  .\ipmitool.exe <connection-options> wcscli set system console host -i 12
  .\ipmitool.exe <connection-options> sol activate

Select BMC instead with `console bmc`. The selector uses OEM NetFn 34h,
command 93h, payload 01 00 for Host or 01 04 for BMC. SOL activation remains
the standard ipmitool command and is intentionally not hidden in wcscli.

See WCSCLI_DIRECT_BMC_SPEC.md and WCSCLI_COMMAND_MATRIX.md. The NVMe path and
normal process exit were verified against an Olympus BMC at 10.46.127.44.
