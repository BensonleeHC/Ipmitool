# WCSCLI direct-BMC command matrix

This matrix controls the incremental implementation.  A command is enabled
only after its standard or OEM BMC protocol has been confirmed and tested.

| WCSCLI-compatible command | Backend | Status | Notes |
| --- | --- | --- | --- |
| `show system nvme` | OCS OEM IPMI | Implemented | Reuses `ocsoem nvme` |
| `show system info` | Standard + OCS OEM IPMI | Planned | Compose MC, FRU and OEM identity |
| `show system power` | Standard IPMI | Planned | Chassis power status |
| `set system power on/off/cycle/reset` | Standard IPMI | Planned | Reuse chassis handlers |
| `show system boot` | Standard IPMI | Planned | Get boot parameters |
| `set system boot ...` | Standard IPMI | Planned | Boot-device mapping required |
| `show system sensors` | Standard IPMI | Planned | Reuse SDR/sensor handlers |
| `show system fru` | Standard IPMI | Planned | Reuse FRU handler |
| `show system sel` | Standard IPMI | Planned | Reuse SEL handler |
| `show system health` | Standard + OCS OEM IPMI | Research | Define output aggregation |
| `show system bios` | OCS OEM IPMI | Research | Existing OEM functions available |
| `show system nvdimm` | OCS OEM IPMI | Research | Confirm firmware command set |
| `console select bmc` | OCS OEM `34h/93h` | Planned | Payload `01 04` |
| `console select host` | OCS OEM `34h/93h` | Planned | Payload `01 00` |
| `console activate bmc/host` | OEM selector + standard SOL | Planned | Reuse existing SOL implementation |
| Rack Manager API commands | HTTPS/Redfish or legacy REST | Deferred | Separate optional module |
| PMDU/AC commands | Rack-local hardware/API | Out of scope | Not required |
| Linux-local hardware commands | Rack-local hardware | Out of scope | Not required |
| simulated service commands | None | Out of scope | Deferred |

## Protocol acceptance criteria

Before adding a command, record:

1. Standard or OEM NetFn/command and request bytes.
2. Minimum and maximum response length.
3. Completion-code behavior and known firmware variants.
4. Multi-request sequencing, retry, delay, or pagination requirements.
5. A captured response with sensitive fields removed.
6. Offline parser tests and a live-BMC validation result.
