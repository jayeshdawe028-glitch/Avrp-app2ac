package com.multirium.mobile.launcher

import android.content.Context

object Config {
    // game hook
    @JvmField
    var GAME_PATH: String? = null

    fun initGamePath(context: Context) {
        GAME_PATH = context.getExternalFilesDir(null)?.toString()
    }
}