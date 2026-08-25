#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
output_dir="${1:-${repo_dir}/dist/ipmitool-wcscli-linux-x86_64}"

cd "${repo_dir}"
./configure \
  --enable-intf-lan \
  --enable-intf-lanplus \
  --disable-intf-free \
  --disable-intf-open \
  --disable-intf-bmc \
  CFLAGS="${CFLAGS:--O2 -std=gnu99 -fcommon}"
make -j"${BUILD_JOBS:-$(getconf _NPROCESSORS_ONLN)}"

rm -rf "${output_dir}"
mkdir -p "${output_dir}"
cp src/.libs/ipmitool "${output_dir}/ipmitool"
cp doc/README-WCSCLI-WINDOWS.txt "${output_dir}/README-WCSCLI.txt"
cp doc/WCSCLI_DIRECT_BMC_SPEC.md doc/WCSCLI_COMMAND_MATRIX.md "${output_dir}/"
chmod 0755 "${output_dir}/ipmitool"

"${output_dir}/ipmitool" -V
"${output_dir}/ipmitool" -h 2>&1 | grep -q wcscli
file "${output_dir}/ipmitool"
sha256sum "${output_dir}/ipmitool" > "${output_dir}/SHA256SUMS"
