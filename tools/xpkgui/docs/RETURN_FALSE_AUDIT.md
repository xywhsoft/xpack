# xpkgui Return FALSE Audit

This audit records how `return FALSE` paths are classified. It is a living
checklist for future changes; it does not require every helper-level false
return to display a message box.

## Classification

- User cancellation: do not show an error. Examples include file pickers,
  input dialogs, task cancellation, and "No" answers to confirmation prompts.
- Capability guard: show an informational message only at command entry points.
  Lower-level helpers can return `FALSE` silently when the caller already
  checked capability.
- Internal probe: return `FALSE` silently when probing availability, decoding,
  filtering, path existence, or optional metadata.
- Recoverable validation: show an informational message at the point where the
  user can correct input.
- System failure: show `GuiShowSystemErrorDetail` or `GuiShowSystemError`.
- xPack failure: show `GuiShowArchiveError` or `GuiShowArchiveErrorPath`.

## Reviewed Modules

- `common.c`: settings, dialogs, clipboard, path helpers, system diagnostics,
  stale temp cleanup.
- `window.c`: command dispatch, command-line launch handling, toolbar/menu
  enable state.
- `dialogs.c`: normal modal dialog cancellation and validation.
- `archive.c`: open/save, enumerate, add, extract, verify, rename, delete,
  duplicate, replace, metadata, InfoExt, external open/edit, temp extraction,
  and task worker paths.
- `shell/xpkgui_shext.c`: Explorer selection loading and verb command launch.

## Required Diagnostics

The following failure paths must have direct user-facing diagnostics:

- opening or creating an archive
- saving or rebuilding an archive
- reading archive entries
- creating temporary extraction directories
- launching external programs for extracted files
- write-back after external edit
- importing/exporting metadata and InfoExt
- clipboard write failures
- shell extension registration/unregistration failures
- read-only, access-denied, sharing, lock, or write-protect failures

## Accepted Silent Returns

The following false returns are intentional:

- user cancelled a file dialog, input dialog, confirmation dialog, or task
- command is disabled by selection/archive state
- an optional text decode probe failed and a binary preview fallback is used
- a path existence probe returns no match
- a cleanup attempt failed because a file is still in use
- a helper failed after its caller already emitted the diagnostic

## Maintenance Rule

When adding new `return FALSE` paths, classify the branch before committing it.
If the branch is not user cancellation, capability guard, internal probe, or
caller-reported failure, add a diagnostic at the closest useful user action.
