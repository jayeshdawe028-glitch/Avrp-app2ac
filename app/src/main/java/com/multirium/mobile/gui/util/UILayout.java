package com.multirium.mobile.gui.util;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
public abstract class UILayout {
    private UIContainer mParent = null;

    public abstract View onCreateView(LayoutInflater layoutInflater, ViewGroup viewGroup);
    public abstract View getView();

    public abstract void onLayoutClose();

    public abstract void onLayoutShown();

    public void setParent(UIContainer uIContainer) {
        this.mParent = uIContainer;
    }

    public Context getContext() {
        return this.mParent.getContext();
    }
}
