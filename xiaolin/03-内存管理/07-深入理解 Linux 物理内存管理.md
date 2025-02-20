在上篇文章 [《深入理解 Linux 虚拟内存管理》 (opens new window)](https://xiaolincoding.com/os/3_memory/linux_mem.html)中，我分别从进程用户态和内核态的角度详细深入地为大家介绍了 Linux 内核如何对进程虚拟内存空间进行布局以及管理的相关实现。在我们深入理解了虚拟内存之后，那么何不顺带着也探秘一下物理内存的管理呢？

所以本文的目的是在深入理解虚拟内存管理的基础之上继续带大家向前奋进，一举击破物理内存管理的知识盲区，使大家能够俯瞰整个 Linux 内存管理子系统的整体全貌。

而在正式开始物理内存管理的主题之前，我觉得有必须在带大家回顾下上篇文章中介绍的虚拟内存管理的相关知识，方便大家来回对比虚拟内存和物理内存，从而可以全面整体地掌握 Linux 内存管理子系统。

在上篇文章的一开始，我首先为大家展现了我们应用程序频繁接触到的虚拟内存地址，清晰地为大家介绍了到底什么是虚拟内存地址，以及虚拟内存地址分别在 32 位系统和 64 位系统中的具体表现形式：

![image.png](https://cdn.xiaolincoding.com//mysql/other/c95cdba56215610d4ba97744d37fa962.png)

![image.png](https://cdn.xiaolincoding.com//mysql/other/0b65150e4e49d06a76ecdd350ec79ec6.png)

在我们清楚了虚拟内存地址这个基本概念之后，随后我又抛出了一个问题：为什么我们要通过虚拟内存地址访问内存而不是直接通过物理地址访问？

原来是在多进程系统中直接操作物理内存地址的话，我们需要精确地知道每一个变量的位置都被安排在了哪里，而且还要注意当前进程在和多个进程同时运行的时候，不能共用同一个地址，否则就会造成地址冲突。

![image.png](https://cdn.xiaolincoding.com//mysql/other/7d6bcdb035e849153ece34f14c54f928.png)

而虚拟内存空间的引入正是为了解决多进程地址冲突的问题，使得进程与进程之间的虚拟内存地址空间相互隔离，互不干扰。每个进程都认为自己独占所有内存空间，将多进程之间的协同相关细节统统交给内核中的内存管理模块来处理，极大地解放了程序员的心智负担。这一切都是因为虚拟内存能够为进程提供内存地址空间隔离的功劳。

![image.png](https://cdn.xiaolincoding.com//mysql/other/27fc4998f1da0e948133ba0ef721a94f.png)

在我们清楚了虚拟内存空间引入的意义之后，我紧接着为大家介绍了**进程用户态**虚拟内存空间分别在 32 位机器和 64 位机器上的布局情况：

![32 位机器.png](https://cdn.xiaolincoding.com//mysql/other/b8b5a22f54d82f62bc9553fe2090c6d6.png)

![64 位机器.png](https://cdn.xiaolincoding.com//mysql/other/a84f3a5e84c68335f7f4824467fbdeda.png)

在了解了用户态虚拟内存空间的布局之后，紧接着我们又介绍了 Linux 内核如何对用户态虚拟内存空间进行管理以及相应的管理数据结构：

![image.png](https://cdn.xiaolincoding.com//mysql/other/f28079c8b8fbf1853570302bee2a1929.png)

在介绍完用户态虚拟内存空间的布局以及管理之后，我们随后又介绍了**内核态**虚拟内存空间的布局情况，并结合之前介绍的**用户态**虚拟内存空间，得到了 Linux 虚拟内存空间分别在 32 位和 64 位系统中的整体布局情况：

![32位系统中虚拟内存空间整体布局.png](https://cdn.xiaolincoding.com//mysql/other/779f8407ff85edd011ecb016e044fc28.png)

![64位系统中虚拟内存空间整体布局.png](https://cdn.xiaolincoding.com//mysql/other/f1333d3ef3642863576f544c7ddf720e.png)

在虚拟内存全部介绍完毕之后，为了能够承上启下，于是我继续在上篇文章的最后一个小节从计算机组成原理的角度介绍了物理内存的物理组织结构，方便让大家理解到底什么是真正的物理内存 ？物理内存地址到底是什么 ？由此为本文的主题 —— 物理内存的管理 ，埋下伏笔~~~

![内存IO单位.png](https://cdn.xiaolincoding.com//mysql/other/4f01d26470f66f1cd50ea2f3eda83dcc.png)

最后我介绍了 CPU 如何通过物理内存地址向物理内存读写数据的完整过程：

![CPU读取内存.png](https://cdn.xiaolincoding.com//mysql/other/0666683d16c291cfff9a1a1c02e8cdbd.png)

在我们回顾完上篇文章介绍的用户态和内核态虚拟内存空间的管理，以及物理内存在计算机中的真实组成结构之后，下面我就来正式地为大家介绍本文的主题 —— Linux 内核如何对物理内存进行管理

![本文概要.png](https://cdn.xiaolincoding.com//mysql/other/fac308ee48560ae3d60d8d3316dd2d43.png)

## [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_2-从-cpu-角度看物理内存模型)2. 从 CPU 角度看物理内存模型

在前边的文章中，我曾多次提到内核是以页为基本单位对物理内存进行管理的，通过将物理内存划分为一页一页的内存块，每页大小为 4K。一页大小的内存块在内核中用 struct page 结构体来进行管理，struct page 中封装了每页内存块的状态信息，比如：组织结构，使用信息，统计信息，以及与其他结构的关联映射信息等。

而为了快速索引到具体的物理内存页，内核为每个物理页 struct page 结构体定义了一个索引编号：PFN（Page Frame Number）。PFN 与 struct page 是一一对应的关系。

内核提供了两个宏来完成 PFN 与 物理页结构体 struct page 之间的相互转换。它们分别是 page_to_pfn 与 pfn_to_page。

内核中如何组织管理这些物理内存页 struct page 的方式我们称之为做物理内存模型，不同的物理内存模型，应对的场景以及 page_to_pfn 与 pfn_to_page 的计算逻辑都是不一样的。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_2-1-flatmem-平坦内存模型)2.1 FLATMEM 平坦内存模型

我们先把物理内存想象成一片地址连续的存储空间，在这一大片地址连续的内存空间中，内核将这块内存空间分为一页一页的内存块 struct page 。

由于这块物理内存是连续的，物理地址也是连续的，划分出来的这一页一页的物理页必然也是连续的，并且每页的大小都是固定的，所以我们很容易想到用一个数组来组织这些连续的物理内存页 struct page 结构，其在数组中对应的下标即为 PFN 。这种内存模型就叫做平坦内存模型 FLATMEM 。

![image.png](https://cdn.xiaolincoding.com//mysql/other/89fe28d0feb1cd31cbaad5352e1f43d9.png)

内核中使用了一个 mem_map 的全局数组用来组织所有划分出来的物理内存页。mem_map 全局数组的下标就是相应物理页对应的 PFN 。

在平坦内存模型下 ，page_to_pfn 与 pfn_to_page 的计算逻辑就非常简单，本质就是基于 mem_map 数组进行偏移操作。

```c
#if defined(CONFIG_FLATMEM)
#define __pfn_to_page(pfn) (mem_map + ((pfn)-ARCH_PFN_OFFSET))
#define __page_to_pfn(page) ((unsigned long)((page)-mem_map) + ARCH_PFN_OFFSET)
#endif
```

> ARCH_PFN_OFFSET 是 PFN 的起始偏移量。

Linux 早期使用的就是这种内存模型，因为在 Linux 发展的早期所需要管理的物理内存通常不大（比如几十 MB），那时的 Linux 使用平坦内存模型 FLATMEM 来管理物理内存就足够高效了。

> 内核中的默认配置是使用 FLATMEM 平坦内存模型。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_2-2-discontigmem-非连续内存模型)2.2 DISCONTIGMEM 非连续内存模型

FLATMEM 平坦内存模型只适合管理一整块连续的物理内存，而对于多块非连续的物理内存来说使用 FLATMEM 平坦内存模型进行管理则会造成很大的内存空间浪费。

因为 FLATMEM 平坦内存模型是利用 mem_map 这样一个全局数组来组织这些被划分出来的物理页 page 的，而对于物理内存存在大量不连续的内存地址区间这种情况时，这些不连续的内存地址区间就形成了内存空洞。

由于用于组织物理页的底层数据结构是 mem_map 数组，数组的特性又要求这些物理页是连续的，所以只能为这些内存地址空洞也分配 struct page 结构用来填充数组使其连续。

而每个 struct page 结构大部分情况下需要占用 40 字节（struct page 结构在不同场景下内存占用会有所不同，这一点我们后面再说），如果物理内存中存在的大块的地址空洞，那么为这些空洞而分配的 struct page 将会占用大量的内存空间，导致巨大的浪费。

![image.png](https://cdn.xiaolincoding.com//mysql/other/e29eec09835747396b91ee363cd8b697.png)

为了组织和管理这些不连续的物理内存，内核于是引入了 DISCONTIGMEM 非连续内存模型，用来消除这些不连续的内存地址空洞对 mem_map 的空间浪费。

在 DISCONTIGMEM 非连续内存模型中，内核将物理内存从宏观上划分成了一个一个的节点 node （微观上还是一页一页的物理页），每个 node 节点管理一块连续的物理内存。这样一来这些连续的物理内存页均被划归到了对应的 node 节点中管理，就避免了内存空洞造成的空间浪费。

![image.png](https://cdn.xiaolincoding.com//mysql/other/ae106d5d780328aae34d40560dc0442f.png)

内核中使用 struct pglist_data 表示用于管理连续物理内存的 node 节点（内核假设 node 中的物理内存是连续的），既然每个 node 节点中的物理内存是连续的，于是在每个 node 节点中还是采用 FLATMEM 平坦内存模型的方式来组织管理物理内存页。每个 node 节点中包含一个 `struct page *node_mem_map` 数组，用来组织管理 node 中的连续物理内存页。

```c
typedef struct pglist_data {
   #ifdef CONFIG_FLATMEM
	  struct page *node_mem_map;
   #endif
}
```

我们可以看出 DISCONTIGMEM 非连续内存模型其实就是 FLATMEM 平坦内存模型的一种扩展，在面对大块不连续的物理内存管理时，通过将每段连续的物理内存区间划归到 node 节点中进行管理，避免了为内存地址空洞分配 struct page 结构，从而节省了内存资源的开销。

由于引入了 node 节点这个概念，所以在 DISCONTIGMEM 非连续内存模型下 page_to_pfn 与 pfn_to_page 的计算逻辑就比 FLATMEM 内存模型下的计算逻辑多了一步定位 page 所在 node 的操作。

- 通过 arch_pfn_to_nid 可以根据物理页的 PFN 定位到物理页所在 node。
- 通过 page_to_nid 可以根据物理页结构 struct page 定义到 page 所在 node。

当定位到物理页 struct page 所在 node 之后，剩下的逻辑就和 FLATMEM 内存模型一模一样了。

```c
#if defined(CONFIG_DISCONTIGMEM)

#define __pfn_to_page(pfn)			\
({	unsigned long __pfn = (pfn);		\
	unsigned long __nid = arch_pfn_to_nid(__pfn);  \
	NODE_DATA(__nid)->node_mem_map + arch_local_page_offset(__pfn, __nid);\
})

#define __page_to_pfn(pg)						\
({	const struct page *__pg = (pg);					\
	struct pglist_data *__pgdat = NODE_DATA(page_to_nid(__pg));	\
	(unsigned long)(__pg - __pgdat->node_mem_map) +			\
	 __pgdat->node_start_pfn;					\
})
```

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_2-3-sparsemem-稀疏内存模型)2.3 SPARSEMEM 稀疏内存模型

随着内存技术的发展，内核可以支持物理内存的热插拔了（后面我会介绍），这样一来物理内存的不连续就变为常态了，在上小节介绍的 DISCONTIGMEM 内存模型中，其实每个 node 中的物理内存也不一定都是连续的。

![image.png](https://cdn.xiaolincoding.com//mysql/other/071573961065779baa2dcbb72bd8246a.png)

而且每个 node 中都有一套完整的内存管理系统，如果 node 数目多的话，那这个开销就大了，于是就有了对连续物理内存更细粒度的管理需求，为了能够更灵活地管理粒度更小的连续物理内存，SPARSEMEM 稀疏内存模型就此登场了。

SPARSEMEM 稀疏内存模型的核心思想就是对粒度更小的连续内存块进行精细的管理，用于管理连续内存块的单元被称作 section 。物理页大小为 4k 的情况下， section 的大小为 128M ，物理页大小为 16k 的情况下， section 的大小为 512M。

在内核中用 struct mem_section 结构体表示 SPARSEMEM 模型中的 section。

```c
struct mem_section {
	unsigned long section_mem_map;
        ...
}
```

由于 section 被用作管理小粒度的连续内存块，这些小的连续物理内存在 section 中也是通过数组的方式被组织管理，每个 struct mem_section 结构体中有一个 section_mem_map 指针用于指向 section 中管理连续内存的 page 数组。

SPARSEMEM 内存模型中的这些所有的 mem_section 会被存放在一个全局的数组中，并且每个 mem_section 都可以在系统运行时改变 offline / online （下线 / 上线）状态，以便支持内存的热插拔（hotplug）功能。

```c
#ifdef CONFIG_SPARSEMEM_EXTREME
extern struct mem_section *mem_section[NR_SECTION_ROOTS];
```

![image.png](https://cdn.xiaolincoding.com//mysql/other/e3956ea9e4dab708d57c7c183c6b91d6.png)

在 SPARSEMEM 稀疏内存模型下 page_to_pfn 与 pfn_to_page 的计算逻辑又发生了变化。

- 在 page_to_pfn 的转换中，首先需要通过 page_to_section 根据 struct page 结构定位到 mem_section 数组中具体的 section 结构。然后在通过 section_mem_map 定位到具体的 PFN。

> 在 struct page 结构中有一个 `unsigned long flags` 属性，在 flag 的高位 bit 中存储着 page 所在 mem_section 数组中的索引，从而可以定位到所属 section。

- 在 pfn_to_page 的转换中，首先需要通过 __pfn_to_section 根据 PFN 定位到 mem_section 数组中具体的 section 结构。然后在通过 PFN 在 section_mem_map 数组中定位到具体的物理页 Page 。

> PFN 的高位 bit 存储的是全局数组 mem_section 中的 section 索引，PFN 的低位 bit 存储的是 section_mem_map 数组中具体物理页 page 的索引。

```c
#if defined(CONFIG_SPARSEMEM)
/*
 * Note: section's mem_map is encoded to reflect its start_pfn.
 * section[i].section_mem_map == mem_map's address - start_pfn;
 */
#define __page_to_pfn(pg)					\
({	const struct page *__pg = (pg);				\
	int __sec = page_to_section(__pg);			\
	(unsigned long)(__pg - __section_mem_map_addr(__nr_to_section(__sec)));	\
})

#define __pfn_to_page(pfn)				\
({	unsigned long __pfn = (pfn);			\
	struct mem_section *__sec = __pfn_to_section(__pfn);	\
	__section_mem_map_addr(__sec) + __pfn;		\
})
#endif
```

从以上的内容介绍中，我们可以看出 SPARSEMEM 稀疏内存模型已经完全覆盖了前两个内存模型的所有功能，因此稀疏内存模型可被用于所有内存布局的情况。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_2-3-1-物理内存热插拔)2.3.1 物理内存热插拔

前面提到随着内存技术的发展，物理内存的热插拔 hotplug 在内核中得到了支持，由于物理内存可以动态的从主板中插入以及拔出，所以导致了物理内存的不连续已经成为常态，因此内核引入了 SPARSEMEM 稀疏内存模型以便应对这种情况，提供对更小粒度的连续物理内存的灵活管理能力。

本小节我就为大家介绍一下物理内存热插拔 hotplug 功能在内核中的实现原理，作为 SPARSEMEM 稀疏内存模型的扩展内容补充。

在大规模的集群中，尤其是现在我们处于云原生的时代，为了实现集群资源的动态均衡，可以通过物理内存热插拔的功能实现集群机器物理内存容量的动态增减。

集群的规模一大，那么物理内存出故障的几率也会大大增加，物理内存的热插拔对提供集群高可用性也是至关重要的。

从总体上来讲，内存的热插拔分为两个阶段：

- 物理热插拔阶段：这个阶段主要是从物理上将内存硬件插入（hot-add），拔出（hot-remove）主板的过程，其中涉及到硬件和内核的支持。
- 逻辑热插拔阶段：这一阶段主要是由内核中的内存管理子系统来负责，涉及到的主要工作为：如何动态的上线启用（online）刚刚 hot-add 的内存，如何动态下线（offline）刚刚 hot-remove 的内存。

物理内存拔出的过程需要关注的事情比插入的过程要多的多，实现起来也更加的困难, 这就好比在[《Java 技术栈中间件优雅停机方案设计与实现全景图》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247485290&idx=1&sn=58d7eb2c25c2e51b41e35a4ce4dab517&chksm=ce77c12df900483bf5a0edc6b31e710d6e3019deeeb788ed89faa69f9055872e85d518ee52f6&token=65686925&lang=zh_CN#rd)一文中我们讨论服务优雅启动，停机时提到的：优雅停机永远比优雅启动要考虑的场景要复杂的多，因为停机的时候，线上的服务正在承载着生产的流量需要确保做到业务无损。

同样的道理，物理内存插入比较好说，困难的是物理内存的动态拔出，因为此时即将要被拔出的物理内存中可能已经为进程分配了物理页，如何妥善安置这些已经被分配的物理页是一个棘手的问题。

前边我们介绍 SPARSEMEM 内存模型的时候提到，每个 mem_section 都可以在系统运行时改变 offline ，online 状态，以便支持内存的热插拔（hotplug）功能。 当 mem_section offline 时, 内核会把这部分内存隔离开, 使得该部分内存不可再被使用, 然后再把 mem_section 中已经分配的内存页迁移到其他 mem_section 的内存上. 。

![image.png](https://cdn.xiaolincoding.com//mysql/other/3011fad3b8da8311807d9bc0012a1757.png)

但是这里会有一个问题，就是并非所有的物理页都可以迁移，因为迁移意味着物理内存地址的变化，而内存的热插拔应该对进程来说是透明的，所以这些迁移后的物理页映射的虚拟内存地址是不能变化的。

这一点在进程的用户空间是没有问题的，因为进程在用户空间访问内存都是根据虚拟内存地址通过页表找到对应的物理内存地址，这些迁移之后的物理页，虽然物理内存地址发生变化，但是内核通过修改相应页表中虚拟内存地址与物理内存地址之间的映射关系，可以保证虚拟内存地址不会改变。

![image.png](https://cdn.xiaolincoding.com//mysql/other/6a7bb7d369e2155b01473d91d9ccf6d0.png)

但是在内核态的虚拟地址空间中，有一段直接映射区，在这段虚拟内存区域中虚拟地址与物理地址是直接映射的关系，虚拟内存地址直接减去一个固定的偏移量（0xC000 0000 ） 就得到了物理内存地址。

直接映射区中的物理页的虚拟地址会随着物理内存地址变动而变动, 因此这部分物理页是无法轻易迁移的，然而不可迁移的页会导致内存无法被拔除，因为无法妥善安置被拔出内存中已经为进程分配的物理页。那么内核是如何解决这个头疼的问题呢？

既然是这些不可迁移的物理页导致内存无法拔出，那么我们可以把内存分一下类，将内存按照物理页是否可迁移，划分为不可迁移页，可回收页，可迁移页。

> 大家这里需要记住一点，内核会将物理内存按照页面是否可迁移的特性进行分类，我后面在介绍内核如何避免内存碎片的时候还会在提到

然后在这些可能会被拔出的内存中只分配那些可迁移的内存页，这些信息会在内存初始化的时候被设置，这样一来那些不可迁移的页就不会包含在可能会拔出的内存中，当我们需要将这块内存热拔出时, 因为里边的内存页全部是可迁移的, 从而使内存可以被拔除。

## [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_3-从-cpu-角度看物理内存架构)3. 从 CPU 角度看物理内存架构

在上小节中我为大家介绍了三种物理内存模型，这三种物理内存模型是从 CPU 的视角来看待物理内存内部是如何布局，组织以及管理的，主角是物理内存。

在本小节中我为大家提供一个新的视角，这一次我们把物理内存看成一个整体，从 CPU 访问物理内存的角度来看一下物理内存的架构，并从 CPU 与物理内存的相对位置变化来看一下不同物理内存架构下对性能的影响。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_3-1-一致性内存访问-uma-架构)3.1 一致性内存访问 UMA 架构

我们在上篇文章 [《深入理解 Linux 虚拟内存管理》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247486732&idx=1&sn=435d5e834e9751036c96384f6965b328&chksm=ce77cb4bf900425d33d2adfa632a4684cf7a63beece166c1ffedc4fdacb807c9413e8c73f298&token=1468822011&lang=zh_CN#rd)的 “ 8.2 CPU 如何读写主存” 小节中提到 CPU 与内存之间的交互是通过总线完成的。

![CPU与内存之间的总线结构.png](https://cdn.xiaolincoding.com//mysql/other/46d08862052d4bbaf0d7bfa778f07978.png)

- 首先 CPU 将物理内存地址作为地址信号放到系统总线上传输。随后 IO bridge 将系统总线上的地址信号转换为存储总线上的电子信号。
- 主存感受到存储总线上的地址信号并通过存储控制器将存储总线上的物理内存地址 A 读取出来。
- 存储控制器通过物理内存地址定位到具体的存储器模块，从 DRAM 芯片中取出物理内存地址对应的数据。
- 存储控制器将读取到的数据放到存储总线上，随后 IO bridge 将存储总线上的数据信号转换为系统总线上的数据信号，然后继续沿着系统总线传递。
- CPU 芯片感受到系统总线上的数据信号，将数据从系统总线上读取出来并拷贝到寄存器中。

上图展示的是单核 CPU 访问内存的架构图，那么在多核服务器中多个 CPU 与内存之间的架构关系又是什么样子的呢？

![image.png](https://cdn.xiaolincoding.com//mysql/other/79170f0256ed383e3934b99a156911fd.png)

在 UMA 架构下，多核服务器中的多个 CPU 位于总线的一侧，所有的内存条组成一大片内存位于总线的另一侧，所有的 CPU 访问内存都要过总线，而且距离都是一样的，由于所有 CPU 对内存的访问距离都是一样的，所以在 UMA 架构下所有 CPU 访问内存的速度都是一样的。这种访问模式称为 SMP（Symmetric multiprocessing），即对称多处理器。

> 这里的一致性是指同一个 CPU 对所有内存的访问的速度是一样的。即一致性内存访问 UMA（Uniform Memory Access）。

但是随着多核技术的发展，服务器上的 CPU 个数会越来越多，而 UMA 架构下所有 CPU 都是需要通过总线来访问内存的，这样总线很快就会成为性能瓶颈，主要体现在以下两个方面：

1. 总线的带宽压力会越来越大，随着 CPU 个数的增多导致每个 CPU 可用带宽会减少
2. 总线的长度也会因此而增加，进而增加访问延迟

UMA 架构的优点很明显就是结构简单，所有的 CPU 访问内存速度都是一致的，都必须经过总线。然而它的缺点我刚刚也提到了，就是随着处理器核数的增多，总线的带宽压力会越来越大。解决办法就只能扩宽总线，然而成本十分高昂，未来可能仍然面临带宽压力。

为了解决以上问题，提高 CPU 访问内存的性能和扩展性，于是引入了一种新的架构：非一致性内存访问 NUMA（Non-uniform memory access）。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_3-2-非一致性内存访问-numa-架构)3.2 非一致性内存访问 NUMA 架构

在 NUMA 架构下，内存就不是一整片的了，而是被划分成了一个一个的内存节点 （NUMA 节点），每个 CPU 都有属于自己的本地内存节点，CPU 访问自己的本地内存不需要经过总线，因此访问速度是最快的。当 CPU 自己的本地内存不足时，CPU 就需要跨节点去访问其他内存节点，这种情况下 CPU 访问内存就会慢很多。

在 NUMA 架构下，任意一个 CPU 都可以访问全部的内存节点，访问自己的本地内存节点是最快的，但访问其他内存节点就会慢很多，这就导致了 CPU 访问内存的速度不一致，所以叫做非一致性内存访问架构。

![image.png](https://cdn.xiaolincoding.com//mysql/other/672a905532a801f8811040265cae2d0f-20230310000450243.png)

如上图所示，CPU 和它的本地内存组成了 NUMA 节点，CPU 与 CPU 之间通过 QPI（Intel QuickPath Interconnect）点对点完成互联，在 CPU 的本地内存不足的情况下，CPU 需要通过 QPI 访问远程 NUMA 节点上的内存控制器从而在远程内存节点上分配内存，这就导致了远程访问比本地访问多了额外的延迟开销（需要通过 QPI 遍历远程 NUMA 节点）。

> 在 NUMA 架构下，只有 DISCONTIGMEM 非连续内存模型和 SPARSEMEM 稀疏内存模型是可用的。而 UMA 架构下，前面介绍的三种内存模型都可以配置使用。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_3-2-1-numa-的内存分配策略)3.2.1 NUMA 的内存分配策略

NUMA 的内存分配策略是指在 NUMA 架构下 CPU 如何请求内存分配的相关策略，比如：是优先请求本地内存节点分配内存呢 ？还是优先请求指定的 NUMA 节点分配内存 ？是只能在本地内存节点分配呢 ？还是允许当本地内存不足的情况下可以请求远程 NUMA 节点分配内存 ？

|   内存分配策略    |                           策略描述                           |
| :---------------: | :----------------------------------------------------------: |
|     MPOL_BIND     |   必须在绑定的节点进行内存分配，如果内存不足，则进行 swap    |
|  MPOL_INTERLEAVE  |              本地节点和远程节点均可允许分配内存              |
|  MPOL_PREFERRED   | 优先在指定节点分配内存，当指定节点内存不足时，选择离指定节点最近的节点分配内存 |
| MPOL_LOCAL (默认) | 优先在本地节点分配，当本地节点内存不足时，可以在远程节点分配内存 |

我们可以在应用程序中通过 libnuma 共享库中的 API 调用 set_mempolicy 接口设置进程的内存分配策略。

```c
#include <numaif.h>

long set_mempolicy(int mode, const unsigned long *nodemask,
                          unsigned long maxnode);
```

- mode : 指定 NUMA 内存分配策略。
- nodemask：指定 NUMA 节点 Id。
- maxnode：指定最大 NUMA 节点 Id，用于遍历远程节点，实现跨 NUMA 节点分配内存。

> libnuma 共享库 API 文档：https://man7.org/linux/man-pages/man3/numa.3.html#top_of_page

> set_mempolicy 接口文档：https://man7.org/linux/man-pages/man2/set_mempolicy.2.html

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_3-2-2-numa-的使用简介)3.2.2 NUMA 的使用简介

在我们理解了物理内存的 NUMA 架构，以及在 NUMA 架构下的内存分配策略之后，本小节我来为大家介绍下如何正确的利用 NUMA 提升我们应用程序的性能。

前边我们介绍了这么多的理论知识，但是理论的东西总是很虚，正所谓眼见为实，大家一定想亲眼看一下 NUMA 架构在计算机中的具体表现形式，比如：在支持 NUMA 架构的机器上到底有多少个 NUMA 节点？每个 NUMA 节点包含哪些 CPU 核，具体是怎样的一个分布情况？

前面也提到 CPU 在访问本地 NUMA 节点中的内存时，速度是最快的。但是当访问远程 NUMA 节点，速度就会相对很慢，那么到底有多慢？本地节点与远程节点之间的访问速度差异具体是多少 ？

#### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_3-2-2-1-查看-numa-相关信息)3.2.2.1 查看 NUMA 相关信息

> numactl 文档：https://man7.org/linux/man-pages/man8/numactl.8.html

针对以上具体问题，`numactl -H` 命令可以给出我们想要的答案：

```vim
available: 4 nodes (0-3)
node 0 cpus: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 
node 0 size: 64794 MB
node 0 free: 55404 MB

node 1 cpus: 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
node 1 size: 65404 MB
node 1 free: 58642 MB

node 2 cpus: 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47
node 2 size: 65404 MB
node 2 free: 61181 MB

node 3 cpus:  48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63
node 3 size: 65402 MB
node 3 free: 55592 MB

node distances:
node   0   1   2   3
  0:  10  16  32  33
  1:  16  10  25  32
  2:  32  25  10  16
  3:  33  32  16  10
```

`numactl -H` 命令可以查看服务器的 NUMA 配置，上图中的服务器配置共包含 4 个 NUMA 节点（0 - 3），每个 NUMA 节点中包含 16个 CPU 核心，本地内存大小约为 64G。

大家可以关注下最后 `node distances:` 这一栏，node distances 给出了不同 NUMA 节点之间的访问距离，对角线上的值均为本地节点的访问距离 10 。比如 [0,0] 表示 NUMA 节点 0 的本地内存访问距离。

我们可以很明显的看到当出现跨 NUMA 节点访问的时候，访问距离就会明显增加，比如节点 0 访问节点 1 的距离 [0,1] 是16，节点 0 访问节点 3 的距离 [0,3] 是 33。距离越远，跨 NUMA 节点内存访问的延时越大。应用程序运行时应减少跨 NUMA 节点访问内存。

此外我们还可以通过 `numactl -s` 来查看 NUMA 的内存分配策略设置：

```vim
policy: default
preferred node: current
```

通过 `numastat` 还可以查看各个 NUMA 节点的内存访问命中率：

```bash
                           node0           node1            node2           node3
numa_hit              1296554257       918018444         1296574252       828018454
numa_miss                8541758        40297198           7544751        41267108
numa_foreign            40288595         8550361          41488585         8450375
interleave_hit             45651           45918            46654           49718
local_node            1231897031       835344122         1141898045       915354158
other_node              64657226        82674322           594657725       82675425 
```

- numa_hit ：内存分配在该节点中成功的次数。
- numa_miss : 内存分配在该节点中失败的次数。
- numa_foreign：表示其他 NUMA 节点本地内存分配失败，跨节点（numa_miss）来到本节点分配内存的次数。
- interleave_hit : 在 MPOL_INTERLEAVE 策略下，在本地节点分配内存的次数。
- local_node：进程在本地节点分配内存成功的次数。
- other_node：运行在本节点的进程跨节点在其他节点上分配内存的次数。

> numastat 文档：https://man7.org/linux/man-pages/man8/numastat.8.html

#### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_3-2-2-2-绑定-numa-节点)3.2.2.2 绑定 NUMA 节点

numactl 工具可以让我们应用程序指定运行在哪些 CPU 核心上，同时也可以指定我们的应用程序可以在哪些 NUMA 节点上分配内存。通过将应用程序与具体的 CPU 核心和 NUMA 节点绑定，从而可以提升程序的性能。

```vim
numactl --membind=nodes  --cpunodebind=nodes  command
```

- 通过 `--membind` 可以指定我们的应用程序只能在哪些具体的 NUMA 节点上分配内存，如果这些节点内存不足，则分配失败。
- 通过 `--cpunodebind` 可以指定我们的应用程序只能运行在哪些 NUMA 节点上。

```vim
numactl --physcpubind=cpus  command
```

另外我们还可以通过 `--physcpubind` 将我们的应用程序绑定到具体的物理 CPU 上。这个选项后边指定的参数我们可以通过 `cat /proc/cpuinfo` 输出信息中的 processor 这一栏查看。例如：通过 `numactl --physcpubind= 0-15 ./numatest.out` 命令将进程 numatest 绑定到 0~15 CPU 上执行。

我们可以通过 numactl 命令将 numatest 进程分别绑定在相同的 NUMA 节点上和不同的 NUMA 节点上，运行观察。

```vim
numactl --membind=0 --cpunodebind=0 ./numatest.out
numactl --membind=0 --cpunodebind=1 ./numatest.out
```

大家肯定一眼就能看出绑定在相同 NUMA 节点的进程运行会更快，因为通过前边对 NUMA 架构的介绍，我们知道 CPU 访问本地 NUMA 节点的内存是最快的。

> 除了 numactl 这个工具外，我们还可以通过共享库 libnuma 在程序中进行 NUMA 相关的操作。这里我就不演示了，感兴趣可以查看下 libnuma 的 API 文档：https://man7.org/linux/man-pages/man3/numa.3.html#top_of_page

## [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_4-内核如何管理-numa-节点)4. 内核如何管理 NUMA 节点

在前边我们介绍物理内存模型和物理内存架构的时候提到过：在 NUMA 架构下，只有 DISCONTIGMEM 非连续内存模型和 SPARSEMEM 稀疏内存模型是可用的。而 UMA 架构下，前面介绍的三种内存模型均可以配置使用。

无论是 NUMA 架构还是 UMA 架构在内核中都是使用相同的数据结构来组织管理的，在内核的内存管理模块中会把 UMA 架构当做只有一个 NUMA 节点的伪 NUMA 架构。这样一来这两种架构模式就在内核中被统一管理起来。

下面我先从最顶层的设计开始为大家介绍一下内核是如何管理这些 NUMA 节点的~~

![image.png](https://cdn.xiaolincoding.com//mysql/other/672a905532a801f8811040265cae2d0f.png)

> NUMA 节点中可能会包含多个 CPU，这些 CPU 均是物理 CPU，这点大家需要注意一下。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_4-1-内核如何统一组织-numa-节点)4.1 内核如何统一组织 NUMA 节点

首先我们来看第一个问题，在内核中是如何将这些 NUMA 节点统一管理起来的？

内核中使用了 struct pglist_data 这样的一个数据结构来描述 NUMA 节点，**在内核 2.4 版本之前**，内核是使用一个 pgdat_list 单链表将这些 NUMA 节点串联起来的，单链表定义在 `/include/linux/mmzone.h` 文件中：

```c
extern pg_data_t *pgdat_list;
```

每个 NUMA 节点的数据结构 struct pglist_data 中有一个 next 指针，用于将这些 NUMA 节点串联起来形成 pgdat_list 单链表，链表的末尾节点 next 指针指向 NULL。

```c
typedef struct pglist_data {
    struct pglist_data *pgdat_next;
}
```

**在内核 2.4 之后的版本中**，内核移除了 struct pglist_data 结构中的 pgdat_next 之指针, 同时也删除了 pgdat_list 单链表。取而代之的是，内核使用了一个大小为 MAX_NUMNODES ，类型为 struct pglist_data 的全局数组 node_data[] 来管理所有的 NUMA 节点。

![img](https://cdn.xiaolincoding.com//mysql/other/41eaa83e504823a49d921ed7be0b83cc.png)

全局数组 node_data[] 定义在文件 `/arch/arm64/include/asm/mmzone.h`中：

```c
#ifdef CONFIG_NUMA
extern struct pglist_data *node_data[];
#define NODE_DATA(nid)		(node_data[(nid)])
```

> NODE_DATA(nid) 宏可以通过 NUMA 节点的 nodeId，找到对应的 struct pglist_data 结构。

node_data[] 数组大小 MAX_NUMNODES 定义在 `/include/linux/numa.h`文件中：

```c
#ifdef CONFIG_NODES_SHIFT
#define NODES_SHIFT     CONFIG_NODES_SHIFT
#else
#define NODES_SHIFT     0
#endif
#define MAX_NUMNODES    (1 << NODES_SHIFT)
```

> UMA 架构下 NODES_SHIFT 为 0 ，所以内核中只用一个 NUMA 节点来管理所有物理内存。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_4-2-numa-节点描述符-pglist-data-结构)4.2 NUMA 节点描述符 pglist_data 结构

```c
typedef struct pglist_data {
    // NUMA 节点id
    int node_id;
    // 指向 NUMA 节点内管理所有物理页 page 的数组
    struct page *node_mem_map;
    // NUMA 节点内第一个物理页的 pfn
    unsigned long node_start_pfn;
    // NUMA 节点内所有可用的物理页个数（不包含内存空洞）
    unsigned long node_present_pages;
    // NUMA 节点内所有的物理页个数（包含内存空洞）
    unsigned long node_spanned_pages; 
    // 保证多进程可以并发安全的访问 NUMA 节点
    spinlock_t node_size_lock;
        .............
}
```

node_id 表示 NUMA 节点的 id，我们可以通过 numactl -H 命令的输出结果查看节点 id。从 0 开始依次对 NUMA 节点进行编号。

struct page 类型的数组 node_mem_map 中包含了 NUMA节点内的所有的物理内存页。

![image.png](https://cdn.xiaolincoding.com//mysql/other/db3d57220d493ece4fc9eef49bbbd69d.png)

node_start_pfn 指向 NUMA 节点内第一个物理页的 PFN，系统中所有 NUMA 节点中的物理页都是依次编号的，每个物理页的 PFN 都是**全局唯一的**（不只是其所在 NUMA 节点内唯一）

![image.png](https://cdn.xiaolincoding.com//mysql/other/4d9142121fb3308b4f9c79675ff536a9.png)

node_present_pages 用于统计 NUMA 节点内所有真正可用的物理页面数量（不包含内存空洞）。

由于 NUMA 节点内包含的物理内存并不总是连续的，可能会包含一些内存空洞，node_spanned_pages 则是用于统计 NUMA 节点内所有的内存页，包含不连续的物理内存地址（内存空洞）的页面数。

![image.png](https://cdn.xiaolincoding.com//mysql/other/b1ec8d6f4a2f6955654f791052c1a006.png)

以上内容是我从整体上为大家介绍的 NUMA 节点如何管理节点内部的本地内存。事实上内核还会将 NUMA 节点中的本地内存做近一步的划分。那么为什么要近一步划分呢？

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_4-3-numa-节点物理内存区域的划分)4.3 NUMA 节点物理内存区域的划分

我们都知道内核对物理内存的管理都是以页为最小单位来管理的，每页默认 4K 大小，理想状况下任何种类的数据都可以存放在任何页框中，没有什么限制。比如：存放内核数据，用户数据，磁盘缓冲数据等。

但是实际的计算机体系结构受到硬件方面的制约，间接导致限制了页框的使用方式。

比如在 X86 体系结构下，ISA 总线的 DMA （直接内存存取）控制器，只能对内存的前16M 进行寻址，这就导致了 ISA 设备不能在整个 32 位地址空间中执行 DMA，只能使用物理内存的前 16M 进行 DMA 操作。

因此直接映射区的前 16M 专门让内核用来为 DMA 分配内存，这块 16M 大小的内存区域我们称之为 ZONE_DMA。

> 用于 DMA 的内存必须从 ZONE_DMA 区域中分配。

![image.png](https://cdn.xiaolincoding.com//mysql/other/0c7c49a5019cc2d2ab4ee54256ce8cf7.png)

而直接映射区中剩下的部分也就是从 16M 到 896M（不包含 896M）这段区域，我们称之为 ZONE_NORMAL。从字面意义上我们可以了解到，这块区域包含的就是正常的页框（没有任何使用限制）。

ZONE_NORMAL 由于也是属于直接映射区的一部分，对应的物理内存 16M 到 896M 这段区域也是被直接映射至内核态虚拟内存空间中的 3G + 16M 到 3G + 896M 这段虚拟内存上。

而物理内存 896M 以上的区域被内核划分为 ZONE_HIGHMEM 区域，我们称之为高端内存。

由于内核虚拟内存空间中的前 896M 虚拟内存已经被直接映射区所占用，而在 32 体系结构下内核虚拟内存空间总共也就 1G 的大小，这样一来内核剩余可用的虚拟内存空间就变为了 1G - 896M = 128M。

显然物理内存中剩下的这 3200M 大小的 ZONE_HIGHMEM 区域无法继续通过直接映射的方式映射到这 128M 大小的虚拟内存空间中。

这样一来物理内存中的 ZONE_HIGHMEM 区域就只能采用动态映射的方式映射到 128M 大小的内核虚拟内存空间中，也就是说只能动态的一部分一部分的分批映射，先映射正在使用的这部分，使用完毕解除映射，接着映射其他部分。

所以内核会根据各个物理内存区域的功能不同，将 NUMA 节点内的物理内存主要划分为以下四个物理内存区域：

1. ZONE_DMA：用于那些无法对全部物理内存进行寻址的硬件设备，进行 DMA 时的内存分配。例如前边介绍的 ISA 设备只能对物理内存的前 16M 进行寻址。该区域的长度依赖于具体的处理器类型。
2. ZONE_DMA32：与 ZONE_DMA 区域类似，该区域内的物理页面可用于执行 DMA 操作，不同之处在于该区域是提供给 32 位设备（只能寻址 4G 物理内存）执行 DMA 操作时使用的。该区域只在 64 位系统中起作用，因为只有在 64 位系统中才会专门为 32 位设备提供专门的 DMA 区域。
3. ZONE_NORMAL：这个区域的物理页都可以直接映射到内核中的虚拟内存，由于是线性映射，内核可以直接进行访问。
4. ZONE_HIGHMEM：这个区域包含的物理页就是我们说的高端内存，内核不能直接访问这些物理页，这些物理页需要动态映射进内核虚拟内存空间中（非线性映射）。该区域只在 32 位系统中才会存在，因为 64 位系统中的内核虚拟内存空间太大了（128T），都可以进行直接映射。

以上这些物理内存区域的划分定义在 `/include/linux/mmzone.h` 文件中：

```c
enum zone_type {
#ifdef CONFIG_ZONE_DMA
	ZONE_DMA,
#endif
#ifdef CONFIG_ZONE_DMA32
	ZONE_DMA32,
#endif
	ZONE_NORMAL,
#ifdef CONFIG_HIGHMEM
	ZONE_HIGHMEM,
#endif
	ZONE_MOVABLE,
#ifdef CONFIG_ZONE_DEVICE
	ZONE_DEVICE,
#endif
    // 充当结束标记, 在内核中想要迭代系统中所有内存域时, 会用到该常量
	__MAX_NR_ZONES

};
```

大家可能注意到内核中定义的 zone_type 除了上边为大家介绍的四个物理内存区域，又多出了两个区域：ZONE_MOVABLE 和 ZONE_DEVICE。

ZONE_DEVICE 是为支持热插拔设备而分配的非易失性内存（ Non Volatile Memory ），也可用于内核崩溃时保存相关的调试信息。

ZONE_MOVABLE 是内核定义的一个**虚拟内存区域**，该区域中的物理页可以来自于上边介绍的几种真实的物理区域。该区域中的页全部都是可以迁移的，主要是为了防止内存碎片和支持内存的热插拔。

**既然有了这些实际的物理内存区域，那么内核为什么又要划分出一个 ZONE_MOVABLE 这样的虚拟内存区域呢** ？

因为随着系统的运行会伴随着不同大小的物理内存页的分配和释放，这种内存不规则的分配释放随着系统的长时间运行就会导致内存碎片，内存碎片会使得系统在明明有足够内存的情况下，依然无法为进程分配合适的内存。

![image.png](https://cdn.xiaolincoding.com//mysql/other/53b0a5e41c05025582ebad44f7eac51d.png)

如上图所示，假如现在系统一共有 16 个物理内存页，当前系统只是分配了 3 个物理页，那么在当前系统中还剩余 13 个物理内存页的情况下，如果内核想要分配 8 个连续的物理页的话，就会由于内存碎片的存在导致分配失败。（只能分配最多 4 个连续的物理页）

> 内核中请求分配的物理页面数只能是 2 的次幂！！

如果这些物理页处于 ZONE_MOVABLE 区域，它们就可以被迁移，内核可以通过迁移页面来避免内存碎片的问题：

![image.png](https://cdn.xiaolincoding.com//mysql/other/4075ce5ce90abb12abcbe8ef72852884.png)

内核通过迁移页面来规整内存，这样就可以避免内存碎片，从而得到一大片连续的物理内存，以满足内核对大块连续内存分配的请求。**所以这就是内核需要根据物理页面是否能够迁移的特性，而划分出 ZONE_MOVABLE 区域的目的**。

到这里，我们已经清楚了 NUMA 节点中物理内存区域的划分，下面我们继续回到 struct pglist_data 结构中看下内核如何在 NUMA 节点中组织这些划分出来的内存区域：

```c
typedef struct pglist_data {
  // NUMA 节点中的物理内存区域个数
	int nr_zones; 
  // NUMA 节点中的物理内存区域
	struct zone node_zones[MAX_NR_ZONES];
  // NUMA 节点的备用列表
	struct zonelist node_zonelists[MAX_ZONELISTS];
} pg_data_t;
```

nr_zones 用于统计 NUMA 节点内包含的物理内存区域个数，**不是每个 NUMA 节点都会包含以上介绍的所有物理内存区域，NUMA 节点之间所包含的物理内存区域个数是不一样的**。

事实上只有第一个 NUMA 节点可以包含所有的物理内存区域，其它的节点并不能包含所有的区域类型，因为有些内存区域比如：ZONE_DMA，ZONE_DMA32 必须从物理内存的起点开始。这些在物理内存开始的区域可能已经被划分到第一个 NUMA 节点了，后面的物理内存才会被依次划分给接下来的 NUMA 节点。因此后面的 NUMA 节点并不会包含 ZONE_DMA，ZONE_DMA32 区域。

![image.png](https://cdn.xiaolincoding.com//mysql/other/194e75f0490fe3b8b9bc4b0e1e731c33.png)

ZONE_NORMAL、ZONE_HIGHMEM 和 ZONE_MOVABLE 是可以出现在所有 NUMA 节点上的。

![image.png](https://cdn.xiaolincoding.com//mysql/other/d69d03bbccd05ad1eef0ac25f6e92ac5-20230310000145255.png)

node_zones[MAX_NR_ZONES] 数组包含了 NUMA 节点中的所有物理内存区域，物理内存区域在内核中的数据结构是 struct zone 。

node_zonelists[MAX_ZONELISTS] 是 struct zonelist 类型的数组，它包含了备用 NUMA 节点和这些备用节点中的物理内存区域。备用节点是按照访问距离的远近，依次排列在 node_zonelists 数组中，数组第一个备用节点是访问距离最近的，这样当本节点内存不足时，可以从备用 NUMA 节点中分配内存。

> 各个 NUMA 节点之间的内存分配情况我们可以通过前边介绍的 `numastat` 命令查看。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_4-4-numa-节点中的内存规整与回收)4.4 NUMA 节点中的内存规整与回收

内存可以说是计算机系统中最为宝贵的资源了，再怎么多也不够用，当系统运行时间长了之后，难免会遇到内存紧张的时候，这时候就需要内核将那些不经常使用的内存页面回收起来，或者将那些可以迁移的页面进行内存规整，从而可以腾出连续的物理内存页面供内核分配。

内核会为每个 NUMA 节点分配一个 kswapd 进程用于回收不经常使用的页面，还会为每个 NUMA 节点分配一个 kcompactd 进程用于内存的规整避免内存碎片。

```c
typedef struct pglist_data {
        .........
    // 页面回收进程
    struct task_struct *kswapd;
    wait_queue_head_t kswapd_wait;
    // 内存规整进程
    struct task_struct *kcompactd;
    wait_queue_head_t kcompactd_wait;

        ..........
} pg_data_t;
```

NUMA 节点描述符 struct pglist_data 结构中的 struct task_struct *kswapd 属性用于指向内核为 NUMA 节点分配的 kswapd 进程。

kswapd_wait 用于 kswapd 进程周期性回收页面时使用到的等待队列。

同理 struct task_struct *kcompactd 用于指向内核为 NUMA 节点分配的 kcompactd 进程。

kcompactd_wait 用于 kcompactd 进程周期性规整内存时使用到的等待队列。

> 本小节我主要为大家介绍 NUMA 节点的数据结构 struct pglist_data。详细的内存回收会在本文后面的章节单独介绍。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_4-5-numa-节点的状态-node-states)4.5 NUMA 节点的状态 node_states

如果系统中的 NUMA 节点多于一个，内核会维护一个位图 node_states，用于维护各个 NUMA 节点的状态信息。

> 如果系统中只有一个 NUMA 节点，则没有节点位图。

节点位图以及节点的状态掩码值定义在 `/include/linux/nodemask.h` 文件中：

```c
typedef struct { DECLARE_BITMAP(bits, MAX_NUMNODES); } nodemask_t;
extern nodemask_t node_states[NR_NODE_STATES];
```

节点的状态可通过以下掩码表示：

```c
enum node_states {
	N_POSSIBLE,		/* The node could become online at some point */
	N_ONLINE,		/* The node is online */
	N_NORMAL_MEMORY,	/* The node has regular memory */
#ifdef CONFIG_HIGHMEM
	N_HIGH_MEMORY,		/* The node has regular or high memory */
#else
	N_HIGH_MEMORY = N_NORMAL_MEMORY,
#endif
#ifdef CONFIG_MOVABLE_NODE
	N_MEMORY,		/* The node has memory(regular, high, movable) */
#else
	N_MEMORY = N_HIGH_MEMORY,
#endif
	N_CPU,		/* The node has one or more cpus */
	NR_NODE_STATES
};
```

N_POSSIBLE 表示 NUMA 节点在某个时刻可以变为 online 状态，N_ONLINE 表示 NUMA 节点当前的状态为 online 状态。

我们在本文《2.3.1 物理内存热插拔》小节中提到，在稀疏内存模型中，NUMA 节点的状态可以在系统运行的过程中随时切换 online ，offline 的状态，用来支持内存的热插拔。

![image.png](https://cdn.xiaolincoding.com//mysql/other/90a21a0f483fcbb44ac8e34ae325132e.png)

N_NORMAL_MEMORY 表示节点没有高端内存，只有 ZONE_NORMAL 内存区域。

N_HIGH_MEMORY 表示节点有 ZONE_NORMAL 内存区域或者有 ZONE_HIGHMEM 内存区域。

N_MEMORY 表示节点有 ZONE_NORMAL，ZONE_HIGHMEM，ZONE_MOVABLE 内存区域。

N_CPU 表示节点包含一个或多个 CPU。

此外内核还提供了两个辅助函数用于设置或者清除指定节点的特定状态：

```c
static inline void node_set_state(int node, enum node_states state)
static inline void node_clear_state(int node, enum node_states state)
```

内核提供了 for_each_node_state 宏用于迭代处于特定状态的所有 NUMA 节点。

```c
#define for_each_node_state(__node, __state) \
	for_each_node_mask((__node), node_states[__state])
```

比如：for_each_online_node 用于迭代所有 online 的 NUMA 节点：

```c
#define for_each_online_node(node) for_each_node_state(node, N_ONLINE)
```

## [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_5-内核如何管理-numa-节点中的物理内存区域)5. 内核如何管理 NUMA 节点中的物理内存区域

![image.png](https://cdn.xiaolincoding.com//mysql/other/d69d03bbccd05ad1eef0ac25f6e92ac5-20230310000124245.png)

在前边《4.3 NUMA 节点物理内存区域的划分》小节的介绍中，由于实际的计算机体系结构受到硬件方面的制约，间接限制了页框的使用方式。于是内核会根据各个物理内存区域的功能不同，将 NUMA 节点内的物理内存划分为：ZONE_DMA，ZONE_DMA32，ZONE_NORMAL，ZONE_HIGHMEM 这几个物理内存区域。

> ZONE_MOVABLE 区域是内核从逻辑上的划分，区域中的物理页面来自于上述几个内存区域，目的是避免内存碎片和支持内存热插拔（前边我已经介绍过了）。

我们可以通过 `cat /proc/zoneinfo | grep Node` 命令来查看 NUMA 节点中内存区域的分布情况：

![image.png](https://cdn.xiaolincoding.com//mysql/other/f1081fda840d287ca41c8e5ec9cf2f49.png)

> 我使用的服务器是 64 位，所以不包含 ZONE_HIGHMEM 区域。

通过 `cat /proc/zoneinfo` 命令来查看系统中各个 NUMA 节点中的各个内存区域的内存使用情况：

> 下图中我们以 NUMA Node 0 中的 ZONE_NORMAL 区域为例说明，大家只需要浏览一个大概，图中每个字段的含义我会在本小节的后面一一为大家介绍~~~

![image.png](https://cdn.xiaolincoding.com//mysql/other/f2719bbfa3c5bcd244d5f839841279f8-20230310000114083.png)

内核中用于描述和管理 NUMA 节点中的物理内存区域的结构体是 struct zone，上图中显示的 ZONE_NORMAL 区域中，物理内存使用统计的相关数据均来自于 struct zone 结构体，我们先来看一下内核对 struct zone 结构体的整体布局情况：

```c
struct zone {

    .............省略..............

    ZONE_PADDING(_pad1_)

    .............省略..............

    ZONE_PADDING(_pad2_)

    .............省略..............

    ZONE_PADDING(_pad3_)

    .............省略..............

} ____cacheline_internodealigned_in_smp;
```

由于 struct zone 结构体在内核中是一个访问非常频繁的结构体，在多处理器系统中，会有不同的 CPU 同时大量频繁的访问 struct zone 结构体中的不同字段。

因此内核对 struct zone 结构体的设计是相当考究的，将这些频繁访问的字段信息归类为 4 个部分，并通过 ZONE_PADDING 来分割。

目的是通过 ZONE_PADDING 来填充字节，将这四个部分，分别填充到不同的 CPU 高速缓存行（cache line）中，使得它们各自独占 cache line，提高访问性能。

根据前边物理内存区域划分的相关内容介绍，我们知道内核会把 NUMA 节点中的物理内存区域顶多划分为 ZONE_DMA，ZONE_DMA32，ZONE_NORMAL，ZONE_HIGHMEM 这几个物理内存区域。因此 struct zone 的实例在内核中会相对比较少，通过 ZONE_PADDING 填充字节，带来的 struct zone 结构体实例内存占用增加是可以忽略不计的。

在结构体的最后内核还是用了 `____cacheline_internodealigned_in_smp` 编译器关键字来实现最优的高速缓存行对齐方式。

> 关于 CPU 高速缓存行对齐的详细内容，感兴趣的同学可以回看下我之前的文章 [《一文聊透对象在JVM中的内存布局，以及内存对齐和压缩指针的原理及应用》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247484304&idx=1&sn=54bf0d07e69c5621c145afaece8f50d6&chksm=ce77c5d7f9004cc1249a03dfd0fb12b7d75171f1b87acea1fa44bbb11ca374b6f42a66fa274d&token=174519396&lang=zh_CN#rd)。

我为了使大家能够更好地理解内核如何使用 struct zone 结构体来描述内存区域，从而把结构体中的字段按照一定的层次结构重新排列介绍，这并不是原生的字段对齐方式，这一点需要大家注意！！！

```c
struct zone {
    // 防止并发访问该内存区域
    spinlock_t      lock;
    // 内存区域名称：Normal ，DMA，HighMem
    const char      *name;
    // 指向该内存区域所属的 NUMA 节点
    struct pglist_data  *zone_pgdat;
    // 属于该内存区域中的第一个物理页 PFN
    unsigned long       zone_start_pfn;
    // 该内存区域中所有的物理页个数（包含内存空洞）
    unsigned long       spanned_pages;
    // 该内存区域所有可用的物理页个数（不包含内存空洞）
    unsigned long       present_pages;
    // 被伙伴系统所管理的物理页数
    atomic_long_t       managed_pages;
    // 伙伴系统的核心数据结构
    struct free_area    free_area[MAX_ORDER];
    // 该内存区域内存使用的统计信息
    atomic_long_t       vm_stat[NR_VM_ZONE_STAT_ITEMS];
} ____cacheline_internodealigned_in_smp;
```

struct zone 是会被内核频繁访问的一个结构体，在多核处理器中，多个 CPU 会并发访问 struct zone，为了防止并发访问，内核使用了一把 spinlock_t lock 自旋锁来防止并发错误以及不一致。

name 属性会根据该内存区域的类型不同保存内存区域的名称，比如：Normal ，DMA，HighMem 等。

前边我们介绍 NUMA 节点的描述符 struct pglist_data 的时候提到，pglist_data 通过 struct zone 类型的数组 node_zones 将 NUMA 节点中划分的物理内存区域连接起来。

```c
typedef struct pglist_data {
    // NUMA 节点中的物理内存区域个数
    int nr_zones; 
    // NUMA 节点中的物理内存区域
    struct zone node_zones[MAX_NR_ZONES];
}
```

这些物理内存区域也会通过 struct zone 中的 zone_pgdat 指向自己所属的 NUMA 节点。

![image.png](https://cdn.xiaolincoding.com//mysql/other/c94df404b28ec5d4e1fc32e30fb0764b.png)

NUMA 节点 struct pglist_data 结构中的 node_start_pfn 指向 NUMA 节点内第一个物理页的 PFN。同理物理内存区域 struct zone 结构中的 zone_start_pfn 指向的是该内存区域内所管理的第一个物理页面 PFN 。

后面的属性也和 NUMA 节点对应的字段含义一样，比如：spanned_pages 表示该内存区域内所有的物理页总数（包含内存空洞），通过 `spanned_pages = zone_end_pfn - zone_start_pfn` 计算得到。

present_pages 则表示该内存区域内所有实际可用的物理页面总数（不包含内存空洞），通过 `present_pages = spanned_pages - absent_pages(pages in holes)` 计算得到。

在 NUMA 架构下，物理内存被划分成了一个一个的内存节点（NUMA 节点），在每个 NUMA 节点内部又将其所管理的物理内存按照功能不同划分成了不同的内存区域，每个内存区域管理一片用于具体功能的物理内存，而内核会为每一个内存区域分配一个伙伴系统用于管理该内存区域下物理内存的分配和释放。

> 物理内存在内核中管理的层级关系为：`None -> Zone -> page`

![image.png](https://cdn.xiaolincoding.com//mysql/other/bba0c0b540cbfcbbb709077d2a22ee3d-20230310000055320.png)

struct zone 结构中的 managed_pages 用于表示该内存区域内被伙伴系统所管理的物理页数量。

数组 free_area[MAX_ORDER] 是伙伴系统的核心数据结构，我会在后面的系列文章中详细为大家介绍伙伴系统的实现。

vm_stat 维护了该内存区域物理内存的使用统计信息，前边介绍的 `cat /proc/zoneinfo`命令的输出数据就来源于这个 vm_stat。

![image.png](https://cdn.xiaolincoding.com//mysql/other/f2719bbfa3c5bcd244d5f839841279f8.png)

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_5-1-物理内存区域中的预留内存)5.1 物理内存区域中的预留内存

除了前边介绍的关于物理内存区域的这些基本信息之外，每个物理内存区域 struct zone 还为操作系统预留了一部分内存，这部分预留的物理内存用于内核的一些核心操作，这些操作无论如何是不允许内存分配失败的。

什么意思呢？内核中关于内存分配的场景无外乎有两种方式：

1. 当进程请求内核分配内存时，如果此时内存比较充裕，那么进程的请求会被立刻满足，如果此时内存已经比较紧张，内核就需要将一部分不经常使用的内存进行回收，从而腾出一部分内存满足进程的内存分配的请求，在这个回收内存的过程中，进程会一直阻塞等待。
2. 另一种内存分配场景，进程是不允许阻塞的，内存分配的请求必须马上得到满足，比如执行中断处理程序或者执行持有自旋锁等临界区内的代码时，进程就不允许睡眠，因为中断程序无法被重新调度。这时就需要内核提前为这些核心操作预留一部分内存，当内存紧张时，可以使用这部分预留的内存给这些操作分配。

```c
struct zone {
             ...........

    unsigned long nr_reserved_highatomic;
    long lowmem_reserve[MAX_NR_ZONES];
            
             ...........
}
```

nr_reserved_highatomic 表示的是该内存区域内预留内存的大小，范围为 128 到 65536 KB 之间。

lowmem_reserve 数组则是用于规定每个内存区域必须为自己保留的物理页数量，防止更高位的内存区域对自己的内存空间进行过多的侵占挤压。

那么什么是高位内存区域 ？什么是低位内存区域 ？ 高位内存区域为什么会对低位内存区域进行侵占挤压呢 ？

因为物理内存区域比如前边介绍的 ZONE_DMA，ZONE_DMA32，ZONE_NORMAL，ZONE_HIGHMEM 这些都是针对物理内存进行的划分，所谓的低位内存区域和高位内存区域其实还是按照物理内存地址从低到高进行排列布局：

![image.png](https://cdn.xiaolincoding.com//mysql/other/94d5c632fb1d35c883bc8aaac886eed3.png)

根据物理内存地址的高低，低位内存区域到高位内存区域的顺序依次是：ZONE_DMA，ZONE_DMA32，ZONE_NORMAL，ZONE_HIGHMEM。

**高位内存区域为什么会对低位内存区域进行挤压呢** ？

一些用于特定功能的物理内存必须从特定的内存区域中进行分配，比如外设的 DMA 控制器就必须从 ZONE_DMA 或者 ZONE_DMA32 中分配内存。

但是一些用于常规用途的物理内存则可以从多个物理内存区域中进行分配，当 ZONE_HIGHMEM 区域中的内存不足时，内核可以从 ZONE_NORMAL 进行内存分配，ZONE_NORMAL 区域内存不足时可以进一步降级到 ZONE_DMA 区域进行分配。

而低位内存区域中的内存总是宝贵的，内核肯定希望这些用于常规用途的物理内存从常规内存区域中进行分配，这样能够节省 ZONE_DMA 区域中的物理内存保证 DMA 操作的内存使用需求，但是如果内存很紧张了，高位内存区域中的物理内存不够用了，那么内核就会去占用挤压其他内存区域中的物理内存从而满足内存分配的需求。

但是内核又不会允许高位内存区域对低位内存区域的无限制挤压占用，因为毕竟低位内存区域有它特定的用途，所以每个内存区域会给自己预留一定的内存，防止被高位内存区域挤压占用。而每个内存区域为自己预留的这部分内存就存储在 lowmem_reserve 数组中。

每个内存区域是按照一定的比例来计算自己的预留内存的，这个比例我们可以通过 `cat /proc/sys/vm/lowmem_reserve_ratio` 命令查看：

![image.png](https://cdn.xiaolincoding.com//mysql/other/7a2e4153e2e6ab32232856000d8dc630.png)

从左到右分别代表了 ZONE_DMA，ZONE_DMA32，ZONE_NORMAL，ZONE_MOVABLE，ZONE_DEVICE 物理内存区域的预留内存比例。

> 我使用的服务器是 64 位，所以没有 ZONE_HIGHMEM 区域。

**那么每个内存区域如何根据各自的 lowmem_reserve_ratio 来计算各自区域中的预留内存大小呢**？

为了让大家更好的理解，下面我们以 ZONE_DMA，ZONE_NORMAL，ZONE_HIGHMEM 这三个物理内存区域举例，它们的 lowmem_reserve_ratio 分别为 256，32，0。它们的大小分别是：8M，64M，256M，按照每页大小 4K 计算它们区域里包含的物理页个数分别为：2048, 16384, 65536。

|              | lowmem_reserve_ratio | 内存区域大小 | 物理内存页个数 |
| :----------: | :------------------: | :----------: | :------------: |
|   ZONE_DMA   |         256          |      8M      |      2048      |
| ZONE_NORMAL  |          32          |     64M      |     16384      |
| ZONE_HIGHMEM |          0           |     256M     |     65536      |

- ZONE_DMA 为防止被 ZONE_NORMAL 挤压侵占，而为自己预留的物理内存页为：`16384 / 256 = 64`。
- ZONE_DMA 为防止被 ZONE_HIGHMEM 挤压侵占而为自己预留的物理内存页为：`(65536 + 16384) / 256 = 320`。
- ZONE_NORMAL 为防止被 ZONE_HIGHMEM 挤压侵占而为自己预留的物理内存页为：`65536 / 32 = 2048`。

各个内存区域为防止被高位内存区域过度挤压占用，而为自己预留的内存大小，我们可以通过前边 `cat /proc/zoneinfo` 命令来查看，输出信息的 `protection：`则表示各个内存区域预留内存大小。

![image.png](https://cdn.xiaolincoding.com//mysql/other/941ba99527db4e0edd9afb0cde916f1f.png)

此外我们还可以通过 `sysctl`对内核参数 `lowmem_reserve_ratio` 进行动态调整，这样内核会根据新的 lowmem_reserve_ratio 动态重新计算各个内存区域的预留内存大小。

前面介绍的物理内存区域内被伙伴系统所管理的物理页数量 managed_pages 的计算方式就通过 present_pages 减去这些预留的物理内存页 reserved_pages 得到的。

> 调整内核参数的多种方法，我在[《从 Linux 内核角度探秘 JDK NIO 文件读写本质》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247486623&idx=1&sn=0cafed9e89b60d678d8c88dc7689abda&chksm=ce77cad8f90043ceaaca732aaaa7cb692c1d23eeb6c07de84f0ad690ab92d758945807239cee&token=1468822011&lang=zh_CN#rd)一文中的 "13.6 脏页回写参数的相关配置方式" 小节中已经详细介绍过了，感兴趣的同学可以在回看下。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_5-2-物理内存区域中的水位线)5.2 物理内存区域中的水位线

内存资源是系统中最宝贵的系统资源，是有限的。当内存资源紧张的时候，系统的应对方法无非就是三种：

1. 产生 OOM，内核直接将系统中占用大量内存的进程，将 OOM 优先级最高的进程干掉，释放出这个进程占用的内存供其他更需要的进程分配使用。
2. 内存回收，将不经常使用到的内存回收，腾挪出来的内存供更需要的进程分配使用。
3. 内存规整，将可迁移的物理页面进行迁移规整，消除内存碎片。从而获得更大的一片连续物理内存空间供进程分配。

我们都知道，内核将物理内存划分成一页一页的单位进行管理（每页 4K 大小）。内存回收的单位也是按页来的。在内核中，物理内存页有两种类型，针对这两种类型的物理内存页，内核会有不同的回收机制。

第一种就是文件页，所谓文件页就是其物理内存页中的数据来自于磁盘中的文件，当我们进行文件读取的时候，内核会根据局部性原理将读取的磁盘数据缓存在 page cache 中，page cache 里存放的就是文件页。当进程再次读取读文件页中的数据时，内核直接会从 page cache 中获取并拷贝给进程，省去了读取磁盘的开销。

对于文件页的回收通常会比较简单，因为文件页中的数据来自于磁盘，所以当回收文件页的时候直接回收就可以了，当进程再次读取文件页时，大不了再从磁盘中重新读取就是了。

但是当进程已经对文件页进行修改过但还没来得及同步回磁盘，此时文件页就是脏页，不能直接进行回收，需要先将脏页回写到磁盘中才能进行回收。

我们可以在进程中通过 fsync() 系统调用将指定文件的所有脏页同步回写到磁盘，同时内核也会根据一定的条件唤醒专门用于回写脏页的 pflush 内核线程。

> 关于文件页相关的详细内容，感兴趣的同学可以回看下我的这篇文章 [《从 Linux 内核角度探秘 JDK NIO 文件读写本质》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247486623&idx=1&sn=0cafed9e89b60d678d8c88dc7689abda&chksm=ce77cad8f90043ceaaca732aaaa7cb692c1d23eeb6c07de84f0ad690ab92d758945807239cee&token=1468822011&lang=zh_CN#rd)。

而另外一种物理页类型是匿名页，所谓匿名页就是它背后并没有一个磁盘中的文件作为数据来源，匿名页中的数据都是通过进程运行过程中产生的，比如我们应用程序中动态分配的堆内存。

当内存资源紧张需要对不经常使用的那些匿名页进行回收时，因为匿名页的背后没有一个磁盘中的文件做依托，所以匿名页不能像文件页那样直接回收，无论匿名页是不是脏页，都需要先将匿名页中的数据先保存在磁盘空间中，然后在对匿名页进行回收。

并把释放出来的这部分内存分配给更需要的进程使用，当进程再次访问这块内存时，在重新把之前匿名页中的数据从磁盘空间中读取到内存就可以了，而这块磁盘空间可以是单独的一片磁盘分区（Swap 分区）或者是一个特殊的文件（Swap 文件）。匿名页的回收机制就是我们经常看到的 Swap 机制。

所谓的页面换出就是在 Swap 机制下，当内存资源紧张时，内核就会把不经常使用的这些匿名页中的数据写入到 Swap 分区或者 Swap 文件中。从而释放这些数据所占用的内存空间。

所谓的页面换入就是当进程再次访问那些被换出的数据时，内核会重新将这些数据从 Swap 分区或者 Swap 文件中读取到内存中来。

综上所述，物理内存区域中的内存回收分为文件页回收（通过 pflush 内核线程）和匿名页回收（通过 kswapd 内核进程）。Swap 机制主要针对的是匿名页回收。

**那么当内存紧张的时候，内核到底是该回收文件页呢？还是该回收匿名页呢**？

事实上 Linux 提供了一个 swappiness 的内核选项，我们可以通过 `cat /proc/sys/vm/swappiness` 命令查看，swappiness 选项的取值范围为 0 到 100，默认为 60。

swappiness 用于表示 Swap 机制的积极程度，数值越大，Swap 的积极程度越高，内核越倾向于回收匿名页。数值越小，Swap 的积极程度越低。内核就越倾向于回收文件页。

> 注意： swappiness 只是表示 Swap 积极的程度，当内存非常紧张的时候，即使将 swappiness 设置为 0 ，也还是会发生 Swap 的。

那么到底什么时候内存才算是紧张的？紧张到什么程度才开始 Swap 呢？这一切都需要一个量化的标准，于是就有了本小节的主题 —— 物理内存区域中的水位线。

内核会为每个 NUMA 节点中的每个物理内存区域定制三条用于指示内存容量的水位线，分别是：WMARK_MIN（页最小阈值）， WMARK_LOW （页低阈值），WMARK_HIGH（页高阈值）。

![image.png](https://cdn.xiaolincoding.com//mysql/other/bba0c0b540cbfcbbb709077d2a22ee3d-20230310000006606.png)

这三条水位线定义在 `/include/linux/mmzone.h` 文件中：

```c
enum zone_watermarks {
	WMARK_MIN,
	WMARK_LOW,
	WMARK_HIGH,
	NR_WMARK
};

#define min_wmark_pages(z) (z->_watermark[WMARK_MIN] + z->watermark_boost)
#define low_wmark_pages(z) (z->_watermark[WMARK_LOW] + z->watermark_boost)
#define high_wmark_pages(z) (z->_watermark[WMARK_HIGH] + z->watermark_boost)
```

这三条水位线对应的 watermark 数值存储在每个物理内存区域 struct zone 结构中的 _watermark[NR_WMARK] 数组中。

```c
struct zone {
    // 物理内存区域中的水位线
    unsigned long _watermark[NR_WMARK];
    // 优化内存碎片对内存分配的影响，可以动态改变内存区域的基准水位线。
    unsigned long watermark_boost;

} ____cacheline_internodealigned_in_smp;
```

> 注意：下面提到的物理内存区域的剩余内存是需要刨去上小节介绍的 lowmem_reserve 预留内存大小。

![image.png](https://cdn.xiaolincoding.com//mysql/other/c0fccb4d8e1c8beb6887b521216c4b84-20230310000000767.png)

- 当该物理内存区域的剩余内存容量高于 _watermark[WMARK_HIGH] 时，说明此时该物理内存区域中的内存容量非常充足，内存分配完全没有压力。
- 当剩余内存容量在 _watermark[WMARK_LOW] 与_watermark[WMARK_HIGH] 之间时，说明此时内存有一定的消耗但是还可以接受，能够继续满足进程的内存分配需求。
- 当剩余内容容量在 _watermark[WMARK_MIN] 与 _watermark[WMARK_LOW] 之间时，说明此时内存容量已经有点危险了，内存分配面临一定的压力，但是还可以满足进程的内存分配要求，当给进程分配完内存之后，就会唤醒 kswapd 进程开始内存回收，直到剩余内存高于 _watermark[WMARK_HIGH] 为止。

> 在这种情况下，进程的内存分配会触发内存回收，但请求进程本身不会被阻塞，由内核的 kswapd 进程异步回收内存。

- 当剩余内容容量低于 _watermark[WMARK_MIN] 时，说明此时的内容容量已经非常危险了，如果进程在这时请求内存分配，内核就会进行**直接内存回收**，这时请求进程会同步阻塞等待，直到内存回收完毕。

> 位于 _watermark[WMARK_MIN] 以下的内存容量是预留给内核在紧急情况下使用的，这部分内存就是我们在 《5.1 物理内存区域中的预留内存》小节中介绍的预留内存 nr_reserved_highatomic。

我们可以通过 `cat /proc/zoneinfo` 命令来查看不同 NUMA 节点中不同内存区域中的水位线：

![image.png](https://cdn.xiaolincoding.com//mysql/other/e83cbf290eaf1e758ba97b8f119e5066-20230309235948784.png)

其中大部分字段的含义我已经在前面的章节中为大家介绍过了，下面我们只介绍和本小节内容相关的字段含义：

- free 就是该物理内存区域内剩余的内存页数，它的值和后面的 nr_free_pages 相同。
- min、low、high 就是上面提到的三条内存水位线：_watermark[WMARK_MIN]，_watermark[WMARK_LOW] ，_watermark[WMARK_HIGH]。
- nr_zone_active_anon 和 nr_zone_inactive_anon 分别是该内存区域内活跃和非活跃的匿名页数量。
- nr_zone_active_file 和 nr_zone_inactive_file 分别是该内存区域内活跃和非活跃的文件页数量。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_5-3-水位线的计算)5.3 水位线的计算

在上小节中我们介绍了内核通过对物理内存区域设置内存水位线来决定内存回收的时机，那么这三条内存水位线的值具体是多少，内核中是根据什么计算出来的呢？

事实上 WMARK_MIN，WMARK_LOW ，WMARK_HIGH 这三个水位线的数值是通过内核参数 `/proc/sys/vm/min_free_kbytes` 为基准分别计算出来的，用户也可以通过 `sysctl` 来动态设置这个内核参数。

> 内核参数 min_free_kbytes 的单位为 KB 。

![image.png](https://cdn.xiaolincoding.com//mysql/other/f6821ca876fd1c5f021952168a264f41.png)

通常情况下 WMARK_LOW 的值是 WMARK_MIN 的 1.25 倍，WMARK_HIGH 的值是 WMARK_LOW 的 1.5 倍。而 WMARK_MIN 的数值就是由这个内核参数 min_free_kbytes 来决定的。

下面我们就来看下内核中关于 min_free_kbytes 的计算方式：

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_5-4-min-free-kbytes-的计算逻辑)5.4 min_free_kbytes 的计算逻辑

> 以下计算逻辑是针对 64 位系统中内存区域水位线的计算，在 64 位系统中没有高端内存 ZONE_HIGHMEM 区域。

min_free_kbytes 的计算逻辑定义在内核文件 `/mm/page_alloc.c` 的 `init_per_zone_wmark_min` 方法中，用于计算最小水位线 WMARK_MIN 的数值也就是这里的 min_free_kbytes （单位为 KB）。 水位线的单位是物理内存页的数量。

```c
int __meminit init_per_zone_wmark_min(void)
{
  // 低位内存区域（除高端内存之外）的总和
	unsigned long lowmem_kbytes;
  // 待计算的 min_free_kbytes
	int new_min_free_kbytes;

  // 将低位内存区域内存容量总的页数转换为 KB
	lowmem_kbytes = nr_free_buffer_pages() * (PAGE_SIZE >> 10);
  // min_free_kbytes 计算逻辑：对 lowmem_kbytes * 16 进行开平方
	new_min_free_kbytes = int_sqrt(lowmem_kbytes * 16);
  // min_free_kbytes 的范围为 128 到 65536 KB 之间
	if (new_min_free_kbytes > user_min_free_kbytes) {
		min_free_kbytes = new_min_free_kbytes;
		if (min_free_kbytes < 128)
			min_free_kbytes = 128;
		if (min_free_kbytes > 65536)
			min_free_kbytes = 65536;
	} else {
		pr_warn("min_free_kbytes is not updated to %d because user defined value %d is preferred\n",
				new_min_free_kbytes, user_min_free_kbytes);
	}
  // 计算内存区域内的三条水位线
	setup_per_zone_wmarks();
  // 计算内存区域的预留内存大小，防止被高位内存区域过度挤压占用
	setup_per_zone_lowmem_reserve();
        .............省略................
	return 0;
}
core_initcall(init_per_zone_wmark_min)
```

首先我们需要先计算出当前 NUMA 节点中所有低位内存区域（除高端内存之外）中内存总容量之和。也即是说 lowmem_kbytes 的值为： ZONE_DMA 区域中 managed_pages + ZONE_DMA32 区域中 managed_pages + ZONE_NORMAL 区域中 managed_pages 。

lowmem_kbytes 的计算逻辑在 `nr_free_zone_pages` 方法中：

```c
/**
 * nr_free_zone_pages - count number of pages beyond high watermark
 * @offset: The zone index of the highest zone
 *
 * nr_free_zone_pages() counts the number of counts pages which are beyond the
 * high watermark within all zones at or below a given zone index.  For each
 * zone, the number of pages is calculated as:
 *     managed_pages - high_pages
 */
static unsigned long nr_free_zone_pages(int offset)
{
	struct zoneref *z;
	struct zone *zone;

	unsigned long sum = 0;
    // 获取当前 NUMA 节点中的所有物理内存区域 zone
	struct zonelist *zonelist = node_zonelist(numa_node_id(), GFP_KERNEL);
    // 计算所有物理内存区域内 managed_pages - high_pages 的总和
	for_each_zone_zonelist(zone, z, zonelist, offset) {
		unsigned long size = zone->managed_pages;
		unsigned long high = high_wmark_pages(zone);
		if (size > high)
			sum += size - high;
	}
    // lowmem_kbytes 的值
	return sum;
}
```

nr_free_zone_pages 方法上面的注释大家可能看的有点蒙，这里需要为大家解释一下，nr_free_zone_pages 方法的计算逻辑本意是给定一个 zone index （方法参数 offset），计算范围为：这个给定 zone 下面的所有低位内存区域。

nr_free_zone_pages 方法会计算这些低位内存区域内在 high watermark 水位线之上的内存容量（ managed_pages - high_pages ）之和。作为该方法的返回值。

但此时我们正准备计算这些水位线，水位线还没有值，所以此时这个方法的语义就是计算低位内存区域内被伙伴系统所管理的内存容量（ managed_pages ）之和。也就是我们想要的 lowmem_kbytes。

接下来在 init_per_zone_wmark_min 方法中会对 lowmem_kbytes * 16 进行开平方得到 new_min_free_kbytes。

![image.png](https://cdn.xiaolincoding.com//mysql/other/7f2ffff8410d8da7a838ca117b457a11.png)

如果计算出的 new_min_free_kbytes 大于用户设置的内核参数值 `/proc/sys/vm/min_free_kbytes` ，那么最终 min_free_kbytes 就是 new_min_free_kbytes。如果小于用户设定的值，那么就采用用户指定的 min_free_kbytes 。

> min_free_kbytes 的取值范围限定在 128 到 65536 KB 之间。

随后内核会根据这个 min_free_kbytes 在 setup_per_zone_wmarks() 方法中计算出该物理内存区域的三条水位线。

最后在 setup_per_zone_lowmem_reserve() 方法中计算内存区域的预留内存大小，防止被高位内存区域过度挤压占用。该方法的逻辑就是我们在《5.1 物理内存区域中的预留内存》小节中提到的内容。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_5-5-setup-per-zone-wmarks-计算水位线)5.5 setup_per_zone_wmarks 计算水位线

> 这里我们依然不会考虑高端内存区域 ZONE_HIGHMEM。

物理内存区域内的三条水位线：WMARK_MIN，WMARK_LOW，WMARK_HIGH 的最终计算逻辑是在 `__setup_per_zone_wmarks` 方法中完成的：

```c
static void __setup_per_zone_wmarks(void)
{
  // 将 min_free_kbytes 转换为页
	unsigned long pages_min = min_free_kbytes >> (PAGE_SHIFT - 10);
  // 所有低位内存区域 managed_pages 之和
	unsigned long lowmem_pages = 0;
	struct zone *zone;
	unsigned long flags;

	/* Calculate total number of !ZONE_HIGHMEM pages */
	for_each_zone(zone) {
		if (!is_highmem(zone))
			lowmem_pages += zone->managed_pages;
	}

  // 循环计算各个内存区域中的水位线
	for_each_zone(zone) {
		u64 tmp;
		tmp = (u64)pages_min * zone->managed_pages;
  // 计算 WMARK_MIN 水位线的核心方法
		do_div(tmp, lowmem_pages);
		if (is_highmem(zone)) {
            ...........省略高端内存区域............
		} else {
    // WMARK_MIN水位线
			zone->watermark[WMARK_MIN] = tmp;
		}
  // 这里可暂时忽略
		tmp = max_t(u64, tmp >> 2,
			    mult_frac(zone->managed_pages,
				      watermark_scale_factor, 10000));

		zone->watermark[WMARK_LOW]  = min_wmark_pages(zone) + tmp;
		zone->watermark[WMARK_HIGH] = min_wmark_pages(zone) + tmp * 2;
	}
}
```

在 for_each_zone 循环内依次遍历 NUMA 节点中的所有内存区域 zone，计算每个内存区域 zone 里的内存水位线。其中计算 WMARK_MIN 水位线的核心逻辑封装在 do_div 方法中，在 do_div 方法中会先计算每个 zone 内存容量之间的比例，然后根据这个比例去从 min_free_kbytes 中划分出对应 zone 的 WMARK_MIN 水位线来。

比如：当前 NUMA 节点中有两个 zone ：ZONE_DMA 和 ZONE_NORMAL，内存容量大小分别是：100 M 和 800 M。那么 ZONE_DMA 与 ZONE_NORMAL 之间的比例就是 1 ：8。

根据这个比例，ZONE_DMA 区域里的 WMARK_MIN 水位线就是：min_free_kbytes * `1 / 8` 。ZONE_NORMAL 区域里的 WMARK_MIN 水位线就是：min_free_kbytes * `7 / 8`。

计算出了 WMARK_MIN 的值，那么接下来 WMARK_LOW， WMARK_HIGH 的值也就好办了，它们都是基于 WMARK_MIN 计算出来的。

WMARK_LOW 的值是 WMARK_MIN 的 1.25 倍，WMARK_HIGH 的值是 WMARK_LOW 的 1.5 倍。

此外，大家可能对下面这段代码比较有疑问?

```c
      /*
         * Set the kswapd watermarks distance according to the
         * scale factor in proportion to available memory, but
         * ensure a minimum size on small systems.
         */
        tmp = max_t(u64, tmp >> 2,
                mult_frac(zone->managed_pages,
                      watermark_scale_factor, 10000));
```

这段代码主要是通过内核参数 watermark_scale_factor 来调节水位线：WMARK_MIN，WMARK_LOW，WMARK_HIGH 之间的间距，那么为什么要调整水位线之间的间距大小呢？

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_5-6-watermark-scale-factor-调整水位线的间距)5.6 watermark_scale_factor 调整水位线的间距

![image.png](https://cdn.xiaolincoding.com//mysql/other/c0fccb4d8e1c8beb6887b521216c4b84.png)

为了避免内核的直接内存回收 direct reclaim 阻塞进程影响系统的性能，所以我们需要尽量保持内存区域中的剩余内存容量尽量在 WMARK_MIN 水位线之上，但是有一些极端情况，比如突然遇到网络流量增大，需要短时间内申请大量的内存来存放网络请求数据，此时 kswapd 回收内存的速度可能赶不上内存分配的速度，从而造成直接内存回收 direct reclaim，影响系统性能。

在内存分配过程中，剩余内存容量处于 WMARK_MIN 与 WMARK_LOW 水位线之间会唤醒 kswapd 进程来回收内存，直到内存容量恢复到 WMARK_HIGH 水位线之上。

剩余内存容量低于 WMARK_MIN 水位线时就会触发直接内存回收 direct reclaim。

而剩余内存容量高于 WMARK_LOW 水位线又不会唤醒 kswapd 进程，因此 kswapd 进程活动的关键范围在 WMARK_MIN 与 WMARK_LOW 之间，而为了应对这种突发的网络流量暴增，我们需要保证 kswapd 进程活动的范围大一些，这样内核就能够时刻进行内存回收使得剩余内存容量较长时间的保持在 WMARK_HIGH 水位线之上。

这样一来就要求 WMARK_MIN 与 WMARK_LOW 水位线之间的间距不能太小，因为 WMARK_LOW 水位线之上就不会唤醒 kswapd 进程了。

因此内核引入了 `/proc/sys/vm/watermark_scale_factor` 参数来调节水位线之间的间距。该内核参数默认值为 10，最大值为 3000。

![image.png](https://cdn.xiaolincoding.com//mysql/other/ff7c3eeed13024ea96dfe92cb6ac1db6.png)

那么如何使用 watermark_scale_factor 参数调整水位线之间的间距呢？

水位线间距计算公式：(watermark_scale_factor / 10000) * managed_pages 。

```c
        zone->watermark[WMARK_MIN] = tmp;
        // 水位线间距的计算逻辑
        tmp = max_t(u64, tmp >> 2,
                mult_frac(zone->managed_pages,
                      watermark_scale_factor, 10000));

        zone->watermark[WMARK_LOW]  = min_wmark_pages(zone) + tmp;
        zone->watermark[WMARK_HIGH] = min_wmark_pages(zone) + tmp * 2;
```

在内核中水位线间距计算逻辑是：(WMARK_MIN / 4) 与 (zone_managed_pages * watermark_scale_factor / 10000) 之间较大的那个值。

用户可以通过 `sysctl` 来动态调整 watermark_scale_factor 参数，内核会动态重新计算水位线之间的间距，使得 WMARK_MIN 与 WMARK_LOW 之间留有足够的缓冲余地，使得 kswapd 能够有时间回收足够的内存，**从而解决直接内存回收导致的性能抖动问题**。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_5-7-物理内存区域中的冷热页)5.7 物理内存区域中的冷热页

之前我在[《一文聊透对象在JVM中的内存布局，以及内存对齐和压缩指针的原理及应用》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247484304&idx=1&sn=54bf0d07e69c5621c145afaece8f50d6&chksm=ce77c5d7f9004cc1249a03dfd0fb12b7d75171f1b87acea1fa44bbb11ca374b6f42a66fa274d#rd)一文中为大家介绍 CPU 的高速缓存时曾提到过，根据摩尔定律：芯片中的晶体管数量每隔 18 个月就会翻一番。导致 CPU 的性能和处理速度变得越来越快，而提升 CPU 的运行速度比提升内存的运行速度要容易和便宜的多，所以就导致了 CPU 与内存之间的速度差距越来越大。

CPU 与 内存之间的速度差异到底有多大呢？ 我们知道寄存器是离 CPU 最近的，CPU 在访问寄存器的时候速度近乎于 0 个时钟周期，访问速度最快，基本没有时延。而访问内存则需要 50 - 200 个时钟周期。

所以为了弥补 CPU 与内存之间巨大的速度差异，提高CPU的处理效率和吞吐，于是我们引入了 L1 , L2 , L3 高速缓存集成到 CPU 中。CPU 访问高速缓存仅需要用到 1 - 30 个时钟周期，CPU 中的高速缓存是对内存热点数据的一个缓存。

![CPU缓存结构.png](https://cdn.xiaolincoding.com//mysql/other/ad4588e557446544b62d43452b94f74d.png)

CPU 访问高速缓存的速度比访问内存的速度快大约10倍，引入高速缓存的目的在于消除CPU与内存之间的速度差距，CPU 用高速缓存来用来存放内存中的热点数据。

另外我们根据程序的时间局部性原理可以知道，内存的数据一旦被访问，那么它很有可能在短期内被再次访问，如果我们把经常访问的物理内存页缓存在 CPU 的高速缓存中，那么当进程再次访问的时候就会直接命中 CPU 的高速缓存，避免了进一步对内存的访问，极大提升了应用程序的性能。

> 程序局部性原理表现为：时间局部性和空间局部性。时间局部性是指如果程序中的某条指令一旦执行，则不久之后该指令可能再次被执行；如果某块数据被访问，则不久之后该数据可能再次被访问。空间局部性是指一旦程序访问了某个存储单元，则不久之后，其附近的存储单元也将被访问。

本文我们的主题是 Linux 物理内存的管理，那么在 NUMA 内存架构下，这些 NUMA 节点中的物理内存区域 zone 管理的这些物理内存页，哪些是在 CPU 的高速缓存中？哪些又不在 CPU 的高速缓存中呢？内核如何来管理这些加载进 CPU 高速缓存中的物理内存页呢？

![image.png](https://cdn.xiaolincoding.com//mysql/other/d69d03bbccd05ad1eef0ac25f6e92ac5.png)

> 本小节标题中所谓的热页就是已经加载进 CPU 高速缓存中的物理内存页，所谓的冷页就是还未加载进 CPU 高速缓存中的物理内存页，冷页是热页的后备选项。

我先以内核版本 2.6.25 之前的冷热页相关的管理逻辑为大家讲解，因为这个版本的逻辑比较直观，大家更容易理解。在这个基础之上，我会在介绍内核 5.0 版本对于冷热页管理的逻辑，差别不是很大。

```c
struct zone {
    struct per_cpu_pageset	pageset[NR_CPUS];
}
```

在 2.6.25 版本之前的内核源码中，物理内存区域 struct zone 包含了一个 struct per_cpu_pageset 类型的数组 pageset。其中内核关于冷热页的管理全部封装在 struct per_cpu_pageset 结构中。

因为每个 CPU 都有自己独立的高速缓存，所以每个 CPU 对应一个 per_cpu_pageset 结构，pageset 数组容量 NR_CPUS 是一个可以在编译期间配置的宏常数，表示内核可以支持的最大 CPU个数，注意该值并不是系统实际存在的 CPU 数量。

在 NUMA 内存架构下，每个物理内存区域都是属于一个特定的 NUMA 节点，NUMA 节点中包含了一个或者多个 CPU，NUMA 节点中的每个内存区域会关联到一个特定的 CPU 上，但 struct zone 结构中的 pageset 数组包含的是系统中所有 CPU 的高速缓存页。

因为虽然一个内存区域关联到了 NUMA 节点中的一个特定 CPU 上，但是其他CPU 依然可以访问该内存区域中的物理内存页，因此其他 CPU 上的高速缓存仍然可以包含该内存区域中的物理内存页。

每个 CPU 都可以访问系统中的所有物理内存页，尽管访问速度不同（这在前边我们介绍 NUMA 架构的时候已经介绍过），因此特定的物理内存区域 struct zone 不仅要考虑到所属 NUMA 节点中相关的 CPU，还需要照顾到系统中的其他 CPU。

在表示每个 CPU 高速缓存结构 struct per_cpu_pageset 中有一个 struct per_cpu_pages 类型的数组 pcp，容量为 2。 数组 pcp 索引 0 表示该内存区域加载进 CPU 高速缓存的热页集合，索引 1 表示该内存区域中还未加载进 CPU 高速缓存的冷页集合。

```c
struct per_cpu_pageset {
	struct per_cpu_pages pcp[2];	/* 0: hot.  1: cold */
}
```

struct per_cpu_pages 结构则是最终用于管理 CPU 高速缓存中的热页，冷页集合的数据结构：

```c
struct per_cpu_pages {
	int count;		/* number of pages in the list */
	int high;		/* high watermark, emptying needed */
	int batch;		/* chunk size for buddy add/remove */
	struct list_head list;	/* the list of pages */
};
```

- int count ：表示集合中包含的物理页数量，如果该结构是热页集合，则表示加载进 CPU 高速缓存中的物理页面个数。
- struct list_head list ：该 list 是一个双向链表，保存了当前 CPU 的热页或者冷页。
- int batch：每次批量向 CPU 高速缓存填充或者释放的物理页面个数。
- int high：如果集合中页面的数量 count 值超过了 high 的值，那么表示 list 中的页面太多了，内核会从高速缓存中释放 batch 个页面到物理内存区域中的伙伴系统中。
- int low : 在之前更老的版本中，per_cpu_pages 结构还定义了一个 low 下限值，如果 count 低于 low 的值，那么内核会从伙伴系统中申请 batch 个页面填充至当前 CPU 的高速缓存中。之后的版本中取消了 low ，内核对容量过低的页面集合并没有显示的使用水位值 low，当列表中没有其他成员时，内核会重新填充高速缓存。

以上则是内核版本 2.6.25 之前管理 CPU 高速缓存冷热页的相关数据结构，我们看到在 2.6.25 之前，内核是使用两个 per_cpu_pages 结构来分别管理冷页和热页集合的

后来内核开发人员通过测试发现，用两个列表来管理冷热页，并不会比用一个列表集中管理冷热页带来任何的实质性好处，因此在内核版本 2.6.25 之后，将冷页和热页的管理合并在了一个列表中，热页放在列表的头部，冷页放在列表的尾部。

在内核 5.0 的版本中， struct zone 结构中去掉了原来使用 struct per_cpu_pageset 数，因为 struct per_cpu_pageset 结构中分别管理了冷页和热页。

```c
struct zone {
	struct per_cpu_pages	__percpu *per_cpu_pageset;

	int pageset_high;
	int pageset_batch;

} ____cacheline_internodealigned_in_smp;
```

直接使用 struct per_cpu_pages 结构的链表来集中管理系统中所有 CPU 高速缓存冷热页。

```c
struct per_cpu_pages {
	int count;		/* number of pages in the list */
	int high;		/* high watermark, emptying needed */
	int batch;		/* chunk size for buddy add/remove */
        
        .............省略............

	/* Lists of pages, one per migrate type stored on the pcp-lists */
	struct list_head lists[NR_PCP_LISTS];
};
```

前面我们提到，内核为了最大程度的防止内存碎片，将物理内存页面按照是否可迁移的特性分为了多种迁移类型：可迁移，可回收，不可迁移。在 struct per_cpu_pages 结构中，每一种迁移类型都会对应一个冷热页链表。

## [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_6-内核如何描述物理内存页)6. 内核如何描述物理内存页

![image.png](https://cdn.xiaolincoding.com//mysql/other/bba0c0b540cbfcbbb709077d2a22ee3d-20230309235827777.png)

经过前边几个小节的介绍，我想大家现在应该对 Linux 内核整个内存管理框架有了一个总体上的认识。

如上图所示，在 NUMA 架构下内存被划分成了一个一个的内存节点（NUMA Node），在每个 NUMA 节点中，内核又根据节点内物理内存的功能用途不同，将 NUMA 节点内的物理内存划分为四个物理内存区域分别是：ZONE_DMA，ZONE_DMA32，ZONE_NORMAL，ZONE_HIGHMEM。其中 ZONE_MOVABLE 区域是逻辑上的划分，主要是为了防止内存碎片和支持内存的热插拔。

物理内存区域中管理的就是物理内存页（ Linux 内存管理的最小单位），前面我们介绍的内核对物理内存的换入，换出，回收，内存映射等操作的单位就是页。内核为每一个物理内存区域分配了一个伙伴系统，用于管理该物理内存区域下所有物理内存页面的分配和释放。

Linux 默认支持的物理内存页大小为 4KB，在 64 位体系结构中还可以支持 8KB，有的处理器还可以支持 4MB，支持物理地址扩展 PAE 机制的处理器上还可以支持 2MB。

**那么 Linux 为什么会默认采用 4KB 作为标准物理内存页的大小呢** ？

首先关于物理页面的大小，Linux 规定必须是 2 的整数次幂，因为 2 的整数次幂可以将一些数学运算转换为移位操作，比如乘除运算可以通过移位操作来实现，这样效率更高。

那么系统支持 4KB，8KB，2MB，4MB 等大小的物理页面，它们都是 2 的整数次幂，为啥偏偏要选 4KB 呢？

因为前面提到，在内存紧张的时候，内核会将不经常使用到的物理页面进行换入换出等操作，还有在内存与文件映射的场景下，都会涉及到与磁盘的交互，数据在磁盘中组织形式也是根据一个磁盘块一个磁盘块来管理的，4kB 和 4MB 都是磁盘块大小的整数倍，但在大多数情况下，内存与磁盘之间传输小块数据时会更加的高效，所以综上所述内核会采用 4KB 作为默认物理内存页大小。

------

假设我们有 4G 大小的物理内存，每个物理内存页大小为 4K，那么这 4G 的物理内存会被内核划分为 1M 个物理内存页，内核使用一个 struct page 的结构体来描述物理内存页，而每个 struct page 结构体占用内存大小为 40 字节，那么内核就需要用额外的 40 * 1M = 40M 的内存大小来描述物理内存页。

对于 4G 物理内存而言，这额外的 40M 内存占比相对较小，这个代价勉强可以接受，但是对内存锱铢必较的内核来说，还是会尽最大努力想尽一切办法来控制 struct page 结构体的大小。

因为对于 4G 的物理内存来说，内核就需要使用 1M 个物理页面来管理，1M 个物理页的数量已经是非常庞大的了，因此在后续的内核迭代中，对于 struct page 结构的任何微小改动，都可能导致用于管理物理内存页的 struct page 实例所需要的内存暴涨。

回想一下我们经历过的很多复杂业务系统，由于业务逻辑已经非常复杂，在加上业务版本日积月累的迭代，整个业务系统已经变得异常复杂，在这种类型的业务系统中，我们经常会使用一个非常庞大的类来包装全量的业务响应信息用以应对各种复杂的场景，但是这个类已经包含了太多太多的业务字段了，而且这些业务字段在有的场景中会用到，在有的场景中又不会用到，后面还可能继续临时增加很多字段。系统的维护就这样变得越来越困难。

相比上面业务系统开发中随意地增加改动类中的字段，在内核中肯定是不会允许这样的行为发生的。struct page 结构是内核中访问最为频繁的一个结构体，就好比是 Linux 世界里最繁华的地段，在这个最繁华的地段租间房子，那租金可谓是相当的高，同样的道理，内核在 struct page 结构体中增加一个字段的代价也是非常之大，该结构体中每个字段中的每个比特，内核用的都是淋漓尽致。

但是 struct page 结构同样会面临很多复杂的场景，结构体中的某些字段在某些场景下有用，而在另外的场景下却没有用，而内核又不可能像业务系统开发那样随意地为 struct page 结构增加字段，那么内核该如何应对这种情况呢？

下面我们即将会看到 struct page 结构体里包含了大量的 union 结构，而 union 结构在 C 语言中被用于同一块内存根据不同场景保存不同类型数据的一种方式。内核之所以在 struct page 结构中使用 union，是因为一个物理内存页面在内核中的使用场景和使用方式是多种多样的。在这多种场景下，利用 union 尽最大可能使 struct page 的内存占用保持在一个较低的水平。

struct page 结构可谓是内核中最为繁杂的一个结构体，应用在内核中的各种功能场景下，在本小节中一一解释清楚各个字段的含义是不现实的，下面我只会列举 struct page 中最为常用的几个字段，剩下的字段我会在后续相关文章中专门介绍。

```c
struct page {
    // 存储 page 的定位信息以及相关标志位
    unsigned long flags;        

    union {
        struct {    /* Page cache and anonymous pages */
            // 用来指向物理页 page 被放置在了哪个 lru 链表上
            struct list_head lru;
            // 如果 page 为文件页的话，低位为0，指向 page 所在的 page cache
            // 如果 page 为匿名页的话，低位为1，指向其对应虚拟地址空间的匿名映射区 anon_vma
            struct address_space *mapping;
            // 如果 page 为文件页的话，index 为 page 在 page cache 中的索引
            // 如果 page 为匿名页的话，表示匿名页在对应进程虚拟内存区域 VMA 中的偏移
            pgoff_t index;
            // 在不同场景下，private 指向的场景信息不同
            unsigned long private;
        };
        
        struct {    /* slab, slob and slub */
            union {
                // 用于指定当前 page 位于 slab 中的哪个具体管理链表上。
                struct list_head slab_list;
                struct {
                    // 当 page 位于 slab 结构中的某个管理链表上时，next 指针用于指向链表中的下一个 page
                    struct page *next;
#ifdef CONFIG_64BIT
                    // 表示 slab 中总共拥有的 page 个数
                    int pages;  
                    // 表示 slab 中拥有的特定类型的对象个数
                    int pobjects;   
#else
                    short int pages;
                    short int pobjects;
#endif
                };
            };
            // 用于指向当前 page 所属的 slab 管理结构
            struct kmem_cache *slab_cache; 
        
            // 指向 page 中的第一个未分配出去的空闲对象
            void *freelist;     
            union {
                // 指向 page 中的第一个对象
                void *s_mem;    
                struct {            /* SLUB */
                    // 表示 slab 中已经被分配出去的对象个数
                    unsigned inuse:16;
                    // slab 中所有的对象个数
                    unsigned objects:15;
                    // 当前内存页 page 被 slab 放置在 CPU 本地缓存列表中，frozen = 1，否则 frozen = 0
                    unsigned frozen:1;
                };
            };
        };
        struct {    /* 复合页 compound page 相关*/
            // 复合页的尾页指向首页
            unsigned long compound_head;    
            // 用于释放复合页的析构函数，保存在首页中
            unsigned char compound_dtor;
            // 该复合页有多少个 page 组成
            unsigned char compound_order;
            // 该复合页被多少个进程使用，内存页反向映射的概念，首页中保存
            atomic_t compound_mapcount;
        };

        // 表示 slab 中需要释放回收的对象链表
        struct rcu_head rcu_head;
    };

    union {     /* This union is 4 bytes in size. */
        // 表示该 page 映射了多少个进程的虚拟内存空间，一个 page 可以被多个进程映射
        atomic_t _mapcount;

    };

    // 内核中引用该物理页的次数，表示该物理页的活跃程度。
    atomic_t _refcount;

#if defined(WANT_PAGE_VIRTUAL)
    void *virtual;  // 内存页对应的虚拟内存地址
#endif /* WANT_PAGE_VIRTUAL */

} _struct_page_alignment;
```

下面我就来为大家介绍下 struct page 结构在不同场景下的使用方式：

第一种使用方式是内核直接分配使用一整页的物理内存，在《5.2 物理内存区域中的水位线》小节中我们提到，内核中的物理内存页有两种类型，分别用于不同的场景：

1. 一种是匿名页，匿名页背后并没有一个磁盘中的文件作为数据来源，匿名页中的数据都是通过进程运行过程中产生的，匿名页直接和进程虚拟地址空间建立映射供进程使用。
2. 另外一种是文件页，文件页中的数据来自于磁盘中的文件，文件页需要先关联一个磁盘中的文件，然后再和进程虚拟地址空间建立映射供进程使用，使得进程可以通过操作虚拟内存实现对文件的操作，这就是我们常说的内存文件映射。

```c
struct page {
    // 如果 page 为文件页的话，低位为0，指向 page 所在的 page cache
    // 如果 page 为匿名页的话，低位为1，指向其对应虚拟地址空间的匿名映射区 anon_vma
    struct address_space *mapping;
    // 如果 page 为文件页的话，index 为 page 在 page cache 中的索引
    // 如果 page 为匿名页的话，表示匿名页在对应进程虚拟内存区域 VMA 中的偏移
    pgoff_t index; 
}
```

我们首先来介绍下 struct page 结构中的 struct address_space *mapping 字段。提到 struct address_space 结构，如果大家之前看过我 [《从 Linux 内核角度探秘 JDK NIO 文件读写本质》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247486623&idx=1&sn=0cafed9e89b60d678d8c88dc7689abda&chksm=ce77cad8f90043ceaaca732aaaa7cb692c1d23eeb6c07de84f0ad690ab92d758945807239cee&token=1468822011&lang=zh_CN#rd)这篇文章的话，一定不会对 struct address_space 感到陌生。

![image.png](https://cdn.xiaolincoding.com//mysql/other/b3cdb64391f7ae4bbb33dec71a777ad1.png)

在内核中每个文件都会有一个属于自己的 page cache（页高速缓存），页高速缓存在内核中的结构体就是这个 struct address_space。它被文件的 inode 所持有。

如果当前物理内存页 struct page 是一个文件页的话，那么 mapping 指针的最低位会被设置为 0 ，指向该内存页关联文件的 struct address_space（页高速缓存），pgoff_t index 字段表示该内存页 page 在页高速缓存 page cache 中的 index 索引。内核会利用这个 index 字段从 page cache 中查找该物理内存页，

![image.png](https://cdn.xiaolincoding.com//mysql/other/4ec67b6f29e6d817e3273c26f8a10c59.png)

同时该 pgoff_t index 字段也表示该内存页中的文件数据在文件内部的偏移 offset。偏移单位为 page size。

> 对相关查找细节感兴趣的同学可以在回看下我 [《从 Linux 内核角度探秘 JDK NIO 文件读写本质》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247486623&idx=1&sn=0cafed9e89b60d678d8c88dc7689abda&chksm=ce77cad8f90043ceaaca732aaaa7cb692c1d23eeb6c07de84f0ad690ab92d758945807239cee&token=1468822011&lang=zh_CN#rd)文章中的《8. page cache 中查找缓存页》小节。

如果当前物理内存页 struct page 是一个匿名页的话，那么 mapping 指针的最低位会被设置为 1 ， 指向该匿名页在进程虚拟内存空间中的匿名映射区域 struct anon_vma 结构（每个匿名页对应唯一的 anon_vma 结构），用于物理内存到虚拟内存的反向映射。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_6-1-匿名页的反向映射)6.1 匿名页的反向映射

我们通常所说的内存映射是正向映射，即从虚拟内存到物理内存的映射。而反向映射则是从物理内存到虚拟内存的映射，用于当某个物理内存页需要进行回收或迁移时，此时需要去找到这个物理页被映射到了哪些进程的虚拟地址空间中，并断开它们之间的映射。

在没有反向映射的机制前，需要去遍历所有进程的虚拟地址空间中的映射页表，这个效率显然是很低下的。有了反向映射机制之后内核就可以直接找到该物理内存页到所有进程映射的虚拟地址空间 VMA ，并从 VMA 使用的进程页表中取消映射，

谈到 VMA 大家一定不会感到陌生，VMA 相关的内容我在 [《深入理解 Linux 虚拟内存管理》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247486732&idx=1&sn=435d5e834e9751036c96384f6965b328&chksm=ce77cb4bf900425d33d2adfa632a4684cf7a63beece166c1ffedc4fdacb807c9413e8c73f298&token=1468822011&lang=zh_CN#rd)这篇文章中详细的介绍过。

如下图所示，进程的虚拟内存空间在内核中使用 struct mm_struct 结构表示，进程的虚拟内存空间包含了一段一段的虚拟内存区域 VMA，比如我们经常接触到的堆，栈。内核中使用 struct vm_area_struct 结构来描述这些虚拟内存区域。

![image.png](https://cdn.xiaolincoding.com//mysql/other/349d2cf08f9d8f994fa3b64b62391c6d.png)

这里我只列举出 struct vm_area_struct 结构中与匿名页反向映射相关的字段属性：

```c
struct vm_area_struct {  

    struct list_head anon_vma_chain;
    struct anon_vma *anon_vma;   
}
```

这里大家可能会感到好奇，既然内核中有了 struct vm_area_struct 结构来描述虚拟内存区域，那不管是文件页也好，还是匿名页也好，都可以使用 struct vm_area_struct 结构体来进行描述，这里为什么有会出现 struct anon_vma 结构和 struct anon_vma_chain 结构？这两个结构到底是干嘛的？如何利用它俩来完成匿名内存页的反向映射呢？

根据前几篇文章的内容我们知道，进程利用 fork 系统调用创建子进程的时候，内核会将父进程的虚拟内存空间相关的内容拷贝到子进程的虚拟内存空间中，此时子进程的虚拟内存空间和父进程的虚拟内存空间是一模一样的，其中虚拟内存空间中映射的物理内存页也是一样的，在内核中都是同一份，在父进程和子进程之间共享（包括 anon_vma 和 anon_vma_chain）。

当进程在向内核申请内存的时候，内核首先会为进程申请的这块内存创建初始化一段虚拟内存区域 struct vm_area_struct 结构，但是并不会为其分配真正的物理内存。

当进程开始访问这段虚拟内存时，内核会产生缺页中断，在缺页中断处理函数中才会去真正的分配物理内存（这时才会为子进程创建自己的 anon_vma 和 anon_vma_chain），并建立虚拟内存与物理内存之间的映射关系（正向映射）。

```c
static vm_fault_t handle_pte_fault(struct vm_fault *vmf)
{
        .............

	if (!vmf->pte) {
		if (vma_is_anonymous(vmf->vma))
            // 处理匿名页缺页
			return do_anonymous_page(vmf);
		else
            // 处理文件页缺页
			return do_fault(vmf);
	}

        .............

	if (vmf->flags & (FAULT_FLAG_WRITE|FAULT_FLAG_UNSHARE)) {
		if (!pte_write(entry))
            // 子进程缺页处理
			return do_wp_page(vmf);
	}
```

这里我们主要关注 do_anonymous_page 函数，正是在这里内核完成了 struct anon_vma 结构和 struct anon_vma_chain 结构的创建以及相关匿名页反向映射数据结构的相互关联。

```c
static vm_fault_t do_anonymous_page(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	struct page *page;	

        ........省略虚拟内存到物理内存正向映射相关逻辑.........

	if (unlikely(anon_vma_prepare(vma)))
		goto oom;

	page = alloc_zeroed_user_highpage_movable(vma, vmf->address);

	if (!page)
		goto oom;
  // 建立反向映射关系
	page_add_new_anon_rmap(page, vma, vmf->address);

        ........省略虚拟内存到物理内存正向映射相关逻辑.........
}
```

在 do_anonymous_page 匿名页缺页处理函数中会为 struct vm_area_struct 结构创建匿名页相关的 struct anon_vma 结构和 struct anon_vma_chain 结构。

并在 anon_vma_prepare 函数中实现 anon_vma 和 anon_vma_chain 之间的关联 ，随后调用 alloc_zeroed_user_highpage_movable 从伙伴系统中获取物理内存页 struct page，并在 page_add_new_anon_rmap 函数中完成 struct page 与 anon_vma 的关联（这里正是反向映射关系建立的关键）

在介绍匿名页反向映射源码实现之前，我先来为大家介绍一下相关的两个重要数据结构 struct anon_vma 和 struct anon_vma_chain，方便大家理解为何 struct page 与 anon_vma 关联起来就能实现反向映射？

前面我们提到，匿名页的反向映射关键就是建立物理内存页 struct page 与进程虚拟内存空间 VMA 之间的映射关系。

匿名页的 struct page 中的 mapping 指针指向的是 struct anon_vma 结构。

```c
struct page {
    struct address_space *mapping; 
    pgoff_t index;  
}
```

只要我们实现了 anon_vma 与 vm_area_struct 之间的关联，那么 page 到 vm_area_struct 之间的映射就建立起来了，struct anon_vma_chain 结构做的事情就是建立 anon_vma 与 vm_area_struct 之间的关联关系。

```c
struct anon_vma_chain {
    // 匿名页关联的进程虚拟内存空间（vma属于一个特定的进程，多个进程多个vma）
    struct vm_area_struct *vma;
    // 匿名页 page 指向的 anon_vma
    struct anon_vma *anon_vma;
    struct list_head same_vma;   
    struct rb_node rb;         
    unsigned long rb_subtree_last;
#ifdef CONFIG_DEBUG_VM_RB
    unsigned long cached_vma_start, cached_vma_last;
#endif
};
```

struct anon_vma_chain 结构通过其中的 vma 指针和 anon_vma 指针将相关的匿名页与其映射的进程虚拟内存空间关联了起来。

![image.png](https://cdn.xiaolincoding.com//mysql/other/e352341f7a7950372b2dc4f272edf87d.png)

从目前来看匿名页 struct page 算是与 anon_vma 建立了关系，又通过 anon_vma_chain 将 anon_vma 与 vm_area_struct 建立了关系。那么就剩下最后一道关系需要打通了，就是如何通过 anon_vma 找到 anon_vma_chain 进而找到 vm_area_struct 呢？这就需要我们将 anon_vma 与 anon_vma_chain 之间的关系也打通。

我们知道每个匿名页对应唯一的 anon_vma 结构，但是一个匿名物理页可以映射到不同进程的虚拟内存空间中，每个进程的虚拟内存空间都是独立的，也就是说不同的进程就会有不同的 VMA。

![image.png](https://cdn.xiaolincoding.com//mysql/other/554cc27bd03e1bd2d9cba6171879281f.png)

不同的 VMA 意味着同一个匿名页 anon_vma 就会对应多个 anon_vma_chain。那么如何通过一个 anon_vma 找到和他关联的所有 anon_vma_chain 呢？找到了这些 anon_vma_chain 也就意味着 struct page 找到了与它关联的所有进程虚拟内存空间 VMA。

我们看看能不能从 struct anon_vma 结构中寻找一下线索：

```c
struct anon_vma {
    struct anon_vma *root;      /* Root of this anon_vma tree */
    struct rw_semaphore rwsem; 
    atomic_t refcount;
    unsigned degree;
    struct anon_vma *parent;    /* Parent of this anon_vma */
    struct rb_root rb_root; /* Interval tree of private "related" vmas */
};
```

我们重点来看 struct anon_vma 结构中的 rb_root 字段，struct anon_vma 结构中管理了一颗红黑树，这颗红黑树上管理的全部都是与该 anon_vma 关联的 anon_vma_chain。我们可以通过 struct page 中的 mapping 指针找到 anon_vma，然后遍历 anon_vma 中的这颗红黑树 rb_root ，从而找到与其关联的所有 anon_vma_chain。

```c
struct anon_vma_chain {
    // 匿名页关联的进程虚拟内存空间（vma属于一个特定的进程，多个进程多个vma）
    struct vm_area_struct *vma;
    // 匿名页 page 指向的 anon_vma
    struct anon_vma *anon_vma;
    // 指向 vm_area_struct 中的 anon_vma_chain 列表
    struct list_head same_vma;   
    // anon_vma 管理的红黑树中该 anon_vma_chain 对应的红黑树节点
    struct rb_node rb;         
};
```

struct anon_vma_chain 结构中的 rb 字段表示其在对应 anon_vma 管理的红黑树中的节点。

![image.png](https://cdn.xiaolincoding.com//mysql/other/d20c19e6a57e303cae3b219de1215820.png)

到目前为止，物理内存页 page 到与其映射的进程虚拟内存空间 VMA，这样一种一对多的映射关系现在就算建立起来了。

而 vm_area_struct 表示的只是进程虚拟内存空间中的一段虚拟内存区域，这块虚拟内存区域中可能会包含多个匿名页，所以 VMA 与物理内存页 page 也是有一对多的映射关系存在。而这个映射关系在哪里保存呢？

大家注意 struct anon_vma_chain 结构中还有一个列表结构 same_vma，从这个名字上我们很容易就能猜到这个列表 same_vma 中存储的 anon_vma_chain 对应的 VMA 全都是一样的，而列表元素 anon_vma_chain 中的 anon_vma 却是不一样的。内核用这样一个链表结构 same_vma 存储了进程相应虚拟内存区域 VMA 中所包含的所有匿名页。

struct vm_area_struct 结构中的 `struct list_head anon_vma_chain` 指向的也是这个列表 same_vma。

```c
struct vm_area_struct {  
    // 存储该 VMA 中所包含的所有匿名页 anon_vma
    struct list_head anon_vma_chain;
    // 用于快速判断 VMA 有没有对应的匿名 page
    // 一个 VMA 可以包含多个 page，但是该区域内的所有 page 只需要一个 anon_vma 来反向映射即可。
    struct anon_vma *anon_vma;   
}
```

![image.png](https://cdn.xiaolincoding.com//mysql/other/d9c755dbf8b1bb329a43b83a5d0678ad-20230309235718694.png)

现在整个匿名页到进程虚拟内存空间的反向映射链路关系，我就为大家梳理清楚了，下面我们接着回到 do_anonymous_page 函数中，来一一验证上述映射逻辑：

```c
static vm_fault_t do_anonymous_page(struct vm_fault *vmf)
{
	struct vm_area_struct *vma = vmf->vma;
	struct page *page;	

        ........省略虚拟内存到物理内存正向映射相关逻辑.........

	if (unlikely(anon_vma_prepare(vma)))
		goto oom;

	page = alloc_zeroed_user_highpage_movable(vma, vmf->address);

	if (!page)
		goto oom;

	page_add_new_anon_rmap(page, vma, vmf->address);

        ........省略虚拟内存到物理内存正向映射相关逻辑.........
}
```

在 do_anonymous_page 中首先会调用 anon_vma_prepare 方法来为匿名页创建 anon_vma 实例和 anon_vma_chain 实例，并建立它们之间的关联关系。

```c
int __anon_vma_prepare(struct vm_area_struct *vma)
{
    // 获取进程虚拟内存空间
	struct mm_struct *mm = vma->vm_mm;
    // 准备为匿名页分配 anon_vma 以及 anon_vma_chain
	struct anon_vma *anon_vma, *allocated;
	struct anon_vma_chain *avc;
    // 分配 anon_vma_chain 实例
	avc = anon_vma_chain_alloc(GFP_KERNEL);
	if (!avc)
		goto out_enomem;
    // 在相邻的虚拟内存区域 VMA 中查找可复用的 anon_vma
	anon_vma = find_mergeable_anon_vma(vma);
	allocated = NULL;
	if (!anon_vma) {
        // 没有可复用的 anon_vma 则创建一个新的实例
		anon_vma = anon_vma_alloc();
		if (unlikely(!anon_vma))
			goto out_enomem_free_avc;
		allocated = anon_vma;
	}

	anon_vma_lock_write(anon_vma);
	/* page_table_lock to protect against threads */
	spin_lock(&mm->page_table_lock);
	if (likely(!vma->anon_vma)) {
        // VMA 中的 anon_vma 属性就是在这里赋值的
		vma->anon_vma = anon_vma;
        // 建立反向映射关联
		anon_vma_chain_link(vma, avc, anon_vma);
		/* vma reference or self-parent link for new root */
		anon_vma->degree++;
		allocated = NULL;
		avc = NULL;
	}
        .................
}
```

anon_vma_prepare 方法中调用 anon_vma_chain_link 方法来建立 anon_vma，anon_vma_chain，vm_area_struct 三者之间的关联关系：

```c
static void anon_vma_chain_link(struct vm_area_struct *vma,
				struct anon_vma_chain *avc,
				struct anon_vma *anon_vma)
{
    // 通过 anon_vma_chain 关联 anon_vma 和对应的 vm_area_struct
	avc->vma = vma;
	avc->anon_vma = anon_vma;
    // 将 vm_area_struct 中的 anon_vma_chain 链表加入到 anon_vma_chain 中的 same_vma 链表中
	list_add(&avc->same_vma, &vma->anon_vma_chain);
    // 将初始化好的 anon_vma_chain 加入到 anon_vma 管理的红黑树 rb_root 中
	anon_vma_interval_tree_insert(avc, &anon_vma->rb_root);
}
```

![image.png](https://cdn.xiaolincoding.com//mysql/other/d9c755dbf8b1bb329a43b83a5d0678ad.png)

到现在为止还缺关键的最后一步，就是打通匿名内存页 page 到 vm_area_struct 之间的关系，首先我们就需要调用 alloc_zeroed_user_highpage_movable 方法从伙伴系统中申请一个匿名页。当获取到 page 实例之后，通过 page_add_new_anon_rmap 最终建立起 page 到 vm_area_struct 的整条反向映射链路。

```c
static void __page_set_anon_rmap(struct page *page,
    struct vm_area_struct *vma, unsigned long address, int exclusive)
{
    struct anon_vma *anon_vma = vma->anon_vma;
           .........省略..............
    // 低位置 1
    anon_vma = (void *) anon_vma + PAGE_MAPPING_ANON;
    // 转换为 address_space 指针赋值给 page 结构中的 mapping 字段
    page->mapping = (struct address_space *) anon_vma;
    // page 结构中的 index 表示该匿名页在虚拟内存区域 vma 中的偏移
    page->index = linear_page_index(vma, address);
}
```

现在让我们再次回到本小节 《6.1 匿名页的反向映射》的开始，再来看这段话，是不是感到非常清晰了呢~~

> 如果当前物理内存页 struct page 是一个匿名页的话，那么 mapping 指针的最低位会被设置为 `1` ， 指向该匿名页在进程虚拟内存空间中的匿名映射区域 struct anon_vma 结构（每个匿名页对应唯一的 anon_vma 结构），用于物理内存到虚拟内存的反向映射。

> 如果当前物理内存页 struct page 是一个文件页的话，那么 mapping 指针的最低位会被设置为 `0` ，指向该内存页关联文件的 struct address_space（页高速缓存）。pgoff_t index 字段表示该内存页 page 在页高速缓存中的 index 索引，也表示该内存页中的文件数据在文件内部的偏移 offset。偏移单位为 page size。

struct page 结构中的 `struct address_space *mapping` 指针的最低位如何置 1 ，又如何置 0 呢？关键在下面这条语句：

```c
    struct anon_vma *anon_vma = vma->anon_vma;
    // 低位置 1
    anon_vma = (void *) anon_vma + PAGE_MAPPING_ANON;
```

anon_vma 指针加上 PAGE_MAPPING_ANON ，并转换为 address_space 指针，这样可确保 address_space 指针的低位为 1 表示匿名页。

address_space 指针在转换为 anon_vma 指针的时候可通过如下语句实现：

```c
anon_vma = (struct anon_vma *) (mapping - PAGE_MAPPING_ANON)
```

PAGE_MAPPING_ANON 常量定义在内核 `/include/linux/page-flags.h` 文件中：

```c
#define PAGE_MAPPING_ANON	0x1
```

而对于文件页来说，page 结构的 mapping 指针最低位本来就是 0 ，因为 address_space 类型的指针实现总是对齐至 `sizeof(long)`，因此在 Linux 支持的所有计算机上，指向 address_space 实例的指针最低位总是为 0 。

**内核可以通过这个技巧直接检查 page 结构中的 mapping 指针的最低位来判断该物理内存页到底是匿名页还是文件页**。

前面说了文件页的 page 结构的 index 属性表示该内存页 page 在磁盘文件中的偏移 offset ，偏移单位为 page size 。

那匿名页的 page 结构中的 index 属性表示什么呢？我们接着来看 linear_page_index 函数：

```c
static inline pgoff_t linear_page_index(struct vm_area_struct *vma,
                    unsigned long address)
{
    pgoff_t pgoff;
    if (unlikely(is_vm_hugetlb_page(vma)))
        return linear_hugepage_index(vma, address);
    pgoff = (address - vma->vm_start) >> PAGE_SHIFT;
    pgoff += vma->vm_pgoff;
    return pgoff;
}
```

逻辑很简单，就是表示匿名页在对应进程虚拟内存区域 VMA 中的偏移。

在本小节最后，还有一个与反向映射相关的重要属性就是 page 结构中的 _mapcount。

```c
struct page {
    struct address_space *mapping; 
    pgoff_t index;  
    // 表示该 page 映射了多少个进程的虚拟内存空间，一个 page 可以被多个进程映射
    atomic_t _mapcount
}
```

经过本小节详细的介绍，我想大家现在已经猜到 _mapcount 字段的含义了，我们知道一个物理内存页可以映射到多个进程的虚拟内存空间中，比如：共享内存映射，父子进程的创建等。page 与 VMA 是一对多的关系，这里的 _mapcount 就表示该物理页映射到了多少个进程的虚拟内存空间中。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_6-2-内存页回收相关属性)6.2 内存页回收相关属性

我们接着来看 struct page 中剩下的其他属性，我们知道物理内存页在内核中分为匿名页和文件页，在《5.2 物理内存区域中的水位线》小节中，我还提到过两个重要的链表分别为：active 链表和 inactive 链表。

其中 active 链表用来存放访问非常频繁的内存页（热页）， inactive 链表用来存放访问不怎么频繁的内存页（冷页），当内存紧张的时候，内核就会优先将 inactive 链表中的内存页置换出去。

内核在回收内存的时候，这两个列表中的回收优先级为：inactive 链表尾部 > inactive 链表头部 > active 链表尾部 > active 链表头部。

我们可以通过 `cat /proc/zoneinfo` 命令来查看不同 NUMA 节点中不同内存区域中的 active 链表和 inactive 链表中物理内存页的个数：

![image.png](https://cdn.xiaolincoding.com//mysql/other/e83cbf290eaf1e758ba97b8f119e5066.png)

- nr_zone_active_anon 和 nr_zone_inactive_anon 分别是该内存区域内活跃和非活跃的匿名页数量。
- nr_zone_active_file 和 nr_zone_inactive_file 分别是该内存区域内活跃和非活跃的文件页数量。

**为什么会有 active 链表和 inactive 链表**？

内存回收的关键是如何实现一个高效的页面替换算法 PFRA (Page Frame Replacement Algorithm) ，提到页面替换算法大家可能立马会想到 LRU (Least-Recently-Used) 算法。LRU 算法的核心思想就是那些最近最少使用的页面，在未来的一段时间内可能也不会再次被使用，所以在内存紧张的时候，会优先将这些最近最少使用的页面置换出去。在这种情况下其实一个 active 链表就可以满足我们的需求。

但是这里会有一个严重的问题，LRU 算法更多的是在时间维度上的考量，突出最近最少使用，但是它并没有考量到使用频率的影响，假设有这样一种状况，就是一个页面被疯狂频繁的使用，毫无疑问它肯定是一个热页，但是这个页面最近的一次访问时间离现在稍微久了一点点，此时进来大量的页面，这些页面的特点是只会使用一两次，以后将再也不会用到。

在这种情况下，根据 LRU 的语义这个之前频繁地被疯狂访问的页面就会被置换出去了（本来应该将这些大量一次性访问的页面置换出去的），当这个页面在不久之后要被访问时，此时已经不在内存中了，还需要在重新置换进来，造成性能的损耗。这种现象也叫 Page Thrashing（页面颠簸）。

因此，内核为了将页面使用频率这个重要的考量因素加入进来，于是就引入了 active 链表和 inactive 链表。工作原理如下：

1. 首先 inactive 链表的尾部存放的是访问频率最低并且最少访问的页面，在内存紧张的时候，这些页面被置换出去的优先级是最大的。
2. 对于文件页来说，当它被第一次读取的时候，内核会将它放置在 inactive 链表的头部，如果它继续被访问，则会提升至 active 链表的尾部。如果它没有继续被访问，则会随着新文件页的进入，内核会将它慢慢的推到 inactive 链表的尾部，如果此时再次被访问则会直接被提升到 active 链表的头部。大家可以看出此时页面的使用频率这个因素已经被考量了进来。
3. 对于匿名页来说，当它被第一次读取的时候，内核会直接将它放置在 active 链表的尾部，注意不是 inactive 链表的头部，这里和文件页不同。因为匿名页的换出 Swap Out 成本会更大，内核会对匿名页更加优待。当匿名页再次被访问的时候就会被被提升到 active 链表的头部。
4. 当遇到内存紧张的情况需要换页时，内核会从 active 链表的尾部开始扫描，将一定量的页面降级到 inactive 链表头部，这样一来原来位于 inactive 链表尾部的页面就会被置换出去。

> 内核在回收内存的时候，这两个列表中的回收优先级为：inactive 链表尾部 > inactive 链表头部 > active 链表尾部 > active 链表头部。

**为什么会把 active 链表和 inactive 链表分成两类，一类是匿名页，一类是文件页**？

在本文 《5.2 物理内存区域中的水位线》小节中，我为大家介绍了一个叫做 swappiness 的内核参数， 我们可以通过 `cat /proc/sys/vm/swappiness` 命令查看，swappiness 选项的取值范围为 0 到 100，默认为 60。

swappiness 用于表示 Swap 机制的积极程度，数值越大，Swap 的积极程度，越高越倾向于**回收匿名页**。数值越小，Swap 的积极程度越低，越倾向于**回收文件页**。

因为回收匿名页和回收文件页的代价是不一样的，回收匿名页代价会更高一点，所以引入 swappiness 来控制内核回收的倾向。

> 注意： swappiness 只是表示 Swap 积极的程度，当内存非常紧张的时候，即使将 swappiness 设置为 0 ，也还是会发生 Swap 的。

假设我们现在只有 active 链表和 inactive 链表，不对这两个链表进行匿名页和文件页的归类，在需要页面置换的时候，内核会先从 active 链表尾部开始扫描，当 swappiness 被设置为 0 时，内核只会置换文件页，不会置换匿名页。

由于 active 链表和 inactive 链表没有进行物理页面类型的归类，所以链表中既会有匿名页也会有文件页，如果链表中有大量的匿名页的话，内核就会不断的跳过这些匿名页去寻找文件页，并将文件页替换出去，这样从性能上来说肯定是低效的。

因此内核将 active 链表和 inactive 链表按照匿名页和文件页进行了归类，当 swappiness 被设置为 0 时，内核只需要去 nr_zone_active_file 和 nr_zone_inactive_file 链表中扫描即可，提升了性能。

其实除了以上我介绍的四种 LRU 链表（匿名页的 active 链表，inactive 链表和文件页的active 链表， inactive 链表）之外，内核还有一种链表，比如进程可以通过 mlock() 等系统调用把内存页锁定在内存里，保证该内存页无论如何不会被置换出去，比如出于安全或者性能的考虑，页面中可能会包含一些敏感的信息不想被 swap 到磁盘上导致泄密，或者一些频繁访问的内存页必须一直贮存在内存中。

当这些被锁定在内存中的页面很多时，内核在扫描 active 链表的时候也不得不跳过这些页面，所以内核又将这些被锁定的页面单独拎出来放在一个独立的链表中。

现在我为大家介绍五种用于存放 page 的链表，内核会根据不同的情况将一个物理页放置在这五种链表其中一个上。那么对于物理页的 struct page 结构中就需要有一个属性用来标识该物理页究竟被内核放置在哪个链表上。

```c
struct page {
   struct list_head lru;
   atomic_t _refcount;
}
```

struct list_head lru 属性就是用来指向物理页被放置在了哪个链表上。

atomic_t _refcount 属性用来记录内核中引用该物理页的次数，表示该物理页的活跃程度。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_6-3-物理内存页属性和状态的标志位-flag)6.3 物理内存页属性和状态的标志位 flag

```c
struct page {
    unsigned long flags;
} 
```

在本文 《2.3 SPARSEMEM 稀疏内存模型》小节中，我们提到，内核为了能够更灵活地管理粒度更小的连续物理内存，于是就此引入了 SPARSEMEM 稀疏内存模型。

![image.png](https://cdn.xiaolincoding.com//mysql/other/588f3357c39cd72180d3ccec659f8587.png)

SPARSEMEM 稀疏内存模型的核心思想就是提供对粒度更小的连续内存块进行精细的管理，用于管理连续内存块的单元被称作 section 。内核中用于描述 section 的数据结构是 struct mem_section。

由于 section 被用作管理小粒度的连续内存块，这些小的连续物理内存在 section 中也是通过数组的方式被组织管理（图中 struct page 类型的数组）。

每个 struct mem_section 结构体中有一个 section_mem_map 指针用于指向连续内存的 page 数组。而所有的 mem_section 也会被存放在一个全局的数组 mem_section 中。

**那么给定一个具体的 struct page，在稀疏内存模型中内核如何定位到这个物理内存页到底属于哪个 mem_section 呢** ？这是第一个问题~~

我在《5. 内核如何管理 NUMA 节点中的物理内存区域》小节中讲到了内存的架构，在 NUMA 架构下，物理内存被划分成了一个一个的内存节点（NUMA 节点），在每个 NUMA 节点内部又将其所管理的物理内存按照功能不同划分成了不同的内存区域 zone，每个内存区域管理一片用于特定具体功能的物理内存 page。

> 物理内存在内核中管理的层级关系为：None -> Zone -> page

![image.png](https://cdn.xiaolincoding.com//mysql/other/bba0c0b540cbfcbbb709077d2a22ee3d-20230309235625678.png)

**那么在 NUMA 架构下，给定一个具体的 struct page，内核又该如何确定该物理内存页究竟属于哪个 NUMA 节点，属于哪块内存区域 zone 呢**？ 这是第二个问题。

关于以上我提出的两个问题所需要的定位信息全部存储在 struct page 结构中的 flags 字段中。前边我们提到，struct page 是 Linux 世界里最繁华的地段，这里的地价非常昂贵，所以 page 结构中这些字段里的每一个比特内核都会物尽其用。

```c
struct page {
    unsigned long flags;
} 
```

因此这个 unsigned long 类型的 flags 字段中不仅包含上面提到的定位信息还会包括物理内存页的一些属性和标志位。flags 字段的高 8 位用来表示 struct page 的定位信息，剩余低位表示特定的标志位。

![image.png](https://cdn.xiaolincoding.com//mysql/other/a13de2f9a59d461f2b4eed6338388d0d.png)

struct page 与其所属上层结构转换的相应函数定义在 `/include/linux/mm.h` 文件中：

```c
static inline unsigned long page_to_section(const struct page *page)
{
	return (page->flags >> SECTIONS_PGSHIFT) & SECTIONS_MASK;
}

static inline pg_data_t *page_pgdat(const struct page *page)
{
	return NODE_DATA(page_to_nid(page));
}

static inline struct zone *page_zone(const struct page *page)
{
	return &NODE_DATA(page_to_nid(page))->node_zones[page_zonenum(page)];
}
```

在我们介绍完了 flags 字段中高位存储的位置定位信息之后，接下来就该来介绍下在低位比特中表示的物理内存页的那些标志位~~

物理内存页的这些标志位定义在内核 `/include/linux/page-flags.h`文件中：

```c
enum pageflags {
	PG_locked,		/* Page is locked. Don't touch. */
	PG_referenced,
	PG_uptodate,
	PG_dirty,
	PG_lru,
	PG_active,
	PG_slab,
	PG_reserved,
    PG_compound,
	PG_private,		
	PG_writeback,		
	PG_reclaim,		
#ifdef CONFIG_MMU
	PG_mlocked,		/* Page is vma mlocked */
	PG_swapcache = PG_owner_priv_1,	

        ................
};
```

- PG_locked 表示该物理页面已经被锁定，如果该标志位置位，说明有使用者正在操作该 page , 则内核的其他部分不允许访问该页， 这可以防止内存管理出现竞态条件，例如：在从硬盘读取数据到 page 时。
- PG_mlocked 表示该物理内存页被进程通过 mlock 系统调用锁定常驻在内存中，不会被置换出去。
- PG_referenced 表示该物理页面刚刚被访问过。
- PG_active 表示该物理页位于 active list 链表中。PG_referenced 和 PG_active 共同控制了系统使用该内存页的活跃程度，在内存回收的时候这两个信息非常重要。
- PG_uptodate 表示该物理页的数据已经从块设备中读取到内存中，并且期间没有出错。
- PG_readahead 当进程在顺序访问文件的时候，内核会预读若干相邻的文件页数据到 page 中，物理页 page 结构设置了该标志位，表示它是一个正在被内核预读的页。相关详细内容可回看我之前的这篇文章 [《从 Linux 内核角度探秘 JDK NIO 文件读写本质》(opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247486623&idx=1&sn=0cafed9e89b60d678d8c88dc7689abda&chksm=ce77cad8f90043ceaaca732aaaa7cb692c1d23eeb6c07de84f0ad690ab92d758945807239cee&token=1468822011&lang=zh_CN#rd)
- PG_dirty 物理内存页的脏页标识，表示该物理内存页中的数据已经被进程修改，但还没有同步会磁盘中。我在 [《从 Linux 内核角度探秘 JDK NIO 文件读写本质》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247486623&idx=1&sn=0cafed9e89b60d678d8c88dc7689abda&chksm=ce77cad8f90043ceaaca732aaaa7cb692c1d23eeb6c07de84f0ad690ab92d758945807239cee&token=1468822011&lang=zh_CN#rd)一文中也详细介绍过。
- PG_lru 表示该物理内存页现在被放置在哪个 lru 链表上，比如：是在 active list 链表中 ？ 还是在 inactive list 链表中 ？
- PG_highmem 表示该物理内存页是在高端内存中。
- PG_writeback 表示该物理内存页正在被内核的 pdflush 线程回写到磁盘中。详情可回看文章[《从 Linux 内核角度探秘 JDK NIO 文件读写本质》 (opens new window)](https://mp.weixin.qq.com/s?__biz=Mzg2MzU3Mjc3Ng==&mid=2247486623&idx=1&sn=0cafed9e89b60d678d8c88dc7689abda&chksm=ce77cad8f90043ceaaca732aaaa7cb692c1d23eeb6c07de84f0ad690ab92d758945807239cee&token=1468822011&lang=zh_CN#rd)。
- PG_slab 表示该物理内存页属于 slab 分配器所管理的一部分。
- PG_swapcache 表示该物理内存页处于 swap cache 中。 struct page 中的 private 指针这时指向 swap_entry_t 。
- PG_reclaim 表示该物理内存页已经被内核选中即将要进行回收。
- PG_buddy 表示该物理内存页是空闲的并且被伙伴系统所管理。
- PG_compound 表示物理内存页属于复合页的其中一部分。
- PG_private 标志被置位的时候表示该 struct page 结构中的 private 指针指向了具体的对象。不同场景指向的对象不同。

除此之外内核还定义了一些标准宏，用来检查某个物理内存页 page 是否设置了特定的标志位，以及对这些标志位的操作，这些宏在内核中的实现都是原子的，命名格式如下：

- PageXXX(page)：检查 page 是否设置了 PG_XXX 标志位
- SetPageXXX(page)：设置 page 的 PG_XXX 标志位
- ClearPageXXX(page)：清除 page 的 PG_XXX 标志位
- TestSetPageXXX(page)：设置 page 的 PG_XXX 标志位，并返回原值

另外在很多情况下，内核通常需要等待物理页 page 的某个状态改变，才能继续恢复工作，内核提供了如下两个辅助函数，来实现在特定状态的阻塞等待：

```c
static inline void wait_on_page_locked(struct page *page)
static inline void wait_on_page_writeback(struct page *page)
```

当物理页面在锁定的状态下，进程调用了 wait_on_page_locked 函数，那么进程就会阻塞等待知道页面解锁。

当物理页面正在被内核回写到磁盘的过程中，进程调用了 wait_on_page_writeback 函数就会进入阻塞状态直到脏页数据被回写到磁盘之后被唤醒。

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_6-4-复合页-compound-page-相关属性)6.4 复合页 compound_page 相关属性

我们都知道 Linux 管理内存的最小单位是 page，每个 page 描述 4K 大小的物理内存，但在一些对于内存敏感的使用场景中，用户往往期望使用一些巨型大页。

> 巨型大页就是通过两个或者多个物理上连续的内存页 page 组装成的一个比普通内存页 page 更大的页，

因为这些巨型页要比普通的 4K 内存页要大很多，所以遇到缺页中断的情况就会相对减少，由于减少了缺页中断所以性能会更高。

另外，由于巨型页比普通页要大，所以巨型页需要的页表项要比普通页要少，页表项里保存了虚拟内存地址与物理内存地址的映射关系，当 CPU 访问内存的时候需要频繁通过 MMU 访问页表项获取物理内存地址，由于要频繁访问，所以页表项一般会缓存在 TLB 中，因为巨型页需要的页表项较少，所以节约了 TLB 的空间同时降低了 TLB 缓存 MISS 的概率，从而加速了内存访问。

还有一个使用巨型页受益场景就是，当一个内存占用很大的进程（比如 Redis）通过 fork 系统调用创建子进程的时候，会拷贝父进程的相关资源，其中就包括父进程的页表，由于巨型页使用的页表项少，所以拷贝的时候性能会提升不少。

以上就是巨型页存在的原因以及使用的场景，但是在 Linux 内存管理架构中都是统一通过 struct page 来管理内存，而巨型大页却是通过两个或者多个物理上连续的内存页 page 组装成的一个比普通内存页 page 更大的页，那么巨型页的管理与普通页的管理如何统一呢？

这就引出了本小节的主题-----复合页 compound_page，下面我们就来看下 Linux 如果通过统一的 struct page 结构来描述这些巨型页（compound_page）：

虽然巨型页（compound_page）是由多个物理上连续的普通 page 组成的，但是在内核的视角里它还是被当做一个特殊内存页来看待。

下图所示，是由 4 个连续的普通内存页 page 组成的一个 compound_page：

![image.png](https://cdn.xiaolincoding.com//mysql/other/54a92bb6502d54b58c3bfea2879e0653.png)

组成复合页的第一个 page 我们称之为首页（Head Page），其余的均称之为尾页（Tail Page）。

我们来看一下 struct page 中关于描述 compound_page 的相关字段：

```c
      struct page {      
            // 首页 page 中的 flags 会被设置为 PG_head 表示复合页的第一页
            unsigned long flags;	
            // 其余尾页会通过该字段指向首页
            unsigned long compound_head;   
            // 用于释放复合页的析构函数，保存在首页中
            unsigned char compound_dtor;
            // 该复合页有多少个 page 组成，order 还是分配阶的概念，首页中保存
            // 本例中的 order = 2 表示由 4 个普通页组成
            unsigned char compound_order;
            // 该复合页被多少个进程使用，内存页反向映射的概念，首页中保存
            atomic_t compound_mapcount;
            // 复合页使用计数，首页中保存
            atomic_t compound_pincount;
      }
```

首页对应的 struct page 结构里的 flags 会被设置为 PG_head，表示这是复合页的第一页。

另外首页中还保存关于复合页的一些额外信息，比如用于释放复合页的析构函数会保存在首页 struct page 结构里的 compound_dtor 字段中，复合页的分配阶 order 会保存在首页中的 compound_order 中，以及用于指示复合页的引用计数 compound_pincount，以及复合页的反向映射个数（该复合页被多少个进程的页表所映射）compound_mapcount 均在首页中保存。

复合页中的所有尾页都会通过其对应的 struct page 结构中的 compound_head 指向首页，这样通过首页和尾页就组装成了一个完整的复合页 compound_page 。

![image.png](https://cdn.xiaolincoding.com//mysql/other/76d48cc5a2033be3bcd767728b5cc6bf.png)

### [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#_6-5-slab-对象池相关属性)6.5 Slab 对象池相关属性

> 本小节只是对 slab 的一个简单介绍，大家有个大概的印象就可以了，后面我会有一篇专门的文章为大家详细介绍 slab 的相关实现细节，到时候还会在重新详细介绍 struct page 中的相关属性。

内核中对内存页的分配使用有两种方式，一种是一页一页的分配使用，这种以页为单位的分配方式内核会向相应内存区域 zone 里的伙伴系统申请以及释放。

另一种方式就是只分配小块的内存，不需要一下分配一页的内存，比如前边章节中提到的 struct page ，anon_vma_chain ，anon_vma ，vm_area_struct 结构实例的分配，这些结构通常就是几十个字节大小，并不需要按页来分配。

为了满足类似这种小内存分配的需要，Linux 内核使用 slab allocator 分配器来分配，slab 就好比一个对象池，内核中的数据结构对象都对应于一个 slab 对象池，用于分配这些固定类型对象所需要的内存。

它的基本原理是从伙伴系统中申请一整页内存，然后划分成多个大小相等的小块内存被 slab 所管理。这样一来 slab 就和物理内存页 page 发生了关联，由于 slab 管理的单元是物理内存页 page 内进一步划分出来的小块内存，所以当 page 被分配给相应 slab 结构之后，struct page 里也会存放 slab 相关的一些管理数据。

```c
struct page {

        struct {    /* slab, slob and slub */
            union {
                struct list_head slab_list;
                struct {    /* Partial pages */
                    struct page *next;
#ifdef CONFIG_64BIT
                    int pages;  /* Nr of pages left */
                    int pobjects;   /* Approximate count */
#else
                    short int pages;
                    short int pobjects;
#endif
                };
            };
            struct kmem_cache *slab_cache; /* not slob */
            /* Double-word boundary */
            void *freelist;     /* first free object */
            union {
                void *s_mem;    /* slab: first object */
                struct {            /* SLUB */
                    unsigned inuse:16;
                    unsigned objects:15;
                    unsigned frozen:1;
                };
            };
        };

}
```

- struct list_head slab_list ：slab 的管理结构中有众多用于管理 page 的链表，比如：完全空闲的 page 链表，完全分配的 page 链表，部分分配的 page 链表，slab_list 用于指定当前 page 位于 slab 中的哪个具体链表上。
- struct page *next ： 当 page 位于 slab 结构中的某个管理链表上时，next 指针用于指向链表中的下一个 page。
- int pages : 表示 slab 中总共拥有的 page 个数。
- int pobjects ： 表示 slab 中拥有的特定类型的对象个数。
- struct kmem_cache *slab_cache ： 用于指向当前 page 所属的 slab 管理结构，通过 slab_cache 将 page 和 slab 关联起来。
- void *freelist ： 指向 page 中的第一个未分配出去的空闲对象，前面介绍过，slab 向伙伴系统申请一个或者多个 page，并将一整页 page 划分出多个大小相等的内存块，用于存储特定类型的对象。
- void *s_mem ： 指向 page 中的第一个对象。
- unsigned inuse ： 表示 slab 中已经被分配出去的对象个数，当该值为 0 时，表示 slab 中所管理的对象全都是空闲的，当所有的空闲对象达到一定数目，该 slab 就会被伙伴系统回收掉。
- unsigned objects ： slab 中所有的对象个数。
- unsigned frozen : 当前内存页 page 被 slab 放置在 CPU 本地缓存列表中，frozen = 1，否则 frozen = 0 。

## [#](https://xiaolincoding.com/os/3_memory/linux_mem2.html#总结)总结

到这里，关于 Linux 物理内存管理的相关内容我就为大家介绍完了，本文的内容比较多，尤其是物理内存页反向映射相关的内容比较复杂，涉及到的关联关系比较多，现在我在带大家总结一下本文的主要内容，方便大家复习回顾：

在本文的开始，我首先从 CPU 角度为大家介绍了三种物理内存模型：FLATMEM 平坦内存模型，DISCONTIGMEM 非连续内存模型，SPARSEMEM 稀疏内存模型。

随后我又接着介绍了两种物理内存架构：一致性内存访问 UMA 架构，非一致性内存访问 NUMA 架构。

在这个基础之上，又按照内核对物理内存的组织管理层次，分别介绍了 Node 节点，物理内存区域 zone 等相关内核结构。它们的层次如下图所示：

![image.png](https://cdn.xiaolincoding.com//mysql/other/bba0c0b540cbfcbbb709077d2a22ee3d.png)

在把握了物理内存的总体架构之后，又引出了众多细节性的内容，比如：物理内存区域的管理与划分，物理内存区域中的预留内存，物理内存区域中的水位线及其计算方式，物理内存区域中的冷热页。

最后，我详细介绍了内核如何通过 struct page 结构来描述物理内存页，其中匿名页反向映射的内容比较复杂，需要大家多多梳理回顾一下。