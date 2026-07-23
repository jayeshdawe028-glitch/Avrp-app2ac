package com.multirium.mobile.common.utils

import android.content.Context
import androidx.recyclerview.widget.LinearLayoutManager
import kotlin.jvm.internal.Intrinsics

class NoScrollLinearLayoutManager(context: Context?, i: Int) :
    LinearLayoutManager(context, i, false) {
    override fun canScrollHorizontally(): Boolean {
        return false
    }

    override fun canScrollVertically(): Boolean {
        return false
    }

    init {
        Intrinsics.checkNotNullParameter(context, "context")
    }

    companion object {
        const val `$stable`: Int = 0
    }
}
