ENABLE_LANGUAGE (ASM)
IF(CMAKE_SYSTEM_NAME MATCHES "Darwin")
	SET(POLL fd_kqueue.c)
ENDIF()

IF(CMAKE_SYSTEM_NAME MATCHES "Linux")
	SET(POLL fd_epoll.c)
ENDIF()

SET(SRC asm.s context.c task.c thread.c sched.c channel.c fd.c fd_kqueue.c net.c test.c)
ADD_LIBRARY(grt STATIC ${SRC})
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
