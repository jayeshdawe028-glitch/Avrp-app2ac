package com.multirium.mobile.launcher

import android.content.Context
import android.os.Environment
import java.io.File

/**
 * Unified path resolver for game cache.
 * We intentionally check several common locations so it works across builds.
 */
object CachePaths {
    const val gameCheckRelPath: String = "texdb/gta3.img"

    /** Preferred install dir (app-scoped external files). */
    fun baseDir(context: Context): File {
        return context.getExternalFilesDir(null) ?: context.filesDir
    }

    /** All candidate absolute files we consider as "cache exists". */
    fun candidateFiles(context: Context): List<File> {
        val pkg = context.packageName
        val rel = gameCheckRelPath

        val appScoped = File(baseDir(context), rel)

        val androidData = File(
            Environment.getExternalStorageDirectory(),
            "Android/data/$pkg/files/$rel"
        )

        val legacyRoot = File(Environment.getExternalStorageDirectory(), rel)

        return listOf(appScoped, androidData, legacyRoot).distinct()
    }

    fun cacheExists(context: Context): Boolean {
        return candidateFiles(context).any { f ->
            try {
                f.exists() && f.isFile && f.length() > 1024L
            } catch (_: Throwable) {
                false
            }
        }
    }

    /** Where we will install downloaded cache (we install into baseDir by default). */
    fun installDir(context: Context): File = baseDir(context)
}
