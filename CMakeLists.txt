cmake_minimum_required(VERSION 3.15)
project(lex VERSION 0.5.2 LANGUAGES C)
include(GNUInstallDirs)
set(CMAKE_C_STANDARD 11)

# Define resource directory and syntax files list
set(RESOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/resources")

set(SYNTAX_FILES
    ${RESOURCE_DIR}/syntax/c.json
    ${RESOURCE_DIR}/syntax/cpp.json
    ${RESOURCE_DIR}/syntax/go.json
    ${RESOURCE_DIR}/syntax/html.json
    ${RESOURCE_DIR}/syntax/java.json
    ${RESOURCE_DIR}/syntax/javascript.json
    ${RESOURCE_DIR}/syntax/json.json
    ${RESOURCE_DIR}/syntax/make.json
    ${RESOURCE_DIR}/syntax/php.json
    ${RESOURCE_DIR}/syntax/python.json
    ${RESOURCE_DIR}/syntax/ruby.json
    ${RESOURCE_DIR}/syntax/rust.json
    ${RESOURCE_DIR}/syntax/shell.json
    ${RESOURCE_DIR}/syntax/sql.json
    ${RESOURCE_DIR}/syntax/typescript.json
    ${RESOURCE_DIR}/syntax/zig.json
    ${RESOURCE_DIR}/syntax/markdown.json
)

# ------------------------------------------------------------------------------
# Bundler Logic (Compiles syntax files into bundle.h)
# ------------------------------------------------------------------------------
set(BUNDLER_SOURCE "${RESOURCE_DIR}/bundler.c")
add_executable(bundler ${BUNDLER_SOURCE})
set(BUNDLER_BIN $<TARGET_FILE:bundler>)
set(BUNDLED_FILE "${RESOURCE_DIR}/bundle.h")

# Custom command to generate bundle.h whenever syntax files change
add_custom_command(
    OUTPUT ${BUNDLED_FILE}
    COMMAND ${BUNDLER_BIN} ${BUNDLED_FILE} ${SYNTAX_FILES}
    DEPENDS bundler ${SYNTAX_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

# ------------------------------------------------------------------------------
# Core Sources
# ------------------------------------------------------------------------------
set(CORE_SOURCES
    src/action.c
    src/action.h
    src/buildnum.c
    src/buildnum.h
    src/config.c
    src/config.h
    src/common.h
    src/editor.c
    src/editor.h
    src/file_io.c
    src/file_io.h
    src/highlight.c
    src/highlight.h
    src/input.c
    src/input.h
    src/json.h
    src/lex.c
    src/opt.h
    src/os.h
    src/output.c
    src/output.h
    src/prompt.c
    src/prompt.h
    src/row.c
    src/row.h
    src/select.c
    src/select.h
    src/terminal.c
    src/terminal.h
    src/unicode.c
    src/unicode.h
    src/utils.c
    src/utils.h
)

# Platform-specific OS files
if (WIN32)
    list(APPEND CORE_SOURCES
        src/os_win32.c
        src/os_win32.h
    )
else()
    list(APPEND CORE_SOURCES
        src/os_unix.c
        src/os_unix.h
    )
endif()

add_executable(${PROJECT_NAME} ${CORE_SOURCES} ${BUNDLED_FILE})

# ------------------------------------------------------------------------------
# Build Number Logic
# ------------------------------------------------------------------------------
# Custom target to update buildnum.c on every build (touch it)
add_custom_target(file_toucher
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/src/buildnum.c
)

add_dependencies(${PROJECT_NAME} file_toucher)

# ------------------------------------------------------------------------------
# Compile Options and Definitions
# ------------------------------------------------------------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE
    EDITOR_NAME="${PROJECT_NAME}"
    EDITOR_VERSION="${CMAKE_PROJECT_VERSION}"
)

# Set compile flags based on compiler (MSVC vs others)
set(COMMON_HEADER "${CMAKE_SOURCE_DIR}/src/common.h")
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4244 /wd4267 /wd4996 /FI "${COMMON_HEADER}")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -include "${COMMON_HEADER}")
endif()

# ------------------------------------------------------------------------------
# Installation
# ------------------------------------------------------------------------------
# Ini adalah target instalasi utama.
# CMake akan mencatat semua file yang diinstal ke install_manifest.txt.
install(TARGETS ${PROJECT_NAME})

# ------------------------------------------------------------------------------
# Additional Targets: UNINSTALL dan PURGE
# ------------------------------------------------------------------------------

# 1. UNINSTALL Target (MENGGANTIKAN uninstall.sh)
# Target ini akan menjalankan skrip CMake yang dihasilkan untuk menghapus file yang
# terdaftar di install_manifest.txt.
# ------------------------------------------------------------------------------
if (NOT TARGET uninstall)
    # 1. Pastikan direktori 'cmake' ada untuk menyimpan template
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake) 
    
    # 2. Definisikan file manifest (log instalasi)
    # Lokasi file ini penting: harus di dalam direktori build (BINARY_DIR)
    SET(CMAKE_INSTALL_MANIFEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")

    # 3. Konfigurasi template skrip uninstalasi
    # Menggunakan template 'uninstall.cmake.in' untuk membuat 'uninstall.cmake' di direktori build.
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
        @ONLY
    )
    
    # 4. Definisikan target 'uninstall'
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
        USES_TERMINAL
        COMMENT "Removing installed files based on install_manifest.txt..."
    )
    message(STATUS "Uninstall target created. Run 'sudo make uninstall' to remove installed files.")
endif()


# 2. PURGE / DEEP CLEAN Target
# Target ini menghapus seluruh konten direktori build. Berguna setelah 'make uninstall'.
# ------------------------------------------------------------------------------
add_custom_target(purge
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Removing all build artifacts and cache from ${CMAKE_CURRENT_BINARY_DIR}"
)
message(STATUS "Purge target created. Run 'make purge' to delete the entire build directory.")