# Generated Cmake Pico project file

#CMakeの最小バージョン指定など
cmake_minimum_required(VERSION 3.13)

#言語の規格指定などなど
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

#触るなって意味↓
# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# ボードの種類を指定
set(PICO_BOARD pico2 CACHE STRING "Board type")

# Raspberry Pi Pico SDKのインポート
include(pico_sdk_import.cmake)

# プロジェクトの設定
project(RP2354_effect_program C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

#どのファイルをコンパイル対象にするか指定
add_executable(RP2354_effect_program RP2354_effect_program.c )

# プログラム名とバージョンの設定
pico_set_program_name(RP2354_effect_program "RP2354_effect_program")
pico_set_program_version(RP2354_effect_program "0.1")

# 標準入出力の設定
pico_enable_stdio_uart(RP2354_effect_program 0)
pico_enable_stdio_usb(RP2354_effect_program 1)

# 標準ライブラリのリンク
target_link_libraries(RP2354_effect_program
        pico_stdlib)

# 標準インクルードファイルの追加
target_include_directories(RP2354_effect_program PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# 追加の出力ファイルの生成
pico_add_extra_outputs(RP2354_effect_program)

