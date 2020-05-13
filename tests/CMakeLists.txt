add_executable(PF_Test_UnitTests main.cpp)
target_link_libraries(PF_Test_UnitTests PF_Test)
target_compile_options(PF_Test_UnitTests PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>)
