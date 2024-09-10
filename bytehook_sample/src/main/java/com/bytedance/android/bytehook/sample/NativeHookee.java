package com.bytedance.android.bytehook.sample;

public class NativeHookee {
    public static void test() {
        nativeTest();
    }

    private static native void nativeTest();




    @CalledByNative("Native Code hookee.c call")
    public static String getStack() {
        return stackTraceToString(new Throwable().getStackTrace());
    }

    private static String stackTraceToString(final StackTraceElement[] arr) {
        if (arr == null) {
            return "";
        }

        StringBuffer sb = new StringBuffer();

        for (StackTraceElement stackTraceElement : arr) {
            String className = stackTraceElement.getClassName();
            // remove unused stacks
         /*   if (className.contains("java.lang.Thread")) {
                continue;
            }*/

            sb.append(stackTraceElement).append('\n');
        }
        return sb.toString();
    }
}
