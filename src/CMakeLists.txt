add_library(PF_Test STATIC main.cpp PF_Test/UnitTest.hpp)
target_include_directories(PF_Test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(PF_Test PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>)

if (${PFTEST_MEMORY_LEAK})
    target_compile_definitions(PF_Test PRIVATE "PFTEST_MEMORY_LEAK_DETECTION")
endif()
