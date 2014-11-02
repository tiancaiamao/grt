ENABLE_LANGUAGE (ASM)

# os dependent
SET(SYSOFILES pthread.c Darwin-x86_64-asm.s Darwin-x86_64-swapcontext.c)
SET(OFILES
	${SYSOFILES}
	channel.c
	daemonize.c
	exec.c
	ioproc.c
	iorw.c
	ref.c
	thread.c
	wait.c)

SET(HFILES thread.h threadimpl.h)

ADD_LIBRARY(thread STATIC ${OFILES})
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)