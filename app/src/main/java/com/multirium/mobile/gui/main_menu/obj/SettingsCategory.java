package com.multirium.mobile.gui.main_menu.obj;
public class SettingsCategory {
    boolean isActive;
    String name;
    public SettingsCategory(String name, boolean isActive){
        this.isActive = isActive;
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public boolean isActive() {
        return isActive;
    }

    public void setName(String name) {
        this.name = name;
    }

    public void setActive(boolean active) {
        isActive = active;
    }
}
