package com.multirium.mobile.gui.main_menu.obj;

import java.util.Arrays;

public enum SettingCategory {
    PERSONALIZATION(1),
    CONTROL(2),
    GRAPHIC(3),
    AUDIO(4),
    GAME(5);

    private final int id;

    SettingCategory(int id) {
        this.id = id;
    }

    public int getId() {
        return id;
    }

    private static final SettingCategory[] VALUES = values();

    public static SettingCategory of(int id) {
        return Arrays.stream(VALUES)
                .filter(category -> category.getId() == id)
                .findFirst()
                .orElse(null);
    }
}