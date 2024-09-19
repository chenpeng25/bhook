//
// Created by chenpeng on 2024/9/10.
//
#include <android/log.h>
#include <errno.h>
#include <jni.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <ucontext.h>
#include <unistd.h>

#include "include/kdvtype.h"
#include "xunwind.h"
#include "simple.h"

#define HACKER_TAG            "bytehook_tag"
#define LOG_PRIORITY           ANDROID_LOG_INFO

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#define LOG(fmt, ...) __android_log_print(LOG_PRIORITY, HACKER_TAG, fmt, ##__VA_ARGS__)
#pragma clang diagnostic pop


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#define SAMPLE_TEMP_FAILURE_RETRY(exp)     \
  ({                                       \
    __typeof__(exp) _rc;                   \
    do {                                   \
      errno = 0;                           \
      _rc = (exp);                         \
    } while (_rc == -1 && errno == EINTR); \
    _rc;                                   \
  })
#pragma clang diagnostic pop


// parameters from JNI
static u8 g_solution = 0;
static bool g_remote_unwind = false;
static bool g_with_context = false;
static bool g_signal_interrupted = false;

//for sigsegv Linux的信号处理机制
static struct sigaction g_sigsegv_oldact;
static pid_t g_cur_tid =0;
static sigjmp_buf g_jb;

//uintptr_t是什么东西 无符号长整形
static uintptr_t g_frames[128];
static size_t g_frames_sz = 0;


/*
 *  ***Notice***
 *  在信号处理中进行CFI栈回溯的相关操作是不可靠
 */


/**
 * 这段代码的作用是通过在函数 sample_make_sigsegv 中写入一个非法内存地址（0x0），
 * 来强制触发一个段错误（SIGSEGV）。编译器优化在这个函数中被关闭，以确保这个行为不会被编译器优化掉
 *
 * 这个函数的目的是故意触发一个内存访问违规（Segmentation Fault, SIGSEGV），这通常用于测试异常处理或模拟崩溃情况
 */
#pragma clang optimize off
static void sample_make_sigsegv() {
  *((volatile int *)0) = 0;
}
#pragma clang optimize on

/*
 * fp栈回溯的原理
 * EH栈回溯的原理
 * CFI栈回溯的原理
 *
 * #define SIGABRT 6 // 调用abort函数生成的信号，表示程序异常
 */
static void sample_sigabrt_handler(int signum,siginfo_t *siginfo,void *context){
  (void)signum,(void)siginfo;

  if (g_solution == SAMPLE_SOLUTION_FP || g_solution == SAMPLE_SOLUTION_EH){
    if(!g_signal_interrupted){
      if (g_solution == SAMPLE_SOLUTION_FP){
        //fp进行栈回溯
#ifdef __aarch64__
        size_t  frames_sz = xunwind_fp_unwind(g_frames,sizeof(g_frames)/sizeof (g_frames[0]),g_with_context ? context :NULL);
        __atomic_store_n(&g_frames_sz,frames_sz,__ATOMIC_SEQ_CST);

#else
        (void)context;
        LOG("FP unwinding is only support on arm64 ");
#endif
      } else if ( g_solution == SAMPLE_SOLUTION_EH){
        //EH 栈回溯
        size_t  frames_sz = xunwind_eh_unwind(g_frames,sizeof(g_frames)/sizeof (g_frames[0]),g_with_context ? context :NULL);
        __atomic_store_n(&g_frames_sz,frames_sz,__ATOMIC_SEQ_CST);
      }

    } else{

      /*0
       * **
       * sigsetjmp(g_jb, 1)：设置一个跳转点并保存当前的堆栈上下文和信号掩码到 g_jb 中。

        sigsetjmp 返回值为 0 时表示这是第一次执行 sigsetjmp，而不是从 siglongjmp 跳转回来。
        g_jb 是一个 sigjmp_buf 类型的变量，保存了当前的堆栈和寄存器状态。
       *
       *
       * 通过 sigsetjmp 和 siglongjmp，可以在错误发生时控制程序的执行流程，从而在发生段错误后做出相应的恢复处理
       * 如果 sigsetjmp 返回 0，表示当前是第一次调用，则继续执行 sample_make_sigsegv() 以触发段错误。段错误发生后，
       * 程序会跳转回 sigsetjmp 的位置，并使其返回非零值（由 siglongjmp 触发）
       *
       */
      // trigger a segfault, we will do "FP local unwind" in the sigsegv's signal handler
      __atomic_store_n((pid_t *)&g_cur_tid, gettid(), __ATOMIC_SEQ_CST);
      if (0 == sigsetjmp(g_jb,1)) sample_make_sigsegv();
    }
  }else if (!g_remote_unwind ){
    //CFi 本地栈回溯
    xunwind_cfi_log(XUNWIND_CURRENT_PROCESS,XUNWIND_CURRENT_THREAD,context,
                    HACKER_TAG,LOG_PRIORITY,NULL);
  }else{
    //CFI remoute unwind
    LOG("CFI unwinding is no support CFI remote unwind ");
  }
}

/**
 * #define SIGSEGV 11
 * // 非法内存操作，与SIGBUS不同，他是对合法地址的非法访问，比如访问没有读权限的内存，向没有写权限的地址写数据

 */
static void sample_sigsegv_handler(int signum,siginfo_t *siginfo,void *context){
  (void)signum,(void)siginfo;
  pid_t tid = gettid();
  //下面这行代码是什么意思
  if (__atomic_compare_exchange_n(&g_cur_tid, &tid, 0, true, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)){
    if (g_solution == SAMPLE_SOLUTION_FP){
      //fp进行栈回溯
#ifdef __aarch64__
      size_t  frames_sz = xunwind_fp_unwind(g_frames,sizeof(g_frames)/sizeof (g_frames[0]),g_with_context ? context :NULL);
      __atomic_store_n(&g_frames_sz,frames_sz,__ATOMIC_SEQ_CST);

#else
      (void)context;
      LOG("FP unwinding is only support on arm64 ");
#endif
    }else if (g_solution == SAMPLE_SOLUTION_EH){
      //EH 栈回溯
      size_t  frames_sz = xunwind_eh_unwind(g_frames,sizeof(g_frames)/sizeof (g_frames[0]),g_with_context ? context :NULL);
      __atomic_store_n(&g_frames_sz,frames_sz,__ATOMIC_SEQ_CST);
    }
    siglongjmp(g_jb,1); //将当前执行流跳转到之前调用 sigsetjmp(g_jb, ...) 的位置。执行 siglongjmp() 后，sigsetjmp() 将返回 1，而不是通常的 0。
  }else {
    sigaction(SIGSEGV, &g_sigsegv_oldact, NULL);
  }

}
 void sample_signal_register(void) {
  struct sigaction act;
  memset(&act,0,sizeof(act));
  sigfillset(&act.sa_mask);
  sigdelset(&act.sa_mask,SIGSEGV);
  act.sa_sigaction = sample_sigabrt_handler;
  act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
  sigaction(SIGABRT, &act, NULL);


  memset(&act,0,sizeof(act));
  sigfillset(&act.sa_mask);
  act.sa_sigaction = sample_sigsegv_handler;
  act.sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK;
  sigaction(SIGSEGV, &act, &g_sigsegv_oldact);
}


 void sample_test(u8 solution,bool remote_unwind,bool with_context,bool signal_interrupted){
  g_solution = solution;
  g_remote_unwind = remote_unwind;
  g_with_context = with_context;
  g_signal_interrupted = signal_interrupted;

  __atomic_store_n(&g_frames_sz, 0, __ATOMIC_SEQ_CST);

  tgkill(getpid(),gettid(),SIGABRT);

  if ((solution == SAMPLE_SOLUTION_FP || solution == SAMPLE_SOLUTION_EH) &&
      __atomic_load_n(&g_frames_sz, __ATOMIC_SEQ_CST) > 0)
    xunwind_frames_log(g_frames, g_frames_sz, HACKER_TAG, LOG_PRIORITY, NULL);
}

    /***
     *
     *1、信号注册的原理
     *2、siglongjmp和sigsetjmp函数的作用
     *3、debugger -b 能反射到的java层+native层日志
     * 4、hook原理
     *  PLE Hook，执行
     *
     *
     *
     *
     */


