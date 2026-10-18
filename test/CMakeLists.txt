find_package(GTest REQUIRED)
include(GoogleTest)

# Single test executable; test/main.cc provides the only main. Each file keeps
# its own fixture so TEST_F state is local, while the process is shared, so
# tests must clean up their global ObjectTable/Design/Liberty objects.
set(DB_UT_SOURCES
    main.cc
    baseplatform/string_mgr_test.cc
    baseplatform/object_table_test.cc
    baseplatform/id_array_mgr_test.cc
    design/design_name_test.cc
    design/inst_create_test.cc
    design/path_name_test.cc
    design/name_index_test.cc
    design/navigation_api_test.cc
    design/connectivity_test.cc
    design/eco_test.cc
    design/persistence_roundtrip_test.cc
    liberty/liberty_ports_test.cc
)

add_executable(db_ut ${DB_UT_SOURCES})
target_link_libraries(db_ut PRIVATE database GTest::gtest)
# libdatabase carries pre-existing unresolved HNet symbols; none are used here.
target_link_options(db_ut PRIVATE "LINKER:--allow-shlib-undefined")
gtest_discover_tests(db_ut)
