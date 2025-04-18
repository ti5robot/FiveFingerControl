# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_hand_angle_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED hand_angle_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(hand_angle_FOUND FALSE)
  elseif(NOT hand_angle_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(hand_angle_FOUND FALSE)
  endif()
  return()
endif()
set(_hand_angle_CONFIG_INCLUDED TRUE)

# output package information
if(NOT hand_angle_FIND_QUIETLY)
  message(STATUS "Found hand_angle: 0.0.0 (${hand_angle_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'hand_angle' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${hand_angle_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(hand_angle_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${hand_angle_DIR}/${_extra}")
endforeach()
