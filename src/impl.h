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

enum {
	G_RUNNING,
	G_READY,
	G_BLOCK,
	G_DEAD,
	G_MAX
};

struct G {
	char	name[128];
	char	state[256];
	char status;	// G_XXX
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

	int	allgrtslot;
	int	system;

	void	(*startfn)(void*);
	void	*startarg;
	void	*udata;
};

struct Glist {
	struct G	*head;
	struct G	*tail;
	int length;
};

struct M {
	struct M		*next;
	struct M		*prev;
	char		msg[128];
	pthread_t		osprocid;
	pthread_mutex_t		lock;
	int			nswitch;
	int nsleep;
	struct G		*g;	// G_RUNNING
	struct Glist	runqueue; // G_READY
	struct Glist	idlequeue; // G_BLOCK
	struct Glist	allg;
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
void _threadstart(struct M *m, void (*fn)(struct M*));
struct M* _threadalloc(void);
void _delthread(struct M *m);
void _sysmon();
void _threadsleep(struct Cond*);
void _threadwakeup(struct M*);

void _scheduler(struct M *m);
struct G* _grtcreate(struct M* m, void (*fn)(void*), void *arg, uint stack);
void _grtready(struct G *g);
void _grtblock(struct G *g);
void _needstack(int n);
void _grtstate(char*, ...);
void _grtswitch(void);
void _delgrt(struct Glist *l, struct G *g);
void _addgrt(struct Glist *l, struct G *g);

void _contextswitch(Context *from, Context *to);

void fdgrt(struct M*);
