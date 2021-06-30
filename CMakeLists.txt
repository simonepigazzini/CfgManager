cmake_minimum_required(VERSION 3.10)

# set the project name
project(CfgManager VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the actual CfgManager library
add_library(CfgManager SHARED src/CfgManager.cc)
                       
set_target_properties(CfgManager PROPERTIES VERSION  ${PROJECT_VERSION})

target_include_directories(CfgManager PRIVATE interface)

include(GNUInstallDirs)

install(TARGETS CfgManager
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Scikit-Build does not add your site-packages to the search path automatically,
# so we need to add it _or_ the pybind11 specific directory here.
execute_process( COMMAND
                 python -c
                 "import pybind11; print(pybind11.get_cmake_dir())"
                 OUTPUT_VARIABLE _tmp_dir
                 OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT )
list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")

find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(cfgmanager MODULE src/python_bindings.cc)
target_include_directories(cfgmanager PRIVATE interface)
target_compile_definitions(cfgmanager PRIVATE VERSION_INFO=${PROJECT_VERSION})
target_link_libraries(cfgmanager PRIVATE CfgManager)

install(TARGETS cfgmanager DESTINATION .)
install(TARGETS CfgManager DESTINATION lib)
