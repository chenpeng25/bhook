//
// Created by chenpeng on 2024/9/20.
//

#include "thread_call_monitor.h"

ThreadCallMonitor::ThreadCallMonitor():looper(){}

ThreadCallMonitor::~ThreadCallMonitor()  = default;

void ThreadCallMonitor::t_create(ThreadCallCreate *call) {
  post(ACTION_CREATE,call);
}

void ThreadCallMonitor::t_start(ThreadCallStart *call) {
  post(ACTION_START,call);
}

void ThreadCallMonitor::t_set_name(ThreadCallSetName *call) {
  post(ACTION_SET_NAME,call);
}

void ThreadCallMonitor::post(int what, void *data) {
  looper::post(what,data);
}

void ThreadCallMonitor::handle(int what, void *data) {

}