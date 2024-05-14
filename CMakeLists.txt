cmake_minimum_required(VERSION 3.6.2)

project(AlphaMin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# include(cmake/utils.cmake)

set(CMAKE_CXX_STANDARD 11)

include_directories(./alpha)

file(GLOB SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/alpha/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/alpha/http/*.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/alpha/db/*.cc
)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
add_library(alpha SHARED ${SRC})

target_include_directories(alpha PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(MySQL REQUIRED)
if(MYSQL_FOUND)
    include_directories(${MYSQL_INCLUDE_DIR})
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

find_library(PTHREAD pthread)

set(
    LIB_LIB
    pthread
    yaml-cpp
    dl
    mysqlclient
    ${OPENSSL_LIBRARIES}
)

# add_executable(test_thread tests/test_thread.cc ${SRC})
# target_link_libraries(test_thread ${LIB_LIB})

# add_executable(test_log tests/test_log.cc ${SRC})
# target_link_libraries(test_log ${LIB_LIB})

# add_executable(test_fiber tests/test_fiber.cc ${SRC})
# target_link_libraries(test_fiber ${LIB_LIB})

# add_executable(test_scheduler tests/test_scheduler.cc ${SRC})
# target_link_libraries(test_scheduler ${LIB_LIB})

# add_executable(test_iomanager tests/test_iomanager.cc ${SRC})
# target_link_libraries(test_iomanager ${LIB_LIB})

# add_executable(test_hook tests/test_hook.cc ${SRC})
# target_link_libraries(test_hook ${LIB_LIB})

# add_executable(test_address tests/test_address.cc ${SRC})
# target_link_libraries(test_address ${LIB_LIB})

# add_executable(test_socket tests/test_socket.cc ${SRC})
# target_link_libraries(test_socket ${LIB_LIB})

# add_executable(test_bytearray tests/test_bytearray.cc ${SRC})
# target_link_libraries(test_bytearray ${LIB_LIB})

# add_executable(test_boost tests/test_boost.cc ${SRC})
# target_link_libraries(test_boost ${LIB_LIB})

# add_executable(test_tcp_server tests/test_tcp_server.cc ${SRC})
# target_link_libraries(test_tcp_server ${LIB_LIB})

# add_executable(test_http tests/test_http.cc ${SRC})
# target_link_libraries(test_http ${LIB_LIB})

# add_executable(test_http_parser tests/test_http_parser.cc ${SRC})
# target_link_libraries(test_http_parser ${LIB_LIB})

# add_executable(echo_server examples/echo_server.cc ${SRC})
# target_link_libraries(echo_server ${LIB_LIB})

# add_executable(test_http_server tests/test_http_server.cc ${SRC})
# target_link_libraries(test_http_server ${LIB_LIB})

# add_executable(test_http_connection tests/test_http_connection.cc ${SRC})
# target_link_libraries(test_http_connection ${LIB_LIB})

# add_executable(test_mysql tests/test_mysql.cc ${SRC})
# target_link_libraries(test_mysql ${LIB_LIB})

# add_executable(test_multiplexer tests/test_multiplexer.cc ${SRC})
# target_link_libraries(test_multiplexer ${LIB_LIB})

# add_executable(test_fiber_sync tests/test_fiber_sync.cc ${SRC})
# target_link_libraries(test_fiber_sync ${LIB_LIB})

# add_executable(test_fiber_cond tests/test_fiber_cond.cc ${SRC})
# target_link_libraries(test_fiber_cond ${LIB_LIB})

# add_executable(test_chan tests/test_chan.cc ${SRC})
# target_link_libraries(test_chan ${LIB_LIB})

add_executable(test_pool tests/test_pool.cc ${SRC})
target_link_libraries(test_pool ${LIB_LIB})

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
SET(CMAKE_BUILD_TYPE "Debug")  # 定义编译类型
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g2 -ggdb") # 定义Debug编译参数
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") # 定义Release编译参数