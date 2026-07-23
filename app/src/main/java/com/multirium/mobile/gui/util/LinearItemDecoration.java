package com.multirium.mobile.gui.util;

import android.graphics.Rect;
import android.view.View;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
public class LinearItemDecoration extends RecyclerView.ItemDecoration {
    public static final int $stable = 0;
    private final float itemHeightPercent;
    private final float itemWidthPercent;
    private final float marginRightPercent;

    public LinearItemDecoration(float Width, float Height, float marginRight) {
        this.itemWidthPercent = Width;
        this.itemHeightPercent = Height;
        this.marginRightPercent = marginRight;
    }

    public void getItemOffsets(Rect rect, View view, RecyclerView recyclerView, RecyclerView.State state) {
        int width = recyclerView.getWidth();
        int height = recyclerView.getHeight();
        float f = (float) width;
        int i = (int) (this.itemWidthPercent * f);
        int i2 = (int) (((float) height) * this.itemHeightPercent);
        RecyclerView.LayoutManager layoutManager = recyclerView.getLayoutManager();
        if (recyclerView.getChildAdapterPosition(view) + 1 == ((LinearLayoutManager) layoutManager).getItemCount()) {
            rect.right = 0;
        } else {
            rect.right = (int) (f * this.marginRightPercent);
        }
        view.getLayoutParams().width = i;
        view.getLayoutParams().height = i2;
    }
}
