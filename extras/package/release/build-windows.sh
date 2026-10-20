#!/usr/bin/env bash
# Run inside the pinned VLC Debian cross-build image, from a separate work dir.
set -euo pipefail
root=$(cd "$(dirname "$0")/../../.." && pwd)
RELEASE_TAG=$(python3 "$root/extras/package/release/version.py")
export RELEASE_TAG
# Keep contrib Rust packages below the root Cargo workspace's excluded contrib/.
work="$root/contrib/contrib-release-win64"
mkdir -p "$work"
cd "$work"
# Avoid -i: upstream also makes installers, whereas this release is portable.
# Build patched contribs from source; upstream prebuilt contribs lack fork patches.
# Target executables need a Windows host; Wine may not work under ARM emulation.
export VLC_NO_EXE_WRAPPER=1
export MCONFIGFLAGS="${MCONFIGFLAGS:-} -Dmedialibrary=enabled -Dtests=disabled -Dnsis=disabled -Dwix=disabled"
"$root/extras/package/win32/build.sh" -a x86_64 -r -i '' -m -l -o "$work/install"
# Unlike Windows API DLLs, the Vulkan loader is a redistributable dependency.
cp "$work/contrib/x86_64-w64-mingw32/bin/vulkan-1.dll" "$work/install/"
mkdir -p "$work/install/licenses/vulkan-loader"
cp "$work/contrib/contrib-win64/vulkan-loader/LICENSE.txt" \
    "$work/contrib/contrib-win64/vulkan-loader/LICENSES/"*.txt \
    "$work/install/licenses/vulkan-loader/"
python3 "$root/extras/package/release/package.py" windows "$work/install" --output "$root/build-release-output"
