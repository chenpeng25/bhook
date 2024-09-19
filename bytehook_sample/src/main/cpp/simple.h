//
// Created by chenpeng on 2024/9/13.
//


#ifndef BHOOK_SIMPLE_H
#define BHOOK_SIMPLE_H

#include <stdbool.h>

#include "kdvtype.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLE_SOLUTION_CFI 1
#define SAMPLE_SOLUTION_FP  2
#define SAMPLE_SOLUTION_EH  3

void sample_test(u8 solution,bool remote_unwind,bool with_context,bool signal_interrupted);
void sample_signal_register(void);

#ifdef __cplusplus
}
#endif

#endif  // BHOOK_SIMPLE_H
