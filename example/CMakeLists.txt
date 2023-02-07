include_directories(..)

add_executable(test1 test1.c)
target_link_libraries(test1 too)

add_executable(toohttp toohttp.c)
target_link_libraries(toohttp too)



