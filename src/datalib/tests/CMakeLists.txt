find_package(Qt5 5.12 REQUIRED COMPONENTS Test Core Network)

enable_testing()

set(TEST_SOURCES
    tst_datamanager.cpp
    tst_dynamicdata.cpp
)

foreach(test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common/logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common/config.cpp
    )
    target_link_libraries(${test_name} PRIVATE
        cpss_data
        Qt5::Test
        Qt5::Core
        Qt5::Network
    )
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../common
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
