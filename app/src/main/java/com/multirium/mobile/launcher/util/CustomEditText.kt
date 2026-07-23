package com.multirium.mobile.launcher.util

import android.annotation.SuppressLint
import android.content.Context
import android.util.AttributeSet
import android.view.KeyEvent
import android.widget.EditText
import com.multirium.mobile.core.Samp

@SuppressLint("AppCompatCustomView")
class CustomEditText(context: Context?, attrs: AttributeSet?) :
    EditText(context, attrs) {
    private var mContext: Context? = null

    init {
        mContext = context
    }

    fun SetBackListener(context: Context?) {
        mContext = context
    }

    override fun onKeyPreIme(keyCode: Int, event: KeyEvent): Boolean {
        if (keyCode == KeyEvent.KEYCODE_BACK) {
            (mContext as Samp).onEventBackPressed()
            return true
        }
        return false
    }
}
