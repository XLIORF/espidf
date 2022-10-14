# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/wlx/program/espressif/esp-idf/components/bootloader/subproject"
  "/home/wlx/learn/espidf_examples/EspLcd/build/bootloader"
  "/home/wlx/learn/espidf_examples/EspLcd/build/bootloader-prefix"
  "/home/wlx/learn/espidf_examples/EspLcd/build/bootloader-prefix/tmp"
  "/home/wlx/learn/espidf_examples/EspLcd/build/bootloader-prefix/src/bootloader-stamp"
  "/home/wlx/learn/espidf_examples/EspLcd/build/bootloader-prefix/src"
  "/home/wlx/learn/espidf_examples/EspLcd/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/wlx/learn/espidf_examples/EspLcd/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
