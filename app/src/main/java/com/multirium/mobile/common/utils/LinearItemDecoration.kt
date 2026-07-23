package com.multirium.mobile.common.utils

import android.graphics.Rect
import android.view.View
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import kotlin.jvm.internal.Intrinsics

class LinearItemDecoration(
    private val itemWidthPercent: Float,
    private val itemHeightPercent: Float,
    private val marginRightPercent: Float
) : RecyclerView.ItemDecoration() {
    override fun getItemOffsets(
        rect: Rect,
        view: View,
        recyclerView: RecyclerView,
        state: RecyclerView.State
    ) {
        Intrinsics.checkNotNullParameter(rect, "outRect")
        Intrinsics.checkNotNullParameter(view, "view")
        Intrinsics.checkNotNullParameter(recyclerView, "parent")
        Intrinsics.checkNotNullParameter(state, "state")
        val width = recyclerView.width
        val height = recyclerView.height
        val f = width.toFloat()
        val i = (this.itemWidthPercent * f).toInt()
        val i2 = ((height.toFloat()) * this.itemHeightPercent).toInt()
        val layoutManager = recyclerView.layoutManager
        Intrinsics.checkNotNull(
            layoutManager,
            "null cannot be cast to non-null type androidx.recyclerview.widget.LinearLayoutManager"
        )
        if (recyclerView.getChildAdapterPosition(view) + 1 == (layoutManager as LinearLayoutManager).itemCount) {
            rect.right = 0
        } else {
            rect.right = (f * this.marginRightPercent).toInt()
        }
        view.layoutParams.width = i
        view.layoutParams.height = i2
    }

    companion object {
        const val `$stable`: Int = 0
    }
}
