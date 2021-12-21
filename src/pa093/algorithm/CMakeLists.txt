add_subdirectory(convex_hull)
add_subdirectory(kd_tree)
add_subdirectory(triangulation)

target_sources(
  ${PROJECT_NAME}
  PRIVATE
  constants.cpp
  geometric_functions.cpp
  utility.cpp
)
