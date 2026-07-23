package com.multirium.mobile.gui.notify;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.ObjectAnimator;
import android.app.Activity;
import android.media.SoundPool;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.DecelerateInterpolator;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.core.view.ViewCompat;

import com.multirium.mobile.R;
import com.multirium.mobile.core.Samp;

public class CustomNotify {
    private static View rootView;
    private static TextView tvText;
    private static ImageView ivIcon;

    private static int soundId = 0;

    private static int lastNpcId = 0;
    private static int lastActionId = 0;

    private static ObjectAnimator pulseScaleX;
    private static ObjectAnimator pulseScaleY;
    private static boolean pulseRunning = false;

    private static native void nativeOnClick(int npcId, int actionId);
    private static native void nativeInit();

    public static void init() {
        try { nativeInit(); } catch (Throwable t) {}
    }

    private static int dp(Activity a, int v) {
        float d = a.getResources().getDisplayMetrics().density;
        return (int)(v * d + 0.5f);
    }

    private static void ensure(Activity a) {
        if (rootView != null) return;

        FrameLayout ui = a.findViewById(R.id.ui_layout);
        if (ui == null) return;

        rootView = LayoutInflater.from(a).inflate(R.layout.view_custom_notify, ui, false);

        tvText = rootView.findViewById(R.id.message);
        ivIcon = rootView.findViewById(R.id.icon);

        rootView.setClickable(true);
        rootView.setFocusable(true);
        rootView.setFocusableInTouchMode(true);

        // держим реально сверху
        rootView.bringToFront();
        ViewCompat.setTranslationZ(rootView, 1000f);

        // ЖЁСТКИЙ ТАП: ловим ACTION_UP и шлём клик всегда
        View.OnTouchListener touch = (v, e) -> {
            if (e.getAction() == MotionEvent.ACTION_UP) {
                nativeOnClick(lastNpcId, lastActionId);
                v.performClick();
            }
            return true; // важно: съедаем тач, чтобы игра не забирала
        };

        rootView.setOnTouchListener(touch);
        if (tvText != null) tvText.setOnTouchListener(touch);
        if (ivIcon != null) ivIcon.setOnTouchListener(touch);

        rootView.setAlpha(0f);
        rootView.setTranslationY(220f);
        rootView.setScaleX(0.98f);
        rootView.setScaleY(0.98f);
        rootView.setVisibility(View.GONE);

        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT,
                ViewGroup.LayoutParams.WRAP_CONTENT
        );
        lp.gravity = Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL;
        lp.bottomMargin = dp(a, 18);
        ui.addView(rootView, lp);

        SoundPool sp = Samp.soundPool;
        if (sp != null) soundId = sp.load(a, R.raw.uved, 1);

        pulseScaleX = ObjectAnimator.ofFloat(rootView, View.SCALE_X, 1.0f, 1.03f, 1.0f);
        pulseScaleX.setDuration(850);
        pulseScaleX.setRepeatCount(ObjectAnimator.INFINITE);
        pulseScaleX.setInterpolator(new AccelerateDecelerateInterpolator());

        pulseScaleY = ObjectAnimator.ofFloat(rootView, View.SCALE_Y, 1.0f, 1.03f, 1.0f);
        pulseScaleY.setDuration(850);
        pulseScaleY.setRepeatCount(ObjectAnimator.INFINITE);
        pulseScaleY.setInterpolator(new AccelerateDecelerateInterpolator());
    }

    private static void startPulse() {
        if (rootView == null) return;
        if (pulseRunning) return;
        pulseRunning = true;
        pulseScaleX.start();
        pulseScaleY.start();
    }

    private static void stopPulse() {
        pulseRunning = false;
        if (pulseScaleX != null) pulseScaleX.cancel();
        if (pulseScaleY != null) pulseScaleY.cancel();
    }

    public static void show(String text, int npcId, int actionId) {
        Activity a = Samp.getActivity();
        if (a == null) return;

        a.runOnUiThread(() -> {
            ensure(a);
            if (rootView == null) return;

            lastNpcId = npcId;
            lastActionId = actionId;

            if (tvText != null) tvText.setText(text == null ? "" : text);

            rootView.animate().setListener(null);
            rootView.animate().cancel();
            rootView.clearAnimation();
            stopPulse();

            rootView.setVisibility(View.VISIBLE);
            rootView.bringToFront();
            ViewCompat.setTranslationZ(rootView, 1000f);

            rootView.setAlpha(0f);
            rootView.setTranslationY(220f);
            rootView.setScaleX(0.98f);
            rootView.setScaleY(0.98f);

            rootView.animate()
                    .alpha(1f)
                    .translationY(0f)
                    .scaleX(1f)
                    .scaleY(1f)
                    .setDuration(260)
                    .setInterpolator(new DecelerateInterpolator())
                    .withEndAction(CustomNotify::startPulse)
                    .start();

            SoundPool sp = Samp.soundPool;
            if (sp != null && soundId != 0) sp.play(soundId, 1f, 1f, 1, 0, 1f);
        });
    }

    public static void hide() {
        Activity a = Samp.getActivity();
        if (a == null) return;

        a.runOnUiThread(() -> {
            if (rootView == null) return;

            rootView.animate().setListener(null);
            rootView.animate().cancel();
            rootView.clearAnimation();
            stopPulse();

            rootView.animate()
                    .alpha(0f)
                    .translationY(220f)
                    .scaleX(0.98f)
                    .scaleY(0.98f)
                    .setDuration(200)
                    .setInterpolator(new DecelerateInterpolator())
                    .setListener(new AnimatorListenerAdapter() {
                        @Override
                        public void onAnimationEnd(Animator animation) {
                            if (rootView != null) rootView.setVisibility(View.GONE);
                        }
                    })
                    .start();
        });
    }
}
