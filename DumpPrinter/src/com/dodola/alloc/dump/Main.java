package com.dodola.alloc.dump;

import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.util.ArrayList;

public class Main {

    public static void main(String[] args) throws IOException {
        if (args.length > 0) {
            String filePath = args[0];
            FileInputStream fIn = new FileInputStream(filePath);
            FileChannel fChan = fIn.getChannel();
            long fSize = fChan.size();
            ByteBuffer mBuf = ByteBuffer.allocate((int) fSize);
            fChan.read(mBuf);
            mBuf.rewind();
            ArrayList<AllocationInfo> allocationInfos = HandleHeap.handleRecentAlloc(mBuf);
            HandleHeap.dumpRecords(allocationInfos);
        } else {
            System.out.println("请传入日志地址");
        }
    }
}
