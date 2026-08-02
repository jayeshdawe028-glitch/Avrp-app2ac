package com.multirium.mobile.launcher

/* ============================================================================

    Echo Cross
    Telegram: @tasjJson

============================================================================ */

import android.Manifest
import android.annotation.SuppressLint
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.ConnectivityManager
import android.os.Build
import android.os.Bundle
import android.provider.Settings
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import com.multirium.mobile.databinding.LauncherDownloadBinding
import net.lingala.zip4j.ZipFile
import okhttp3.Call
import okhttp3.Callback
import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.Response
import java.io.BufferedInputStream
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.util.Locale
import java.util.Timer
import java.util.TimerTask
import java.util.concurrent.TimeUnit
import kotlin.concurrent.thread
import kotlin.math.roundToInt

class LoaderActivity : AppCompatActivity() {

    private var binding: LauncherDownloadBinding? = null
    private fun b(): LauncherDownloadBinding = requireNotNull(binding)

    // ====== INSTALLER CONFIG ======
    private val cacheUrl = "https://archive.org/download/cache_202608/cache.zip"
    private val gameCheckRelPath = "texdb/gta3.img"

    private var pathZip: File? = null
    private var destinationDir: File? = null
    private var downloadStartTime = 0L
    private var installingNow = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        Config.initGamePath(this)

        binding = LauncherDownloadBinding.inflate(layoutInflater)
        setContentView(b().root)

        window.decorView.post { hideSystemUI() }

        destinationDir = getExternalFilesDir(null)
        pathZip = File(cacheDir, "cache.zip")

        if (hasAllPermissions()) {
            startFlow()
        } else {
            requestPermissions(
                arrayOf(
                    Manifest.permission.READ_EXTERNAL_STORAGE,
                    Manifest.permission.WRITE_EXTERNAL_STORAGE,
                    Manifest.permission.RECORD_AUDIO
                ),
                1000
            )
        }
    }

    private fun hasAllPermissions(): Boolean {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            val r = checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
            val w = checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED
            val a = checkSelfPermission(Manifest.permission.RECORD_AUDIO) == PackageManager.PERMISSION_GRANTED
            return r && w && a
        }
        return true
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 1000) startFlow()
    }

    private fun startFlow() {
        // небольшой "кинематографичный" старт, как у тебя было 3.3 сек
        Timer().schedule(object : TimerTask() {
            override fun run() {
                runOnUiThread {
                    if (isGameInstalled()) {
                        setStatus("ПРОВЕРКА...")
                        setProgress(100)
                    } else {
                        startAutoInstall()
                    }
                }
            }
        }, 1200L)
    }

    private fun isGameInstalled(): Boolean {
        val dest = destinationDir ?: return false
        return File(dest, gameCheckRelPath).exists()
    }

    private fun startAutoInstall() {
        if (installingNow) return
        installingNow = true

        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        setStatus("ПОДГОТОВКА...")
        setProgress(0)

        if (!isInternetAvailable()) {
            setStatus("НЕТ ИНТЕРНЕТА")
            toast("Нет подключения к интернету")
            stopAutoInstallUi()
            return
        }

        val dest = destinationDir
        if (dest == null) {
            setStatus("ОШИБКА ПАМЯТИ")
            toast("Ошибка доступа к памяти устройства")
            stopAutoInstallUi()
            return
        }

        if (!dest.exists()) dest.mkdirs()

        val freeMb = dest.freeSpace / 1024 / 1024
        if (freeMb < 2000) {
            setStatus("НЕТ МЕСТА")
            toast("Недостаточно места: $freeMb MB")
            stopAutoInstallUi()
            return
        }

        startDownloadZip()
    }

    private fun startDownloadZip() {
        setStatus("ПОДКЛЮЧЕНИЕ К СЕРВЕРУ...")
        setProgress(0)

        val client = OkHttpClient.Builder()
            .retryOnConnectionFailure(true)
            .connectTimeout(15, TimeUnit.SECONDS)
            .readTimeout(120, TimeUnit.SECONDS)
            .writeTimeout(120, TimeUnit.SECONDS)
            .build()

        val request = Request.Builder().url(cacheUrl).build()
        downloadStartTime = System.currentTimeMillis()

        client.newCall(request).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                runOnUiThread {
                    setStatus("ОШИБКА ЗАГРУЗКИ")
                    toast("Ошибка загрузки: ${e.message ?: "unknown"}")
                    stopAutoInstallUi()
                }
            }

            override fun onResponse(call: Call, response: Response) {
                val body = response.body
                if (!response.isSuccessful || body == null) {
                    runOnUiThread {
                        setStatus("ОШИБКА СЕРВЕРА")
                        toast("Ошибка ответа: ${response.code}")
                        stopAutoInstallUi()
                    }
                    return
                }

                val totalSize = body.contentLength()
                val zip = pathZip ?: return

                var input: BufferedInputStream? = null
                var output: FileOutputStream? = null

                try {
                    input = BufferedInputStream(body.byteStream(), 16384)
                    output = FileOutputStream(zip)

                    val buffer = ByteArray(16384)
                    var downloaded = 0L

                    runOnUiThread {
                        setStatus("СКАЧИВАНИЕ...")
                        setProgress(0)
                    }

                    while (true) {
                        val read = input.read(buffer)
                        if (read == -1) break
                        output.write(buffer, 0, read)
                        downloaded += read

                        val percent = if (totalSize > 0)
                            ((downloaded.toDouble() / totalSize.toDouble()) * 100.0).roundToInt().coerceIn(0, 100)
                        else 0

                        val elapsedSec = (System.currentTimeMillis() - downloadStartTime) / 1000.0
                        val downloadedMb = downloaded / 1024.0 / 1024.0
                        val speed = if (elapsedSec > 0) downloadedMb / elapsedSec else 0.0

                        runOnUiThread {
                            setProgress(percent)
                            setStatus(String.format(Locale.US, "СКАЧИВАНИЕ %d%%  (%.2f MB/s)", percent, speed))
                        }
                    }

                    output.flush()
                } catch (_: Throwable) {
                    runOnUiThread {
                        setStatus("ОШИБКА ЗАГРУЗКИ")
                        toast("Ошибка при загрузке файла")
                        stopAutoInstallUi()
                    }
                    safeClose(output)
                    safeClose(input)
                    return
                }

                safeClose(output)
                safeClose(input)

                if (!zip.exists() || zip.length() == 0L) {
                    runOnUiThread {
                        setStatus("ОШИБКА ФАЙЛА")
                        toast("Ошибка: файл кеша пустой")
                        stopAutoInstallUi()
                    }
                    return
                }

                runOnUiThread {
                    setStatus("РАСПАКОВКА...")
                    setProgress(0)
                }
                unzipArchive()
            }
        })
    }

    private fun unzipArchive() = thread {
        val zip = pathZip
        val dest = destinationDir

        if (zip == null || !zip.exists() || zip.length() == 0L) {
            runOnUiThread {
                setStatus("ОШИБКА ZIP")
                toast("Файл кеша не найден или пустой")
                stopAutoInstallUi()
            }
            return@thread
        }
        if (dest == null) {
            runOnUiThread {
                setStatus("ОШИБКА ПАМЯТИ")
                toast("Ошибка доступа к памяти при распаковке")
                stopAutoInstallUi()
            }
            return@thread
        }

        try {
            val z = ZipFile(zip)
            if (!z.isValidZipFile) {
                runOnUiThread {
                    setStatus("АРХИВ ПОВРЕЖДЕН")
                    toast("Архив повреждён")
                    stopAutoInstallUi()
                }
                return@thread
            }

            val headers = z.fileHeaders
            val total = headers.size.coerceAtLeast(1)
            var cur = 0

            for (h in headers) {
                z.extractFile(h, dest.absolutePath)
                cur++
                val percent = ((cur.toDouble() / total.toDouble()) * 100.0).roundToInt().coerceIn(0, 100)
                runOnUiThread {
                    setProgress(percent)
                    setStatus("РАСПАКОВКА $percent%")
                }
            }

            zip.delete()

            val checkFile = File(dest, gameCheckRelPath)
            if (checkFile.exists()) {
                runOnUiThread {
                    setProgress(100)
                    setStatus("ГОТОВО! ЗАПУСК...")
                    stopAutoInstallUi(keepUi = true)
                }
            } else {
                runOnUiThread {
                    setStatus("ОШИБКА УСТАНОВКИ")
                    toast("Файл gta3.img не найден после распаковки")
                    stopAutoInstallUi()
                }
            }
        } catch (e: Throwable) {
            runOnUiThread {
                setStatus("ОШИБКА РАСПАКОВКИ")
                toast("Ошибка распаковки: ${e.message ?: "unknown"}")
                stopAutoInstallUi()
            }
        }
    }

    private fun stopAutoInstallUi(keepUi: Boolean = false) {
        installingNow = false
        window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        if (!keepUi) {
            setStatus("...")
            setProgress(0)
        }
    }


    // ====== UI (launcher_download) ======

    @SuppressLint("SetTextI18n")
    private fun setStatus(text: String) {
        try {
            b().loadingStatus.text = text
        } catch (_: Throwable) {
        }
    }

    private fun setProgressUi(percent: Int) {
        val p = percent.coerceIn(0, 100)
        try {
            b().progressBar.progress = p
            b().progressText.text = "$p%"
        } catch (_: Throwable) {
        }
    }


    private fun toast(text: String) {
        Toast.makeText(this, text, Toast.LENGTH_SHORT).show()
    }

    private fun isInternetAvailable(): Boolean {
        return try {
            val cm = getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
            val net = cm.activeNetworkInfo
            net != null && net.isConnected
        } catch (_: Throwable) {
            false
        }
    }

    private fun hideSystemUI() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            window.insetsController?.let { controller ->
                controller.hide(WindowInsets.Type.systemBars())
                controller.systemBarsBehavior =
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility =
                View.SYSTEM_UI_FLAG_FULLSCREEN or
                        View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
                        View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) hideSystemUI()
    }

    private fun safeClose(c: Any?) {
        try {
            when (c) {
                is FileOutputStream -> c.close()
                is BufferedInputStream -> c.close()
            }
        } catch (_: Throwable) {
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        binding = null
    }
}
