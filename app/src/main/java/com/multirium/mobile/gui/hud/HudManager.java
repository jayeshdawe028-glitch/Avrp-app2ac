package com.multirium.mobile.gui.hud;

import android.app.Activity;
import android.text.Html;
import android.view.GestureDetector;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.widget.ImageView;
import android.widget.TextView;

import com.joom.paranoid.Obfuscate;
import com.multirium.mobile.R;
import com.multirium.mobile.core.Samp;
import com.multirium.mobile.databinding.CrossHudBinding;

import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;

@Obfuscate
public class HudManager {

    private final Activity activity;
    private final CrossHudBinding binding;
    public ViewGroup viewGroup = null;

    public boolean isShow;
    private boolean isHudSetPos = false;

    public DecimalFormat formatter;

    private final ArrayList<ImageView> hud_wanted = new ArrayList<>();

    private boolean isRecording = false;

    private ChatFeedAdapter chatAdapter;

    private boolean newChatVisible = true;

    private int lastWeaponId = 0;
    private int lastAmmo = 0;
    private int lastAmmoClip = 0;

    private boolean weaponHidden = true;

    native void HudInit();
    native void nativeBoolShowHud(boolean toggle);
    native void SetRadarBgPos(float x1, float y1, float x2, float y2);
    native void SetRadarPos(float x1, float y1, float size);
    native void onWeaponChanged();
    native void alt();
    native void N();
    private native void nativeVoiceStart();
    private native void nativeVoiceStop();

    public HudManager(Activity activity) {
        this.activity = activity;

        binding = CrossHudBinding.inflate(LayoutInflater.from(activity));
        viewGroup = binding.getRoot();
        Samp.getInstance().getBackUILayout().addView(viewGroup, -1, -1);

        HudInit();

        formatter = new DecimalFormat();
        DecimalFormatSymbols symbols = DecimalFormatSymbols.getInstance();
        symbols.setGroupingSeparator('.');
        formatter.setDecimalFormatSymbols(symbols);

        setupWanted();
        setupVoiceButton();
        setupChatFeed();
        setupHudChatClickToOpenKeyboard();
        setupFistWeaponClick();

        if (binding.menuCircle != null) {
            binding.menuCircle.setOnClickListener(v -> System.out.println("menu_circle"));
        }

        isShow = false;
        viewVisible(viewGroup, View.GONE);
        updateDateTime();

        setOldChatVisible(false);
        setNewChatVisible(true);

        applyWeaponUi(0, 0, 0);
    }

    public void UpdateAmmo(int weaponid, int ammo, int ammoclip) {
        lastWeaponId = weaponid;
        lastAmmo = ammo;
        lastAmmoClip = ammoclip;

        weaponHidden = (weaponid == 0 || weaponid == 1);

        activity.runOnUiThread(() -> applyWeaponUi(lastWeaponId, lastAmmo, lastAmmoClip));
    }

    private void setupChatFeed() {
        if (binding.chatFeedList == null) return;
        chatAdapter = new ChatFeedAdapter(activity);
        chatAdapter.attach(binding.chatFeedList);

        binding.chatFeedList.setNestedScrollingEnabled(true);
        binding.chatFeedList.setOverScrollMode(View.OVER_SCROLL_ALWAYS);
    }

    private void setupHudChatClickToOpenKeyboard() {
        final GestureDetector detector = new GestureDetector(activity, new GestureDetector.SimpleOnGestureListener() {
            @Override
            public boolean onSingleTapUp(MotionEvent e) {
                try {
                    if (!newChatVisible) return false;
                    if (Samp.getInstance().mChat != null) {
                        Samp.getInstance().mChat.ChatClickFromHud();
                        return true;
                    }
                } catch (Throwable ignored) { }
                return false;
            }
        });

        final View.OnTouchListener touch = (v, e) -> {
            if (!newChatVisible) return false;

            boolean tapped = detector.onTouchEvent(e);

            int a = e.getActionMasked();
            if (a == MotionEvent.ACTION_MOVE || a == MotionEvent.ACTION_SCROLL) {
                return false;
            }

            return tapped;
        };

        if (binding.chatFeedContainer != null) {
            binding.chatFeedContainer.setClickable(true);
            binding.chatFeedContainer.setFocusable(false);
            binding.chatFeedContainer.setOnTouchListener(touch);
        }

        if (binding.chatFeedList != null) {
            binding.chatFeedList.setClickable(true);
            binding.chatFeedList.setFocusable(false);
            binding.chatFeedList.setOnTouchListener(touch);
        }
    }

    private void setupFistWeaponClick() {
        View.OnClickListener l = v -> {
            try {
                weaponHidden = !weaponHidden;
                applyWeaponUi(lastWeaponId, lastAmmo, lastAmmoClip);
                onWeaponChanged();
            } catch (Throwable ignored) { }
        };

        if (binding.fistContainer != null) binding.fistContainer.setOnClickListener(l);
        if (binding.fistBg != null) binding.fistBg.setOnClickListener(l);
        if (binding.fistIcon != null) binding.fistIcon.setOnClickListener(l);
    }

    public void setOldChatVisible(boolean visible) {
        activity.runOnUiThread(() -> {
            if (Samp.getInstance().mChat != null && Samp.getInstance().mChat.binding != null) {
                if (Samp.getInstance().mChat.binding.chatBox != null)
                    Samp.getInstance().mChat.binding.chatBox.setVisibility(visible ? View.VISIBLE : View.GONE);
                if (Samp.getInstance().mChat.binding.hideChat != null)
                    Samp.getInstance().mChat.binding.hideChat.setVisibility(visible ? View.VISIBLE : View.GONE);
            }
        });
    }

    public void setNewChatVisible(boolean visible) {
        newChatVisible = visible;
        activity.runOnUiThread(() -> {
            if (binding.chatFeedContainer != null) {
                binding.chatFeedContainer.setVisibility(visible ? View.VISIBLE : View.GONE);
            }
        });
    }

    private void setupWanted() {
        if (binding.star2 != null) hud_wanted.add(binding.star2);
        if (binding.star3 != null) hud_wanted.add(binding.star3);
        if (binding.star4 != null) hud_wanted.add(binding.star4);
        if (binding.star5 != null) hud_wanted.add(binding.star5);
        if (binding.star6 != null) hud_wanted.add(binding.star6);

        for (int i = 0; i < hud_wanted.size(); i++) {
            hud_wanted.get(i).setVisibility(View.GONE);
        }
    }

    private void setupVoiceButton() {
        if (binding.voiceBtn == null) return;

        binding.voiceBtn.setOnClickListener(v -> {
            if (!isRecording) {
                isRecording = true;
                binding.voiceBtn.setImageResource(R.drawable.test_micro);
                nativeVoiceStart();
            } else {
                isRecording = false;
                binding.voiceBtn.setImageResource(R.drawable.control_settings_voice_btn);
                nativeVoiceStop();
            }
        });
    }

    private void updateDateTime() {
        activity.runOnUiThread(() -> {
            if (binding.hudDate != null) {
                binding.hudDate.setText(new SimpleDateFormat("dd.MM.yyyy").format(Calendar.getInstance().getTime()));
            }
            if (binding.hudTime != null) {
                binding.hudTime.setText(new SimpleDateFormat("HH:mm").format(Calendar.getInstance().getTime()));
            }
        });
    }

    public void updatePlayerInfo(String nick, int id) {
        activity.runOnUiThread(() -> {
            if (binding.hudNickId != null) {
                String n = (nick == null || nick.isEmpty()) ? "Player" : nick;
                binding.hudNickId.setText(n + " \u2022 ID: " + id);
            }
            updateDateTime();
        });
    }

    public void toggleAll(boolean toggle, boolean showNewChat) {
        isShow = toggle;
        nativeBoolShowHud(isShow);

        activity.runOnUiThread(() -> {
            System.out.println("\n[toggle] HUD: " + toggle);

            if (toggle) {
                viewVisible(viewGroup, View.VISIBLE);

                setNewChatVisible(showNewChat);
                setOldChatVisible(false);

                if (binding.radarDisk != null) {
                    binding.radarDisk.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
                        @Override
                        public void onGlobalLayout() {
                            binding.radarDisk.getViewTreeObserver().removeOnGlobalLayoutListener(this);
                            if (isHudSetPos) return;

                            SetRadarBgPos(binding.radarDisk.getX(), binding.radarDisk.getY(),
                                    binding.radarDisk.getWidth(), binding.radarDisk.getHeight());

                            int screenwidth = binding.getRoot().getWidth();
                            int screenheight = binding.getRoot().getHeight();

                            float real_prcX = ((binding.radarDisk.getX() + (binding.radarDisk.getWidth() / 2f)) / (float) screenwidth) * 100f;
                            float real_prcY = ((binding.radarDisk.getY() + (binding.radarDisk.getHeight() / 2.2f)) / (float) screenheight) * 100f;

                            float gtaX = 640f * (real_prcX / 100f);
                            float gtaY = 480f * (real_prcY / 100f);

                            SetRadarPos(gtaX, gtaY, 36.0f);

                            binding.radarDisk.setVisibility(View.INVISIBLE);
                            isHudSetPos = true;
                        }
                    });
                }

                updateDateTime();
                applyWeaponUi(lastWeaponId, lastAmmo, lastAmmoClip);
            } else {
                viewVisible(viewGroup, View.GONE);
                setOldChatVisible(false);
            }
        });
    }

    public void UpdateHudInfo(int health, int armour) {
        final int hp = clamp01to100(health);
        final int ar = clamp01to100(armour);

        activity.runOnUiThread(() -> {
            if (binding.hpCount != null) binding.hpCount.setText(String.valueOf(hp));
            if (binding.armorCount != null) binding.armorCount.setText(String.valueOf(ar));

            if (binding.hpProgress != null) binding.hpProgress.setProgress(hp);
            if (binding.armorProgress != null) binding.armorProgress.setProgress(ar);
        });
    }

    public void UpdateHudFood(int food) {
        final int f = clamp01to100(food);
        activity.runOnUiThread(() -> {
            if (binding.foodCount != null) binding.foodCount.setText(String.valueOf(f));
            if (binding.foodProgress != null) binding.foodProgress.setProgress(f);
        });
    }

    private void applyWeaponUi(int weaponid, int ammo, int ammoclip) {
        if (binding == null) return;

        int drawWeaponId = weaponHidden ? 0 : weaponid;

        if (binding.fistIcon != null) {
            int id = activity.getResources().getIdentifier(
                    String.format("weapon_%d", drawWeaponId),
                    "drawable",
                    activity.getPackageName()
            );
            if (id > 0) {
                binding.fistIcon.setImageResource(id);
            }
        }

        boolean showAmmo = !weaponHidden && (weaponid > 15) && (weaponid < 44) && (weaponid != 21);

        View ammoBg = null;
        View ammoIc = null;
        TextView ammoText = null;

        try { ammoBg = (View) CrossHudBinding.class.getField("radarCartridgesBg").get(binding); } catch (Throwable ignored) {}
        try { ammoIc = (View) CrossHudBinding.class.getField("radarCartridgesIc").get(binding); } catch (Throwable ignored) {}
        try { ammoText = (TextView) CrossHudBinding.class.getField("radarCartridgesCount").get(binding); } catch (Throwable ignored) {}

        if (ammoBg != null || ammoIc != null || ammoText != null) {
            if (showAmmo) {
                if (ammoBg != null) ammoBg.setVisibility(View.VISIBLE);
                if (ammoIc != null) ammoIc.setVisibility(View.VISIBLE);
                if (ammoText != null) {
                    String text = String.format("%d <font color='#B0B0B0'>/ %d</font>",
                            ammoclip, Math.max(0, ammo - ammoclip));
                    ammoText.setText(Html.fromHtml(text, Html.FROM_HTML_MODE_LEGACY));
                    ammoText.setVisibility(View.VISIBLE);
                }
            } else {
                if (ammoBg != null) ammoBg.setVisibility(View.GONE);
                if (ammoIc != null) ammoIc.setVisibility(View.GONE);
                if (ammoText != null) ammoText.setVisibility(View.GONE);
            }
        }
    }

    public void updateMoney(int money) {
        activity.runOnUiThread(() -> {
            if (binding.moneyCount == null) return;
            binding.moneyCount.setText(formatter != null ? formatter.format(money) : String.valueOf(money));
        });
    }

    public void UpdateWanted(int wantedLVL) {
        activity.runOnUiThread(() -> {
            for (int i = 0; i < hud_wanted.size(); i++) {
                hud_wanted.get(i).setVisibility(i < wantedLVL ? View.VISIBLE : View.GONE);
            }
        });
    }

    public void toggleGps(boolean toggle) { activity.runOnUiThread(() -> {}); }
    public void toggleGreenZone(boolean toggle) { activity.runOnUiThread(() -> {}); }
    public void toggleLogo(boolean toggle) { activity.runOnUiThread(() -> {}); }

    public void toggleVoice(boolean toggle) {
        activity.runOnUiThread(() -> {
            if (binding.voiceBtn != null) binding.voiceBtn.setVisibility(toggle ? View.VISIBLE : View.GONE);
        });
    }

    public void pushChatLine(int type, String text) {
        if (chatAdapter == null) return;
        String time = new SimpleDateFormat("HH:mm").format(Calendar.getInstance().getTime());
        chatAdapter.push(type, time, text == null ? "" : text);
    }

    public void clearChat() {
        if (chatAdapter == null) return;
        chatAdapter.clear();
    }

    public void viewVisible(ViewGroup viewGroup, int view) {
        if (viewGroup != null) {
            viewGroup.setAlpha(view == View.VISIBLE ? 1.0f : 0.0f);
            viewGroup.setVisibility(view);
        }
    }

    private int clamp01to100(int v) {
        if (v < 0) return 0;
        if (v > 100) return 100;
        return v;
    }
}
