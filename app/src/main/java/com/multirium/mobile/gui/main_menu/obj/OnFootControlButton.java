package com.multirium.mobile.gui.main_menu.obj;

import kotlin.jvm.internal.Intrinsics;
public class OnFootControlButton {
    public static int $stable = 8;
    private ButtonInfo enterAim;
    private ButtonInfo enterCar;
    private ButtonInfo exitAim;
    private ButtonInfo fireGun;
    private ButtonInfo fireGun2;
    private ButtonInfo fist;
    private ButtonInfo sprint;
    private ButtonInfo voice;

    public static OnFootControlButton copy$default(OnFootControlButton onFootControlButton, ButtonInfo buttonInfo, ButtonInfo buttonInfo2, ButtonInfo buttonInfo3, ButtonInfo buttonInfo4, ButtonInfo buttonInfo5, ButtonInfo buttonInfo6, ButtonInfo buttonInfo7, int i, Object obj) {
        if ((i & 1) != 0) {
            buttonInfo = onFootControlButton.sprint;
        }
        if ((i & 2) != 0) {
            buttonInfo2 = onFootControlButton.fist;
        }
        ButtonInfo buttonInfo8 = buttonInfo2;
        if ((i & 4) != 0) {
            buttonInfo3 = onFootControlButton.enterCar;
        }
        ButtonInfo buttonInfo9 = buttonInfo3;
        if ((i & 8) != 0) {
            buttonInfo4 = onFootControlButton.voice;
        }
        ButtonInfo buttonInfo10 = buttonInfo4;
        if ((i & 16) != 0) {
            buttonInfo5 = onFootControlButton.enterAim;
        }
        ButtonInfo buttonInfo11 = buttonInfo5;
        if ((i & 32) != 0) {
            buttonInfo6 = onFootControlButton.exitAim;
        }
        ButtonInfo buttonInfo12 = buttonInfo6;
        if ((i & 64) != 0) {
            buttonInfo7 = onFootControlButton.fireGun;
        }
        return onFootControlButton.copy(buttonInfo, buttonInfo8, buttonInfo9, buttonInfo10, buttonInfo11, buttonInfo12, buttonInfo7, buttonInfo8);
    }

    public ButtonInfo component1() {
        return this.sprint;
    }

    public ButtonInfo component2() {
        return this.fist;
    }

    public ButtonInfo component3() {
        return this.enterCar;
    }

    public ButtonInfo component4() {
        return this.voice;
    }

    public ButtonInfo component5() {
        return this.enterAim;
    }

    public ButtonInfo component6() {
        return this.exitAim;
    }

    public ButtonInfo component7() {
        return this.fireGun;
    }

    public OnFootControlButton copy(ButtonInfo buttonInfo, ButtonInfo buttonInfo2, ButtonInfo buttonInfo3, ButtonInfo buttonInfo4, ButtonInfo buttonInfo5, ButtonInfo buttonInfo6, ButtonInfo buttonInfo7, ButtonInfo buttonInfo8) {
        Intrinsics.checkNotNullParameter(buttonInfo, "sprint");
        Intrinsics.checkNotNullParameter(buttonInfo2, "fist");
        Intrinsics.checkNotNullParameter(buttonInfo3, "enterCar");
        Intrinsics.checkNotNullParameter(buttonInfo4, "voice");
        Intrinsics.checkNotNullParameter(buttonInfo5, "enterAim");
        Intrinsics.checkNotNullParameter(buttonInfo6, "exitAim");
        Intrinsics.checkNotNullParameter(buttonInfo8, "fireGun");
        return new OnFootControlButton(buttonInfo, buttonInfo2, buttonInfo3, buttonInfo4, buttonInfo5, buttonInfo6, buttonInfo7, buttonInfo8);
    }

    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof OnFootControlButton)) {
            return false;
        }
        OnFootControlButton onFootControlButton = (OnFootControlButton) obj;
        return Intrinsics.areEqual((Object) this.sprint, (Object) onFootControlButton.sprint) && Intrinsics.areEqual((Object) this.fist, (Object) onFootControlButton.fist) && Intrinsics.areEqual((Object) this.enterCar, (Object) onFootControlButton.enterCar) && Intrinsics.areEqual((Object) this.voice, (Object) onFootControlButton.voice) && Intrinsics.areEqual((Object) this.enterAim, (Object) onFootControlButton.enterAim) && Intrinsics.areEqual((Object) this.exitAim, (Object) onFootControlButton.exitAim) && Intrinsics.areEqual((Object) this.fireGun, (Object) onFootControlButton.fireGun);
    }

    public int hashCode() {
        return (((((((((((this.sprint.hashCode() * 31) + this.fist.hashCode()) * 31) + this.enterCar.hashCode()) * 31) + this.voice.hashCode()) * 31) + this.enterAim.hashCode()) * 31) + this.exitAim.hashCode()) * 31) + this.fireGun.hashCode();
    }

    public String toString() {
        return "OnFootControlButton(sprint=" + this.sprint + ", fist=" + this.fist + ", enterCar=" + this.enterCar + ", voice=" + this.voice + ", enterAim=" + this.enterAim + ", exitAim=" + this.exitAim + ", fireGun=" + this.fireGun + ')';
    }

    public OnFootControlButton(ButtonInfo buttonInfo, ButtonInfo buttonInfo2, ButtonInfo buttonInfo3, ButtonInfo buttonInfo4, ButtonInfo buttonInfo5, ButtonInfo buttonInfo6, ButtonInfo buttonInfo7, ButtonInfo buttonInfo8) {
        Intrinsics.checkNotNullParameter(buttonInfo, "sprint");
        Intrinsics.checkNotNullParameter(buttonInfo2, "fist");
        Intrinsics.checkNotNullParameter(buttonInfo3, "enterCar");
        Intrinsics.checkNotNullParameter(buttonInfo4, "voice");
        Intrinsics.checkNotNullParameter(buttonInfo5, "enterAim");
        Intrinsics.checkNotNullParameter(buttonInfo6, "exitAim");
        Intrinsics.checkNotNullParameter(buttonInfo7, "fireGun");
        Intrinsics.checkNotNullParameter(buttonInfo8, "fireGun2");
        this.sprint = buttonInfo;
        this.fist = buttonInfo2;
        this.enterCar = buttonInfo3;
        this.voice = buttonInfo4;
        this.enterAim = buttonInfo5;
        this.exitAim = buttonInfo6;
        this.fireGun = buttonInfo7;
        this.fireGun2 = buttonInfo8;
    }

    public ButtonInfo getSprint() {
        return this.sprint;
    }

    public ButtonInfo getFist() {
        return this.fist;
    }

    public ButtonInfo getEnterCar() {
        return this.enterCar;
    }

    public ButtonInfo getVoice() {
        return this.voice;
    }

    public ButtonInfo getEnterAim() {
        return this.enterAim;
    }

    public ButtonInfo getExitAim() {
        return this.exitAim;
    }

    public ButtonInfo getFireGun() {
        return this.fireGun;
    }
    public ButtonInfo getFireGun2() {
        return this.fireGun2;
    }
}
