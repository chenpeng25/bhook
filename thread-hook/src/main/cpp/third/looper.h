//
// Created by chenpeng on 2024/9/19.
//

#ifndef BHOOK_LOOPER_H
#define BHOOK_LOOPER_H

namespace thread_hook {

#include <pthread.h>
#include <semaphore.h>

struct LooperMessage;

class looper {
 public:
  looper();
  ~looper();
  virtual void post(int what,void *data,bool flush = false);
  void quit();
  virtual void handle(int what,void *data);

 private:
  void addMsg(LooperMessage *msg,bool flush);
  static void *trampoline(void *p);
  void loop();
  LooperMessage *head = nullptr;
  LooperMessage *tail = nullptr;
  pthread_t worker;
  sem_t headWriteProtect; //这个字段不理解
  sem_t headDataAvailable;//这个字段不理解
  bool running;

};

}  // namespace thread_hook

#endif  // BHOOK_LOOPER_H
