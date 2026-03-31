<div align="center">

# xPack

[![Language: C](https://img.shields.io/badge/language-C-brightgreen.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platforms](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-blue.svg)](#)
[![Focus](https://img.shields.io/badge/focus-runtime%20package%20library-orange.svg)](#)

**A cross-platform C package format and packaging library for dynamic updates and runtime asset delivery**

English | [简体中文](README.md)

</div>

**Documentation Entry Points:** [Design Spec](docs/design.md) | [Implementation Spec](docs/spec.md) | [Development Guide](docs/development.md) | [Testing Guide](docs/testing.md) | [Coverage Matrix](docs/test_matrix.md) | [Windows Status](docs/windows_status.md) | [Stress Guide](docs/stress.md)


## Positioning

**xPack is not a general-purpose serialization library, and it is not a cold-archive format built only to maximize compression ratio.**

xPack is intended to be a C infrastructure library for runtime asset packages:

- It primarily targets game asset packs, micro-client content delivery, and runtime resource loading
- It supports add/update/remove/query workflows inside a package instead of assuming a one-shot offline archive
- It balances runtime read performance, dynamic write convenience, and space reclamation in an engineering-oriented way
- It provides unified position-based, index-based, and path-based access models
- It is designed with Windows and Linux path semantics, read-only layouts, volume layouts, and large-file boundaries in mind

Its core stance is very explicit:

- **dynamic updates matter more than extreme compression**
- **`save` commits consistency, while `build` performs full reconstruction and compaction**
- **it is primarily a runtime asset package line, and only secondarily an archive format**


## What xPack Already Provides

xPack is no longer just a loose set of APIs.  
From a user-facing perspective, it already forms a fairly complete package-management line:

### 1. Four unified package access models

xPack supports four package types, but they all share one underlying format model rather than four unrelated implementations:

- `Core`
  - access by sequential position
  - suitable for the most direct and lightweight package workflow
- `Index`
  - access by integer index
  - suitable for resource IDs, config IDs, table IDs, and similar use cases
- `Linux`
  - access by path
  - case-sensitive
  - preserves Linux-style path semantics
- `Win32`
  - access by path
  - case-insensitive
  - performs Win32-style path normalization automatically

That means one library can cover:

- simple position-based asset packages
- ID-driven index packages
- path-driven content packages
- cross-platform distribution that must preserve Linux and Windows path behavior


### 2. A write model built for dynamic package changes

xPack does not center its design around rewriting the whole package every time a file changes:

- supports `add / update / remove / rename / attr`
- supports package-level `meta`
- supports per-entry `infoExt`
- supports both `buffered` and `immediate` write policies
- supports readonly open, failure rollback, package replacement, and rebuild temp paths

The most important behavioral boundary is:

- `xpkSave`
  - commits current changes
  - returns the package to a consistent state
  - **does not actively reclaim old holes**
- `xpkBuild`
  - reconstructs the entire package
  - reclaims old holes
  - reorganizes layout
  - also serves as the formal rebuild step for layout transitions such as `normal / solid / volume`

This makes xPack a better fit for projects that repeatedly update asset packages rather than projects that only need one-shot archival.


### 3. Compression, solid layout, and volume support

xPack currently supports:

- `STORE`
- `LZ4`
- `LZ4HC`
- `ZSTD`
- `LZMA2`

Compression is not treated as an afterthought. Layout capability is part of the formal model:

- `normal mode`
  - standard per-file compression
  - suited to the main dynamic read/write path
- `solid mode`
  - compresses the data area as a whole
  - friendly to distribution and read-side efficiency
  - once data enters solid form, file data can no longer be appended or updated
- `volume mode`
  - maps a single logical package into sequential volumes through `volumeSize`
  - useful for storage environments with single-file size limits

`solid` is not a separate package type; it is a data-area layout mode.  
`volume` is not a secondary index system; it is a storage view over one logical package.


### 4. A runtime-oriented read, export, and verification line

xPack is not just able to write data. It also provides a fairly complete read/export/verification line:

- `xpkReadToMemory / xpkReadToFile`
- `xpkVerify / xpkVerifyAll`
- `xpkEach / xpkEachMatch`
- `xpkStatGet`
- `xpkLastError / xpkLastErrorMessage`

The current engineering focus has already covered:

- direct read and export paths for normal entries
- slice-based read and verification paths for `solid` entries
- mapped open and decode paths for non-volume package tails
- regression coverage for `4GB+` file positioning and large logical-offset volume I/O
- low-peak-memory paths for large-file input, export, and rebuild chains

In other words, xPack is not just trying to make the API surface work.  
It is trying to keep these main paths under control even under large files and complex layouts:

- open
- read
- verify
- export
- rebuild


### 5. Distribution and integration-oriented engineering forms

xPack currently offers three clear integration forms:

- `xpack.c + xpack.h`
  - the main implementation entry and public header
- `singlehead/xpack.h`
  - suitable for single-header distribution or embedded integration
- `DLL / SO`
  - the repository already provides Windows and Linux build scripts

At the engineering level, the project currently uses:

- a single implementation entry
- generated public headers
- a separate test entry
- vendor compression libraries built independently
- `xrt` as the foundational runtime dependency


## Why xPack Is Not a Generic Archive Library

Many packaging formats prioritize:

- smaller output
- compatibility with historical tooling
- one-time packaging with little or no later mutation

xPack prioritizes something else:

- **avoiding whole-package rewrites during dynamic asset updates**
- **keeping runtime read and verification paths explicit**
- **maintaining a stable boundary between `save` and `build`**
- **making path packages, index packages, and basic packages share one underlying line**

So if your goal is:

- a ZIP replacement
- long-term cold storage
- broad interoperability with external generic unpacking tools

then xPack is not designed primarily for that.

But if your goal is:

- game resource packages
- client-side runtime content bundles
- micro-client download-and-land workflows
- a package format that supports repeated updates, deletions, lookups, and rebuilds

then xPack is much closer to the actual shape of that problem.


## Core Characteristics

### 1. Dynamic updates come first

xPack treats package mutation as a first-class scenario, not an extra feature.

### 2. The runtime path is explicit

Open, read, export, verify, and rebuild are part of one engineering line rather than a set of disconnected helper functions.

### 3. Unified instead of fragmented

`Core / Index / Linux / Win32` share one format model instead of relying on four separately maintained storage backends.

### 4. Compression and layout are formal capabilities

`STORE / LZ4 / LZ4HC / ZSTD / LZMA2`, `solid`, and `volume` are part of the public capability model rather than bolt-on features.

### 5. Large-file and low-peak-memory oriented

The current implementation has already spent sustained effort on large-file input, volume I/O, compressed export paths, and slice-based verification for `solid`.


## Signals of Maturity

xPack already shows several concrete engineering signals of maturity:

- On Windows, the `open / close / save / build` main line is already stable
- All four package types, `Core / Index / Linux / Win32`, have formal regression coverage
- `read / add / update / remove / rename / attr / meta / verify` already form a complete line
- `solid / volume / buffered / immediate` all have cross-module integration coverage
- `4GB+` positioning and large logical-offset volume I/O have dedicated verification
- Windows already has a standalone stress entry; Debian 13 main regression has passed, and Linux stress work is ongoing

Current boundaries should also be stated clearly:

- longer release-grade continuous stress coverage is still being expanded
- near-`4GB` volume-limit cases and larger logical files still need more real pressure validation
- single-file data blocks and codec behavior remain constrained by the current block model


## Typical Use Cases

- game client resource packs
- staged content packages for micro-clients or launchers
- runtime asset bundles that need path-based or ID-based lookup
- hot-update, patching, and content rebuild pipelines
- resource distribution that must keep Windows and Linux path semantics under control


## Design Principles

The current xPack design line can be summarized by a few explicit principles:

- fixed `64B` file header
- unified `32B` base entry + `infoExt` extension model
- `save` handles consistency, `build` handles reconstruction and compaction
- `solid` is a read-only data-area mode, not a different format family
- `volume` is a storage view, not an extra segment-table system
- the spec has priority over implementation, and behavior has priority over historical compatibility


## Current State

xPack is no longer a small proof-of-concept experiment.  
It already has a complete main line, and many core scenarios can now be approached as formal engineering use cases.

The more accurate description today is:

- the core capability line is in place
- the Windows side is usable for controlled production scenarios
- Linux main regression has passed and stress coverage is still being expanded
- the project is still unifying documentation and pushing further on larger boundaries and longer pressure validation

In short, xPack is not trying to stop at “usable enough”. It is still pushing to make this asset-package infrastructure line more solid.


## Quick Start

Below is a minimal `Win32 Path` package example.  
It creates a package, writes a path entry, saves it, reopens it, verifies it, and reads the content back:

```c
#include <stdio.h>
#include <string.h>

#include "xpack.h"

int main(void)
{
	xpkObject pkg = NULL;
	xpkOpenOptions openOpt = {0};
	xpkWriteOptions writeOpt = {0};
	const char* text = "hello xpack";
	void* data = NULL;
	uint64_t dataSize = 0;

	openOpt.createIfMissing = 1;

	writeOpt.compLevel = 6;
	writeOpt.writePolicy = XPK_WRITE_BUFFERED;

	pkg = xpkOpen("assets.xpk", &openOpt);
	if (pkg == NULL) {
		return 1;
	}

	if (xpkSetPackType(pkg, XPK_PACK_WIN32) != XPK_OK) {
		goto fail;
	}
	if (xpkPathAddData(pkg, "assets/hello.txt", text, (uint64_t)strlen(text), &writeOpt) != XPK_OK) {
		goto fail;
	}
	if (xpkSave(pkg) != XPK_OK) {
		goto fail;
	}
	if (xpkClose(pkg) != XPK_OK) {
		return 2;
	}
	pkg = NULL;

	pkg = xpkOpen("assets.xpk", NULL);
	if (pkg == NULL) {
		return 3;
	}
	if (xpkVerifyAll(pkg) != XPK_OK) {
		goto fail;
	}

	data = xpkPathReadToMemory(pkg, "assets/hello.txt", &dataSize);
	if (data == NULL) {
		goto fail;
	}

	fwrite(data, 1, (size_t)dataSize, stdout);
	putchar('\n');

	xpkFree(data);
	data = NULL;

	return xpkClose(pkg) == XPK_OK ? 0 : 4;

fail:
	if (data != NULL) {
		xpkFree(data);
	}
	if (pkg != NULL) {
		fprintf(stderr, "xpack error: %s\n", xpkLastErrorMessage(pkg));
		xpkClose(pkg);
	}
	return 5;
}
```

If you need:

- position-based access: use `xpkAddData / xpkReadToMemory`
- integer-index access: set `XPK_PACK_INDEX` first, then use `xpkIndex*`
- Linux path access: set `XPK_PACK_LINUX` first, then use `xpkPath*`


## Build and Validation

The repository already provides ready-to-run build scripts.

Common Windows commands:

```bat
build_header.bat
build_GCC_DLL_x64.bat
build_GCC_TEST_x64.bat
release\x64\xpack_test.exe
```

Common Linux commands:

```sh
./build.sh
./build_test.sh
release/x64/xpack_test all
```

For standalone stress entry points:

```bat
build_GCC_STRESS_x64.bat
```

```sh
./build_stress.sh
```


## Single-Header Distribution

If you prefer a single-header distribution or embedded integration flow, generate:

```bat
build_single_head.bat
```

or:

```sh
./build_single_head.sh
```

The generated output is:

```text
singlehead/xpack.h
```

Typical usage looks like this:

```c
#define XRT_IMPLEMENTATION
#include "xrt.h"

#define XPACK_IMPLEMENTATION
#include "xpack.h"
```

Note that the single-header variant still depends on external:

- `xrt.h`
- `lz4 / lz4hc`
- `zstd`
- `lzma`

So the single-header flow solves **xPack’s own distribution form**, not the problem of turning every third-party dependency into one final header.


## Documentation

- [Design Spec](docs/design.md): format design, product positioning, mode boundaries, and public capability model
- [Implementation Spec](docs/spec.md): project structure, public-header generation, build organization, and error model
- [Development Guide](docs/development.md): directory responsibilities, build scripts, recommended workflow, and current boundaries
- [Testing Guide](docs/testing.md): test structure, helper conventions, filters, and execution flow
- [Coverage Matrix](docs/test_matrix.md): feature-to-test mapping index
- [Windows Status](docs/windows_status.md): current Windows capability conclusion and remaining boundaries
- [Stress Guide](docs/stress.md): Windows/Linux stress entry points, logs, and history records


## Dependencies

xPack currently depends on and integrates these building blocks:

- `xrt`
- `LZ4 / LZ4HC`
- `Zstandard`
- `LZMA SDK`

`xrt` provides foundational runtime support, while the compression libraries back the `STORE / LZ4 / LZ4HC / ZSTD / LZMA2` codec line.


## Final Words

If you are not looking for a tool that only does offline packaging,  
but instead want a **C asset-package infrastructure line** that can support:

- dynamic package mutation
- runtime reads
- position-, index-, and path-based access
- `solid / volume / recompress / rebuild`
- Windows and Linux resource distribution

then xPack is moving in exactly that direction.
