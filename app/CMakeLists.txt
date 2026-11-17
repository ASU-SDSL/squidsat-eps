cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(eps)

list(APPEND CMAKE_MODULE_PATH ${ZEPHYR_BASE}/modules/nanopb)
include(nanopb)

target_include_directories(app PRIVATE 
						   ${CMAKE_CURRENT_LIST_DIR}/drivers/include
						   ${CMAKE_CURRENT_LIST_DIR}/utilities/include
						   ${CMAKE_CURRENT_LIST_DIR}/tasks/include
						   ${CMAKE_CURRENT_LIST_DIR}/lib/sdsl-can/sdsl-can-lib/include)

zephyr_nanopb_sources(app proto/eps_link.proto)
target_sources(app PRIVATE
	main.c
	${CMAKE_CURRENT_LIST_DIR}/lib/sdsl-can/sdsl-can-lib/src/can_link.c
	${CMAKE_CURRENT_LIST_DIR}/utilities/src/can_wrapper.c
	${CMAKE_CURRENT_LIST_DIR}/tasks/src/sensor_task.c
	${CMAKE_CURRENT_LIST_DIR}/drivers/src/nka103c1b1.c
	${CMAKE_CURRENT_LIST_DIR}/drivers/src/ntcle101.c
	${CMAKE_CURRENT_LIST_DIR}/utilities/src/set_rail.c
	${CMAKE_CURRENT_LIST_DIR}/drivers/src/ina219.c
	${CMAKE_CURRENT_LIST_DIR}/drivers/src/max17049.c
	${CMAKE_CURRENT_LIST_DIR}/drivers/src/adc.c
	${CMAKE_CURRENT_LIST_DIR}/drivers/src/battery_heater.c
)
