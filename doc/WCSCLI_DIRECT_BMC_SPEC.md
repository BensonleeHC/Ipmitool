# WCSCLI-compatible direct-BMC command specification

Status: initial implementation

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

## 4. Initial command

```text
wcscli show system nvme [-i 1..48]
```

Mapping:

```text
wcscli show system nvme -> existing ocsoem nvme implementation
```

The implementation therefore retains the existing NVMe storage-map request
and subsequent OEM I2C transactions without introducing another protocol
stack.

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
lib/ipmi_ocsoem.c              existing OCS OEM features
include/ipmitool/ipmi_wcscli.h public dispatcher declaration
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
