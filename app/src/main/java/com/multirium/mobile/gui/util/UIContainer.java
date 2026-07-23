package com.multirium.mobile.gui.util;

import android.content.Context;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.FrameLayout;

import java.util.ArrayList;
import java.util.Stack;
public class UIContainer extends FrameLayout {
    private Stack<Integer> mBackstack;
    private ArrayList<UILayout> mLayouts;
    private int mShowingUI;

    public UIContainer(Context context) {
        super(context);
        this.mLayouts = null;
        this.mBackstack = null;
        this.mShowingUI = -1;
    }

    public UIContainer(Context context, AttributeSet attributeSet) {
        super(context, attributeSet);
        this.mLayouts = null;
        this.mBackstack = null;
        this.mShowingUI = -1;
        this.mLayouts = new ArrayList<>();
        this.mBackstack = new Stack<>();
    }

    public UIContainer(Context context, AttributeSet attributeSet, int i) {
        super(context, attributeSet, i);
        this.mLayouts = null;
        this.mBackstack = null;
        this.mShowingUI = -1;
        this.mLayouts = new ArrayList<>();
        this.mBackstack = new Stack<>();
    }

    private void showLayout(int i) {
        int i2 = this.mShowingUI;
        if (i2 == i) {
            this.mLayouts.get(i).onLayoutShown();
            return;
        }
        if (i2 != -1) {
            this.mLayouts.get(i2).onLayoutClose();
        }
        removeAllViews();
        if (i != -1) {
            UILayout uILayout = this.mLayouts.get(i);
            if (uILayout.getView() == null) {
                uILayout.onCreateView((LayoutInflater) getContext().getSystemService("layout_inflater"), this);
            }
            if (uILayout.getView() != null) {
                if (uILayout.getView().getParent() != null) {
                    ((ViewGroup) uILayout.getView().getParent()).removeView(uILayout.getView());
                }
                addView(uILayout.getView());
            }
            uILayout.onLayoutShown();
        }
    }

    public UILayout getLayout(int i) {
        return this.mLayouts.get(i);
    }

    public void addLayout(int i, UILayout uILayout) {
        while (this.mLayouts.size() < i) {
            this.mLayouts.add(null);
        }
        this.mLayouts.add(i, uILayout);
        uILayout.setParent(this);
        uILayout.onCreateView((LayoutInflater) getContext().getSystemService("layout_inflater"), this);
    }

    public void addLazyLayout(int i, UILayout uILayout) {
        while (this.mLayouts.size() < i) {
            this.mLayouts.add(null);
        }
        this.mLayouts.add(i, uILayout);
        uILayout.setParent(this);
    }

    public void removeLayout(int i) {
        if (i == this.mShowingUI) {
            this.mShowingUI = -1;
            showLayout(-1);
        }
        this.mLayouts.remove(i);
        for (int i2 = 0; i2 < this.mBackstack.size(); i2++) {
            if (((Integer) this.mBackstack.get(i2)).intValue() == i) {
                this.mBackstack.remove(i2);
            }
        }
    }

    public void setCurrentLayout(int i) {
        int i2 = this.mShowingUI;
        if (i2 != -1) {
            this.mBackstack.push(Integer.valueOf(i2));
        }
        showLayout(i);
        this.mShowingUI = i;
    }

    public void closeCurrentLayout() throws Exception {
        if (this.mShowingUI != -1) {
            int intValue = this.mBackstack.pop().intValue();
            this.mShowingUI = intValue;
            showLayout(intValue);
            return;
        }
        throw new Exception("Invalid operation");
    }

    public void replaceCurrentLayout(int i) {
        this.mShowingUI = i;
        showLayout(i);
    }

    public int getCurrentLayoutId() {
        return this.mShowingUI;
    }

    /* access modifiers changed from: protected */
    public void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        int i = this.mShowingUI;
        if (i != -1) {
            this.mLayouts.get(i).onLayoutClose();
        }
    }
}
