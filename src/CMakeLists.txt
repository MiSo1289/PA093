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
  CONAN_PKG::glm
  CONAN_PKG::glpp
  CONAN_PKG::fmt
  CONAN_PKG::range-v3
  CONAN_PKG::spdlog
)

add_subdirectory(pa093)
