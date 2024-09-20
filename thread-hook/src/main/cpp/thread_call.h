//
// Created by chenpeng on 2024/9/20.
//

#ifndef BHOOK_THREAD_CALL_H
#define BHOOK_THREAD_CALL_H

enum ThreadAction { ACTION_CREATE, ACTION_START, ACTION_SET_NAME };

class ThreadCall {};

class ThreadCallCreate : public ThreadCall {
 public:
  ThreadCallCreate(int cid,char *stackstrace):cid(cid),stackstrace(stackstrace){}
    ~ThreadCallCreate();
    int cid;
    char *stackstrace;

};

class ThreadCallStart: public ThreadCall{

 public:
  ThreadCallStart(int tid,int cid):tid(tid),cid(cid){}
  ~ThreadCallStart();
  int tid;
  int cid;
};

class ThreadCallSetName : public ThreadCall{
 public:
  ThreadCallSetName(int tid,char *name):tid(tid),name(name){}
  ~ThreadCallSetName();
  int tid;
  char *name;
};

#endif  // BHOOK_THREAD_CALL_H
