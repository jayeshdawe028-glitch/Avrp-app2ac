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

    // A cache containing gta3.img alone is not considered a valid game install.
    // The native client explicitly loads the SAMP texture database at startup.
    private val requiredFiles = listOf(
        "texdb/gta3.img",
        "texdb/samp/samp.txt",
        "texdb/samp/samp.dxt.tmb",
        "texdb/samp/samp.dxt.dat",
        "texdb/samp/samp.dxt.toc"
    )

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

    private fun isValidInstall(dir: File): Boolean {
        return requiredFiles.all { rel ->
            try {
                val file = File(dir, rel)
                file.isFile && file.length() > 0L
            } catch (_: Throwable) {
                false
            }
        }
    }

    fun cacheExists(context: Context): Boolean {
        val candidates = linkedSetOf<File>()
        candidates += baseDir(context)
        candidates += File(
            Environment.getExternalStorageDirectory(),
            "Android/data/${context.packageName}/files"
        )
        candidates += Environment.getExternalStorageDirectory()

        return candidates.any { isValidInstall(it) }
    }

    /** Where we will install downloaded cache (we install into baseDir by default). */
    fun installDir(context: Context): File = baseDir(context)
}
