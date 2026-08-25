#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output_dir="${1:-${repo_dir}/dist/ipmitool-wcscli-windows-x64}"

cd "${repo_dir}"
./configure \
  --enable-intf-lan \
  --enable-intf-lanplus \
  --disable-intf-free \
  --disable-intf-open \
  --disable-intf-bmc \
  CFLAGS="${CFLAGS:--O2 -std=gnu99 -fcommon -include sys/select.h -Wno-error=implicit-function-declaration -Wno-error=incompatible-pointer-types -Wno-error=int-conversion}"
make -j"${BUILD_JOBS:-2}"

rm -rf "${output_dir}"
mkdir -p "${output_dir}"
cp src/.libs/ipmitool.exe /usr/bin/msys-2.0.dll /usr/bin/msys-crypto-3.dll "${output_dir}/"
cp doc/README-WCSCLI-WINDOWS.txt doc/WCSCLI_DIRECT_BMC_SPEC.md \
  doc/WCSCLI_COMMAND_MATRIX.md "${output_dir}/"

"${output_dir}/ipmitool.exe" -V
"${output_dir}/ipmitool.exe" -h 2>&1 | grep -q wcscli
sha256sum "${output_dir}"/* > "${output_dir}/SHA256SUMS"
