package com.dodola.alloctrack;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity {
    private AllocTracker tracker = new AllocTracker();
    private Button dumpLogBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tracker.setSaveDataDirectory(getFilesDir().getAbsolutePath());
        tracker.initForArt(BuildConfig.VERSION_CODE, 5000);//从 start 开始触发到5000的数据就 dump 到文件中
        dumpLogBtn = findViewById(R.id.gen_obj);
        findViewById(R.id.btn_start).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                tracker.startAllocationTracker();
                dumpLogBtn.setEnabled(true);
            }
        });
        findViewById(R.id.btn_stop).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                tracker.stopAllocationTracker();
                dumpLogBtn.setEnabled(false);

            }
        });


        dumpLogBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        tracker.dumpAllocationDataInLog();
                    }
                }).start();
            }
        });
    }
}
