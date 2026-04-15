
file(GLOB CURRENT_SOURCES "*.cpp" )
target_sources(database PRIVATE ${CURRENT_SOURCES})

add_subdirectory(Design)
add_subdirectory(Liberty)
add_subdirectory(Physical)

