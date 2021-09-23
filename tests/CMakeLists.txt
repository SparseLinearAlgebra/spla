function(spla_test_target target)
    add_executable(${target} ${target}.cpp)
    target_link_libraries(${target} PRIVATE spla)
    target_link_libraries(${target} PRIVATE gtest)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
endfunction()

spla_test_target(TestBasic)
spla_test_target(TestExpression)
spla_test_target(TestDataMatrix)

