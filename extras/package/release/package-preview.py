#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Stage a verified native Windows runtime for the Reimagined preview."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import tarfile
import tempfile
import zipfile

ROOT = Path(__file__).resolve().parents[3]


def verified_run(run):
    if (run.get('conclusion') != 'success' or run.get('headBranch') != 'master'
            or run.get('workflowName') != 'Windows x64 VLC build'
            or not re.fullmatch(r'[0-9a-f]{40}', run.get('headSha', ''))):
        raise ValueError('A successful Windows build from master is required')
    jobs = {job['name']: job.get('conclusion') for job in run.get('jobs', [])}
    for name in ('packaging-tests', 'windows-x64', 'windows-smoke',
                 'native-qt-tests / native-qt-tests'):
        if jobs.get(name) != 'success':
            raise ValueError(f'Required validation job did not succeed: {name}')
    return run['headSha']


def stage_runtime(archive, checksums, output, run, packaging_commit, release_tag):
    binary_commit = verified_run(run)
    expected = checksums.read_text().strip().split()[0].lower()
    actual = hashlib.sha256(archive.read_bytes()).hexdigest()
    if expected != actual:
        raise ValueError('Native runtime archive checksum mismatch')
    if output.exists():
        raise ValueError('Use a new, empty output path')
    with tempfile.TemporaryDirectory() as temporary:
        extraction = Path(temporary)
        with tarfile.open(archive) as bundle:
            # Python 3.12+: reject links escaping the extraction directory and devices.
            bundle.extractall(extraction, filter='data')
        runtime = extraction / 'vlc-win64'
        required = ('vlc.exe', 'libvlc.dll', 'libvlccore.dll', 'vulkan-1.dll',
                    'plugins', 'COPYING', 'COPYING.LIB', 'licenses/Inter',
                    'licenses/design', 'licenses/vulkan-loader')
        for name in required:
            if not (runtime / name).exists():
                raise ValueError(f'Incomplete native runtime: {name}')
        # Never redistribute a test profile or a machine-specific plugin cache.
        if (runtime / 'portable').exists() and any((runtime / 'portable').iterdir()):
            raise ValueError('Native archive contains a populated portable profile')
        target = output / 'VLC Reimagined'
        shutil.copytree(runtime, target)
    for cache in target.rglob('plugins.dat'):
        cache.unlink()
    (target / 'portable').mkdir(exist_ok=True)
    (target / 'SOURCE_COMMIT.txt').write_text(binary_commit + '\n', newline='\n')
    for name in ('README.md', 'AUTHORS', 'THANKS'):
        shutil.copy2(ROOT / name, target / name)
    shutil.copy2(ROOT / 'modules/gui/qt/pixmaps/redesign/CREDITS.md', target / 'DESIGN_CREDITS.md')
    (target / 'START_HERE.txt').write_text(
        'VLC Reimagined - unofficial Windows x64 preview\n\n'
        'Portable: extract the entire ZIP, then open VLC Reimagined\\vlc.exe.\n'
        'Installer: run vlc-reimagined-windows-x64-setup.exe and follow the wizard.\n'
        'The installer adds a Start menu shortcut and an uninstaller; no admin is required.\n'
        'Keep DLLs/plugins beside vlc.exe. Settings stay in the portable folder.\n'
        'This unsigned community build is not endorsed by VideoLAN or Juxtopposed.\n'
        'Design: Juxtopposed (CC BY 4.0). Native implementation: David Sojevic and contributors.\n'
        'Engine: VideoLAN VLC. See COPYING, COPYING.LIB and licenses.\n'
        'Downloads and updates: https://github.com/Uvaisbugh/vlc/releases\n'
        'Use that page manually; the inherited in-app updater targets the earlier fork.\n',
        encoding='utf-8', newline='\n')
    metadata = dict(product='VLC Reimagined', channel='preview', platform='windows-x86_64',
                    binary_source_commit=binary_commit, packaging_source_commit=packaging_commit,
                    release_tag=release_tag, native_runtime_sha256=actual, signing='unsigned',
                    build_and_validation_run=run['url'], native_qt_validation='passed',
                    installer_validation='see release workflow and VALIDATION.txt',
                    full_figma_parity='not fully audited',
                    credits=dict(design='Juxtopposed', native_implementation='David Sojevic and contributors',
                                 engine='VideoLAN and VLC contributors'))
    (output / 'BUILD_INFO.json').write_text(json.dumps(metadata, indent=2) + '\n', newline='\n')
    shutil.copy2(output / 'BUILD_INFO.json', target / 'BUILD_INFO.json')
    portable = output / 'vlc-reimagined-windows-x64-portable.zip'
    with zipfile.ZipFile(portable, 'w', zipfile.ZIP_DEFLATED, strict_timestamps=False) as bundle:
        for path in sorted(target.rglob('*')):
            bundle.write(path, path.relative_to(output))
    return target


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--runtime', type=Path, required=True)
    parser.add_argument('--checksums', type=Path, required=True)
    parser.add_argument('--run-json', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--packaging-commit', required=True)
    parser.add_argument('--release-tag', required=True)
    args = parser.parse_args()
    if not re.fullmatch(r'[0-9a-f]{40}', args.packaging_commit):
        parser.error('packaging commit must be a full SHA')
    if not re.fullmatch(r'reimagined-preview-\d{4}\.\d{2}\.\d{2}(?:\.\d+)?', args.release_tag):
        parser.error('release tag must be reimagined-preview-YYYY.MM.DD[.N]')
    stage_runtime(args.runtime, args.checksums, args.output,
                  json.loads(args.run_json.read_text(encoding='utf-8-sig')),
                  args.packaging_commit, args.release_tag)


if __name__ == '__main__':
    main()
