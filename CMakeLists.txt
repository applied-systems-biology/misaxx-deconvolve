cmake_minimum_required(VERSION 3.11) # Or higher if required
project(misaxx-deconvolve VERSION 1.0.0 DESCRIPTION "Deconvolution example")

find_package(misaxx-core REQUIRED)
find_package(misaxx-imaging REQUIRED)
# Add additional packages if necessary

add_library(misaxx-deconvolve include/misaxx-deconvolve/module_interface.h
                include/misaxx-deconvolve/module.h
                src/misaxx-deconvolve/module_interface.cpp
                src/misaxx-deconvolve/module.cpp
                src/misaxx-deconvolve/module_info.cpp src/misaxx-deconvolve/algorithms/convolve_task.cpp src/misaxx-deconvolve/algorithms/convolve_task.h src/misaxx-deconvolve/algorithms/deconvolve_task.cpp src/misaxx-deconvolve/algorithms/deconvolve_task.h)

# Add additional link targets if necessary
target_link_libraries(misaxx-deconvolve misaxx::misaxx-core misaxx::misaxx-imaging)

# MISA++ helper script (automatically included by Core Library)
set(MISAXX_LIBRARY misaxx-deconvolve)
set(MISAXX_LIBRARY_NAMESPACE misaxx_deconvolve::)
set(MISAXX_API_NAME misaxx_deconvolve)
set(MISAXX_API_INCLUDE_PATH misaxx-deconvolve)
set(MISAXX_API_NAMESPACE misaxx_deconvolve)
misaxx_with_default_module_info()
misaxx_with_default_api()

# Only if it's a worker module:
misaxx_with_default_executable()
