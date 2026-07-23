package com.multirium.mobile.launcher.telegram;

import android.animation.TimeInterpolator;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.SystemClock;
import android.provider.Settings;
import android.text.TextUtils;
import android.view.Choreographer;
import android.view.HapticFeedbackConstants;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.multirium.mobile.R;

public class TgBindActivity extends AppCompatActivity {

    public static final String EXTRA_NICK = "extra_nick";
    public static final String EXTRA_SLOT = "extra_slot";

    // JNI
    public static native String nativeGetClientNick();
    public static native void nativeSetClientNick(String nick);

    private static final String TG_BOT_USERNAME = "volya_mobile_bot";
    private static final String TG_DEEPLINK_BASE = "https://t.me/" + TG_BOT_USERNAME + "?start=";

    private View root;
    private View btnBack;
    private View tgCard;
    private TextView tvTitle;
    private TextView tvSubtitle;

    private boolean exitInProgress = false;

    // forced anim
    private interface FloatUpdate { void update(float f); }
    private interface EndCb { void end(); }

    private void forcedAnim(long durationMs, FloatUpdate update, EndCb end) {
        final long start = SystemClock.uptimeMillis();
        final long d = Math.max(1L, durationMs);
        final TimeInterpolator interp = new AccelerateDecelerateInterpolator();

        Choreographer.getInstance().postFrameCallback(new Choreographer.FrameCallback() {
            @Override public void doFrame(long frameTimeNanos) {
                long now = SystemClock.uptimeMillis();
                float raw = Math.min(1f, Math.max(0f, (now - start) / (float) d));
                float f = interp.getInterpolation(raw);
                if (update != null) update.update(f);

                if (raw < 1f) {
                    Choreographer.getInstance().postFrameCallback(this);
                } else {
                    if (end != null) end.end();
                }
            }
        });
    }

    private void buttonClick(View v) {
        if (v == null) return;

        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1)
                v.performHapticFeedback(HapticFeedbackConstants.TEXT_HANDLE_MOVE);
            else
                v.performHapticFeedback(HapticFeedbackConstants.KEYBOARD_TAP);
        } catch (Throwable ignored) {}

        final float s0 = 1f;
        final float s1 = 0.93f;
        forcedAnim(90, f -> {
            float s = s0 + (s1 - s0) * f;
            v.setScaleX(s);
            v.setScaleY(s);
        }, () -> forcedAnim(120, f -> {
            float s = s1 + (s0 - s1) * f;
            v.setScaleX(s);
            v.setScaleY(s);
        }, () -> {
            v.setScaleX(1f);
            v.setScaleY(1f);
        }));
    }

    private void hideSystemUI() {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                WindowInsetsController c = getWindow().getInsetsController();
                if (c != null) {
                    c.hide(WindowInsets.Type.systemBars());
                    c.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                }
            } else {
                @SuppressWarnings("deprecation")
                int flags = View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
                getWindow().getDecorView().setSystemUiVisibility(flags);
            }
        } catch (Throwable ignored) {}
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.launcher_auth);

        root = findViewById(android.R.id.content);
        btnBack = findViewById(R.id.btnBack);
        tgCard = findViewById(R.id.tgCard);


        hideSystemUI();

        if (btnBack != null) btnBack.setOnClickListener(v -> {
            buttonClick(v);
            finishFancy();
        });

        if (tgCard != null) {
            tgCard.setOnClickListener(v -> {
                buttonClick(v);

                Intent it = getIntent();
                String nick = it != null ? it.getStringExtra(EXTRA_NICK) : null;
                int slot = it != null ? it.getIntExtra(EXTRA_SLOT, 1) : 1;

                // приоритет: ник из клиента
                String clientNick = "";
                try { clientNick = nativeGetClientNick(); } catch (Throwable ignored) {}
                if (!TextUtils.isEmpty(clientNick)) nick = clientNick;

                if (TextUtils.isEmpty(nick)) {
                    Toast.makeText(this, "Ник не задан", Toast.LENGTH_SHORT).show();
                    return;
                }

                slot = Math.max(1, Math.min(3, slot));
                openTelegramBot(nick, slot);
            });
        }

        // красивый enter
        runEnterAnim();

        // мягкая жизнь на tgCard
        pulseCardLoop();
    }

    private void runEnterAnim() {
        if (root != null) {
            root.setAlpha(0f);
        }
        if (tgCard != null) {
            tgCard.setAlpha(0f);
            tgCard.setScaleX(0.96f);
            tgCard.setScaleY(0.96f);
            tgCard.setTranslationY(26f);
        }
        if (btnBack != null) {
            btnBack.setAlpha(0f);
            btnBack.setTranslationY(10f);
        }

        forcedAnim(260, f -> {
            if (root != null) root.setAlpha(f);

            if (tgCard != null) {
                tgCard.setAlpha(f);
                tgCard.setScaleX(0.96f + 0.04f * f);
                tgCard.setScaleY(0.96f + 0.04f * f);
                tgCard.setTranslationY(26f * (1f - f));
            }

            if (btnBack != null) {
                btnBack.setAlpha(f);
                btnBack.setTranslationY(10f * (1f - f));
            }
        }, null);
    }

    private boolean pulseOn = true;

    private void pulseCardLoop() {
        if (!pulseOn || tgCard == null) return;

        forcedAnim(1100, f -> {
            float wave = (float) Math.sin(f * Math.PI); // 0..1..0
            float s = 1f + 0.018f * wave;
            tgCard.setScaleX(s);
            tgCard.setScaleY(s);
            tgCard.setAlpha(0.92f + 0.08f * wave);
        }, this::pulseCardLoop);
    }

    private void finishFancy() {
        if (exitInProgress) return;
        exitInProgress = true;
        pulseOn = false;

        final View r = root;
        final View card = tgCard;

        float a0 = r != null ? r.getAlpha() : 1f;
        float ca0 = card != null ? card.getAlpha() : 1f;

        forcedAnim(200, f -> {
            float inv = 1f - f;
            if (r != null) r.setAlpha(a0 * inv);
            if (card != null) {
                card.setAlpha(ca0 * inv);
                card.setTranslationY(16f * f);
                card.setScaleX(1f - 0.02f * f);
                card.setScaleY(1f - 0.02f * f);
            }
        }, () -> {
            try { finish(); } catch (Throwable ignored) {}
            overridePendingTransition(0, 0);
        });
    }

    @Override
    public void onBackPressed() {
        finishFancy();
    }

    private void openTelegramBot(String nick, int slot) {
        String payload = buildBotStartPayload(nick, slot);
        String url = TG_DEEPLINK_BASE + Uri.encode(payload);

        try {
            Intent i = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
            i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(i);
        } catch (Exception e) {
            Toast.makeText(this, "Не удалось открыть Telegram", Toast.LENGTH_SHORT).show();
        }
    }

    private String buildBotStartPayload(String nick, int slot) {
        String n = sanitizeStartPart(safeNick(nick), 24);

        String androidId = sanitizeStartPart(getDeviceIdSafe(), 20);
        String mm = sanitizeStartPart(safeStr(Build.MANUFACTURER) + "_" + safeStr(Build.MODEL), 18);

        String sdk = sanitizeStartPart(String.valueOf(Build.VERSION.SDK_INT), 3);
        String rel = sanitizeStartPart(safeStr(Build.VERSION.RELEASE), 6);

        int s = Math.max(1, Math.min(3, slot));

        String payload = "bind_s" + s
                + "_" + n
                + "_d" + androidId
                + "_m" + mm
                + "_k" + sdk
                + "_a" + rel;

        if (payload.length() > 64) payload = payload.substring(0, 64);
        return payload;
    }

    private String getDeviceIdSafe() {
        String androidId = "";
        try { androidId = Settings.Secure.getString(getContentResolver(), Settings.Secure.ANDROID_ID); }
        catch (Exception ignored) {}
        if (TextUtils.isEmpty(androidId)) androidId = "unknown";
        return androidId;
    }

    private String sanitizeStartPart(String s, int maxLen) {
        String v = safeStr(s).trim();
        v = v.replaceAll("[^a-zA-Z0-9_-]", "");
        if (v.length() > maxLen) v = v.substring(0, maxLen);
        if (TextUtils.isEmpty(v)) v = "x";
        return v;
    }

    private String safeStr(String s) { return s == null ? "" : s; }
    private String safeNick(String s) { return s == null ? "" : s.trim(); }

    @Override
    protected void onResume() {
        super.onResume();
        hideSystemUI();
    }
}
