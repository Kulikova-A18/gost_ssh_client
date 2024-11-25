cmake_minimum_required(VERSION 3.10)

project(SSHClient)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBSSH REQUIRED libssh)

set(CMAKE_C_STANDARD 11)

add_executable(ssh_client ssh_client.c)

target_include_directories(ssh_client PRIVATE ${GLIB_INCLUDE_DIRS} ${LIBSSH_INCLUDE_DIRS})
target_link_libraries(ssh_client PRIVATE ${GLIB_LIBRARIES} ${LIBSSH_LIBRARIES})

set_target_properties(ssh_client PROPERTIES
    CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GLIB_CFLAGS} ${LIBSSH_CFLAGS}"
)
