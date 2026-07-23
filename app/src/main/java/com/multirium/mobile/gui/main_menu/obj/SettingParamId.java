package com.multirium.mobile.gui.main_menu.obj;

import java.util.Arrays;

public enum SettingParamId {
    MS_Visuals(0),
    MS_Resolution(1),
    MS_DrawDistance(2),
    MS_SteerType(3),
    MS_Traffic(4),
    MS_Shadows(5),
    MS_Targeting(6),
    MS_CarReflections(7),
    MS_Accelerometer(8),
    MS_TouchLayout(9),
    MS_CamHeight(10),
    MS_SFXVolume(11),
    MS_MusicVolume(12),
    MS_Autotune(13),
    MS_DefaultStation(14),
    MS_InvertLook(15),
    MS_Brightness(16),
    MS_Language(17),
    MS_BikeControl(18),
    MS_HUD(19),
    MS_SimulationSpeed(20),
    MS_CinematicCam(21),
    MS_DebugCam(22),
    MS_SteerAnalogScale(23),
    MS_Subtitles(24),
    MS_JoypadLookX(25),
    MS_JoypadLookY(26),
    MS_TweakAnalogLow(27),
    MS_TweakAnalogLowMax(28),
    MS_VibrationFeedback(29),
    MS_FrameLimiter(30),
    MS_MouseSensitivityX(31),
    MS_MouseSensitivityY(32),
    MS_HRTF(33),
    MS_XBOXControlScheme(34),
    MS_AutoClimb(35),
    MS_ReplayKit(36),
    NONE1(37),
    NONE2(38),
    FPS_LIMIT(39),
    CUSTOM_SKY(40),
    VOICE_VOLUME(41),
    MUSIC_VOLUME(42),
    RENDER_DEBUG(43),
    DAMAGE_INFORMER(44),
    KEYBOARD_TYPE(45),
    CHAT_HELPER(46),
    ACS_IN_CAR(47),
    VOICE_BUTTON_LOGIC(48),
    KEYBOARD_SETTING(49);

    private final int value;

    SettingParamId(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    private static final SettingParamId[] VALUES = values();

    public static SettingParamId of(int value) {
        return Arrays.stream(VALUES)
                .filter(param -> param.getValue() == value)
                .findFirst()
                .orElse(null);
    }
}