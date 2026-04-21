# xpkgui Error Handling Review

This document tracks the error-handling rules used by xpkgui. It exists because
not every `return FALSE` should show a message box: many paths are normal user
cancellation or internal guard checks.

## Error Categories

User-facing archive errors should identify the failure category:

- `用户取消`: the user closed a dialog or cancelled a task.
- `格式错误`: the input is not a supported current xpk archive.
- `IO 错误`: file, path, read, write, or open failure.
- `xPack API 状态错误`: core API returned a failure state without a more
  specific category.

Archive open errors include the archive path and explicitly mention when
`invalid xpk package format` may come from an old package or an invalid
temporary inner `.xpk`.

System errors should include copyable diagnostics:

- context
- path, when available
- decimal and hexadecimal error code
- formatted Windows message
- permission hint for access denied, sharing violation, lock violation, or
  write-protect errors

## Reviewed Return Paths

The following high-impact `return FALSE` groups have explicit diagnostics:

- Opening an archive path.
- Saving an archive, including read-only archive files.
- Creating a temporary extraction root.
- Launching a temporary extracted file with the system default program.
- Launching a temporary inner `.xpk` in a new `xpkgui.exe`.
- Launching the external text editor for `Edit Externally`.
- Writing an externally edited temporary file back into the archive.
- Exporting metadata / InfoExt files.
- Clipboard copy failures.

The following groups intentionally do not show error boxes directly:

- Dialog cancellation, including Open, Save As, folder picker, and input boxes.
- Capability guard checks such as “no archive open” or invalid selection.
- Internal helper probes that are immediately followed by a higher-level error.
- Background task cancellation, which is reported by the task dialog result.

## Temporary File Lifecycle

Temporary files used by Open, Open With, Show in Explorer, Duplicate, Set
FileIndex, and Edit Externally are created under:

`%TEMP%\xpkgui-open\...`

Each extraction root includes the archive name, archive path hash, process id,
and a sequence number. The GUI intentionally keeps these files after external
open/edit operations because the external program may still need the file after
xpkgui returns from the operation.

If external edit write-back fails, xpkgui displays the preserved temporary file
path so the user can recover the edited content manually or retry with Replace.

Do not delete these directories immediately after a successful ShellExecute or
Explorer launch.

xpkgui performs conservative stale temp cleanup at startup:

- only `%TEMP%\xpkgui-open\*` child directories are considered
- directories younger than 7 days are kept
- deletion is best-effort and silent
- cleanup never runs immediately after opening or editing a file

This keeps recent external-open/edit files recoverable while preventing
long-term accumulation of stale extraction roots.
