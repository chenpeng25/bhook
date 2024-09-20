//
// Created by chenpeng on 2024/9/19.
//

#ifndef BHOOK_STR_UTIL_H
#define BHOOK_STR_UTIL_H

#include <jni.h>
#include <cstring>

jbyteArray c_2_jbyteArray(JNIEnv *env,char* cstr){
  if (nullptr == cstr || nullptr == env){
    return nullptr;
  }
  size_t len = strlen(cstr);
  jbyteArray ba = env->NewByteArray(len);
  env->SetByteArrayRegion(ba, 0, len, reinterpret_cast<const jbyte *>(cstr));
  return ba;
}

#endif  // BHOOK_STR_UTIL_H
