# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.26.4)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(ctags-populate NONE)



include(ExternalProject)
ExternalProject_Add(ctags-populate
                     "UPDATE_DISCONNECTED" "False" "DOWNLOAD_EXTRACT_TIMESTAMP" "TRUE" "PREFIX" "C:/Users/geoma/.Arduino_Core_STM32_dl/f785d07b28d650ae4167b867299f09fc" "URL" "https://github.com/arduino/ctags/releases/download/5.8-arduino11/ctags-5.8-arduino11-i686-mingw32.zip" "URL_HASH" "SHA512=580e1e82aa1ca2b2810cd04f7a6a0d12e4211abf66f57f845bdd946b92919e713d50e7adde3b1a131f046d9cb1642eb33a13507a64d3d4746a4e7c380c15d9a4" "UPDATE_DISCONNECTED"
                    SOURCE_DIR          "C:/Users/geoma/.Arduino_Core_STM32_dl/f785d07b28d650ae4167b867299f09fc/dist/ctags"
                    BINARY_DIR          "D:/Arduino/cmake/1_Blink/build/_deps/ctags-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)


