# Search all header and source files
file(GLOB srcs "*.cpp")
file(GLOB hdrs "*.h")

set(dart_math_hdrs ${hdrs} PARENT_SCOPE)
set(dart_math_srcs ${srcs} PARENT_SCOPE)

# Library
#dart_add_library(dart_math ${srcs} ${hdrs})
#target_link_libraries(dart_math ${DART_CORE_DEPENDENCIES})

# Generate header for this namespace
dart_get_filename_components(header_names "math headers" ${hdrs})
dart_generate_include_header_list(
  math_headers
  "dart/math/"
  "math headers"
  ${header_names}
)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/math.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/math.h
)

# Install
install(
  FILES ${hdrs} ${CMAKE_CURRENT_BINARY_DIR}/math.h
  DESTINATION include/dart/math
  COMPONENT headers
)
#install(TARGETS dart_math EXPORT DARTCoreTargets DESTINATION lib)
#install(TARGETS dart_math EXPORT DARTTargets DESTINATION lib)

