add_executable(accumulate accumulate.cpp)
add_executable(custom_builder custom_builder.cpp)
add_executable(double_arg double_arg.cpp)

target_link_libraries(accumulate PRIVATE argparser)
target_include_directories(accumulate PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(custom_builder PRIVATE argparser)
target_include_directories(custom_builder PUBLIC ${PROJECT_SOURCE_DIR})

target_link_libraries(double_arg PRIVATE argparser)
target_include_directories(double_arg PUBLIC ${PROJECT_SOURCE_DIR})
