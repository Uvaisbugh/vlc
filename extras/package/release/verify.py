#!/usr/bin/env python3
"""Smoke-test an installed release with generated media and ignored VLC preferences."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile
from version import verify_binary_version

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('executable', type=Path)
parser.add_argument('--expected-version', help='Require the exact compiled release version')
parser.add_argument('--sample', type=Path, help='Use existing media instead of generating it with FFmpeg')
parser.add_argument('--gui', action='store_true', help='Also open the Qt interface for a short audio playback check')
parser.add_argument('--video', action='store_true', help='With --gui, also require video decoding and a Qt video window')
args = parser.parse_args()
if args.video and not args.gui:
    parser.error('--video requires --gui')
with tempfile.TemporaryDirectory(prefix='vlc-release-smoke-') as temp:
    state = Path(temp)
    env = dict(os.environ, XDG_CONFIG_HOME=str(state / 'config'),
               XDG_DATA_HOME=str(state / 'data'), XDG_CACHE_HOME=str(state / 'cache'),
               VLC_USERDATA_PATH=str(state / 'userdata'))
    executable = str(args.executable.resolve())
    try:
        print(verify_binary_version(executable, args.expected_version, env))
    except (ValueError, OSError, subprocess.SubprocessError) as error:
        raise SystemExit(str(error))
    modules = subprocess.run([executable, '--ignore-config', '--list'], env=env, timeout=60,
                             check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    for required in ('qt', 'avcodec', 'medialibrary'):
        if not any(line.split() and line.split()[0] == required for line in modules.stdout.splitlines()):
            raise SystemExit(f'Required module missing: {required}')
    sample = args.sample.resolve() if args.sample else state / 'sample.mkv'
    if not args.sample:
        subprocess.run(['ffmpeg', '-nostdin', '-v', 'error', '-f', 'lavfi', '-i',
                        'testsrc2=size=320x180:rate=24', '-f', 'lavfi', '-i',
                        'sine=frequency=440:sample_rate=48000', '-t', '2',
                        '-c:v', 'ffv1', '-c:a', 'pcm_s16le', str(sample)], check=True)
    # --no-video avoids the AppKit minimal-vout path during a headless Mac check.
    playback = subprocess.run([executable, '--ignore-config', '--no-media-library',
        '--no-metadata-network-access', '-I', 'dummy', '--no-video', '--aout=dummy',
        '--play-and-exit', '-vv', str(sample)], env=env, timeout=45, check=True,
        stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    print(playback.stdout)
    if 'using audio decoder module' not in playback.stdout:
        raise SystemExit('No audio decoder engaged during playback')
    if args.gui:
        gui = subprocess.run([executable, '--ignore-config', '--no-media-library',
            '--no-metadata-network-access', '-I', 'qt', *([] if args.video else ['--no-video']), '--aout=dummy',
            '--no-qt-privacy-ask', '--no-qt-updates-notif', '--play-and-exit', '-vv', str(sample)], env=env,
            timeout=60, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        print(gui.stdout)
        if 'using interface module "qt"' not in gui.stdout:
            raise SystemExit('Qt interface did not start')
        if args.video:
            for required in ('using video decoder module', 'using vout window module "qt"', 'using vout display module'):
                if required not in gui.stdout:
                    raise SystemExit(f'Video playback missing: {required}')
            if 'using vout display module "vdummy"' in gui.stdout or 'video output display creation failed' in gui.stdout:
                raise SystemExit('Video output did not render to a working display')
        for failure in ('failed to load component', 'QQmlApplicationEngine failed', 'is not installed', 'qml loading', 'Cannot load the VLC interface', 'svg icon plugin is not found'):
            if failure in gui.stdout:
                raise SystemExit(f'Qt interface reported: {failure}')
print('Release version, required modules and playback smoke checks passed.')
