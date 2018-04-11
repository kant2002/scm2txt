include_directories("${scm2txt_SOURCE_DIR}/dependencies/StormLib/src/")

find_package(Boost 1.68.0)
if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
	add_definitions(-DUSE_BOOST_STREAMS)
	message("Use Boost iostreams")
else()
	message("Use STL streams")
endif()

include_directories("../include")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")

add_library(libscm
	mapreader.cpp
	tilesetreader.cpp
	generated/tileset.cpp)

target_link_libraries(libscm storm)

include_directories("../include")
add_executable(scm2txt
    main.cpp)

target_link_libraries(scm2txt libscm)

add_executable(gen_tileset
    gen_tileset_main.cpp)

target_link_libraries(gen_tileset libscm)