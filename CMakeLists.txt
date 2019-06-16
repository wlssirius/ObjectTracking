cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(ObjectTracking)

add_executable (ObjectTracking main.cpp target.h target.cpp tracking.h tracking.cpp)

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
  target_include_directories(ObjectTracking PUBLIC
    ${OpenCV_INCLUDE_DIRS})
  target_link_libraries (ObjectTracking ${OpenCV_LIBS})

  get_target_property(__dll_dbg opencv_world IMPORTED_LOCATION_DEBUG) 
  get_target_property(__dll_release opencv_world  IMPORTED_LOCATION_RELEASE)

  add_custom_command(TARGET ObjectTracking POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different           
    "$<$<CONFIG:debug>:${__dll_dbg}>$<$<CONFIG:release>:${__dll_release}>"      
    $<TARGET_FILE_DIR:ObjectTracking>)
endif()