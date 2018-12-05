package com.dodola.alloc.dump;

public class AllocationInfo implements Comparable<AllocationInfo>, IStackTraceInfo {
    private String mAllocatedClass;
    private int mAllocationSize;
    private short mThreadId;
    private StackTraceElement[] mStackTrace;
    /*
     * Simple constructor.
     */
    AllocationInfo(String allocatedClass, int allocationSize,
        short threadId, StackTraceElement[] stackTrace) {
        mAllocatedClass = allocatedClass;
        mAllocationSize = allocationSize;
        mThreadId = threadId;
        mStackTrace = stackTrace;
    }
    
    /**
     * Returns the name of the allocated class.
     */
    public String getAllocatedClass() {
        return mAllocatedClass;
    }
    /**
     * Returns the size of the allocation.
     */
    public int getSize() {
        return mAllocationSize;
    }
    /**
     * Returns the id of the thread that performed the allocation.
     */
    public short getThreadId() {
        return mThreadId;
    }
    /*
     * (non-Javadoc)
     * @see com.android.ddmlib.IStackTraceInfo#getStackTrace()
     */
    public StackTraceElement[] getStackTrace() {
        return mStackTrace;
    }
    public int compareTo(AllocationInfo otherAlloc) {
        return otherAlloc.mAllocationSize - mAllocationSize;
    }
}