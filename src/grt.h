/* Copyright (c) 2005 Russ Cox, MIT; see COPYRIGHT */

#ifndef _TASK_H_
#define _TASK_H_ 1

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdarg.h>
#include <inttypes.h>
#include <sys/types.h>
#include <pthread.h>
    
    typedef struct G *g;
    
    int		anyready(void);
    int		grtcreate(void (*f)(void *arg), void *arg, unsigned int stacksize);
    void		grtexit(int);
    void		grtexitall(int);
    void		grtmain(int argc, char *argv[]);
    int		grtyield(void);
    void**		grtdata(void);
    void		needstack(int);
    void		grtname(char*, ...);
    void		grtstate(char*, ...);
    char*		grtgetname(void);
    char*		grtgetstate(void);
    void		grtsystem(void);
    unsigned int	grtdelay(unsigned int);
    unsigned int	grtid(void);
    
	int maxprocs(int);
    
//    void	grtsleep(Rendez*);
//    int	grtwakeup(Rendez*);
//    int	grtwakeupall(Rendez*);
    
    /*
     * channel communication
     */
    typedef struct Alt Alt;
    typedef struct Altarray Altarray;
    typedef struct C Channel;
    
    enum {
        CHANEND,
        CHANSND,
        CHANRCV,
        CHANNOP,
        CHANNOBLK,
    };
    
    struct Alt {
        Channel		*c;
        void		*v;
        unsigned int	op;
        struct G		*grt;
        Alt		*xalt;
    };
    
    struct Altarray {
        Alt		**a;
        unsigned int	n;
        unsigned int	m;
    };
    
    struct C {
        unsigned int	bufsize;
        unsigned int	elemsize;
        unsigned char	*buf;
        unsigned int	nbuf;
        unsigned int	off;
        Altarray	asend;
        Altarray	arecv;
        char		*name;
		pthread_mutex_t mutex; 
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

