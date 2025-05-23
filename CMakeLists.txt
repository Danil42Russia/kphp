cmake_minimum_required(VERSION 3.16)
project(kphp
        VERSION 1.0.1
        LANGUAGES C CXX ASM
        DESCRIPTION "Compiler for PHP (aka KPHP)"
        HOMEPAGE_URL https://github.com/VKCOM/kphp)

if ("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
    message(FATAL_ERROR [[
        In-source build is forbidden! Run following command:
        rm -rf CMakeFiles CMakeCache.txt && cmake -H. -Bbuild
    ]])
endif()

# include(CMakeGraphVizOptions)
# set(GRAPHVIZ_CUSTOM_TARGETS TRUE)

include(CMakePrintHelpers)
include(CheckCXXCompilerFlag)
include(AddFileDependencies)
include(FetchContent)
include(GNUInstallDirs)
include(ExternalProject)

# Global includes must be before all other includes/add_subdirectories
include(cmake/utils.cmake)
include(cmake/init-global-vars.cmake)
include(cmake/init-compilation-options.cmake)
include(cmake/external-libraries.cmake)
include(cmake/init-compilation-flags.cmake)
include(cmake/popular-common.cmake)

# Common third-parties build
include(${THIRD_PARTY_DIR}/zlib-cmake/zlib.cmake)
include(${THIRD_PARTY_DIR}/openssl-cmake/openssl.cmake)
include(${THIRD_PARTY_DIR}/zstd-cmake/zstd.cmake)
include(${COMMON_DIR}/unicode/unicode.cmake)
# RE2 depends on unicode
include(${THIRD_PARTY_DIR}/re2-cmake/re2.cmake)
include(${THIRD_PARTY_DIR}/yaml-cpp-cmake/yaml-cpp.cmake)

# TODO: use FetchContent_Declare instead of include_directories
include_directories(.)
include_directories(${INCLUDE_DIR})

# Custom modules
include(${BASE_DIR}/flex/flex.cmake)
include(${BASE_DIR}/net/net.cmake)
include(${BASE_DIR}/vkext/vkext.cmake)

include(${COMMON_DIR}/binlog/binlog.cmake)
include(${COMMON_DIR}/common.cmake)
include(${COMMON_DIR}/tl/tl.cmake)

include(${RUNTIME_COMMON_DIR}/runtime-common.cmake)

if(COMPILE_RUNTIME_LIGHT)
    include(${BASE_DIR}/runtime-light/runtime-light.cmake)
    file(GLOB EXCLUDE_FILES "runtime/*")
else()
    include(${BASE_DIR}/runtime/runtime.cmake)
    include(${BASE_DIR}/server/server.cmake)
    file(GLOB EXCLUDE_FILES "runtime-light/*")
endif()

include(${BASE_DIR}/compiler/compiler.cmake)
include(${BASE_DIR}/tests/tests.cmake)


if(KPHP_CUSTOM_CMAKE AND EXISTS ${OBJS_DIR}/custom_php_project.cmake)
    include(${OBJS_DIR}/custom_php_project.cmake)
endif()

add_custom_target(kphp ALL DEPENDS ${OBJS_DIR}/php_lib_version.sha256)

if (COMPILE_RUNTIME_LIGHT)
    add_dependencies(kphp kphp2cpp kphp-light-runtime-pic)
else ()
    add_dependencies(kphp kphp2cpp kphp-full-runtime-no-pic kphp-full-runtime-pic)
endif ()

install(TARGETS kphp2cpp
        COMPONENT KPHP
        RUNTIME DESTINATION ${VK_INSTALL_DIR}/bin/)
install_symlink(${VK_INSTALL_DIR}/bin/kphp2cpp ${CMAKE_INSTALL_PREFIX}/bin/kphp KPHP)

# k2 specific
if(COMPILE_RUNTIME_LIGHT)
    install(FILES ${OBJS_DIR}/libk2kphp-rt.a
            COMPONENT KPHP
            DESTINATION ${INSTALL_KPHP_SOURCE}/objs)

    install(DIRECTORY ${COMMON_DIR}
                      ${RUNTIME_COMMON_DIR}
                      ${RUNTIME_LIGHT_DIR}
            COMPONENT KPHP
            DESTINATION ${INSTALL_KPHP_SOURCE}
            FILES_MATCHING REGEX ".*\\.(hpp|h|inl)$"
    )

    set(CPACK_DEBIAN_KPHP_PACKAGE_DEPENDS "clang-18, libc++1-18, libc++abi1-18")
    set(CPACK_DEBIAN_KPHP_PACKAGE_RECOMMENDS "vk-tl-tools")
    set(CPACK_DEBIAN_KPHP_DESCRIPTION "kphp compiler and asynchronous runtime")
    set(CPACK_DEBIAN_KPHP_PACKAGE_NAME "k2kphp")
else() # kphp specific
    install(FILES ${OBJS_DIR}/libkphp-full-runtime-no-pic.a
            COMPONENT KPHP
            DESTINATION ${INSTALL_KPHP_SOURCE}/objs)

    install(FILES ${OBJS_DIR}/libkphp-full-runtime-pic.a
            COMPONENT KPHP
            DESTINATION ${INSTALL_KPHP_SOURCE}/objs)

    install(DIRECTORY ${COMMON_DIR}
                      ${BASE_DIR}/runtime
                      ${RUNTIME_COMMON_DIR}
                      ${BASE_DIR}/server
                      ${BASE_DIR}/third-party
            COMPONENT KPHP
            DESTINATION ${INSTALL_KPHP_SOURCE}
            FILES_MATCHING REGEX ".*\\.(hpp|h|inl)$")

    set(CPACK_DEBIAN_KPHP_PACKAGE_DEPENDS "vk-flex-data, gcc, g++")

    if (PDO_DRIVER_MYSQL)
        set(CPACK_DEBIAN_KPHP_PACKAGE_DEPENDS "${CPACK_DEBIAN_KPHP_PACKAGE_DEPENDS}, libmysqlclient-dev")
    endif()

    if (PDO_DRIVER_PGSQL)
        set(CPACK_DEBIAN_KPHP_PACKAGE_DEPENDS "${CPACK_DEBIAN_KPHP_PACKAGE_DEPENDS}, libpq-dev")
    endif()

    set(CPACK_DEBIAN_KPHP_PACKAGE_RECOMMENDS "vk-tl-tools")
    set(CPACK_DEBIAN_KPHP_DESCRIPTION "kphp2cpp compiler and runtime for it")
    set(CPACK_DEBIAN_KPHP_PACKAGE_NAME "kphp")
endif()

# common
install(DIRECTORY ${OBJS_DIR}/include
        COMPONENT KPHP
        DESTINATION ${INSTALL_KPHP_SOURCE}/objs)

install(DIRECTORY ${OBJS_DIR}/lib
        COMPONENT KPHP
        DESTINATION ${INSTALL_KPHP_SOURCE}/objs)

install(FILES ${OBJS_DIR}/php_lib_version.sha256
        COMPONENT KPHP
        DESTINATION ${INSTALL_KPHP_SOURCE}/objs)

install(DIRECTORY ${BASE_DIR}/builtin-functions
        COMPONENT KPHP
        DESTINATION ${INSTALL_KPHP_SOURCE}/)

install(FILES ${COMMON_DIR}/php-functions.h
        COMPONENT KPHP
        DESTINATION ${INSTALL_KPHP_SOURCE}/)

install(FILES ${AUTO_DIR}/runtime/runtime-headers.h
        COMPONENT KPHP
        DESTINATION ${INSTALL_KPHP_SOURCE}/objs/generated/auto/runtime/)

add_custom_target(force_clean_all
                  COMMAND rm -rf ${OBJS_DIR}
                  COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/*)

include(CPack)
