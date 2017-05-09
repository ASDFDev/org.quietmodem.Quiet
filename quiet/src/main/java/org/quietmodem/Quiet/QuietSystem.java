package org.quietmodem.Quiet;

public class QuietSystem {
    private static QuietSystem instance = null;

    private native long nativeOpen() throws ModemException;
    private native void nativeClose();
    private native void nativeOpenOpenSL() throws ModemException;
    private native void nativeOpenLoopback() throws ModemException;

    final long sys_ptr;
    private boolean init_opensl;

    protected QuietSystem() throws ModemException {
        this.sys_ptr = nativeOpen();
        this.init_opensl = false;
    }

    synchronized static QuietSystem getSystem() throws ModemException {
        if (instance == null) {
            instance = new QuietSystem();
        }
        return instance;
    }

    private synchronized static void deleteSystem() {
        instance = null;
    }

    public synchronized void close() {
        nativeClose();
        deleteSystem();
    }

    synchronized void initOpenSL() throws ModemException {
        if (!this.init_opensl) {
            nativeOpenOpenSL();
            this.init_opensl = true;
        }
    }

    static {
        QuietInit.init();
    }
}
