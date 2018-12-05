package com.dodola.alloctrack;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;

public class MainActivity extends AppCompatActivity {
    static AllocTracker tracker;

    static {


    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        tracker = new AllocTracker();
        tracker.setSaveDataDirectory(getFilesDir().getAbsolutePath());
        tracker.initForArt(BuildConfig.VERSION_CODE, 1000);
//        tracker.startAllocationTracker();
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

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}
