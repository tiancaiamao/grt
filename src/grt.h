/* Copyright (c) 2005 Russ Cox, MIT; see COPYRIGHT */

#ifndef _TASK_H_
#define _TASK_H_ 1

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdarg.h>
#include <inttypes.h>
#include <sys/types.h>
    
    /*
     * basic procs and threads
     */
    
    typedef struct G *g;
    
    int		anyready(void);
    int		taskcreate(void (*f)(void *arg), void *arg, unsigned int stacksize);
    void		taskexit(int);
    void		taskexitall(int);
    void		taskmain(int argc, char *argv[]);
    int		taskyield(void);
    void**		taskdata(void);
    void		needstack(int);
    void		taskname(char*, ...);
    void		taskstate(char*, ...);
    char*		taskgetname(void);
    char*		taskgetstate(void);
    void		tasksystem(void);
    unsigned int	taskdelay(unsigned int);
    unsigned int	taskid(void);
    
	int maxprocs(int);
    
//    void	tasksleep(Rendez*);
//    int	taskwakeup(Rendez*);
//    int	taskwakeupall(Rendez*);
    
    /*
     * channel communication
     */
    typedef struct Alt Alt;
    typedef struct Altarray Altarray;
    typedef struct C Channel;
    
    enum
    {
        CHANEND,
        CHANSND,
        CHANRCV,
        CHANNOP,
        CHANNOBLK,
    };
    
    struct Alt
    {
        Channel		*c;
        void		*v;
        unsigned int	op;
        struct G		*task;
        Alt		*xalt;
    };
    
    struct Altarray
    {
        Alt		**a;
        unsigned int	n;
        unsigned int	m;
    };
    
    struct C
    {
        unsigned int	bufsize;
        unsigned int	elemsize;
        unsigned char	*buf;
        unsigned int	nbuf;
        unsigned int	off;
        Altarray	asend;
        Altarray	arecv;
        char		*name;
    };
    
    int		chanalt(Alt *alts);
    Channel*	chancreate(int elemsize, int elemcnt);
    void		chanfree(Channel *c);
    int		chaninit(Channel *c, int elemsize, int elemcnt);
    int		channbrecv(Channel *c, void *v);
    void*		channbrecvp(Channel *c);
    unsigned long	channbrecvul(Channel *c);
    int		channbsend(Channel *c, void *v);
    int		channbsendp(Channel *c, void *v);
    int		channbsendul(Channel *c, unsigned long v);
    int		chanrecv(Channel *c, void *v);
    void*		chanrecvp(Channel *c);
    unsigned long	chanrecvul(Channel *c);
    int		chansend(Channel *c, void *v);
    int		chansendp(Channel *c, void *v);
    int		chansendul(Channel *c, unsigned long v);
    
    /*
     * Threaded I/O.
     */
    ssize_t		fdread(int, void*, int);
//    int		fdread1(int, void*, int);	/* always uses fdwait */
    ssize_t		fdwrite(int, void*, int);
    void		fdwait(int, int);
    int		fdnoblock(int);
    
    /*
     * Network dialing - sets non-blocking automatically
     */
    enum
    {
        UDP = 0,
        TCP = 1,
    };
    
    int		netannounce(int, char*, int);
    int		netaccept(int, char*, int*);
    int		netdial(int, char*, int);
    int		netlookup(char*, uint32_t*);	/* blocks entire program! */
    
#ifdef __cplusplus
}
#endif
#endif

