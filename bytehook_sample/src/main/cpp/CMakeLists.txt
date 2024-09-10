cmake_minimum_required(VERSION 3.22.1)
project(sample)

find_package(bytehook REQUIRED CONFIG)

# libhacker.so
add_library(hacker SHARED hacker.c)
target_compile_features(hacker PUBLIC c_std_17)
target_compile_options(hacker PUBLIC -std=c17 -Weverything -Werror)
target_link_libraries(hacker bytehook::bytehook log)


# libhookee.so

# 声明库并包含源文件
add_library(hookee SHARED hookee.c android_jni.c)

# 设置编译标准和选项
target_compile_features(hookee PUBLIC c_std_17)
target_compile_options(hookee PUBLIC -std=c17 -Weverything -Werror)
# 包含头文件目录
target_include_directories(hookee PUBLIC . include)
target_link_libraries(hookee log)