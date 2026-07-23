package com.multirium.mobile.gui.main_menu.obj

class SettingDeep(
    var name: String?,
    var value: Int,
    var isActive: Boolean,
    var minValue: Int,
    var maxValue: Int,
    var isNoPercent: Boolean,
    @Transient
    var callback: Function0<Unit>?
)
