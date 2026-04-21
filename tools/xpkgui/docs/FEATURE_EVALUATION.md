# xpkgui Feature Evaluation

This document records feature-level decisions that depend on xPack core API
semantics. It is intentionally separate from `API_COVERAGE.md`: coverage says
which public API is used, while this file explains why some GUI features are not
implemented yet.

## Index Batch FileIndex Renumbering

Current GUI coverage:

- Single selected `index` entry can be moved to a different `fileIndex`.
- The implementation validates duplicate indexes, extracts the entry to a
  temporary file, adds it under the new `fileIndex`, removes the old entry, and
  saves the package.

Current API gap:

- xPack exposes `xpkIndexAddFile`, `xpkIndexAddData`, `xpkIndexUpdateFile`,
  `xpkIndexUpdateData`, `xpkIndexRemove`, and `xpkIndexSetFlag`.
- xPack does not currently expose a metadata-only API such as
  `xpkIndexRename`, `xpkIndexSetFileIndex`, or a transactional batch reorder
  operation.

Risk assessment:

- A GUI batch renumber command would need to rewrite every affected entry.
- Failure recovery is weak without an xPack-level transaction or dry-run plan.
- Duplicate collision handling becomes complex when source and destination
  ranges overlap.
- Solid and volume packages can make a batch rewrite much more expensive than
  users expect.
- Rewriting entries may accidentally change metadata, packed layout, or
  compression behavior that a metadata-only operation should preserve.

Decision:

- Do not implement `Renumber Indexes...` in xpkgui with the current public API.
- Keep the existing single-entry `Set FileIndex...` command as the safe
  supported workflow.
- If xPack later adds `xpkIndexSetFileIndex`, `xpkIndexRename`, or a
  transactional batch index reorder API, add a GUI command with preview,
  collision diagnostics, and an explicit apply step.

## Linux / Win32 Timestamp Editing

Current GUI coverage:

- `linux` and `win32` package entries display create, modify, and access times
  when the package provides them.
- Existing add/update flows preserve the normal xPack write behavior.

Current API gap:

- xPack exposes timestamp fields through path information.
- xPack currently exposes `xpkPathSetAttr` for path attributes.
- xPack does not currently expose a timestamp setter such as `xpkPathSetTimes`
  or a generic path metadata update API.

Risk assessment:

- Editing timestamps through a temporary extraction and re-add would rewrite
  file data for what should be a metadata-only operation.
- External editor workflows may update the temporary file's filesystem
  timestamps in ways that do not match the archived timestamps the user wants.
- Timezone and UI precision rules must match xPack's stored timestamp format,
  otherwise the GUI can introduce silent timestamp drift.
- Solid and volume packages make rewrite-based timestamp edits expensive and
  harder to cancel safely.

Decision:

- Do not implement timestamp editing in xpkgui with the current public API.
- Continue to display timestamps read from the archive.
- If xPack later adds `xpkPathSetTimes` or a generic path metadata setter, add
  timestamp editing through the entry Properties dialog, with validation and a
  clear distinction between local time display and stored value semantics.

## Large Archive Loading

Current GUI coverage:

- Opening and refreshing an archive enumerates entries through `xpkEach` and
  rebuilds the list view synchronously.
- Long-running add, extract, verify, copy, and data operations already use the
  task progress dialog and cancellation state.
- Filtering is applied to the in-memory view after enumeration.

Risk assessment:

- A very large archive can block the UI while `xpkEach` and list view rebuild
  complete.
- Moving enumeration to a worker thread touches several shared state areas:
  archive handle lifetime, current folder, filter text, selection restore, and
  progress cancellation.
- Partial list population must preserve stable sorting, column visibility, and
  virtual folder grouping semantics.

Decision:

- Do not introduce background archive loading in the first version.
- Keep synchronous open/refresh until there is measured evidence that it is a
  real bottleneck on representative xpk packages.
- If large-archive testing shows visible UI stalls, implement this as a focused
  `Background Archive Loading` feature: worker-side enumeration snapshot,
  UI-thread list replacement, cancellable reload token, and a busy indicator
  that does not mutate the archive while loading.

## Dark Mode

Current GUI coverage:

- xpkgui uses classic Win32 controls and dialog resources.
- The current UI follows the system classic/light control rendering path.

Risk assessment:

- Full dark mode is not just a color table change. It requires consistent
  handling for menus, toolbar, list view, edit controls, status bar, static
  labels, common dialogs, and custom message boxes.
- Partial dark mode would make xpkgui look inconsistent and can hurt
  readability in archive-management workflows.
- Windows dark-mode APIs are version-sensitive and need runtime probing.

Decision:

- Do not implement dark mode for the first xpkgui version.
- Prefer correctness, shell integration, metadata editing, and package
  operation stability first.
- If dark mode becomes a release requirement, implement it as a separate theme
  pass with runtime OS feature detection, control-by-control rendering checks,
  and screenshots for light/dark/high-contrast modes.

## High DPI Layout

Current GUI coverage:

- Dialog resources use dialog units with `Segoe UI 9`, so standard Win32 dialog
  scaling applies.
- The main window enforces a minimum track size to avoid unusable compressed
  layouts.
- The list view has resizable columns and persisted widths.
- The toolbar, path bar, filter bar, list, empty-state text, and status bar are
  laid out from the current client rectangle on `WM_SIZE`.

Risk assessment:

- Real DPI quality cannot be proven by static checks alone. It needs screenshots
  or direct inspection at common Windows scale factors.
- The most likely issues are clipped dialog labels, too-small fixed controls,
  and persisted column widths copied from one scale factor to another.

Decision:

- Keep the current Win32 auto-scaling approach for the first version.
- Treat DPI verification as manual release validation at 100%, 125%, 150%, and
  200% scale.
- If clipping is found, fix individual dialogs/layout constants rather than
  introducing a custom DPI framework prematurely.
