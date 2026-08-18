package com.multirium.mobile.core;

import android.content.Intent;
import android.media.AudioAttributes;
import android.media.SoundPool;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.widget.FrameLayout;

import androidx.appcompat.app.AppCompatActivity;

import com.google.firebase.crashlytics.FirebaseCrashlytics;
import com.joom.paranoid.Obfuscate;
import com.multirium.mobile.GUIManager;
import com.multirium.mobile.R;
import com.multirium.mobile.gui.auth.Autorization;
import com.multirium.mobile.gui.auth.RegistrationNative;
import com.multirium.mobile.gui.auth.SpawnSelectNative;
import com.multirium.mobile.gui.chat.Chat;
import com.multirium.mobile.gui.hud.HudManager;
import com.multirium.mobile.gui.miner.MinerIndicatorManager;
import com.multirium.mobile.gui.speedometr.Speedometr;
import com.multirium.mobile.launcher.MainScreen;
import com.multirium.mobile.launcher.CacheLoaderActivity;
import com.multirium.mobile.launcher.CachePaths;

import org.json.JSONObject;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;

import kotlin.jvm.internal.Intrinsics;

@Obfuscate
public class Samp extends GTASA implements HeightProvider.HeightListener {
    public static final Companion Companion = new Companion();
    private static final String TAG = "MULTIRIUM";
    private static Samp instance;

    private FrameLayout mBackUILayout = null;
    private FrameLayout mMainMenuRoot = null;

    public static AppCompatActivity activity;
    private static boolean sIsPaused = false;

    public MainScreen mMainScreen;
    public HudManager mHudManager;
    public Speedometr mSpeedometr;
    public Chat mChat;
    public MinerIndicatorManager mMinerIndicator;

    public static SoundPool soundPool;

    private static final ArrayList<View> sCustomDialogHiddenViews = new ArrayList<>();

    public static Samp getInstance() { return instance; }
    public static final AppCompatActivity getActivity() { return Companion.getActivity(); }

    private void showLoadingScreen() {
        runOnUiThread(() -> {
            JSONObject data = new JSONObject();
            try { data.put("o", 2); } catch (Exception e) { e.printStackTrace(); }
            GUIManager.receiveUIpacket(1, data);
        });
    }

    private void updateLoadingScreen() {
        runOnUiThread(() -> {
            JSONObject data = new JSONObject();
            try { data.put("loading", "ИДЁТ ЗАГРУЗКА ИГРЫ..."); } catch (Exception e) { e.printStackTrace(); }
            GUIManager.receiveUIpacket(1, data);
        });
    }

    private void hideLoadingScreen() {
        runOnUiThread(() -> {
            mMainScreen.show();
            JSONObject data = new JSONObject();
            try { data.put("o", 1); } catch (Exception e) { e.printStackTrace(); }
            GUIManager.receiveUIpacket(1, data);
        });
    }

    public void setPauseState(boolean pause) {
        sIsPaused = pause;
        runOnUiThread(() -> {
            FrameLayout mAndroidUI = findViewById(R.id.ui_layout);
            runOnUiThread(() -> mAndroidUI.setVisibility(pause ? View.GONE : View.VISIBLE));
        });
    }

    public static void setCustomDialogUiHidden(boolean hidden) {
        AppCompatActivity a = activity;
        if (a == null) return;

        a.runOnUiThread(() -> {
            FrameLayout ui = a.findViewById(R.id.ui_layout);
            if (ui == null) return;

            if (hidden) {
                ui.setVisibility(View.VISIBLE);

                sCustomDialogHiddenViews.clear();
                for (int i = 0; i < ui.getChildCount(); i++) {
                    View child = ui.getChildAt(i);
                    if (child == null) continue;

                    Object tag = child.getTag();
                    if (tag != null && "gta_dialog_root".equals(tag.toString())) continue;

                    if (child.getVisibility() == View.VISIBLE) {
                        sCustomDialogHiddenViews.add(child);
                        child.setVisibility(View.GONE);
                    }
                }
            } else {
                for (int i = 0; i < sCustomDialogHiddenViews.size(); i++) {
                    View v = sCustomDialogHiddenViews.get(i);
                    if (v != null) v.setVisibility(View.VISIBLE);
                }
                sCustomDialogHiddenViews.clear();
                ui.setVisibility(sIsPaused ? View.GONE : View.VISIBLE);
            }
        });
    }

    public FrameLayout getBackUILayout() { return this.mBackUILayout; }

    public void exitGame() {
        FirebaseCrashlytics.getInstance().setCrashlyticsCollectionEnabled(true);
        finishAndRemoveTask();
        System.exit(0);
    }

    private native void onInputEnd(byte[] str);

    public void OnInputEnd(String str) {
        byte[] toReturn = null;
        try { toReturn = str.getBytes("windows-1251"); } catch (UnsupportedEncodingException ignored) {}

        try { onInputEnd(toReturn); }
        catch (UnsatisfiedLinkError e5) { Log.e(TAG, e5.getMessage()); }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "**** onCreate");

        Companion companion = Companion;
        companion.setActivity(this);
        instance = this;

        // ВАЖНО: super.onCreate() ДОЛЖЕН быть вызван всегда, иначе SuperNotCalledException
        super.onCreate(savedInstanceState);

        // ========= CACHE GATE (после super) =========
        if (!CachePaths.INSTANCE.cacheExists(this)) {
            try {
                Intent i = new Intent(this, CacheLoaderActivity.class);
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
                startActivity(i);
            } catch (Throwable t) {
                Log.e(TAG, "CacheLoaderActivity start failed: " + t.getMessage());
            }
            finish();
            return;
        }

        // =============================
        // дальше обычный запуск только если кеш есть
        // =============================

        soundPool = new SoundPool.Builder()
                .setMaxStreams(8)
                .setAudioAttributes(
                        new AudioAttributes.Builder()
                                .setUsage(AudioAttributes.USAGE_GAME)
                                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                                .build()
                )
                .build();

        mBackUILayout = findViewById(R.id.ui_layout);
        mMainMenuRoot = findViewById(R.id.main_fl_root);

        mMainScreen = new MainScreen(this);

        mChat = new Chat(this);
        mHudManager = new HudManager(this);
        mSpeedometr = new Speedometr(this);
        mMinerIndicator = new MinerIndicatorManager(this);

        GUIManager.setup(this, mBackUILayout, mMainMenuRoot);

        showLoadingScreen();
        updateLoadingScreen();

        Autorization.nativeInit();
        SpawnSelectNative.nativeInit(this);
        com.multirium.mobile.gui.notify.CustomNotify.init();
        com.multirium.mobile.gui.dialog.GtaDialog.init();
        RegistrationNative.nativeInit(this);

        try {
            initializeSAMP();
        } catch (UnsatisfiedLinkError e5) {
            Log.e(TAG, e5.getMessage());
        }
    }

    private native void initializeSAMP();

    @Override public void onStart() { super.onStart(); }
    @Override public void onRestart() { super.onRestart(); }
    @Override public void onResume() { super.onResume(); }

    public native void onEventBackPressed();

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        onEventBackPressed();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            onEventBackPressed();
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override public void onPause() { super.onPause(); }
    @Override public void onStop() { super.onStop(); }

    @Override
    public void onDestroy() {
        Companion.nativeDisconnect();
        super.onDestroy();
    }

    public void testMinerIndicatorCmd() {
        if (mMinerIndicator == null) return;
        mMinerIndicator.update(true, true, 12, 3);
    }

    @Override
    public void onHeightChanged(int orientation, int height) { }

    public static final class Companion {
        public static final Companion INSTANCE = new Companion();
        private Companion() { }
        public final native void nativeDisconnect();

        public final AppCompatActivity getActivity() {
            AppCompatActivity appCompatActivity = activity;
            if (appCompatActivity != null) return appCompatActivity;
            Intrinsics.throwUninitializedPropertyAccessException("activity");
            return null;
        }

        public final void setActivity(AppCompatActivity appCompatActivity) {
            Intrinsics.checkNotNullParameter(appCompatActivity, "<set-?>");
            activity = appCompatActivity;
        }
    }

    public void pushHudChat(int type, String text) {
        if (mHudManager == null) return;
        mHudManager.pushChatLine(type, text);
    }

    public void setOldChatVisible(boolean visible) {
        if (mHudManager == null) return;
        mHudManager.setOldChatVisible(visible);
    }

    public void setNewChatVisible(boolean visible) {
        if (mHudManager == null) return;
        mHudManager.setNewChatVisible(visible);
    }
}
