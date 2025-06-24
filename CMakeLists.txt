cmake_minimum_required(VERSION 3.5)
project(audio_analyzer)

set(CMAKE_CXX_STANDARD 17)
find_package(PkgConfig REQUIRED)

# locate FFTW3
pkg_check_modules(FFTW3 REQUIRED fftw3)
# locate aubio
pkg_check_modules(AUBIO REQUIRED aubio)

# your headers (crow_all.h, json.hpp) live in include/
include_directories(
  ${FFTW3_INCLUDE_DIRS}
  ${AUBIO_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/include
)

link_directories(
  ${FFTW3_LIBRARY_DIRS}
  ${AUBIO_LIBRARY_DIRS}
)

add_executable(audio_analyzer
  src/main.cpp
  # plus any other .cpp files
)

target_include_directories(audio_analyzer PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${FFTW3_INCLUDE_DIRS}
  ${AUBIO_INCLUDE_DIRS}
)

# link in fftw3, aubio, pthread (aubio needs it), math
target_link_libraries(audio_analyzer
  ${FFTW3_LIBRARIES}
  ${AUBIO_LIBRARIES}
  pthread
  m
)

enable_testing()

add_executable(test_fft_bpm
  tests/test_fft_bpm.cpp
)

target_include_directories(test_fft_bpm PRIVATE
  ${CMAKE_SOURCE_DIR}/include
  ${FFTW3_INCLUDE_DIRS}
  ${AUBIO_INCLUDE_DIRS}
)

target_link_libraries(test_fft_bpm
  ${FFTW3_LIBRARIES}
  ${AUBIO_LIBRARIES}
  pthread
  m
)

add_test(NAME FFT_BPM COMMAND test_fft_bpm)
