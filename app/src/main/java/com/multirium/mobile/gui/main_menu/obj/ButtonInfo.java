package com.multirium.mobile.gui.main_menu.obj;
public class ButtonInfo {
    private float height;
    private int id;
    private float width;
    private float x;
    private float y;

    public static ButtonInfo copy$default(ButtonInfo buttonInfo, int i, float f, float f2, float f3, float f4, int i2, Object obj) {
        if ((i2 & 1) != 0) {
            i = buttonInfo.id;
        }
        if ((i2 & 2) != 0) {
            f = buttonInfo.x;
        }
        float f5 = f;
        if ((i2 & 4) != 0) {
            f2 = buttonInfo.y;
        }
        float f6 = f2;
        if ((i2 & 8) != 0) {
            f3 = buttonInfo.width;
        }
        float f7 = f3;
        if ((i2 & 16) != 0) {
            f4 = buttonInfo.height;
        }
        return buttonInfo.copy(i, f5, f6, f7, f4);
    }

    public int component1() {
        return this.id;
    }

    public float component2() {
        return this.x;
    }

    public float component3() {
        return this.y;
    }

    public float component4() {
        return this.width;
    }

    public float component5() {
        return this.height;
    }

    public ButtonInfo copy(int i, float f, float f2, float f3, float f4) {
        return new ButtonInfo(i, f, f2, f3, f4);
    }

    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof ButtonInfo)) {
            return false;
        }
        ButtonInfo buttonInfo = (ButtonInfo) obj;
        return this.id == buttonInfo.id && Float.compare(this.x, buttonInfo.x) == 0 && Float.compare(this.y, buttonInfo.y) == 0 && Float.compare(this.width, buttonInfo.width) == 0 && Float.compare(this.height, buttonInfo.height) == 0;
    }

    public int hashCode() {
        return (((((((this.id * 31) + Float.floatToIntBits(this.x)) * 31) + Float.floatToIntBits(this.y)) * 31) + Float.floatToIntBits(this.width)) * 31) + Float.floatToIntBits(this.height);
    }

    public String toString() {
        return "ButtonInfo(id=" + this.id + ", x=" + this.x + ", y=" + this.y + ", width=" + this.width + ", height=" + this.height + ')';
    }

    public ButtonInfo(int id, float x, float y, float width, float height) {
        this.id = id;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
    }

    public int getId() {
        return this.id;
    }

    public float getX() {
        return this.x;
    }

    public void setX(float f) {
        this.x = f;
    }

    public float getY() {
        return this.y;
    }

    public void setY(float f) {
        this.y = f;
    }

    public float getWidth() {
        return this.width;
    }

    public void setWidth(float f) {
        this.width = f;
    }

    public float getHeight() {
        return this.height;
    }

    public void setHeight(float f) {
        this.height = f;
    }

}
