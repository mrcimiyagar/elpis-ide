
cmake_minimum_required(VERSION 3.5.1)
project(EogenVirtualMachine)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/home/keyhan/globe_libs/boost_1_70_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/home/keyhan/globe_libs/boost_1_70_0/libs")

SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/home/keyhan/projects/c++/EogenVirtualMachine/include/bdwgc/.libs")

find_package(Boost)
IF (Boost_FOUND)
    message("-- Found boost")
    include_directories(${Boost_INCLUDE_DIR})
else()
    message("-- Boost not found")
endif()

include_directories(/home/keyhan/globe_libs/boost_1_70_0)
include_directories(/home/keyhan/globe_libs/mpir-3.0.0)
include_directories(/home/keyhan/projects/c++/EogenVirtualMachine/api)
include_directories(/home/keyhan/projects/c++/EogenVirtualMachine/include)
include_directories(/usr/include/postgresql)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/public")

add_executable(
        EogenVirtualMachine
        web/driver/main.c
        web/runner/Kasper.c
        web/runner/Kasper.h
        web/structures/dictionary/Dictionary.h
        web/structures/dictionary/Dictionary.c
        web/models/Models.h
        web/structures/list/List.h
        web/structures/list/List.c
        web/structures/stack/Stack.h
        web/structures/stack/Stack.c
        web/api/IO/ConsolePrinter.c
        web/structures/array/array.c
        web/structures/array/array.h
        web/api/Cipher/Sha256.c
        web/api/Cipher/Sha256.h
        web/api/String/String.c
        web/api/String/String.h
        web/api/IO/HttpServer.c
        web/api/IO/HttpServer.h
        web/utils/GarbageCenter.c
        web/utils/GarbageCenter.h web/utils/json.c web/utils/json.h)

target_compile_options(EogenVirtualMachine PRIVATE -lmpir -lgmp -lm -lssl -lcrypto -libuuid -libpq -lpthread)

target_link_libraries(EogenVirtualMachine gmp m ssl crypto uuid pq pthread)