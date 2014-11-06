add_executable(helloworld helloworld.c)
target_link_libraries(helloworld grt)

add_executable(echoserver echoserver.c)
target_link_libraries(echoserver grt)

add_executable(chan chan.c)
target_link_libraries(chan grt)