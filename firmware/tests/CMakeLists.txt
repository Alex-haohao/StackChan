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

add_executable(image_avatar_mapping_test
    image_avatar_mapping_test.cpp
    ../main/stackchan/avatar/skins/image/image_avatar_mapping.cpp
)

target_include_directories(image_avatar_mapping_test PRIVATE
    ../main
)

target_compile_features(image_avatar_mapping_test PRIVATE cxx_std_17)

add_test(NAME image_avatar_mapping_test COMMAND $<TARGET_FILE:image_avatar_mapping_test>)
