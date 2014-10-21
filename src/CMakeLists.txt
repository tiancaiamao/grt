ENABLE_LANGUAGE (ASM)
SET(SRC asm.s context.c task.c thread.c main.c channel.c fd.c net.c test.c)
ADD_LIBRARY(grt STATIC ${SRC})
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)