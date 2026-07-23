package com.multirium.mobile.gui.main_menu.utils;

import android.os.Handler;
import android.os.Looper;
import android.widget.TextView;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;
public  class MainMenuTimer {
    public static final int $stable = 8;
    private ScheduledExecutorService executorService = Executors.newSingleThreadScheduledExecutor();
    private Handler handler = new Handler(Looper.getMainLooper());
    private ScheduledFuture<?> scheduledFuture;
    private int seconds;
    private TextView textView;
    private String text;

    public MainMenuTimer(TextView textView, String text) {
        this.textView = textView;
        this.text = text;
    }

    public void start() {
        if (scheduledFuture != null) {

            if (!scheduledFuture.isCancelled()) {
                return;
            }
        }
        this.scheduledFuture = this.executorService.scheduleAtFixedRate(new Runnable() {
            @Override
            public void run() {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        updateText();
                        seconds++;
                    }
                });
            }
        }, 0, 1, TimeUnit.SECONDS);

    }


    public void stop() {
        ScheduledFuture<?> scheduledFuture2 = this.scheduledFuture;
        if (scheduledFuture2 != null) {
            scheduledFuture2.cancel(false);
        }
    }

    public void reset() {
        stop();
        this.seconds = 0;
        updateText();
        start();
    }

    private void updateText() {
        String str;
        int i = this.seconds;
        int i2 = i / 3600;
        int i3 = (i % 3600) / 60;
        int i4 = i % 60;
        if (i2 > 0) {
            str = String.format("%02d:%02d:%02d",i2,i3,i4);
        } else {
            str = String.format("%02d:%02d",i3,i4);
        }
        textView.setText(text + str);
    }
}
