package com.multirium.mobile.launcher.util

import android.content.Context
import android.util.AttributeSet
import android.view.MotionEvent
import androidx.recyclerview.widget.RecyclerView

class CustomRecyclerView : RecyclerView {
    var isEnableScrolling: Boolean = true

    constructor(context: Context) : super(context)

    constructor(context: Context, attrs: AttributeSet?) : super(context, attrs)

    constructor(context: Context, attrs: AttributeSet?, defStyle: Int) : super(
        context,
        attrs,
        defStyle
    )

    override fun onInterceptTouchEvent(motionEvent: MotionEvent): Boolean {
        if (isEnableScrolling) {
            return super@CustomRecyclerView.onInterceptTouchEvent(motionEvent)
        }
        return false
    }

    override fun onTouchEvent(motionEvent: MotionEvent): Boolean {
        if (isEnableScrolling) {
            return super@CustomRecyclerView.onTouchEvent(motionEvent)
        }
        return false
    }

    val scrollForRecycler: Int
        get() = computeVerticalScrollOffset()
}
