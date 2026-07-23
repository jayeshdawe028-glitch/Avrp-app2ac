package com.multirium.mobile.launcher

import java.io.File

object Utils {
    @JvmStatic
    val isGameInstalled: Boolean
        get() {
            val str: String = Config.GAME_PATH ?: return false
            val file = File("$str/anim/")
            val file2 = File("$str/audio/")
            val file3 = File("$str/data/")
            val file4 = File("$str/models/")
            val file5 = File("$str/texdb/")
            val file6 = File("$str/SAMP/", "settings.ini")
            return file.exists() && file2.exists() && file3.exists() && file4.exists() && file5.exists() && file6.exists()
        }
}