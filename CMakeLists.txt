cmake_minimum_required(VERSION 2.8.12)

IF(DEFINED ENV{VITASDK})
   include("$ENV{VITASDK}/share/vita.toolchain.cmake" REQUIRED)
   include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
   set(VITA_APP_NAME "Abuse")
   set(VITA_TITLEID  "ABUS00001")
   set(VITA_VERSION  "01.00")
   set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
ELSE()
   message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
ENDIF()

project(abuse C CXX)
set(abuse_VERSION 0.9)
set(abuse_VERSION_MAJOR 0)
set(abuse_VERSION_MINOR 9)
set(abuse_VERSION_PATCH 0)
set(abuse_VERSION_TWEAK 0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(SDL2 2.0.0 REQUIRED)
find_package(SDL2_mixer 2.0.0 REQUIRED)

include(CheckIncludeFiles)

check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files("sys/time.h" HAVE_SYS_TIME_H)
check_include_files("sys/ioctl.h" HAVE_SYS_IOCTL_H)
check_include_files("netinet/in.h" HAVE_NETINET_IN_H)
check_include_files(bstring.h HAVE_BSTRING_H)

set(HAVE_NETWORK FALSE CACHE BOOL "Enable networking support")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(ASSETDIR "app0:/data/")

add_definitions(-DASSETDIR="app0:/data/")

add_definitions(-DPACKAGE_NAME="abuse")
add_definitions(-DPACKAGE_VERSION="${abuse_VERSION}")
add_definitions(-DHAVE_CONFIG_H)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)
