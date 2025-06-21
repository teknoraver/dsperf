cmake_minimum_required(VERSION 3.10)
project(dperf VERSION 0.0.35 LANGUAGES C CXX)

# Opzione per abilitare/disabilitare libdaas
option(WITH_DAAS "Enable support for libdaas" OFF)
message(STATUS "WITH_DAAS = ${WITH_DAAS}")

# Standard C/C++
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

# Directory dei sorgenti
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(INC_DIR ${LIB_DIR}/include)
set(SYSTEM_INFO_DIR ${SRC_DIR}/system_info_wrapper)
set(LOOPBACK_DIR ${SRC_DIR})

# Generazione dinamica dell'header di versione
configure_file(
    ${SRC_DIR}/version.h.in
    ${SRC_DIR}/version.h
    @ONLY
)

# File sorgente principali
set(SOURCES
    ${LOOPBACK_DIR}/main.cpp
    ${SYSTEM_INFO_DIR}/system_info.c
    ${LOOPBACK_DIR}/test_models/model_bandwidth.cpp
    ${LOOPBACK_DIR}/timer.c
    ${LOOPBACK_DIR}/dperf_local.cpp
)

## Header comuni
set(HEADERS
    ${LOOPBACK_DIR}/test_models/model_bandwidth.h
    ${LOOPBACK_DIR}/timer.h
    ${LOOPBACK_DIR}/version.h
    ${LOOPBACK_DIR}/dperf_local.hpp
)

# Aggiunge facoltativamente header daas **solo se esistono**
if(WITH_DAAS)
    if(EXISTS "${INC_DIR}/daas.hpp" AND EXISTS "${INC_DIR}/daas_types.hpp")
        list(APPEND HEADERS
            ${INC_DIR}/daas.hpp
            ${INC_DIR}/daas_types.hpp
        )
    else()
        message(FATAL_ERROR "WITH_DAAS è abilitato, ma i file daas.hpp o daas_types.hpp non esistono in ${INC_DIR}")
    endif()
endif()

# Creazione eseguibile
add_executable(dperf
    ${SOURCES}
    ${HEADERS}
)

# Librerie di sistema
target_link_libraries(
    dperf
    PRIVATE pthread
    PRIVATE bluetooth
)

# Se DAAS è abilitato, aggiungi include, define e link
if(WITH_DAAS)
    message(STATUS "Compiling WITH libdaas")
    target_compile_definitions(dperf PRIVATE "WITH_DAAS")
    target_include_directories(dperf PRIVATE ${INC_DIR})
    target_link_libraries(dperf PRIVATE ${LIB_DIR}/libdaas.a)
else()
    message(STATUS "Compiling WITHOUT libdaas")
endif()

# Include directory comuni
target_include_directories(dperf PRIVATE
    ${SYSTEM_INFO_DIR}
    ${LOOPBACK_DIR}
)
