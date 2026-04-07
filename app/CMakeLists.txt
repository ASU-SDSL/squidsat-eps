cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(eps)

list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)

target_include_directories(app PRIVATE include)

zephyr_nanopb_sources(app proto/eps_link.proto)
target_sources(app PRIVATE
	src/main.c
	src/can_link.c
)
