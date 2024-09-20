//
// Created by chenpeng on 2024/9/20.
//

#ifndef BHOOK_THREAD_CALL_MONITOR_H
#define BHOOK_THREAD_CALL_MONITOR_H

#include "thread_call.h"
#include "third/looper.h"
class ThreadCallMonitor: public thread_hook::looper {
 public:
    ThreadCallMonitor();
    ~ThreadCallMonitor();

    void t_create(ThreadCallCreate* call);
    void t_start(ThreadCallStart* call);
    void t_set_name(ThreadCallSetName* call);

    void handle(int what,void *data);
    void post(int what,void *data);

};

#endif  // BHOOK_THREAD_CALL_MONITOR_H
