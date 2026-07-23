package com.multirium.mobile.gui.miner;

import android.app.Activity;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.util.TypedValue;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import com.joom.paranoid.Obfuscate;
import com.multirium.mobile.R;
import com.multirium.mobile.core.Samp;

@Obfuscate
public class MinerIndicatorManager {
    private Activity activity;
    private ViewGroup root;

    private TextView status;
    private TextView ore;
    private TextView ingot;

    private View indicatorRoot;

    private boolean alignedOnce = false;
    private boolean wasVisible = false;

    private final Handler animHandler = new Handler(Looper.getMainLooper());
    private Runnable animRunnable = null;

    private native void nativeInit();

    public MinerIndicatorManager(Activity activity) {
        this.activity = activity;

        root = (ViewGroup) LayoutInflater.from(activity).inflate(R.layout.hud_miner_indicator, null, false);

        FrameLayout back = Samp.getInstance().getBackUILayout();

        FrameLayout.LayoutParams lp = new FrameLayout.LayoutParams(
                FrameLayout.LayoutParams.WRAP_CONTENT,
                FrameLayout.LayoutParams.WRAP_CONTENT
        );
        lp.gravity = Gravity.TOP | Gravity.END;

        lp.topMargin = dp(110);
        lp.rightMargin = 0;

        back.addView(root, lp);

        indicatorRoot = root.findViewById(R.id.miner_indicator_root);
        if (indicatorRoot != null) {
            indicatorRoot.setVisibility(View.GONE);
            indicatorRoot.setScaleX(0.85f);
            indicatorRoot.setScaleY(0.85f);
        }

        status = root.findViewById(R.id.miner_indicator_status);
        ore = root.findViewById(R.id.miner_indicator_ore);
        ingot = root.findViewById(R.id.miner_indicator_ingot);

        setupAutoPositionUnderButtons();

        nativeInit();
    }

    private void setupAutoPositionUnderButtons() {
        final ViewTreeObserver vto = Samp.getInstance().getBackUILayout().getViewTreeObserver();
        vto.addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
            @Override
            public void onGlobalLayout() {
                View menu = findAnyView("menuBtn", "menu_btn", "menu");
                View inv  = findAnyView("inventoryBtn", "inventory_btn", "invBtn", "inv_btn");

                if (menu == null && inv == null) return;

                int bottom = 0;
                if (menu != null) bottom = Math.max(bottom, getBottomOnScreen(menu));
                if (inv != null)  bottom = Math.max(bottom, getBottomOnScreen(inv));

                int newTop = bottom + dp(28);

                FrameLayout.LayoutParams cur = (FrameLayout.LayoutParams) root.getLayoutParams();
                if (cur != null && cur.topMargin != newTop) {
                    cur.gravity = Gravity.TOP | Gravity.END;
                    cur.topMargin = newTop;
                    cur.rightMargin = 0;
                    root.setLayoutParams(cur);

                    alignedOnce = false;
                }
            }
        });
    }

    private View findAnyView(String... names) {
        for (String n : names) {
            int id = activity.getResources().getIdentifier(n, "id", activity.getPackageName());
            if (id != 0) {
                View v = activity.findViewById(id);
                if (v != null) return v;
            }
        }
        return null;
    }

    private int getBottomOnScreen(View v) {
        int[] loc = new int[2];
        v.getLocationOnScreen(loc);
        return loc[1] + v.getHeight();
    }

    private void alignToRightEdgeExact() {
        if (indicatorRoot == null) return;

        View backView = Samp.getInstance().getBackUILayout();
        if (backView == null) return;

        int[] backLoc = new int[2];
        int[] indLoc = new int[2];

        backView.getLocationOnScreen(backLoc);
        indicatorRoot.getLocationOnScreen(indLoc);

        int backRight = backLoc[0] + backView.getWidth();
        int indRight = indLoc[0] + indicatorRoot.getWidth();

        int diff = backRight - indRight;
        indicatorRoot.setTranslationX(indicatorRoot.getTranslationX() + diff);

        alignedOnce = true;
    }

    private void cancelManualAnim() {
        if (animRunnable != null) {
            animHandler.removeCallbacks(animRunnable);
            animRunnable = null;
        }
    }

    private void manualAnimate(final float fromX, final float toX, final float fromA, final float toA,
                               final long durationMs, final Runnable endAction) {
        if (indicatorRoot == null) return;

        cancelManualAnim();

        final long start = SystemClock.uptimeMillis();
        indicatorRoot.setTranslationX(fromX);
        indicatorRoot.setAlpha(fromA);

        animRunnable = new Runnable() {
            @Override
            public void run() {
                if (indicatorRoot == null) return;

                float t = (SystemClock.uptimeMillis() - start) / (float) durationMs;
                if (t > 1f) t = 1f;

                float k = 1f - (1f - t) * (1f - t);

                float x = fromX + (toX - fromX) * k;
                float a = fromA + (toA - fromA) * k;

                indicatorRoot.setTranslationX(x);
                indicatorRoot.setAlpha(a);

                if (t < 1f) {
                    animHandler.postDelayed(this, 16);
                } else {
                    animRunnable = null;
                    if (endAction != null) endAction.run();
                }
            }
        };

        animHandler.post(animRunnable);
    }

    private void playShowAnimFromRight() {
        if (indicatorRoot == null) return;
        manualAnimate(dp(46), 0f, 0f, 1f, 220, null);
    }

    private void playHideAnimToRight() {
        if (indicatorRoot == null) return;

        manualAnimate(0f, dp(36), 1f, 0f, 150, () -> {
            indicatorRoot.setVisibility(View.GONE);
            indicatorRoot.setAlpha(1f);
            indicatorRoot.setTranslationX(0f);
        });
    }

    public void update(final boolean show, final boolean inWork, final int oreCount, final int ingotCount) {
        if (activity == null || root == null) return;

        activity.runOnUiThread(() -> {
            if (indicatorRoot == null) {
                indicatorRoot = root.findViewById(R.id.miner_indicator_root);
                if (indicatorRoot == null) return;

                indicatorRoot.setScaleX(0.85f);
                indicatorRoot.setScaleY(0.85f);
            }

            if (!show) {
                if (wasVisible) {
                    playHideAnimToRight();
                } else {
                    indicatorRoot.setVisibility(View.GONE);
                }
                wasVisible = false;
                alignedOnce = false;
                cancelManualAnim();
                return;
            }

            indicatorRoot.setVisibility(View.VISIBLE);

            if (status != null) {
                if (inWork) {
                    status.setText("В РАБОТЕ");
                    status.setBackgroundResource(R.drawable.bg_miner_badge_work);
                    status.setTextColor(0xFFFFFFFF);
                } else {
                    status.setText("НЕ АКТИВЕН");
                    status.setBackgroundResource(R.drawable.bg_miner_badge);
                    status.setTextColor(0xBFFFFFFF);
                }
            }

            if (ore != null) ore.setText(String.valueOf(oreCount));
            if (ingot != null) ingot.setText(String.valueOf(ingotCount));

            indicatorRoot.post(() -> {
                indicatorRoot.setPivotX(indicatorRoot.getWidth());
                indicatorRoot.setPivotY(0f);

                if (!alignedOnce) {
                    alignToRightEdgeExact();
                }

                if (!wasVisible) {
                    playShowAnimFromRight();
                }

                wasVisible = true;
            });
        });
    }

    private int dp(int v) {
        return (int) TypedValue.applyDimension(
                TypedValue.COMPLEX_UNIT_DIP,
                v,
                activity.getResources().getDisplayMetrics()
        );
    }
}
