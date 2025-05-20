cmake_minimum_required(VERSION 3.0.0)
project(usbcd VERSION 0.1.0 LANGUAGES C)

include(CTest)
enable_testing()

add_executable(usbcd init.c main.c daemon.c opm.c)

# Find the libudev library
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUDEV REQUIRED libudev)

# Include the libudev headers
include_directories(${LIBUDEV_INCLUDE_DIRS})

# Link the libudev library to your executable
target_link_libraries(usbcd ${LIBUDEV_LIBRARIES})

# Find the GLib library
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)

# Include the GLib headers
include_directories(${GLIB_INCLUDE_DIRS})

# Link the GLib library to your executable
target_link_libraries(usbcd ${GLIB_LIBRARIES})

# Include the current directory for header files
include_directories(${CMAKE_SOURCE_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(NOTIFY REQUIRED libnotify)

include_directories(${NOTIFY_INCLUDE_DIRS})
link_directories(${NOTIFY_LIBRARY_DIRS})

add_definitions(${NOTIFY_CFLAGS_OTHER})

target_link_libraries(usbcd ${NOTIFY_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
