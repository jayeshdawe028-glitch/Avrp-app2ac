package com.multirium.mobile.gui.auth

import android.text.Editable
import android.text.TextWatcher
import android.view.View
import com.multirium.mobile.R
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.LoginLayoutBinding
import com.multirium.mobile.gui.NativeGui
import com.multirium.mobile.services.ActivityService
import com.multirium.mobile.services.impl.ActivityServiceImpl
import org.ini4j.Wini
import java.io.File
import java.io.IOException
import kotlin.jvm.internal.Reflection

class Autorization(
    private var isEmail: Boolean,
    private var isAutoAuth: Boolean,
    private var skinId: Int
) : NativeGui<LoginLayoutBinding>(
    Reflection.getOrCreateKotlinClass(LoginLayoutBinding::class.java),
    false, false, true, true
) {

    companion object {
        @JvmStatic external fun nativeInit()
    }

    private val bind: LoginLayoutBinding = getBinding() as LoginLayoutBinding
    private val activityService: ActivityService = ActivityServiceImpl()

    external fun onLoginClick(pass: String)
    external fun ToggleAutoLogin(toggle: Boolean)

    init {
        activity.runOnUiThread {
            setupNickname()
            setupCharacterImage()
            setupLoginButton()
            setupPasswordWatcher()
            setupRecoveryLink()
        }
    }

    fun show(isEmail: Boolean, isAutoAuth: Boolean, skinId: Int) {
        this.isEmail = isEmail
        this.isAutoAuth = isAutoAuth
        this.skinId = skinId

        activity.runOnUiThread {
            bind.tvRecover.visibility = if (isEmail) View.VISIBLE else View.GONE
        }
    }

    override fun receivePacket(type: Int, data: String?) {}

    private fun setupNickname() {
        bind.tvNickname.text = loadNicknameFromSettings()
    }

    private fun setupCharacterImage() {
        bind.loginPerson.setImageResource(R.drawable.login_pers)
    }

    private fun setupLoginButton() {
        bind.btnLogin.setOnClickListener {
            val password = bind.passInput.text.toString().trim()

            android.util.Log.i("AUTH", "Click login, passLen=${password.length}")

            if (password.length >= 6) {
                try {
                    onLoginClick(password)
                } catch (e: Throwable) {
                    android.util.Log.e("AUTH", "onLoginClick failed", e)
                    activityService.showErrorMessage("Ошибка native вызова", activity)
                }
            } else {
                activityService.showErrorMessage("Длина пароля минимум 6 символов", activity)
            }
        }
    }

    private fun setupPasswordWatcher() {
        bind.passInput.addTextChangedListener(object : TextWatcher {
            override fun afterTextChanged(s: Editable?) {}
            override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}
            override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {
                bind.btnLogin.alpha = if (bind.passInput.text.length >= 6) 1.0f else 0.5f
            }
        })
    }

    private fun setupRecoveryLink() {
        bind.tvRecover.visibility = if (isEmail) View.VISIBLE else View.GONE
        bind.tvRecover.setOnClickListener {
            activityService.showErrorMessage("Функция восстановления пароля пока недоступна", activity)
        }
    }

    private fun loadNicknameFromSettings(): String {
        val iniFile = File(activity.getExternalFilesDir(null), "SAMP/settings.ini")
        return if (iniFile.exists()) {
            try {
                val w = Wini(iniFile)
                w.get("client", "name", String::class.java)?.trim()
                    .takeUnless { it.isNullOrBlank() } ?: "Player"
            } catch (e: IOException) {
                "Player"
            }
        } else {
            "Player"
        }
    }
}
