# SPDX-License-Identifier: GPL-2.0-or-later
import contextlib
import io
from pathlib import Path
import runpy
import subprocess
import sys
import unittest
from unittest import mock

SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))


class VerifyTests(unittest.TestCase):
    def test_reported_banner_reaches_playback_checks(self):
        def command(args, **kwargs):
            if '--version' in args:
                output = 'VLC version 4.0.0-beta.4 Otto Chriek (4.0.0-dev-40063-ge93a0c3478)\n'
            elif '--list' in args:
                output = 'qt Qt interface\navcodec FFmpeg decoder\nmedialibrary Media library\n'
            else:
                output = ('using audio decoder module "avcodec"\n'
                          'using interface module "qt"\n'
                          'using video decoder module "avcodec"\n'
                          'using vout window module "qt"\n'
                          'using vout display module "opengl"\n')
            return subprocess.CompletedProcess(args, 0, stdout=output)

        argv = ['verify.py', '/path with spaces/vlc', '--expected-version', '4.0.0-beta.4',
                '--sample', '/sample.mkv', '--gui', '--video']
        with mock.patch.object(sys, 'argv', argv), \
             mock.patch.object(subprocess, 'run', side_effect=command) as run, \
             contextlib.redirect_stdout(io.StringIO()) as output:
            runpy.run_path(str(SCRIPTS / 'verify.py'), run_name='__main__')
        self.assertEqual(run.call_count, 4)
        self.assertIn('playback smoke checks passed', output.getvalue())

    def test_wrong_version_stops_before_module_and_playback_checks(self):
        result = subprocess.CompletedProcess([], 0, stdout='VLC version 4.0.0-beta.3 Otto Chriek\n')
        argv = ['verify.py', '/vlc', '--expected-version', '4.0.0-beta.4']
        with mock.patch.object(sys, 'argv', argv), \
             mock.patch.object(subprocess, 'run', return_value=result) as run, \
             self.assertRaisesRegex(SystemExit, 'expected 4.0.0-beta.4; got 4.0.0-beta.3'):
            runpy.run_path(str(SCRIPTS / 'verify.py'), run_name='__main__')
        self.assertEqual(run.call_count, 1)
