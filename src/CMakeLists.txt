find_package(fmt REQUIRED)
find_package(glm REQUIRED)
find_package(glpp REQUIRED)
find_package(range-v3 REQUIRED)
find_package(spdlog REQUIRED)

add_executable(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
  fmt::fmt
  glm::glm
  glpp::glpp
  range-v3::range-v3
  spdlog::spdlog
)

add_subdirectory(pa093)
