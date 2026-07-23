package com.multirium.mobile.gui.dialog;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.app.Activity;
import android.text.TextUtils;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.DecelerateInterpolator;
import android.widget.FrameLayout;
import android.widget.TextView;

import androidx.core.view.ViewCompat;

import com.multirium.mobile.R;
import com.multirium.mobile.core.Samp;

public class GtaDialog {
    private static View rootView;
    private static View dialogPanel;
    private static TextView tvTitle;
    private static TextView tvText;
    private static TextView btnLeft;
    private static TextView btnRight;

    private static int lastDialogId = 0;

    private static native void nativeOnButton(int dialogId, int button);
    private static native void nativeInit();

    public static void init() {
        try { nativeInit(); } catch (Throwable t) {}
    }

    private static int dp(Activity a, int v) {
        float d = a.getResources().getDisplayMetrics().density;
        return (int)(v * d + 0.5f);
    }

    private static boolean isPointInsideView(View v, float rawX, float rawY) {
        if (v == null) return false;
        int[] loc = new int[2];
        v.getLocationOnScreen(loc);
        float x = rawX;
        float y = rawY;
        return x >= loc[0] && x <= loc[0] + v.getWidth()
                && y >= loc[1] && y <= loc[1] + v.getHeight();
    }

    private static void ensure(Activity a) {
        if (rootView != null) return;

        FrameLayout ui = a.findViewById(R.id.ui_layout);
        if (ui == null) return;

        rootView = LayoutInflater.from(a).inflate(R.layout.ui_gta_dialog, ui, false);
        rootView.setTag("gta_dialog_root");

        tvTitle = rootView.findViewById(R.id.gta_dialog_title);
        tvText = rootView.findViewById(R.id.gta_dialog_text);
        btnLeft = rootView.findViewById(R.id.gta_dialog_btn_left);
        btnRight = rootView.findViewById(R.id.gta_dialog_btn_right);

        dialogPanel = rootView.findViewById(R.id.gta_dialog_panel);

        rootView.setClickable(true);
        rootView.setFocusable(true);
        rootView.setFocusableInTouchMode(true);

        rootView.bringToFront();
        ViewCompat.setTranslationZ(rootView, 1000f);

        rootView.setOnTouchListener((v, e) -> {
            float x = e.getRawX();
            float y = e.getRawY();

            boolean inside =
                    isPointInsideView(dialogPanel, x, y) ||
                            isPointInsideView(tvText, x, y) ||
                            isPointInsideView(btnLeft, x, y) ||
                            isPointInsideView(btnRight, x, y) ||
                            isPointInsideView(tvTitle, x, y);

            return !inside;
        });

        View.OnTouchListener touchBtn = (v, e) -> {
            if (e.getAction() == MotionEvent.ACTION_UP) v.performClick();
            return true;
        };

        btnLeft.setOnTouchListener(touchBtn);
        btnRight.setOnTouchListener(touchBtn);

        btnLeft.setOnClickListener(v -> {
            nativeOnButton(lastDialogId, 0);
            hide();
        });

        btnRight.setOnClickListener(v -> {
            nativeOnButton(lastDialogId, 1);
            hide();
        });

        rootView.setAlpha(0f);
        rootView.setTranslationY(dp(a, 220));
        rootView.setScaleX(0.98f);
        rootView.setScaleY(0.98f);
        rootView.setVisibility(View.GONE);

        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
        );
        lp.gravity = Gravity.BOTTOM;
        ui.addView(rootView, lp);
    }

    public static void show(int dialogId, String title, String text, String left, String right) {
        Activity a = Samp.getActivity();
        if (a == null) return;

        a.runOnUiThread(() -> {
            ensure(a);
            if (rootView == null) return;

            Samp.setCustomDialogUiHidden(true);

            lastDialogId = dialogId;

            String tTitle = stripSampColors(title);
            String tText = stripSampColors(text);

            String leftTxt = stripSampColors(left);
            String rightTxt = stripSampColors(right);

            if (tvTitle != null) tvTitle.setText(tTitle);

            if (tvText != null) {
                tvText.setText(tText);
                setupTextView(tvText);
            }

            if (btnLeft != null) {
                btnLeft.setText(safeButtonText(leftTxt));
                setupButton(btnLeft);
            }

            if (btnRight != null) {
                btnRight.setText(safeButtonText(rightTxt));
                setupButton(btnRight);
            }

            rootView.animate().setListener(null);
            rootView.animate().cancel();
            rootView.clearAnimation();

            rootView.setVisibility(View.VISIBLE);
            rootView.bringToFront();
            ViewCompat.setTranslationZ(rootView, 1000f);

            rootView.setAlpha(0f);
            rootView.setTranslationY(dp(a, 220));
            rootView.setScaleX(0.98f);
            rootView.setScaleY(0.98f);

            rootView.animate()
                    .alpha(1f)
                    .translationY(0f)
                    .scaleX(1f)
                    .scaleY(1f)
                    .setDuration(220)
                    .setInterpolator(new DecelerateInterpolator())
                    .start();
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

            rootView.animate()
                    .alpha(0f)
                    .translationY(dp(a, 220))
                    .scaleX(0.98f)
                    .scaleY(0.98f)
                    .setDuration(170)
                    .setInterpolator(new DecelerateInterpolator())
                    .setListener(new AnimatorListenerAdapter() {
                        @Override
                        public void onAnimationEnd(Animator animation) {
                            if (rootView != null) rootView.setVisibility(View.GONE);
                            lastDialogId = 0;
                            Samp.setCustomDialogUiHidden(false);
                        }
                    })
                    .start();
        });
    }

    private static String stripSampColors(String s) {
        if (s == null) return "";

        s = s.replaceAll("(?i)[\\{\\(][0-9A-F]{6,8}[\\}\\)]", "");
        s = s.replaceAll("(?i)\\b[0-9A-F]{6,8}[\\}\\)]", "");

        s = s.replace('\u00A0', ' ');
        s = s.replaceAll("[ \\t\\x0B\\f\\r]+", " ").trim();

        return s;
    }

    private static void setupTextView(TextView tv) {
        if (tv == null) return;
        tv.setSingleLine(false);
        tv.setMaxLines(6);
        tv.setEllipsize(null);
    }

    private static String safeButtonText(String s) {
        String v = stripSampColors(s);
        if (v == null) v = "";
        v = v.replace('\n', ' ').replace('\r', ' ').trim();
        v = v.replaceAll("\\s+", " ");

        // если сервер прислал пусто — чтобы не было пустых кнопок
        if (v.length() == 0) v = "OK";

        if (v.length() > 16) v = v.substring(0, 16).trim();
        return v;
    }

    private static void setupButton(TextView tv) {
        tv.setAllCaps(false);
        tv.setSingleLine(true);
        tv.setEllipsize(TextUtils.TruncateAt.END);
        tv.setGravity(Gravity.CENTER);
        tv.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16f);
    }
}
