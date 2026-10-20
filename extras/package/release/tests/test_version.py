# SPDX-License-Identifier: GPL-2.0-or-later
import importlib.util
from pathlib import Path
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'version.py'
spec = importlib.util.spec_from_file_location('release_version', SCRIPT)
version = importlib.util.module_from_spec(spec)
spec.loader.exec_module(version)


class VersionTests(unittest.TestCase):
    def test_source_matches_next_release(self):
        current = version.source_version()['version']
        self.assertEqual(version.validate_tag(current)['version'], current)

    def test_supported_tags(self):
        for tag in ('4.0.0', '4.0.0-beta.4', '4.0.0-beta.10', '4.1.0-rc.1', '5.0.0-alpha.2'):
            self.assertEqual(version.parse_version(tag)['version'], tag)
        for tag in ('beta-4', '4.0', '04.0.0', '4.0.0-beta-4', '4.0.0-beta.0', '4.0.0-beta.04', '4.0.0-dev'):
            with self.assertRaises(ValueError, msg=tag):
                version.parse_version(tag)

    def test_mismatched_tag(self):
        with self.assertRaises(ValueError):
            version.validate_tag('999999.0.0-beta.999999')

    def test_mismatched_build_definitions(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / 'meson.build').write_text("project('VLC', version: '4.0.0-beta.4')")
            (root / 'configure.ac').write_text('AC_INIT([vlc], [4.0.0-beta.3])')
            with self.assertRaises(ValueError):
                version.source_version(root)

    def test_macos_bundle_versions(self):
        self.assertEqual(version.bundle_version('4.0.0-beta.4'), '4.0.0b4')
        self.assertEqual(version.bundle_version('4.0.0-rc.1'), '4.0.0fc1')
        self.assertEqual(version.bundle_version('4.0.0'), '4.0.0')
        with self.assertRaises(ValueError):
            version.bundle_version('4.0.0-beta.256')


if __name__ == '__main__':
    unittest.main()
