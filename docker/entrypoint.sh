#!/bin/sh
# SudharshanMart container entrypoint.
#
# Runs the application as an unprivileged user while still allowing Render's
# persistent disk (mounted at the DB_PATH parent directory) to be prepared by
# the container's initial root process.
set -eu

if [ "$(id -u)" = "0" ]; then
    DATA_DIR="$(dirname "${DB_PATH:-/var/data/sudharshanmart.db}")"
    mkdir -p "$DATA_DIR"
    chown -R appuser:appuser "$DATA_DIR" 2>/dev/null || true

    if command -v setpriv >/dev/null 2>&1; then
        exec setpriv --reuid=appuser --regid=appuser --init-groups "$@"
    fi
fi

exec "$@"
