
file(GLOB CURRENT_SOURCES "*.cpp" )
target_sources(database PRIVATE ${CURRENT_SOURCES})

add_subdirectory(Common)
add_subdirectory(Design)
add_subdirectory(Liberty)
add_subdirectory(MCMM)
add_subdirectory(Parastic)
add_subdirectory(SPI)
add_subdirectory(Tech)
add_subdirectory(TimingConstraints)
add_subdirectory(PowerConstraints)
