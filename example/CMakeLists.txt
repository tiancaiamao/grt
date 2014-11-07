add_executable(helloworld helloworld.c)
target_link_libraries(helloworld grt)

add_executable(echoserver echoserver.c)
target_link_libraries(echoserver grt)
add_executable(echoclient echoclient.c)
target_link_libraries(echoclient grt)

add_executable(chan chan.c)
target_link_libraries(chan grt)