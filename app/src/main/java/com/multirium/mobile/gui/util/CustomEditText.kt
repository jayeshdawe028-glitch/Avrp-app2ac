package com.multirium.mobile.gui.util


import android.annotation.SuppressLint
import android.content.Context
import android.os.SystemClock
import android.util.AttributeSet
import android.util.Log
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.EditText
import com.multirium.mobile.R
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.gui.keyboard.SAMPKeyboard
import com.multirium.mobile.gui.main_menu.obj.SettingCategory
import com.multirium.mobile.gui.main_menu.utils.SettingMenuUtil.getValueSetting

import com.nvidia.devtech.NvEventQueueActivity


@SuppressLint("AppCompatCustomView")
class CustomEditText : EditText {
    var rootContent: ViewGroup
    var useCustomView = false
    var isKeyBoardShowS = false
    private var currentKeyboard = getValueSetting(SettingCategory.PERSONALIZATION, "Выбор клавиатуры")
    var keyboard: SAMPKeyboard

    constructor(context: Context) : super(context) {
        this.rootContent = activity.findViewById(android.R.id.content)
        //this.rootContent = NvEventQueueActivity.Instance.mAndroidUI

        // if(currentKeyboard == 1) {
        //    useCustomView = true
        //}
        keyboard = SAMPKeyboard(this)
        keyboard.onClickEnter = {
            onEditorAction(6)
            keyboard.clear()
            keyboard.setUIVisible(false)
        }

    }

    constructor(context: Context, attributeSet: AttributeSet) : super(context, attributeSet) {
        this.rootContent = activity.findViewById(android.R.id.content)
        //this.rootContent = NvEventQueueActivity.Instance.mAndroidUI

        // if(currentKeyboard == 1) {
        //   useCustomView = true
        // }

        keyboard = SAMPKeyboard(this)
        keyboard.onClickEnter = {
            onEditorAction(6)
            keyboard.clear()
            keyboard.setUIVisible(false)
        }

    }

    constructor(context: Context, attributeSet: AttributeSet, int: Int) : super(context, attributeSet, int) {
        this.rootContent = activity.findViewById(android.R.id.content)
        //this.rootContent = NvEventQueueActivity.Instance.mAndroidUI

        // if(currentKeyboard == 1) {
        //    useCustomView = true
        // }

        keyboard = SAMPKeyboard(this)
        keyboard.onClickEnter = {
            onEditorAction(6)
            keyboard.clear()
            keyboard.setUIVisible(false)
        }

    }
    fun setEnterKey(function0: Function0<Unit>){
        keyboard.onClickEnter = function0
    }
    fun setCustomKeyboardType(i: Int) {
        if (i in 0..1) {
            this.currentKeyboard = i
        }
    }
    fun hideAllKeyboards() {
        this.keyboard.setUIVisible(false)
    }

    fun simulateTouchEvent(i: Int) {
        val uptimeMillis: Long = SystemClock.uptimeMillis()
        val obtain = MotionEvent.obtain(uptimeMillis, uptimeMillis, i, 0.0f, 0.0f, 0)
        dispatchTouchEvent(obtain)
        obtain.recycle()
    }
    /*override fun onTouchEvent(motionEvent: MotionEvent): Boolean {
        if (motionEvent.action != 1 || !this.useCustomView) {
            return super.onTouchEvent(motionEvent)
        }
        showCustomView()
        return super.onTouchEvent(motionEvent)
    }*/
    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(motionEvent: MotionEvent): Boolean {
        if (motionEvent.action == MotionEvent.ACTION_UP && this.useCustomView) {
            showCustomView()
            return true
        }
        return super.onTouchEvent(motionEvent)
    }


    private fun showCustomView() {
        if (isKeyBoardShowS) return
        val childCount = rootContent.childCount
        var f = 0.0f
        for (i in 0 until childCount) {
            val elevation = rootContent.getChildAt(i).elevation
            if (elevation > f) {
                f = elevation
            }
        }

        requestFocus()
        setShowSoftInputOnFocus(false);
        //val view: ViewGroup = if (this.currentKeyboard == 0) this.keyboardSAMP.getView() else this.keyboardRussiaGameView.getView()
        val view: ViewGroup = this.keyboard.binding.root
        view.elevation = f + 1.0f

        if (rootContent.indexOfChild(view) == -1) {
            rootContent.addView(view, ViewGroup.LayoutParams(-1, -1))
            Log.d("CustumEditText", "Create keyboard view")
        }
        /*if (this.currentKeyboard == 0) {
            this.keyboardSAMP.setUIVisible(true)
        } else {
            this.keyboardRussiaGameView.setUIVisible(true)
        }*/
        keyboard.setUIVisible(true)
        isKeyBoardShowS = true
    }
}