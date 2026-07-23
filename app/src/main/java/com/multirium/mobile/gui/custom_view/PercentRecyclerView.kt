package com.multirium.mobile.gui.custom_view

import android.content.Context
import android.graphics.Rect
import android.util.AttributeSet
import android.view.View
import androidx.recyclerview.widget.RecyclerView
import com.multirium.mobile.R
import kotlin.jvm.internal.DefaultConstructorMarker
import kotlin.jvm.internal.Intrinsics

class PercentRecyclerView(context: Context, attributeSet: AttributeSet?, i: Int) :
    RecyclerView(context, attributeSet, i) {
    private val itemHeightMultiplier: Float
    private val itemHeightPercent: Float
    private val itemWidthPercent: Float
    private val marginBottomPercent: Float
    private val marginLeftPercent: Float
    private val marginRightPercent: Float
    private val marginTopPercent: Float

    constructor(context: Context) : this(
        context,
        null as AttributeSet?,
        0,
        6,
        null as DefaultConstructorMarker?
    ) {
        Intrinsics.checkNotNullParameter(context, "context")
    }

    constructor(context: Context, attributeSet: AttributeSet?) : this(
        context,
        attributeSet,
        0,
        4,
        null as DefaultConstructorMarker?
    ) {
        Intrinsics.checkNotNullParameter(context, "context")
    }

    constructor(
        context: Context,
        attributeSet: AttributeSet?,
        i: Int,
        i2: Int,
        defaultConstructorMarker: DefaultConstructorMarker?
    ) : this(context, if ((i2 and 2) != 0) null else attributeSet, if ((i2 and 4) != 0) 0 else i)

    init {
        Intrinsics.checkNotNullParameter(context, "context")
        val obtainStyledAttributes =
            context.obtainStyledAttributes(attributeSet, R.styleable.PercentRecyclerView)
        Intrinsics.checkNotNullExpressionValue(
            obtainStyledAttributes,
            "obtainStyledAttributes(...)"
        )
        this.itemWidthPercent =
            obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_itemWidthPercent, 0.0f)
        this.itemHeightPercent =
            obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_itemHeightPercent, 0.0f)
        this.itemHeightMultiplier = obtainStyledAttributes.getFloat(
            R.styleable.PercentRecyclerView_itemHeightMultiplier,
            0.0f
        )
        this.marginTopPercent =
            obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_marginTopPercent, 0.0f)
        this.marginBottomPercent = obtainStyledAttributes.getFloat(
            R.styleable.PercentRecyclerView_marginBottomPercent,
            0.0f
        )
        this.marginLeftPercent =
            obtainStyledAttributes.getFloat(R.styleable.PercentRecyclerView_marginLeftPercent, 0.0f)
        this.marginRightPercent = obtainStyledAttributes.getFloat(
            R.styleable.PercentRecyclerView_marginRightPercent,
            0.0f
        )
        obtainStyledAttributes.recycle()
        addItemDecoration(
            PercentItemDecoration(
                this.itemWidthPercent,
                this.itemHeightPercent,
                this.itemHeightMultiplier,
                this.marginTopPercent,
                this.marginBottomPercent,
                this.marginLeftPercent,
                this.marginRightPercent
            )
        )
    }

    class PercentItemDecoration(
        private val itemWidthPercent: Float,
        private val itemHeightPercent: Float,
        private val itemHeightMultiplier: Float,
        private val marginTopPercent: Float,
        private val marginBottomPercent: Float,
        private val marginLeftPercent: Float,
        private val marginRightPercent: Float
    ) :
        ItemDecoration() {
        override fun getItemOffsets(
            rect: Rect,
            view: View,
            recyclerView: RecyclerView,
            state: State
        ) {
            var i: Int
            val f: Float
            Intrinsics.checkNotNullParameter(rect, "outRect")
            Intrinsics.checkNotNullParameter(view, "view")
            Intrinsics.checkNotNullParameter(recyclerView, "parent")
            Intrinsics.checkNotNullParameter(state, "state")
            val width = recyclerView.width
            val height = recyclerView.height
            val f2 = this.itemWidthPercent
            val i2 =
                if (f2 == 0.0f) ((height.toFloat()) * this.itemHeightPercent).toInt() else ((width.toFloat()) * f2).toInt()
            var f3 = this.itemHeightMultiplier
            if (f3 == 0.0f) {
                f3 = this.itemHeightPercent
                if (f3 == 0.0f) {
                    i = i2
                    val f4 = height.toFloat()
                    val f5 = width.toFloat()
                    val i3 = (this.marginLeftPercent * f5).toInt()
                    val i4 = (f5 * this.marginRightPercent).toInt()
                    view.layoutParams.width = i2
                    view.layoutParams.height = i
                    rect[i3, (this.marginTopPercent * f4).toInt(), i4] =
                        (f4 * this.marginBottomPercent).toInt()
                }
                f = height.toFloat()
            } else {
                f = i2.toFloat()
            }
            i = (f * f3).toInt()
            val f42 = height.toFloat()
            val f52 = width.toFloat()
            val i32 = (this.marginLeftPercent * f52).toInt()
            val i42 = (f52 * this.marginRightPercent).toInt()
            view.layoutParams.width = i2
            view.layoutParams.height = i
            rect[i32, (this.marginTopPercent * f42).toInt(), i42] =
                (f42 * this.marginBottomPercent).toInt()
        }

        companion object {
            const val `$stable`: Int = 0
        }
    }

    companion object {
        const val `$stable`: Int = 8
    }
}
