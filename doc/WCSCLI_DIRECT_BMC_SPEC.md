# WCSCLI-compatible direct-BMC command specification

Status: implementation complete; NVMe path live-BMC qualified

Branch: `feature/wcscli-direct-bmc`

Target: Windows x64 MSYS build of ipmitool 1.8.16-cvs

## 1. Purpose

Expose familiar Olympus Rack Manager command names while connecting directly
to a server BMC.  A Rack Manager is not required for commands implemented by
the BMC through standard IPMI or Microsoft OCS OEM IPMI messages.

The command boundary is:

```text
ipmitool [global connection options] wcscli [WCSCLI command and options]
```

Example:

```powershell
$env:IPMI_PASSWORD = "password"
.\ipmitool.exe -I lanplus -H 192.0.2.10 -U admin -E `
    wcscli show system nvme -i 12
Remove-Item Env:IPMI_PASSWORD
```

Global options must precede `wcscli`.  The global parser stops at the first
non-option so that uppercase `-I` belongs to ipmitool and lowercase `-i`
belongs to the WCSCLI parser.

## 2. Target selection

In direct-BMC mode, `-H <BMC-IP>` is authoritative.  The WCSCLI option
`-i <instance>` is accepted in the range 1 through 48 solely for command-line
compatibility with Rack Manager usage.  It does not override or translate the
host selected by `-H`.

A future Rack Manager transport may assign server-selection semantics to
`-i`; that transport must remain optional and separate from direct-BMC OEM
commands.

## 3. Parser ownership

| Parser | Owned arguments |
| --- | --- |
| ipmitool global parser | `-I`, `-H`, `-U`, `-E`, `-P`, timeout and session options |
| WCSCLI dispatcher | verbs, targets, features, `-i`, and feature-specific options |
| Feature module | OEM payload creation, response validation, and decoding |

The WCSCLI dispatcher must not duplicate OEM request implementations.  It
routes a compatible command to a shared feature handler.  Legacy `ocsoem`
syntax remains available during migration.

## 4. Command surface

```text
wcscli show system <info|health|fru|nvme|state|nextboot|led> [-i 1..48]
wcscli show system log [read] [-i 1..48]
wcscli show system bios <config|code> [options] [-i 1..48]
wcscli show system tpm presence [-i 1..48]
wcscli set system <on|off|reset> [-i 1..48]
wcscli set system <nextboot|led|log|bios|tpm|console> ... [-i 1..48]
```

The dispatcher reuses standard `power`, `chassis`, `fru`, `sdr`, `sel`, and
`mc` handlers plus existing `ocsoem` handlers. See `WCSCLI_COMMAND_MATRIX.md`
for exact mappings. NVMe therefore retains its storage-map and subsequent OEM
I2C transactions without a duplicate protocol stack.

The console selector is its own OEM protocol module. It sends NetFn `0x34`,
command `0x93`, payload `01 04` for BMC or `01 00` for Host. The user then runs
the existing `sol activate` command.

## 5. Error behavior

- Missing command components return a non-zero exit status and show usage.
- Unknown commands return a non-zero exit status and show supported commands.
- `-i` without a value, outside 1-48, or containing non-decimal input fails.
- Unsupported WCSCLI options fail instead of being silently ignored.
- BMC/IPMI completion errors propagate from the shared feature handler.

## 6. Module boundaries

```text
src/ipmitool.c                 top-level command registration
lib/ipmi_main.c                global option boundary
lib/ipmi_wcscli.c              WCSCLI grammar and dispatch only
lib/ipmi_ocs_console.c          OEM console-mux request module
lib/ipmi_ocsoem.c              existing OCS OEM features
include/ipmitool/ipmi_wcscli.h public dispatcher declaration
include/ipmitool/ipmi_ocs_console.h console selector interface
```

Future protocol refactoring should separate generic OCS request helpers from
feature decoders, but it must preserve the shared-handler rule: `wcscli` and
`ocsoem` are alternate command surfaces over one implementation.

## 7. Explicitly out of scope

- PMDU and AC socket management
- Rack Manager Linux-local networking, users, and services
- Linux-local GPIO, ADC, UART, and I2C access
- legacy simulated WCSCLI service commands
- serial-session management outside standard IPMI SOL
- automatic server-ID-to-BMC-address translation in direct-BMC mode

## 8. Windows build and live validation

The Windows x64 build must expose POSIX/GNU function prototypes consistently.
In particular, `log.c` defines `_GNU_SOURCE` before system headers so that
`strdup()` has the correct pointer return type. Without this on a strict C99
64-bit build, shutdown can abort inside `log_halt()` and appear to hang.

Live test target `10.46.127.44` verified the following with `-N 1 -R 1`:

| Command | Result | Elapsed |
| --- | --- | --- |
| `raw 0x38 0x86` | Exit 0, `0d 00 00 00 00 00` | 400 ms |
| `ocsoem nvme` | Exit 0 | 406 ms |
| `wcscli show system nvme -i 1` | Exit 0, output matched `ocsoem` | 411 ms |

Some drive-specific OEM I2C requests returned completion code `0xCC`; those
errors were reported without preventing normal process termination.

WCSCLI log output prefixes every decoded SEL row with the complete 16-byte
record in BMC byte order, matching Rack Manager output. Standard
`ipmitool sel elist` remains unchanged.
