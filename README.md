# Chapter03

该例子主要实现了在运行时动态获取对象分配的情况，可以运用在自动化的分析中。

开发环境
=======
Android Studio 3.2.1
NDK 16~19

运行环境
======
项目支持Dalvik 和 Art 虚拟机，理论上兼容了4.0~8.1的机型，但只在7.1和8.0上实测过。
由于国内机型的差异化，无法适配所有机型，项目只做展示原理使用，并不稳定。

使用方式
======

1. 点击`开始记录`即可触发对象分配记录会在 logcat 中看见如下日志
```
/com.dodola.alloctrack I/AllocTracker: ====current alloc count 111=====
```
说明已经开始记录对象的分配

2. 当对象达到设置的最大数量的时候触发内存 dump,会有如下日志
```
com.dodola.alloctrack I/AllocTracker: saveARTAllocationData /data/user/0/com.dodola.alloctrack/files/1544005106 file, fd: 63
com.dodola.alloctrack I/AllocTracker: saveARTAllocationData write file to /data/user/0/com.dodola.alloctrack/files/1544005106
```


3. 数据会保存在 `/data/data/com.dodola/alloctrack/files`目录下

4. 数据解析。采集下来的数据无法直接通过编辑器打开，需要通过 dumpprinter 工具来进行解析，操作如下
```
//dump 工具存放在tools/dumper.jar 中
//调用方法：
java -jar tools/dumper.jar dump文件路径 > dump_log.txt
```

5. 然后就可以在 `dump_log.txt` 中看到解析出来的数据
采集到的数据基本格式如下：

```
Found 10240 records://dump 下来的数据包含对象数量
tid=7205 java.lang.Class (4144 bytes)//当前线程  类名  分配的大小
		//下面是分配该对象的时候当前线程的堆栈信息
    android.support.v7.widget.Toolbar.ensureMenuView (Toolbar.java:1047)
    android.support.v7.widget.Toolbar.setMenu (Toolbar.java:551)
    android.support.v7.widget.ToolbarWidgetWrapper.setMenu (ToolbarWidgetWrapper.java:370)
    android.support.v7.widget.ActionBarOverlayLayout.setMenu (ActionBarOverlayLayout.java:721)
    android.support.v7.app.AppCompatDelegateImpl.preparePanel (AppCompatDelegateImpl.java:1583)
    android.support.v7.app.AppCompatDelegateImpl.doInvalidatePanelMenu (AppCompatDelegateImpl.java:1869)
    android.support.v7.app.AppCompatDelegateImpl$2.run (AppCompatDelegateImpl.java:230)
    android.os.Handler.handleCallback (Handler.java:792)
    android.os.Handler.dispatchMessage (Handler.java:98)
    android.os.Looper.loop (Looper.java:176)
    android.app.ActivityThread.main (ActivityThread.java:6701)
    java.lang.reflect.Method.invoke (Native method)
    com.android.internal.os.Zygote$MethodAndArgsCaller.run (Zygote.java:246)
    com.android.internal.os.ZygoteInit.main (ZygoteInit.java:783)
```

原理解析
======
项目使用了ELF hook 和 inline hook 来拦截内存对象分配时候的 `RecordAllocation` 函数，通过拦截该接口可以快速获取到当时分配对象的类名和分配的内存大小。

在初始化的时候我们设置了一个分配对象数量的最大值，如果从 start 开始对象分配数量超过最大值就会触发内存 dump，然后清空 alloc 对象列表，重新计算。该功能和  Android Studio 里的 HeapDump 类似，只不过可以在代码级别更细粒度的进行控制。可以精确到方法级别。



Thanks
======
[HookZZ](https://github.com/jmpews/HookZz)
Substrate
fbjni
