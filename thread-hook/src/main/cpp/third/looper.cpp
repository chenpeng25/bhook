//
// Created by chenpeng on 2024/9/19.
//

#include "looper.h"

#include <android/log.h>
#include <linux/resource.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/wait.h>

#include <cstdio>
#include <cstring>

namespace thread_hook {

#define HOOK_JNI_VERSION     JNI_VERSION_1_6
#define HOOK_TAG             "thread-hook"
#define THREAD_LOOP_PRIORITY -10

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO, HOOK_TAG, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) __android_log_print(ANDROID_LOG_WARN, HOOK_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, HOOK_TAG, fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop

struct LooperMessage {
  int what;
  void *obj;
  LooperMessage *next;
  bool quit;
};
void *looper::trampoline(void *p) {
  // 指定PR_SET_NAME（对应数字15），即设置线程名
  prctl(PR_SET_NAME, HOOK_TAG);
  setpriority(PRIO_PROCESS, 0, THREAD_LOOP_PRIORITY);
  ((looper *)p)->loop();
  return nullptr;
}

looper::looper() {
  // 1、信号量初始化
  sem_init(&headDataAvailable, 0, 0);
  sem_init(&headWriteProtect, 0, 1);

  /*
   * 2、它表示 线程属性对象，用于在创建线程时指定线程的各种属性，例如线程栈大小、调度策略
   * 线程栈大小 默认1020K
   * 调度策略，默认是什么调度
   * 线程分离状态
   */
  pthread_attr_t attr;
  size_t stacksize = 0;
  int policy;
  // 初始化线程属性对象
  pthread_attr_init(&attr);
  //&取地址运算符
  pthread_attr_getstacksize(&attr, &stacksize);
  LOG("Default stack size = %zu bytes\n", stacksize);
  pthread_attr_getschedpolicy(&attr, &policy);
  LOG("Default scheduling policy is : %d\n", policy);
  // int pthread_create(pthread_t* __pthread_ptr, pthread_attr_t const* __attr, void*
  // (*__start_routine)(void*), void*);
  pthread_create(&worker, &attr, trampoline, this);
  running = true;
}

looper::~looper() {
  if (running) {
    LOG("Looper deleted while still running,Some messages will not be processed");
    quit();
  }
}


void looper::post(int what, void *data, bool flush) {
  auto *msg = new LooperMessage();
  msg->what = what;
  msg->obj = data;
  msg->next = nullptr;
  msg->quit = false;
  addMsg(msg, flush);
}

void looper::addMsg(thread_hook::LooperMessage *msg, bool flush) {
  sem_wait(&headWriteProtect);//这行的作用
  LooperMessage *h = head;
  if (flush){
      while (h){
        LooperMessage *next = h->next;
        delete h;
        h = next;
      }
      h = nullptr;
  }

  if (h != nullptr){
    tail->next = msg;
    tail = msg;
  } else{
    head = msg;
    tail = msg;
  }
  sem_post(&headWriteProtect);
  sem_post(&headDataAvailable);
}

void looper::loop() {
  while (true){
    //wait for available message
    sem_wait(&headDataAvailable); //线程是否进入挂起
    //get next available message
    sem_wait(&headWriteProtect); //线程是否进入挂起
    LooperMessage *msg = head;
    if (msg == nullptr){
      LOGW("no message");
      sem_post(&headWriteProtect);
      continue;
    }
    head = msg->next;
    sem_post(&headWriteProtect);
    if (msg->quit){
      LOGW("quitting");
      delete msg;
      return;
    }
    LOG("processing msg %d",msg->what);
    handle(msg->what,msg->obj);
    delete msg;
  }
}

void looper::quit() {
  LOGW("Looper quit");
  auto *msg = new LooperMessage();
  msg->what = 0;
  msg->obj = nullptr;
  msg->next = nullptr;
  msg->quit = true;
  addMsg(msg, false);
  void *val;
  pthread_join(worker,&val);
  sem_destroy(&headDataAvailable);
  sem_destroy(&headWriteProtect);
  running = false;
}

void looper::handle(int what, void *data) {
  LOG("dropping msg %d %p",what,data);
}



}  // namespace thread_hook