cmake_minimum_required ( VERSION 3.2 )
project ( GpO VERSION 0.0.1)
set( GPO_VERSION_ABI 1 )

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC ")

if( "${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}" )
 message( FATAL "no in source building allowed." )
endif( )

list( APPEND CMAKE_MODULE_PATH
 ${PROJECT_SOURCE_DIR}/CMake
)

set( GPO_DESCRIPTION "GpO Base Project" )
set( GPO_LICENSE LGPL )

find_package(OpenGL REQUIRED)

add_subdirectory(libs/glfw EXCLUDE_FROM_ALL)
add_subdirectory(libs/glm )
add_subdirectory(libs/glad )

include_directories(libs/glad/include)
include_directories(libs/glm)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/KHR)
include_directories(${PROJECT_SOURCE_DIR}/include/stb)

set(GPO_SOURCE_FILES include/stb/stb_image.h
					 include/GpO.h
					 include/GpOObjectLoader.hpp
					 include/GpOObjectLoader.cpp
					 src/GPO_project_stippling.cpp
					 src/GPO_aux.cpp)
add_executable(gpo_project_stippling ${GPO_SOURCE_FILES})
target_link_libraries(gpo_project_stippling PRIVATE ${OPENGL_LIBRARIES} glfw glm glad_interface)

set(GPO_SOURCE_FILES include/stb/stb_image.h
					 include/GpO.h
					 include/GpOObjectLoader.hpp
					 include/GpOObjectLoader.cpp
					 src/GPO_04_entrega_ejer3.cpp
					 src/GPO_aux.cpp)
add_executable(gpo_04_entrega_ejer3 ${GPO_SOURCE_FILES})
target_link_libraries(gpo_04_entrega_ejer3 PRIVATE ${OPENGL_LIBRARIES} glfw glm glad_interface)

set(GPO_SOURCE_FILES include/stb/stb_image.h
					 include/GpO.h
					 include/GpOObjectLoader.hpp
					 include/GpOObjectLoader.cpp
					 src/GPO_project_ink_technique_NPR.cpp
					 src/GPO_aux.cpp)
add_executable(GPO_project_ink_technique_NPR ${GPO_SOURCE_FILES})
target_link_libraries(GPO_project_ink_technique_NPR PRIVATE ${OPENGL_LIBRARIES} glfw glm glad_interface)

set(GPO_SOURCE_FILES include/stb/stb_image.h
					 include/GpO.h
					 include/GpOObjectLoader.hpp
					 include/GpOObjectLoader.cpp
					 src/GPO_project_inkwash.cpp
					 src/GPO_aux.cpp)
add_executable(GPO_project_inkwash ${GPO_SOURCE_FILES})
target_link_libraries(GPO_project_inkwash PRIVATE ${OPENGL_LIBRARIES} glfw glm glad_interface)

set_target_properties(gpo_project_stippling
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(gpo_04_entrega_ejer3
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin") 
set_target_properties(GPO_project_ink_technique_NPR
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set_target_properties(GPO_project_inkwash
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
file(COPY data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin)

file(GLOB MY_CONTENT "data/*")
file(COPY ${MY_CONTENT} DESTINATION "${CMAKE_BINARY_DIR}/bin/data")
