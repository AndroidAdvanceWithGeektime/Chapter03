package com.dodola.alloctrack;

/**
 * Created by dodola on 2018-11-15.
 */
public class AllocTracker {
    static {
        System.loadLibrary("alloc-lib");
    }

    public native void setSaveDataDirectory(String dir);

    public native void dumpAllocationDataInLog();

    public native void startAllocationTracker();

    public native void stopAllocationTracker();

    public native int initForArt(int apiLevel, int allocRecordMax);

}
