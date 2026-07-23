package com.multirium.mobile.gui.util;

import com.google.firebase.messaging.Constants;
import kotlin.jvm.internal.DefaultConstructorMarker;
import kotlin.jvm.internal.Intrinsics;

public final class ConvertViewCoordsToGta {
    public static final ConvertViewCoordsToGta INSTANCE = new ConvertViewCoordsToGta();

    private ConvertViewCoordsToGta() {

    }
    public static final class Data {
        private float height;
        private float width;
        private float x;
        private float y;

        public Data() {
            this(0.0f, 0.0f, 0.0f, 0.0f, 15, (DefaultConstructorMarker) null);
        }

        public Data(float f, float f2, float f3, float f4) {
            this.x = f;
            this.y = f2;
            this.width = f3;
            this.height = f4;
        }

        public Data(float f, float f2, float f3, float f4, int i, DefaultConstructorMarker defaultConstructorMarker) {
            this((i & 1) != 0 ? 0.0f : f,
                    (i & 2) != 0 ? 0.0f : f2,
                    (i & 4) != 0 ? 0.0f : f3,
                    (i & 8) != 0 ? ((i & 4) != 0 ? 0.0f : f3) : f4);
        }

        public final float getX() {
            return this.x;
        }

        public final void setX(float f) {
            this.x = f;
        }

        public final float getY() {
            return this.y;
        }

        public final void setY(float f) {
            this.y = f;
        }

        public final float getWidth() {
            return this.width;
        }

        public final void setWidth(float f) {
            this.width = f;
        }

        public final float getHeight() {
            return this.height;
        }

        public final void setHeight(float f) {
            this.height = f;
        }
    }

    /*public final ConvertViewCoordsToGta.Data convertPercentToGta(ConvertViewCoordsToGta.Data data) {
        Intrinsics.checkNotNullParameter(data, Constants.ScionAnalytics.MessageType.DATA_MESSAGE);
        ConvertViewCoordsToGta.Data data2 = new ConvertViewCoordsToGta.Data();
        float intValue = (float) Utils.INSTANCE.getScreenSize().getFirst().intValue();
        float intValue2 = (float) Utils.INSTANCE.getScreenSize().getSecond().intValue();
        data2.setX(((data.getX() + (data.getWidth() / 2.0f)) / intValue) * 640.0f);
        data2.setY(((data.getY() + (data.getHeight() / 2.2f)) / intValue2) * 480.0f);
        data2.setWidth(((data.getWidth() / 2.0f) / intValue) * 640.0f);
        data2.setHeight(((data.getHeight() / 3.333f) / intValue2) * 480.0f);
        return data2;
    }

    public final ConvertViewCoordsToGta.Data convertCoordsToGta(ConvertViewCoordsToGta.Data data) {
        Intrinsics.checkNotNullParameter(data, Constants.ScionAnalytics.MessageType.DATA_MESSAGE);
        ConvertViewCoordsToGta.Data data2 = new ConvertViewCoordsToGta.Data();
        float intValue = (float) Utils.INSTANCE.getScreenSize().getFirst().intValue();
        float intValue2 = (float) Utils.INSTANCE.getScreenSize().getSecond().intValue();
        data2.setX(((data.getX() + (data.getWidth() / 2.0f)) / intValue) * 640.0f);
        data2.setY(((data.getY() + (data.getHeight() / 2.2f)) / intValue2) * 480.0f);
        data2.setWidth(((data.getWidth() / 2.0f) / intValue) * 640.0f);
        data2.setHeight(((data.getHeight() / 3.333f) / intValue2) * 480.0f);
        return data2;
    }*/
}