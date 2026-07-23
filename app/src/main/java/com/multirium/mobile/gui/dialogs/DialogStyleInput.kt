package com.multirium.mobile.gui.dialogs

import android.view.MotionEvent
import android.view.View
import android.view.View.OnTouchListener
import android.widget.FrameLayout
import androidx.constraintlayout.widget.ConstraintLayout
import com.multirium.mobile.R
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.DialogStyleInputBinding
import com.multirium.mobile.gui.NativeGui
import com.multirium.mobile.gui.util.Utils
import com.nvidia.devtech.NvEventQueueActivity
import kotlin.jvm.internal.Reflection

class DialogStyleInput(var dialogTypeId: Int,var caption: String, var content: String, var leftBtnText: String, var rightBtnText: String) : NativeGui<DialogStyleInputBinding>(Reflection.getOrCreateKotlinClass(DialogStyleInputBinding::class.java), false, false,true, true){
    init {
        kotlin.run {
            if (dialogTypeId == 1) {
                (getBinding() as DialogStyleInputBinding).input.inputType = 1
            } else if (dialogTypeId == 3) {
                (getBinding() as DialogStyleInputBinding).input.inputType = 128
            } else if (dialogTypeId == 6) {
                (getBinding() as DialogStyleInputBinding).input.inputType = 2
            }
            (getBinding() as DialogStyleInputBinding).caption.text =
                Utils.transfromColors(this.caption)
            (getBinding() as DialogStyleInputBinding).contentText.text = Utils.transfromColors(this.content)
            (getBinding() as DialogStyleInputBinding).leftButtText.text =
                Utils.transfromColors(this.leftBtnText)
            (getBinding() as DialogStyleInputBinding).leftButton.setOnClickListener {
                Dialog.INSTANCE.sendResponse(1, -1, (getBinding() as DialogStyleInputBinding).input.text.toString())
            }
            if (this.rightBtnText.isEmpty()) {
                (getBinding() as DialogStyleInputBinding).rightButton.visibility = View.GONE
                return@run
            }
            (getBinding() as DialogStyleInputBinding).rightButtText.text = Utils.transfromColors(this.rightBtnText)
            (getBinding() as DialogStyleInputBinding).rightButton.setOnClickListener {
                Dialog.INSTANCE.sendResponse(0, -1, (getBinding() as DialogStyleInputBinding).input.text.toString())
            }
            /*
            (getBinding() as DialogStyleInputBinding).input.setEnterKey {
                (getBinding() as DialogStyleInputBinding).input.onEditorAction(6)
                (getBinding() as DialogStyleInputBinding).input.keyboard.clear()
                (getBinding() as DialogStyleInputBinding).input.keyboard.setUIVisible(false)
                //Dialog.INSTANCE.sendResponse(1, -1, (getBinding() as DialogStyleInputBinding).input.text.toString())

                var layoutParamss = (getBinding() as DialogStyleInputBinding).root.layoutParams as FrameLayout.LayoutParams
                layoutParamss.bottomMargin = activity.resources.getDimensionPixelSize(R.dimen._1sdp)
                (getBinding() as DialogStyleInputBinding).root.layoutParams = layoutParamss
            }
            (getBinding() as DialogStyleInputBinding).input.setOnTouchListener(object : OnTouchListener {
                override fun onTouch(v: View?, event: MotionEvent): Boolean {
                    if ((getBinding() as DialogStyleInputBinding).input.useCustomView)
                        Utils.INSTANCE.fixInputKeyboard((getBinding() as DialogStyleInputBinding).root)
                    return v?.onTouchEvent(event) ?: true
                }
            })*/
        }
    }

    override fun receivePacket(type: Int, data: String?) {
        TODO("Not yet implemented")
    }
}