find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(sprint0_gui
    main.cpp
)

target_link_libraries(sprint0_gui
    PRIVATE Qt6::Widgets
)

add_executable(sprint0_tests
    calc.cpp
    calc_test.cpp
)

target_link_libraries(sprint0_tests
    PRIVATE
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(sprint0_tests)