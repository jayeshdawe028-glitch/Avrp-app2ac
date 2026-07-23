package com.multirium.mobile.gui.main_menu.utils;


import static com.multirium.mobile.core.Samp.activity;

import static com.multirium.mobile.gui.main_menu.utils.SettingGameButtonUtil.updateGameButton;

import android.util.Log;

import com.multirium.mobile.gui.main_menu.obj.OnFootControlButton;
import com.multirium.mobile.gui.main_menu.obj.OnVehicleControlButton;
import com.multirium.mobile.gui.main_menu.obj.SettingCategory;

import com.multirium.mobile.gui.main_menu.obj.SettingParamId;
import com.multirium.mobile.gui.main_menu.obj.SettingsType;
import com.multirium.mobile.gui.main_menu.utils.SettingGameButtonUtil;
import com.multirium.mobile.gui.main_menu.obj.SettingDeep;
import com.multirium.mobile.gui.main_menu.obj.SettingParam;
import com.multirium.mobile.launcher.util.JsonUtilKt;
import org.apache.commons.io.IOUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import kotlin.Unit;
import kotlin.collections.CollectionsKt;
import kotlin.collections.MapsKt;
import kotlin.jvm.JvmStatic;
import kotlin.jvm.functions.Function0;
import kotlin.ranges.RangesKt;

public  class SettingMenuUtil {
    public static  String AUDIO_SETTINGS = "audio_settings.json";
    public static  String CONTROL_SETTINGS = "control_settings.json";
    public static  String GAME_SETTINGS = "game_settings.json";
    public static  String GRAPHIC_SETTINGS = "graphic_settings.json";
    public static  String PERSONALIZATION_SETTINGS = "personalization_settings.json";
    public static  SettingMenuUtil INSTANCE = new SettingMenuUtil();
    private static final List<SettingParam> settingsCategoryAudio = CollectionsKt.mutableListOf(
            new SettingParam(SettingParamId.MS_SFXVolume.ordinal(), SettingsType.SEEKBAR, "СПЕЦЭФФЕКТЫ", CollectionsKt.listOf(new SettingDeep((String) null, 70, true, 0, 100, true, (Function0<Unit>) null))),
            new SettingParam(SettingParamId.MUSIC_VOLUME.ordinal(), SettingsType.SEEKBAR, "МУЗЫКА", CollectionsKt.listOf(new SettingDeep((String) null, 70, true, 0, 100, true, (Function0<Unit>) null))),
            new SettingParam(SettingParamId.MS_HRTF.ordinal(), SettingsType.CHECKBOX, "ОБЪЕМНЫЙ ЗВУК", CollectionsKt.listOf(new SettingDeep("ВЫКЛ.", 0, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("ВКЛ.", 1, true, 0, 0, false, (Function0<Unit>) null))));
    private static final List<SettingParam> settingsCategoryControl = CollectionsKt.mutableListOf(
                    new SettingParam(0, SettingsType.BUTTON, "НАСТРОЙКА УПРАВЛЕНИЯ", CollectionsKt.listOf(new SettingDeep("", 0, false, 0, 0, false, (Function0<Unit>) null))),
                    new SettingParam(SettingParamId.MS_SteerType.ordinal(), SettingsType.CHECKBOX, "СЕНСОРНЫЙ РУЛЬ", CollectionsKt.listOf(new SettingDeep("КНОПКИ", 1, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("ДЖОЙСТИК", 0, true, 0, 0, false, (Function0<Unit>) null))),
                    new SettingParam(SettingParamId.MS_Accelerometer.ordinal(), SettingsType.CHECKBOX, "АКСЕЛЕРОМЕТР", CollectionsKt.listOf(new SettingDeep("ВЫКЛ.", 0, true, 0, 0, false, (Function0<Unit>) null), new SettingDeep("ВКЛ.", 1, false, 0, 0, false, (Function0<Unit>) null))));
    private static final List<SettingParam> settingsCategoryGame = CollectionsKt.mutableListOf(
            new SettingParam(SettingParamId.MS_CamHeight.ordinal(), SettingsType.CHECKBOX, "ВЫСОТА КАМЕРЫ", CollectionsKt.listOf(new SettingDeep("НИЗКО", 0, true, 0, 0, false, (Function0<Unit>) null), new SettingDeep("СРЕДНЕ", 0, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("ВЫСОКО", 1, false, 0, 0, false, (Function0<Unit>) null))),
            new SettingParam(SettingParamId.MS_VibrationFeedback.ordinal(), SettingsType.CHECKBOX, "ВИБРАЦИЯ", CollectionsKt.listOf(new SettingDeep("ВЫКЛ.", 0, true, 0, 0, false, (Function0<Unit>) null), new SettingDeep("ВКЛ.", 1, false, 0, 0, false, (Function0<Unit>) null))),
            new SettingParam(SettingParamId.FPS_LIMIT.ordinal(), SettingsType.CHECKBOX, "Кол-во FPS", CollectionsKt.listOf(new SettingDeep("30", 30, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("60", 60, true, 0, 0, false, (Function0<Unit>) null), new SettingDeep("120", 120, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("144", 144, false, 0, 0, false, (Function0<Unit>) null))));
    private static final List<SettingParam> settingsCategoryGraphic = CollectionsKt.mutableListOf(
            new SettingParam(SettingParamId.MS_Brightness.ordinal(), SettingsType.SEEKBAR, "ЯРКОСТЬ", CollectionsKt.listOf(new SettingDeep("", 60, true, 0, 100, true, (Function0<Unit>) null))),
            new SettingParam(SettingParamId.MS_Visuals.ordinal(), SettingsType.CHECKBOX, "СПЕЦЭФФЕКТЫ", CollectionsKt.listOf(new SettingDeep("НИЗК.", 1, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("СРЕДНЕ", 2, true, 0, 0, false, (Function0<Unit>) null), new SettingDeep("ВЫСОКО", 3, false, 0, 0, false, (Function0<Unit>) null))),
            new SettingParam(SettingParamId.MS_Resolution.ordinal(), SettingsType.SEEKBAR, "РАЗРЕШЕНИЕ", CollectionsKt.listOf(new SettingDeep("", 80, true, 0, 100, true, (Function0<Unit>) null))),
            new SettingParam(SettingParamId.MS_DrawDistance.ordinal(), SettingsType.SEEKBAR, "ДАЛЬНОСТЬ ПРОРИСОВКИ", CollectionsKt.listOf(new SettingDeep("", 50, true, 0, 100, true, (Function0<Unit>) null))),
            new SettingParam(SettingParamId.MS_CarReflections.ordinal(), SettingsType.CHECKBOX, "ОТРАЖЕНИЯ НА ТРАНСПОРТЕ", CollectionsKt.listOf(new SettingDeep("ВЫКЛ.", 0, true, 0, 0, false, (Function0<Unit>) null), new SettingDeep("НИЗК.", 1, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("СРЕДНЕ", 2, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("ВЫСОКО", 3, false, 0, 0, false, (Function0<Unit>) null))),
            new SettingParam(SettingParamId.MS_Shadows.ordinal(), SettingsType.CHECKBOX, "ТЕНИ", CollectionsKt.listOf(new SettingDeep("НИЗК.", 0, true, 0, 0, false, (Function0<Unit>) null), new SettingDeep("СРЕДНЕ", 1, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("ВЫСОКО", 2, false, 0, 0, false, (Function0<Unit>) null))));
    private static final List<SettingParam> settingsPersonalization = CollectionsKt.mutableListOf(
            new SettingParam(SettingParamId.KEYBOARD_SETTING.ordinal(), SettingsType.CHECKBOX, "Выбор клавиатуры", CollectionsKt.listOf(new SettingDeep("Multirium", 1, false, 0, 0, false, (Function0<Unit>) null), new SettingDeep("Системная", 2, false, 0, 0, false, (Function0<Unit>) null))));


    public static native void updateMenuParam(int id, int value);

    private SettingMenuUtil() {
    }


    public static void loadSettings() {
        Log.d("pausemenu", "loadSettings");
        List<SettingParam> loadSettings = SettingMenuUtil.INSTANCE.loadSettings(SettingCategory.AUDIO);
        List<SettingParam> loadSettings2 = SettingMenuUtil.INSTANCE.loadSettings(SettingCategory.CONTROL);
        List<SettingParam> loadSettings3 = SettingMenuUtil.INSTANCE.loadSettings(SettingCategory.GAME);

        Log.d("pausemenu", "loadSettings2");
        for (SettingParam settingParam : CollectionsKt.plus(CollectionsKt.plus(CollectionsKt.plus(loadSettings, loadSettings2), loadSettings3), SettingMenuUtil.INSTANCE.loadSettings(SettingCategory.GRAPHIC))) {
            for (SettingDeep settingDeep : settingParam.getDeeps()) {
                if (settingDeep.isActive()) {

                    int paramId = settingParam.getParamId();
                    int value = settingDeep.getValue();
                    updateMenuParam(paramId, value);
                }
            }
        }
    }

    public static void loadWidgetSettings(){
        OnVehicleControlButton onVehicleControlButton = (OnVehicleControlButton) JsonUtilKt.toModel(SettingGameButtonUtil.INSTANCE.loadJsonFromFile(SettingGameButtonUtil.ON_VEHICLE_CONTROL_BUTTON), OnVehicleControlButton.class);
        updateGameButton(onVehicleControlButton.getBrake().getId(), onVehicleControlButton.getBrake().getX(), onVehicleControlButton.getBrake().getY(), onVehicleControlButton.getBrake().getWidth(), onVehicleControlButton.getBrake().getHeight());
        updateGameButton(onVehicleControlButton.getAcceleration().getId(), onVehicleControlButton.getAcceleration().getX(), onVehicleControlButton.getAcceleration().getY(), onVehicleControlButton.getAcceleration().getWidth(), onVehicleControlButton.getAcceleration().getHeight());
        updateGameButton(onVehicleControlButton.getEnterCar().getId(), onVehicleControlButton.getEnterCar().getX(), onVehicleControlButton.getEnterCar().getY(), onVehicleControlButton.getEnterCar().getWidth(), onVehicleControlButton.getEnterCar().getHeight());
        updateGameButton(onVehicleControlButton.getHorn().getId(), onVehicleControlButton.getHorn().getX(), onVehicleControlButton.getHorn().getY(), onVehicleControlButton.getHorn().getWidth(), onVehicleControlButton.getHorn().getHeight());
        updateGameButton(onVehicleControlButton.getHandBrake().getId(), onVehicleControlButton.getHandBrake().getX(), onVehicleControlButton.getHandBrake().getY(), onVehicleControlButton.getHandBrake().getWidth(), onVehicleControlButton.getHandBrake().getHeight());
        updateGameButton(onVehicleControlButton.getLeftArrow().getId(), onVehicleControlButton.getLeftArrow().getX(), onVehicleControlButton.getLeftArrow().getY(), onVehicleControlButton.getLeftArrow().getWidth(), onVehicleControlButton.getLeftArrow().getHeight());
        updateGameButton(onVehicleControlButton.getRightArrow().getId(), onVehicleControlButton.getRightArrow().getX(), onVehicleControlButton.getRightArrow().getY(), onVehicleControlButton.getRightArrow().getWidth(), onVehicleControlButton.getRightArrow().getHeight());

        OnFootControlButton onFootControlButton = (OnFootControlButton) JsonUtilKt.toModel(SettingGameButtonUtil.INSTANCE.loadJsonFromFile(SettingGameButtonUtil.ON_FOOT_CONTROL_BUTTON), OnFootControlButton.class);
        updateGameButton(onFootControlButton.getEnterAim().getId(), onFootControlButton.getEnterAim().getX(), onFootControlButton.getEnterAim().getY(), onFootControlButton.getEnterAim().getWidth(), onFootControlButton.getEnterAim().getHeight());
        updateGameButton(onFootControlButton.getExitAim().getId(), onFootControlButton.getEnterAim().getX(), onFootControlButton.getEnterAim().getY(), onFootControlButton.getEnterAim().getWidth(), onFootControlButton.getEnterAim().getHeight());
        updateGameButton(onFootControlButton.getSprint().getId(), onFootControlButton.getSprint().getX(), onFootControlButton.getSprint().getY(), onFootControlButton.getSprint().getWidth(), onFootControlButton.getSprint().getHeight());
        updateGameButton(onFootControlButton.getVoice().getId(), onFootControlButton.getVoice().getX(), onFootControlButton.getVoice().getY(), onFootControlButton.getVoice().getWidth(), onFootControlButton.getVoice().getHeight());
        updateGameButton(onFootControlButton.getEnterCar().getId(), onFootControlButton.getEnterCar().getX(), onFootControlButton.getEnterCar().getY(), onFootControlButton.getEnterCar().getWidth(), onFootControlButton.getEnterCar().getHeight());
        updateGameButton(onFootControlButton.getFist().getId(), onFootControlButton.getFist().getX(), onFootControlButton.getFist().getY(), onFootControlButton.getFist().getWidth(), onFootControlButton.getFist().getHeight());
        updateGameButton(onFootControlButton.getFireGun().getId(), onFootControlButton.getFireGun().getX(), onFootControlButton.getFireGun().getY(), onFootControlButton.getFireGun().getWidth(), onFootControlButton.getFireGun().getHeight());
        if(onFootControlButton.getFireGun2() != null)
            updateGameButton(onFootControlButton.getFireGun2().getId(), onFootControlButton.getFireGun2().getX(), onFootControlButton.getFireGun2().getY(), onFootControlButton.getFireGun2().getWidth(), onFootControlButton.getFireGun2().getHeight());
    }

    public void saveJsonToFile(String json, String name) {
        File file = new File(activity.getExternalFilesDir(null).toString() + "/", "SAMP/" + name);

        File parentFile = file.getParentFile();
        if (parentFile != null) {
            parentFile.mkdirs();
        }
        try {
            FileWriter writer = new FileWriter(file);
            writer.write(json);
            writer.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public List<SettingParam> loadSettings(SettingCategory settingCategory) {
        String str = "";

        switch (settingCategory.ordinal()){
            case 0:{
                str = PERSONALIZATION_SETTINGS;
                break;
            }
            case 1:{
                str = CONTROL_SETTINGS;
                break;
            }
            case 2:{
                str = GRAPHIC_SETTINGS;
                break;
            }
            case 3:{
                str = AUDIO_SETTINGS;
                break;
            }
            case 4:{
                str = GAME_SETTINGS;
                break;
            }
            default:{
                break;
            }
        }

        List<SettingParam> parseJsonToSettings = parseJsonToSettings(loadJsonFromFile(str), settingCategory);
        List<SettingParam> list = getDefault(str);
        if (list == null) {
            list = CollectionsKt.emptyList();
        }
        List<SettingParam> mergeSettings = mergeSettings(parseJsonToSettings, list);
        saveJsonToFile(JsonUtilKt.toStringJson(mergeSettings), str);
        return mergeSettings;
    }
    private List<SettingParam> mergeSettings(List<SettingParam> list, List<SettingParam> list2) {
        List<SettingParam> arrayList = new ArrayList<>();

        Map<String, SettingParam> linkedHashMap = new LinkedHashMap<>();
        for (SettingParam settingParam : list) {
            linkedHashMap.put(settingParam.getTitle(), settingParam);
        }

        for (SettingParam settingParam : list2) {
            SettingParam existingSetting = linkedHashMap.get(settingParam.getTitle());
            if (existingSetting != null) {
                arrayList.add(existingSetting);
            } else {
                arrayList.add(settingParam);
            }
        }

        Set<String> titlesInList2 = new HashSet<>();
        for (SettingParam settingParam : list2) {
            titlesInList2.add(settingParam.getTitle());
        }

        List<SettingParam> removedSettings = new ArrayList<>();
        for (SettingParam settingParam : list) {
            if (!titlesInList2.contains(settingParam.getTitle())) {
                removedSettings.add(settingParam);
            }
        }

        if (!removedSettings.isEmpty()) {
            StringBuilder sb = new StringBuilder("Удалены устаревшие настройки: ");
            List<String> removedTitles = new ArrayList<>();
            for (SettingParam settingParam : removedSettings) {
                removedTitles.add(settingParam.getTitle());
            }
            sb.append(removedTitles);
            System.out.println(sb.toString());
        }

        return arrayList;
    }
    public void setDefaultSettings(SettingCategory settingCategory) {
        switch (settingCategory.ordinal()){
            case 0:{
                saveJsonToFile(JsonUtilKt.toStringJson(settingsPersonalization), PERSONALIZATION_SETTINGS);
                break;
            }
            case 1:{
                saveJsonToFile(JsonUtilKt.toStringJson(settingsCategoryControl), CONTROL_SETTINGS);
                break;
            }
            case 2:{
                saveJsonToFile(JsonUtilKt.toStringJson(settingsCategoryGraphic), GRAPHIC_SETTINGS);
                break;
            }
            case 3:{
                saveJsonToFile(JsonUtilKt.toStringJson(settingsCategoryAudio), AUDIO_SETTINGS);
                break;
            }
            case 4:{
                saveJsonToFile(JsonUtilKt.toStringJson(settingsCategoryGame), GAME_SETTINGS);
                break;
            }
            default:{
                break;
            }
        }

    }

    public static int getValueSetting(SettingCategory settingCategory, String name){
        List<SettingParam> settingParams = SettingMenuUtil.INSTANCE.loadSettings(settingCategory);
        for(SettingParam settings : settingParams){
            if(name.equals(settings.getTitle())){
                for(SettingDeep deep : settings.getDeeps()) {
                    if (deep.isActive()) {
                        return deep.getValue();
                    }
                }
            }
        }

        return -1;
    }

    private List<SettingParam> parseJsonToSettings(String str, SettingCategory settingCategory) {
        switch (settingCategory.ordinal()){
            case 0:{
                return JsonUtilKt.toListModel(str, SettingParam.class);
            }
            case 1:{
                return JsonUtilKt.toListModel(str, SettingParam.class);
            }
            case 2:{
                return JsonUtilKt.toListModel(str, SettingParam.class);
            }
            case 3:{
                return JsonUtilKt.toListModel(str, SettingParam.class);
            }
            case 4:{
                return JsonUtilKt.toListModel(str, SettingParam.class);
            }
            default:{
                break;
            }
        }

        return Collections.emptyList();
    }

    private String loadJsonFromFile(String str) {
        File file = new File(activity.getExternalFilesDir(null).toString() + "/", "SAMP/" + str);

        if (!file.exists()) {
            String defaultJson = getDefaultJson(str);
            File parentFile = file.getParentFile();
            if (parentFile != null) {
                parentFile.mkdirs();
            }
            try {
                FileWriter writer = new FileWriter(file);
                writer.write(defaultJson);
                writer.flush();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        String everything = null;
        try {
            FileInputStream inputStream = new FileInputStream(file);
            try {
                everything = IOUtils.toString(inputStream);
            } finally {
                inputStream.close();
            }
        }catch (IOException f){

        }

        return everything;
    }

    public List<SettingParam> getDefault(String str) {
        switch (str.hashCode()) {
            case -1622810376:
                if (str.equals(PERSONALIZATION_SETTINGS)) {
                    return settingsPersonalization;
                }
                break;
            case -800074170:
                if (str.equals(GAME_SETTINGS)) {
                    return settingsCategoryGame;
                }
                break;
            case -649931076:
                if (str.equals(GRAPHIC_SETTINGS)) {
                    return settingsCategoryGraphic;
                }
                break;
            case 212637809:
                if (str.equals(CONTROL_SETTINGS)) {
                    return settingsCategoryControl;
                }
                break;
            case 1814136618:
                if (str.equals(AUDIO_SETTINGS)) {
                    return settingsCategoryAudio;
                }
                break;
        }
        return null;
    }

    private String getDefaultJson(String str) {
        switch (str.hashCode()) {
            case -1622810376:
                if (str.equals(PERSONALIZATION_SETTINGS)) {
                    return JsonUtilKt.toStringJson(settingsPersonalization);
                }
                break;
            case -800074170:
                if (str.equals(GAME_SETTINGS)) {
                    return JsonUtilKt.toStringJson(settingsCategoryGame);
                }
                break;
            case -649931076:
                if (str.equals(GRAPHIC_SETTINGS)) {
                    return JsonUtilKt.toStringJson(settingsCategoryGraphic);
                }
                break;
            case 212637809:
                if (str.equals(CONTROL_SETTINGS)) {
                    return JsonUtilKt.toStringJson(settingsCategoryControl);
                }
                break;
            case 1814136618:
                if (str.equals(AUDIO_SETTINGS)) {
                    return JsonUtilKt.toStringJson(settingsCategoryAudio);
                }
                break;
        }
        return "";
    }
}
