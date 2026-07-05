cmake_minimum_required(VERSION 3.16)

project(stackchan_firmware_tests LANGUAGES CXX)

enable_testing()

add_executable(motion_math_test
    motion_math_test.cpp
    ../main/stackchan/motion/motion_math.cpp
)

target_include_directories(motion_math_test PRIVATE
    ../main
)

target_compile_features(motion_math_test PRIVATE cxx_std_17)

add_test(NAME motion_math_test COMMAND $<TARGET_FILE:motion_math_test>)

add_executable(full_sprite_pack_test
    full_sprite_pack_test.cpp
    ../main/stackchan/avatar/skins/full_sprite/full_sprite_pack.cpp
)

target_include_directories(full_sprite_pack_test PRIVATE
    ../main
)

target_compile_features(full_sprite_pack_test PRIVATE cxx_std_17)

add_test(NAME full_sprite_pack_test COMMAND $<TARGET_FILE:full_sprite_pack_test>)
