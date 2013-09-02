---
layout: default
---

# libll++ blog

我是一个40多岁的程序员，生活在中国，北京。在1998-2013年之间，我一直用c语言开发类unix系统的服务器应用。2013年，我打算陪我儿子度过一个夏季，有点空闲时间，我突然想试试c++。由于以前的开发背景，我没有boost相关的知识背景，但是c++11已经被标准化了。我想我没必要从最原始开始，就从c++11开始吧。c++的思维模式跟c语言有很大的差别，在此期间我经历了很长时间的思维冲突，反复的思考和困惑，甚至现在有时候也是这样。

libll++是我这段时间学习开发库，发布在：https://github.com/storming/libllpp.git.

如果你对libll++有任何建议，可以发email给我：storm@vip.163.com

2013-9-1

------------------------------------------------------------------------------------------------------------------------------------------
##2013-9-2 list

昨天提到的member.h里的内容并不多。但是，这是我第一次大量开始使用模板、模板偏例和接触c++11，整个过程显得颇为曲折，改了很多版本最终是现在这个
样子。我想以后可能还会有所变化，这样发现更好的方式，哪怕是把整个库推翻了，也在所不惜。呵呵，毕竟这是个学习库。

在处理完member后，我就迫不及待的去开始动手写我的第一个实用数据结构封装，list。

list是个非常非常常用的数据组织方式，几乎每天都要用到，在所有数据结构中它的出场率我认为是最高的。在c语言中，对数据结构的操作往往比较细致。
这跟很多高级语言或者c++的stl有本质的不同。stl的list好像是个循环链表，也仅此而已。而在用c开发应用的时候，常用的list有4种。

####slist，一个单纯的单向链表。

可以参阅linux的queue.h的SLIST_相关宏。下面是个简单示例。

	struct entry {
        	struct type *slh_first; /* first element */
	}
	struct head {
	       	struct type *sle_next;  /* next element */
	}

链表和节点都是一个指针，相当的简单和紧凑。它在使用上限制颇多，只能对链表头进行快速insert和remove操作，如果要删除某个中间节点必须要找到该节点
的上一个节点。用c++的语意来说它只能很好的完成push_front和pop_front。不过，我们应该满足，这么小的开销完成这些已经很不错了。但是在实际应用中
它的适用面相当的广泛。我随便举一些简单的例子。

单向链表往往具有堆栈的特性，当对它进行head操作的时候，数据是FILO的。程序递归的本质是建立在堆栈上的，这个堆栈不只是调用堆栈。有时候，我们可能
想避免调用递归，就会选择一个调用堆栈的替代堆栈。单向链表是个天然的良好替代品，简单，高效，开销小。

有时候，我们会把它用到内存回收算法上。由于任何内存分配器都要考虑对齐问题，比如malloc(1)实际分配的可用内存大小是`sizeof(void*)`。而slist的entry
只有一个指针，完全符合内存分配器的最小分配。下面的示例对相同大小的内存进行cache。

	struct node {
		entry _entry;
	};
	head recycle_list;
	void *alloc() 
	{
		node *p = recycle_list.pop_front();
		if (!p) {
			p = malloc(some_size);
		}
		return p;
	}
	void free(void *p) 
	{
		recycle_list.push_front((node*)p);
	}


还有一些比较常规的想法，比如我们只对当前状态感兴趣，也就是head指向的那个元素，同时我们想在某个时机去追溯以前的元素。只要不是想直接remove一个
中间元素，slist都是非常不错的选择。在以后obstack的代码中，会看到这个分配器是怎么用slist去管理它的page列表的。一个简单的比喻来总结它，head
指向的是当前，entry的next指向的是过去。

slist的另外一个重要的出场场合是它是hash表的一个基础数据结构。

在这里要说一下使用链表基本意图，使用链表是一定要遍历它的，否则就不存在使用它的意义。但是遍历链表不等于用遍历去定位元素，这在
链表操作中是要尽量避免的。当然事情没有绝对的，如果理论上确认链表足够的短，直接遍历定位还是可行的。就像有些数据库相关文章说到，
在表足够小的时候，不要建索引，索引的维护反倒导致性能下降。我们在讨论元素定位的时候，还要讨论插入和删除时数据结构的效率。想象
一下，一个8元素有序数组2分法查找或者8元素avl查找，最差是3次比对看上去比链表要好很多，但是他们插入删除的开销则高很多。hash表
的散列在理论上决定了链表的长度不会很高，所以它往往是用链表作为散列后数据组织工具。不过在通常意义下，slist只是使用push_front，
pop_front, for遍历，尽量不要考虑元素定位和中间元素删除。

有一个面试题，会经常出现在很多公司的考卷上：把一个单向链表反转。这其实就是在考单向链表head操作的stack特性。从原链表上pop_front，
push_front到新的链表，就完成了反转过程。而且，没有额外开销。


####stlist，一个可以尾部插入的slist增强版

可以参阅linux的queue.h的STAILQ_相关宏。它的entry跟slist完全一样，只是head不同。

	struct head {
        	struct type *stqh_first;        /* first element */
        	struct type **stqh_last;        /* addr of last next element */
	}

前面提到我们一般对链表的两端进行操作，尽量不去插入或者移除中间元素。这让链表具有了一个特性，时间有序性(我简称为时序性)。链表头部操作都是FILO，
有时候我们不满足于这种情况，我们希望FIFO，也就是实现队列。这需要在slist的基础上多出一个指针来记录队尾的状态。跟普通数据结构书籍上提到的
不同，这个实现的last指向了entry->next的地址，也就是说它是个指针的指针。如果是个空链表或者初始化态，last指向first的地址。这极大的提高
了尾部插入的性能。

slist在插入时序上是逆序，stlist的push_back则是正序，当然了push_front还是逆序。比如说一个xml解析，我们可以把xml node当作一个多叉树的
节点，node->children其实是个链表。如果我们想用单向链表来完成sibling关系的组织，为了保障节点的有序性stlist就是个很好的选择。这往往会
出现在sax解析中，我们需要自己去组织节点关系。

####list，最常用且功能强大的list

可以参阅linux的queue.h的LIST_相关宏，它的head和slist完全一样，只是entry多了个指针。

	struct entry {
		type *next;
		type **prev; /* addr of prev element's next, 有些代码会用ref这个词而不是prev */
	};

list跟slist类似，在本质上只是个单向链表，但是它的entry多了个指针，让它具有了一个特殊的能力：自移除能力。由于prev指向的是上个element
的next指针的地址（如果是push_front，会指向head的first），所以element可以不去遍历list就能把自己从链表中移除出去。甚至element不需要
知道它属于哪个链表，只要它知道它在链表里就能把自己移除出去。这很有意思，slist的特性这里就不再说了，只是讨论remove self的使用。

	enum {
		peer_closed,
		peer_connecting,
		peer_connected,
		peer_idle,
		peer_busy,
		peer_linger,
		peer_state_max,
	};
	struct peer_t {
		int state;
		entry entry;
	};
	list peer_lists[peer_state_max];
	void update_peer_state(peer_t *peer, int state) {
		if (peer->state == state) {
			return;
		}
		list_remove(peer->entry);
	
		//do something
	
		peer->state = state;
		peer_lists[state].push_front(peer);
	}
	
上面这个例子有时候用在高性能异步网络开发上。list_remove表现了2种能力，第一不去判断自己属于哪个链表，第二不去遍历去确定上一个是谁，
程序逻辑只是确定它肯定在某个链表里，直接把自己从链表中remove出去。这产生了极高的性能。

如果hash表使用list作为散列后数据结构，那么这个hash表就也具有这种特性。不需要通过key，element直接就能把自己从hash表中remove出去。在
网络开发中，每个peer一般都会有个逻辑id，比如说用户名。当侦测到对端关闭了连接，这时候我们需要清除这个context。常规想法是通过id去删除
相应的hash entry，但是list的这种特性可以让程序员直接完成这个操作而不必通过比对查找。这会产生极高的性能提升。

libll++的默认内存池来自于apr pool，在以后的blog中会提到它。apr pool的chunk管理使用的就是这种list，但是它把它的应用提到了一个新的
高度。一般list的初始化只是把first清0即可。每次push_front，element的prev实际是指向了first的地址。apr pool的chunk管理有个前提，就是
链表里至少要有一个chunk，它的实现示例代码如下：

	void pool_init(pool *pool) {
		chunk *new_chunk = some alloc....
		new_chunk->next = null;
		new_chunk->prev = &new_chunk->next;
		pool->chunk_list = new_chunk;
	}

这里prev不是指向了head的first，而是指向了自己的next。这导致它完成了一个单向循环链表，而且不丢失remove self能力。我第一次看到这种
设计，给我的第一感觉是老式的拨号盘电话。list的head也不再具有特定的意义，它只是标注最关心的当前的那个节点，也就是“主分配偏好”节点。
这是splay tree的单向循环链表版本，相当的让人叹为观止。

####clist，一个万能的双向循环链表


------------------------------------------------------------------------------------------------------------------------------------------

##2013-9-1 member

对于c语言工程师来说，使用内存池和嵌入式的数据结构还有宏是相当习惯的事情。某种意义上，这种语法上的缺失保障了c语言程序的高效，就像拥有越少的人越努力。
下面的例子是个很标准的c语言链表声明。

	struct foo {
		LIST_ENTRY(foo) _entry
	}


这种链表相关宏，在linux的queue.h中。这跟STL的方式有较大的不同，STL是数据算法分离模式，而在c中往往是融合模式。
这在内存处理上有较大的优势。而STL则显得更加学术性。
这种模式阐述了一种本质，代码是静态的，只有数据是动态的。静态说明本质。如果你曾经属于某个容器，你就具有这个容器的特质。
在现实的开发中，这屡屡被验证，甚至几乎不会出现偏差。

如果要在c++中实现c的这种模式，难度是很大的。offsetof在g++中需要编译选项，container_of(这是linux kernel的宏)就更加困难了。

好在c++模板支持类成员地址，这给我的想法提供了一线生机。
对于开发者意图来说，如果用c++，我就不太情愿用宏，除非在语法上无法再简化了。
效率不是我担心的问题，到语法树级别，c和c++可能会有差别，但是差距不大。

为此，我在libll++里写的第一个文件就是member.h，它是一切class member相关操作的基础。
考虑到c++是个多重继承语言，在做这方面的操作的时候，需要有个清晰的认识。

	/* typeof_member */
	template <typename _T, typename _C>
	_T typeof_member_helper(_T _C::*member);
	#define typeof_member(x) decltype(ll::typeof_member_helper(x))


	/* typeof_container */
	template <typename _T, typename _C>
	_C typeof_container_helper(_T _C::*member);
	#define typeof_container(x) decltype(ll::typeof_container_helper(x))

类成员地址模板很麻烦，c++11里有那么多的aoto也不是空穴来风。这2个宏一个是提取类成员的类型，一个是提取类成员的宿主。
这是个很有意思的事情。

	struct foo {
		some_entry _entry;
	};

	struct foo2 : foo {
		something
	};

那么typeof_container(&foo2::_entry)是哪个？是foo，这个要注意。

offsetof_member和containerof_member是c++版的offsetof和container_of。我的核心想法是要在c++上实现c语言级的效率。
stl过于学术性，它的分离性，导致它的过分依赖内存分配效率。

member.h剩下的部分是个很大的宏，它用c++的sfinae去检测类成员类型是否存在。
在很少的情况下，会用到它。但是，我把它当作脑筋急转弯收录到member.h中。
里面的模板偏例应用相当高超，我的第一个模板偏例例子就是这个，我整整看了2天才看明白。
至于能够自己完成，则在一段时间以后。

