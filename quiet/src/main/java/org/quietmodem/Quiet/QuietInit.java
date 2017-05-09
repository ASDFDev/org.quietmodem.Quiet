package org.quietmodem.Quiet;

public class QuietInit {
    private static native void nativeLWIPInit();

    private static boolean has_init = false;
    

    static void init() {
        if (has_init) {
            return;
        }
        has_init = true;
        System.loadLibrary("complex");
        System.loadLibrary("fec");
        System.loadLibrary("jansson");
        System.loadLibrary("liquid");
        System.loadLibrary("quiet");
        System.loadLibrary("quiet-jni");
    }
}
