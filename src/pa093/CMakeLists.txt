add_subdirectory(algorithm)

target_sources(
  ${PROJECT_NAME}
  PRIVATE
  app.cpp
  shader_cache.cpp
  main.cpp
  mesh.cpp
)
