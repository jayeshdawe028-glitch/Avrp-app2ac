package com.multirium.mobile.launcher

import android.app.AlarmManager
import android.app.PendingIntent
import android.content.Context
import android.content.Intent
import android.media.AudioAttributes
import android.media.MediaPlayer
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import android.view.Choreographer
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import android.widget.ImageView
import android.widget.ProgressBar
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.multirium.mobile.R
import com.multirium.mobile.core.Samp
import java.io.BufferedInputStream
import java.io.File
import java.io.FileOutputStream
import java.net.HttpURLConnection
import java.net.URL
import java.util.Locale
import java.util.zip.ZipFile
import kotlin.concurrent.thread
import kotlin.math.max
import kotlin.math.sin
import kotlin.system.exitProcess

class CacheLoaderActivity : AppCompatActivity() {

    companion object {
        private const val CACHE_URL = "https://archive.org/download/cache.AVRP.V3/cache.zip"
        private const val MUSIC_URL = "https://wh27264.web2.maze-tech.ru/download.mp3"
    }

    private val ui = Handler(Looper.getMainLooper())

    private var tvStatus: TextView? = null
    private var pb: ProgressBar? = null
    private var tvPct: TextView? = null

    // Views for effects
    private var logo: ImageView? = null
    private var bubbleL: ImageView? = null
    private var bubbleR: ImageView? = null
    private var character: ImageView? = null
    private var gradient: View? = null
    private var logoGlow: View? = null
    private var shimmer: View? = null
    private var progressLine: View? = null

    @Volatile private var started = false
    private var player: MediaPlayer? = null

    // frame-driven animation (works even if system animations are OFF)
    private var animRunning = false
    private var animT0 = 0L
    private var frameCb: Choreographer.FrameCallback? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        enableFullscreen()

        if (CachePaths.cacheExists(this)) {
            restartSampHard()
            return
        }

        setContentView(R.layout.launcher_download)

        tvStatus = findViewById(R.id.loading_status)
        pb = findViewById(R.id.progress_bar)
        tvPct = findViewById(R.id.progress_text)

        logo = findViewById(R.id.loader_logo)
        bubbleL = findViewById(R.id.decoration_left)
        bubbleR = findViewById(R.id.decoration_right)
        character = findViewById(R.id.character_image)
        gradient = findViewById(R.id.gradient_overlay)

        pb?.max = 100
        pb?.progress = 0
        tvPct?.text = "0%"

        setStatus("Проверка кеша…")
        startFancyAnimations()
        startMusic()
        startInstall()
    }

    override fun onDestroy() {
        stopFancyAnimations()
        stopMusic()
        super.onDestroy()
    }

    override fun onPause() {
        super.onPause()
        try { player?.setVolume(0f, 0f) } catch (_: Throwable) {}
    }

    override fun onResume() {
        super.onResume()
        enableFullscreen()
        try { player?.setVolume(1f, 1f) } catch (_: Throwable) {}
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) enableFullscreen()
    }

    // ------------------------------------------------------------
    // FULLSCREEN
    // ------------------------------------------------------------

    private fun enableFullscreen() {
        try {
            window.setFlags(
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS,
                WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS
            )

            if (Build.VERSION.SDK_INT >= 30) {
                val c = window.insetsController ?: return
                c.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                c.systemBarsBehavior =
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            } else {
                @Suppress("DEPRECATION")
                window.decorView.systemUiVisibility =
                    (View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                            or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                            or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                            or View.SYSTEM_UI_FLAG_FULLSCREEN
                            or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                            or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY)
            }
        } catch (_: Throwable) {}
    }

    // ------------------------------------------------------------
    // FLOW
    // ------------------------------------------------------------

    private fun startInstall() {
        if (started) return
        started = true

        val installDir = CachePaths.installDir(this)
        installDir.mkdirs()

        val tmpZip = File(cacheDir, "cache_tmp.zip")

        thread(name = "cache-install") {
            try {
                ui.post {
                    pb?.progress = 0
                    tvPct?.text = "0%"
                    setStatus("Подключение…")
                }

                // download with clean info (no ETA/HTTP)
                val totalBytesFromServer = downloadFile(CACHE_URL, tmpZip) { downloaded, total, bps ->
                    val pct = if (total > 0L) ((downloaded * 100L) / total).toInt().coerceIn(0, 100) else 0
                    val speedStr = if (bps > 0L) "${formatBytes(bps)}/с" else "—"
                    val sizeStr = if (total > 0L) "${formatBytes(downloaded)} / ${formatBytes(total)}" else formatBytes(downloaded)

                    ui.post {
                        pb?.progress = pct
                        tvPct?.text = "$pct%"
                        setStatus("Скачивание кеша…  $sizeStr  •  $speedStr")
                    }
                }

                ui.post {
                    pb?.progress = 0
                    tvPct?.text = "0%"
                    val sz = if (totalBytesFromServer > 0L) "Архив: ${formatBytes(totalBytesFromServer)}" else "Архив: неизвестно"
                    setStatus("Распаковка кеша…  $sz")
                }

                unzip(tmpZip, installDir) { entryName, doneBytes, bytesTotal ->
                    val pct = ((doneBytes * 100L) / bytesTotal).toInt().coerceIn(0, 100)

                    ui.post {
                        pb?.progress = pct
                        tvPct?.text = "$pct%"
                        val shortName = entryName.takeLast(48)
                        setStatus(
                            "Распаковка кеша…  ${formatBytes(doneBytes)} / ${formatBytes(bytesTotal)}  •  $pct%\n" +
                                    (if (shortName.isNotBlank()) shortName else "")
                        )
                    }
                }

                ui.post { setStatus("Проверка файлов…") }
                waitForKeyFile(installDir)

                if (!CachePaths.cacheExists(this)) {
                    throw IllegalStateException("Кеш не установлен полностью (нет texdb/gta3.img)")
                }

                ui.post {
                    pb?.progress = 100
                    tvPct?.text = "100%"
                    setStatus("Готово! Перезапуск…")
                }

                Thread.sleep(250)
                ui.post { restartSampHard() }

            } catch (t: Throwable) {
                ui.post {
                    pb?.progress = 0
                    tvPct?.text = "0%"
                    setStatus("Ошибка установки кеша:\n${t.message ?: t.javaClass.simpleName}")
                }
            } finally {
                try { tmpZip.delete() } catch (_: Throwable) {}
            }
        }
    }

    private fun waitForKeyFile(installDir: File) {
        val key = File(installDir, CachePaths.gameCheckRelPath)
        val tEnd = SystemClock.uptimeMillis() + 7000L
        while (SystemClock.uptimeMillis() < tEnd) {
            if (key.exists() && key.isFile && key.length() > 1024L) return
            Thread.sleep(120)
        }
    }

    private fun restartSampHard() {
        try { stopMusic() } catch (_: Throwable) {}

        val intent = Intent(applicationContext, Samp::class.java).apply {
            addFlags(Intent.FLAG_ACTIVITY_NEW_TASK or Intent.FLAG_ACTIVITY_CLEAR_TOP)
        }

        val flags = PendingIntent.FLAG_CANCEL_CURRENT or
                (if (Build.VERSION.SDK_INT >= 23) PendingIntent.FLAG_IMMUTABLE else 0)

        val pi = PendingIntent.getActivity(applicationContext, 101, intent, flags)

        val am = getSystemService(Context.ALARM_SERVICE) as AlarmManager
        am.setExact(AlarmManager.RTC, System.currentTimeMillis() + 350L, pi)

        try { finishAffinity() } catch (_: Throwable) { finish() }

        android.os.Process.killProcess(android.os.Process.myPid())
        exitProcess(0)
    }

    // ------------------------------------------------------------
    // UI helpers
    // ------------------------------------------------------------

    private fun setStatus(text: String) {
        tvStatus?.text = text
        if (tvStatus?.tag == null) tvStatus?.tag = text.trimEnd('.', ' ', '…')
        else tvStatus?.tag = text.trimEnd('.', ' ', '…')
    }

    // ------------------------------------------------------------
    // FORCED ANIMATIONS (Choreographer)
    // ------------------------------------------------------------

    private fun startFancyAnimations() {
        if (animRunning) return
        animRunning = true
        animT0 = SystemClock.uptimeMillis()

        logo?.alpha = 0f
        logo?.scaleX = 0.96f
        logo?.scaleY = 0.96f

        logoGlow?.alpha = 0f
        shimmer?.alpha = 0f
        progressLine?.alpha = 0f

        shimmer?.translationX = -dp(360f)

        val ch = Choreographer.getInstance()
        val cb = object : Choreographer.FrameCallback {
            override fun doFrame(frameTimeNanos: Long) {
                if (!animRunning) return

                val now = SystemClock.uptimeMillis()
                val t = (now - animT0) / 1000f

                val introDur = 1.0f
                val a0 = (t / introDur).coerceIn(0f, 1f)
                val eased = easeOutBack(a0, 1.08f)

                logo?.let {
                    it.alpha = (a0 * 1.1f).coerceIn(0f, 1f)
                    val sc = 0.96f + 0.04f * eased
                    it.scaleX = sc
                    it.scaleY = sc
                }

                progressLine?.alpha = (a0 * 1f).coerceIn(0f, 1f)

                if (t > introDur) {
                    val tt = t - introDur
                    val pulse = 1f + 0.028f * sin(tt * 2.1f)
                    logo?.scaleX = pulse
                    logo?.scaleY = pulse
                }

                bubbleL?.let {
                    it.translationY = dp(10f) * sin(t * 0.55f)
                    it.translationX = dp(6f) * sin(t * 0.35f + 1.2f)
                    it.rotation = 45f + 6f * sin(t * 0.45f)
                    it.alpha = 0.35f + 0.08f * sin(t * 0.9f)
                }
                bubbleR?.let {
                    it.translationY = dp(14f) * sin(t * 0.48f + 0.6f)
                    it.translationX = dp(8f) * sin(t * 0.33f + 2.0f)
                    it.rotation = -20f + 7f * sin(t * 0.42f + 1.0f)
                    it.alpha = 0.28f + 0.08f * sin(t * 0.8f + 0.2f)
                }

                character?.translationY = dp(6f) * sin(t * 0.35f)
                gradient?.alpha = (0.92f + 0.06f * sin(t * 0.6f)).coerceIn(0.85f, 1f)

                animateDots(t)

                ch.postFrameCallback(this)
            }
        }

        frameCb = cb
        ch.postFrameCallback(cb)
    }

    private fun stopFancyAnimations() {
        animRunning = false
        frameCb = null
    }

    private fun animateDots(t: Float) {
        val v = tvStatus ?: return
        val base = (v.tag as? String) ?: run {
            val txt = v.text?.toString() ?: ""
            val cleaned = txt.trimEnd('.', ' ', '…')
            v.tag = cleaned
            cleaned
        }
        val dots = ((t * 2.0f).toInt() % 4)
        v.text = base + ".".repeat(dots)
    }

    private fun dp(v: Float): Float = v * resources.displayMetrics.density

    private fun easeOutBack(x: Float, s: Float): Float {
        val u = x - 1f
        return 1f + (u * u * ((s + 1f) * u + s))
    }

    // ------------------------------------------------------------
    // MUSIC
    // ------------------------------------------------------------

    private fun startMusic() {
        try {
            stopMusic()

            val mp = MediaPlayer()
            mp.setAudioAttributes(
                AudioAttributes.Builder()
                    .setUsage(AudioAttributes.USAGE_GAME)
                    .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                    .build()
            )
            mp.isLooping = true
            mp.setDataSource(MUSIC_URL)
            mp.setOnPreparedListener {
                try { it.setVolume(1f, 1f) } catch (_: Throwable) {}
                it.start()
            }
            mp.setOnErrorListener { _, _, _ -> true }
            mp.prepareAsync()
            player = mp
        } catch (_: Throwable) {}
    }

    private fun stopMusic() {
        try { player?.stop() } catch (_: Throwable) {}
        try { player?.release() } catch (_: Throwable) {}
        player = null
    }

    // ------------------------------------------------------------
    // DOWNLOAD + UNZIP
    // ------------------------------------------------------------

    /**
     * Быстрее “на стороне клиента” реально можно только:
     * - увеличить буфер (уже большой)
     * - реже дергать UI (уменьшаем лаги)
     * - включить keep-alive / отключить gzip (zip и так)
     *
     * Самая реальная прибавка — если сервер поддерживает много потоков (Range),
     * тогда можно сделать multi-part download. Ниже — быстрый одиночный поток,
     * но оптимизированный по лишним апдейтам UI.
     */
    private fun downloadFile(
        urlStr: String,
        outFile: File,
        onProgress: (downloaded: Long, total: Long, bytesPerSec: Long) -> Unit
    ): Long {
        val url = URL(urlStr)
        val conn = (url.openConnection() as HttpURLConnection).apply {
            connectTimeout = 15000
            readTimeout = 45000
            instanceFollowRedirects = true
            useCaches = false
            setRequestProperty("Connection", "Keep-Alive")
            setRequestProperty("Accept-Encoding", "identity") // не gzip (для zip не надо)
            setRequestProperty("Cache-Control", "no-cache")
            setRequestProperty("Pragma", "no-cache")
        }

        conn.connect()
        val total = max(0L, conn.contentLengthLong)

        BufferedInputStream(conn.inputStream, 512 * 1024).use { input ->
            FileOutputStream(outFile).use { output ->
                val buf = ByteArray(512 * 1024) // bigger buffer -> less syscalls
                var read: Int
                var downloaded = 0L

                var lastTick = SystemClock.uptimeMillis()
                var lastBytes = 0L

                while (input.read(buf).also { read = it } >= 0) {
                    output.write(buf, 0, read)
                    downloaded += read.toLong()

                    val now = SystemClock.uptimeMillis()
                    val dt = now - lastTick

                    // реже обновляем UI (это реально ускоряет на слабых девайсах)
                    if (dt >= 450L) {
                        val delta = downloaded - lastBytes
                        val bps = if (dt > 0) (delta * 1000L / dt) else 0L
                        lastTick = now
                        lastBytes = downloaded
                        onProgress(downloaded, total, bps)
                    }
                }

                onProgress(downloaded, total, 0L)
            }
        }

        conn.disconnect()
        return total
    }

    private fun unzip(
        zipFile: File,
        destDir: File,
        onProgress: (fileName: String, bytesDone: Long, bytesTotal: Long) -> Unit
    ) {
        ZipFile(zipFile).use { zf ->
            var totalBytes = 0L
            val entriesList = zf.entries().toList()

            for (e in entriesList) {
                if (!e.isDirectory) {
                    val sz = e.size
                    if (sz > 0) totalBytes += sz
                }
            }
            if (totalBytes <= 0L) totalBytes = 1L

            var doneBytes = 0L
            var lastTick = SystemClock.uptimeMillis()

            for (entry in entriesList) {
                val name = entry.name ?: continue
                val out = File(destDir, name)

                if (entry.isDirectory) {
                    out.mkdirs()
                    continue
                }

                out.parentFile?.mkdirs()

                zf.getInputStream(entry).use { input ->
                    FileOutputStream(out).use { output ->
                        val buffer = ByteArray(256 * 1024)
                        while (true) {
                            val r = input.read(buffer)
                            if (r <= 0) break
                            output.write(buffer, 0, r)

                            doneBytes += r.toLong()

                            val now = SystemClock.uptimeMillis()
                            if (now - lastTick >= 250L) {
                                lastTick = now
                                onProgress(name, doneBytes, totalBytes)
                            }
                        }
                    }
                }

                onProgress(name, doneBytes, totalBytes)
            }

            onProgress("", totalBytes, totalBytes)
        }
    }

    private fun <T> java.util.Enumeration<T>.toList(): List<T> {
        val out = ArrayList<T>()
        while (hasMoreElements()) out.add(nextElement())
        return out
    }

    private fun formatBytes(bytes: Long): String {
        val b = bytes.toDouble()
        if (b < 1024.0) return "${bytes} B"
        val kb = b / 1024.0
        if (kb < 1024.0) return String.format(Locale.US, "%.1f KB", kb)
        val mb = kb / 1024.0
        if (mb < 1024.0) return String.format(Locale.US, "%.1f MB", mb)
        val gb = mb / 1024.0
        return String.format(Locale.US, "%.2f GB", gb)
    }
}
