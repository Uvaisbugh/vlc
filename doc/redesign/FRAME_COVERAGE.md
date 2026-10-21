# Figma screen coverage map

Observed in the signed-in Figma layer panel on 2026-10-20. This is a map of
identified screen families to integrated native source and tests, not a complete
per-node inventory or a claim of passing rendered parity. Repeated frame names
must still be distinguished by node, theme, and state in the final audit.

All paths below are under `modules/gui/qt/`. Components are in
`player/qml/redesign/` unless stated otherwise; tests are under `tests/`.

| Observed Figma frame/component | Native implementation | Focused checks / remaining work |
| --- | --- | --- |
| Player (`247:445`), topbar, Controls, play/pause | `player/qml/redesign/RedesignPlayer.qml`, `RedesignPlayerView.qml`, `RedesignPlayerChrome.qml`, `RedesignButton.qml` | Player/chrome/layout tests; native full view and video playback pending |
| Player - Light (`2425:13624`, 1920 x 1080) | `RedesignTheme.qml`, same player view/chrome | Theme tests; rendered light frame audit pending |
| Player - hide UI | `RedesignPlayerView.qml` | Cursor/chrome/appearance tests; native fullscreen and hotkey audit pending |
| Player - Music (two observed frames) | `RedesignAudioArtwork.qml`, `RedesignPlayerSeekBar.qml` | Waveform tests; native artwork provider and rendered frames pending |
| Player - customize (multiple observed variants) | `RedesignPlayerLayout.qml`, player chrome/customization components | Customization/editor/layout/preset tests; per-node variant audit pending |
| sidebar, playlist/media items, sorting/view options, context menus | `RedesignLibraryView.qml`, `RedesignLibraryNavigation.qml`, `RedesignBreadcrumbs.qml`, `RedesignQueuePanel.qml` | Library, queue, action-menu tests; native library integration and visual audit pending |
| Miniplayer (`970:3091`) | `player/qml/MiniPlayer.qml`, redesign library/player integration | Component identified; reference variants and native transitions pending |
| Onboarding (multiple observed frames) | `RedesignOnboardingView.qml`, `RedesignOnboardingWindow.qml` | Native onboarding tests and step/theme audit pending |
| new playlist (`6259:14954`) | `RedesignPlaylistDialog.qml`, `RedesignDialogSurface.qml` | Measured 713 x 589, radius 14, dark #191919 at 95%; geometry/input/surface checks pass; native render pending |
| Open network stream | `RedesignNetworkDialog.qml`, `RedesignNetworkStreams.qml` | Network tests; native success/failure behavior and visual audit pending |
| Keyboard Shortcuts | `RedesignShortcutEditor.qml`, `RedesignShortcuts.qml` | Shortcut-editor tests; native persistence and visual audit pending |
| Add Subtitles to Playlist/Folder (four observed frames) | `RedesignTrackAssignmentDialog.qml`, `RedesignTrackAssignments.qml`, `RedesignPlaylistTrackTarget.qml` | Track assignment/default tests; each overlay variant and native loading pending |
| Settings - global (`3046:9282`, `999:5047`) | `RedesignSettingsDialog.qml`, `RedesignSettings.qml`, `RedesignPreferencesCatalog.qml` | Measured 1048 x 865, radius 14, #0e0e0e at 95%; surface/resize tests pass; content/frame audit pending |
| Settings - interface, privacy, playback | `RedesignPreferencesCatalog.qml`, `RedesignPlaybackSettings.qml` | Preferences/playback/appearance tests; per-frame audit pending |
| Settings - video, Video - Colors/Crop/Essential | `RedesignVideoSettings.qml` | Video settings tests; native effects and each visual state pending |
| Settings - audio, Audio - Equalizer/Compressor/Spatializer/Stereo Widener | `RedesignAudioSettings.qml`, `RedesignEqualizer.qml`, `RedesignSpatializer.qml` | Audio/equalizer/spatializer tests; native effects and visual audit pending |
| Settings - subtitles | `RedesignSubtitleSettings.qml`, `RedesignTrackDefaults.qml` | Track/default/preferences tests; native subtitle rendering and visual audit pending |
| Settings - controls/library/network/performance/recording | `RedesignPreferencesCatalog.qml`, `RedesignPreferencesWindow.qml` | Preferences tests; platform capability and visual audit pending |
| Settings - extensions | `RedesignAddons.qml` | Add-on metadata/action tests; native installation and visual audit pending |

Reusable theme, button, menu, combo box, tooltip, slider, icon, and backdrop
components exist in the integration. Their presence does not establish that
every Figma component variant has been audited. Font loading, metadata updates,
breadcrumb measurement, and measured dark playlist/settings surface opacity were
corrected during this fork's validation.
