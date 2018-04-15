set (CMAKE_CXX_STANDARD 11)
set(Boost_USE_STATIC_LIBS   ON)

find_package(Boost 1.62.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

link_directories(${Boost_LIBRARY_DIRS})
add_definitions(-DNOMINMAX)

file(GLOB BOOST_TEST_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *Test.cpp)
foreach(testSrc ${BOOST_TEST_SRCS})
	#Extract the filename without an extension (NAME_WE)
    get_filename_component(testName ${testSrc} NAME_WE)

    #Add compile target
	add_definitions(-DWITH_CMAKE)
	add_definitions(-DTEST_PATH=${CMAKE_CURRENT_SOURCE_DIR})

    add_executable(${testName}
		${testSrc})
	target_include_directories(${testName} BEFORE PUBLIC
		"../include/"
	)

    #link to Boost libraries AND your targets and dependencies
    #target_link_libraries(${testName} ${Boost_LIBRARIES} [YOUR TARGETS/LIBRARIES])
	target_link_libraries(${testName} PUBLIC libscm)

    #I like to move testing binaries into a testBin directory
    #set_target_properties(${testName} PROPERTIES 
    #    RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/testBin)

    #Finally add it to test execution - 
    #Notice the WORKING_DIRECTORY and COMMAND
    #add_test(NAME ${testName} COMMAND ${testName})
	add_test(NAME ${testName} 
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
        COMMAND ${testName})

endforeach(testSrc)

file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})