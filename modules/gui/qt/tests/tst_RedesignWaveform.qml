// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick
import QtTest
import "../player/qml/redesign"

TestCase {
    name: "JuxtopposedWaveform"
    width: 480; height: 160
    visible: true; when: windowShown
    RedesignPlayerSeekBar {
        id: seek
        x: 20; y: 40; width: 420; height: 28
        enabled: true; mediaStarted: true; playerPosition: .25
        waveformEnabled: true
    }
    SignalSpy { id: seekSpy; target: seek; signalName: "seekRequested" }
    SignalSpy { id: finished; target: seek; signalName: "dragFinished" }
    function init() {
        seek.waveformEnabled = true; seek.mediaStarted = true; seek.enabled = true
        seek.timelineEnabled = false; seek.timelineState = "off"; seek.timelineCues = []
        seek.waveformPeaks = [0, .25, .8, .5]; seek.width = 420
        seek.playerPosition = .25; seekSpy.clear(); finished.clear()
        wait(20)
    }
    function test_missingDataUsesTheNormalSeekbar() {
        verify(seek.waveformActive)
        seek.waveformPeaks = []
        verify(!seek.waveformActive); compare(seek.waveformBars.length, 0)
        seek.waveformPeaks = [0, 0, 0]
        verify(seek.waveformActive); compare(seek.waveformBars.length, 3)
    }
    function test_resizePreservesPeaksAndBoundsVisibleBars() {
        const peaks = []
        for (let i = 0; i < 2048; ++i) peaks.push(i === 1100 ? .95 : .1)
        seek.waveformPeaks = peaks
        verify(seek.waveformBars.length < 100)
        verify(Math.max(...seek.waveformBars) > .9)
        seek.width = 80
        verify(seek.waveformBars.length < 20)
        verify(Math.max(...seek.waveformBars) > .9)
        compare(seek.waveformPeaks.length, 2048)
    }
    function test_waveformDoesNotInterceptSeekingOrKeyboardInput() {
        mouseClick(seek, seek.width * .75, seek.height / 2)
        tryCompare(finished, "count", 1)
        verify(finished.signalArguments[0][1])
        verify(Math.abs(finished.signalArguments[0][0] - .75) < .03)
        verify(seekSpy.count > 0)
        seek.forceActiveFocus(); keyClick(Qt.Key_Right)
        verify(seekSpy.count > 1)
    }
    function test_subtitleTimingKeepsItsOwnInteractionLayer() {
        seek.timelineEnabled = true; seek.timelineState = "ready"
        seek.durationSeconds = 100; seek.timelineCues = [{start: 10, stop: 30}]
        verify(seek.timelineActive); verify(!seek.waveformActive)
        seek.timelineEnabled = false
        verify(seek.waveformActive)
    }
    function test_disablingWaveformRemovesItsBars() {
        verify(findChild(seek, "audioWaveform").visible)
        seek.waveformEnabled = false
        verify(!findChild(seek, "audioWaveform").visible)
        compare(seek.waveformBars.length, 0)
    }
}
