// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick

QtObject {
    id: root
    property var ctx
    property var entries: []
    readonly property string storageKey: "Juxtopposed/library-visits"
    function load() {
        try {
            const saved = JSON.parse(ctx.settingValue(storageKey, "[]"))
            entries = Array.isArray(saved) ? saved.filter(entry => entry && typeof entry.key === "string" && typeof entry.name === "string" && ["folders", "playlists"].includes(entry.kind) && Number.isFinite(entry.count) && entry.count > 0) : []
        } catch (error) { entries = [] }
    }
    Component.onCompleted: load()
    property Connections settingsObserver: Connections {
        target: root.ctx || null
        ignoreUnknownSignals: true
        function onSettingsReloaded() { root.load() }
    }
    function record(kind, key, name, category) {
        if (!key.length || !name.length || !["folders", "playlists"].includes(kind)) return
        const next = entries.filter(entry => entry.kind !== kind || entry.key !== key)
        const previous = entries.find(entry => entry.kind === kind && entry.key === key)
        const entry = {kind: kind, key: key, name: name, count: previous ? previous.count + 1 : 1, last: Date.now()}
        if (kind === "folders") entry.category = category === 1 || category === 2 ? category
            : previous && previous.category === 2 ? 2 : 1
        next.push(entry)
        next.sort((a, b) => b.count - a.count || b.last - a.last)
        entries = next.slice(0, 400)
        ctx.setSettingValue(storageKey, JSON.stringify(entries))
    }
    function ranked(kind, search) {
        const pattern = search.toLocaleLowerCase()
        return entries.filter(entry => entry.kind === kind && entry.name.toLocaleLowerCase().includes(pattern))
            .sort((a, b) => b.count - a.count || (b.last || 0) - (a.last || 0))
    }
    function rename(kind, key, name) {
        if (!name.length) return
        const next = entries.map(entry => entry.kind === kind && entry.key === key
            ? Object.assign({}, entry, {name: name}) : entry)
        entries = next
        ctx.setSettingValue(storageKey, JSON.stringify(entries))
    }
}
