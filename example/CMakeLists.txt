add_executable(helloworld helloworld.c)
target_link_libraries(helloworld 9 thread)

add_executable(echotime echotime.c)
target_link_libraries(echotime 9 thread)

add_executable(chan chan.c)
target_link_libraries(chan 9 thread)