add_subdirectory(algorithm)
add_subdirectory(render)

target_sources(
  ${PROJECT_NAME}
  PRIVATE
  app.cpp
  main.cpp
)
