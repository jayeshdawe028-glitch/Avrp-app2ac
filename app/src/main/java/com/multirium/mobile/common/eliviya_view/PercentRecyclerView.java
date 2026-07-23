package com.multirium.mobile.common.eliviya_view;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.View;
import androidx.recyclerview.widget.RecyclerView;
import com.multirium.mobile.R;
import kotlin.jvm.internal.DefaultConstructorMarker;
import kotlin.jvm.internal.Intrinsics;

public final class PercentRecyclerView extends RecyclerView {
    private float itemHeightMultiplier;
    private float itemHeightPercent;
    private float itemWidthPercent;
    private float marginBottomPercent;
    private float marginLeftPercent;
    private float marginRightPercent;
    private float marginTopPercent;

    public PercentRecyclerView(Context context) {
        this(context, (AttributeSet) null, 0, 6, (DefaultConstructorMarker) null);
        Intrinsics.checkNotNullParameter(context, "context");
    }

    public PercentRecyclerView(Context context, AttributeSet attributeSet) {
        this(context, attributeSet, 0, 4, (DefaultConstructorMarker) null);
        Intrinsics.checkNotNullParameter(context, "context");
    }

    public PercentRecyclerView(Context context, AttributeSet attributeSet, int i, int i2, DefaultConstructorMarker defaultConstructorMarker) {
        this(context, (i2 & 2) != 0 ? null : attributeSet, (i2 & 4) != 0 ? 0 : i);
    }

    public PercentRecyclerView(Context context, AttributeSet attributeSet, int i) {
        super(context, attributeSet, i);
        Intrinsics.checkNotNullParameter(context, "context");
        TypedArray obtainStyledAttributes = context.obtainStyledAttributes(attributeSet, R.styleable.PercentRecyclerView);
        Intrinsics.checkNotNullExpressionValue(obtainStyledAttributes, "obtainStyledAttributes(...)");
        this.itemWidthPercent = obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_itemWidthPercent, 0.0f);
        this.itemHeightPercent = obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_itemHeightPercent, 0.0f);
        this.itemHeightMultiplier = obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_itemHeightMultiplier, 0.0f);
        this.marginTopPercent = obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_marginTopPercent, 0.0f);
        this.marginBottomPercent = obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_marginBottomPercent, 0.0f);
        this.marginLeftPercent = obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_marginLeftPercent, 0.0f);
        this.marginRightPercent = obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_marginRightPercent, 0.0f);
        obtainStyledAttributes.recycle();
        addItemDecoration(new PercentItemDecoration(this.itemWidthPercent, this.itemHeightPercent, this.itemHeightMultiplier, this.marginTopPercent, this.marginBottomPercent, this.marginLeftPercent, this.marginRightPercent));
    }

    public static final class PercentItemDecoration extends ItemDecoration {
        private final float itemHeightMultiplier;
        private final float itemHeightPercent;
        private final float itemWidthPercent;
        private final float marginBottomPercent;
        private final float marginLeftPercent;
        private final float marginRightPercent;
        private final float marginTopPercent;

        public PercentItemDecoration(float f, float f2, float f3, float f4, float f5, float f6, float f7) {
            this.itemWidthPercent = f;
            this.itemHeightPercent = f2;
            this.itemHeightMultiplier = f3;
            this.marginTopPercent = f4;
            this.marginBottomPercent = f5;
            this.marginLeftPercent = f6;
            this.marginRightPercent = f7;
        }

        public void getItemOffsets(Rect rect, View view, RecyclerView recyclerView, State state) {
            int i;
            float f;
            Intrinsics.checkNotNullParameter(rect, "outRect");
            Intrinsics.checkNotNullParameter(view, "view");
            Intrinsics.checkNotNullParameter(recyclerView, "parent");
            Intrinsics.checkNotNullParameter(state, "state");
            int width = recyclerView.getWidth();
            int height = recyclerView.getHeight();
            float f2 = this.itemWidthPercent;
            int i2 = f2 == 0.0f ? (int) (((float) height) * this.itemHeightPercent) : (int) (((float) width) * f2);
            float f3 = this.itemHeightMultiplier;
            if (f3 == 0.0f) {
                f3 = this.itemHeightPercent;
                if (f3 == 0.0f) {
                    i = i2;
                    float f4 = (float) height;
                    float f5 = (float) width;
                    int i3 = (int) (this.marginLeftPercent * f5);
                    int i4 = (int) (f5 * this.marginRightPercent);
                    view.getLayoutParams().width = i2;
                    view.getLayoutParams().height = i;
                    rect.set(i3, (int) (this.marginTopPercent * f4), i4, (int) (f4 * this.marginBottomPercent));
                }
                f = (float) height;
            } else {
                f = (float) i2;
            }
            i = (int) (f * f3);
            float f42 = (float) height;
            float f52 = (float) width;
            int i32 = (int) (this.marginLeftPercent * f52);
            int i42 = (int) (f52 * this.marginRightPercent);
            view.getLayoutParams().width = i2;
            view.getLayoutParams().height = i;
            rect.set(i32, (int) (this.marginTopPercent * f42), i42, (int) (f42 * this.marginBottomPercent));
        }
    }
}
