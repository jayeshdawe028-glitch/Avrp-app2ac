package com.multirium.mobile.gui.main_menu

import com.multirium.mobile.databinding.MenuMapLayoutBinding
import com.multirium.mobile.gui.NativeGui
import kotlin.jvm.internal.Reflection

class MenuMap : NativeGui<MenuMapLayoutBinding>(
    Reflection.getOrCreateKotlinClass(
        MenuMapLayoutBinding::class.java), false, false, true, false) {

    val bind = getBinding() as MenuMapLayoutBinding
    var exitCallback: Function0<Unit>? = null
    external fun toggleMap(toggle: Boolean)
    init {
        toggleMap(true)
        (getBinding() as MenuMapLayoutBinding).backBtn.setOnClickListener {
            super.destroy()
            toggleMap(false)
            exitCallback?.invoke()
        }
    }

    override fun receivePacket(type: Int, data: String?) {
        TODO("Not yet implemented")
    }
}