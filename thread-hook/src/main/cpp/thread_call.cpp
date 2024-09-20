//
// Created by chenpeng on 2024/9/20.
//

#include "thread_call.h"


ThreadCallStart::~ThreadCallStart() = default;

ThreadCallSetName::~ThreadCallSetName() {
  delete[] name;
}

ThreadCallCreate::~ThreadCallCreate() {
  delete[] stackstrace;
}