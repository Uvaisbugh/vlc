#!/usr/bin/env python3
"""Validate release tags against both build systems before building or packaging."""
import argparse
import os
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[3]
PATTERN = re.compile(r'(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(?:-(alpha|beta|rc)\.([1-9][0-9]*))?')


def parse_version(value):
    match = PATTERN.fullmatch(value)
    if not match:
        raise ValueError('Expected MAJOR.MINOR.PATCH or MAJOR.MINOR.PATCH-beta.N (also alpha.N / rc.N)')
    major, minor, patch = map(int, match.group(1, 2, 3))
    stage, number = match.group(4, 5)
    return dict(version=value, major=major, minor=minor, patch=patch,
                channel=stage or 'stable', number=int(number) if number else None,
                prerelease=f'{stage}.{number}' if stage else '')


def source_version(root=ROOT):
    meson = re.search(r"version:\s*'([^']+)'", (root / 'meson.build').read_text())
    configure = (root / 'configure.ac').read_text()
    autoconf = re.search(r'AC_INIT\(\[vlc\],\s*\[([^]]+)\]\)', configure)
    if not meson or not autoconf or meson[1] != autoconf[1]:
        raise ValueError('Meson and Autoconf application versions must agree')
    info = parse_version(meson[1])
    for name, expected in (('MAJOR', info['major']), ('MINOR', info['minor']),
                           ('REVISION', info['patch']), ('EXTRA', 0), ('DEV', info['prerelease'])):
        value = re.search(rf'^VERSION_{name}=(.*)$', configure, re.MULTILINE)
        if not value or value[1].strip("\"'") != str(expected):
            raise ValueError(f'Autoconf VERSION_{name} must match {info["version"]}')
    return info


def validate_tag(tag, root=ROOT):
    info = source_version(root)
    if tag is not None and tag != info['version']:
        raise ValueError(f'Release tag {tag!r} does not match compiled version {info["version"]!r}')
    return info


def bundle_version(tag):
    info = parse_version(tag)
    core = f'{info["major"]}.{info["minor"]}.{info["patch"]}'
    if info['channel'] == 'stable':
        return core
    if info['number'] > 255:
        raise ValueError('macOS bundle prerelease numbers must be between 1 and 255')
    return core + {'alpha': 'a', 'beta': 'b', 'rc': 'fc'}[info['channel']] + str(info['number'])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('tag', nargs='?', default=os.environ.get('RELEASE_TAG') or None)
    args = parser.parse_args()
    try:
        print(validate_tag(args.tag)['version'])
    except ValueError as error:
        parser.error(str(error))


if __name__ == '__main__':
    main()
