package com.multirium.mobile.gui.util

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.util.AttributeSet
import androidx.appcompat.widget.AppCompatTextView

class StrokedTextView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyle: Int = 0
) :
    AppCompatTextView(context, attrs, defStyle) {
    override fun onDraw(canvas: Canvas) {
        val p: Paint = paint
        p.style = Paint.Style.FILL
        super.onDraw(canvas)
        p.style = Paint.Style.STROKE
        p.strokeWidth = 4f
        setTextColor(Color.rgb(0, 0, 0))
        super.onDraw(canvas)
    }

    companion object {
        private const val DEFAULT_STROKE_WIDTH = 0

        /**
         * Convenience method to convert density independent pixel(dp) value
         * into device display specific pixel value.
         * @param context Context to access device specific display metrics
         * @param dp density independent pixel value
         * @return device specific pixel value.
         */
        fun dpToPx(context: Context, dp: Float): Int {
            val scale = context.resources.displayMetrics.density
            return (dp * scale + 0.5f).toInt()
        }
    }
}