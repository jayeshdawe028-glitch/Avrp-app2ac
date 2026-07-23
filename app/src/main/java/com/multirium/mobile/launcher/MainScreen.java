package com.multirium.mobile.launcher;

import android.animation.TimeInterpolator;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.LinearGradient;
import android.graphics.Shader;
import android.media.AudioAttributes;
import android.media.SoundPool;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemClock;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.view.Choreographer;
import android.view.HapticFeedbackConstants;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.content.res.AppCompatResources;
import androidx.constraintlayout.widget.ConstraintLayout;
import androidx.core.content.FileProvider;

import com.multirium.mobile.R;
import com.multirium.mobile.launcher.telegram.TgBindActivity;
import com.multirium.mobile.launcher.util.Util;

import org.ini4j.Wini;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.Charset;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.Locale;

public class MainScreen {

    private final Activity activity;

    // ==== natives ====
    native void startConnect();
    native void setClientNick(String nick);
    native void setLauncherSkin(int skinId);
    native String getClientNick();

    public ConstraintLayout mMainLayout;

    // Create Character overlay (keeps in-game background)
    private CreateCharacterController createCharacterController;

    // ===== Views =====
    private View btnTg, btnVk, btnSupport;
    private View btnBalancePlus;
    private TextView tvVipTitle;

    private View launcherMainCurrentServer;
    private TextView launcherMainCurrentServerNum;
    private TextView launcherMainCurrentServerName;
    private TextView launcherMainCurrentServerOnlineInfo;
    private ProgressBar launcherMainCurrentServerProgress;

    private View btnPlay;

    private View tvLoginBadge;
    private TextView tvLoginValue;
    private TextView tvCharacterNick;

    private TextView tvBalanceValue;

    // ONLINE мониторинг (badge)
    private TextView tvOnlineCount;

    // badge x3 pulse
    private TextView badgeX3;
    private boolean x3PulseOn = false;

    // VIP / Admin chips
    private View launcherVipCard;
    private TextView tvVipDays;
    private TextView chipAdmin;

    // Nick overlay
    private View nickOverlay;
    private View nickCard;
    private View btnNickClose, btnNickSave, btnNickSendLogs;
    private View btnLogout;

    private TextView etNickFirst, etNickLast;

    // Slots
    private View btnChangeCharacter;
    private View slot1, slot2, slot3;
    private TextView slot1Nick, slot2Nick, slot3Nick;
    private android.widget.ImageView slot1Bind, slot2Bind, slot3Bind;
    private View slot1Clear, slot2Clear, slot3Clear;
    private boolean slotsOpened = false;
    // Slot delete confirm overlay (mini, forced anims even when system animations disabled)
    private View deleteOverlay;
    private View deleteCard;
    private TextView deleteTitle;
    private TextView deleteDesc;
    private View deleteCancel;
    private View deleteConfirm;
    private int pendingDeleteSlot = -1;



    // ===== Force anim (ВСЕГДА ВКЛ) =====
    private final Handler ui = new Handler(Looper.getMainLooper());

    // ===== Click sound (raw/click.mp3) =====
    private SoundPool soundPool;
    private int clickSoundId = 0;
    private volatile boolean clickSoundLoaded = false;

    // ===== URLs =====
    private static final String ACCESS_URL = "https://wh27264.web2.maze-tech.ru/launcher_access.php";
    private static final String PLAYER_INFO_URL = "https://wh27264.web2.maze-tech.ru/player_info.php";
    private static final String STATUS_URL = "https://wh27264.web2.maze-tech.ru/cache/status.php";
    private static final String CREATE_CHARACTER_URL = "https://wh27264.web2.maze-tech.ru/create_character.php";

    private static final String TG_GROUP_URL = "https://t.me/volya_off";
    private static final String VK_URL = "https://vk.com/";
    private static final String SUPPORT_URL = "https://example.com/";
    private static final String TOPUP_URL = "https://example.com/donate";

    // ===== Slots prefs =====
    private static final String PREFS_LAUNCHER = "launcher_prefs";
    private static final String KEY_SELECTED_SLOT = "selected_slot";
    private static final String KEY_NICK_1 = "nick_1";
    private static final String KEY_NICK_2 = "nick_2";
    private static final String KEY_NICK_3 = "nick_3";
    private static final String KEY_SKIN_1 = "skin_1";
    private static final String KEY_SKIN_2 = "skin_2";
    private static final String KEY_SKIN_3 = "skin_3";

    // bind status
    private static final String KEY_BIND_1 = "bind_1";
    private static final String KEY_BIND_2 = "bind_2";
    private static final String KEY_BIND_3 = "bind_3";
    private static final String KEY_BIND_CHECKING_1 = "bind_check_1";
    private static final String KEY_BIND_CHECKING_2 = "bind_check_2";
    private static final String KEY_BIND_CHECKING_3 = "bind_check_3";

    private static final String DEFAULT_NICK = "Nick_Name";
    private static final int DEFAULT_SKIN = 94;

    private SharedPreferences launcherPrefs;
    private int selectedSlot = 1;

    private final DecimalFormat moneyFmt;

    // ===== Logs =====
    private static final String TAG = "LauncherBind";

    // ===== Online monitor =====
    private final Runnable onlineTick = new Runnable() {
        @Override public void run() {
            fetchOnlineOnce();
            ui.postDelayed(this, 10_000); // 10 сек
        }
    };
    private int lastOnline = -1;
    private int lastMax = -1;

    // play pulse
    private boolean playPulseOn = false;

    public MainScreen(Activity activity) {
        this.activity = activity;

        ConstraintLayout layout = (ConstraintLayout) activity.getLayoutInflater()
                .inflate(R.layout.launcher_main, null);
        activity.addContentView(layout, new ConstraintLayout.LayoutParams(-1, -1));
        mMainLayout = layout;

        DecimalFormatSymbols sym = new DecimalFormatSymbols(Locale.US);
        sym.setGroupingSeparator(' ');
        moneyFmt = new DecimalFormat("#,###", sym);

        bindViews();

        // Create character overlay controller (does NOT replace game scene; only UI)
        createCharacterController = new CreateCharacterController(activity, mMainLayout, new CreateCharacterController.Host() {
            @Override public void playClickSound() { MainScreen.this.playClickSound(); }
            @Override public void buttonClick(View v) { MainScreen.this.buttonClick(v); }
            @Override public void hapticTick(View v) { MainScreen.this.hapticTick(v); }

            @Override public void forcedAnim(long durationMs, CreateCharacterController.FloatUpdate update, CreateCharacterController.EndCb end) {
                MainScreen.this.forcedAnim(durationMs, update::update, end::end);
            }

            @Override public void setLauncherSkin(int skinId) { MainScreen.this.setLauncherSkin(skinId); }

            @Override public boolean isSlotEmpty(int slot) { return MainScreen.this.isSlotEmpty(slot); }

            @Override public String getSlotNick(int slot) { return MainScreen.this.getSlotNick(slot); }
            @Override public void setSlotNick(int slot, String nick) { MainScreen.this.setSlotNick(slot, nick); }

            @Override public int getSlotSkin(int slot) { return MainScreen.this.getSlotSkin(slot); }
            @Override public void setSlotSkin(int slot, int skinId) { MainScreen.this.setSlotSkin(slot, skinId); }

            @Override public void setBindStatus(int slot, boolean bound) { MainScreen.this.setBindStatus(slot, bound); }

            @Override public void ensureUniqueNicks() { MainScreen.this.ensureUniqueNicks(); }
            @Override public void updateSlotsListUI() { MainScreen.this.updateSlotsListUI(); }
            @Override public void applySelectedSlotToUIAndClient(boolean fromUser) { MainScreen.this.applySelectedSlotToUIAndClient(fromUser); }

            @Override public void requestCreateCharacter(int slot, String nick, int skin, int sex, CreateCharacterController.CreateCb cb) {
                MainScreen.this.requestCreateCharacter(slot, nick, skin, sex, cb);
            }
            @Override public void checkNickExists(String nick, CreateCharacterController.NickCheckCb cb) {
                MainScreen.this.checkNickExistsAsync(nick, cb);
            }
            @Override public void toast(String text) { MainScreen.this.toast(text); }
            @Override public void hideSystemUI() { MainScreen.this.hideSystemUI(); }
        });

        hideSystemUI();

        // init click sound once
        initClickSound();

        initLauncherUI();

        Util.HideLayout(this.mMainLayout, false);
    }

    /** Вызови это из Activity.onDestroy() чтобы не тек SoundPool */
    public void releaseSounds() {
        try {
            if (soundPool != null) {
                soundPool.release();
                soundPool = null;
            }
        } catch (Throwable ignored) {}
        clickSoundId = 0;
        clickSoundLoaded = false;
    }

    public void show() {
        Util.ShowLayout(this.mMainLayout, false);
        hideSystemUI();

        // мониторинг онлайна
        startOnlineMonitoring();

        // пульс на PLAY
        pulsePlayButton(true);

        // пульс на x3 (если есть)
        startX3Pulse();
    }

    // =========================================================
    // ===================== INIT / BIND =======================
    // =========================================================

    private void bindViews() {
        btnTg = mMainLayout.findViewById(R.id.btnTg);
        btnVk = mMainLayout.findViewById(R.id.btnVk);
        btnSupport = mMainLayout.findViewById(R.id.btnSupport);

        btnBalancePlus = mMainLayout.findViewById(R.id.btnBalancePlus);
        tvBalanceValue = mMainLayout.findViewById(R.id.tvBalanceValue);

        launcherVipCard = mMainLayout.findViewById(R.id.launcherVipCard);
        tvVipDays = mMainLayout.findViewById(R.id.tvVipDays);
        chipAdmin = mMainLayout.findViewById(R.id.chipAdmin);
        tvVipTitle = mMainLayout.findViewById(R.id.tvVipTitle);
        btnLogout = mMainLayout.findViewById(R.id.btnLogout);

        launcherMainCurrentServer = mMainLayout.findViewById(R.id.launcherMainCurrentServer);
        launcherMainCurrentServerNum = mMainLayout.findViewById(R.id.launcherMainCurrentServerNum);
        launcherMainCurrentServerName = mMainLayout.findViewById(R.id.launcherMainCurrentServerName);
        launcherMainCurrentServerOnlineInfo = mMainLayout.findViewById(R.id.launcher_main_current_server_online_info);
        launcherMainCurrentServerProgress = mMainLayout.findViewById(R.id.launcherMainCurrentServerProgress);

        btnPlay = mMainLayout.findViewById(R.id.btn_play);

        tvLoginBadge = mMainLayout.findViewById(R.id.tvLoginBadge);
        tvLoginValue = mMainLayout.findViewById(R.id.tvLoginValue);
        tvCharacterNick = mMainLayout.findViewById(R.id.tvCharacterNick);

        nickOverlay = mMainLayout.findViewById(R.id.nickOverlay);
        nickCard = mMainLayout.findViewById(R.id.nickCard);

        btnNickClose = mMainLayout.findViewById(R.id.btnNickClose);
        btnNickSave = mMainLayout.findViewById(R.id.btnNickSave);
        btnNickSendLogs = mMainLayout.findViewById(R.id.btnNickSendLogs);

        etNickFirst = mMainLayout.findViewById(R.id.etNickFirst);
        etNickLast = mMainLayout.findViewById(R.id.etNickLast);

        btnChangeCharacter = mMainLayout.findViewById(R.id.btnChangeCharacter);
        slot1 = mMainLayout.findViewById(R.id.slot1);
        slot2 = mMainLayout.findViewById(R.id.slot2);
        slot3 = mMainLayout.findViewById(R.id.slot3);

        slot1Clear = mMainLayout.findViewById(R.id.slot1Clear);
        slot2Clear = mMainLayout.findViewById(R.id.slot2Clear);
        slot3Clear = mMainLayout.findViewById(R.id.slot3Clear);

        slot1Nick = mMainLayout.findViewById(R.id.slot1Nick);
        slot2Nick = mMainLayout.findViewById(R.id.slot2Nick);
        slot3Nick = mMainLayout.findViewById(R.id.slot3Nick);

        slot1Bind = mMainLayout.findViewById(R.id.slot1Bind);
        slot2Bind = mMainLayout.findViewById(R.id.slot2Bind);
        slot3Bind = mMainLayout.findViewById(R.id.slot3Bind);


        // online badge & x3 badge
        tvOnlineCount = mMainLayout.findViewById(R.id.tvOnlineCount);
        badgeX3 = mMainLayout.findViewById(R.id.badgeX3);

        if (tvVipTitle != null) setVipTitleGold(tvVipTitle);

        // дефолт для online badge — как на скрине
        applyOnlineBadgeText(-1, -1, true);
    }

    private void initLauncherUI() {
        launcherPrefs = activity.getSharedPreferences(PREFS_LAUNCHER, Context.MODE_PRIVATE);

        String iniNick = safeNick(readNickFromIni());
        if (!TextUtils.isEmpty(iniNick)) {
            String s1 = getSlotNick(1);
            if (TextUtils.isEmpty(s1) || DEFAULT_NICK.equalsIgnoreCase(s1)) {
                setSlotNick(1, iniNick);
            }
        }

        ensureDefaults();

        selectedSlot = clampSlot(launcherPrefs.getInt(KEY_SELECTED_SLOT, 1));
        if (isSlotEmpty(selectedSlot)) {
            for (int i = 1; i <= 3; i++) {
                if (!isSlotEmpty(i)) { selectedSlot = i; break; }
            }
        }
        launcherPrefs.edit().putInt(KEY_SELECTED_SLOT, selectedSlot).apply();

        updateServerUI();
        hideNickOverlayInstant();
        forceHideSlotsInstant();

        applySelectedSlotToUIAndClient(false);
        forceCheckBindForSlot(selectedSlot, "init");

        // forced click anim + HAPTIC + SOUND на ВСЕ кнопки
        View.OnClickListener click = v -> {
            playClickSound();
            buttonClick(v);
            hapticTick(v);
        };

        if (btnLogout != null) btnLogout.setOnClickListener(v -> {
            click.onClick(v);
            logoutSelectedSlotFancy();
        });

        if (btnTg != null) btnTg.setOnClickListener(v -> { click.onClick(v); openUrl(TG_GROUP_URL); });
        if (btnVk != null) btnVk.setOnClickListener(v -> { click.onClick(v); openUrl(VK_URL); });
        if (btnSupport != null) btnSupport.setOnClickListener(v -> { click.onClick(v); openUrl(SUPPORT_URL); });

        if (btnBalancePlus != null) btnBalancePlus.setOnClickListener(v -> {
            click.onClick(v);
            openUrl(TOPUP_URL);
        });

        if (launcherMainCurrentServer != null) launcherMainCurrentServer.setOnClickListener(v -> {
            click.onClick(v);
            openServerSelection();
        });

        if (btnPlay != null) btnPlay.setOnClickListener(v -> {
            click.onClick(v);
            onPlayPressed();
        });

        if (tvLoginBadge != null) tvLoginBadge.setOnClickListener(v -> {
            click.onClick(v);
            // Nick overlay only for empty slots (no rename when slots already occupied)
            if (!isSlotEmpty(selectedSlot)) {
                toast("Слот уже занят — смена ника отключена");
                return;
            }
            prefillNickInputsFromSelectedSlot();
            showNickOverlay();
        });

        if (btnNickClose != null) btnNickClose.setOnClickListener(v -> { click.onClick(v); hideNickOverlay(); });
        if (btnNickSave != null) btnNickSave.setOnClickListener(v -> { click.onClick(v); saveNickToSelectedSlot(); });
        if (btnNickSendLogs != null) btnNickSendLogs.setOnClickListener(v -> { click.onClick(v); sendLogFile(); });

        if (btnChangeCharacter != null) btnChangeCharacter.setOnClickListener(v -> { click.onClick(v); toggleSlots(); });

        if (slot1 != null) slot1.setOnClickListener(v -> { click.onClick(v); selectSlot(1, true); });
        if (slot2 != null) slot2.setOnClickListener(v -> { click.onClick(v); selectSlot(2, true); });
        if (slot3 != null) slot3.setOnClickListener(v -> { click.onClick(v); selectSlot(3, true); });

        // Clear buttons (remove slot by button, not by clicking slot row)
        if (slot1Clear != null) slot1Clear.setOnClickListener(v -> { click.onClick(v); onClearSlotPressed(1); });
        if (slot2Clear != null) slot2Clear.setOnClickListener(v -> { click.onClick(v); onClearSlotPressed(2); });
        if (slot3Clear != null) slot3Clear.setOnClickListener(v -> { click.onClick(v); onClearSlotPressed(3); });

        updateSlotsListUI();

        // мониторинг онлайна старт
        startOnlineMonitoring();
    }

    // =========================================================
    // ===================== CLICK SOUND =======================
    // =========================================================

    private void initClickSound() {
        try {
            if (soundPool != null) return;

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                AudioAttributes aa = new AudioAttributes.Builder()
                        .setUsage(AudioAttributes.USAGE_ASSISTANCE_SONIFICATION)
                        .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                        .build();

                soundPool = new SoundPool.Builder()
                        .setMaxStreams(4)
                        .setAudioAttributes(aa)
                        .build();
            } else {
                @SuppressWarnings("deprecation")
                SoundPool sp = new SoundPool(4, android.media.AudioManager.STREAM_MUSIC, 0);
                soundPool = sp;
            }

            clickSoundLoaded = false;
            clickSoundId = soundPool.load(activity, R.raw.click, 1);

            soundPool.setOnLoadCompleteListener((sp, sampleId, status) -> {
                if (status == 0 && sampleId == clickSoundId) {
                    clickSoundLoaded = true;
                }
            });
        } catch (Throwable ignored) {
            soundPool = null;
            clickSoundId = 0;
            clickSoundLoaded = false;
        }
    }

    private void playClickSound() {
        try {
            if (soundPool == null || clickSoundId == 0 || !clickSoundLoaded) return;

            float vol = 0.45f; // громкость 0..1
            soundPool.play(clickSoundId, vol, vol, 1, 0, 1.0f);
        } catch (Throwable ignored) {}
    }

    // =========================================================
    // ===================== ONLINE MONITOR ====================
    // =========================================================

    private void startOnlineMonitoring() {
        ui.removeCallbacks(onlineTick);
        ui.post(onlineTick);
    }

    private void stopOnlineMonitoring() {
        ui.removeCallbacks(onlineTick);
    }

    private void logoutSelectedSlotFancy() {
        final int slot = selectedSlot;

        // небольшая анимация “выхода”
        if (nickCard != null) {
            forcedAnim(140, f -> {
                float inv = 1f - f;
                nickCard.setScaleX(1f - 0.02f * f);
                nickCard.setScaleY(1f - 0.02f * f);
                nickCard.setAlpha(inv);
            }, () -> {
                // чистим слот
                clearSlot(slot);
                resetClientNickToDefault();

                // если удалили текущий — выберем следующий непустой, иначе останемся на этом
                int newSel = slot;
                if (isSlotEmpty(newSel)) {
                    for (int i = 1; i <= 3; i++) {
                        if (!isSlotEmpty(i)) { newSel = i; break; }
                    }
                }
                selectedSlot = clampSlot(newSel);
                launcherPrefs.edit().putInt(KEY_SELECTED_SLOT, selectedSlot).apply();

                // обновим UI + клиент
                applySelectedSlotToUIAndClient(false);
                updateSlotsListUI();

                // скрываем оверлей
                hideNickOverlayInstant();

                // если все пустые — сразу открыть ввод ника
                if (isSlotEmpty(selectedSlot)) {
                    prefillNickInputsFromSelectedSlot();
                    showNickOverlay();
                }

                // восстановим вид nickCard на будущее
                if (nickCard != null) {
                    nickCard.setScaleX(1f);
                    nickCard.setScaleY(1f);
                    nickCard.setAlpha(1f);
                }

                toast("Слот очищен");
            });
        } else {
            clearSlot(slot);
            applySelectedSlotToUIAndClient(false);
            updateSlotsListUI();
            hideNickOverlay();
            toast("Слот очищен");
        }
    }

    private void resetClientNickToDefault() {
        try { writeNickToIniCompat(DEFAULT_NICK); } catch (Throwable ignored) {}
        try { setClientNick(DEFAULT_NICK); } catch (Throwable ignored) {}
    }

    private void fetchOnlineOnce() {
        new Thread(() -> {
            try {
                String body = httpGet(STATUS_URL);
                JSONObject obj = new JSONObject(body);
                int online = obj.optInt("online", 0);
                int max = obj.optInt("max", 1000);

                ui.post(() -> applyOnlineBadgeText(online, max, false));
            } catch (Throwable ignored) {}
        }).start();
    }

    /**
     * ВАЖНО: отображение как на скрине:
     * 1 строка: "ОНЛ"
     * 2 строка: число (или "--")
     */
    @SuppressLint("SetTextI18n")
    private void applyOnlineBadgeText(int online, int max, boolean immediate) {
        if (tvOnlineCount == null) return;

        tvOnlineCount.setIncludeFontPadding(false);
        tvOnlineCount.setLineSpacing(0f, 1.0f);

        boolean unknown = (online < 0);
        if (max <= 0) max = 1000;

        if (!unknown) {
            if (online < 0) online = 0;
            if (online > max) online = max;
        }

        if (immediate || unknown || lastOnline < 0) {
            tvOnlineCount.setText(unknown ? "--" : String.valueOf(online));

            lastOnline = unknown ? -1 : online;
            lastMax = max;

            tvOnlineCount.setAlpha(1f);
            tvOnlineCount.setScaleX(1f);
            tvOnlineCount.setScaleY(1f);
            return;
        }

        final int from = lastOnline;
        final int to = online;

        lastOnline = online;
        lastMax = max;

        forcedAnim(280, f -> {
            int cur = (int) Math.round(from + (to - from) * f);
            tvOnlineCount.setText(String.valueOf(cur));

            float wave = (float) Math.sin(f * Math.PI);
            float s = 1f + 0.03f * wave;
            tvOnlineCount.setScaleX(s);
            tvOnlineCount.setScaleY(s);
            tvOnlineCount.setAlpha(0.90f + 0.10f * wave);
        }, () -> {
            tvOnlineCount.setScaleX(1f);
            tvOnlineCount.setScaleY(1f);
            tvOnlineCount.setAlpha(1f);
        });
    }

    // =========================================================
    // ===================== X3 BADGE PULSE ====================
    // =========================================================

    private void startX3Pulse() {
        if (badgeX3 == null) return;
        if (x3PulseOn) return;
        x3PulseOn = true;
        pulseLoopBadge(badgeX3);
    }

    private void stopX3Pulse() {
        x3PulseOn = false;
        if (badgeX3 != null) {
            badgeX3.setScaleX(1f);
            badgeX3.setScaleY(1f);
            badgeX3.setAlpha(1f);
        }
    }

    private void pulseLoopBadge(View v) {
        if (!x3PulseOn || v == null) return;

        forcedAnim(1000, f -> {
            float wave = (float) Math.sin(f * Math.PI); // 0..1..0
            float s = 1f + 0.04f * wave;
            v.setScaleX(s);
            v.setScaleY(s);
            v.setAlpha(0.86f + 0.14f * wave);
        }, () -> pulseLoopBadge(v));
    }

    // =========================================================
    // ===================== BIND STATUS =======================
    // =========================================================

    private String bindKey(int slot) {
        slot = clampSlot(slot);
        return (slot == 1) ? KEY_BIND_1 : (slot == 2) ? KEY_BIND_2 : KEY_BIND_3;
    }

    private String bindCheckingKey(int slot) {
        slot = clampSlot(slot);
        return (slot == 1) ? KEY_BIND_CHECKING_1 : (slot == 2) ? KEY_BIND_CHECKING_2 : KEY_BIND_CHECKING_3;
    }

    private void setBindChecking(int slot, boolean checking) {
        launcherPrefs.edit().putBoolean(bindCheckingKey(slot), checking).apply();
    }

    private boolean isBindChecking(int slot) {
        return launcherPrefs.getBoolean(bindCheckingKey(slot), false);
    }

    private void setBindStatus(int slot, boolean bound) {
        launcherPrefs.edit().putBoolean(bindKey(slot), bound).apply();
    }

    private boolean getBindStatus(int slot) {
        return launcherPrefs.getBoolean(bindKey(slot), false);
    }

    private String formatSlotTitle(int slot, String nickRaw) {
        String nick = TextUtils.isEmpty(nickRaw) ? DEFAULT_NICK : nickRaw;

        if (!isRealNick(nick)) return "Свободно";
        if (isBindChecking(slot)) return nick + "  ·  Проверка…";

        boolean bound = getBindStatus(slot);
        return nick + (bound ? "  ·  Привязан" : "  ·  Не привязан");
    }

    private void logBind(String msg) { Log.i(TAG, msg); }

    // =========================================================
    // ===================== SLOTS (LOGIC) =====================
    // =========================================================

    private int clampSlot(int slot) { return Math.max(1, Math.min(3, slot)); }

    private boolean isSlotEmpty(int slot) {
        String n = getSlotNick(slot);
        return TextUtils.isEmpty(n) || DEFAULT_NICK.equalsIgnoreCase(n);
    }

    private int findFirstFreeSlot() {
        for (int i = 1; i <= 3; i++) if (isSlotEmpty(i)) return i;
        return -1;
    }

    private String getSlotNick(int slot) {
        slot = clampSlot(slot);
        if (slot == 1) return launcherPrefs.getString(KEY_NICK_1, DEFAULT_NICK);
        if (slot == 2) return launcherPrefs.getString(KEY_NICK_2, DEFAULT_NICK);
        return launcherPrefs.getString(KEY_NICK_3, DEFAULT_NICK);
    }

    private int getSlotSkin(int slot) {
        slot = clampSlot(slot);
        if (slot == 1) return launcherPrefs.getInt(KEY_SKIN_1, DEFAULT_SKIN);
        if (slot == 2) return launcherPrefs.getInt(KEY_SKIN_2, DEFAULT_SKIN);
        return launcherPrefs.getInt(KEY_SKIN_3, DEFAULT_SKIN);
    }

    private void setSlotNick(int slot, String nick) {
        slot = clampSlot(slot);
        String v = safeNick(nick);
        if (TextUtils.isEmpty(v)) v = DEFAULT_NICK;

        SharedPreferences.Editor ed = launcherPrefs.edit();
        if (slot == 1) ed.putString(KEY_NICK_1, v);
        else if (slot == 2) ed.putString(KEY_NICK_2, v);
        else ed.putString(KEY_NICK_3, v);
        ed.apply();
    }

    private void setSlotSkin(int slot, int skinId) {
        slot = clampSlot(slot);
        int s = Math.max(0, Math.min(311, skinId));

        SharedPreferences.Editor ed = launcherPrefs.edit();
        if (slot == 1) ed.putInt(KEY_SKIN_1, s);
        else if (slot == 2) ed.putInt(KEY_SKIN_2, s);
        else ed.putInt(KEY_SKIN_3, s);
        ed.apply();
    }

    private void ensureDefaults() {
        if (!launcherPrefs.contains(KEY_NICK_1)) launcherPrefs.edit().putString(KEY_NICK_1, DEFAULT_NICK).apply();
        if (!launcherPrefs.contains(KEY_NICK_2)) launcherPrefs.edit().putString(KEY_NICK_2, DEFAULT_NICK).apply();
        if (!launcherPrefs.contains(KEY_NICK_3)) launcherPrefs.edit().putString(KEY_NICK_3, DEFAULT_NICK).apply();

        if (!launcherPrefs.contains(KEY_SKIN_1)) launcherPrefs.edit().putInt(KEY_SKIN_1, DEFAULT_SKIN).apply();
        if (!launcherPrefs.contains(KEY_SKIN_2)) launcherPrefs.edit().putInt(KEY_SKIN_2, DEFAULT_SKIN).apply();
        if (!launcherPrefs.contains(KEY_SKIN_3)) launcherPrefs.edit().putInt(KEY_SKIN_3, DEFAULT_SKIN).apply();

        if (!launcherPrefs.contains(KEY_BIND_1)) launcherPrefs.edit().putBoolean(KEY_BIND_1, false).apply();
        if (!launcherPrefs.contains(KEY_BIND_2)) launcherPrefs.edit().putBoolean(KEY_BIND_2, false).apply();
        if (!launcherPrefs.contains(KEY_BIND_3)) launcherPrefs.edit().putBoolean(KEY_BIND_3, false).apply();
        if (!launcherPrefs.contains(KEY_BIND_CHECKING_1)) launcherPrefs.edit().putBoolean(KEY_BIND_CHECKING_1, false).apply();
        if (!launcherPrefs.contains(KEY_BIND_CHECKING_2)) launcherPrefs.edit().putBoolean(KEY_BIND_CHECKING_2, false).apply();
        if (!launcherPrefs.contains(KEY_BIND_CHECKING_3)) launcherPrefs.edit().putBoolean(KEY_BIND_CHECKING_3, false).apply();

        // убираем дубли ников
        String n1 = safeNick(getSlotNick(1));
        String n2 = safeNick(getSlotNick(2));
        String n3 = safeNick(getSlotNick(3));
        if (!TextUtils.isEmpty(n1) && !DEFAULT_NICK.equalsIgnoreCase(n1)) {
            if (n1.equalsIgnoreCase(n2)) setSlotNick(2, DEFAULT_NICK);
            if (n1.equalsIgnoreCase(n3)) setSlotNick(3, DEFAULT_NICK);
        }
        if (!TextUtils.isEmpty(n2) && !DEFAULT_NICK.equalsIgnoreCase(n2)) {
            if (n2.equalsIgnoreCase(n3)) setSlotNick(3, DEFAULT_NICK);
        }
    }

    private void ensureUniqueNicks() {
        String n1 = safeNick(getSlotNick(1));
        String n2 = safeNick(getSlotNick(2));
        String n3 = safeNick(getSlotNick(3));

        if (!isRealNick(n1)) n1 = "";
        if (!isRealNick(n2)) n2 = "";
        if (!isRealNick(n3)) n3 = "";

        if (!TextUtils.isEmpty(n1)) {
            if (n1.equalsIgnoreCase(n2)) setSlotNick(2, DEFAULT_NICK);
            if (n1.equalsIgnoreCase(n3)) setSlotNick(3, DEFAULT_NICK);
        }
        if (!TextUtils.isEmpty(n2)) {
            if (n2.equalsIgnoreCase(n3)) setSlotNick(3, DEFAULT_NICK);
        }
    }

    private void selectSlot(int slot, boolean closeDropdown) {
        slot = clampSlot(slot);
        selectedSlot = slot;
        launcherPrefs.edit().putInt(KEY_SELECTED_SLOT, selectedSlot).apply();

        applySelectedSlotToUIAndClient(true);
        forceCheckBindForSlot(selectedSlot, "slot_switch");
        updateSlotsListUI();

        if (closeDropdown && slotsOpened) {
            slotsOpened = false;
            hideSlotsAnimated();
        }
    }

    @SuppressLint("SetTextI18n")
    private void updateSlotsListUI() {
        String n1 = getSlotNick(1);
        String n2 = getSlotNick(2);
        String n3 = getSlotNick(3);

        // NICK label: when empty -> "Свободно"
        if (slot1Nick != null) slot1Nick.setText(getSlotNickLabel(1, n1));
        if (slot2Nick != null) slot2Nick.setText(getSlotNickLabel(2, n2));
        if (slot3Nick != null) slot3Nick.setText(getSlotNickLabel(3, n3));

        // Bind label: shown only when slot has real nick
        applyBindLabel(slot1Bind, 1, n1);
        applyBindLabel(slot2Bind, 2, n2);
        applyBindLabel(slot3Bind, 3, n3);


        // Clear button visibility (only if slot has real nick)
        if (slot1Clear != null) slot1Clear.setVisibility(isRealNick(n1) ? View.VISIBLE : View.INVISIBLE);
        if (slot2Clear != null) slot2Clear.setVisibility(isRealNick(n2) ? View.VISIBLE : View.INVISIBLE);
        if (slot3Clear != null) slot3Clear.setVisibility(isRealNick(n3) ? View.VISIBLE : View.INVISIBLE);

        setSlotRowState(slot1, selectedSlot == 1);
        setSlotRowState(slot2, selectedSlot == 2);
        setSlotRowState(slot3, selectedSlot == 3);
    }

    private void setSlotRowState(View row, boolean selected) {
        if (row == null) return;
        row.setEnabled(!selected);
        row.setAlpha(selected ? 0.55f : 1.0f);
    }

    private String getSlotNickLabel(int slot, String nickRaw) {
        String nick = TextUtils.isEmpty(nickRaw) ? DEFAULT_NICK : nickRaw;
        return isRealNick(nick) ? nick : "Свободно";
    }

    private void applyBindLabel(android.widget.ImageView iv, int slot, String nickRaw) {
        if (iv == null) return;

        String nick = TextUtils.isEmpty(nickRaw) ? DEFAULT_NICK : nickRaw;

        // если слот пустой — ничего не показываем
        if (!isRealNick(nick)) {
            iv.setVisibility(View.GONE);
            return;
        }

        // если идет проверка — тоже скрываем (или можешь показывать "часики", если захочешь)
        if (isBindChecking(slot)) {
            iv.setVisibility(View.GONE);
            return;
        }

        boolean bound = getBindStatus(slot);

        // показываем ТОЛЬКО если привязан
        iv.setVisibility(bound ? View.VISIBLE : View.GONE);

        // если хочешь чуть приятнее — легкая анимация появления (forcedAnim)
        if (bound && iv.getAlpha() < 1f) {
            iv.setAlpha(0f);
            forcedAnim(140, f -> iv.setAlpha(f), null);
        }
    }



    // =========================================================
    // =================== SLOT CLEAR (BUTTON) =================
    // =========================================================
    private void onClearSlotPressed(int slot) {
        final String nick = getSlotNick(slot); // как у тебя уже сделано

        // если слот пустой — не даём очистку
        if (!isRealNick(nick)) {
            toast("Слот №" + slot + " уже пуст");
            return;
        }

        final int finalSlot = slot;

        showSlotDeleteOverlay(finalSlot, nick);
    }




    // =========================================================
    // ============ MINI OVERLAY: CONFIRM SLOT DELETE ===========
    // =========================================================

    private void ensureSlotDeleteOverlay() {
        if (deleteOverlay != null) return;
        if (activity == null || mMainLayout == null) return;

        try {
            ViewGroup root = (ViewGroup) mMainLayout;

            deleteOverlay = activity.getLayoutInflater().inflate(R.layout.overlay_slot_delete_confirm, root, false);
            deleteOverlay.setVisibility(View.GONE);
            deleteOverlay.setAlpha(0f);

            deleteCard = deleteOverlay.findViewById(R.id.deleteCard);
            deleteTitle = deleteOverlay.findViewById(R.id.deleteTitle);
            deleteDesc = deleteOverlay.findViewById(R.id.deleteDesc);
            deleteCancel = deleteOverlay.findViewById(R.id.deleteCancel);
            deleteConfirm = deleteOverlay.findViewById(R.id.deleteConfirm);

            // click outside -> close
            deleteOverlay.setOnClickListener(v -> dismissSlotDeleteOverlay(null));
            if (deleteCard != null) deleteCard.setOnClickListener(v -> { /* consume */ });

            if (deleteCancel != null) {
                deleteCancel.setOnClickListener(v -> {
                    playClickSound();
                    buttonClick(v);
                    hapticTick(v);
                    pendingDeleteSlot = -1;
                    dismissSlotDeleteOverlay(null);
                });
            }

            if (deleteConfirm != null) {
                deleteConfirm.setOnClickListener(v -> {
                    playClickSound();
                    buttonClick(v);
                    hapticTick(v);
                    final int s = pendingDeleteSlot;
                    pendingDeleteSlot = -1;
                    dismissSlotDeleteOverlay(() -> {
                        if (s > 0) clearSlot(s);
                    });
                });
            }

            root.addView(deleteOverlay);
        } catch (Throwable t) {
            deleteOverlay = null;
            deleteCard = null;
            deleteTitle = null;
            deleteDesc = null;
            deleteCancel = null;
            deleteConfirm = null;
        }
    }

    private void showSlotDeleteOverlay(int slot, String nick) {
        ensureSlotDeleteOverlay();

        // fallback (if layout missing)
        if (deleteOverlay == null) {
            showConfirmDialog(
                    "Подтверждение",
                    "Очистить слот №" + slot + "?\n" +
                            "Ник: " + nick + "\n\n" +
                            "Слот будет очищен в лаунчере.\n" +
                            "Персонаж на сервере не удаляется.",
                    "Удалить",
                    "Отмена",
                    () -> clearSlot(slot),
                    null
            );
            return;
        }

        pendingDeleteSlot = slot;

        if (deleteTitle != null) deleteTitle.setText("Очистить слот №" + slot + "?");
        if (deleteDesc != null) {
            deleteDesc.setText(
                    "Ник: " + nick + "\n\n" +
                            "Слот в лаунчере будет очищен.\n" +
                            "Персонаж на сервере не удаляется."
            );

        }

        deleteOverlay.setVisibility(View.VISIBLE);
        deleteOverlay.setAlpha(0f);

        if (deleteCard != null) {
            deleteCard.setScaleX(0.94f);
            deleteCard.setScaleY(0.94f);
            deleteCard.setTranslationY(18f);
        }

        forcedAnim(200, f -> {
            if (deleteOverlay != null) deleteOverlay.setAlpha(f);
            if (deleteCard != null) {
                float s = 0.94f + (1f - 0.94f) * f;
                deleteCard.setScaleX(s);
                deleteCard.setScaleY(s);
                deleteCard.setTranslationY(18f * (1f - f));
            }
        }, null);
    }

    private void dismissSlotDeleteOverlay(Runnable after) {
        if (deleteOverlay == null) {
            if (after != null) after.run();
            return;
        }

        final float a0 = deleteOverlay.getAlpha();
        final float y0 = (deleteCard != null) ? deleteCard.getTranslationY() : 0f;
        final float s0 = (deleteCard != null) ? deleteCard.getScaleX() : 1f;

        forcedAnim(170, f -> {
            float inv = 1f - f;
            if (deleteOverlay != null) deleteOverlay.setAlpha(a0 * inv);
            if (deleteCard != null) {
                float s = s0 + (0.94f - s0) * f;
                deleteCard.setScaleX(s);
                deleteCard.setScaleY(s);
                deleteCard.setTranslationY(y0 + 18f * f);
            }
        }, () -> {
            try {
                if (deleteOverlay != null) {
                    deleteOverlay.setVisibility(View.GONE);
                    deleteOverlay.setAlpha(0f);
                }
                if (deleteCard != null) {
                    deleteCard.setScaleX(1f);
                    deleteCard.setScaleY(1f);
                    deleteCard.setTranslationY(0f);
                }
            } catch (Throwable ignored) {}

            if (after != null) after.run();
        });
    }

    private void showConfirmDialog(String caption, String content, String leftText, String rightText,
                                   Runnable onLeft, Runnable onRight) {
        if (activity == null) return;

        final android.app.Dialog d = new android.app.Dialog(activity, android.R.style.Theme_Translucent_NoTitleBar);
        d.setCancelable(true);
        d.setCanceledOnTouchOutside(true);

        try {
            d.setContentView(R.layout.dialog_msgbox);

            TextView tvCaption = d.findViewById(R.id.caption);
            TextView tvContent = d.findViewById(R.id.contentText);

            View leftBtn = d.findViewById(R.id.leftButton);
            View rightBtn = d.findViewById(R.id.rightButton);

            TextView tvLeft = d.findViewById(R.id.leftButtText);
            TextView tvRight = d.findViewById(R.id.rightButtText);

            if (tvCaption != null) tvCaption.setText(caption);
            if (tvContent != null) tvContent.setText(content);

            if (tvLeft != null) tvLeft.setText(leftText);
            if (leftBtn != null) leftBtn.setOnClickListener(v -> {
                try { if (onLeft != null) onLeft.run(); } catch (Throwable ignored) {}
                try { d.dismiss(); } catch (Throwable ignored) {}
            });

            if (TextUtils.isEmpty(rightText)) {
                if (rightBtn != null) rightBtn.setVisibility(View.GONE);
            } else {
                if (tvRight != null) tvRight.setText(rightText);
                if (rightBtn != null) rightBtn.setVisibility(View.VISIBLE);
                rightBtn.setOnClickListener(v -> {
                    try { if (onRight != null) onRight.run(); } catch (Throwable ignored) {}
                    try { d.dismiss(); } catch (Throwable ignored) {}
                });
            }

            d.show();
        } catch (Throwable t) {
            // Fallback: system AlertDialog if layout missing
            try {
                new android.app.AlertDialog.Builder(activity)
                        .setTitle(caption)
                        .setMessage(content)
                        .setNegativeButton(rightText, null)
                        .setPositiveButton(leftText, (dd, which) -> { if (onLeft != null) onLeft.run(); })
                        .show();
            } catch (Throwable ignored) {}
        }
    }

    private void clearSlot(int slot) {
        slot = clampSlot(slot);

        setSlotNick(slot, DEFAULT_NICK);
        setSlotSkin(slot, DEFAULT_SKIN);
        setBindChecking(slot, false);
        setBindStatus(slot, false);

        ensureUniqueNicks();

        if (selectedSlot == slot) {
            applySelectedSlotToUIAndClient(true);
        }

        updateSlotsListUI();
        toast("Слот №" + slot + " очищен");
    }


    // =========================================================
    // ===================== APPLY SLOT ========================
    // =========================================================

    @SuppressLint("SetTextI18n")
    private void applySelectedSlotToUIAndClient(boolean fromUser) {
        final String desiredNick = safeNick(getSlotNick(selectedSlot));
        final int skin = getSlotSkin(selectedSlot);

        if (tvLoginValue != null) tvLoginValue.setText(desiredNick);
        if (tvCharacterNick != null) tvCharacterNick.setText(desiredNick.toUpperCase(Locale.ROOT));

        // sync into client/ini even if it's DEFAULT_NICK (user wants full reset on clear)
        syncNickToClientAndIniAllowDefault(desiredNick);

        final String actualNick = getClientNickSafe(desiredNick);

        if (isRealNick(actualNick) && !actualNick.equalsIgnoreCase(desiredNick)) {
            setSlotNick(selectedSlot, actualNick);
            ensureUniqueNicks();
            if (tvLoginValue != null) tvLoginValue.setText(actualNick);
            if (tvCharacterNick != null) tvCharacterNick.setText(actualNick.toUpperCase(Locale.ROOT));
            syncNickToClientAndIni(actualNick);
        }

        try { setLauncherSkin(skin); } catch (Throwable ignored) {}

        // Эти методы у тебя ниже по файлу (оставь как было в твоём полном файле)
        if (isRealNick(actualNick)) fetchPlayerInfoForNick(selectedSlot, actualNick);
        else applyPlayerInfoToUI(null);

        if (fromUser && !isRealNick(actualNick)) {
            // free slot -> open character creation overlay (keeps GTA/ped background)
            if (createCharacterController != null) {
                createCharacterController.show(selectedSlot);
            } else {
                toast("Слот пустой — создайте персонажа");
                prefillNickInputsFromSelectedSlot();
                showNickOverlay();
            }
        }
    }
    // Called from CreateCharacterController when user finished creating a new character

    // Called from CreateCharacterController: create character on server, then persist slot only after OK
    private void requestCreateCharacter(int slot, String nick, int skin, int sex, CreateCharacterController.CreateCb cb) {
        final int s = clampSlot(slot);
        final String n = safeNick(nick);
        final int sk = skin;
        final int sx = sex;

        new Thread(() -> {
            try {
                String dev = getDeviceIdSafe();
                String model = safeStr(Build.MODEL);
                String sdk = String.valueOf(Build.VERSION.SDK_INT);

                String url = CREATE_CHARACTER_URL
                        + "?name=" + Uri.encode(n)
                        + "&skin=" + Uri.encode(String.valueOf(sk))
                        + "&sex=" + Uri.encode(String.valueOf(sx))
                        + "&slot=" + Uri.encode(String.valueOf(s))
                        + "&device=" + Uri.encode(dev)
                        + "&model=" + Uri.encode(model)
                        + "&sdk=" + Uri.encode(sdk);

                logBind("CREATE GET: " + url);

                String body = httpGet(url);
                JSONObject response = new JSONObject(body);

                boolean ok = response.optBoolean("ok", false);
                String err = response.optString("error", response.optString("reason", ""));
                String rNick = response.optString("name", response.optString("nick", n));
                int rSkin = response.optInt("skin", sk);

                if (!ok) throw new RuntimeException(err);

                ui.post(() -> {
                    selectedSlot = s;
                    launcherPrefs.edit().putInt(KEY_SELECTED_SLOT, selectedSlot).apply();

                    setSlotNick(s, rNick);
                    setSlotSkin(s, rSkin);
                    setBindStatus(s, false);
                    setBindChecking(s, false);
                    ensureUniqueNicks();

                    syncNickToClientAndIni(rNick);
                    try { setLauncherSkin(rSkin); } catch (Throwable ignored) {}

                    updateSlotsListUI();
                    applySelectedSlotToUIAndClient(false);

                    if (slotsOpened) {
                        slotsOpened = false;
                        hideSlotsAnimated();
                    }

                    toast("Персонаж создан: " + rNick);
                    forceCheckBindForSlotWithToast(s, "created");

                    if (cb != null) cb.onSuccess(rNick, rSkin);
                });

            } catch (Throwable e) {
                final String msg = mapCreateErrToMessage(e);
                ui.post(() -> { if (cb != null) cb.onError(msg); });
            }
        }).start();
    }


    // =========================================================
    // ===================== NICK CHECK ========================
    // =========================================================

    private void checkNickExistsAsync(String nick, CreateCharacterController.NickCheckCb cb) {
        final String n = safeNick(nick);

        if (!isRealNick(n)) {
            if (cb != null) cb.onError("Неверный ник");
            return;
        }

        new Thread(() -> {
            try {
                String dev = getDeviceIdSafe();

                // server supports: ?check=1&name=Nick_Name
                String url = CREATE_CHARACTER_URL
                        + "?check=1"
                        + "&name=" + Uri.encode(n)
                        + "&device=" + Uri.encode(dev);

                String body = httpGet(url);
                JSONObject obj = new JSONObject(body);

                boolean ok = obj.optBoolean("ok", false);
                boolean exists = obj.optBoolean("exists", false);

                if (!ok) throw new RuntimeException(obj.optString("error", "check_failed"));

                ui.post(() -> { if (cb != null) cb.onResult(exists); });

            } catch (Throwable e) {
                String m = e.getMessage();
                if (m == null || m.isEmpty()) m = "Ошибка проверки ника";
                final String msg = m;
                ui.post(() -> { if (cb != null) cb.onError(msg); });
            }
        }).start();
    }

    private String mapCreateErrToMessage(Throwable e) {
        String m = (e == null) ? "" : safeStr(e.getMessage());
        String s = m.toLowerCase(Locale.ROOT);

        if (s.contains("nick_taken") || s.contains("duplicate") || s.contains("exists")) return "Ник уже занят";
        if (s.contains("bad_name") || s.contains("invalid") || s.contains("name")) return "Некорректный ник";
        if (s.contains("bad_skin") || s.contains("skin")) return "Некорректный скин";

        // если сервер вернул что-то читаемое — покажем
        if (!TextUtils.isEmpty(m) && m.length() <= 80) return m;

        return "Ошибка создания персонажа";
    }




    // =========================================================
    // ===================== PLAY FLOW =========================
    // =========================================================

    private void onPlayPressed() {
        final String desiredNick = safeNick(getSlotNick(selectedSlot));

        if (!isRealNick(desiredNick)) {
            // no nick in selected slot -> go to character creation (background remains)
            if (createCharacterController != null) {
                createCharacterController.show(selectedSlot);
            } else {
                prefillNickInputsFromSelectedSlot();
                showNickOverlay();
            }
            return;
        }

        syncNickToClientAndIni(desiredNick);

        final String actualNick = getClientNickSafe(desiredNick);

        logBind("PLAY pressed: slot=" + selectedSlot + " desired=" + desiredNick + " actual=" + actualNick);

        if (isRealNick(actualNick) && !actualNick.equalsIgnoreCase(desiredNick)) {
            setSlotNick(selectedSlot, actualNick);
            ensureUniqueNicks();
            if (tvLoginValue != null) tvLoginValue.setText(actualNick);
            if (tvCharacterNick != null) tvCharacterNick.setText(actualNick.toUpperCase(Locale.ROOT));
            syncNickToClientAndIni(actualNick);
        }

        setBindChecking(selectedSlot, true);
        updateSlotsListUI();

        checkAccessOnly(selectedSlot, actualNick, new AccessResultCb() {
            @Override public void onAllowed(boolean okFlag, boolean allowedFlag, String reason, String rawBody) {
                logBind("PLAY access OK: " + reason);
                setBindChecking(selectedSlot, false);
                setBindStatus(selectedSlot, true);
                updateSlotsListUI();
                connectNow();
            }

            @Override public void onNeedBind(boolean okFlag, boolean allowedFlag, String reason, String rawBody) {
                logBind("PLAY need bind: " + reason);
                setBindChecking(selectedSlot, false);
                setBindStatus(selectedSlot, false);
                updateSlotsListUI();
                openBindScreenFancy(actualNick, selectedSlot);
            }

            @Override public void onDenied(boolean okFlag, boolean allowedFlag, String reason, String rawBody) {
                logBind("PLAY denied: " + reason);
                setBindChecking(selectedSlot, false);
                setBindStatus(selectedSlot, false);
                updateSlotsListUI();
                toast("Нет доступа" + (!TextUtils.isEmpty(reason) ? (": " + reason) : ""));
            }

            @Override public void onError(Throwable e) {
                logBind("PLAY error: " + e.getClass().getSimpleName());
                setBindChecking(selectedSlot, false);
                setBindStatus(selectedSlot, false);
                updateSlotsListUI();
                toast("Ошибка проверки привязки: " + e.getClass().getSimpleName());
            }
        });
    }

    private void connectNow() {
        stopOnlineMonitoring();
        pulsePlayButton(false);
        stopX3Pulse();

        startConnect();
        Util.HideLayout(this.mMainLayout, false);
    }

    // =========================================================
    // ===================== NICK OVERLAY ======================
    // =========================================================

    private void prefillNickInputsFromSelectedSlot() {
        String name = safeNick(getSlotNick(selectedSlot));
        if (DEFAULT_NICK.equalsIgnoreCase(name)) name = "";

        String first = "";
        String last = "";
        if (name.contains("_")) {
            String[] parts = name.split("_", 2);
            if (parts.length == 2) { first = parts[0]; last = parts[1]; }
        }

        if (etNickFirst != null) etNickFirst.setText(first);
        if (etNickLast != null) etNickLast.setText(last);
    }

    private boolean nickPartOk(String s) {
        if (s == null) return false;
        String t = s.trim();
        if (t.length() < 3) return false;
        if (t.length() > 12) return false;

        for (int i = 0; i < t.length(); i++) {
            char c = t.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
            if (!ok) return false;
        }
        return true;
    }

    private void saveNickToSelectedSlot() {
        // Only for empty slots: no rename when slot already used
        if (!isSlotEmpty(selectedSlot)) {
            toast("Слот уже занят — смена ника отключена");
            hideNickOverlay();
            return;
        }

        String first = etNickFirst != null ? etNickFirst.getText().toString().trim() : "";
        String last  = etNickLast  != null ? etNickLast.getText().toString().trim()  : "";

        if (!nickPartOk(first)) { toast("Имя: латиница/цифры, минимум 3 символа"); return; }
        if (!nickPartOk(last))  { toast("Фамилия: латиница/цифры, минимум 3 символа"); return; }

        final String nick = first + "_" + last;

        for (int i = 1; i <= 3; i++) {
            if (i == selectedSlot) continue;
            if (nick.equalsIgnoreCase(getSlotNick(i))) {
                toast("Этот ник уже используется в другом слоте");
                return;
            }
        }

        // Disable save while checking
        if (btnNickSave != null) {
            btnNickSave.setEnabled(false);
            btnNickSave.setAlpha(0.6f);
        }

        checkNickExistsAsync(nick, new CreateCharacterController.NickCheckCb() {
            @Override public void onResult(boolean exists) {
                // re-enable button
                if (btnNickSave != null) {
                    btnNickSave.setEnabled(true);
                    btnNickSave.setAlpha(1f);
                }

                if (exists) {
                    // Existing character: attach to slot
                    setSlotNick(selectedSlot, nick);
                    syncNickToClientAndIni(nick);

                    if (tvLoginValue != null) tvLoginValue.setText(nick);
                    if (tvCharacterNick != null) tvCharacterNick.setText(nick.toUpperCase(Locale.ROOT));

                    setBindChecking(selectedSlot, true);
                    updateSlotsListUI();
                    hideNickOverlay();

                    fetchPlayerInfoForNick(selectedSlot, nick);
                    forceCheckBindForSlotWithToast(selectedSlot, "nick_saved_exists");

                    toast("Ник сохранён (найден в базе)");
                    return;
                }

                // Not exists: open create character immediately
                hideNickOverlay();
                if (createCharacterController != null) {
                    createCharacterController.show(selectedSlot, first, last);
                } else {
                    toast("Не удалось открыть создание персонажа");
                }
            }

            @Override public void onError(String message) {
                if (btnNickSave != null) {
                    btnNickSave.setEnabled(true);
                    btnNickSave.setAlpha(1f);
                }
                toast(message != null && !message.isEmpty() ? message : "Ошибка проверки ника");
            }
        });
    }


    // =========================================================
    // ===================== OVERLAY ANIMS =====================
    // =========================================================

    private void hideNickOverlayInstant() {
        if (nickOverlay != null) {
            nickOverlay.setAlpha(1f);
            nickOverlay.setVisibility(View.GONE);
        }
        if (nickCard != null) {
            nickCard.setAlpha(1f);
            nickCard.setScaleX(1f);
            nickCard.setScaleY(1f);
            nickCard.setTranslationY(0f);
        }
    }

    private void showNickOverlay() {
        if (nickOverlay == null) return;

        nickOverlay.setVisibility(View.VISIBLE);
        nickOverlay.setAlpha(0f);

        if (nickCard != null) {
            nickCard.setAlpha(0f);
            nickCard.setScaleX(0.96f);
            nickCard.setScaleY(0.96f);
            nickCard.setTranslationY(24f);
        }

        forcedAnim(220, f -> {
            nickOverlay.setAlpha(f);
            if (nickCard != null) {
                nickCard.setAlpha(f);
                nickCard.setScaleX(0.96f + 0.04f * f);
                nickCard.setScaleY(0.96f + 0.04f * f);
                nickCard.setTranslationY(24f * (1f - f));
            }
        }, null);
    }

    private void hideNickOverlay() {
        if (nickOverlay == null) return;

        final float a0 = nickOverlay.getAlpha();
        final float cardA0 = (nickCard != null) ? nickCard.getAlpha() : 1f;
        final float y0 = (nickCard != null) ? nickCard.getTranslationY() : 0f;

        forcedAnim(200, f -> {
            float inv = 1f - f;
            nickOverlay.setAlpha(a0 * inv);
            if (nickCard != null) {
                nickCard.setAlpha(cardA0 * inv);
                nickCard.setTranslationY(y0 + 18f * f);
                nickCard.setScaleX(1f - 0.03f * f);
                nickCard.setScaleY(1f - 0.03f * f);
            }
        }, () -> nickOverlay.setVisibility(View.GONE));
    }

    // =========================================================
    // ===================== ACCESS + TG BIND ==================
    // =========================================================

    private interface AccessResultCb {
        void onAllowed(boolean okFlag, boolean allowedFlag, String reason, String rawBody);
        void onNeedBind(boolean okFlag, boolean allowedFlag, String reason, String rawBody);
        void onDenied(boolean okFlag, boolean allowedFlag, String reason, String rawBody);
        void onError(Throwable e);
    }

    private void checkAccessOnly(int slot, String nick, AccessResultCb cb) {
        final String n = safeNick(nick);
        final int s = clampSlot(slot);

        new Thread(() -> {
            try {
                String dev = getDeviceIdSafe();
                String model = safeStr(Build.MODEL);
                String sdk = String.valueOf(Build.VERSION.SDK_INT);

                String url = ACCESS_URL
                        + "?nick=" + Uri.encode(n)
                        + "&slot=" + Uri.encode(String.valueOf(s))
                        + "&device=" + Uri.encode(dev)
                        + "&model=" + Uri.encode(model)
                        + "&sdk=" + Uri.encode(sdk);

                logBind("ACCESS GET: " + url);

                String body = httpGet(url);
                JSONObject response = new JSONObject(body);

                boolean ok = response.optBoolean("ok", false);
                boolean allowed = response.optBoolean("allowed", false) || response.optBoolean("access", false);
                String reason = response.optString("reason", "");

                JSONObject r = response.optJSONObject("result");
                if (r != null) {
                    if (!ok) ok = r.optBoolean("ok", ok);
                    allowed = r.optBoolean("allowed", allowed) || r.optBoolean("access", allowed);
                    if (TextUtils.isEmpty(reason)) reason = r.optString("reason", "");
                }

                final boolean fOk = ok;
                final boolean fAllowed = allowed;
                final String fReason = safeStr(reason);
                final String fBody = body;

                ui.post(() -> {
                    if (cb == null) return;

                    if (fOk && fAllowed) { cb.onAllowed(fOk, fAllowed, fReason, fBody); return; }

                    String rr = fReason.toLowerCase(Locale.ROOT);

                    if (fOk && !fAllowed && TextUtils.isEmpty(rr)) {
                        cb.onNeedBind(fOk, fAllowed, fReason, fBody);
                        return;
                    }

                    if (rr.contains("bind") || rr.contains("not_bound") || rr.contains("need") || rr.contains("tg")) {
                        cb.onNeedBind(fOk, fAllowed, fReason, fBody);
                        return;
                    }

                    cb.onDenied(fOk, fAllowed, fReason, fBody);
                });

            } catch (Throwable e) {
                ui.post(() -> { if (cb != null) cb.onError(e); });
            }
        }).start();
    }

    private void forceCheckBindForSlot(int slot, String reason) {
        final int s = clampSlot(slot);
        final String desired = safeNick(getSlotNick(s));

        if (!isRealNick(desired)) {
            logBind("forceCheckBind: slot=" + s + " reason=" + reason + " => empty");
            setBindChecking(s, false);
            setBindStatus(s, false);
            updateSlotsListUI();
            return;
        }

        syncNickToClientAndIni(desired);
        final String actual = getClientNickSafe(desired);

        logBind("forceCheckBind: slot=" + s + " reason=" + reason + " nick=" + actual);

        setBindChecking(s, true);
        updateSlotsListUI();

        checkAccessOnly(s, actual, new AccessResultCb() {
            @Override public void onAllowed(boolean okFlag, boolean allowedFlag, String reasonText, String rawBody) {
                setBindChecking(s, false);
                setBindStatus(s, true);
                updateSlotsListUI();
            }

            @Override public void onNeedBind(boolean okFlag, boolean allowedFlag, String reasonText, String rawBody) {
                setBindChecking(s, false);
                setBindStatus(s, false);
                updateSlotsListUI();
            }

            @Override public void onDenied(boolean okFlag, boolean allowedFlag, String reasonText, String rawBody) {
                setBindChecking(s, false);
                setBindStatus(s, false);
                updateSlotsListUI();
            }

            @Override public void onError(Throwable e) {
                setBindChecking(s, false);
                setBindStatus(s, false);
                updateSlotsListUI();
            }
        });
    }

    private void forceCheckBindForSlotWithToast(int slot, String reason) {
        final int s = clampSlot(slot);
        final String nick = safeNick(getSlotNick(s));

        if (!isRealNick(nick)) return;

        setBindChecking(s, true);
        updateSlotsListUI();

        checkAccessOnly(s, nick, new AccessResultCb() {
            @Override public void onAllowed(boolean okFlag, boolean allowedFlag, String reasonText, String rawBody) {
                setBindChecking(s, false);
                setBindStatus(s, true);
                updateSlotsListUI();
                toast("Привязка: привязан");
            }

            @Override public void onNeedBind(boolean okFlag, boolean allowedFlag, String reasonText, String rawBody) {
                setBindChecking(s, false);
                setBindStatus(s, false);
                updateSlotsListUI();
                toast("Привязка: не привязан");
            }

            @Override public void onDenied(boolean okFlag, boolean allowedFlag, String reasonText, String rawBody) {
                setBindChecking(s, false);
                setBindStatus(s, false);
                updateSlotsListUI();
                toast("Привязка: не привязан");
            }

            @Override public void onError(Throwable e) {
                setBindChecking(s, false);
                setBindStatus(s, false);
                updateSlotsListUI();
                toast("Привязка: ошибка проверки");
            }
        });
    }


    private void openBindScreenFancy(String nick, int slot) {
        try {
            Intent i = new Intent(activity, TgBindActivity.class);
            i.putExtra(TgBindActivity.EXTRA_NICK, nick);
            i.putExtra(TgBindActivity.EXTRA_SLOT, slot);

            View root = mMainLayout;
            if (root == null) {
                activity.startActivity(i);
                return;
            }

            float a0 = root.getAlpha();
            forcedAnim(170, f -> {
                root.setAlpha(a0 * (1f - f));
                root.setScaleX(1f - 0.01f * f);
                root.setScaleY(1f - 0.01f * f);
            }, () -> {
                try { activity.startActivity(i); } catch (Throwable ignored) {}
                root.setAlpha(a0);
                root.setScaleX(1f);
                root.setScaleY(1f);

                try { activity.overridePendingTransition(0, 0); } catch (Throwable ignored) {}
            });
        } catch (Throwable t) {
            toast("Не удалось открыть привязку Telegram");
        }
    }

    // =========================================================
    // ===================== PLAYER INFO =======================
    // =========================================================

    private static class PlayerInfo {
        public boolean success;
        public int money;
        public int vip;
        public int admin;
        public int level;
        public int skin;
        public String prefix;
    }

    private void fetchPlayerInfoForNick(int slot, String nick) {
        final int s = clampSlot(slot);
        final String n = safeNick(nick);

        if (!isRealNick(n)) {
            ui.post(() -> {
                applyPlayerInfoToUI(null);
                updateSlotsListUI();
            });
            return;
        }

        new Thread(() -> {
            PlayerInfo info = null;
            try {
                String body;
                JSONObject obj;

                String url = PLAYER_INFO_URL + "?nick=" + Uri.encode(n);
                body = httpGet(url);
                obj = new JSONObject(body);

                boolean preOk = obj.optBoolean("ok", false) || obj.optBoolean("success", false) || obj.optJSONObject("result") != null;
                if (!preOk) {
                    String url2 = PLAYER_INFO_URL + "?playerName=" + Uri.encode(n);
                    body = httpGet(url2);
                    obj = new JSONObject(body);
                }

                JSONObject r = obj.optJSONObject("result");
                boolean ok = obj.optBoolean("ok", false);
                if (!ok) ok = obj.optBoolean("success", false);
                if (r != null && !ok) ok = r.optBoolean("ok", ok);

                if (ok) {
                    JSONObject srcObj = (r != null) ? r : obj;

                    info = new PlayerInfo();
                    info.success = true;
                    info.money = srcObj.optInt("money", 0);
                    info.vip = srcObj.optInt("vip", 0);
                    info.admin = srcObj.optInt("admin", 0);
                    info.level = srcObj.optInt("level", 0);
                    info.skin = srcObj.has("skin") ? srcObj.optInt("skin", getSlotSkin(s)) : getSlotSkin(s);
                    info.prefix = srcObj.optString("prefix", "");
                }
            } catch (Throwable ignored) {}

            final PlayerInfo finalInfo = info;

            ui.post(() -> {
                String cur = safeNick(getSlotNick(s));
                if (!cur.equalsIgnoreCase(n)) return;

                if (finalInfo != null && finalInfo.success) {
                    if (finalInfo.skin >= 0 && finalInfo.skin <= 311) {
                        setSlotSkin(s, finalInfo.skin);
                        if (s == selectedSlot) {
                            try { setLauncherSkin(finalInfo.skin); } catch (Throwable ignored) {}
                        }
                    }
                }

                applyPlayerInfoToUI(finalInfo);
                updateSlotsListUI();
            });
        }).start();
    }

    @SuppressLint("SetTextI18n")
    private void applyPlayerInfoToUI(PlayerInfo info) {
        if (tvBalanceValue != null) {
            int money = (info != null && info.success) ? info.money : 0;
            tvBalanceValue.setText(moneyFmt.format(money));
        }

        if (info == null || !info.success) {
            if (launcherVipCard != null) launcherVipCard.setVisibility(View.GONE);
            if (chipAdmin != null) chipAdmin.setVisibility(View.GONE);
            if (tvVipDays != null) {
                tvVipDays.setText("VIP");
                tvVipDays.getPaint().setShader(null);
                tvVipDays.setShadowLayer(0f, 0f, 0f, 0);
            }
            return;
        }

        if (launcherVipCard != null) launcherVipCard.setVisibility(info.vip > 0 ? View.VISIBLE : View.GONE);

        if (tvVipDays != null) {
            if (info.vip > 0) {
                String label = vipName(info.vip);
                tvVipDays.setText(label);
                tvVipDays.setAllCaps(true);
                tvVipDays.setLetterSpacing(0.06f);
                styleVipLabel(tvVipDays, info.vip);
            } else {
                tvVipDays.setText("VIP");
                tvVipDays.getPaint().setShader(null);
                tvVipDays.setShadowLayer(0f, 0f, 0f, 0);
            }
        }

        if (chipAdmin != null) {
            if (info.admin > 0) {
                chipAdmin.setVisibility(View.VISIBLE);
                if (chipAdmin instanceof TextView) {
                    ((TextView) chipAdmin).setText("АДМИН " + info.admin);
                }
            } else {
                chipAdmin.setVisibility(View.GONE);
            }
        }
    }

    // =========================================================
    // ===================== VIP STYLES ========================
    // =========================================================

    private void setVipTitleGold(TextView tv) {
        tv.post(() -> {
            float w = tv.getPaint().measureText(tv.getText().toString());
            if (w <= 0f) w = tv.getWidth();
            LinearGradient shader = new LinearGradient(
                    0, 0, w, 0,
                    new int[]{ 0xFFFFF2B0, 0xFFFFD34D, 0xFFFFA800 },
                    null,
                    Shader.TileMode.CLAMP
            );
            tv.getPaint().setShader(shader);
            tv.invalidate();
        });
    }

    private String vipName(int vip) {
        if (vip <= 0) return "VIP";
        if (vip == 1) return "SILVER";
        if (vip == 2) return "GOLD";
        return "PLATINUM";
    }

    private void styleVipLabel(TextView tv, int vip) {
        if (tv == null) return;

        tv.setIncludeFontPadding(false);
        tv.setShadowLayer(8f, 0f, 2f, 0x55000000);

        tv.post(() -> {
            float w = tv.getPaint().measureText(tv.getText().toString());
            if (w <= 0f) w = tv.getWidth();

            int[] colors;
            if (vip == 1) {
                colors = new int[]{ 0xFFF6F7FA, 0xFFCFD6E3, 0xFF9AA4B6 };
            } else if (vip == 2) {
                colors = new int[]{ 0xFFFFF2B0, 0xFFFFD34D, 0xFFFFA800 };
            } else {
                colors = new int[]{ 0xFFF7FDFF, 0xFFBFE7FF, 0xFF7FB2FF };
            }

            LinearGradient shader = new LinearGradient(
                    0, 0, w, 0,
                    colors, null,
                    Shader.TileMode.CLAMP
            );
            tv.getPaint().setShader(shader);
            tv.invalidate();
        });
    }

    // =========================================================
    // ===================== SERVER UI =========================
    // =========================================================

    @SuppressLint("SetTextI18n")
    public void updateServerUI() {
        SharedPreferences prefs = activity.getSharedPreferences("server_prefs", Context.MODE_PRIVATE);

        int serverId = prefs.getInt("serverId", 1);
        String serverName = prefs.getString("serverName", "Moscow");
        int onlinePlayers = prefs.getInt("onlinePlayers", 0);
        int maxPlayers = prefs.getInt("maxPlayers", 1000);
        boolean isAvailable = prefs.getBoolean("isAvailable", true);

        if (launcherMainCurrentServerNum != null) {
            launcherMainCurrentServerNum.setText("#" + serverId);
            setServerNumberStyle(serverId);
        }

        int progress1000 = calculateProgress1000(onlinePlayers, maxPlayers);
        if (launcherMainCurrentServerProgress != null) {
            launcherMainCurrentServerProgress.setMax(1000);
            launcherMainCurrentServerProgress.setProgress(progress1000);

            int drawable;
            if (onlinePlayers > 600) drawable = R.drawable.launcher_server_online_high_progress_bar;
            else if (onlinePlayers < 300) drawable = R.drawable.launcher_server_online_low_progress_bar;
            else drawable = R.drawable.launcher_server_online_medium_progress_bar;

            launcherMainCurrentServerProgress.setProgressDrawable(
                    AppCompatResources.getDrawable(activity, drawable)
            );
        }

        if (!isAvailable) {
            if (launcherMainCurrentServerProgress != null)
                launcherMainCurrentServerProgress.setVisibility(View.GONE);
            if (launcherMainCurrentServerOnlineInfo != null)
                launcherMainCurrentServerOnlineInfo.setText("СЕРВЕР ВЫКЛЮЧЕН");
        } else {
            if (launcherMainCurrentServerProgress != null)
                launcherMainCurrentServerProgress.setVisibility(View.VISIBLE);
            if (launcherMainCurrentServerOnlineInfo != null)
                launcherMainCurrentServerOnlineInfo.setText("ОНЛАЙН: " + onlinePlayers + " / " + maxPlayers);
        }

        if (launcherMainCurrentServerName != null)
            launcherMainCurrentServerName.setText(serverName != null ? serverName : "Moscow");
    }

    private int calculateProgress1000(int onlinePlayers, int maxPlayers) {
        if (maxPlayers <= 0) return 0;
        double ratio = (double) onlinePlayers / (double) maxPlayers;
        int v = (int) Math.round(ratio * 1000.0);
        if (v < 0) v = 0;
        if (v > 1000) v = 1000;
        return v;
    }

    private void setServerNumberStyle(int serverId) {
        if (launcherMainCurrentServerNum == null) return;

        if (serverId == 1) launcherMainCurrentServerNum.setBackgroundResource(R.drawable.launcher_server_badge_bg);
        else if (serverId == 2) launcherMainCurrentServerNum.setBackgroundResource(R.drawable.launcher_servers_item_purple_bg);
    }

    private void openServerSelection() { toast("Заглушка: экран выбора сервера"); }

    // =========================================================
    // ===================== SLOTS UI (ANIM) ===================
    // =========================================================

    private void toggleSlots() {
        slotsOpened = !slotsOpened;
        if (slotsOpened) showSlotsAnimated();
        else hideSlotsAnimated();
    }

    private void forceHideSlotsInstant() {
        if (slot1 != null) { slot1.setAlpha(1f); slot1.setTranslationY(0f); slot1.setVisibility(View.GONE); }
        if (slot2 != null) { slot2.setAlpha(1f); slot2.setTranslationY(0f); slot2.setVisibility(View.GONE); }
        if (slot3 != null) { slot3.setAlpha(1f); slot3.setTranslationY(0f); slot3.setVisibility(View.GONE); }
    }

    private void showSlotsAnimated() {
        if (slot1 != null) slot1.setVisibility(View.VISIBLE);
        if (slot2 != null) slot2.setVisibility(View.VISIBLE);
        if (slot3 != null) slot3.setVisibility(View.VISIBLE);

        long d = 0;
        if (slot1 != null) { animIn(slot1, d); d += 60; }
        if (slot2 != null) { animIn(slot2, d); d += 60; }
        if (slot3 != null) { animIn(slot3, d); }
    }

    private void hideSlotsAnimated() {
        long d = 0;
        if (slot3 != null) { animOut(slot3, d, () -> slot3.setVisibility(View.GONE)); d += 60; }
        if (slot2 != null) { animOut(slot2, d, () -> slot2.setVisibility(View.GONE)); d += 60; }
        if (slot1 != null) { animOut(slot1, d, () -> slot1.setVisibility(View.GONE)); d += 60; }
    }


    // =========================================================
    // ===================== FORCED ANIMS ======================
    // =========================================================

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
                if (raw < 1f) Choreographer.getInstance().postFrameCallback(this);
                else if (end != null) end.end();
            }
        });
    }

    private void animIn(View v, long delay) {
        if (v == null) return;

        v.setAlpha(0f);
        v.setTranslationY(18f);

        Runnable run = () -> forcedAnim(180, f -> {
            v.setAlpha(f);
            v.setTranslationY(18f * (1f - f));
        }, null);

        if (delay > 0) v.postDelayed(run, delay);
        else run.run();
    }

    private void animOut(View v, long delay, EndCb end) {
        if (v == null) {
            if (end != null) end.end();
            return;
        }

        final float a0 = v.getAlpha();
        final float y0 = v.getTranslationY();

        Runnable run = () -> forcedAnim(180, f -> {
            v.setAlpha(a0 * (1f - f));
            v.setTranslationY(y0 + 18f * f);
        }, end);

        if (delay > 0) v.postDelayed(run, delay);
        else run.run();
    }

    private void buttonClick(View v) {
        if (v == null) return;

        final float s0 = 1f;
        final float s1 = 0.92f;

        forcedAnim(90, f -> {
            float s = s0 + (s1 - s0) * f;
            v.setScaleX(s);
            v.setScaleY(s);
        }, () -> forcedAnim(110, f -> {
            float s = s1 + (s0 - s1) * f;
            v.setScaleX(s);
            v.setScaleY(s);
        }, () -> {
            v.setScaleX(1f);
            v.setScaleY(1f);
        }));
    }

    private void hapticTick(View v) {
        try {
            if (v == null) return;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1)
                v.performHapticFeedback(HapticFeedbackConstants.TEXT_HANDLE_MOVE);
            else
                v.performHapticFeedback(HapticFeedbackConstants.KEYBOARD_TAP);
        } catch (Throwable ignored) {}
    }

    // “от себя”: лёгкий “дыхательный” пульс на PLAY
    private void pulsePlayButton(boolean enable) {
        playPulseOn = enable;
        if (!enable) {
            if (btnPlay != null) {
                btnPlay.setScaleX(1f);
                btnPlay.setScaleY(1f);
                btnPlay.setAlpha(1f);
            }
            return;
        }
        if (btnPlay == null) return;
        pulseLoop(btnPlay);
    }

    private void pulseLoop(View v) {
        if (!playPulseOn || v == null) return;

        forcedAnim(900, f -> {
            float wave = (float) Math.sin(f * Math.PI);
            float s = 1f + 0.02f * wave;
            v.setScaleX(s);
            v.setScaleY(s);
            v.setAlpha(0.92f + 0.08f * wave);
        }, () -> pulseLoop(v));
    }

    // =========================================================
    // ===================== NICK + INI ========================
    // =========================================================

    private File getSettingsIniFile() {
        File base = activity.getExternalFilesDir(null);
        if (base == null) return null;

        File sampDir = new File(base, "SAMP");
        if (!sampDir.exists()) sampDir.mkdirs();

        return new File(sampDir, "settings.ini");
    }

    private String readNickFromIni() {
        try {
            File settingsFile = getSettingsIniFile();
            if (settingsFile == null || !settingsFile.exists()) return null;

            Wini w = new Wini(settingsFile);
            return w.get("client", "name");
        } catch (Throwable t) {
            return null;
        }
    }

    private void writeNickToIniCompat(String nick) throws Exception {
        File f = getSettingsIniFile();
        if (f == null) return;
        if (!f.exists()) f.createNewFile();

        Wini w = new Wini(f);
        if (w.get("client") == null) w.add("client");
        w.put("client", "name", nick);
        w.store();
    }

    private void sendLogFile() {
        File file = new File(activity.getApplicationContext().getExternalFilesDir(null), "samp_log.txt");
        if (file.exists()) {
            try {
                Uri uri = FileProvider.getUriForFile(
                        activity,
                        activity.getApplicationContext().getPackageName() + ".provider",
                        file
                );
                Intent intent = new Intent(Intent.ACTION_SEND);
                intent.setType("text/plain");
                intent.putExtra(Intent.EXTRA_STREAM, uri);
                intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                activity.startActivity(Intent.createChooser(intent, "Отправить логи"));
                return;
            } catch (Throwable ignored) {}
        }
        toast("Файл samp_log.txt не найден");
    }

    // =========================================================
    // ===================== NET HELPERS =======================
    // =========================================================

    private String httpGet(String urlStr) throws Exception {
        HttpURLConnection c = null;
        try {
            URL url = new URL(urlStr);
            c = (HttpURLConnection) url.openConnection();
            c.setConnectTimeout(7000);
            c.setReadTimeout(7000);
            c.setRequestMethod("GET");

            int code = c.getResponseCode();
            BufferedReader br = new BufferedReader(new InputStreamReader(
                    (code >= 200 && code < 400) ? c.getInputStream() : c.getErrorStream(),
                    Charset.forName("UTF-8")
            ));

            StringBuilder sb = new StringBuilder();
            String line;
            while ((line = br.readLine()) != null) sb.append(line);
            br.close();

            if (code < 200 || code >= 400) throw new RuntimeException("HTTP " + code + " " + sb);
            return sb.toString();
        } finally {
            if (c != null) try { c.disconnect(); } catch (Throwable ignored) {}
        }
    }

    private String getDeviceIdSafe() {
        String androidId = "";
        try { androidId = Settings.Secure.getString(activity.getContentResolver(), Settings.Secure.ANDROID_ID); }
        catch (Exception ignored) {}
        if (TextUtils.isEmpty(androidId)) androidId = "unknown";
        return androidId;
    }

    private String safeStr(String s) { return s == null ? "" : s; }
    private String safeNick(String s) { return s == null ? "" : s.trim(); }

    private boolean isRealNick(String nick) {
        String n = safeNick(nick);
        return !TextUtils.isEmpty(n) && !DEFAULT_NICK.equalsIgnoreCase(n);
    }

    private String getClientNickSafe(String fallbackNick) {
        String v = "";
        try { v = getClientNick(); } catch (Throwable ignored) {}
        v = safeNick(v);
        if (isRealNick(v)) return v;
        return safeNick(fallbackNick);
    }

    private void toast(String text) { Toast.makeText(activity, text, Toast.LENGTH_SHORT).show(); }

    private void openUrl(String url) {
        try { activity.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(url))); }
        catch (Throwable ignored) {}
    }

    private void hideSystemUI() {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                WindowInsetsController controller = activity.getWindow().getInsetsController();
                if (controller != null) {
                    controller.hide(WindowInsets.Type.systemBars());
                    controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                }
            } else {
                @SuppressWarnings("deprecation")
                int flags = View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
                activity.getWindow().getDecorView().setSystemUiVisibility(flags);
            }
        } catch (Throwable ignored) {}
    }


    private void syncNickToClientAndIniAllowDefault(String nick) {
        String n = safeNick(nick);
        if (TextUtils.isEmpty(n)) n = DEFAULT_NICK;

        try { writeNickToIniCompat(n); } catch (Throwable ignored) {}
        try { setClientNick(n); } catch (Throwable ignored) {}
    }

    private void syncNickToClientAndIni(String nick) {
        String n = safeNick(nick);
        if (!isRealNick(n)) return;

        try { writeNickToIniCompat(n); } catch (Throwable ignored) {}
        try { setClientNick(n); } catch (Throwable ignored) {}
    }
}