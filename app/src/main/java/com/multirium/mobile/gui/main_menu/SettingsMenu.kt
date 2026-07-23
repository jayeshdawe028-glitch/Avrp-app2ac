package com.multirium.mobile.gui.pausemenu

import android.util.DisplayMetrics
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.SettingsMenuLayoutBinding
import com.multirium.mobile.gui.NativeGui
import com.multirium.mobile.gui.main_menu.obj.SettingCategory
import com.multirium.mobile.gui.main_menu.obj.SettingsCategory
import com.multirium.mobile.gui.main_menu.obj.SettingsType
import com.multirium.mobile.gui.main_menu.utils.SettingMenuUtil.updateMenuParam
import com.multirium.mobile.gui.main_menu.adapter.SettingsCategoryAdapter
import com.multirium.mobile.gui.main_menu.adapter.SettingsParamsAdapter
import com.multirium.mobile.gui.main_menu.obj.SettingDeep
import com.multirium.mobile.gui.main_menu.obj.SettingParam
import com.multirium.mobile.gui.main_menu.utils.SettingMenuUtil
import com.multirium.mobile.common.EasyAnimation
import com.multirium.mobile.launcher.util.JsonUtilKt
import java.util.Locale
import kotlin.jvm.internal.Intrinsics
import kotlin.jvm.internal.Reflection


class SettingsMenu : NativeGui<SettingsMenuLayoutBinding>(Reflection.getOrCreateKotlinClass(SettingsMenuLayoutBinding::class.java), false, false, true, false) {
    var displayMetrics: DisplayMetrics = DisplayMetrics()

    var screenHeight: Int = 0
    var screenWidth: Int = 0

    var settingsId: Int = 0

    var settingsMenuCategoryAdapter: SettingsCategoryAdapter = SettingsCategoryAdapter()

    var settingsMenuParamsAdapter: SettingsParamsAdapter
    lateinit var exitCallback: Function0<Unit>

    lateinit var AudioSettings: List<SettingParam>
    lateinit var ControlSettings: List<SettingParam>
    lateinit var GameSettings: List<SettingParam>
    lateinit var GraphicSettings: List<SettingParam>
    lateinit var PesonalizationSettings: List<SettingParam>

    val bind: SettingsMenuLayoutBinding = getBinding() as SettingsMenuLayoutBinding

    init {
        activity.windowManager.defaultDisplay.getMetrics(displayMetrics)
        screenWidth = displayMetrics.widthPixels

        screenHeight = displayMetrics.heightPixels

        this.settingsMenuParamsAdapter = SettingsParamsAdapter(screenWidth, screenHeight)
        for (settingParam in settingsOnFootOrVehicle) {
            for (callback in settingParam.deeps) {
                callback.callback = {
                    val settingsControl = SettingsControl()
                    setChildrenVisibility(bind.root, false)
                    val upperCase: String = settingParam.title.toUpperCase(Locale.ROOT)
                    if (Intrinsics.areEqual(upperCase as Any, "УПРАВЛЕНИЕ ПЕШКОМ" as Any)) {
                        settingsControl.showOnFootControl()
                    }
                    val upperCase2: String = settingParam.title.toUpperCase(Locale.ROOT)

                    if (Intrinsics.areEqual(upperCase2 as Any, "УПРАВЛЕНИЕ В АВТО" as Any)) {
                        settingsControl.showOnVehicleControl()
                    }
                    settingsControl.exitCallback = {
                        settingsControl.destroy()
                        setChildrenVisibility(bind.root, true)
                    }
                }
            }
        }
        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(bind.backBtn){
            if(exitCallback != null){
                exitCallback.invoke()
            }
        }

        bind.categoryRc.setLayoutManager(LinearLayoutManager(activity, RecyclerView.VERTICAL, false))
        bind.settingsRc.setLayoutManager(LinearLayoutManager(activity, RecyclerView.VERTICAL, false))

        bind.categoryRc.itemAnimator = null
        bind.settingsRc.itemAnimator = null

        bind.categoryRc.adapter = settingsMenuCategoryAdapter
        bind.settingsRc.adapter = settingsMenuParamsAdapter

    }

    fun showSettingsMenu() {
        activity.runOnUiThread {
            AudioSettings =
                SettingMenuUtil.INSTANCE.loadSettings(SettingCategory.AUDIO)
            ControlSettings =
                SettingMenuUtil.INSTANCE.loadSettings(SettingCategory.CONTROL)
            GameSettings =
                SettingMenuUtil.INSTANCE.loadSettings(SettingCategory.GAME)
            GraphicSettings =
                SettingMenuUtil.INSTANCE.loadSettings(SettingCategory.GRAPHIC)
            PesonalizationSettings =
                SettingMenuUtil.INSTANCE.loadSettings(SettingCategory.PERSONALIZATION)


            val i = this.settingsId
            if (i == 0) {
                settingsMenuParamsAdapter.addAll(PesonalizationSettings)
            } else if (i == 1) {
                settingsMenuParamsAdapter.addAll(ControlSettings)
            } else if (i == 2) {
                settingsMenuParamsAdapter.addAll(GraphicSettings)
            } else if (i == 3) {
                settingsMenuParamsAdapter.addAll(AudioSettings)
            } else if (i == 4) {
                settingsMenuParamsAdapter.addAll(GameSettings)
            }
            settingsMenuCategoryAdapter.addAll(
                listOf(
                    SettingsCategory("ПЕРСОНАЛИЗАЦИЯ", true),
                    SettingsCategory("УПРАВЛЕНИЕ", false),
                    SettingsCategory("ГРАФИКА", false),
                    SettingsCategory("АУДИО", false),
                    SettingsCategory("ИГРА", false)
                )
            )
            settingsMenuCategoryAdapter.onClick = { name, pos ->
                val upperCase: String = name.toUpperCase(Locale.ROOT)
                when (upperCase.hashCode()) {
                    -885903251 -> if (upperCase == "ГРАФИКА") {
                        settingsMenuParamsAdapter.addAll(GraphicSettings)
                    }

                    -89829555 -> if (upperCase == "ПЕРСОНАЛИЗАЦИЯ") {
                        settingsMenuParamsAdapter.addAll(PesonalizationSettings)
                    }

                    32257067 -> if (upperCase == "ИГРА") {
                        settingsMenuParamsAdapter.addAll(GameSettings)

                    }

                    668821946 -> if (upperCase == "УПРАВЛЕНИЕ") {
                        settingsMenuParamsAdapter.addAll(ControlSettings)

                    }

                    993047335 -> if (upperCase == "АУДИО") {
                        settingsMenuParamsAdapter.addAll(AudioSettings)
                    }
                }
                settingsId = pos
            }
            for (settingParam in PesonalizationSettings) {
                for (settingDeep in settingParam.deeps) {
                    settingDeep.callback = {
                        SettingMenuUtil.INSTANCE.saveJsonToFile(
                            JsonUtilKt.toStringJson(
                                settingsMenuParamsAdapter.list
                            ), SettingMenuUtil.PERSONALIZATION_SETTINGS
                        )
                    }
                }
            }
            for (settingParam2 in ControlSettings) {
                for (settingDeep2 in settingParam2.deeps) {
                    settingDeep2.callback = {
                        run {
                            val upperCase: String = settingParam2.title.toUpperCase(Locale.ROOT)

                            if (Intrinsics.areEqual(upperCase as Any, "НАСТРОЙКА УПРАВЛЕНИЯ" as Any))
                            {
                                settingsMenuParamsAdapter.addAll(settingsOnFootOrVehicle)
                                return@run
                            }
                        }
                        if (settingDeep2.isActive) {
                            updateMenuParam(
                                settingParam2.paramId,
                                settingDeep2.value
                            )
                        }
                        SettingMenuUtil.INSTANCE.saveJsonToFile(
                            JsonUtilKt.toStringJson(
                                settingsMenuParamsAdapter.list
                            ), SettingMenuUtil.CONTROL_SETTINGS
                        )
                    }
                }
            }
            for (settingParam3 in GameSettings) {
                for (settingDeep3 in settingParam3.deeps) {
                    settingDeep3.callback = {
                        if (settingDeep3.isActive) {
                            updateMenuParam(
                                settingParam3.paramId,
                                settingDeep3.value
                            )
                        }
                        SettingMenuUtil.INSTANCE.saveJsonToFile(
                            JsonUtilKt.toStringJson(
                                settingsMenuParamsAdapter.list
                            ), SettingMenuUtil.GAME_SETTINGS
                        )
                    }
                }
            }
            for (settingParam4 in AudioSettings) {
                for (settingDeep4 in settingParam4.deeps) {
                    settingDeep4.callback = {
                        if (settingDeep4.isActive) {
                            updateMenuParam(
                                settingParam4.paramId,
                                settingDeep4.value
                            )
                        }
                        SettingMenuUtil.INSTANCE.saveJsonToFile(
                            JsonUtilKt.toStringJson(
                                settingsMenuParamsAdapter.list
                            ), SettingMenuUtil.AUDIO_SETTINGS
                        )
                    }
                }
            }
            for (settingParam5 in GraphicSettings) {
                var i2 = 0
                for (next in settingParam5.deeps) {
                    val i3 = i2 + 1

                    next.callback = {
                        if (next.isActive) {
                            updateMenuParam(
                                settingParam5.paramId,
                                next.value
                            )
                        }
                        SettingMenuUtil.INSTANCE.saveJsonToFile(
                            JsonUtilKt.toStringJson(
                                settingsMenuParamsAdapter.list
                            ), SettingMenuUtil.GRAPHIC_SETTINGS
                        )
                    }
                    i2 = i3
                }
            }
        }
    }
    fun setChildrenVisibility(viewGroup: ViewGroup, z: Boolean) {
        viewGroup.visibility = if (z) View.VISIBLE else View.GONE
    }
    override fun receivePacket(type: Int, data: String?) {
        TODO("Not yet implemented")
    }
    companion object {
        @JvmStatic
        var settingsOnFootOrVehicle: List<SettingParam> = mutableListOf(
            SettingParam(
                0,
                SettingsType.BUTTON,
                "УПРАВЛЕНИЕ ПЕШКОМ",
                listOf(SettingDeep("", 0, false, 0, 0, false, null) )
            ),
            SettingParam(
                0,
                SettingsType.BUTTON,
                "УПРАВЛЕНИЕ В АВТО",
                listOf(SettingDeep("", 0, false,0, 0, false, null) )
            )
        )



    }
}