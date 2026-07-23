package com.multirium.mobile.gui.dialogs

import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.gui.NativeGui
import java.nio.charset.Charset

import kotlin.jvm.internal.Intrinsics

class Dialog {

    init {
        activity.runOnUiThread {

        }
    }

    private fun close() {
        val obj2 = obj
        val nativeGui = if (obj2 is NativeGui<*>) obj2 else null
        nativeGui?.destroy()
        obj = null
    }

    fun sendResponse(input: Int, i2: Int, str: String) {
        Intrinsics.checkNotNullParameter(str, "input")
        val forName = Charset.forName("windows-1251")
        val bytes = str.toByteArray(forName)
        nativeSendResponse(DialogId,input, i2, bytes)
        close()
    }
    companion object {
        @JvmStatic
        var DialogId:Int = 0
        @JvmStatic
        private external fun nativeSendResponse(id:Int,i: Int, i2: Int, bArr: ByteArray)
        @JvmStatic
        val BUTTON1: Int = 1
        @JvmStatic
        val BUTTON2: Int = 0
        @JvmStatic
        val INSTANCE: Dialog = Dialog()
        @JvmStatic
        val STYLE_INPUT: Int = 1
        @JvmStatic
        val STYLE_INPUT_NUMBER: Int = 6
        @JvmStatic
        val STYLE_INPUT_PASSWORD: Int = 3
        @JvmStatic
        val STYLE_LIST: Int = 2
        @JvmStatic
        val STYLE_MSGBOX: Int = 0
        @JvmStatic
        val STYLE_TABLIST: Int = 4
        @JvmStatic
        val STYLE_TABLIST_HEADER: Int = 5
        @JvmStatic
        var obj: Any? = null

        @JvmStatic
        fun show(dialogId:Int, dialogTypeId: Int, caption: String, content: String, leftBtnText: String, rightBtnText: String) {
            println("in dialogid $dialogId")
            activity.runOnUiThread {
                DialogId = dialogId
                INSTANCE.close()
                when (dialogTypeId) {
                    0 -> {
                        obj = DialogStyleMsgBox(caption, content, leftBtnText, rightBtnText)
                        return@runOnUiThread
                    }

                    1, 3, 6 -> {
                        obj = DialogStyleInput(dialogTypeId, caption, content, leftBtnText, rightBtnText)
                        return@runOnUiThread
                    }

                    2 -> {
                        obj = DialogStyleList(caption, content, leftBtnText, rightBtnText)
                        return@runOnUiThread
                    }

                    4 -> {
                        obj = DialogStyleTabList(
                            caption,
                            content,
                            leftBtnText,
                            rightBtnText,
                            false
                        )
                        return@runOnUiThread
                    }

                    5 -> {
                        obj = DialogStyleTabList(caption, content, leftBtnText, rightBtnText, true)
                        return@runOnUiThread
                    }

                    else -> return@runOnUiThread
                }
            }
        }
    }
}