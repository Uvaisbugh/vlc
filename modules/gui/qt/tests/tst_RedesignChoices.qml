// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick
import QtTest
import "../player/qml/redesign"

TestCase {
    id: testCase
    name: "JuxtopposedChoices"
    width: 700; height: 1080
    visible: true; when: windowShown
    Component {
        id: factory
        RedesignComboBox {
            x: 20; y: 650; width: 362; implicitHeight: 46
            choiceHeight: 46; showSelectionMark: true
            model: Array.from({length: 19}, (_, i) => "Preset " + i)
        }
    }
    property var combo
    function init() {
        failOnWarning(/.?/)
        combo = createTemporaryObject(factory, testCase)
        verify(combo)
    }
    function cleanup() { combo.popup.close() }
    function open() {
        mouseClick(combo)
        tryCompare(combo.popup, "opened", true)
    }
    function test_allPresetsUseAvailableWindowHeight() {
        open()
        compare(combo.popup.height, 19 * 46)
        const origin = combo.popup.contentItem.mapToItem(testCase, 0, 0)
        verify(origin.y >= 12)
        verify(origin.y + combo.popup.height <= height - 12)
        keyClick(Qt.Key_End)
        tryCompare(combo, "highlightedIndex", 18)
        compare(combo.currentIndex, 0)
        keyClick(Qt.Key_Return)
        tryCompare(combo.popup, "visible", false)
        compare(combo.currentIndex, 18)
        open()
        compare(combo.highlightedIndex, 18)
        keyClick(Qt.Key_Escape)
        compare(combo.currentIndex, 18)
    }
    function test_smallWindowKeepsLastPresetReachable() {
        const window = testCase.Window.window
        const originalHeight = window.height
        window.height = 400
        combo.y = 100
        open()
        compare(combo.popup.height, 376)
        keyClick(Qt.Key_End)
        tryCompare(combo, "highlightedIndex", 18)
        const list = combo.popup.contentItem
        tryVerify(() => list.itemAtIndex(18) !== null)
        tryVerify(() => list.itemAtIndex(18).y - list.contentY >= 0)
        tryVerify(() => list.itemAtIndex(18).y - list.contentY + 46 <= list.height)
        keyClick(Qt.Key_Return)
        compare(combo.currentIndex, 18)
        window.height = originalHeight
    }
    function test_customCurveRemainsVisibleBesideNamedPreviews() {
        combo.model = [
            {label: "Flat", value: "flat", curve: [0, 0, 0]},
            {label: "Custom", value: "custom", curve: [8, 0, -8]}
        ]
        combo.textRole = "label"
        combo.showCurvePreviews = true
        open()
        const list = combo.popup.contentItem
        tryVerify(() => list.itemAtIndex(1) !== null)
        const custom = list.itemAtIndex(1)
        verify(findChild(custom, "choiceCurvePreview").visible)
        verify(!findChild(custom, "choicePresetPreview").visible)
        const flat = list.itemAtIndex(0)
        verify(findChild(flat, "choicePresetPreview").visible)
        verify(!findChild(flat, "choiceCurvePreview").visible)
    }
}
