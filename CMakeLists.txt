cmake_minimum_required(VERSION 3.10)
project(vocoder-tui VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(NCURSESW REQUIRED ncursesw)
pkg_check_modules(FFTW3F REQUIRED fftw3f)

include_directories(
    ${ALSA_INCLUDE_DIRS}
    ${NCURSESW_INCLUDE_DIRS}
    ${FFTW3F_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

set(SOURCES
    src/main.cpp
    src/audio/alsa.cpp
    src/dsp/fft.cpp
    src/dsp/pitchshift.cpp
    src/ui/tui.cpp
    src/utils/logger.cpp
)

add_executable(vocoder-tui ${SOURCES})

target_link_libraries(vocoder-tui
    ${ALSA_LIBRARIES}
    ${NCURSESW_LIBRARIES}
    ${FFTW3F_LIBRARIES}
    m
    pthread
)

install(TARGETS vocoder-tui DESTINATION bin)
