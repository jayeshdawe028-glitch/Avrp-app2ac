package com.multirium.mobile.gui.speedometr;

import android.app.Activity;
import android.graphics.Color;
import android.text.SpannableStringBuilder;
import android.text.style.ForegroundColorSpan;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AnimationUtils;

import androidx.appcompat.content.res.AppCompatResources;

import com.multirium.mobile.R;

import com.multirium.mobile.common.EasyAnimation;
import com.multirium.mobile.core.Samp;
import com.multirium.mobile.databinding.HudBinding;
import com.multirium.mobile.databinding.SpeedometerFragmentBinding;
import com.multirium.mobile.gui.main_menu.obj.ButtonInfo;
import com.multirium.mobile.gui.main_menu.obj.Widget;
import com.multirium.mobile.gui.main_menu.utils.SettingGameButtonUtil;

public class Speedometr {
    private final int BUTTON_TURN_LEFT = 2;
    private final int BUTTON_TURN_RIGHT = 3;
    private final int BUTTON_TURN_ALL = 4;
    private final int BUTTON_ENGINE = 0;
    private final int BUTTON_LIGHT = 1;
    private final int BUTTON_DOOR = 5;

    private final SpeedometerFragmentBinding binding;
    private boolean indicators = false;
    private SpannableStringBuilder spannableStringBuilder;

    native void nativeSendClick(int id);
    native void nativeInit();
    private Activity activity;
    private final int turnlight_tick_sound_1 =
            Samp.soundPool.load(Samp.activity, R.raw.turnlight_tick_1, 0);
    private final int turnlight_tick_sound_2 =
            Samp.soundPool.load(Samp.activity, R.raw.turnlight_tick_2, 0);

    public Speedometr(Activity activity) {
        this.activity = activity;
        nativeInit();
        binding = SpeedometerFragmentBinding.inflate(LayoutInflater.from(activity));
        ButtonInfo setting = SettingGameButtonUtil.INSTANCE.getSetting(Widget.WIDGET_VEHICLE_STEER_LEFT);
        ButtonInfo setting2 = SettingGameButtonUtil.INSTANCE.getSetting(Widget.WIDGET_VEHICLE_STEER_RIGHT);
        if (setting != null) {
            SettingGameButtonUtil.updateGameButton(setting.getId(), setting.getX(), setting.getY(), setting.getWidth(), setting.getHeight());
        }
        if (setting2 != null) {
            SettingGameButtonUtil.updateGameButton(setting2.getId(), setting2.getX(), setting2.getY(), setting2.getWidth(), setting2.getHeight());
        }

        binding.progressHp.setMax(Math.round(1f));
        binding.progressAmountFuel.setMax(Math.round(1f));

        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(binding.tvHudSpeedometer, () -> {
            showMenu(indicators);
            return null;
        });
        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(binding.containerCarHeadlights, () -> {
            nativeSendClick(BUTTON_LIGHT);
            return null;
        });
        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(binding.containerEnabledCar, () -> {
            nativeSendClick(BUTTON_ENGINE);
            return null;
        });
        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(binding.containerCarDoors, () -> {
            nativeSendClick(BUTTON_DOOR);
            return null;
        });
        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(binding.containerCarLeftTurnSignal, () -> {
            nativeSendClick(BUTTON_TURN_RIGHT);
            return null;
        });
        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(binding.containerCarRightTurnSignal, () -> {
            nativeSendClick(BUTTON_TURN_LEFT);
            return null;
        });
        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(binding.containerCarAlarm, () -> {
            nativeSendClick(BUTTON_TURN_ALL);
            return null;
        });

        EasyAnimation.INSTANCE.HideLayout(binding.containerActionsCarLeft);
        EasyAnimation.INSTANCE.HideLayout(binding.containerActionsCarRight);
    }

    public void showMenu(boolean toggle) {
        indicators = !toggle;
        if (indicators) {
            EasyAnimation.INSTANCE.ShowLayout(binding.containerActionsCarLeft);
            EasyAnimation.INSTANCE.ShowLayout(binding.containerActionsCarRight);
            binding.containerActionsCarLeft.startAnimation(AnimationUtils.loadAnimation(activity, R.anim.slide_show_right_to_left));
            binding.containerActionsCarRight.startAnimation(AnimationUtils.loadAnimation(activity, R.anim.slide_show_left_to_right));
        } else {
            binding.containerActionsCarLeft.startAnimation(AnimationUtils.loadAnimation(activity, R.anim.slide_hide_left_to_right));
            binding.containerActionsCarRight.startAnimation(AnimationUtils.loadAnimation(activity, R.anim.slide_hide_right_to_left));
            EasyAnimation.INSTANCE.HideLayout(binding.containerActionsCarLeft);
            EasyAnimation.INSTANCE.HideLayout(binding.containerActionsCarRight);
        }
    }

    public void viewVisible(ViewGroup viewGroup, int view) {
        if (viewGroup != null) {
            viewGroup.setAlpha(view == View.VISIBLE ? 1.0f : 0.0f);
            viewGroup.setVisibility(view);
        }
    }

    public void show() {
        activity.runOnUiThread(() -> {
            if (this.binding.getRoot() != null) {
                Samp.getInstance().getBackUILayout().removeView(binding.getRoot());
            }
            Samp.getInstance().getBackUILayout().addView(binding.getRoot(), -1, -1);
            viewVisible(binding.getRoot(), View.VISIBLE);
        });
    }

    public void tempToggle(boolean toggle) {
        activity.runOnUiThread(() -> {
            viewVisible(binding.getRoot(), toggle ? View.VISIBLE : View.GONE);
        });
    }

    public void hide() {
        activity.runOnUiThread(() -> {
            viewVisible(binding.getRoot(), View.VISIBLE);
            if (this.binding.getRoot() != null) {
                Samp.getInstance().getBackUILayout().removeView(binding.getRoot());
            }
        });
    }
    public enum TurnLightState {
        TURNLIGHT_OFF,
        TURNLIGHT_LEFT,
        TURNLIGHT_RIGHT,
        TURNLIGHT_ALL
    }
    public void updateTurn(boolean isTurn, int state) {
        Samp.activity.runOnUiThread(() -> {

            if (!isTurn || state == TurnLightState.TURNLIGHT_OFF.ordinal()) {
                binding.containerCarAlarm.setBackgroundResource(R.drawable.background_disabled_car);
                binding.containerCarLeftTurnSignal.setBackgroundResource(R.drawable.background_disabled_car);
                binding.containerCarRightTurnSignal.setBackgroundResource(R.drawable.background_disabled_car);

                Samp.soundPool.play(turnlight_tick_sound_1, 0.1f, 0.2f, 1, 0, 1.0f);
                return;
            }

            if (state == TurnLightState.TURNLIGHT_LEFT.ordinal()) {
                binding.containerCarLeftTurnSignal.setBackgroundResource(R.drawable.background_enabled_car);
            } else if (state == TurnLightState.TURNLIGHT_RIGHT.ordinal()) {
                binding.containerCarRightTurnSignal.setBackgroundResource(R.drawable.background_enabled_car);
            } else if (state == TurnLightState.TURNLIGHT_ALL.ordinal()) {
                binding.containerCarAlarm.setBackgroundResource(R.drawable.background_enabled_car);
                binding.containerCarLeftTurnSignal.setBackgroundResource(R.drawable.background_enabled_car);
                binding.containerCarRightTurnSignal.setBackgroundResource(R.drawable.background_enabled_car);
            }

            Samp.soundPool.play(turnlight_tick_sound_2, 0.1f, 0.2f, 1, 0, 1.0f);
        });
    }

    public void updateSpeed(int speed) {
        activity.runOnUiThread(() -> {
            char[] xz = String.format("%03d", speed).toCharArray();

            binding.tvHubSpeed1.setText(String.valueOf(xz[0]));
            binding.tvHubSpeed2.setText(String.valueOf(xz[1]));
            binding.tvHubSpeed3.setText(String.valueOf(xz[2]));

            int ne = Color.parseColor("#80FFFFFF");
            int da = Color.parseColor("#FFFFFF");

            binding.tvHubSpeed1.setTextColor((xz[0] == '0') ? ne : da);
            binding.tvHubSpeed2.setTextColor((xz[0] == '0' && xz[1] == '0') ? ne : da);
            binding.tvHubSpeed3.setTextColor(da);
        });
    }

    public void update(int fuel, int hp, int engine, int light, int lock) {
        activity.runOnUiThread(() -> {
            binding.containerCarDoors.setBackgroundResource(lock == 1 ? R.drawable.background_enabled_car : R.drawable.background_disabled_car);
            binding.containerEnabledCar.setBackgroundResource(engine == 1 ? R.drawable.background_enabled_car : R.drawable.background_disabled_car);
            binding.containerCarHeadlights.setBackgroundResource(light == 1 ? R.drawable.background_enabled_car : R.drawable.background_disabled_car);

         //   System.out.printf("fuel %d, hp %d", fuel, hp);
            binding.progressHp.setProgress(hp / 10);
            binding.progressAmountFuel.setProgress(Math.min(fuel, 100));
          //  binding.progressHp.setProgressCompat(Math.round(hp / 10f), true);
           // binding.progressAmountFuel.setProgressCompat(Math.round(fuel / 1.5f), true);
        });
    }

    public enum eTurnLightState {
        TURNLIGHT_OFF,
        TURNLIGHT_LEFT,
        TURNLIGHT_RIGHT,
        TURNLIGHT_ALL;
    }
}
