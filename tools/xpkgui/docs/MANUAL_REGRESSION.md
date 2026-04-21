# xpkgui Manual Regression Checklist

This checklist is for human validation before release. It should be executed on
a fresh x64 build after `tools\xpkgui\smoke_x64.bat` passes.

Do not mark items complete from automation alone. Record the Windows version,
display scale, xpkgui build time, and archive paths used for the run.

## Environment Record

- Windows version:
- Display scale:
- xpkgui build time:
- Test workspace:
- Tester:
- Date:

## Baseline GUI Packages

- [ ] Create a new `core` package, add files, save, close, reopen.
- [ ] Create a new `index` package, add files with default indexes, save,
  close, reopen.
- [ ] Create a new `linux` package, add files under nested paths, save, close,
  reopen.
- [ ] Create a new `win32` package, add files under nested paths, save, close,
  reopen.
- [ ] Verify the file list shows expected columns: name/path, size, packed,
  ratio, method, file type, modified time, ID, hash, attr.
- [ ] Verify archive Properties shows pack type, entry count, sizes, ratio,
  physical size, holes, metadata, and header/table summary.

## Archive Operations

- [ ] Add files with `Add`.
- [ ] Add a directory recursively.
- [ ] Extract selected entries to a chosen folder.
- [ ] Extract all entries to a chosen folder.
- [ ] Verify selected entries.
- [ ] Verify the whole archive.
- [ ] Delete entries and save.
- [ ] Rename entries and save.
- [ ] Replace an entry from an external file.
- [ ] Duplicate an entry.
- [ ] Move and copy path entries between folders.
- [ ] Refresh after externally modifying the archive and confirm the reload
  prompt is understandable.

## Open And Edit

- [ ] Double-click a normal package entry and confirm it opens through the
  system default program.
- [ ] Double-click an inner `.xpk` entry and confirm a new `xpkgui.exe` window
  opens it.
- [ ] Use `Open With...` and confirm the Windows picker appears when needed.
- [ ] Use `Show In Explorer` and confirm Explorer selects the temporary file.
- [ ] Use `Edit Externally` on a text file, modify it, and write it back.
- [ ] Use `Edit Text...` on a small UTF-8 text entry and write it back.
- [ ] Confirm external-edit write-back failure leaves a visible temporary file
  path in the warning message.

## Navigation And View

- [ ] Browse into folders, use Back, Forward, Root, Up, and Go.
- [ ] Toggle Flat View.
- [ ] Use Locate In Tree from Flat View.
- [ ] Filter with a single keyword.
- [ ] Filter with multiple keywords.
- [ ] Filter with `*` and `?` wildcards.
- [ ] Sort each visible column and confirm sort arrows update.
- [ ] Hide and show columns through `View -> Columns...`.
- [ ] Toggle Show Grid Lines.
- [ ] Toggle Full Row Select.
- [ ] Copy visible details and confirm hidden columns are not copied.
- [ ] Export TSV and confirm visible column order is respected.

## xPack-Specific Metadata

- [ ] Set File Type on selected entries.
- [ ] Set `fileIndex` on a single `index` entry and confirm duplicate indexes
  are rejected.
- [ ] Set `platformAttr` on `linux` / `win32` entries.
- [ ] View and edit package metadata.
- [ ] Import, export, and clear package metadata.
- [ ] View and edit `core` Entry InfoExt.
- [ ] Import, export, and clear Entry InfoExt.
- [ ] Change archive settings and confirm pack type / solid / volume warnings
  are shown before saving risky changes.

## Persistence

- [ ] Confirm recent archives persist after restart.
- [ ] Confirm window position and size persist after restart.
- [ ] Confirm column widths persist after restart.
- [ ] Confirm sort column and direction persist after restart.
- [ ] Confirm column visibility persists after restart.
- [ ] Confirm New archive defaults persist after restart.
- [ ] Confirm `Reset UI Preferences...` resets layout preferences but keeps
  recent archives.
- [ ] Confirm old 9-column `xpkgui.ini` layouts migrate safely to the 10-column
  layout.

## Explorer Integration

- [ ] Run `tools\xpkgui\install_x64.bat`.
- [ ] Confirm double-clicking `.xpk` opens `xpkgui.exe`.
- [ ] Confirm `.xpk` right-click menu includes open, extract, verify, and
  properties verbs.
- [ ] Confirm ordinary file right-click menu includes add-to-xPack.
- [ ] Confirm directory right-click menu includes add-to-xPack.
- [ ] Confirm multi-select files can be added.
- [ ] Confirm multi-select directories can be added.
- [ ] Confirm selection containing `.xpk` and ordinary files has the expected
  menu behavior.
- [ ] On Windows 10, confirm classic Explorer menu text and ordering.
- [ ] On Windows 11, confirm the path through "Show more options".
- [ ] Run `tools\xpkgui\uninstall_x64.bat`.
- [ ] Confirm `.xpk` association and shell extension entries are removed or
  reported by the uninstall diagnostics.

## Release Checks

- [ ] Run `tools\xpkgui\check_release_x64.bat`.
- [ ] For final packaging, run `tools\xpkgui\check_release_x64.bat /strict`.
- [ ] Confirm only manifest-approved files are included in the xpkgui release
  package.
- [ ] Confirm `xpkgui.exe /?` shows command-line help.
- [ ] Confirm `xpkgui.exe` starts on a clean machine or VM with no developer
  environment variables.
