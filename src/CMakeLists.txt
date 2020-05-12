
add_library(PF_Test STATIC main.cpp PF_Test/UnitTest.hpp)
target_include_directories(PF_Test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (MSVC)
    target_compile_options(PF_Test PRIVATE /W4 /permissive-)
else()
    target_compile_options(PF_Test PRIVATE -Wall -Wextra)
endif()

if (${PFTEST_MEMORY_LEAK})
    target_compile_definitions(PF_Test PRIVATE "PFTEST_MEMORY_LEAK_DETECTION")
endif()
