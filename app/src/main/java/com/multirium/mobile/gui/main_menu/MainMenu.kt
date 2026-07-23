package com.multirium.mobile.gui.main_menu


import android.app.AlertDialog
import android.content.DialogInterface
import android.view.View
import android.view.ViewGroup
import com.multirium.mobile.R
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.MainMenuLayoutBinding
import com.multirium.mobile.gui.NativeGui
import com.multirium.mobile.gui.main_menu.obj.MapIcon
import com.multirium.mobile.gui.main_menu.utils.MainMenuTimer
import com.multirium.mobile.gui.main_menu.adapter.MainMenuAdapter
import com.multirium.mobile.gui.pausemenu.SettingsMenu
import com.multirium.mobile.gui.util.LinearItemDecoration
import com.multirium.mobile.gui.util.NoScrollLinearLayoutManager
import java.util.Locale
import kotlin.jvm.internal.Reflection


class MainMenu : NativeGui<MainMenuLayoutBinding>(Reflection.getOrCreateKotlinClass(MainMenuLayoutBinding::class.java), false, false, true, false) {
    val list: List<MapIcon> = listOf(
        MapIcon("ВЫЙТИ ИЗ ИГРЫ", R.drawable.target_notify_exit),
        MapIcon("НАСТРОЙКИ", R.drawable.main_menu_settings_icon),
        MapIcon("КАРТА", R.drawable.main_menu_map_icon),
        MapIcon("ПРОДОЛЖИТЬ", R.drawable.main_menu_continue_icon))

    val bind = getBinding() as MainMenuLayoutBinding
    val view: View = bind.root
    val adapter = MainMenuAdapter(list)
    var timer: MainMenuTimer = MainMenuTimer(bind.gameMenuAfkText, "НА ПАУЗЕ :")
    external fun exit()
    init {
        adapter.onItemClick = {
            val upperCase: String = it.title.toUpperCase(Locale.ROOT)
            when (upperCase.hashCode()) {
                -1806384667 -> {
                    if (upperCase == "НАСТРОЙКИ") {
                        activity.runOnUiThread {
                            setChildrenVisibility(bind.root, false)
                            val settingsMenu = SettingsMenu()
                            settingsMenu.showSettingsMenu()
                            settingsMenu.exitCallback = {
                                settingsMenu.destroy()
                                setChildrenVisibility(bind.root, true)
                            }
                        }
                    }
                }

                55390688 -> {
                    if (upperCase == "ПРОДОЛЖИТЬ") {
                        destroy()
                        timer.stop()
                        exit()
                    }
                }

                411199421 -> {
                    if (upperCase == "ВЫЙТИ ИЗ ИГРЫ") {
                        val builder: AlertDialog.Builder = AlertDialog.Builder(activity)
                        builder.setTitle("Подтверждение выхода")
                        builder.setMessage("Вы уверены, что хотите выйти из игры?")
                        builder.setPositiveButton("Да", object : DialogInterface.OnClickListener{
                            override fun onClick(dialog: DialogInterface, which: Int) {
                                activity.finishAndRemoveTask()
                            }
                        })
                        builder.setNegativeButton("Нет", object : DialogInterface.OnClickListener{
                            override fun onClick(dialog: DialogInterface, which: Int) {
                                dialog.dismiss()
                            }
                        })
                        builder.create()
                        builder.show()
                    }
                }

                1001728344 -> {
                    if (upperCase == "КАРТА") {
                        val menuMap = MenuMap()
                        bind.root.visibility = View.GONE
                        menuMap.exitCallback = {
                            bind.root.visibility = View.VISIBLE
                        }
                    }
                }
            }
        }
        bind.menuRc.setLayoutManager(NoScrollLinearLayoutManager(activity, 0))
        bind.menuRc.setAdapter(adapter)

        bind.menuRc.addItemDecoration(LinearItemDecoration(0.22115f, 1.0f, 0.036f))

        timer.reset()
        timer.start()

    }

    fun setChildrenVisibility(viewGroup: ViewGroup, toggle: Boolean) {
        viewGroup.visibility = if (toggle) View.VISIBLE else View.GONE
    }
    override fun receivePacket(type: Int, data: String?) {
        TODO("Not yet implemented")
    }
}