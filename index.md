---
layout: default
---

# libll++ blog

我是一个40多岁的程序员，生活在中国，北京。在1998-2013年之间，我一直用c语言开发类unix系统的服务器应用。2013年，我打算陪我儿子度过一个夏季，有点空闲时间，我突然想试试c++。由于以前的开发背景，我没有boost相关的知识背景，但是c++11已经被标准化了。我想我没必要从最原始开始，就从c++11开始吧。c++的思维模式跟c语言有很大的差别，在此期间我经历了很长时间的思维冲突，反复的思考和困惑，甚至现在有时候也是这样。

libll++是我这段时间学习开发库，发布在：https://github.com/storming/libllpp.git.

如果你对libll++有任何建议，可以发email给我：storm@vip.163.com

2013-9-1

------------------------------------------------------------------------------------------------------------------------------------------
##2013-9-9 closure and lambda and tuple_apply

记不清第一次接触closure这个词儿是什么时候了，不过应该是在学习某种脚本语言的时候看到的，lua、C#、AS3？这个单词的计算机类解释是“闭包”，
坦率的说当时没理解“闭包”的含义。我被这种高度抽象、深邃的中文解释迷惑了，我每每都是这么愚钝。我的应对方法就是放弃中文解释，closure就是closure。

我喜欢用c语言来解释现代计算机语言的高级概念，简单，直接。在服务器开发应用中，几乎所有的程序都会用到timer，有时候timer是唯一可依据的“轴”。
比如说一个socket server，accept一个connection进来，到close这个connection，需要一个timer始终跟随它。accept进来后多长时间内收到handshark包，
流控，keep heartbeat，关闭的时候linger close，这些都需要一个timer。下面示例是个简单的timer interface。

	typedef int (*timer_handler_t)(struct timer *, struct timeval*, void *);
	struct timer {
		rbtree_node node; //一般都是用红黑树来管理timer
		struct timeval expires;
		struct timeval interval;
		timer_handler_t handler;
		void *magic;
	};
	struct timer *timer_schedule(struct timeval *expires, struct timeval *interval, timer_handler_t handler, void *magic);

这种数据描述在c里面是非常常见的，特别是在事件驱动程序中，timer管理是个典型的事件驱动模式。timer_handler_t明显是个callback，在c
中往往一看到这种typedef就直接归类到callback，这类callback的最后一个参数基本都是`void *`。callback函数只是定义了方法，`void *`里放置
的是具体callback的context。比如说上面socket server的例子，context可能是某个peer的指针，或者创建了一个全局“秒级”timer，去回收某些资源，
context是某些全局数据结构指针。或者函数知道context是什么，传入个NULL。这些context对timer manager来说是透明的，manager不去解释它，一般我会用
magic这个词来声明类似的变量。

如果把context和callback放到一个结构中，入下面的示例：

	struct timer_closure {
		int (*handler)(struct timer *, struct timeval *, void *);
		void *context;
	};
	struct timer {
		rbtree_node node; //一般都是用红黑树来管理timer
		struct timeval expires;
		struct timeval interval;
		struct timer_closure closure;
	};
	struct timer *timer_schedule(struct timeval *expires, struct timeval *interval, timer_closure *closure);

那么，timer_closure就是个传统意义的closure：一个带context的callback。有些解释说：一个有状态的callback。基本含义都是相同的。在第一个例子
中，本质上也是closure，有callback有context，只不过是分离传入的。从本质上来说所有的callback都是closure，区别只是有些context是默认已知的，
有些context是需要明确指定的。

上面讲的是c的closure实现，那么在脚本语言中closure会有不同么？没有本质的不同，比如说下面的as3代码。

	public class foo {
		static public function dodo(var callback: function): void {
			callback();
		}
	};
	public class foo2 {
		public function doit(): void {
			//do someting
		}
	}
	
	var f: foo2 = new foo2;
	foo::dodo(f.doit);

传入的是`f.doit`而不是foo2::doit，可以这么理解：它把foo2的实例和doit这个函数“打包”传给了foo::dodo。

c语言的closure看上去没什么不好，简单清晰，它从c存在开始就一直被广泛使用，当然了大多数时候没人叫它closure。
事件驱动这种最简单的程序模式，它的应用面却是非常非常广泛的。既然涉及到事件驱动，必须要讨论callback，或者其它
一些本质上无区别的名称上有区别的方式，closure, slot and signal, message queue（send模式）。在这段文字中，
我阐述了一些“嘲讽”和轻佻的“自嘲”。我讨厌过渡封装，如果一个简单的概念被封装的让人看不懂，这算是什么呐？
另外一方面我会在libll++里大量使用closure：噢，我在用c++，这必须要稍微高端点，让封装来的更猛烈点吧。
libll++是个实验性工程，我在暗示自己没必要搞得那么正规。

在c++11的世界里，closure有很多种实现方式。首先就是语言级支持的lambda。第一次看到这个东西让我兴奋异常，
这东西太cool了，然后就是一盆凉水。

自从在baidu和google上看到lambda，我就开始拷问g++，可能有一周的时间。想像一下，一个恶魔拿着鞭子反复的抽
g++ 4.7。我试图用个简单的方式来解释lambda，下面是个lambda代码：

	template <typename _F>
	void walk() {
		for (something) {
			f(v);
		}
	}
	
	int do_sum() {
		int sum = 0;
		walk([&](int n) {sum += n});
		return sum;
	}

下面是我猜测的lambda的实现，c++环境下编译器去生成代码是相当普遍的，最早的就是copy构造。在g++里，lambda
是在基本语言环境下自动生成的。

	int do_sum() {
		int sum = 0;
		class lambda_n {
			int &_sum;
		public:
			lambda_n(int& sum) : _sum(sum) {}
			void operator()(int n) {
				_sum += n;
			}
		} instance_n(sum);
		
		walk(instance_n);
		return sum;
	}

在g++中，lambda是个函数内联类，至少在现在的版本是这样，它符合所有的函数内联类的特性。也就是说每个lambda是个类，
准确的说是个重载了()的functor类。而且这是个特殊的类，我们无法decltype它，你只能用auto去索引它，它的大小根据
它的捕获参数不同而不同。这让人很沮丧。

callback一般有两种模式，同步callback和异步callback。walk和do_sum这个例子就是个同步callback，context的生命周期
跟调用函数一致。异步callback的context的生命周期则是不确定的，比如说事件驱动。

lambda是可以赋值给function的。从语法角度，只要是函数可见域，lambda就可以捕获。我对于这种赋值是否依然生命周期具有局部性
没有继续研究下去。它违背了我的想法，我希望一个异步closure，我希望能够管控内存分配。就是lambda和function配合完成了
第一项，它也无法完成第二项，lambda是变长的内存怎么控制？

ok，有些人会说如果lambda不捕获任何变量，你可以把它当作一个静态函数用。我想说，不捕获变量，它还有什么用？我就
那么懒得写个函数么？

为此，在closure的道路上我首先排除了lambda。c++还是很丰富的还有function和bind。

在这里，我比较郁闷的是我很久很久没有跟踪c++的发展，特别是boost和looki。我只是2个月前开始知道他们，但是没有从源代码
级别去分析他们。c++11的很多类就够我看的了，function显然是boost的function的标准化版本。我对此不太理解，一个语言需要
定义这种级别的功能么？看上去像java。c++定义move copy和move assign，这很恰当。定义function，至少我是很难理解。

bind是个函数模板，它的返回类型未定义。function对于具体实现也差不多。我比较困惑，我希望精确控制内存和构造。
另外一个方面这是个实验性工程，我想去做个自己的“轮子”。于是，我开始设计libll++的closure。

在c++领域，callback有两种模式。在讨论这个之前，先讨论下函数签名，简单的说就是函数参数列表，不包括返回值。所有
callback的合法性是依据函数签名的。callback的一种方式是functor，这包括2个方式，一个是静态函数，另外一个是重载了
operator()的class，我们统称为functor。另外一种callback实现是类和类的成员函数，这个更加灵活且通用。还有一种方式
就是使用oop的虚函数，比如上面的timer的例子，timer_handler_t用虚函数来实现。在类库级封装上，我不太情愿用虚函数，
一虚阶虚，在声明的时候你就限制了自己。所以在拼凑和虚函数上我使用拼凑来解决问题，这个讨论以后会提到。

如果我们只是对functor或者class member进行封装，这不是很困难。关键的问题是我想实现复杂closure。对于functor，class
instance是必须传入的，class member也是必须的。但是其它的参数怎么传入？就像lambda和bind。在我思考这个问题的时候，
第一反应就是typelist，在参考了tuple的文档和代码后，我相信这是个很好的解决方案。完全是编译时刻的实现。tuple就是个
typelist。

	template <typename _Sig, typename ..._Params>
	struct closure;
	
	template <typename _R, typename ..._Args, typename ..._Params>
	struct closure<_R(_Args...), Params...> {
		tuple<_Params...> _captures;
	};

上面的代码就是我想像中的closure的最初原型。_R和_Args构成了函数签名，_Params则是closure捕获参数列表。这跟g++的
lambda内联类比较类似，不同的是它有语言级支持，可以单独的声明捕获变量，而我的closure只能用tuple。

但是，这里有个问题。我可以用tuple去存储捕获的变量，但是怎么把他们传递给callback函数？我在code stack上找到一个实现，
相当的有趣，我把它修改了一下并命名为tuple_apply放到了这个工程里。它的原型如下：

	template<bool __back = false, typename _F, typename _T, typename ..._Args>
        static inline auto apply(_F && f, _T && t, _Args&&...args);

_F是个functor， _T是个tuple，_Args是调用的参数。这个函数的功能是调用_F，并不tuple中的元素展开到_Args的前面或者后面。
比如：

	int foo(int a, int b, int c, int d);
	tuple<int, int> t(1, 2);
	tuple_apply::apply(foo, t, 3, 4);

实际的调用效果等同于`foo(1, 2, 3, 4)`。tuple_apply的设计核心就是定义一个语法级的递归，然后操纵编译器进行递归。
所有的typelist类似的编程基本都遵循这个方式。这很需要点空间想象能力，而且稍不注意就会把编译器搞得死循环或者
直接挂掉。有些文章把这种编程叫做meta program。

现在我们用tuple解决了参数捕获问题，用tuple_apply解决了参数传递问题。尽管这种方式看上去没有bind那么灵活，但是
在功能上是足够用了。下面是closure的使用例子：

	struct foo {
		void operator()(int &a, int b) {
			return a += b;
		}
		void sub()(int &a, int b) {
			return a -= b;
		}
	};
	
	int main()
	{
		foo f;
		int sum = 0;
		closure<void(int)>::instance<foo, int&> c(f, sum);
		c(1), c(2);
		
		closure<void(int)>::instance<foo, int&> c2(f, &foo::sub, sum);
		c2(1), c2(2);

		auto c3 = closure<void(int)>::make(f, sum);
		c3(1), c3(2);

		auto c4 = closure<void(int)>::make(f, &foo::sub, sum);
		c4(1), c4(2);
	}

在这里我把所有捕获的变量展开在函数调用的列表的前部，这主要是考虑函数参数的默认值和变参等因素。closure实现
的功能与function和bind类似，在编程技巧上远远比不上bind。但是closure很简单，一个closure完成了类似function和
bind的组合功能。最关键的是我希望内存强控，关于了libll++的内存理念在后面会提到，这真传统
c++是完全不同的。呵呵，至少我不是个传统c++程序员，我没有传统。

对于c++封装来说，我觉得我又制造了个轮子。是不值得提倡的，唯一可以安慰的是这个轮子的制造时有目的的。

------------------------------------------------------------------------------------------------------------------------------------------
##2013-9-3 libll++的list实现

对于libll++的基础数据结构的实现，我希望能够秉承以下基本原则：

1. 尽量保持统一的接口


2. 对于某类数据结构，尽量考虑它的各种使用情况，而不是只是提供一个粗粒度的统一实现。


3. 尽量使用宿主去包含管理节点（或者叫数据植入），而不是使用频繁的2次内存分配。


4. 尽可能使使用功能factory去拼凑，而不是用虚函数去规划。对于拼凑和规划的理解后续的blog会提到。


对于数据结构节点的植入，在前面已经提到过，如果某个数据结构曾经属于某个容器，它就具有了这个容器的基本属性。stl的list是申请一个内存作为节点，
被管理对象也需要分配内存，我称之为2次分配，内存分配的效率是很低的。根据结构植入理论，node就该属于数据结构，而不是单独分配。在libll++的基础
数据结构中，这种模式比比皆是。性能是一点点挤出来的，如果在基础层不去注意，后面再努力也是白搭。libll++虽然是个学习库，但是不是给那种浪漫
轻松的编程准备的，它要去挑战c的效率。

在c++中实现数据植入是个挺麻烦的事情。c++模板参数只接受编译时可确认的类型，类型，常量，还有类成员。类成员是实现这种想法的唯一选择，
但是它的描述相当的不方便，要知道成员类型，要知道类，要知道成员名。不过由于多重继承和c++根本没有定义内存布局，我也只能从c++合法语法角度
出发。总之，我比较抱怨模板的类成员参数描述不方便。

	struct foo {
		list_entry _entry;
		clist_entry _centry;
	};
	
	ll_list(foo, _entry) alist;
	ll_list(foo, _centry) blist;

由于libll++大量使用数据植入，为了简化模板参数，每个libll++的基础数据结构都有个宏来简化它。ll_list就是libll++的list声明简化宏。

前面提到了4种list版本，我不想放弃任何一个。在后面的数据结构中，我想复用前面的封装而不是单独再弄一个实现。简单的说，1把“手术刀”不够，
我需要4把，如果某一天我发现一个适合某种情况的手术刀，我就把它变成5把。

我希望一个统一的list描述方式，因为我不知道什么时候会出现第5把手术刀。另外，模板参数的复杂度，如果把这种描述都放到list本身上，会太复杂。
简单的想法是list是个模板，我想定义一个统一的模板参数列表，通过某些参数决定模板偏例来实现不同的list。经过反复考量，我把偏例的决定权交给
entry。list_entry, clist_entry，是2个不同的class。它们都有一个相同的constexpr来描述entry type。entry type来决定list的偏例实现。
就像上面的例子alist和blist是2个不同的链表。这看上去很诡异，但是它工作的很好。

这种模式在继承和多重继承上，都工作的相当好。比如foo2派生于foo并且多重继承其它类，放到alist或者blist里，完全没有问题。这得益于类成员地址
作为模板参数，如果是宏可能就不太行。

在list和其它基础数据结构中，用到了大量的offsetof_member和containerof_member。我们管理的元素是foo，而不是entry。在这种封装中，我感觉
很幸福，在c中我们用无穷无尽的宏去实现这个，而在c++中这一切看上去那么自然。

对于list的实现代码，我不想太具体去描述，篇幅有限。我只是阐述下它的设计初衷。那些代码都很简单，毕竟list也不是什么复杂的东西。



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

前面3种链表的实现均来自于linux的queue.h，clist则是我随手写了一个实现。以前曾经看过stl的代码，它的list好像就是用循环链表写的。
循环链表不难理解，list和entry是同一个数据结构。

	struct entry {
		struct entry *next;
		struct entry *prev;
	};

	typedef struct entry clist;

这样clist的first就是head->next，clist的last就是head->prev。head在初始化的时候，next和prev都指向自己。它可以push/pop front和
push/pop back，效率都很高。可以正向遍历也可以逆向遍历。在插入删除的时候，不需要判断NULL，效率很不错。它也可以高效完成
remove self。总之吧，它能完成list的所有功能。当然它的内存开销也是最大的，head和entry都是2个指针。

在用c开发的过程中，clist是个较少被用到的数据结构。一方面它的开销比较大，比list的head多一个指针。单个看不算什么，如果一个
很大的hash表，用clist就很可观了。clist的性能略低于list，不过几乎可以忽略不计，关键的问题在于clist需要特定的初始化。在c
里是没有构造函数的，如果忘记初始化你就悲剧了。而list的初始化则简单的多，清0就行了。

clist的出现场合往往是既要remove self，又要能够保障时序的正序。

某些特定的场合clist具有压倒性的优势。比如，基于生命周期session管理模式，这多用于网络应用。
比如，我们建立一个系统，这个系统最大能够cache 1000个session。用户下线后，session不是立刻被清除，而是cache起来。如果，用户
一段时间内重新登录，立刻启用cache的session。当然，用户数是要远远大于cache数的。当新用户登录后，cache已经满了，没有session可
分配给新用户，就会释放掉最不活跃的session。这就需要一个基于时序性的session活跃度记录。通常的做法是，用户有行为，就会把session
从cache list中摘除然后push_front到cache list中，为了高效完成这个功能必须要有remove self能力。cache list的back则是最不活跃
session。这需要cache list能够高效的检索tail元素，并且能够高效的remove它。这种场合下，clist就是舍我其谁的唯一选择。

####今天总结

本想今天把list的开发过程写完，没想到只是介绍下以前常用的4种list就用了这么大的篇幅，看来只能下个blog再写完了。

我认为写程序的过程，首先是个数据结构选择的过程。很多类库的封装，隐藏了大量的细节，这是我很长时间以来 不太喜欢c++的一个重要原因。
任何一种结构都会有其所得也必然会有其所失。程序员需要去衡量和推敲每种结构的收益和代价。如果即取得了超高的收益，又避免了较高的
代价，这一般是技术飞跃。很可惜，技术飞跃是很难产生的，至少我还没在自己身上碰到过。








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

