package com.multirium.mobile.gui

import android.app.Activity
import android.content.Intent
import android.os.Handler
import android.os.Looper
import android.os.SystemClock
import android.view.Choreographer
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.core.view.isVisible
import com.multirium.mobile.GUIWrapper
import com.multirium.mobile.databinding.GameLoadScreenBinding
import com.multirium.mobile.launcher.CacheLoaderActivity
import com.multirium.mobile.launcher.CachePaths
import org.json.JSONObject
import kotlin.math.sin

class LoadingScreen : GUIWrapper {

    private var binding: GameLoadScreenBinding? = null
    private var activity: Activity? = null
    private var root: ViewGroup? = null

    private val ui = Handler(Looper.getMainLooper())

    // staged (fake) progress + texts
    private var fakeRunning = false
    private var fakeT0 = 0L
    private var fakeStage = 0
    private var fakeStageT0 = 0L
    private var fakeCb: Choreographer.FrameCallback? = null

    // text pacing
    private var lastSubIdx = -1
    private var lastSubChangeAt = 0L
    private val minSubChangeMs = 900L          // <<< задержка смены под-строки (было 650)
    private val minTitleHoldMs = 650L          // <<< чтобы title не дергался на границе стадий

    // smooth text update
    private var lastTitle: String = ""
    private var lastSubText: String = ""
    private var lastPctShown: Int = -1
    private var lastPctUpdateAt = 0L
    private val minPctUpdateMs = 120L          // <<< чтобы % не спамил UI каждый кадр

    // progress line animation
    private var lineRunning = false
    private var lineCb: Choreographer.FrameCallback? = null
    private var lineStartMs = 0L
    private var targetScaleX = 0.05f          // куда хотим прийти (по pct)
    private var smoothScaleX = 0.05f          // сглаженное значение
    private var lastLineFrameMs = 0L

    override fun onShow(data: JSONObject?) {
        val b = binding ?: return
        val a = activity ?: return

        if (b.root.parent == null) root?.addView(b.root)

        playLogoAnim()
        startProgressLineAnim()

        // cache gate
        if (!CachePaths.cacheExists(a)) {
            stopFakeFlow()
            b.gameStatus.text = "Кеш игры не найден"
            b.gameSubStatus.text = "Нужно скачать файлы, чтобы продолжить"
            hideProgressLine()
            ui.postDelayed({
                try { a.startActivity(Intent(a, CacheLoaderActivity::class.java)) } catch (_: Throwable) {}
            }, 450)
            return
        }

        // ALWAYS run fake flow
        startFakeFlow()
    }

    override fun onClose() {
        stopFakeFlow()
        stopProgressLineAnim()
        val b = binding ?: return
        if (b.root.parent != null) root?.removeView(b.root)
    }

    override fun receiveUIpacket(data: JSONObject?) {
        // реальный статус игнорим
    }

    // ------------------------------------------------------------
    // FAKE FLOW (texts + improvised progress)
    // ------------------------------------------------------------

    private data class Stage(
        val title: String,
        val sub: Array<String>,
        val minMs: Long,
        val maxMs: Long,
        val pctFrom: Int,
        val pctTo: Int
    )

    private fun startFakeFlow() {
        if (fakeRunning) return
        fakeRunning = true
        fakeT0 = SystemClock.uptimeMillis()
        fakeStage = 0
        fakeStageT0 = fakeT0

        lastSubIdx = -1
        lastSubChangeAt = 0L
        lastTitle = ""
        lastSubText = ""
        lastPctShown = -1
        lastPctUpdateAt = 0L

        applyFakeUI(
            title = "Подготовка…",
            sub = "Запуск клиента",
            pct = 1,
            forceText = true
        )

        val stages = buildStages()
        val ch = Choreographer.getInstance()

        val cb = object : Choreographer.FrameCallback {
            override fun doFrame(frameTimeNanos: Long) {
                if (!fakeRunning) return
                val now = SystemClock.uptimeMillis()

                val s = stages[fakeStage.coerceIn(0, stages.lastIndex)]
                val stageDur = stageDurationMs(s)
                val t = ((now - fakeStageT0).toFloat() / stageDur.toFloat()).coerceIn(0f, 1f)

                val eased = easeInOut(t)
                val basePct = s.pctFrom + ((s.pctTo - s.pctFrom) * eased).toInt()

                // subtle wobble, never 100
                val wobble = (1.2f * sin(((now - fakeT0).toDouble()) / 260.0)).toInt()
                val pct = (basePct + wobble).coerceIn(0, 99)

                // --- slower sub text ---
                val wantIdx = ((t * s.sub.size).toInt()).coerceIn(0, s.sub.size - 1)

                var showIdx = lastSubIdx
                val canChange = (now - lastSubChangeAt) >= minSubChangeMs

                if (showIdx == -1) {
                    showIdx = wantIdx
                    lastSubIdx = showIdx
                    lastSubChangeAt = now
                } else if (wantIdx != showIdx && canChange) {
                    showIdx = wantIdx
                    lastSubIdx = showIdx
                    lastSubChangeAt = now
                }

                // Title hold to avoid twitch on stage boundary
                val forceText = (now - fakeStageT0) < minTitleHoldMs

                applyFakeUI(
                    title = s.title,
                    sub = s.sub[showIdx.coerceIn(0, s.sub.size - 1)],
                    pct = pct,
                    forceText = forceText
                )

                if (t >= 1f) {
                    if (fakeStage < stages.lastIndex) {
                        fakeStage++
                        fakeStageT0 = now
                        lastSubIdx = -1
                        lastSubChangeAt = 0L
                    } else {
                        // loop last stage smoothly
                        fakeStageT0 = now - (stageDur / 2)
                    }
                }

                ch.postFrameCallback(this)
            }
        }

        fakeCb = cb
        ch.postFrameCallback(cb)
    }

    private fun stopFakeFlow() {
        fakeRunning = false
        fakeCb = null
    }

    private fun applyFakeUI(title: String, sub: String, pct: Int, forceText: Boolean) {
        val b = binding ?: return
        val now = SystemClock.uptimeMillis()

        // --- throttled percent updates (чтобы не дергать анимации каждую итерацию) ---
        val pctChanged = pct != lastPctShown
        val canPctUpdate = (now - lastPctUpdateAt) >= minPctUpdateMs
        val pctToShow = if (pctChanged && canPctUpdate) {
            lastPctShown = pct
            lastPctUpdateAt = now
            pct
        } else {
            // если слишком рано — показываем прошлое значение
            lastPctShown.coerceAtLeast(0)
        }

        val subLine = "$sub  •  $pctToShow%"

        if (forceText) {
            b.gameStatus.text = title
            b.gameSubStatus.text = subLine
            lastTitle = title
            lastSubText = subLine
        } else {
            // Title: меняем только если реально изменился
            if (title != lastTitle) {
                lastTitle = title
                b.gameStatus.animate().cancel()
                b.gameStatus.alpha = 0.88f
                b.gameStatus.text = title
                b.gameStatus.animate().alpha(1f).setDuration(180).start()
            }

            // Sub line: кроссфейд только когда меняется текст (а не каждый тик из-за %)
            val baseSubOnly = sub // без процента
            val lastBaseSubOnly = lastSubText.substringBefore("  •", lastSubText)

            val shouldCrossfade = baseSubOnly != lastBaseSubOnly
            if (shouldCrossfade) {
                lastSubText = subLine
                b.gameSubStatus.animate().cancel()
                b.gameSubStatus.animate()
                    .alpha(0f)
                    .setDuration(140)
                    .withEndAction {
                        b.gameSubStatus.text = subLine
                        b.gameSubStatus.animate().alpha(1f).setDuration(180).start()
                    }
                    .start()
            } else {
                // только обновим проценты (без fade)
                b.gameSubStatus.text = subLine
                lastSubText = subLine
            }
        }

        updateProgressLine(pctToShow.coerceIn(0, 99))
    }

    private fun buildStages(): Array<Stage> {
        return arrayOf(
            Stage(
                title = "Загрузка игры…",
                sub = arrayOf(
                    "Проверяем ресурсы",
                    "Подготавливаем рендер",
                    "Читаем конфиги",
                    "Проверяем шрифты/текстуры"
                ),
                minMs = 1600, maxMs = 2600,
                pctFrom = 1, pctTo = 18
            ),
            Stage(
                title = "Проверка файлов…",
                sub = arrayOf(
                    "Сканирование кеша",
                    "Сверка контрольных файлов",
                    "Проверка gta3.img",
                    "Проверка texdb / txd"
                ),
                minMs = 2200, maxMs = 3600,
                pctFrom = 18, pctTo = 42
            ),
            Stage(
                title = "Инициализация…",
                sub = arrayOf(
                    "Инициализация аудио",
                    "Создание GUI",
                    "Инициализация потоков",
                    "Подготовка сетевых модулей"
                ),
                minMs = 2000, maxMs = 3400,
                pctFrom = 42, pctTo = 64
            ),
            Stage(
                title = "Подключение…",
                sub = arrayOf(
                    "Запрос сессии",
                    "Синхронизация данных",
                    "Проверка аккаунта",
                    "Получение параметров сервера"
                ),
                minMs = 2400, maxMs = 4200,
                pctFrom = 64, pctTo = 86
            ),
            Stage(
                title = "Вход в мир…",
                sub = arrayOf(
                    "Загрузка окружения",
                    "Загрузка объектов",
                    "Подготовка персонажа",
                    "Почти готово"
                ),
                minMs = 2600, maxMs = 4800,
                pctFrom = 86, pctTo = 99
            )
        )
    }

    private fun stageDurationMs(s: Stage): Long {
        val span = (s.maxMs - s.minMs).coerceAtLeast(1L)
        val x = (fakeStage * 1103515245L + 12345L) and 0x7fffffffL
        val off = (x % span)
        return s.minMs + off
    }

    private fun easeInOut(x: Float): Float {
        val t = x.coerceIn(0f, 1f)
        return t * t * (3f - 2f * t) // smoothstep
    }

    // ------------------------------------------------------------
    // PROGRESS LINE (smooth + looped)
    // ------------------------------------------------------------

    private fun startProgressLineAnim() {
        val b = binding ?: return
        val line = b.progressLine ?: return

        if (lineRunning) return
        lineRunning = true

        line.isVisible = true
        line.alpha = 0f
        line.scaleX = 0.05f
        line.pivotX = 0f
        line.pivotY = (line.height / 2f)

        lineStartMs = SystemClock.uptimeMillis()
        lastLineFrameMs = lineStartMs
        smoothScaleX = line.scaleX.coerceAtLeast(0.05f)
        targetScaleX = smoothScaleX

        val ch = Choreographer.getInstance()
        val cb = object : Choreographer.FrameCallback {
            override fun doFrame(frameTimeNanos: Long) {
                if (!lineRunning) return
                val now = SystemClock.uptimeMillis()
                val dt = (now - lastLineFrameMs).coerceAtLeast(1L).coerceAtMost(40L)
                lastLineFrameMs = now

                // Fade-in once
                val tIn = ((now - lineStartMs).toFloat() / 320f).coerceIn(0f, 1f)
                val baseAlpha = easeInOut(tIn)

                // Looping alpha (always looping)
                val breathe = (0.14 * sin(((now - lineStartMs).toDouble()) / 260.0)).toFloat()
                line.alpha = (0.20f + 0.80f * baseAlpha + breathe).coerceIn(0.18f, 1f)

                // Smooth scale towards targetScaleX (low-pass), no pow
                val dtf = dt.toFloat()
                val k = (dtf / 140f).coerceIn(0.02f, 0.22f)   // чем меньше — тем плавнее
                smoothScaleX += (targetScaleX - smoothScaleX) * k

                // Add looping waves (cyclic, always running)
                val tt = (now - lineStartMs).toDouble()
                val wave = (0.045 * sin(tt / 180.0)).toFloat()
                val wave2 = (0.018 * sin(tt / 70.0)).toFloat()

                line.pivotX = 0f
                line.scaleX = (smoothScaleX + wave + wave2).coerceIn(0.05f, 1f)

                ch.postFrameCallback(this)
            }
        }

        lineCb = cb
        ch.postFrameCallback(cb)
    }

    private fun stopProgressLineAnim() {
        lineRunning = false
        lineCb = null
    }

    private fun hideProgressLine() {
        val b = binding ?: return
        val line = b.progressLine ?: return
        line.isVisible = false
    }

    private fun updateProgressLine(pct: Int) {
        val b = binding ?: return
        val line = b.progressLine ?: return
        if (!line.isVisible) line.isVisible = true

        // map 0..99 -> 0.06..0.98
        val base = 0.06f + (pct.coerceIn(0, 99) / 99f) * 0.92f

        // set target; smoothing happens in frame callback
        targetScaleX = base.coerceIn(0.05f, 0.98f)
    }

    // ------------------------------------------------------------
    // LOGO ANIM
    // ------------------------------------------------------------

    private fun playLogoAnim() {
        val b = binding ?: return
        val logo = b.hudLogo

        logo.clearAnimation()
        logo.isVisible = true

        logo.post {
            val startY = (logo.height.toFloat() + dp(36f)).coerceAtLeast(dp(72f))
            logo.alpha = 0f
            logo.translationY = startY
            logo.scaleX = 0.96f
            logo.scaleY = 0.96f

            val start = SystemClock.uptimeMillis()
            val duration = 780L

            val choreographer = Choreographer.getInstance()
            val cb = object : Choreographer.FrameCallback {
                override fun doFrame(frameTimeNanos: Long) {
                    val now = SystemClock.uptimeMillis()
                    val t = ((now - start).toFloat() / duration.toFloat()).coerceIn(0f, 1f)

                    val s = 1.12f
                    val u = t - 1f
                    val eased = 1f + (u * u * ((s + 1f) * u + s))

                    val y = startY * (1f - eased)
                    logo.translationY = y
                    logo.alpha = (t * 1.15f).coerceIn(0f, 1f)
                    val sc = 0.96f + 0.04f * eased
                    logo.scaleX = sc
                    logo.scaleY = sc

                    if (t < 1f) {
                        choreographer.postFrameCallback(this)
                    } else {
                        logo.translationY = 0f
                        logo.alpha = 1f
                        logo.scaleX = 1f
                        logo.scaleY = 1f
                    }
                }
            }
            choreographer.postFrameCallback(cb)
        }
    }

    private fun dp(v: Float): Float {
        val a = activity ?: return v
        return v * a.resources.displayMetrics.density
    }

    companion object {
        @JvmStatic
        fun newInstance(activity: Activity?, root: ViewGroup?): LoadingScreen {
            val screen = LoadingScreen()
            screen.activity = activity
            screen.root = root
            screen.binding = GameLoadScreenBinding.inflate(LayoutInflater.from(activity), root, false)
            return screen
        }
    }
}
