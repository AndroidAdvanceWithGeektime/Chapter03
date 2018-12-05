package com.dodola.alloctrack;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

public class MainActivity extends AppCompatActivity {
    static AllocTracker tracker = new AllocTracker();


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tracker.setSaveDataDirectory(getFilesDir().getAbsolutePath());
        tracker.initForArt(BuildConfig.VERSION_CODE, 5000);
        findViewById(R.id.btn_start).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                tracker.startAllocationTracker();
            }
        });
        findViewById(R.id.btn_stop).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                tracker.stopAllocationTracker();
            }
        });


        findViewById(R.id.gen_obj).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                tracker.dumpAllocationDataInLog();
            }
        });
    }
}
