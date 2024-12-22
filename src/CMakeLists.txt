# Print debug output function
macro(print_debug_list list_name)
    message(STATUS "${list_name}:")
    foreach(item ${${list_name}})
        message(STATUS "  ${item}")
    endforeach()
endmacro()

# Automatically find all .cpp and .c files in src/ and subdirectories
FILE(GLOB_RECURSE component_srcs
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

# Automatically find all header directories
FILE(GLOB_RECURSE component_headers
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# Exclude files in the examples directory
list(FILTER component_srcs EXCLUDE REGEX "^examples/")
list(FILTER component_headers EXCLUDE REGEX "^examples/")

# Debugging: Print source and header files
print_debug_list("component_srcs")
print_debug_list("component_headers")

# Extract directories containing headers
set(component_includes "")
foreach(header ${component_headers})
    get_filename_component(dir ${header} PATH)
    list(APPEND component_includes ${dir})
endforeach()

# Remove duplicate directories from the include paths
list(REMOVE_DUPLICATES component_includes)

# Debugging: Print include directories
print_debug_list("component_includes")

# Register the component
idf_component_register(
    SRCS ${component_srcs}
    INCLUDE_DIRS ${component_includes}
)
