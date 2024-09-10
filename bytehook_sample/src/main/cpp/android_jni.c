//
// Created by chenpeng on 2024/9/9.
//
#include "android_jni.h"

NativeContext g_ctx;

s16 init_JniClass(JavaVM *vm) {
  memset(&g_ctx, 0, sizeof(g_ctx));
  g_ctx.g_vm = vm;
  JNIEnv *env = getJniEnv();
  jclass jcz = (*env)->FindClass(env, GLOBAL_LOADER_CLASS_NAME);  //
  if (jcz == NULL) {
    return JNI_ERR;
  }
  g_ctx.gClassLoader = jcz;
  g_ctx.gFindClassMethod =
      (*env)->GetStaticMethodID(env, jcz, "forName", "(Ljava/lang/String;)Ljava/lang/Class;");
  if (g_ctx.gFindClassMethod == NULL) {
    return JNI_ERR;
  }
  return JNI_OK;
}

JNIEnv *getJniEnv(void) {
  JNIEnv *env;
  int status = (*g_ctx.g_vm)->GetEnv(g_ctx.g_vm, (void **)&env, JNI_VERSION_1_6);
  if (status < 0) {
    return NULL;
  }
  return env;
}

 jclass global_find_class(const char *className) {
  JNIEnv *env = getJniEnv();
  jstring j_name = (*env)->NewStringUTF(env, className);

  jclass ret =
      (jclass)((*env)->CallStaticObjectMethod(env, g_ctx.gClassLoader, g_ctx.gFindClassMethod, j_name));
  (*env)->DeleteLocalRef(env, j_name);
  return ret;

}


/*
 * 1、jstring、char的互相转化
 * 2、strcpy、strcat、strdup的意思
 */
 char *jstringToChar(JNIEnv *env,jstring jstr){
  if (jstr ==NULL){

    return NULL;
  }

  jboolean isCopy = JNI_FALSE;
  const char *str = (*env)->GetStringUTFChars(env,jstr,&isCopy);
  char  *ret = strdup(str);
  (*env)->ReleaseStringUTFChars(env,jstr,str);
  return ret;
}

