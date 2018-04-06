include_directories("${scm2txt_SOURCE_DIR}/dependencies/StormLib/src/")

find_package(Boost 1.62.0)
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)

add_executable(scm2txt
    main.cpp)

target_link_libraries(scm2txt storm)