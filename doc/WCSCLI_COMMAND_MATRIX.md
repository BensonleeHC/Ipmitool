# WCSCLI direct-BMC command matrix

Status means implemented and build-tested; all BMC-dependent rows still need
validation against an Olympus server.

| WCSCLI command | Backend mapping | Status |
| --- | --- | --- |
| `show system info` | `mc info`, then `fru` | Implemented |
| `show system health` | `sdr elist` | Implemented (direct-BMC view) |
| `show system fru` | `fru` | Implemented |
| `show system nvme` | `ocsoem nvme` | Implemented |
| `show system state` | `power status` | Implemented |
| `set system on/off/reset` | `power on/off/reset` | Implemented |
| `show system nextboot` | `chassis bootparam get 5` | Implemented |
| `set system nextboot -t 1..5` | `none/pxe/disk/bios/floppy` | Implemented |
| `show system led` | `ocsoem getledstatus` | Implemented |
| `set system led on/off` | `chassis identify force/0` | Implemented |
| `show system log [read]` | `sel elist` | Implemented; `read` is optional |
| `set system log clear` | `sel clear` | Implemented |
| `show system bios config` | `ocsoem biosconfig` | Implemented |
| `show system bios code [-v current/previous]` | `ocsoem bioscode` | Implemented |
| `set system bios config -j J -n N` | `ocsoem setbiosconfig` | Implemented |
| `show system tpm presence` | `ocsoem gettpmphypresence` | Implemented |
| `set system tpm presence -p 0/1` | `ocsoem settpmphypresence` | Implemented |
| `set system console bmc/host` | OEM `34h/93h`, `01 04/00` | Implemented |
| `sol activate` | Existing standard ipmitool command | Already available |
| Rack Manager API commands | Optional future transport module | Deferred |
| PMDU/AC, Linux-local hardware, simulation | None | Out of scope |

`-i 1..48` may occur anywhere after `wcscli`. It is validated and discarded
because `-H` directly identifies the destination BMC.

## Validation criteria

For release qualification, capture redacted live-BMC output, completion codes,
firmware version, response-length variants, and any required retries for each
OEM command family.
