package com.multirium.mobile.gui.util;

import android.content.Context;
import androidx.recyclerview.widget.LinearLayoutManager;
public  class NoScrollLinearLayoutManager extends LinearLayoutManager {

    public boolean canScrollHorizontally() {
        return false;
    }

    public boolean canScrollVertically() {
        return false;
    }

    public NoScrollLinearLayoutManager(Context context, int i) {
        super(context, i, false);
    }
}
