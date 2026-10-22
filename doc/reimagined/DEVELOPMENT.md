# Developing VLC Reimagined

[Contributing](../../CONTRIBUTING.md) · [Status](STATUS.md) · [Roadmap](../../ROADMAP.md)

## Pick the appropriate path

Documentation changes need only a text editor and Git. For application changes,
the verified infrastructure is a Windows cross-build and a native Linux Qt test
build. This Windows checkout has no configured Docker/Meson/native Qt build
environment; the commands below are traced to repository scripts and prior CI,
not a newly completed clean-machine build.

| Goal | Starting point |
| --- | --- |
| Try the application | [Ready-to-run Windows preview](GETTING_STARTED.md) |
| Build a Windows runtime | Pinned VideoLAN Linux cross-build image and `windows-build.yml`, below |
| Develop/test Qt/QML | Native Linux environment and matching media-library/Qt dependencies, below |
| Explore Linux desktop dependencies | [Inherited Linux guide](../BUILD-linux.md); its baseline disables media library |
| Explore macOS | [Inherited macOS guide](../BUILD-macos.md); no validated macOS distribution from this fork |
| Understand portable packaging | [Release tooling reference](../RELEASING.md), distinct from current preview evidence |
| Build the Windows setup installer and publish a preview | [Windows release guide](RELEASING_WINDOWS.md), with install/repair/uninstall gates |

## Get the right source

In a Git-capable shell:

```sh
git clone --branch master https://github.com/Uvaisbugh/vlc.git vlc-reimagined
cd vlc-reimagined
git rev-parse HEAD
```

Keep full history: dependency selection uses Git history. For an existing shallow
clone, use `git fetch --unshallow`. To inspect the exact published binary source,
use `git switch --detach redesign-preview-2026.10.20`; return to
`master` before creating a contribution branch. The tag's binary
source is `0279317bbd0e26bfaec7e6cc3b854d2b92801de8`.

## Architecture and source map

This modifies VLC itself, using its existing native Qt interface and playback
engine. It is not an Electron/browser wrapper or a newly written media engine.

| Area | Source |
| --- | --- |
| VLC C core / embedding library / modules | `src/`, `lib/`, `modules/` |
| Qt C++ integration and player controller | `modules/gui/qt/`, especially `player/player_controller.cpp` |
| Redesigned QML views and control bindings | `modules/gui/qt/player/qml/redesign/RedesignPlayer.qml` and adjacent components |
| Library models | `modules/gui/qt/medialibrary/`, `RedesignLibrary.qml`; matching patches in `contrib/src/medialibrary/` |
| Native C++/QML tests | `modules/gui/qt/tests/` |
| Resource and build registration | `modules/gui/qt/assets.qrc`, `modules/gui/qt/meson.build`, `modules/gui/qt/Makefile.am` |
| Runtime packaging and checks | `extras/package/release/`, `extras/package/win32/test-runtime.ps1` |

Meson requires version 1.10.0 or later and declares GNU C17/C++17. The Qt module
requires Qt 6.2 or later with private headers and Core, Gui, Widgets, Qml, Network,
Quick and QuickControls2. Those minimums alone do not guarantee a working build:
use the validated environment when reproducing CI. VLC also retains
Autotools/Make; the root Meson file describes Meson support as experimental.

## Windows cross-build: reproduce the validated route

Use a Linux x86_64 host with Docker, Git, network access and enough storage/time
for VLC plus its contrib dependencies. Docker on Windows must use Linux containers;
the following commands are **Bash**, not PowerShell. Local Docker execution has
not been revalidated for this documentation change.

From the repository root, enter the pinned image used for the prior Windows build:

```sh
docker run --rm -it --user root --platform linux/amd64 \
  -v "$PWD:/src" -w /src \
  registry.videolan.org/vlc-debian-win64-posix@sha256:8e83e800036f8f4a6da1031a6b80c2b5bad8337259c072ffa22398baaeaad880 \
  bash
```

Inside that container:

```sh
git config --global --add safe.directory /src
export JOBS=4
extras/package/win32/build.sh -a x86_64 -x -o "$PWD/artifacts/vlc-win64"
```

This builds matching dependencies from source; it omits the workflow's optional
exact-match prebuilt archive optimization. Do not reuse the unchanged upstream
baseline archive for the redesign: the integrated media library is patched and
Qt networking is required. The staged executable is
`artifacts/vlc-win64/vlc.exe`. A Linux container cannot establish Windows runtime
correctness. Container writes may be root-owned on a Linux host.

For a portable distribution, follow the **whole** build/package step in
[`windows-build.yml`](../../.github/workflows/windows-build.yml): strip through
`make -C win64 package-win-strip`, include the Vulkan loader, retain dependency,
Inter, design and VLC notices, archive the complete runtime, and retain SHA-256
and source/environment metadata. A bare staged EXE is not a redistributable package.
The alternative [release wrapper](../RELEASING.md) uses a separate packaging
path and naming scheme; do not describe it as the recipe used for the dated preview.

On Windows, validate the complete runtime from the repository root with
PowerShell 7 (`pwsh`, also used by GitHub's Windows runner):

```powershell
# Use a small local video with an audio track; replace the fixture path.
pwsh -NoProfile -File .\extras\package\win32\test-runtime.ps1 `
  -RuntimeDirectory '.\artifacts\vlc-win64' `
  -EvidenceDirectory '.\build-smoke' `
  -VideoFixture 'C:\path\to\sample.mkv'
```

The script generates PCM audio, checks dummy/Qt startup and decoding, and checks
Qt video output when a video fixture is supplied. It uses dummy audio output;
speaker output and interactive UI checks remain separate. Do not bypass an
execution-policy restriction: run within the development policy of your machine.

## Linux native build and Qt tests

The exact CI recipe is
[`native-qt-tests.yml`](../../.github/workflows/native-qt-tests.yml), using
`registry.videolan.org/vlc-debian-unstable:20260611225331` and Qt 6.10.2.
Its setup installs checksummed matching SVG runtime packages, builds the patched
media library, and runs tests as an ordinary user. Preserve those steps when
reproducing CI; root bypasses read-only fixtures, and a missing/mismatched SVG
plugin invalidates the UI tests.

For an already-provisioned Linux development environment with GCC/Clang, Meson,
Ninja, pkg-config, Python 3, Bash, curl, patch, tar and the media/Qt dependencies
(see the [Linux prerequisite list](../BUILD-linux.md)), run from the repo root:

```sh
JOBS=4 extras/package/release/build-medialibrary.sh
export PKG_CONFIG_PATH="$PWD/build-release-deps/prefix/lib/pkgconfig:${PKG_CONFIG_PATH:-}"
export LD_LIBRARY_PATH="$PWD/build-release-deps/prefix/lib:${LD_LIBRARY_PATH:-}"
meson setup build-redesign --buildtype=debugoptimized \
  -Dqt=enabled -Dmedialibrary=enabled -Dtests=enabled -Dextra_checks=true
meson compile -C build-redesign -j 4
```

Run tests as an **ordinary user**. Match the bundled Inter font and ensure the
Qt SVG image plugin is available. Isolate test data from your regular VLC profile:

```sh
test_root="$(mktemp -d)"
mkdir -p "$test_root/config" "$test_root/data" "$test_root/cache"
export XDG_CONFIG_HOME="$test_root/config"
export XDG_DATA_HOME="$test_root/data"
export XDG_CACHE_HOME="$test_root/cache"
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QT_SCALE_FACTOR=1 \
  meson test -C build-redesign --suite qt --print-errorlogs --timeout-multiplier 2
```

Meson builds test prerequisites automatically; the normal application build does
not build every test target. Do not add `--no-rebuild` unless those targets have
already been built. To limit build concurrency, first run
`ninja -C build-redesign -j 4 meson-test-prereq`, then add `--no-rebuild` to the test
command. A focused example is:

```sh
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software QT_SCALE_FACTOR=1 \
  meson test -C build-redesign tst_RedesignLibrary --print-errorlogs
```

Use `meson introspect --tests build-redesign` to inspect registered names.
Keep `build-redesign/meson-logs/testlog.txt` and relevant error output with your
report. A standalone QML test without the compiled `VLC.Util` module is not a
substitute for the native suite.

For interactive development, keep the isolated XDG variables above, run from a
desktop session, and use the build-tree launcher:

```sh
./build-redesign/bin/vlc-static --no-one-instance --intf=qt
```

Do not set `VLC_PLUGIN_PATH` to your system VLC 3 installation. The build needs
its own VLC 4 plugins and resources. Do not install over the system VLC.

## Debugging and lightweight checks

For native Linux debugging, use `gdb --args ./build-redesign/bin/vlc-static
--no-one-instance --intf=qt -vv` with the isolated profile above. Debug builds and
unstripped binaries are more useful than the stripped preview. The
[Windows build guide](../BUILD-win32.md) explains MinGW/GDB versus LLVM/PDB
debugging and plugin/library paths. Record source SHA and dependency versions
when comparing failures.

From the repository root, use Python 3.12 or later for the release-tool checks;
the preview packager uses tarfile's safe extraction filter. No Qt build is needed:

```sh
python3 extras/package/release/version.py 4.0.0-beta.4
python3 -m unittest discover -s extras/package/release/tests
git diff --check
```

On Windows, use `python` instead of `python3` when that is your installed Python
launcher. The semantic version is the source version; it is distinct from the
published preview's date-based tag. No OpenSubtitles key is required to build;
see [Qt HACKING](../../modules/gui/qt/HACKING) if working on provider integration.
Keep keys and generated private headers out of commits and logs.

## CI and release boundaries

- `windows-build.yml` runs manually or on selected file changes pushed to
  `codex/vlc-redesign`; it does not have a `pull_request` trigger.
- `native-qt-tests.yml` supports `workflow_call` and pushes affecting its own
  configuration on that branch. The Windows workflow invokes it as a reusable
  workflow. Inspect actual jobs, not just a badge.
- `beta-release.yml` is inherited multi-platform tooling. Manual runs build
  artifacts; matching semantic tags can create draft releases. Its existence
  does not establish this fork's Linux/macOS support.

Maintainers should run relevant checks on reviewed candidate code and record
the exact SHA before merging. Missing CI is not passing CI. Contributors do not
need to publish a release or obtain secrets. Building, publishing and changing
repository settings are separate actions.
