# WCSCLI dual-platform build

Every future build must produce both of these artifacts from the same commit:

- `ipmitool-wcscli-windows-x64`: MSYS-linked Windows x64 executable and runtime DLLs.
- `ipmitool-wcscli-ubuntu-22.04-x86_64`: native Linux ELF x86-64 executable.

GitHub Actions builds both artifacts on every push to `main` or
`feature/wcscli-direct-bmc`, every pull request, version tag, and manual run.

## Ubuntu build

Install dependencies and run:

```sh
sudo apt-get update
sudo apt-get install -y build-essential autoconf automake libtool libssl-dev file
./scripts/build-linux.sh
```

The Linux artifact is written to `dist/ipmitool-wcscli-linux-x86_64`. It uses
the Ubuntu OpenSSL runtime; install the matching `libssl3` package when copying
it to another Ubuntu server. The artifact supports `lan`, `lanplus`, `ocsoem`,
and `wcscli`; it does not require an MSYS runtime.

## Windows build

In an MSYS2 MSYS shell with `base-devel`, `autoconf`, `automake`, `gcc`,
`libtool`, `make`, and `openssl-devel`:

```sh
./scripts/build-windows-msys2.sh
```

The Windows artifact includes `ipmitool.exe`, `msys-2.0.dll`, and
`msys-crypto-3.dll` in `dist/ipmitool-wcscli-windows-x64`.

## Release acceptance

For both platforms, `ipmitool -V` must exit zero and help must list `wcscli`.
Before publishing a release, test `wcscli show system nvme` and
`wcscli show system log` against an Olympus BMC and retain `SHA256SUMS`.
