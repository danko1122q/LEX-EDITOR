cmake_minimum_required(VERSION 3.15)
# Ganti project name jika ingin rebranding dari 'lex' ke yang lain
project(lex VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)

set(CMAKE_C_STANDARD 11)

# -------------------------------------------------------------------
# Bundler Logic
# -------------------------------------------------------------------
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

set(BUNDLER_SOURCE "${RESOURCE_DIR}/bundler.c")
add_executable(bundler ${BUNDLER_SOURCE})
set(BUNDLED_FILE "${RESOURCE_DIR}/bundle.h")

add_custom_command(
    OUTPUT ${BUNDLED_FILE}
    COMMAND $<TARGET_FILE:bundler> ${BUNDLED_FILE} ${SYNTAX_FILES}
    DEPENDS bundler ${SYNTAX_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating bundle.h from syntax files..."
    BYPRODUCTS ${BUNDLED_FILE}
    VERBATIM
)

add_custom_target(generate_bundle ALL DEPENDS ${BUNDLED_FILE})

# -------------------------------------------------------------------
# Core Sources (Updated to core_ prefix)
# -------------------------------------------------------------------
set(CORE_SOURCES
    src/core_action.c src/core_action.h
    src/core_buildnum.c src/core_buildnum.h
    src/core_config.c src/core_config.h
    src/core_common.h
    src/core_editor.c src/core_editor.h
    src/core_file_io.c src/core_file_io.h
    src/core_highlight.c src/core_highlight.h
    src/core_input.c src/core_input.h
    src/core_json.h
    src/core_main.c
    src/core_opt.h
    src/core_os.h
    src/core_output.c src/core_output.h
    src/core_prompt.c src/core_prompt.h
    src/core_row.c src/core_row.h
    src/core_select.c src/core_select.h
    src/core_terminal.c src/core_terminal.h
    src/core_unicode.c src/core_unicode.h
    src/core_utils.c src/core_utils.h
)

# Logic for OS specific files
if (WIN32)
    list(APPEND CORE_SOURCES src/core_win.c src/core_win.h)
else()
    list(APPEND CORE_SOURCES src/core_nix.c src/core_nix.h)
endif()

add_executable(${PROJECT_NAME} ${CORE_SOURCES} ${BUNDLED_FILE})
add_dependencies(${PROJECT_NAME} generate_bundle)

# -------------------------------------------------------------------
# Build Number Logic
# -------------------------------------------------------------------
add_custom_target(file_toucher
    COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR}/src/core_buildnum.c
)
add_dependencies(${PROJECT_NAME} file_toucher)

# -------------------------------------------------------------------
# Compile Options and Definitions
# -------------------------------------------------------------------
target_compile_definitions(${PROJECT_NAME} PRIVATE
    EDITOR_NAME="${PROJECT_NAME}"
    EDITOR_VERSION="${CMAKE_PROJECT_VERSION}"
)

# Header guard otomatis menggunakan core_common.h
set(COMMON_HEADER "${CMAKE_SOURCE_DIR}/src/core_common.h")
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /wd4244 /wd4267 /wd4996 /FI "${COMMON_HEADER}")
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -include "${COMMON_HEADER}")
endif()

# -------------------------------------------------------------------
# Installation Rules
# -------------------------------------------------------------------
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT runtime
)

# -------------------------------------------------------------------
# Custom Targets (Uninstall, Force-Uninstall, etc.)
# -------------------------------------------------------------------
if(NOT TARGET uninstall)
    set(CMAKE_INSTALL_MANIFEST_FILE "${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
        @ONLY
    )
    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
        USES_TERMINAL
        COMMENT "Removing installed files..."
    )
endif()

add_custom_target(force-uninstall
    COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_INSTALL_FULL_BINDIR}/${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E remove -f "/usr/local/bin/${PROJECT_NAME}"
    COMMAND ${CMAKE_COMMAND} -E remove -f "/usr/bin/${PROJECT_NAME}"
    COMMENT "Force uninstall complete."
)

add_custom_target(check-install
    COMMAND ${CMAKE_COMMAND} -E echo "Checking installation for ${PROJECT_NAME}..."
    COMMAND ${CMAKE_COMMAND} -E env bash -c "command -v ${PROJECT_NAME} || echo 'Not found in PATH'"
)
