package com.multirium.mobile.gui.main_menu.utils;

import static com.multirium.mobile.core.Samp.activity;

import org.apache.commons.io.IOUtils;
import com.multirium.mobile.gui.main_menu.obj.ButtonInfo;
import com.multirium.mobile.gui.main_menu.obj.OnFootControlButton;
import com.multirium.mobile.gui.main_menu.obj.OnVehicleControlButton;
import com.multirium.mobile.gui.main_menu.obj.Widget;
import com.multirium.mobile.launcher.util.JsonUtilKt;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Map;

import kotlin.TuplesKt;
import kotlin.collections.MapsKt;
import kotlin.jvm.JvmStatic;
import kotlin.jvm.internal.Intrinsics;
public  class SettingGameButtonUtil {
    public static final SettingGameButtonUtil INSTANCE = new SettingGameButtonUtil();
    public static final String ON_FOOT_CONTROL_BUTTON = "on_foot_settings";
    public static final String ON_VEHICLE_CONTROL_BUTTON = "on_vehicle_settings";
    public static native void updateGameButton(int id, float X, float Y, float width, float height);
    private SettingGameButtonUtil() {
    }

    public static  void updateGameButtondefault(int i, float f, float f2, float f3, float f4, int i2) {
        updateGameButton(i, f, f2, (i2 & 8) != 0 ? 24.0f : f3, (i2 & 16) != 0 ? 24.0f : f4);
    }

    @JvmStatic
    public static void loadDefaultSetting() {
        SettingMenuUtil.loadSettings();
        SettingGameButtonUtil settingGameButtonUtil = INSTANCE;
        OnFootControlButton onFootControlButton = (OnFootControlButton) JsonUtilKt.toModel(settingGameButtonUtil.loadJsonFromFile(ON_FOOT_CONTROL_BUTTON), OnFootControlButton.class);
        OnVehicleControlButton onVehicleControlButton = (OnVehicleControlButton) JsonUtilKt.toModel(settingGameButtonUtil.loadJsonFromFile(ON_VEHICLE_CONTROL_BUTTON), OnVehicleControlButton.class);
        updateGameButtondefault(Widget.WIDGET_BUTTON_SPRINT.getValue(), onFootControlButton.getSprint().getX(), onFootControlButton.getSprint().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_ATTACK.getValue(), onFootControlButton.getFist().getX(), onFootControlButton.getFist().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_ENTER_CAR.getValue(), onFootControlButton.getEnterCar().getX(), onFootControlButton.getEnterCar().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_PHONE.getValue(), onFootControlButton.getVoice().getX(), onFootControlButton.getVoice().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_ENTER_TARGETING.getValue(), onFootControlButton.getEnterAim().getX(), onFootControlButton.getEnterAim().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_EXIT_TARGETING.getValue(), onFootControlButton.getExitAim().getX(), onFootControlButton.getExitAim().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_BRAKE.getValue(), onVehicleControlButton.getBrake().getX(), onVehicleControlButton.getBrake().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_ACCELERATE.getValue(), onVehicleControlButton.getAcceleration().getX(), onVehicleControlButton.getAcceleration().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_ENTER_CAR.getValue(), onVehicleControlButton.getEnterCar().getX(), onVehicleControlButton.getEnterCar().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_HORN.getValue(), onVehicleControlButton.getHorn().getX(), onVehicleControlButton.getHorn().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_HANDBRAKE.getValue(), onVehicleControlButton.getHandBrake().getX(), onVehicleControlButton.getHandBrake().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_VEHICLE_STEER_LEFT.getValue(), onVehicleControlButton.getLeftArrow().getX(), onVehicleControlButton.getLeftArrow().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_VEHICLE_STEER_RIGHT.getValue(), onVehicleControlButton.getRightArrow().getX(), onVehicleControlButton.getRightArrow().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_NITRO.getValue(), onVehicleControlButton.getNitro().getX(), onVehicleControlButton.getNitro().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_VC_SHOOT.getValue(), onFootControlButton.getFireGun().getX(), onFootControlButton.getFireGun().getY(), 0.0f, 0.0f, 24);
        updateGameButtondefault(Widget.WIDGET_VC_SHOOT_ALT.getValue(), onFootControlButton.getFireGun2().getX(), onFootControlButton.getFireGun2().getY(), 0.0f, 0.0f, 24);
    }

    public final String loadJsonFromFile(String str) {
        String str2;
        Intrinsics.checkNotNullParameter(str, "filename");
        File file = new File(activity.getExternalFilesDir(null).toString() + "/", "SAMP/" + str + ".json");
        if (!file.exists()) {
            if (Intrinsics.areEqual((Object) str, (Object) ON_FOOT_CONTROL_BUTTON)) {
                str2 = getDefaultOnFootJson();
            } else if (Intrinsics.areEqual((Object) str, (Object) ON_VEHICLE_CONTROL_BUTTON)) {
                str2 = getDefaultOnVehicleJson();
            } else {
                str2 = "";
            }
            File parentFile = file.getParentFile();
            if (parentFile != null) {
                parentFile.mkdirs();
            }
            try {
                FileWriter writer = new FileWriter(file);
                writer.write(str2);
                writer.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        try {
            FileInputStream inputStream = new FileInputStream(file);
            try {
                return IOUtils.toString(inputStream);
            } finally {
                inputStream.close();
            }
        }catch (IOException f){
            return "";
        }

    }

    public  void saveJsonToFile(String jsonData, String filename) {
        File file = new File(activity.getExternalFilesDir(null).toString() + "/", "SAMP/" + filename + ".json");
        File parentFile = file.getParentFile();
        if (parentFile != null) {
            parentFile.mkdirs();
        }
        try {
            FileWriter writer = new FileWriter(file);
            writer.write(jsonData);
            writer.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void setDefaultButtons() {
        saveJsonToFile(getDefaultOnFootJson(), ON_FOOT_CONTROL_BUTTON);
        saveJsonToFile(getDefaultOnVehicleJson(), ON_VEHICLE_CONTROL_BUTTON);
        loadDefaultSetting();
    }

    private String getDefaultOnFootJson() {
        return JsonUtilKt.toStringJson(new OnFootControlButton(new ButtonInfo(Widget.WIDGET_BUTTON_SPRINT.getValue(), 585.9756f, 367.08588f, 28.0f, 28.0f), new ButtonInfo(Widget.WIDGET_ATTACK.getValue(), 591.187f, 256.70453f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_ENTER_CAR.getValue(), 601.88617f, 159.39899f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_PHONE.getValue(), 66.731705f, 223.73233f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_ENTER_TARGETING.getValue(), 492.7805f, 224.62122f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_EXIT_TARGETING.getValue(), 517.6f, 258.42963f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_VC_SHOOT.getValue(), 463.4634f, 310.0101f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_VC_SHOOT_ALT.getValue(), 76.731705f, 253.73233f, 24.0f, 24.0f)));
    }

    private String getDefaultOnVehicleJson() {
        return JsonUtilKt.toStringJson(new OnVehicleControlButton(new ButtonInfo(Widget.WIDGET_BRAKE.getValue(), 526.4f, 380.80002f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_ACCELERATE.getValue(), 586.93335f, 378.31113f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_ENTER_CAR.getValue(), 590.4065f, 176.03789f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_HORN.getValue(), 499.73984f, 251.48232f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_HANDBRAKE.getValue(), 589.8211f, 277.98108f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_VEHICLE_STEER_LEFT.getValue(), 43.512196f, 330.5277f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_VEHICLE_STEER_RIGHT.getValue(), 120.71545f, 330.28787f, 24.0f, 24.0f), new ButtonInfo(Widget.WIDGET_NITRO.getValue(), 524.03253f, 152.27525f, 16.0f, 16.0f)));
    }
    public ButtonInfo getSetting(Widget widget) {
        OnFootControlButton onFootControlButton = (OnFootControlButton) JsonUtilKt.toModel(loadJsonFromFile(ON_FOOT_CONTROL_BUTTON), OnFootControlButton.class);
        OnVehicleControlButton onVehicleControlButton = (OnVehicleControlButton) JsonUtilKt.toModel(loadJsonFromFile(ON_VEHICLE_CONTROL_BUTTON), OnVehicleControlButton.class);
        Map mapOf = MapsKt.mapOf(TuplesKt.to(Widget.WIDGET_BUTTON_SPRINT, onFootControlButton.getSprint()), TuplesKt.to(Widget.WIDGET_ATTACK, onFootControlButton.getFist()), TuplesKt.to(Widget.WIDGET_ENTER_CAR, onFootControlButton.getEnterCar()), TuplesKt.to(Widget.WIDGET_PHONE, onFootControlButton.getVoice()), TuplesKt.to(Widget.WIDGET_ENTER_TARGETING, onFootControlButton.getEnterAim()), TuplesKt.to(Widget.WIDGET_EXIT_TARGETING, onFootControlButton.getExitAim()), TuplesKt.to(Widget.WIDGET_VC_SHOOT, onFootControlButton.getFireGun()), TuplesKt.to(Widget.WIDGET_VC_SHOOT_ALT, onFootControlButton.getFireGun2()));
        Map mapOf2 = MapsKt.mapOf(TuplesKt.to(Widget.WIDGET_BRAKE, onVehicleControlButton.getBrake()), TuplesKt.to(Widget.WIDGET_ACCELERATE, onVehicleControlButton.getAcceleration()), TuplesKt.to(Widget.WIDGET_ENTER_CAR, onVehicleControlButton.getEnterCar()), TuplesKt.to(Widget.WIDGET_HORN, onVehicleControlButton.getHorn()), TuplesKt.to(Widget.WIDGET_HANDBRAKE, onVehicleControlButton.getHandBrake()), TuplesKt.to(Widget.WIDGET_VEHICLE_STEER_LEFT, onVehicleControlButton.getLeftArrow()), TuplesKt.to(Widget.WIDGET_VEHICLE_STEER_RIGHT, onVehicleControlButton.getRightArrow()), TuplesKt.to(Widget.WIDGET_NITRO, onVehicleControlButton.getNitro()));
        ButtonInfo buttonInfo = (ButtonInfo) mapOf.get(widget);
        return buttonInfo == null ? (ButtonInfo) mapOf2.get(widget) : buttonInfo;
    }
}
