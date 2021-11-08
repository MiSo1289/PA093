add_subdirectory(algorithm)
add_subdirectory(datastructure)
add_subdirectory(render)
add_subdirectory(visualization)

target_sources(
  ${PROJECT_NAME}
  PRIVATE
  app.cpp
  main.cpp
)
