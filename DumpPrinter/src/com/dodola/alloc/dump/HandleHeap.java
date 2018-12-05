package com.dodola.alloc.dump;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collections;

/**
 * Created by dodola on 2018-12-05.
 */
public class HandleHeap {

    /**
     * Utility function to copy a String out of a ByteBuffer.
     * <p>
     * This is here because multiple chunk handlers can make use of it,
     * and there's nowhere better to put it.
     */
    static String getString(ByteBuffer buf, int len) {
        char[] data = new char[len];
        for (int i = 0; i < len; i++) {
            data[i] = buf.getChar();
        }
        return new String(data);
    }

    /**
     * Utility function to copy a String into a ByteBuffer.
     */
    static void putString(ByteBuffer buf, String str) {
        int len = str.length();
        for (int i = 0; i < len; i++) {
            buf.putChar(str.charAt(i));
        }
    }

    /**
     * Convert a 4-character string to a 32-bit type.
     */
    static int type(String typeName) {
        int val = 0;
        if (typeName.length() != 4) {
            throw new RuntimeException("Type name must be 4 letter long");
        }
        for (int i = 0; i < 4; i++) {
            val <<= 8;
            val |= (byte) typeName.charAt(i);
        }
        return val;
    }

    /**
     * Convert an integer type to a 4-character string.
     */
    static String name(int type) {
        char[] ascii = new char[4];
        ascii[0] = (char) ((type >> 24) & 0xff);
        ascii[1] = (char) ((type >> 16) & 0xff);
        ascii[2] = (char) ((type >> 8) & 0xff);
        ascii[3] = (char) (type & 0xff);
        return new String(ascii);
    }

    /*
     * For debugging: dump the contents of an AllocRecord array.
     *
     * The array starts with the oldest known allocation and ends with
     * the most recent allocation.
     */
    @SuppressWarnings("unused")
    public static void dumpRecords(ArrayList<AllocationInfo> records) {
        System.out.println("Found " + records.size() + " records:");
        for (AllocationInfo rec : records) {
            System.out.println("tid=" + rec.getThreadId() + " "
                + rec.getAllocatedClass() + " (" + rec.getSize() + " bytes)");
            for (StackTraceElement ste : rec.getStackTrace()) {
                if (ste.isNativeMethod()) {
                    System.out.println("    " + ste.getClassName()
                        + "." + ste.getMethodName()
                        + " (Native method)");
                } else {
                    System.out.println("    " + ste.getClassName()
                        + "." + ste.getMethodName()
                        + " (" + ste.getFileName()
                        + ":" + ste.getLineNumber() + ")");
                }
            }
        }
    }

    public static String dumpRecordsToString(ArrayList<AllocationInfo> records) {
        StringBuilder builder = new StringBuilder();
        builder.append("Found " + records.size() + " records:\n");
        for (AllocationInfo rec : records) {
            builder.append("tid=" + rec.getThreadId() + " "
                + rec.getAllocatedClass() + " (" + rec.getSize() + " bytes)\n");
            for (StackTraceElement ste : rec.getStackTrace()) {
                if (ste.isNativeMethod()) {
                    builder.append("    " + ste.getClassName()
                        + "." + ste.getMethodName()
                        + " (Native method)\n");
                } else {
                    builder.append("    " + ste.getClassName()
                        + "." + ste.getMethodName()
                        + " (" + ste.getFileName()
                        + ":" + ste.getLineNumber() + ")\n");
                }
            }
        }
        return builder.toString();
    }

    /**
     * Converts a VM class descriptor string ("Landroid/os/Debug;") to
     * a dot-notation class name ("android.os.Debug").
     */
    private static String descriptorToDot(String str) {
        // count the number of arrays.
        int array = 0;
        while (str.startsWith("[")) {
            str = str.substring(1);
            array++;
        }
        int len = str.length();
        /* strip off leading 'L' and trailing ';' if appropriate */
        if (len >= 2 && str.charAt(0) == 'L' && str.charAt(len - 1) == ';') {
            str = str.substring(1, len - 1);
            str = str.replace('/', '.');
        } else {
            // convert the basic types
            if ("C".equals(str)) {
                str = "char";
            } else if ("B".equals(str)) {
                str = "byte";
            } else if ("Z".equals(str)) {
                str = "boolean";
            } else if ("S".equals(str)) {
                str = "short";
            } else if ("I".equals(str)) {
                str = "int";
            } else if ("J".equals(str)) {
                str = "long";
            } else if ("F".equals(str)) {
                str = "float";
            } else if ("D".equals(str)) {
                str = "double";
            }
        }
        // now add the array part
        for (int a = 0; a < array; a++) {
            str = str + "[]";
        }
        return str;
    }

    /**
     * Reads a string table out of "data".
     * <p>
     * This is just a serial collection of strings, each of which is a
     * four-byte length followed by UTF-16 data.
     */
    private static void readStringTable(ByteBuffer data, String[] strings) {
        int count = strings.length;
        int i;
        for (i = 0; i < count; i++) {
            int nameLen = data.getInt();
            String descriptor = getString(data, nameLen);
            strings[i] = descriptorToDot(descriptor);
        }
    }

    /*
     * Handle a REcent ALlocation response.
     *
     * Message header (all values big-endian):
     *   (1b) message header len (to allow future expansion); includes itself
     *   (1b) entry header len
     *   (1b) stack frame len
     *   (2b) number of entries
     *   (4b) offset to string table from start of message
     *   (2b) number of class name strings
     *   (2b) number of method name strings
     *   (2b) number of source file name strings
     *   For each entry:
     *     (4b) total allocation size
     *     (2b) threadId
     *     (2b) allocated object's class name index
     *     (1b) stack depth
     *     For each stack frame:
     *       (2b) method's class name
     *       (2b) method name
     *       (2b) method source file
     *       (2b) line number, clipped to 32767; -2 if native; -1 if no source
     *   (xb) class name strings
     *   (xb) method name strings
     *   (xb) source file strings
     *
     *   As with other DDM traffic, strings are sent as a 4-byte length
     *   followed by UTF-16 data.
     */
    public static ArrayList<AllocationInfo> handleRecentAlloc(ByteBuffer data) {
        int messageHdrLen, entryHdrLen, stackFrameLen;
        int numEntries, offsetToStrings;
        int numClassNames, numMethodNames, numFileNames;
        /*
         * Read the header.
         */
        messageHdrLen = (data.get() & 0xff);
        entryHdrLen = (data.get() & 0xff);
        stackFrameLen = (data.get() & 0xff);
        numEntries = (data.getShort() & 0xffff);
        offsetToStrings = data.getInt();
        numClassNames = (data.getShort() & 0xffff);
        numMethodNames = (data.getShort() & 0xffff);
        numFileNames = (data.getShort() & 0xffff);
        /*
         * Skip forward to the strings and read them.
         */
        data.position(offsetToStrings);
        String[] classNames = new String[numClassNames];
        String[] methodNames = new String[numMethodNames];
        String[] fileNames = new String[numFileNames];
        readStringTable(data, classNames);
        readStringTable(data, methodNames);
        //System.out.println("METHODS: "
        //    + java.util.Arrays.deepToString(methodNames));
        readStringTable(data, fileNames);
        /*
         * Skip back to a point just past the header and start reading
         * entries.
         */
        data.position(messageHdrLen);
        ArrayList<AllocationInfo> list = new ArrayList<AllocationInfo>(numEntries);
        for (int i = 0; i < numEntries; i++) {
            int totalSize;
            int threadId, classNameIndex, stackDepth;
            totalSize = data.getInt();
            threadId = (data.getShort() & 0xffff);
            classNameIndex = (data.getShort() & 0xffff);
            stackDepth = (data.get() & 0xff);
            /* we've consumed 9 bytes; gobble up any extra */
            for (int skip = 9; skip < entryHdrLen; skip++) {
                data.get();
            }
            StackTraceElement[] steArray = new StackTraceElement[stackDepth];
            /*
             * Pull out the stack trace.
             */
            for (int sti = 0; sti < stackDepth; sti++) {
                int methodClassNameIndex, methodNameIndex;
                int methodSourceFileIndex;
                short lineNumber;
                String methodClassName, methodName, methodSourceFile;
                methodClassNameIndex = (data.getShort() & 0xffff);
                methodNameIndex = (data.getShort() & 0xffff);
                methodSourceFileIndex = (data.getShort() & 0xffff);
                lineNumber = data.getShort();
                methodClassName = classNames[methodClassNameIndex];
                methodName = methodNames[methodNameIndex];
                methodSourceFile = fileNames[methodSourceFileIndex];
                steArray[sti] = new StackTraceElement(methodClassName,
                    methodName, methodSourceFile, lineNumber);
                /* we've consumed 8 bytes; gobble up any extra */
                for (int skip = 9; skip < stackFrameLen; skip++) {
                    data.get();
                }
            }
            list.add(new AllocationInfo(classNames[classNameIndex],
                totalSize, (short) threadId, steArray));
        }
        // sort biggest allocations first.
        Collections.sort(list);
        return list;
    }
}
