# Project Overview

<cite>
**Referenced Files in This Document**
- [xpack.h](file://src/xpack.h)
- [xpack.c](file://src/xpack.c)
- [xpack_internal.h](file://src/xpack_internal.h)
- [xpack_core.c](file://src/xpack_core.c)
- [xpack_index.c](file://src/xpack_index.c)
- [xpack_compress.c](file://src/xpack_compress.c)
- [zstd.h](file://lib/zstd/zstd.h)
- [xrt.h](file://lib/xrt/xrt.h)
- [spec.md](file://docs/spec.md)
- [design.md](file://docs/design.md)
- [27_integration_real_world.h](file://test/27_integration_real_world.h)
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

## Introduction

xPack Ver7 is a lightweight file compression package library designed to provide a unified compression solution supporting multiple algorithms and package types. The library serves as a comprehensive file packaging and compression system that enables efficient storage and retrieval of files across diverse use cases.

### Purpose and Scope

The primary purpose of xPack is to offer a flexible, high-performance compression solution that can adapt to various scenarios:
- **Multi-algorithm support**: Integrates LZ4, ZSTD, and LZMA2 compression algorithms
- **Multi-package types**: Supports four distinct package modes (Core, Index, Linux, Win32)
- **Unified compression levels**: Provides a standardized 0-15 compression scale
- **Cross-platform compatibility**: Designed for both Linux and Windows environments

### Evolution from Ver5 to Ver7

The journey from Ver5 to Ver7 represents a significant architectural transformation:

**Ver5 (FreeBASIC)**: Initial implementation with basic compression capabilities
**Ver6 (C rewrite)**: Introduced four package types and custom compression callbacks
**Ver7 (Pure ZSTD)**: Major simplification focusing on ZSTD as the primary compression algorithm

Key improvements in Ver7 include:
- **Pure ZSTD scheme**: Simplified compression pipeline using ZSTD exclusively
- **Compression level system**: Standardized 0-15 level compression with strict monotonicity
- **xrt library integration**: Unified dependency management through the xrt ecosystem
- **Bit-field structures**: Modernized data representation using bit-fields instead of traditional masks

## Project Structure

The xPack Ver7 project follows a modular architecture with clear separation of concerns:

```mermaid
graph TB
subgraph "Core Library"
A[src/xpack.c] --> B[src/xpack.h]
B --> C[src/xpack_internal.h]
end
subgraph "Package Modes"
D[src/xpack_core.c] --> B
E[src/xpack_index.c] --> B
F[src/xpack_path.c] --> B
end
subgraph "Compression Layer"
G[src/xpack_compress.c] --> B
H[lib/zstd/zstd.h] --> G
I[lib/lz4/] --> G
J[lib/lzma/] --> G
end
subgraph "Utility Layer"
K[lib/xrt/xrt.h] --> B
L[lib/xrt/lib/] --> K
end
subgraph "Documentation"
M[docs/spec.md] --> B
N[docs/design.md] --> B
end
subgraph "Testing"
O[test/27_integration_real_world.h] --> B
end
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L1-L447)
- [xpack.c](file://src/xpack.c#L1-L762)
- [xpack_internal.h](file://src/xpack_internal.h#L1-L145)

**Section sources**
- [xpack.h](file://src/xpack.h#L1-L447)
- [xpack.c](file://src/xpack.c#L1-L762)
- [xpack_internal.h](file://src/xpack_internal.h#L1-L145)

## Core Components

### xPack Object Model

The central abstraction in xPack is the `xpkObject` handle, which encapsulates all package state and operations:

```mermaid
classDiagram
class xpkObject {
+xfile file
+uint32 baseOffset
+uint8 readonly
+uint8 modified
+xpkHead head
+void* headExt
+xarray_struct ldb
+xpkErrorProc onError
+uint8 solidMode
+uint8 solidCompLevel
+xbuffer solidBuffer
+void* solidDecompressed
+uint32 solidDecompSize
+uint8 solidCached
+xpkVolume volume
}
class xpkHead {
+uint32 fileHead
+xpkFlag flag
+uint32 fileCount
+uint32 headExtSize
+uint32 discCode
+uint32 infoExtSize
+uint32 ldbOffset
+uint32 ldbSize
+uint32 ldbRawSize
+uint32 ldbHash
+xtime createTime
+xtime modifyTime
}
class xpkFlag {
+uint32 value
+uint32 packType : 4
+uint32 ldbComp : 4
+uint32 solidMode : 1
+uint32 volumeMode : 1
+uint32 splitMode : 2
}
xpkObject --> xpkHead : "contains"
xpkHead --> xpkFlag : "uses"
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L120-L148)
- [xpack_internal.h](file://src/xpack_internal.h#L47-L84)

### Package Types

xPack supports four distinct package types, each optimized for specific use cases:

| Type | Value | Access Method | Use Case |
|------|:-----:|---------------|----------|
| Core | 0 | Position-based (1-based) | Fixed-order files, minimal overhead |
| Index | 1 | Integer index | ID-based access, user data storage |
| Linux | 2 | Path-based (case-sensitive) | Unix/Linux systems, preserves case |
| Win32 | 3 | Path-based (case-insensitive) | Windows systems, case normalization |

**Section sources**
- [xpack.h](file://src/xpack.h#L38-L41)
- [xpack.h](file://src/xpack.h#L164-L237)

### Compression Level System

The compression system implements a sophisticated 0-15 level scale with strict monotonicity guarantees:

```mermaid
flowchart TD
A["Compression Level 0-15"] --> B["Algorithm Selection"]
B --> C["LZ4 Fast (Levels 1-2)"]
B --> D["LZ4-HC (Levels 3-4)"]
B --> E["ZSTD Strategies (Levels 5-15)"]
B --> F["Uncompressed (Level 0)"]
E --> G["ZSTD Fast"]
E --> H["ZSTD DFast"]
E --> I["ZSTD Greedy"]
E --> J["ZSTD Lazy"]
E --> K["ZSTD Lazy2"]
E --> L["ZSTD BTLazy2"]
E --> M["ZSTD BTOpt"]
E --> N["ZSTD BTUltra"]
E --> O["ZSTD BTUltra2"]
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L269-L286)
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)

**Section sources**
- [xpack.h](file://src/xpack.h#L269-L286)
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)

## Architecture Overview

The xPack architecture follows a layered design pattern with clear separation between compression, packaging, and utility layers:

```mermaid
graph TB
subgraph "Application Layer"
A[xpk API Functions]
end
subgraph "Package Management Layer"
B[xpkOpen/xpkSave/xpkClose]
C[xpkType/xpkCount/xpkDiscCode]
D[xpkSolidMode/xpkVolumeMode]
end
subgraph "Package Mode Layer"
E[Core Operations]
F[Index Operations]
G[Path Operations]
end
subgraph "Compression Layer"
H[Compression Router]
I[ZSTD Engine]
J[LZ4 Engine]
K[LZMA2 Engine]
end
subgraph "Storage Layer"
L[xrt File System]
M[xrt Memory Management]
N[xrt Hash Functions]
end
A --> B
B --> C
C --> D
D --> E
D --> F
D --> G
E --> H
F --> H
G --> H
H --> I
H --> J
H --> K
I --> L
J --> L
K --> L
L --> M
L --> N
```

**Diagram sources**
- [xpack.c](file://src/xpack.c#L49-L297)
- [xpack_core.c](file://src/xpack_core.c#L18-L128)
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)

**Section sources**
- [xpack.c](file://src/xpack.c#L49-L297)
- [xpack_core.c](file://src/xpack_core.c#L18-L128)
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)

## Detailed Component Analysis

### Compression Engine

The compression engine serves as the heart of xPack's performance characteristics:

```mermaid
sequenceDiagram
participant App as Application
participant API as xpk API
participant Router as Compression Router
participant ZSTD as ZSTD Engine
participant LZ4 as LZ4 Engine
participant Storage as Storage Layer
App->>API : xpkAppendData(data, level)
API->>Router : xpkCompressRouter(level, data)
alt Level 0-4 (LZ4/LZ4-HC)
Router->>LZ4 : LZ4_compress_fast()
LZ4-->>Router : compressed_data
else Level 5-15 (ZSTD)
Router->>ZSTD : ZSTD_compress2()
ZSTD-->>Router : compressed_data
end
Router-->>API : compressed_data
API->>Storage : Write to file
Storage-->>API : Success
API-->>App : Position/Success
```

**Diagram sources**
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)
- [zstd.h](file://lib/zstd/zstd.h#L154-L175)

**Section sources**
- [xpack_compress.c](file://src/xpack_compress.c#L20-L147)
- [zstd.h](file://lib/zstd/zstd.h#L154-L175)

### Package Type Implementations

Each package type implements specialized operations optimized for its access pattern:

#### Core Mode Implementation
Core mode provides the simplest access pattern with sequential position-based indexing:

```mermaid
flowchart TD
A["Core Mode Operations"] --> B["xpkAppendFile()"]
A --> C["xpkExtractFile()"]
A --> D["xpkUpdateFile()"]
A --> E["xpkRemove()"]
B --> F["Read file -> Compress -> Write"]
C --> G["Read compressed -> Decompress -> Write"]
D --> H["Compress new data -> Write"]
E --> I["Remove from LDB"]
F --> J["Update xpkFileInfo"]
G --> K["Verify hash"]
H --> J
I --> L["Shift positions"]
```

**Diagram sources**
- [xpack_core.c](file://src/xpack_core.c#L18-L128)

**Section sources**
- [xpack_core.c](file://src/xpack_core.c#L18-L128)

#### Index Mode Implementation
Index mode supports integer-based access with additional metadata storage:

```mermaid
flowchart LR
A["Index Mode"] --> B["xpkIndexAppendFile()"]
A --> C["xpkIndexExtractFile()"]
A --> D["xpkIndexUpdateFile()"]
A --> E["xpkIndexRemove()"]
B --> F["Generate unique index"]
F --> G["Store fileIndex + userData"]
C --> H["Lookup by index -> Extract"]
D --> I["Update existing index"]
E --> J["Remove from LDB"]
```

**Diagram sources**
- [xpack_index.c](file://src/xpack_index.c#L36-L174)

**Section sources**
- [xpack_index.c](file://src/xpack_index.c#L36-L174)

### Storage and Volume Management

xPack implements sophisticated storage management including volume splitting for large archives:

```mermaid
flowchart TD
A["Volume Management"] --> B["Single Volume"]
A --> C["Multi-Volume"]
B --> D["Direct file access"]
C --> E["Volume Header"]
C --> F["Volume Index"]
C --> G["Volume Offsets"]
E --> H["xpkHead + xpkVolumeInfo"]
F --> I["Sequential numbering"]
G --> J["Calculate offsets"]
K["Volume Writing"] --> L["Check capacity"]
L --> M{"Enough space?"}
M --> |Yes| N["Write to current volume"]
M --> |No| O["Create next volume"]
O --> P["Update volume info"]
P --> Q["Continue writing"]
```

**Diagram sources**
- [xpack.c](file://src/xpack.c#L646-L761)
- [xpack_internal.h](file://src/xpack_internal.h#L23-L42)

**Section sources**
- [xpack.c](file://src/xpack.c#L646-L761)
- [xpack_internal.h](file://src/xpack_internal.h#L23-L42)

## Dependency Analysis

### External Dependencies

xPack maintains a minimal dependency footprint while leveraging powerful compression libraries:

```mermaid
graph TB
subgraph "xPack Core"
A[xpack.h]
B[xpack.c]
C[xpack_compress.c]
end
subgraph "Required Dependencies"
D[xrt.h]
E[lz4.h]
F[zstd.h]
G[lzma.h]
end
subgraph "Internal Libraries"
H[xrt Core]
I[xrt File System]
J[xrt Memory]
K[xrt Hash]
end
A --> D
B --> D
C --> E
C --> F
C --> G
D --> H
H --> I
H --> J
H --> K
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L22-L23)
- [xpack_compress.c](file://src/xpack_compress.c#L9-L14)
- [xrt.h](file://lib/xrt/xrt.h#L1-L800)

### Internal Dependencies

The library exhibits clear internal dependency relationships:

```mermaid
graph LR
A[xpack.h] --> B[xpack_internal.h]
B --> C[xpack.c]
B --> D[xpack_core.c]
B --> E[xpack_index.c]
B --> F[xpack_compress.c]
C --> G[xrt.h]
D --> G
E --> G
F --> G
F --> H[lz4.h]
F --> I[zstd.h]
F --> J[lzma.h]
```

**Diagram sources**
- [xpack.h](file://src/xpack.h#L1-L447)
- [xpack_internal.h](file://src/xpack_internal.h#L10-L11)

**Section sources**
- [xpack.h](file://src/xpack.h#L1-L447)
- [xpack_internal.h](file://src/xpack_internal.h#L10-L11)

## Performance Considerations

### Compression Performance Characteristics

xPack's compression performance varies significantly across different levels and algorithms:

| Level | Algorithm | Compression Speed | Decompression Speed | Typical Ratio |
|-------|-----------|-------------------|-------------------|---------------|
| 0 | Uncompressed | ∞ | ∞ | 1.00 |
| 1 | LZ4 Fast | 780 MB/s | 4500 MB/s | ~2.10 |
| 2 | LZ4-HC Level 4 | 120 MB/s | 4500 MB/s | ~2.45 |
| 3 | LZ4-HC Level 9 | 40 MB/s | 4500 MB/s | ~2.72 |
| 4-15 | ZSTD Levels | 2-500 MB/s | 950-1400 MB/s | ~2.88-3.52 |

### Memory Management

The library employs sophisticated memory management strategies:

```mermaid
flowchart TD
A["Memory Management"] --> B["Static Allocation"]
A --> C["Dynamic Allocation"]
A --> D["Buffer Pooling"]
B --> E["Fixed-size structures"]
B --> F["Pre-allocated arrays"]
C --> G["On-demand allocation"]
C --> H["Temporary buffers"]
D --> I["Solid compression buffers"]
D --> J["Volume buffers"]
D --> K["Compression scratch space"]
I --> L["xrtBufferCreate()"]
J --> M["xrtBufferAppend()"]
K --> N["malloc/free"]
```

**Diagram sources**
- [xpack.c](file://src/xpack.c#L480-L525)
- [xpack_compress.c](file://src/xpack_compress.c#L227-L250)

### File System Integration

xPack integrates seamlessly with the underlying file system through the xrt library:

```mermaid
sequenceDiagram
participant App as Application
participant xPack as xPack
participant xrt as xrt Library
participant FS as File System
App->>xPack : xpkOpen("archive.xpk")
xPack->>xrt : xrtOpen(path, readonly, XRT_CP_BINARY)
xrt->>FS : Open file handle
FS-->>xrt : File descriptor
xrt-->>xPack : xfile handle
xPack-->>App : xpkObject
App->>xPack : xpkAppendData(data, level)
xPack->>xrt : xrtSeek(file, offset, SEEK_SET)
xPack->>xrt : xrtPut(file, data, size)
xrt->>FS : Write to disk
FS-->>xrt : Bytes written
xrt-->>xPack : Success
xPack-->>App : Success
```

**Diagram sources**
- [xpack.c](file://src/xpack.c#L81-L82)
- [xpack_core.c](file://src/xpack_core.c#L100-L107)

**Section sources**
- [xpack.c](file://src/xpack.c#L480-L525)
- [xpack_compress.c](file://src/xpack_compress.c#L227-L250)
- [xpack.c](file://src/xpack.c#L81-L82)

## Troubleshooting Guide

### Common Error Scenarios

The library provides comprehensive error reporting through a centralized error system:

| Error Code | Description | Typical Cause | Solution |
|------------|-------------|---------------|----------|
| 1 | File open failed | Permission denied, path not found | Verify file path and permissions |
| 2 | File read failed | Disk corruption, incomplete file | Check file integrity, retry operation |
| 3 | Memory allocation failed | Insufficient RAM, fragmentation | Free memory, reduce compression level |
| 4 | Invalid pack format | Wrong file format, corrupted header | Verify file is valid xPack archive |
| 5 | Version not supported | Newer library version | Update xPack library |
| 6 | Invalid file position | Index out of bounds | Check file count and indices |
| 7 | Compression failed | Algorithm error, unsupported level | Try different compression level |
| 8 | Decompression failed | Corrupted data, wrong level | Verify data integrity, check level |
| 9 | Hash verification failed | Data corruption, tampering | Recreate archive, verify checksums |
| 10 | Readonly mode write denied | Attempted modification | Open with write permissions |
| 11 | Pack type mismatch | Wrong mode for operation | Set correct package type |

### Debugging Strategies

Effective debugging of xPack operations involves several key approaches:

1. **Error Checking**: Always check return values and use `xpkLastError()` for detailed error information
2. **Validation**: Use `xpkVerify()` and `xpkVerifyAll()` to validate archive integrity
3. **Statistics**: Monitor compression ratios and performance using `xpkStatGet()`
4. **Logging**: Implement custom error handlers via `xpkOnError()` for detailed diagnostics

**Section sources**
- [xpack.c](file://src/xpack.c#L27-L43)
- [xpack.h](file://src/xpack.h#L291-L296)

## Conclusion

xPack Ver7 represents a mature, production-ready compression library that successfully balances simplicity with powerful functionality. The library's evolution from Ver5 to Ver7 demonstrates a clear commitment to architectural refinement, focusing on ZSTD as the primary compression algorithm while maintaining flexibility through the 0-15 compression level system.

### Key Strengths

1. **Unified Design**: Single API surface supporting multiple compression algorithms and package types
2. **Performance Focus**: Optimized for both compression speed and decompression performance
3. **Cross-platform Compatibility**: Seamless operation across Linux and Windows environments
4. **Robust Architecture**: Clear separation of concerns with well-defined interfaces
5. **Comprehensive Testing**: Extensive test suite covering real-world usage scenarios

### Practical Applications

The library excels in several domains:

- **Game Asset Management**: Efficient compression of textures, models, and audio assets
- **Software Distribution**: Reliable packaging and delivery of application updates
- **Data Archiving**: Long-term storage solutions with excellent compression ratios
- **Real-time Systems**: Fast decompression for interactive applications

### Future Directions

The xPack Ver7 foundation provides a solid platform for future enhancements, including potential support for additional compression algorithms, enhanced parallel processing capabilities, and expanded platform support.