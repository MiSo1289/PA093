add_subdirectory(algorithm)

target_sources(
    ${PROJECT_NAME}
    PRIVATE
    app.cpp
    main.cpp
    point_drawer.cpp
)
