# SPDX-License-Identifier: GPL-2.0-or-later
import hashlib
import importlib.util
import io
from pathlib import Path
import tarfile
import tempfile
import unittest
import zipfile

spec = importlib.util.spec_from_file_location('preview', Path(__file__).resolve().parents[1] / 'package-preview.py')
preview = importlib.util.module_from_spec(spec)
spec.loader.exec_module(preview)


def run_record():
    return dict(conclusion='success', headBranch='master', workflowName='Windows x64 VLC build',
                headSha='a' * 40, url='https://github.com/Uvaisbugh/vlc/actions/runs/1',
                jobs=[dict(name=name, conclusion='success') for name in
                      ('packaging-tests', 'windows-x64', 'windows-smoke', 'native-qt-tests / native-qt-tests')])


class PreviewTests(unittest.TestCase):
    def test_failed_missing_or_wrong_source_validation_is_rejected(self):
        for field, value in (('conclusion', 'failure'), ('headBranch', 'other'),
                             ('workflowName', 'Other workflow'), ('headSha', 'not-a-sha'), ('jobs', [])):
            with self.subTest(field=field):
                run = run_record()
                run[field] = value
                with self.assertRaises(ValueError):
                    preview.verified_run(run)

    def fixture(self, root, extra=None):
        archive = root / 'runtime.tar.gz'
        with tarfile.open(archive, 'w:gz') as bundle:
            for path in ('vlc.exe', 'libvlc.dll', 'libvlccore.dll', 'vulkan-1.dll', 'COPYING', 'COPYING.LIB',
                         'plugins/codec/decoder.dll', 'plugins/plugins.dat', 'licenses/Inter/LICENSE',
                         'licenses/design/CREDITS.md', 'licenses/vulkan-loader/LICENSE'):
                info = tarfile.TarInfo('vlc-win64/' + path)
                info.size = 7
                bundle.addfile(info, io.BytesIO(b'fixture'))
            if extra:
                info = tarfile.TarInfo(extra)
                info.size = 7
                bundle.addfile(info, io.BytesIO(b'fixture'))
        checksums = root / 'SHA256SUMS'
        checksums.write_text(hashlib.sha256(archive.read_bytes()).hexdigest() + '  runtime.tar.gz\n')
        return archive, checksums

    def test_portable_archive_has_licenses_identity_and_empty_profile(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive, sums = self.fixture(root)
            output = root / 'output'
            preview.stage_runtime(archive, sums, output, run_record(), 'b' * 40, 'reimagined-preview-2026.10.21')
            with zipfile.ZipFile(output / 'vlc-reimagined-windows-x64-portable.zip') as bundle:
                names = bundle.namelist()
                self.assertIn('VLC Reimagined/portable/', names)
                self.assertIn('VLC Reimagined/DESIGN_CREDITS.md', names)
                self.assertIn('VLC Reimagined/COPYING.LIB', names)
                self.assertEqual(bundle.read('VLC Reimagined/SOURCE_COMMIT.txt'), b'a' * 40 + b'\n')
                self.assertNotIn('VLC Reimagined/plugins/plugins.dat', names)

    def test_corruption_and_profile_data_are_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive, sums = self.fixture(root, 'vlc-win64/portable/private-settings')
            with self.assertRaisesRegex(ValueError, 'populated portable profile'):
                preview.stage_runtime(archive, sums, root / 'output', run_record(), 'b' * 40, 'tag')
            sums.write_text('0' * 64)
            with self.assertRaisesRegex(ValueError, 'checksum mismatch'):
                preview.stage_runtime(archive, sums, root / 'corrupted', run_record(), 'b' * 40, 'tag')

    def test_archive_cannot_escape_extraction_root(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive, sums = self.fixture(root, '../escaped')
            with self.assertRaises(tarfile.FilterError):
                preview.stage_runtime(archive, sums, root / 'output', run_record(), 'b' * 40, 'tag')
