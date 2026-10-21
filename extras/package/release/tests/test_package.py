# SPDX-License-Identifier: GPL-2.0-or-later
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock
import zipfile

SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))
spec = importlib.util.spec_from_file_location('release_package', SCRIPTS / 'package.py')
package = importlib.util.module_from_spec(spec)
spec.loader.exec_module(package)


class PackageTests(unittest.TestCase):
    def test_windows_archive_and_metadata(self):
        tag = package.validate_tag(None)['version']
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            stage = root / 'stage'
            stage.mkdir()
            for name in ('vlc.exe', 'libvlc.dll', 'libvlccore.dll'):
                (stage / name).write_bytes(b'fixture')
            (stage / 'plugins').mkdir()
            output = root / 'output'

            def tool(*args):
                if args[0] == 'git':
                    return 'a' * 40
                if args[0] == 'uname':
                    return 'x86_64'
                if args[0] == 'x86_64-w64-mingw32-objdump':
                    return ''  # Synthetic fixtures have no imported runtime DLLs.
                raise AssertionError(args)

            argv = ['package.py', 'windows', str(stage), '--output', str(output), '--tag', tag]
            with mock.patch.object(sys, 'argv', argv), mock.patch.object(package, 'run', side_effect=tool):
                package.main()
            archive = output / f'vlc-juxtopposed-{tag}-win64.zip'
            self.assertTrue(archive.is_file())
            with zipfile.ZipFile(archive) as bundled:
                self.assertIn('vlc-win64/portable/', bundled.namelist())
                self.assertEqual(bundled.read('vlc-win64/SOURCE_COMMIT.txt'), ('a' * 40 + '\n').encode())
            metadata = json.loads((output / 'windows-build.json').read_text())
            self.assertEqual(metadata['version'], tag)
            self.assertEqual(metadata['asset_name'], archive.name)
            self.assertEqual(metadata['size'], archive.stat().st_size)
            digest = hashlib.sha256(archive.read_bytes()).hexdigest()
            self.assertEqual(metadata['sha256'], digest)
            self.assertEqual(archive.with_suffix('.zip.sha256').read_text(), f'{digest}  {archive.name}\n')

    def test_wrong_version_rejected_before_packaging(self):
        with tempfile.TemporaryDirectory() as temp:
            output = Path(temp) / 'output'
            argv = ['package.py', 'windows', temp, '--output', str(output), '--tag', 'beta-4']
            with mock.patch.object(sys, 'argv', argv), mock.patch('sys.stderr'), self.assertRaises(SystemExit):
                package.main()
            self.assertFalse(output.exists())
