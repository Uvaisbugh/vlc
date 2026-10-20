#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../../.." && pwd)
work=${RELEASE_DEPS_DIR:-"$root/build-release-deps"}
mkdir -p "$work"
archive="$work/medialibrary-0.13.2.tar.bz2"
if [[ ! -f "$archive" ]]; then
    curl --fail --location --retry 3 https://code.videolan.org/videolan/medialibrary/-/archive/0.13.2/medialibrary-0.13.2.tar.bz2 -o "$archive"
fi
python3 - "$archive" "$root/contrib/src/medialibrary/SHA512SUMS" <<'PY'
import hashlib, pathlib, sys
archive = pathlib.Path(sys.argv[1])
expected = pathlib.Path(sys.argv[2]).read_text().split()[0]
if hashlib.sha512(archive.read_bytes()).hexdigest() != expected:
    sys.exit('Media library archive checksum mismatch')
PY
if [[ ! -d "$work/medialibrary-0.13.2" ]]; then
    tar -xjf "$archive" -C "$work"
    for patchfile in "$root"/contrib/src/medialibrary/000*.patch; do
        patch -d "$work/medialibrary-0.13.2" -p1 < "$patchfile"
    done
fi
if [[ ! -f "$work/medialibrary-build/build.ninja" ]]; then
    meson setup "$work/medialibrary-build" "$work/medialibrary-0.13.2" \
        --prefix="$work/prefix" --libdir=lib --buildtype=release \
        -Dlibvlc=disabled -Dtests=disabled
fi
meson compile -C "$work/medialibrary-build" -j "${JOBS:-4}"
meson install -C "$work/medialibrary-build" --no-rebuild
