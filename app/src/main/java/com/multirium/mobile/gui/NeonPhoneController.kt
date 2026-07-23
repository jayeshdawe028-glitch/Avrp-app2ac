package com.multirium.mobile.gui

import android.app.Activity
import android.graphics.Color
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.ImageButton
import android.widget.ImageView
import android.widget.SeekBar
import android.widget.TextView
import com.multirium.mobile.GUIWrapper
import com.multirium.mobile.R
import com.multirium.mobile.core.NeonRpc
import org.json.JSONObject
import kotlin.math.atan2
import kotlin.math.max
import kotlin.math.min
import kotlin.math.sqrt

/**
 * Neon GUI controller, compatible with:
 * interface GUIWrapper {
 *   fun onShow(data: JSONObject?)
 *   fun onClose()
 *   fun receiveUIpacket(data: JSONObject?)
 * }
 *
 * Так как GUIWrapper не передаёт Activity/root напрямую, мы пытаемся получить их через reflection
 * из типичных держателей (GUIManager/Samp и т.п.). Если не нашли — просто не показываем UI
 * (чтобы не падать), но проект будет компилироваться.
 *
 * При желании — можно перед вызовом onShow() вызвать setHost(activity, root) вручную.
 */
class NeonPhoneController : GUIWrapper {

    companion object {
        // ID для GUIManager.receiveUIpacket(id, json)
        // Можно поменять на свой свободный id, но тогда обнови и Samp.openNeonPhoneFromServer()
        const val GUI_ID: Int = 77

        @Volatile private var registered = false

        @JvmStatic fun registerInGuiManager() {
            if (registered) return
            try {
                val cls = Class.forName("com.multirium.mobile.GUIManager")
                val wrapper = NeonPhoneController()

                val candidates = listOf(
                    "registerWrapper",
                    "register",
                    "addWrapper",
                    "add",
                    "setWrapper",
                    "putWrapper"
                )

                var ok = false
                for (name in candidates) {
                    try {
                        val m = cls.methods.firstOrNull { mm ->
                            mm.name == name &&
                                    mm.parameterTypes.size == 2 &&
                                    (mm.parameterTypes[0] == Int::class.javaPrimitiveType || mm.parameterTypes[0] == Int::class.java) &&
                                    mm.parameterTypes[1].isAssignableFrom(wrapper.javaClass)
                        }
                        if (m != null) {
                            m.invoke(null, GUI_ID, wrapper)
                            ok = true
                            break
                        }
                    } catch (_: Throwable) {}
                }

                // fallback: иногда API (id, Class) или (id, String)
                if (!ok) {
                    for (name in candidates) {
                        try {
                            val m = cls.methods.firstOrNull { mm ->
                                mm.name == name && mm.parameterTypes.size == 2 &&
                                        (mm.parameterTypes[0] == Int::class.javaPrimitiveType || mm.parameterTypes[0] == Int::class.java) &&
                                        (mm.parameterTypes[1] == Class::class.java)
                            }
                            if (m != null) {
                                m.invoke(null, GUI_ID, NeonPhoneController::class.java)
                                ok = true
                                break
                            }
                        } catch (_: Throwable) {}
                    }
                }

                registered = ok
            } catch (_: Throwable) {
                // ignore
            }
        }
    }


    // host (можно задать вручную извне)
    private var activity: Activity? = null
    private var root: ViewGroup? = null

    // view
    private var panel: View? = null

    // state
    private var enabled = true
    private var effect = 0 // 0 STATIC,1 BLINK,2 FADE,3 RGB
    private var brightness = 100 // 0..100
    private var speed = 1 // 0..3
    private var hue = 180f // 0..360
    private var vehId = -1

    // cached UI refs
    private var imgWheel: ImageView? = null
    private var seekBrightness: SeekBar? = null
    private var btnPower: ImageButton? = null
    private var modeStatic: TextView? = null
    private var modeBlink: TextView? = null
    private var modeFade: TextView? = null
    private var modeRgb: TextView? = null
    private var btnBack: ImageButton? = null
    private var btnApply: TextView? = null

    private var presetRed: View? = null
    private var presetGreen: View? = null
    private var presetBlue: View? = null
    private var presetYellow: View? = null
    private var presetPink: View? = null

    /** Опционально: если у тебя есть доступ к Activity/root — задай явно перед onShow() */
    fun setHost(activity: Activity?, root: ViewGroup?) {
        this.activity = activity
        this.root = root
    }

    override fun onShow(data: JSONObject?) {
        // Примем входные параметры (если менеджер их шлёт)
        data?.let { applyJsonState(it, applySend = false) }

        // Если host не задан — попробуем найти через reflection
        if (activity == null || root == null) {
            val host = tryResolveHost()
            activity = host?.first
            root = host?.second
        }

        val a = activity ?: return
        val r = root ?: return
        if (panel != null) return

        val v = LayoutInflater.from(a).inflate(R.layout.set_neon, r, false)
        panel = v
        r.addView(v)

        bindUi(v)
        syncUi()
        // первичная отправка
        sendNow()
    }

    override fun onClose() {
        val r = root
        val p = panel
        if (r != null && p != null) r.removeView(p)

        panel = null
        imgWheel = null
        seekBrightness = null
        btnPower = null
        modeStatic = null
        modeBlink = null
        modeFade = null
        modeRgb = null
        btnBack = null
        btnApply = null
        presetRed = null
        presetGreen = null
        presetBlue = null
        presetYellow = null
        presetPink = null
    }

    override fun receiveUIpacket(data: JSONObject?) {
        // Сервер/натив может присылать состояние обратно — применим
        data?.let {
            applyJsonState(it, applySend = false)
            syncUi()
        }
    }

    private fun bindUi(v: View) {
        imgWheel = v.findViewById(R.id.imgColorWheel)
        seekBrightness = v.findViewById(R.id.seekBrightness)
        btnPower = v.findViewById(R.id.btnPower)

        modeStatic = v.findViewById(R.id.modeStatic)
        modeBlink  = v.findViewById(R.id.modeBlink)
        modeFade   = v.findViewById(R.id.modeFade)
        modeRgb    = v.findViewById(R.id.modeRgb)

        btnBack = v.findViewById(R.id.btnBack)
        btnApply = v.findViewById(R.id.btnApply)

        presetRed = v.findViewById(R.id.presetRed)
        presetGreen = v.findViewById(R.id.presetGreen)
        presetBlue = v.findViewById(R.id.presetBlue)
        presetYellow = v.findViewById(R.id.presetYellow)
        presetPink = v.findViewById(R.id.presetPink)

        btnBack?.setOnClickListener { onClose() }

        btnPower?.setOnClickListener {
            enabled = !enabled
            sendNow()
            syncUi()
        }

        modeStatic?.setOnClickListener { effect = 0; sendNow(); syncUi() }
        modeBlink?.setOnClickListener  { effect = 1; sendNow(); syncUi() }
        modeFade?.setOnClickListener   { effect = 2; sendNow(); syncUi() }
        modeRgb?.setOnClickListener    { effect = 3; sendNow(); syncUi() }

        seekBrightness?.max = 100
        seekBrightness?.progress = brightness
        seekBrightness?.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(sb: SeekBar?, p: Int, fromUser: Boolean) {
                if (!fromUser) return
                brightness = p
            }
            override fun onStartTrackingTouch(sb: SeekBar?) {}
            override fun onStopTrackingTouch(sb: SeekBar?) {
                sendNow()
                syncUi()
            }
        })

        presetRed?.setOnClickListener    { setHue(0f) }
        presetYellow?.setOnClickListener { setHue(55f) }
        presetGreen?.setOnClickListener  { setHue(120f) }
        presetBlue?.setOnClickListener   { setHue(210f) }
        presetPink?.setOnClickListener   { setHue(310f) }

        btnApply?.setOnClickListener { sendNow() }

        imgWheel?.setOnTouchListener { view, event ->
            if (event.action != MotionEvent.ACTION_DOWN &&
                event.action != MotionEvent.ACTION_MOVE) return@setOnTouchListener true

            val cx = view.width / 2f
            val cy = view.height / 2f
            val dx = event.x - cx
            val dy = event.y - cy
            val dist = sqrt(dx*dx + dy*dy)
            if (dist < min(cx, cy) * 0.10f) return@setOnTouchListener true

            val ang = atan2(dy, dx)
            var deg = Math.toDegrees(ang.toDouble()).toFloat()
            if (deg < 0f) deg += 360f
            hue = deg

            sendNow()
            syncUi()
            true
        }
    }

    private fun setHue(h: Float) {
        hue = ((h % 360f) + 360f) % 360f
        sendNow()
        syncUi()
    }

    private fun syncUi() {
        fun setSelected(tv: TextView?, sel: Boolean) {
            tv ?: return
            tv.alpha = if (sel) 1.0f else 0.55f
        }
        setSelected(modeStatic, effect == 0)
        setSelected(modeBlink,  effect == 1)
        setSelected(modeFade,   effect == 2)
        setSelected(modeRgb,    effect == 3)

        btnPower?.alpha = if (enabled) 1.0f else 0.55f

        val rgb = hsvToRgb(hue, 1f, 1f)
        val color = Color.rgb(rgb[0], rgb[1], rgb[2])
        btnApply?.setTextColor(color)

        seekBrightness?.progress = brightness
    }

    private fun sendNow() {
        val rgb = hsvToRgb(hue, 1f, 1f)
        val modeByte = ((if (enabled) 0x80 else 0x00) or (effect and 0x7F)).toByte()

        NeonRpc.sendNeonControl(
            vehId,
            modeByte,
            rgb[0].toByte(),
            rgb[1].toByte(),
            rgb[2].toByte(),
            brightness.toByte(),
            speed.toByte()
        )
    }

    private fun applyJsonState(o: JSONObject, applySend: Boolean) {
        // поддерживаем разные ключи, чтобы было проще с нативом/сервером
        if (o.has("vehId")) vehId = o.optInt("vehId", vehId)
        if (o.has("vehicleId")) vehId = o.optInt("vehicleId", vehId)

        if (o.has("enabled")) enabled = o.optInt("enabled", if (enabled) 1 else 0) != 0
        if (o.has("effect")) effect = o.optInt("effect", effect)
        if (o.has("mode")) effect = o.optInt("mode", effect)

        if (o.has("brightness")) brightness = o.optInt("brightness", brightness).coerceIn(0, 100)
        if (o.has("speed")) speed = o.optInt("speed", speed).coerceIn(0, 3)

        if (o.has("hue")) hue = o.optDouble("hue", hue.toDouble()).toFloat()
        if (o.has("color")) {
            // color может быть #RRGGBB или int
            val cAny = o.opt("color")
            val c = when (cAny) {
                is String -> try { Color.parseColor(cAny) } catch (_: Throwable) { 0 }
                is Int -> cAny
                else -> 0
            }
            if (c != 0) {
                // приблизительно восстановим hue из rgb
                val hsv = FloatArray(3)
                Color.colorToHSV(c, hsv)
                hue = hsv[0]
            }
        }

        // нормализуем
        hue = ((hue % 360f) + 360f) % 360f
        effect = effect.coerceIn(0, 3)

        if (applySend) sendNow()
    }

    // returns [r,g,b] 0..255
    private fun hsvToRgb(h: Float, s: Float, v: Float): IntArray {
        val hh = ((h % 360f) + 360f) % 360f
        val c = v * s
        val x = c * (1f - kotlin.math.abs((hh / 60f) % 2f - 1f))
        val m = v - c

        val (r1, g1, b1) = when {
            hh < 60f -> Triple(c, x, 0f)
            hh < 120f -> Triple(x, c, 0f)
            hh < 180f -> Triple(0f, c, x)
            hh < 240f -> Triple(0f, x, c)
            hh < 300f -> Triple(x, 0f, c)
            else -> Triple(c, 0f, x)
        }

        val r = ((r1 + m) * 255f).toInt()
        val g = ((g1 + m) * 255f).toInt()
        val b = ((b1 + m) * 255f).toInt()

        return intArrayOf(
            min(255, max(0, r)),
            min(255, max(0, g)),
            min(255, max(0, b))
        )
    }

    /**
     * Пытаемся достать Activity и root ViewGroup через reflection.
     * Подстраховка, чтобы не требовать от тебя правок по всему проекту.
     *
     * Если у тебя есть конкретный класс-держатель — скажи, я заменю на прямой вызов.
     */
    private fun tryResolveHost(): Pair<Activity?, ViewGroup?>? {
        val candidates = listOf(
            // попытки найти поля/методы в разных местах
            "com.multirium.mobile.gui.GUIManager",
            "com.multirium.mobile.GUIManager",
            "com.multirium.mobile.core.Samp",
            "com.multirium.mobile.Samp",
            "com.multirium.mobile.MainActivity"
        )

        for (cn in candidates) {
            try {
                val clz = Class.forName(cn)
                // 1) поля activity/root/rootView
                val act = runCatching {
                    clz.getDeclaredField("activity").apply { isAccessible = true }.get(null) as? Activity
                }.getOrNull() ?: runCatching {
                    clz.getDeclaredField("mActivity").apply { isAccessible = true }.get(null) as? Activity
                }.getOrNull()

                val rt = runCatching {
                    clz.getDeclaredField("root").apply { isAccessible = true }.get(null) as? ViewGroup
                }.getOrNull() ?: runCatching {
                    clz.getDeclaredField("rootView").apply { isAccessible = true }.get(null) as? ViewGroup
                }.getOrNull() ?: runCatching {
                    clz.getDeclaredField("mRoot").apply { isAccessible = true }.get(null) as? ViewGroup
                }.getOrNull()

                if (act != null && rt != null) return Pair(act, rt)

                // 2) методы getActivity/getRoot
                val act2 = runCatching {
                    clz.getDeclaredMethod("getActivity").apply { isAccessible = true }.invoke(null) as? Activity
                }.getOrNull()

                val rt2 = runCatching {
                    clz.getDeclaredMethod("getRoot").apply { isAccessible = true }.invoke(null) as? ViewGroup
                }.getOrNull()

                if (act2 != null && rt2 != null) return Pair(act2, rt2)
            } catch (_: Throwable) {
                // ignore
            }
        }
        return null
    }
}
