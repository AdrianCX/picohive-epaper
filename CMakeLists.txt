cmake_minimum_required(VERSION 3.12)

include(${CMAKE_CURRENT_LIST_DIR}/os/pico_os.cmake)

pico_os_pre_project()

project(epaper C CXX ASM)

pico_os_init()

if((NOT PICO_OS_PLATFORM STREQUAL "linux") AND (NOT TARGET hardware_powman))
    message(FATAL_ERROR
        "epaper needs a board with powman support (rp2350), e.g. -DPICO_BOARD=pico2_w")
endif()

set(APP_SOURCES
  src/main.cpp
  src/calendar_manager.cpp
  src/epaper_config.cpp
  )

set(APP_LIBRARIES
  pico_epaper
  pico_common
  pico_ical
  pico_sleep
  )

if(NOT PICO_OS_PLATFORM STREQUAL "linux")
  list(APPEND APP_LIBRARIES hardware_spi hardware_powman)
endif()

function(epaper_add_variant TARGET VARIANT PAGE)
    add_executable(${TARGET} ${APP_SOURCES})

    pico_os_web_page(${TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/src/web/${PAGE})

    pico_os_set_stack_size(${TARGET} 16384)

    target_compile_definitions(${TARGET} PRIVATE
      PICO_STDIO_STACK_BUFFER_SIZE=1024
      PICO_MALLOC_PANIC=1
      EPAPER_VARIANT=${VARIANT}
      CONFIG_MAX_JSON=4096
      )

    target_include_directories(${TARGET} PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${CMAKE_CURRENT_SOURCE_DIR}/src/config
      )

    target_link_libraries(${TARGET} ${APP_LIBRARIES})

    pico_commands_add(${TARGET}
      ${CMAKE_CURRENT_SOURCE_DIR}/src/commands
      ${PICO_BUILTIN_COMMANDS}
      ${PICO_SIMPLE_CONFIG_COMMANDS}
      )

    pico_add_extra_outputs(${TARGET})

    pico_enable_stdio_usb(${TARGET} 0)
    pico_enable_stdio_uart(${TARGET} 0)
endfunction()

epaper_add_variant(epaper_color 1 color.yaml)
epaper_add_variant(epaper_rb    2 redblack.yaml)
