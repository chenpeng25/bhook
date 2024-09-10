#include <android/log.h>
#include <fcntl.h>
#include <jni.h>
#include <stdlib.h>
#include <unistd.h>

#include "android_jni.h"

#define HOOKEE_JNI_VERSION    JNI_VERSION_1_6
#define HOOKEE_JNI_CLASS_NAME "com/bytedance/android/bytehook/sample/NativeHookee"
#define HOOKEE_TAG            "bytehook_tag"

// 这部分代码忽略了编译器关于可变参数宏（variadic macro）的警告
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(fmt, ...)  __android_log_print(ANDROID_LOG_INFO, HOOKEE_TAG, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, HOOKEE_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, HOOKEE_TAG, fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop



/*#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-function"
static void hookee_get_stack(JNIEnv *env, jobject thiz) {

}
#pragma clang diagnostic pop*/

static void printFunStack(void){
  JNIEnv *env = getJniEnv();
  jclass jcz = (*env)->FindClass(env, HOOKEE_JNI_CLASS_NAME);  //
  if (jcz == NULL) {
    LOGE("Native registration unable to find class '%s'", HOOKEE_JNI_CLASS_NAME);
    return ;
  }
  jmethodID methodId = (*env)->GetStaticMethodID(env, jcz, "getStack","()Ljava/lang/String;");
  if (methodId == NULL) {
    LOGE("class unable to find  '%s'", "getStack");
    return ;
  }
  jstring  java_stack = (jstring) (*env)->CallStaticObjectMethod(env,jcz,methodId);
  char* stack = jstringToChar(env,java_stack);
  LOGI("stack:%s",stack);

  free(stack);
  (*env)->DeleteLocalRef(env,java_stack);
}


#pragma clang optimize off
static void hookee_test(JNIEnv *env, jobject thiz) {
  (void)env, (void)thiz;

  LOG("libhookee.so PRE open()");
  int fd = open("/dev/null", O_RDWR);
  printFunStack();
  if (fd >= 0) close(fd);
  LOG("libhookee.so POST open()");
}
#pragma clang optimize on






/*
 * 动态注册
 * 定义函数映射表
 * 参数1：Java 方法名
 * 参数2：方法描述符，也就是签名
 * 参数3：C++定义对应 Java native方法的函数名
 *
 *
 * Java 类型	符号
    boolean	Z
    byte	B
    char	C
    short	S
    int	    I
    long	L
    float	F
    double	D
    void	V
    objects对象	Lfully-qualified-class-name;L类名
        有种特殊情况包含内部类：{"AddTitle",
 "(ILcom/tp/nexb/CodecWrapper$VidTitleInfo;)I",(void*)jni_AddTitle}, Arrays数组	[array-type [数组类型
    methods方法	(argument-types)return-type(参数类型)返回类型

*/

#define NELEM(x) ((int)(sizeof(x) / sizeof((x)[0])))
static JNINativeMethod gMethods[] = {
    {"nativeTest", "()V", (void *)hookee_test},

};

// static 限制变量或函数的作用域，使其仅在定义它的文件内可见
static int registerNativeMethods(JNIEnv *env, const char *className, const JNINativeMethod *methods,
                                 int numMethods) {
  LOGI(":::registerNativeMethods:::className is %s\n", className);
  jclass clz = (*env)->FindClass(env, className);
  if (clz == NULL) {
    LOGE("Native registration unable to find class '%s'\n", className);
    return JNI_ERR;
  }

  if ((*env)->RegisterNatives(env, clz, methods, numMethods) < 0) {
    LOGE("Register natives failed for '%s'\n", className);
    return JNI_ERR;
  }

  (*env)->DeleteLocalRef(env, clz);
  return JNI_OK;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  (void)reserved;

  if (NULL == vm) {
    LOGE("JNI_OnLoad vm is null ");
    return JNI_ERR;
  }



  JNIEnv *env = NULL;
  if (JNI_OK != (*vm)->GetEnv(vm, (void **)&env, HOOKEE_JNI_VERSION)) {
    LOGE("ERROR: GetEnv failed \n");
    return JNI_ERR;
  }
  if (NULL == env || NULL == *env) return JNI_ERR;

  /*jclass cls;
  if (NULL == (cls = (*env)->FindClass(env, HOOKEE_JNI_CLASS_NAME))) {
    LOGE("Native registration unable to find class '%s'", HOOKEE_JNI_CLASS_NAME);
    return JNI_ERR;
  }

  JNINativeMethod m[] = {{"nativeTest", "()V", (void *)hookee_test}};
  if (0 != (*env)->RegisterNatives(env, cls, m, sizeof(m) / sizeof(m[0]))) return JNI_ERR;*/
  if (0 != registerNativeMethods(env,HOOKEE_JNI_CLASS_NAME,gMethods,NELEM(gMethods))){
    LOGE("ERROR: JNI_OnLoad native registration failed\n");
    return JNI_ERR;
  }

  //找到全局的classloader
  if (0 != init_JniClass(vm)){
    LOGE("ERROR: JNI_OnLoad find class failed");
    return JNI_ERR;
  }


  return HOOKEE_JNI_VERSION;
}
