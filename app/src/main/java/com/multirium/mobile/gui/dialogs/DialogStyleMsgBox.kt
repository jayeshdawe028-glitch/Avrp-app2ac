package com.multirium.mobile.gui.dialogs

import android.view.View
import com.multirium.mobile.gui.NativeGui
import com.multirium.mobile.gui.util.Utils
import com.multirium.mobile.databinding.DialogMsgboxBinding
import kotlin.jvm.internal.Reflection

class DialogStyleMsgBox(var caption: String, var content: String, var leftBtnText: String, var rightBtnText: String) : NativeGui<DialogMsgboxBinding>(Reflection.getOrCreateKotlinClass(DialogMsgboxBinding::class.java), false, false,true, true) {
    val bind: DialogMsgboxBinding = getBinding() as DialogMsgboxBinding
    init {
        run {
            bind.caption.text = Utils.transfromColors(caption)
            bind.contentText.text = Utils.transfromColors(content)
            bind.leftButtText.text = Utils.transfromColors(leftBtnText)
            bind.leftButtText.setOnClickListener {
                Dialog.INSTANCE.sendResponse(1, -1, "");
            }

            if (rightBtnText.isEmpty()) {
                (getBinding() as DialogMsgboxBinding).rightButtText.visibility = View.GONE
                return@run
            }
            bind.rightButtText.text = Utils.transfromColors(rightBtnText)
            bind.rightButtText.setOnClickListener {
                Dialog.INSTANCE.sendResponse(0, -1, "");
            }
        }
    }
    override fun receivePacket(type: Int, data: String?) {
        TODO("Not yet implemented")
    }
}