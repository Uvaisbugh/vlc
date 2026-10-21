# Build and publish a Windows preview

The Windows preview pipeline produces an installer and portable ZIP from the
same validated native runtime. It requires a fresh successful **Windows x64 VLC
build** from `master`, including Windows playback and native Qt tests. It does
not reuse the dated public preview or turn an old binary into a new source build.

## Build the native runtime

From an authenticated GitHub CLI, start the existing workflow:

```sh
gh workflow run windows-build.yml --repo Uvaisbugh/vlc --ref master
gh run list --repo Uvaisbugh/vlc --workflow windows-build.yml --branch master
```

Record the numeric run ID and wait for all four jobs to succeed. Build logs,
source SHA, environment metadata, runtime checksums and native test results
remain attached to that run. The source and dependency recipe is described in
[DEVELOPMENT.md](DEVELOPMENT.md) and
[BUILD_AND_RELEASE.md](../redesign/BUILD_AND_RELEASE.md).

## Package and validate

Use **Actions → Windows preview installer and release → Run workflow**, select
`master`, provide the successful build run ID and a unique date-based tag such as
`reimagined-preview-2026.10.21`. Leave **publish** unchecked for an artifact-only
preflight. A corresponding CLI example (replace the example run ID):

```sh
gh workflow run windows-preview-release.yml --repo Uvaisbugh/vlc --ref master \
  -f build_run=123456789 -f release_tag=reimagined-preview-2026.10.21 -f publish=false
```

The workflow rejects an incomplete/failed run, a non-master native source, and
application/dependency changes after compilation. Documentation and packaging
may change while the native build runs, but the application source must remain
identical. Python verifies the downloaded archive's checksum, required files,
safe extraction and empty portable profile before creating the portable ZIP.

The Windows runner uses its Inno Setup 6 compiler and
[`reimagined-setup.iss`](../../extras/package/win32/reimagined-setup.iss) to create
the installer. It tests the extracted ZIP and installed program with generated
audio/video, verifies the Start menu shortcut, reinstalls to check profile
preservation, and uninstalls to verify program/shortcut removal and retained
profile data. Its installation path includes spaces. No file associations are
registered. The installer is per-user and requests no administrator privileges.

The output includes `vlc-reimagined-windows-x64-setup.exe`,
`vlc-reimagined-windows-x64-portable.zip`, matching source (including packaging
scripts and contrib recipes), build metadata, validation summary and checksums.
The metadata records both native compilation and packaging/source-snapshot SHAs.
An installer is not a code-signing certificate: the preview remains unsigned.
Windows ARM64/32-bit and other OS packages are not offered by this pipeline.

## Publish and check downloads

After explicit maintainer authorization, rerun with **publish** checked (or
`-f publish=true`). The publish job can run only after packaging and installer
validation succeeds. It verifies all checksums, creates a unique preview tag
and public prerelease, and uploads the tested files. It does not replace an
existing tag or silently overwrite existing release assets.

Verify the public release and both user downloads, then update README and
[GETTING_STARTED.md](GETTING_STARTED.md) to link directly to its setup and ZIP.
Keep older preview links available for rollback. Do not claim validation beyond
the exact checks recorded by the workflow. Keep the original design and software
credits with both packages.

The dated preview tags intentionally do not trigger the inherited semantic-tag
multi-platform beta workflow. In-app update discovery still targets the earlier
community fork; use this repository's releases manually until its full contract
is updated and tested.
