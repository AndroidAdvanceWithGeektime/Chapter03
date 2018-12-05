package com.dodola.alloc.dump;

public interface IStackTraceInfo {
    /**
     * Returns the stack trace. This can be <code>null</code>.
     */
    public StackTraceElement[] getStackTrace();
}