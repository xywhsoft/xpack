# Version Evolution

<cite>
**Referenced Files in This Document**
- [xpack.h](file://src/xpack.h)
- [xpack.c](file://src/xpack.c)
- [xpack_compress.c](file://src/xpack_compress.c)
- [xpack_core.c](file://src/xpack_core.c)
- [xpack_index.c](file://src/xpack_index.c)
- [xpack_volume.c](file://src/xpack_volume.c)
- [xpack_internal.h](file://src/xpack_internal.h)
- [zstd.h](file://lib/zstd/zstd.h)
- [zstddeclib.c](file://lib/zstd/zstddeclib.c)
- [xrt.h](file://lib/xrt/xrt.h)
- [spec.md](file://docs/spec.md)
- [design.md](file://docs/design.md)
- [xpkgui.c](file://tools/xpkgui/xpkgui.c)
</cite>

## Table of Contents
1. [Introduction](#introduction)
2. [Project Structure](#project-structure)
3. [Core Components](#core-components)
4. [Architecture Overview](#architecture-overview)
5. [Detailed Component Analysis](#detailed-component-analysis)
6. [Dependency Analysis](#dependency-analysis)
7. [Performance Considerations](#performance-considerations)
8. [Troubleshooting Guide](#troubleshooting-guide)
9. [Conclusion](#conclusion)
10. [Appendices](#appendices)

## Introduction
This document traces the evolution of the xPack library from Ver5 to Ver7, focusing on the major milestones that shaped its current architecture. It explains the transition from FreeBASIC to a C implementation, the addition of four package types, the introduction of a unified compression level system (0–15), the integration of the xrt library, and the adoption of bit-field structures. It also documents the revolutionary changes in Ver7, including the shift to a pure ZSTD scheme, the compression level system, integration of the xrt library, and adoption of bit-field structures. The rationale behind each change is explained, along with how they improved performance, usability, and maintainability. Migration guidance is provided for developers upgrading from previous versions, highlighting breaking changes and new capabilities.

## Project Structure
The xPack Ver7 codebase is organized around a modular C architecture with clear separation of concerns:
- Public API and data structures are defined in the public header.
- Implementation is split across core modules for different package modes and features.
- Compression routing integrates external libraries (LZ4, ZSTD, LZMA2).
- The xrt library provides cross-platform file and runtime services.
- Tools demonstrate real-world usage and integration.

```mermaid
graph TB
subgraph "Public API"
H["xpack.h"]
end
subgraph "Core Implementation"
C["xpack.c"]
CORE["xpack_core.c"]
INDEX["xpack_index.c"]
COMP["xpack_compress.c"]
VOL["xpack_volume.c"]
INT["xpack_internal.h"]
end
subgraph "External Libraries"
ZSTD["lib/zstd/*.h"]
XRT["lib/xrt/xrt.h"]
end
subgraph "Tools"
GUI["tools/xpkgui/xpkgui.c"]
end
H --> C
H --> CORE
H --> INDEX
H --> COMP
H --> VOL
H --> INT
C --> ZSTD
CORE --> ZSTD
INDEX --> ZSTD
COMP --> ZSTD
C --> XRT
CORE --> XRT
INDEX --> XRT
VOL --> XRT
GUI --> H
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L1-L447)
- [xpack.c](file://src/xpack.c#L1-L762)
- [xpack_core.c](file://src/xpack_core.c#L1-L403)
- [xpack_index.c](file://src/xpack_index.c#L1-L290)
- [xpack_compress.c](file://src/xpack_compress.c#L1-L251)
- [xpack_volume.c](file://src/xpack_volume.c#L1-L353)
- [xpack_internal.h](file://src/xpack_internal.h#L1-L145)
- [zstd.h](file://lib/zstd/zstd.h#L326-L347)
- [xrt.h](file://lib/xrt/xrt.h#L650-L769)
- [xpkgui.c](file://tools/xpkgui/xpkgui.c#L1-L17)

**Section sources**
- [xpack.h](file://src/xpack.h#L1-L447)
- [xpack.c](file://src/xpack.c#L1-L762)
- [xpack_core.c](file://src/xpack_core.c#L1-L403)
- [xpack_index.c](file://src/xpack_index.c#L1-L290)
- [xpack_compress.c](file://src/xpack_compress.c#L1-L251)
- [xpack_volume.c](file://src/xpack_volume.c#L1-L353)
- [xpack_internal.h](file://src/xpack_internal.h#L1-L145)
- [zstd.h](file://lib/zstd/zstd.h#L326-L347)
- [xrt.h](file://lib/xrt/xrt.h#L650-L769)
- [xpkgui.c](file://tools/xpkgui/xpkgui.c#L1-L17)

## Core Components
This section outlines the core components that define Ver7’s capabilities and how they evolved from earlier versions.

- Unified compression level system (0–15): A single compression parameter maps to multiple algorithms with strict monotonicity guarantees.
- Four package types: Core, Index, Linux, and Win32, each optimized for different access patterns and platform characteristics.
- Bit-field structures: Replace bitmask operations with explicit bit fields for clarity and maintainability.
- Pure ZSTD scheme: ZSTD becomes the primary algorithm with configurable strategies, complemented by LZ4, LZ4-HC, and LZMA2 for specialized needs.
- xrt library integration: Provides cross-platform file I/O, memory management, and runtime utilities.
- Solid compression mode: Efficiently stores multiple files in a single compressed block.
- Volume splitting: Enables multi-file archives with optional split-by-size or split-by-file modes.

**Section sources**
- [xpack.h](file://src/xpack.h#L32-L286)
- [xpack.c](file://src/xpack.c#L49-L200)
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)
- [xpack_core.c](file://src/xpack_core.c#L39-L128)
- [xpack_index.c](file://src/xpack_index.c#L69-L174)
- [xpack_volume.c](file://src/xpack_volume.c#L44-L160)
- [xpack_internal.h](file://src/xpack_internal.h#L17-L84)
- [spec.md](file://docs/spec.md#L15-L21)

## Architecture Overview
The Ver7 architecture centers on a unified compression router that maps a single compression level to multiple underlying algorithms. The xrt library abstracts platform-specific file operations, while bit-field structures simplify metadata handling. The design supports four package types with distinct data layouts and access patterns.

```mermaid
graph TB
subgraph "API Layer"
API["Public API<br/>xpack.h"]
end
subgraph "Routing"
ROUTER["Compression Router<br/>xpack_compress.c"]
MAP["Compression Map<br/>xpack.h"]
end
subgraph "Algorithms"
LZ4["LZ4/LZ4HC"]
ZSTD["ZSTD"]
LZMA["LZMA2"]
end
subgraph "Storage"
FILE["xrt File I/O<br/>xrt.h"]
META["Bit-Field Metadata<br/>xpack.h"]
SOLID["Solid Mode<br/>xpack.c"]
VOLUME["Volume Splitting<br/>xpack_volume.c"]
end
API --> ROUTER
ROUTER --> MAP
ROUTER --> LZ4
ROUTER --> ZSTD
ROUTER --> LZMA
API --> FILE
API --> META
API --> SOLID
API --> VOLUME
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L105-L148)
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)
- [xpack.c](file://src/xpack.c#L480-L525)
- [xpack_volume.c](file://src/xpack_volume.c#L178-L229)
- [xrt.h](file://lib/xrt/xrt.h#L668-L714)

**Section sources**
- [xpack.h](file://src/xpack.h#L105-L148)
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)
- [xpack.c](file://src/xpack.c#L480-L525)
- [xpack_volume.c](file://src/xpack_volume.c#L178-L229)
- [xrt.h](file://lib/xrt/xrt.h#L668-L714)

## Detailed Component Analysis

### Transition from FreeBASIC to C Implementation
- Rationale: Moving from FreeBASIC to C enables broader portability, standardized toolchains, and easier integration with modern ecosystems.
- Impact: Simplified build processes, reduced platform-specific dependencies, and improved compatibility with contemporary compilers and linkers.

[No sources needed since this section provides general guidance]

### Addition of Four Package Types
- Core: Sequential position-based access with compact file info layout.
- Index: Integer-indexed access with additional metadata fields.
- Linux: Path-based access with case-sensitive hashing and extended attributes.
- Win32: Path-based access with case-insensitive hashing and Windows-specific timestamps.

```mermaid
classDiagram
class xpkHead {
+uint32_t fileHead
+xpkFlag flag
+uint32_t fileCount
+uint32_t headExtSize
+uint32_t discCode
+uint32_t infoExtSize
+uint32_t reserved
+uint32_t ldbOffset
+uint32_t ldbSize
+uint32_t ldbRawSize
+uint32_t ldbHash
+xtime createTime
+xtime modifyTime
}
class xpkFlag {
+uint32_t value
+uint32_t packType : 4
+uint32_t ldbComp : 4
+uint32_t solidMode : 1
+uint32_t volumeMode : 1
+uint32_t splitMode : 2
+uint32_t reserved : 20
}
class xpkFileInfo {
+uint32_t dataOffset
+uint32_t dataSize
+uint32_t fileSize
+uint32_t fileHash
+xpkFileFlag flag
}
class xpkFileInfoIndex {
+uint32_t dataOffset
+uint32_t dataSize
+uint32_t fileSize
+uint32_t fileHash
+xpkFileFlag flag
+int32_t fileIndex
+int32_t userData
}
class xpkFileInfoLinux {
+uint32_t dataOffset
+uint32_t dataSize
+uint32_t fileSize
+uint32_t fileHash
+xpkFileFlag flag
+char filePath[200]
+uint32_t pathHash
+uint32_t fileAttr
+uint32_t modifyTime
}
class xpkFileInfoWin32 {
+uint32_t dataOffset
+uint32_t dataSize
+uint32_t fileSize
+uint32_t fileHash
+xpkFileFlag flag
+char filePath[200]
+uint32_t pathHash
+uint32_t fileAttr
+uint32_t createTime
+uint32_t modifyTime
}
xpkHead --> xpkFlag : "contains"
xpkFileInfoIndex --> xpkFileFlag : "contains"
xpkFileInfoLinux --> xpkFileFlag : "contains"
xpkFileInfoWin32 --> xpkFileFlag : "contains"
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L120-L237)

**Section sources**
- [xpack.h](file://src/xpack.h#L120-L237)

### Introduction of Custom Compression Callbacks in Ver6
- Rationale: Provide flexibility for advanced users to customize compression behavior.
- Impact: Enhanced extensibility without altering core compression logic.

[No sources needed since this section provides general guidance]

### Revolutionary Changes in Ver7

#### Shift to Pure ZSTD Scheme
- ZSTD strategies are mapped to compression levels, enabling predictable trade-offs between speed and compression ratio.
- LZ4 and LZMA2 remain available for specialized scenarios.

```mermaid
sequenceDiagram
participant App as "Application"
participant API as "xpack API"
participant Router as "xpkCompressRouter"
participant ZSTD as "ZSTD Library"
App->>API : "xpkAppendData(level, data)"
API->>Router : "compress(level, data)"
Router->>Router : "lookup algorithm from map"
Router->>ZSTD : "compress with strategy"
ZSTD-->>Router : "compressed data"
Router-->>API : "success"
API-->>App : "position"
```

**Diagram sources**
- [xpack_compress.c](file://src/xpack_compress.c#L20-L94)
- [zstd.h](file://lib/zstd/zstd.h#L336-L347)

**Section sources**
- [xpack_compress.c](file://src/xpack_compress.c#L20-L94)
- [zstd.h](file://lib/zstd/zstd.h#L336-L347)

#### Compression Level System (0–15)
- Levels map to algorithms with strict monotonicity: higher levels yield better compression ratios at lower speeds.
- Default level is ZSTD greedy, balancing speed and ratio.

```mermaid
flowchart TD
Start(["Select Compression Level"]) --> Map["Lookup Algorithm Map"]
Map --> Store{"Level 0?"}
Store --> |Yes| Copy["Direct Copy (No Compression)"]
Store --> |No| LZ4{"LZ4/LZ4HC?"}
LZ4 --> |Yes| LZ4Comp["Use LZ4/LZ4-HC"]
LZ4 --> |No| ZSTD{"ZSTD?"}
ZSTD --> |Yes| ZSTDComp["Use ZSTD with Strategy"]
ZSTD --> |No| LZMA{"LZMA2?"}
LZMA --> |Yes| LZMAComp["Use LZMA2"]
LZMA --> |No| Fallback["Fallback to No Compression"]
Copy --> Done(["Return Compressed Data"])
LZ4Comp --> Done
ZSTDComp --> Done
LZMAComp --> Done
Fallback --> Done
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L269-L286)
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)

**Section sources**
- [xpack.h](file://src/xpack.h#L269-L286)
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)

#### Integration of the xrt Library
- xrt provides cross-platform file I/O, memory management, and runtime utilities.
- Benefits: Simplified platform abstraction, consistent error handling, and streamlined development.

```mermaid
graph LR
XPACK["xPack Core"]
XRT["xrt Library"]
FS["File System"]
MEM["Memory Manager"]
TIME["Time Utilities"]
XPACK --> XRT
XRT --> FS
XRT --> MEM
XRT --> TIME
```

**Diagram sources**
- [xrt.h](file://lib/xrt/xrt.h#L668-L714)
- [xpack.c](file://src/xpack.c#L55-L81)

**Section sources**
- [xrt.h](file://lib/xrt/xrt.h#L668-L714)
- [xpack.c](file://src/xpack.c#L55-L81)

#### Adoption of Bit-Field Structures
- Bit-fields replace manual bitmask operations, improving readability and reducing errors.
- Used in package flags and file flags to encode metadata compactly.

```mermaid
classDiagram
class xpkFlag {
+uint32_t value
+uint32_t packType : 4
+uint32_t ldbComp : 4
+uint32_t solidMode : 1
+uint32_t volumeMode : 1
+uint32_t splitMode : 2
+uint32_t reserved : 20
}
class xpkFileFlag {
+uint32_t value
+uint32_t compLevel : 4
+uint32_t fileType : 4
+uint32_t encrypted : 1
+uint32_t reserved : 23
}
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L105-L161)

**Section sources**
- [xpack.h](file://src/xpack.h#L105-L161)

### API Design Improvements
- Unified compression parameter simplifies usage across all package types.
- Bit-field structures improve clarity and reduce bitwise errors.
- Solid mode and volume splitting APIs enable efficient storage and distribution.

**Section sources**
- [xpack.h](file://src/xpack.h#L329-L441)
- [xpack.c](file://src/xpack.c#L427-L525)
- [xpack_volume.c](file://src/xpack_volume.c#L178-L229)

### Cross-Platform Support
- xrt abstracts file I/O and system calls, ensuring consistent behavior across platforms.
- Tools like the GUI demonstrate practical integration and usage.

**Section sources**
- [xrt.h](file://lib/xrt/xrt.h#L668-L714)
- [xpkgui.c](file://tools/xpkgui/xpkgui.c#L1-L17)

## Dependency Analysis
The Ver7 architecture exhibits low coupling and high cohesion:
- Compression logic is isolated in a dedicated module with clear interfaces.
- xrt encapsulates platform-specific concerns, minimizing exposure to the rest of the system.
- Package types share common metadata structures but differ in data layouts.

```mermaid
graph TB
COMP["xpack_compress.c"] --> ZSTD["lib/zstd/*.h"]
COMP --> LZ4["LZ4 Headers"]
COMP --> LZMA["LZMA Headers"]
CORE["xpack_core.c"] --> COMP
INDEX["xpack_index.c"] --> COMP
VOL["xpack_volume.c"] --> XRT["lib/xrt/xrt.h"]
MAIN["xpack.c"] --> COMP
MAIN --> VOL
MAIN --> XRT
```

**Diagram sources**
- [xpack_compress.c](file://src/xpack_compress.c#L9-L14)
- [xpack_core.c](file://src/xpack_core.c#L12-L12)
- [xpack_index.c](file://src/xpack_index.c#L12-L12)
- [xpack_volume.c](file://src/xpack_volume.c#L7-L9)
- [xpack.c](file://src/xpack.c#L7-L9)
- [xrt.h](file://lib/xrt/xrt.h#L668-L714)

**Section sources**
- [xpack_compress.c](file://src/xpack_compress.c#L9-L14)
- [xpack_core.c](file://src/xpack_core.c#L12-L12)
- [xpack_index.c](file://src/xpack_index.c#L12-L12)
- [xpack_volume.c](file://src/xpack_volume.c#L7-L9)
- [xpack.c](file://src/xpack.c#L7-L9)
- [xrt.h](file://lib/xrt/xrt.h#L668-L714)

## Performance Considerations
- Compression level selection directly impacts throughput and compression ratio; choose levels based on workload characteristics.
- Solid mode reduces metadata overhead for many small files but restricts updates and deletions.
- Volume splitting improves manageability for large archives but adds complexity to read operations.
- Bit-field structures minimize memory footprint and improve cache locality.

[No sources needed since this section provides general guidance]

## Troubleshooting Guide
Common issues and resolutions:
- Version mismatch: Ver7 uses a new file signature; older versions cannot open Ver7 archives.
- Readonly mode violations: Certain operations fail when the archive is opened in readonly mode.
- Invalid positions: Accessing non-existent indices or positions results in errors.
- Compression failures: The router falls back to no compression if underlying algorithms fail.
- Volume-related errors: Insufficient capacity or missing volume files cause failures during read/write operations.

**Section sources**
- [xpack.c](file://src/xpack.c#L98-L104)
- [xpack.c](file://src/xpack.c#L204-L207)
- [xpack_core.c](file://src/xpack_core.c#L148-L151)
- [xpack_compress.c](file://src/xpack_compress.c#L46-L50)
- [xpack_volume.c](file://src/xpack_volume.c#L196-L198)

## Conclusion
The evolution from Ver5 to Ver7 represents a significant modernization of the xPack library. The move to C, the addition of four package types, and the unified compression level system (0–15) collectively enhance usability, performance, and maintainability. The integration of the xrt library and the adoption of bit-field structures further streamline development and deployment. Ver7 establishes a robust foundation for cross-platform applications while preserving backward compatibility through careful versioning and error handling.

[No sources needed since this section summarizes without analyzing specific files]

## Appendices

### Migration Guidance
- From Ver5 to Ver7:
  - Update build configurations to compile C sources and link against xrt and ZSTD libraries.
  - Replace FreeBASIC-specific constructs with C equivalents.
  - Adopt the unified compression level parameter; adjust application logic to select appropriate levels.
  - Review package type usage; choose the most suitable type (Core, Index, Linux, Win32) for your access patterns.
  - Integrate solid mode and volume splitting APIs where beneficial.
  - Test thoroughly for version compatibility; Ver7 archives are not readable by older versions.

- Breaking changes:
  - New file signature and version numbering scheme.
  - Unified compression parameter replaces algorithm-specific controls.
  - Bit-field structures require updated access patterns compared to raw bitmask operations.

- New capabilities:
  - Pure ZSTD scheme with configurable strategies.
  - Comprehensive cross-platform support via xrt.
  - Efficient solid compression and flexible volume splitting.

**Section sources**
- [xpack.h](file://src/xpack.h#L32-L33)
- [xpack.h](file://src/xpack.h#L269-L286)
- [xpack.c](file://src/xpack.c#L98-L104)
- [xrt.h](file://lib/xrt/xrt.h#L668-L714)