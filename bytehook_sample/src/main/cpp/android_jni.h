//
// Created by chenpeng on 2024/9/9.
//

#ifndef BHOOK_ANDROID_JNI_H
#define BHOOK_ANDROID_JNI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>
#include <jni.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "include/kdvtype.h"

#define GLOBAL_LOADER_CLASS_NAME "java/lang/Class"
/***
 *
 *
 * Class<?> aClass = Class.forName("com.himi.TestBean");
 *
 * java.lang.Class包名
 * public static Class<?> forName(String className) 静态方法
 *   jclass class_loader = (*env)->FindClass(env, "java/lang/Class");
 *   jmethodID loaderMethod = (*env)->GetStaticMethodID(env, class_loader,"forName", "(Ljava/lang/String;)Ljava/lang/Class;");
 *
 *   jstring  java_stack = (jstring)(*env)->CallStaticObjectMethod(env,jcz,jmethodId);
 *
 *
 *
 *
 */
//定义结构体
typedef struct native_context {
  JavaVM *g_vm; //vm对象
  //native查找java类
  jclass gClassLoader;//java类中ClassLoader的对象
  jmethodID gFindClassMethod; //ClassLoader中的Class<?> loadClass(String name)
  //线程锁
  pthread_mutex_t lock;
} NativeContext;

//声明一个外部的 int 类型的全局变量，实际定义在其他地方
extern NativeContext g_ctx;

s16 init_JniClass(JavaVM *vm);

JNIEnv *getJniEnv(void);

jclass global_find_class(const char *className);

char *jstringToChar(JNIEnv *env,jstring jstr);

#ifdef __cplusplus
}
#endif

/*****
 * 1、extern关键字的作用
 *
 *
 */

#endif  // BHOOK_ANDROID_JNI_H
