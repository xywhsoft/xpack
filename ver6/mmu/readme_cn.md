## 1. 项目介绍

### 1.1 简介

MMU 是 Memory Management Unit 的缩写，含义是内存管理单元，它可以帮助开发者高效（即能提升开发效率，也能提升程序运行效率）的管理各种内存中的数据。

最初 MMU 库仅由几个简单的功能组成（PAMM、SAMM、MBMU），他们被用于管理各种结构体和指针数据，依靠自动扩展缓冲区的能力，降低我在编写程序时维护各种结构体表格的工作量，我在工作中遇到了很多内存数据管理方面的挑战，也尝试使用各种现有的代码来提升开发效率，但让程序组织很多零散的代码并不是一个好主意，最终我决定开发一个库，为这类问题提供一个统一的、完善的解决方案。

于是，我将 MMU 从 xLang 项目中被剥离出来（正因为 xLang 对内存和数据结构的大量新需求，让 MMU 库的功能得到了极大的完善），作为一个独立的库开源。

MMU 库专注于解决内存管理问题，包括提升内存的分配与释放性能，将内存揉捏成各种易于使用的形状，提供简单易用的 API 使开发者可以以相当简单的方式与内存打交道，而不是把时间浪费在数据该如何组织这件事上，使用 MMU 模块也可以很容易改善程序的内存泄漏问题。

MMU 具备相当强的性能，必要的时候 MMU 会选择以空间换性能的方式来提升运行效率，但这不代表 MMU 会造成严重的内存浪费，相反 MMU 精打细算，尽可能利用好每一个字节的空间，每一个CPU指令的时间，我一直抱着基础功能不应该过度损耗资源的态度进行开发，高楼能够稳定安全的拔地而起，离不开性能优秀的基础建材的贡献，我希望 MMU 也能够成为优秀的基础。

### 1.2 功能范围

目前 MMU 库提供了 23 个子模块，这些子模块有的用来管理内存，有的用来管理数据形态，一些复杂的子模块构建于更基础的子模块之上。

使用它们你不需要关心内存，它们会自动扩展内存缓冲区，进行必要的内存管理、复制、转移等工作。

要实现有组织的管理大量数据，就需要将这些数据抽象化为各种易于理解的形态，而不是一大堆变量或指针，在C语言中比较复杂的数据结构只有结构体和数组，使用它们来管理大量数据显然需要额外进行很多工作，尤其是增、删、改、查等功能的实现，不仅麻烦，还很难做出较高的性能，MMU 为此而诞生，提供的这些功能模块可以帮助开发者将数据捏成自己想要形状。

未来 MMU 库还可能提供更多（数种乃至数十种）管理器，来管理更多形态的内存数据，以便应对更多挑战。

一些功能是重合的，例如哈希表提供了32位、64位和基于AVLTree和RBTree的组合实现，一共4种，这样做是为了方便最终用户根据自己的业务需要取舍，选择合适的模块使用。

#### 它们包括：

##### PAMM（Point Array Memory Management）：
用于管理指针数组，可以非常方便的插入、删除、交换、排序数组元素，它具备数组的特性，内存块一定是连续的，适合用于替代数组的场合；它还是许多复杂模块的基础，用于管理这些复杂模块的数据指针。

##### SAMM（Struct Array Memory Management）：
用于管理结构体数组，可以非常方便的插入、删除、交换、排序数组元素，它具备数组的特性，内存块一定是连续的，适合用于替代数组的场合。

##### MBMU（Memory Buffer Management Unit）：
用于管理长度不固定内存缓冲区，最常见的用途是作为 StringBuilder 使用，它支持 ANSI、UTF8、UTF16、UTF32、BINARY 等多种不同编码（实际上就是尾部添加截断字符的区别），它也可以用来生成二进制的数据，例如在内存中构建文件格式，再统一写入。

##### BSMM（Blocks Struct Memory Management）：
用于管理结构体数据集，它在功能上和 SAMM 类似，但它不是按照数组的原则设计的，而是将结构体内存划分为多个 256 元素大小的块，再通过 PAMM 来管理这些块，这避免了数组扩容时需要重新申请内存，甚至复制数据带来的性能问题，它不支持插入、删除和排序的操作，因为内存形态的不同，这些操作会导致更严重的性能问题，选择使用哪一个子模块，可以根据需要进行取舍。

##### MMU256（Memory Management Unit 256）：
这是一个相当基础的内存管理单元，它只能管理 256 个固定长度的数据，一旦超出这个长度，它就束手无策了，它允许开发者申请一块内存，或将内存归还给管理器，以及标记一块内存，再对管理器中所有内存进行一次垃圾回收操作（Garbage collection），它主要用于实现更复杂的内存管理系统，不建议开发者单独使用它。

##### MMU64K（Memory Management Unit 64K）：
这是一个相当基础的内存管理单元，它只能管理 65536 个固定长度的数据，一旦超出这个长度，它就束手无策了，它允许开发者申请一块内存，或将内存归还给管理器，以及标记一块内存，再对管理器中所有内存进行一次垃圾回收操作（Garbage collection），它主要用于实现更复杂的内存管理系统，不建议开发者单独使用它。

##### MM256（Memory Management 256）：
基于 MMU256 开发的全功能内存管理器，它可以作为一个元素大小固定的内存池来使用，支持的功能：申请、释放、标记和垃圾回收（Garbage collection）；相比 MM64K，MM256 使用占用内存较小的管理单元，更节约内存，但如果要进行大量的数据元素管理，会产生更频繁的内存申请和释放操作，导致性能低于 MM64K，推荐在管理小于50万个数据元素的场景使用。

##### MM64K（Memory Management 64K）：
基于 MMU64K 开发的全功能内存管理器，它可以作为一个元素大小固定的内存池来使用，支持的功能：申请、释放、标记和垃圾回收（Garbage collection）；相比 MM256，MM64K 进行更少的内存申请和释放操作（因为预深情块更大），但是内存浪费也会更严重，推荐在管理大量数据元素（大于10万）的场景使用使用。

##### SSSTK（Struct Static Stack）：
静态结构体栈用于管理栈型数据结构，栈元素为任意长度的结构体数据，静态栈需要在创建时指定最大栈深度，内存一次性申请，超出最大栈深度后无法再进行压栈操作。

##### PSSTK（Point Static Stack）：
静态指针栈用于管理栈型数据结构，栈元素为一个指针，通常用于 SSSTK 无法处理的情况（栈成员长度大小不统一的情况），静态栈需要在创建时指定最大栈深度，内存一次性申请，超出最大栈深度后无法再进行压栈操作。

##### SDSTK（Struct Dynamic Stack）：
动态结构体栈用于管理栈型数据结构，栈元素为任意长度的结构体数据，动态栈创建时不需要指定最大栈深度，会在压栈操作时自动扩展内存，在出栈操作时自动释放内存，内存会预留一部分做缓冲，以提升操作性能，避免频繁的入栈出栈，每一次都要反复申请和释放内存。

##### PDSTK（Point Dynamic Stack）：
动态指针栈用于管理栈型数据结构，栈元素为一个指针，与 SSSTK 和 PSSTK 的关系类似，PDSTK 用于解决 SDSTK 无法处理的情况，它的特点和 SDSTK 一致，可以使用指针来管理栈成员长度大小不统一的情况。

##### LLIST（Linked List）：
双向链表数据结构，用于管理链状数据，由于单向链表实现起来太简单，没必要作为一个模块提供，所以 MMU 库只实现了双向链表，链表可以实现非常高效的插入、删除操作，但查询操作会消耗更多时间。

##### AVLTree：
平衡二叉树是一种二叉树结构，它可以实现非常高效的查找操作，可以接受的插入和删除操作，是一种性能非常均衡的数据容器，其节点能够保存任意长度的结构体数据。

##### RBTree（Red Blask Tree）：
红黑树是一种二叉树结构，它可以实现非常高效的插入和删除操作，可以接受的查找操作，是一种性能非常均衡的数据容器，其节点能够保存任意长度的结构体数据。

##### MP256（Memory Pool 256）：
内存池是一种可以大幅提升内存申请、释放效率的内存管理器，它与 MM256 的主要区别在于，MP256 能够指定要申请的内存长度，而不是像 MM256 那样每次申请都只能得到预先设置好数据长度大小的内存，内存池的性能非常高，甚至优于 mimalloc 库。

##### MP64K（Memory Pool 64K）：
内存池是一种可以大幅提升内存申请、释放效率的内存管理器，它与 MM256 的主要区别在于，MP256 能够指定要申请的内存长度，而不是像 MM256 那样每次申请都只能得到预先设置好数据长度大小的内存，内存池的性能非常高，甚至优于 mimalloc 库。

##### HASH32：
MMU 库的 32 位 Hash 算法采用了由 James Z.M. Gao 开发的 nmhash32x 算法（基于 BSD 2-Clause 许可协议授权），经过基准测试，它是综合性能最好的 32 为 Hash 算法之一（在所有平台）。

##### HASH64：
MMU 库的 64 位 Hash 算法采用了由 Nicolas De Carli、Wang Yi 开发的 rapidhash 算法（基于 BSD 2-Clause 许可协议授权），经过基准测试，它是综合性能最好的 64 为 Hash 算法之一（在所有平台）。

##### AVLHT32（AVLTree Hash Table 32bit）：
基于 AVLTree 实现的哈希表（使用32位哈希算法），哈希表可以使用任意数据作为 Key，来绑定任意长度的结构体数据，并进行高效的插入、删除、查询等操作。

##### AVLHT64（AVLTree Hash Table 64bit）：
基于 AVLTree 实现的哈希表（使用64位哈希算法），哈希表可以使用任意数据作为 Key，来绑定任意长度的结构体数据，并进行高效的插入、删除、查询等操作。

##### RBHT32（RBTree Hash Table 32bit）：
基于 RBTree 实现的哈希表（使用32位哈希算法），哈希表可以使用任意数据作为 Key，来绑定任意长度的结构体数据，并进行高效的插入、删除、查询等操作。

##### RBHT64（RBTree Hash Table 64bit）：
基于 RBTree 实现的哈希表（使用64位哈希算法），哈希表可以使用任意数据作为 Key，来绑定任意长度的结构体数据，并进行高效的插入、删除、查询等操作。

### 1.3 设计原则

MMU 库是一个力求性能指标先进的基础库，在作者的能力范围内，会尽可能的优化所有性能指标，包括运行速度、内存占用、易用性、安全性冗余。

如上面所说，基础库必须珍惜每一个字节的内存，每一次CPU的心跳，留出最大的性能冗余，基于此建设的上层建筑才能确保安全和稳定，不至于在某天因为一块砖、一根钢筋或一袋混凝土的性能瓶颈，摧毁整栋建筑。

包括代码中使用的第三方功能实现（nmhash32x、rapidhash），也都是经过了充分的基准测试，优中选优采取的。

MMU 设计了性能高效的各类内存管理器，可以非常快速的申请和释放内存，几乎所有管理器都自带内存缓冲机制，避免反复申请和释放带来巨大的性能损耗，通过牺牲尺度可控的内存空间来提升整体的运行效率，是符合 MMU 设计原则的。

MMU 库同样重视易用性，受 肖恩·巴雷特 的影响，过去作者经常使用基于头文件的方式分享模块化代码，用于分发的库应该组织结构简单清晰（如果做不到简单，至少应该保持清晰），易于集成和使用。

因此我拒绝使用 make 工具，程序员应该将注意力集中在功能实现上，而不是被迫学习一大堆工具，优秀的设计师桌面应该是简洁的，高度模块化的，而不是一大坨形态各异的东西，通过冗余度极高的方式把他们拼凑起来。

编译 MMU 不挑环境，不需要再编译过程中解决各种环境下跳出的一大堆报错，甚至不需要编译，仅仅使用一行 #include 代码就可以将 MMU 轻易的集成到自己的工程里，易于使用、不为开发者带来学习负担的代码是更好的开源标准。

MMU 库是易于裁剪的，它提供了许多小组件，但它们不会被全部编译到您的程序里，开发者可以通过宏定义选择自己需要的功能，只保留一部分代码，这样有助于避免程序体积莫名其妙的膨胀。

MMU 功能裁剪使用 #define MMU_USE_* 宏定义，可参考文件：mmu_config.h。

### 1.4 项目集成

想在你的工程中集成 MMU 库，有两个方式：

#### 集成方案 1 ：

1. 将 mmu.c 添加到编译列表
2. 确保 mmu.h 和 mmu_config.h 和 mmu.c 在同一个目录内（或能够搜索到的路径）
3. 修改 mmu_config.h 文件，根据需要选择要使用的功能模块
4. 引用 mmu.h 头文件
5. 调用 API，使用 MMU 库，编译程序检查工作是否正常

#### 集成方案 2 ：

1. 将 mmu_single.h 添加您的代码目录
2. 引用头文件之前，参考 mmu_config.h 的内容，使用宏定义选择要使用的功能模块
3. 引用 mmu_single.h 头文件
4. 调用 API，使用 MMU 库，编译程序检查工作是否正常

### 1.5 功能裁剪定义

#### MMU_USE_PAMM
定义后可使用 PAMM 模块 - Point Array Memory Management [指针数组内存管理器]

#### MMU_USE_SAMM
定义后可使用 SAMM 模块 - Struct Array Memory Management [结构体数组内存管理器]

#### MMU_USE_MBMU
定义后可使用 MBMU 模块 - Memory Buffer Management Unit [内存缓冲区管理单元]

#### MMU_USE_BSMM
定义后可使用 BSMM 模块 - Blocks Struct Memory Management [数据块结构内存管理器]

#### MMU_USE_MMU256
定义后可使用 MMU256 模块 - Memory Management Unit 256 [固定 256 个数量的内存管理单元]

#### MMU_USE_MMU64K
定义后可使用 MMU64K 模块 - Memory Management Unit 64K [固定 65536 个数量的内存管理单元]

#### MMU_USE_MM256
定义后可使用 MM256 模块 - Memory Management 256 [内存管理器（固定大小的内存池，使用 MMU256 加速分配和释放）]

#### MMU_USE_MM64K
定义后可使用 MM64K 模块 - Memory Management 64K [内存管理器（固定大小的内存池，使用 MMU64K 加速分配和释放）]

#### MMU_USE_SSSTK
定义后可使用 SSSTK 模块 - Struct Static Stack [结构体静态栈，初始化时申请内存，栈最大深度固定]

#### MMU_USE_PSSTK
定义后可使用 PSSTK 模块 - Point Static Stack [指针静态栈，初始化时申请内存，栈最大深度固定]

#### MMU_USE_SDSTK
定义后可使用 SDSTK 模块 - Struct Dynamic Stack [结构体动态栈，结构体内存256个递增，栈最大深度不固定]

#### MMU_USE_PDSTK
定义后可使用 PDSTK 模块- Point Dynamic Stack [指针动态栈，结构体内存256个递增，栈最大深度不固定]

#### MMU_USE_LLIST
定义后可使用 LLIST 模块 - Linked List 双向链表 [使用 MM256 管理内存]

#### MMU_USE_AVLTREE
定义后可使用 AVLTREE 模块 - AVLTree AVL树 [使用 MM256 管理内存]

#### MMU_USE_RBTREE
定义后可使用 RBTREE 模块 - Red Black Tree 红黑树 [使用 MM256 管理内存]

#### MMU_USE_MP256
定义后可使用 MP256 模块 - Memory Pool 256（可变成员大小的内存池，使用 MM256 加速分配和释放）

#### MMU_USE_MP64K
定义后可使用 MP64K 模块 - Memory Pool 64K（可变成员大小的内存池，使用 MM64K 加速分配和释放）

#### MMU_USE_HASH32
定义后可使用 HASH32 模块 - nmhash32x Ver 2.0 哈希算法

#### MMU_USE_HASH64
定义后可使用 HASH64 模块 - rapidhash Ver 1.0 哈希算法

#### MMU_USE_AVLHT32
定义后可使用 AVLHT32 模块 - 基于 AVLTree 和 32 位长度哈希值实现的哈希表，可选 MP256 管理 Key 内存（默认使用mmu_malloc）

#### MMU_USE_AVLHT64
定义后可使用 AVLHT64 模块 - 基于 AVLTree 和 64 位长度哈希值实现的哈希表，可选 MP256 管理 Key 内存（默认使用mmu_malloc）

#### MMU_USE_RBHT32
定义后可使用 RBHT32 模块 - 基于 RBTree 和 32 位长度哈希值实现的哈希表，可选 MP256 管理 Key 内存（默认使用mmu_malloc）

#### MMU_USE_RBHT64
定义后可使用 RBHT64 模块 - 基于 RBTree 和 64 位长度哈希值实现的哈希表，可选 MP256 管理 Key 内存（默认使用mmu_malloc）

#### MMU_USE_CSQUE（功能暂未实现）
定义后可使用 CSQUE 模块 - Circular Sequence Queue 循环顺序队列

#### MMU_USE_EEQUE（功能暂未实现）
定义后可使用 EEQUE 模块 - Elastic Expansion Queue 弹性扩展队列 [使用 MM256 管理内存]

#### MMU_USE_PRQUE（功能暂未实现）
定义后可使用 PRQUE 模块 - Priority Queue 优先级队列 [使用红黑树实现]

#### MMU_USE_DGRAPH（功能暂未实现）
定义后可使用 DGRAPH 模块 - Directed Graph 单向图结构

#### MMU_USE_UGRAPH（功能暂未实现）
定义后可使用 UGRAPH 模块 - Undirected Graph 双向图结构

#### MMU_USE_DOM（功能暂未实现）
定义后可使用 DOM 模块 - Document Object Model 文档对象模型

### 1.6 已知缺陷

目前 MMU 还不是线程安全的，在多线程环境下使用各种数据管理器，需要添加线程锁，未来可能会考虑增加这部分功能。

### 1.7 后续计划

#### 完善使用文档

尽管 MMU 库并不复杂，头文件提供了清晰的功能定义，代码中含有大量注释，还提供了完整的测试代码，但由于没有详细的使用文档，开发者仍需较长的时间来阅读代码和测试范例学习如何使用它（这项工作已经在进行中了）。

#### Circular Sequence Queue 循环顺序队列

#### Elastic Expansion Queue 弹性扩展队列 [使用 MM256 管理内存]

#### Priority Queue 优先级队列 [使用红黑树实现]

#### Directed Graph 单向图结构

#### Undirected Graph 双向图结构

#### Document Object Model 通用文档对象模型

## 2. 使用说明

### 2.1 统一说明：

在 test 目录下能够找到每一个子模块的测试代码，这些测试代码也可以作为一种使用和调试方法的参考，其中即包含了常规 API，也包含了对内部数据的访问。

### [2.2 MMU 库](/docs/cn/0000_mmu.md)

### [2.3 PAMM（Point Array Memory Management）模块](/docs/cn/0100_pamm.md)

### [2.4 SAMM（Struct Array Memory Management）模块](/docs/cn/0101_samm.md)

### [2.5 MBMU（Memory Buffer Management Unit）模块](/docs/cn/0102_mbmu.md)

### [2.6 BSMM（Blocks Struct Memory Management）模块](/docs/cn/0103_bsmm.md)

### [2.7 MMU256（Memory Management Unit 256）模块](/docs/cn/0104_mmu256.md)

### [2.8 MMU64K（Memory Management Unit 64K）模块](/docs/cn/0105_mmu64k.md)

### [2.9 MM256（Memory Management 256）模块](/docs/cn/0106_mm256.md)

### [2.10 MM64K（Memory Management 64K）模块](/docs/cn/0107_mm64k.md)

### [2.11 SSSTK（Struct Static Stack）模块](/docs/cn/0108_ssstk.md)

### [2.12 PSSTK（Point Static Stack）模块](/docs/cn/0109_psstk.md)

### [2.13 SDSTK（Struct Dynamic Stack）模块](/docs/cn/0110_sdstk.md)

### [2.14 PDSTK（Point Dynamic Stack）模块](/docs/cn/0111_pdstk.md)

### [2.15 LLIST（Linked List）模块](/docs/cn/0112_llist.md)

### [2.16 AVLTree 模块](/docs/cn/0113_avltree.md)

### [2.17 RBTree（Red Black Tree）模块](/docs/cn/0114_rbtree.md)

### [2.18 Hash32（nmhash32x）模块](/docs/cn/0115_hash32.md)

### [2.19 Hash64（rapidhash）模块](/docs/cn/0116_hash64.md)

### [2.20 AVLHT32（AVLTree Hash Table 32）模块](/docs/cn/0117_avlht32.md)

### [2.21 AVLHT64（AVLTree Hash Table 64）模块](/docs/cn/0118_avlht64.md)

### [2.22 RBHT32（RBTree Hash Table 32）模块](/docs/cn/0119_rbht32.md)

### [2.23 RBHT64（RBTree Hash Table 64）模块](/docs/cn/0120_rbht64.md)

### [2.24 MP256（Memory Pool 256）模块](/docs/cn/0121_mp256.md)

### [2.25 MP64K（Memory Pool 64K）模块](/docs/cn/0122_mp64k.md)

## 3. 代码合并 & 联系作者

更多功能需求可以私信联系作者进行开发或集成，欢迎提交合并请求

作者邮箱：xywhsoft@qq.com