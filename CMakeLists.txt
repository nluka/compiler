cmake_minimum_required(VERSION 3.19)
project(CompilerGUI LANGUAGES CXX)

find_package(Qt5 5.12 REQUIRED COMPONENTS Core Widgets)


# qt_standard_project_setup()

# ----- Qt 5 replacement for qt_standard_project_setup() -----

# Auto processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Folder organization in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# (Optional) MSVC: match Qt 6 warning levels
if (MSVC)
    add_compile_options(/W4)
endif()

# (Optional) Clang/GCC: common warnings but not too strict
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra)
endif()


file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.cxx"
    "src/*.hpp"
    "src/*.h"
    # "src/*.ui"
    # "${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc"   # optionally
)
add_executable(CompilerGUI ${SOURCES}
)
set_property(
    TARGET CompilerGUI PROPERTY VS_DEBUGGER_COMMAND $<TARGET_FILE:CompilerGUI>
)
set_target_properties(CompilerGUI PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)
target_link_libraries(CompilerGUI
    PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)


include(GNUInstallDirs)

install(TARGETS CompilerGUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# qt_generate_deploy_app_script(
#     TARGET CompilerGUI
#     OUTPUT_SCRIPT deploy_script
#     NO_UNSUPPORTED_PLATFORM_ERROR
# )
# install(SCRIPT ${deploy_script})
if (WIN32)
    install(CODE "
        execute_process(
            COMMAND \"${QT_ROOT}/bin/windeployqt.exe\" \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/CompilerGUI.exe\"
        )
    ")
endif()
