package com.multirium.mobile.gui.keyboard.adapter

import android.view.View
import androidx.recyclerview.widget.RecyclerView
import com.multirium.mobile.databinding.SampKeyboardLayoutBinding
import kotlin.jvm.internal.Intrinsics

class CommandViewHolder(view: View) : RecyclerView.ViewHolder(view) {
    @JvmField
    val binding: SampKeyboardLayoutBinding

    init {
        Intrinsics.checkNotNullParameter(view, "view")
        val bind = SampKeyboardLayoutBinding.bind(view)
        Intrinsics.checkNotNullExpressionValue(bind, "bind(...)")
        this.binding = bind
    }

    companion object {
        const val `$stable`: Int = 8
    }
}
