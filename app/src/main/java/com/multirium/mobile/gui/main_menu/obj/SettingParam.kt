package com.multirium.mobile.gui.main_menu.obj


import com.multirium.mobile.gui.main_menu.obj.SettingsType

class SettingParam(
    var paramId: Int,
    var type: SettingsType,
    var title: String,
    var deeps: List<SettingDeep>
)
