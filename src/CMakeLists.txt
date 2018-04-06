include_directories("${scm2txt_SOURCE_DIR}/dependencies/StormLib/src/")

add_executable(scm2txt
    main.cpp)

target_link_libraries(scm2txt storm)