cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(eps)

list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)

#add_subdirectory(/home/aidan/squidsat-eps-1/modules/hal/stm32 app/build_externals)

target_include_directories(app PRIVATE include)
#target_link_libraries(app PRIVATE stm32cube)
#target_include_directories(app PRIVATE /home/aidan/squidsat-eps-1/modules/hal/stm32/stm32cube/stm32f1xx/drivers/include)
#file(GLOB HAL_SOURCES
#	/home/aidan/squidsat-eps-1/modules/hal/stm32/stm32cube/stm32f1xx/drivers/src/*.c
#)

#list(FILTER HAL_SOURCES EXCLUDE REGEX ".*template\\.c$")

zephyr_nanopb_sources(app proto/eps_link.proto)
target_sources(app PRIVATE
	src/main.c
	src/can_link.c
	src/INA219.c
	#${HAL_SOURCES}
)
