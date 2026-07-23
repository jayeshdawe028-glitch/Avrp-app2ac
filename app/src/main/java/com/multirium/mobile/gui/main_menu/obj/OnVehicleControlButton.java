package com.multirium.mobile.gui.main_menu.obj;

import kotlin.jvm.internal.Intrinsics;
public class OnVehicleControlButton {
    public static int $stable = 8;
    private ButtonInfo acceleration;
    private ButtonInfo brake;
    private ButtonInfo enterCar;
    private ButtonInfo handBrake;
    private ButtonInfo horn;
    private ButtonInfo leftArrow;
    private ButtonInfo nitro;
    private ButtonInfo rightArrow;

    public static OnVehicleControlButton copy$default(OnVehicleControlButton onVehicleControlButton, ButtonInfo buttonInfo, ButtonInfo buttonInfo2, ButtonInfo buttonInfo3, ButtonInfo buttonInfo4, ButtonInfo buttonInfo5, ButtonInfo buttonInfo6, ButtonInfo buttonInfo7, ButtonInfo buttonInfo8, int i, Object obj) {
        OnVehicleControlButton onVehicleControlButton2 = onVehicleControlButton;
        int i2 = i;
        return onVehicleControlButton.copy((i2 & 1) != 0 ? onVehicleControlButton2.brake : buttonInfo, (i2 & 2) != 0 ? onVehicleControlButton2.acceleration : buttonInfo2, (i2 & 4) != 0 ? onVehicleControlButton2.enterCar : buttonInfo3, (i2 & 8) != 0 ? onVehicleControlButton2.horn : buttonInfo4, (i2 & 16) != 0 ? onVehicleControlButton2.handBrake : buttonInfo5, (i2 & 32) != 0 ? onVehicleControlButton2.leftArrow : buttonInfo6, (i2 & 64) != 0 ? onVehicleControlButton2.rightArrow : buttonInfo7, (i2 & 128) != 0 ? onVehicleControlButton2.nitro : buttonInfo8);
    }

    public ButtonInfo component1() {
        return this.brake;
    }

    public ButtonInfo component2() {
        return this.acceleration;
    }

    public ButtonInfo component3() {
        return this.enterCar;
    }

    public ButtonInfo component4() {
        return this.horn;
    }

    public ButtonInfo component5() {
        return this.handBrake;
    }

    public ButtonInfo component6() {
        return this.leftArrow;
    }

    public ButtonInfo component7() {
        return this.rightArrow;
    }

    public ButtonInfo component8() {
        return this.nitro;
    }

    public OnVehicleControlButton copy(ButtonInfo buttonInfo, ButtonInfo buttonInfo2, ButtonInfo buttonInfo3, ButtonInfo buttonInfo4, ButtonInfo buttonInfo5, ButtonInfo buttonInfo6, ButtonInfo buttonInfo7, ButtonInfo buttonInfo8) {
        Intrinsics.checkNotNullParameter(buttonInfo, "brake");
        Intrinsics.checkNotNullParameter(buttonInfo2, "acceleration");
        Intrinsics.checkNotNullParameter(buttonInfo3, "enterCar");
        Intrinsics.checkNotNullParameter(buttonInfo4, "horn");
        Intrinsics.checkNotNullParameter(buttonInfo5, "handBrake");
        ButtonInfo buttonInfo9 = buttonInfo6;
        Intrinsics.checkNotNullParameter(buttonInfo9, "leftArrow");
        ButtonInfo buttonInfo10 = buttonInfo7;
        Intrinsics.checkNotNullParameter(buttonInfo10, "rightArrow");
        ButtonInfo buttonInfo11 = buttonInfo8;
        Intrinsics.checkNotNullParameter(buttonInfo11, "nitro");
        return new OnVehicleControlButton(buttonInfo, buttonInfo2, buttonInfo3, buttonInfo4, buttonInfo5, buttonInfo9, buttonInfo10, buttonInfo11);
    }

    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof OnVehicleControlButton)) {
            return false;
        }
        OnVehicleControlButton onVehicleControlButton = (OnVehicleControlButton) obj;
        return Intrinsics.areEqual((Object) this.brake, (Object) onVehicleControlButton.brake) && Intrinsics.areEqual((Object) this.acceleration, (Object) onVehicleControlButton.acceleration) && Intrinsics.areEqual((Object) this.enterCar, (Object) onVehicleControlButton.enterCar) && Intrinsics.areEqual((Object) this.horn, (Object) onVehicleControlButton.horn) && Intrinsics.areEqual((Object) this.handBrake, (Object) onVehicleControlButton.handBrake) && Intrinsics.areEqual((Object) this.leftArrow, (Object) onVehicleControlButton.leftArrow) && Intrinsics.areEqual((Object) this.rightArrow, (Object) onVehicleControlButton.rightArrow) && Intrinsics.areEqual((Object) this.nitro, (Object) onVehicleControlButton.nitro);
    }

    public int hashCode() {
        return (((((((((((((this.brake.hashCode() * 31) + this.acceleration.hashCode()) * 31) + this.enterCar.hashCode()) * 31) + this.horn.hashCode()) * 31) + this.handBrake.hashCode()) * 31) + this.leftArrow.hashCode()) * 31) + this.rightArrow.hashCode()) * 31) + this.nitro.hashCode();
    }

    public String toString() {
        return "OnVehicleControlButton(brake=" + this.brake + ", acceleration=" + this.acceleration + ", enterCar=" + this.enterCar + ", horn=" + this.horn + ", handBrake=" + this.handBrake + ", leftArrow=" + this.leftArrow + ", rightArrow=" + this.rightArrow + ", nitro=" + this.nitro + ')';
    }

    public OnVehicleControlButton(ButtonInfo buttonInfo, ButtonInfo buttonInfo2, ButtonInfo buttonInfo3, ButtonInfo buttonInfo4, ButtonInfo buttonInfo5, ButtonInfo buttonInfo6, ButtonInfo buttonInfo7, ButtonInfo buttonInfo8) {
        Intrinsics.checkNotNullParameter(buttonInfo, "brake");
        Intrinsics.checkNotNullParameter(buttonInfo2, "acceleration");
        Intrinsics.checkNotNullParameter(buttonInfo3, "enterCar");
        Intrinsics.checkNotNullParameter(buttonInfo4, "horn");
        Intrinsics.checkNotNullParameter(buttonInfo5, "handBrake");
        Intrinsics.checkNotNullParameter(buttonInfo6, "leftArrow");
        Intrinsics.checkNotNullParameter(buttonInfo7, "rightArrow");
        Intrinsics.checkNotNullParameter(buttonInfo8, "nitro");
        this.brake = buttonInfo;
        this.acceleration = buttonInfo2;
        this.enterCar = buttonInfo3;
        this.horn = buttonInfo4;
        this.handBrake = buttonInfo5;
        this.leftArrow = buttonInfo6;
        this.rightArrow = buttonInfo7;
        this.nitro = buttonInfo8;
    }

    public ButtonInfo getBrake() {
        return this.brake;
    }

    public ButtonInfo getAcceleration() {
        return this.acceleration;
    }

    public ButtonInfo getEnterCar() {
        return this.enterCar;
    }

    public ButtonInfo getHorn() {
        return this.horn;
    }

    public ButtonInfo getHandBrake() {
        return this.handBrake;
    }

    public ButtonInfo getLeftArrow() {
        return this.leftArrow;
    }

    public ButtonInfo getRightArrow() {
        return this.rightArrow;
    }

    public ButtonInfo getNitro() {
        return this.nitro;
    }
}
