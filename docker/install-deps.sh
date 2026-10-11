#!/usr/bin/env bash
# Resolve the SudharshanMart vcpkg manifest and, on failure, surface the
# underlying vcpkg error at the very end of the build log so it is easy to spot
# in Render (which otherwise only shows the generic "exit code: 1").
set -uo pipefail

LOG_FILE=/tmp/vcpkg-install.log

/opt/vcpkg/vcpkg install \
    --triplet=x64-linux-release \
    --overlay-triplets=/app/triplets \
    --clean-after-build 2>&1 | tee "$LOG_FILE"
status=${PIPESTATUS[0]}

if [ "$status" -ne 0 ]; then
    echo "==================== VCPKG INSTALL FAILED ===================="
    echo ">>> First error line reported by vcpkg:"
    grep -m1 -i "error" "$LOG_FILE" || true
    echo ">>> Last 40 lines of the vcpkg log:"
    tail -n 40 "$LOG_FILE" || true
    echo "=============================================================="
    exit "$status"
fi
