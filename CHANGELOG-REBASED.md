# KeePassXC Rebased Changelog

Changes of KeePassXC Rebased compared to [KeePassXC](https://github.com/keepassxreboot/keepassxc).
Changes of KeePassXC itself are listed in [CHANGELOG.md](CHANGELOG.md); each release below names
the KeePassXC version it is based on.

## Unreleased (based on KeePassXC 2.8.0 development)

### New Features
- Search in all keyboard layouts: a search word also matches as if it was typed in any other
  keyboard layout enabled in the system, e.g. `ецшееук` finds `twitter` (macOS, Windows; enabled
  by default, minimum word length configurable)
- Prompt for quick unlock (Touch ID / Windows Hello) right away when the quick unlock screen of a
  database appears in the main window, once each time it appears (enabled by default)

### Changes
- Renamed to KeePassXC Rebased with its own macOS bundle identifier, launch agent and, on macOS
  and Windows, its own settings
- Check for updates in the KeePassXC Rebased releases
- The fork and the official KeePassXC never run at the same time: the fork refuses to start while
  the official one is running, and an official KeePassXC started later hands over to the fork
- macOS: the app no longer freezes while the Touch ID prompt is shown
