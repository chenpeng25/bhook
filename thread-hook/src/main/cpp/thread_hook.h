//
// Created by chenpeng on 2024/9/20.
//

#ifndef BHOOK_THREAD_HOOK_H
#define BHOOK_THREAD_HOOK_H

#include <cstddef>
#include <jni.h>

struct thread_holder{
  void *(*start_routine)(void *); //函数指针，表示线程的入口函数
  void *start_routing_arg;        //线程入口函数的参数
  size_t count;
  thread_holder(){}               //默认构造函数
  thread_holder(size_t c):count(c){} //带参数的构造函数
};

JNIEnv *AttachEnv(JavaVM* vm){
  JNIEnv *env = nullptr;
  int status = vm->GetEnv((void **)env,JNI_VERSION_1_6);
  if (status == JNI_EDETACHED || env == nullptr){
    status = vm->AttachCurrentThread(&env, nullptr);
    if (status < 0) {
      env = nullptr;
    }
  }

}

#endif  // BHOOK_THREAD_HOOK_H
