#include <signal.h>
#include <pthread.h>
#include "grt.h"
/*
 * OS X before 10.5 (Leopard) does not provide
 * swapcontext nor makecontext, so we have to use our own.
 * In theory, Leopard does provide them, but when we use
 * them, they seg fault.  Maybe we're using them wrong.
 * So just use our own versions, even on Leopard.
 */
#if defined(__APPLE__)
#	define mcontext libthread_mcontext
#	define mcontext_t libthread_mcontext_t
#	define ucontext libthread_ucontext
#	define ucontext_t libthread_ucontext_t
#	if defined(__i386__)
#		include "386-ucontext.h"
#	elif defined(__x86_64__)
#		include "amd64-ucontext.h"
#	else
#		include "power-ucontext.h"
#	endif
#endif

typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char uchar;
enum {
	STACK = 8192
};
typedef struct Context Context;
struct Context {
	ucontext_t	uc;
};

struct Cond {
    pthread_mutex_t	l;
    pthread_cond_t	cond;
    int		asleep;
};

struct G {
	char	name[256];
	char	state[256];
	struct M	*m;
	struct G	*next;
	struct G	*prev;
	struct G	*allnext;
	struct G	*allprev;
	Context	context;
	unsigned long long	alarmtime;
	uint	id;
	uchar	*stk;
	uint	stksize;
	int	exiting;
	int	alltaskslot;
	int	system;
	int	ready;
	void	(*startfn)(void*);
	void	*startarg;
	void	*udata;
};

struct Glist {
	struct G	*head;
	struct G	*tail;
};

struct M {
	struct M		*next;
	struct M		*prev;
	char		msg[128];
	pthread_t		osprocid;
	pthread_mutex_t		lock;
	int			nswitch;
	struct G		*g;
	struct G	*pinthread;
	struct Glist	runqueue;
	struct Glist	idlequeue;
	struct Glist	allg;
    uint		numg;
	uint		sysproc;

    struct Cond cond;
	
	Context	schedcontext;
	void		*udata;
	// Jmp		sigjmp;
	// int		mainproc;
};

struct Sched {
	pthread_mutex_t threadnproclock;
	uint threadnproc;
    uint threadnsysproc;
	
	pthread_mutex_t threadprocslock;
	struct M* threadprocs;
	struct M* threadprocstail;
}_sched;

void _threadinit(void);
void _threadbindm(struct M* m);
struct M* _thread();
void _threadcreate(void (*fn)(void*), void *arg, uint stack);
struct M* _threadalloc(void);
void _delthread(struct M *m);
void _sysmon();
void _threadsleep(struct Cond*);
void _threadwakeup(struct M*);

void _scheduler(struct M *m);
struct G* _taskcreate(struct M* m, void (*fn)(void*), void *arg, uint stack);
void _taskready(struct G *g);
void _needstack(int n);
void _taskswitch(void);
void _deltask(struct Glist *l, struct G *g);
void _addtask(struct Glist *l, struct G *g);

void _contextswitch(Context *from, Context *to);

void fdtask(struct M*);
