#!/usr/bin/env python3
"""Include font/audio libraries omitted by linuxdeploy's desktop baseline."""
import argparse
from pathlib import Path
import re
import shutil
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('appdir', type=Path)
args = parser.parse_args()
destination = args.appdir.resolve() / 'usr/lib'
cache = subprocess.check_output(['ldconfig', '-p'], text=True)
glibc = {'libc.so.6', 'libm.so.6', 'libdl.so.2', 'libpthread.so.0',
         'librt.so.1', 'libresolv.so.2', 'libutil.so.1', 'ld-linux-x86-64.so.2'}
libraries = set()
for name in ('libharfbuzz.so.0', 'libfribidi.so.0', 'libasound.so.2'):
    match = re.search(r'^\s*' + re.escape(name) + r'\s+.*?=> (\S+)', cache, re.M)
    if not match:
        raise SystemExit(f'Required runtime library not installed: {name}')
    source = Path(match[1])
    libraries.add(source)
    dependencies = subprocess.check_output(['ldd', str(source)], text=True)
    if 'not found' in dependencies:
        raise SystemExit(f'Unresolved dependencies for {source}:\n{dependencies}')
    libraries.update(Path(p) for p in re.findall(r'=> (/\S+)', dependencies))
for source in sorted(libraries):
    if source.name in glibc:
        continue
    target = destination / source.name
    if target.exists():
        continue
    shutil.copy2(source, target, follow_symlinks=True)
    target.chmod(target.stat().st_mode | 0o200)
    subprocess.run(['patchelf', '--set-rpath', '$ORIGIN', str(target)], check=True)
    print(f'Bundled desktop runtime: {source.name}')
