package com.seawalker.android.thread.hook;

public class NativeLib {

    // Used to load the 'hook' library on application startup.
    static {
        System.loadLibrary("hook");
    }

    /**
     * A native method that is implemented by the 'hook' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}