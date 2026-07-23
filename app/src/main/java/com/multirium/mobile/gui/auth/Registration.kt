package com.multirium.mobile.gui.auth

import android.animation.ArgbEvaluator
import android.animation.ValueAnimator
import android.content.res.ColorStateList
import android.graphics.Color
import android.media.MediaPlayer
import android.os.Handler
import android.os.Looper
import android.view.View
import android.view.animation.AnimationUtils
import android.widget.ScrollView
import android.widget.TextView
import com.multirium.mobile.R
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.RegLayoutBinding
import com.multirium.mobile.gui.NativeGui
import kotlin.jvm.internal.Reflection

class Registration : NativeGui<RegLayoutBinding>(
    Reflection.getOrCreateKotlinClass(RegLayoutBinding::class.java),
    false, false, true, true
) {
    override fun receivePacket(type: Int, data: String?) {}

    private var mediaPlayer: MediaPlayer? = null
    private val bind: RegLayoutBinding = getBinding() as RegLayoutBinding
    private val handler = Handler(Looper.getMainLooper())
    private var chooseSex = 0
    private var tutorialStep = 0
    private var highlightAnim: ValueAnimator? = null

    // Native методы
    external fun onRegisterClick(pass: String, mail: String, sex: Int)
    external fun onRegisterSkinBackClick()
    external fun onRegisterSkinNextClick()
    external fun onRegisterMaleSend(sex: Int)

    init {
        activity.runOnUiThread { setupUI() }
    }

    /** 🧩 Инициализация интерфейса */
    private fun setupUI() {
        val gridAnim = AnimationUtils.loadAnimation(activity, R.anim.fade_in)
        bind.holoGrid.startAnimation(gridAnim)
        hideAllUI()

        bind.skipButton.setOnClickListener {
            mediaPlayer?.stop()
            mediaPlayer?.release()
            clearHighlight()
            enableUserControls()
            bind.novaContainer.visibility = View.GONE
            showAllMainUI()
        }

        playStep(0)
    }

    /** 🎬 Обучение Новы — пошаговое с озвучкой */
    private fun playStep(step: Int) {
        val fadeAnim = AnimationUtils.loadAnimation(activity, R.anim.fade_in)
        tutorialStep = step

        when (step) {

            // 🔹 Приветствие
            0 -> showNovaMessage(
                """
                    Привет! 🌞  
                    Ого! Ты правда здесь!  
                    Я так рада тебя видеть!  
                    Добро пожаловать в мир Gtech Mobile! 💫  

                    Меня зовут Нова! ✨  
                    Я твой напарник, помощник и просто весёлый бот,  
                    который будет рядом, пока ты создаёшь своего героя! 😄
                """.trimIndent(),
                R.raw.nova_intro
            ) { playStep(1) }

            // 🔹 Email
            1 -> {
                bind.formContainer.visibility = View.VISIBLE
                bind.emailInput.visibility = View.VISIBLE
                bind.emailInput.startAnimation(fadeAnim)
                highlightUI(bind.emailInput)
                showNovaMessage(
                    """
                        Сначала — давай введём твой e-mail! 💌  
                        Так я смогу запомнить тебя  
                        не просто как игрока, а как нового друга! 💕
                    """.trimIndent(),
                    R.raw.nova_email
                ) { playStep(2) }
            }

            // 🔹 Пароль
            2 -> {
                bind.passInput.visibility = View.VISIBLE
                bind.passInput2.visibility = View.VISIBLE
                bind.passInput.startAnimation(fadeAnim)
                highlightUI(bind.passInput)
                showNovaMessage(
                    """
                        Теперь придумай пароль! 🔑  
                        Это как твой ключ — не только от аккаунта,  
                        а от целого приключения!  
                        Только не теряй его, ладно?  
                        Я не хочу, чтобы ты потом ломал голову! 😅  

                        Отлично! Всё получается просто супер!
                    """.trimIndent(),
                    R.raw.nova_password
                ) { playStep(3) }
            }

            // 🔹 Пол
            3 -> {
                bind.genderContainer.visibility = View.VISIBLE
                bind.genderContainer.startAnimation(fadeAnim)
                highlightUI(bind.genderContainer)
                showNovaMessage(
                    """
                        Теперь самое интересное… кто ты? 💭  

                        💠 Мужчина — уверенный, смелый, с добрым сердцем  
                        и отличным чувством юмора!  
                        💠 Или женщина — сильная, умная и немного  
                        опасно обаятельная! 😉  

                        Выбирай спокойно. Не торопись.
                    """.trimIndent(),
                    R.raw.nova_gender
                ) { playStep(4) }
            }

            // 🔹 Скин
            4 -> {
                bind.btnSkinLeft.visibility = View.VISIBLE
                bind.btnSkinRight.visibility = View.VISIBLE
                highlightUI(bind.btnSkinLeft)
                showNovaMessage(
                    """
                        Посмотри, как твой персонаж оживает на экране.  
                        Листай стрелками ⬅️➡️ и найди тот образ,  
                        при виде которого захочется сказать:  
                        “Вот он! Это я!” 😄  

                        Ха! Здорово получается, правда?  
                        Ты уже почти готов!
                    """.trimIndent(),
                    R.raw.nova_skin_select
                ) { playStep(5) }
            }

            // 🔹 Подтверждение
            5 -> {
                bind.btnContinue.visibility = View.VISIBLE
                bind.btnContinue.startAnimation(fadeAnim)
                highlightUI(bind.btnContinue)
                showNovaMessage(
                    """
                        Когда почувствуешь, что всё совпало —  
                        нажми “Подтвердить”.  

                        И тогда начнётся твоя история. ✨
                    """.trimIndent(),
                    R.raw.nova_confirm
                ) { playStep(6) }
            }

            // 🔹 Финал
            6 -> {
                clearHighlight()
                if (chooseSex == 0) chooseSex = 1
                onRegisterMaleSend(chooseSex)
                showNovaMessage(
                    """
                        Добро пожаловать в Gtech Mobile! 🌍  
                        Здесь всё возможно —  
                        новые друзья, весёлые приключения и тысячи историй,  
                        которые ждут именно тебя! 💫  

                        Иногда тут бывает сложно,  
                        но это же часть веселья, правда? 😜  
                        Без приключений не бывает героев!  

                        А я буду рядом — подскажу, подбодрю и…  
                        может, немного подшучу 😄
                    """.trimIndent(),
                    R.raw.nova_outro
                ) {
                    hideNovaSmooth()
                    enableUserControls()
                }
            }
        }
    }

    /** 💬 Отображение текста Новы + озвучка */
    private fun showNovaMessage(text: String, voiceRes: Int, onComplete: () -> Unit) {
        val container = bind.novaContainer
        val textView = bind.novaText
        val scrollView = bind.novaScroll

        mediaPlayer?.release()
        mediaPlayer = MediaPlayer.create(activity, voiceRes)
        mediaPlayer?.setVolume(0.9f, 0.9f)
        mediaPlayer?.start()

        if (container.visibility != View.VISIBLE) {
            val anim = AnimationUtils.loadAnimation(activity, R.anim.fade_in)
            container.startAnimation(anim)
            container.visibility = View.VISIBLE
        }

        typewriterEffect(textView, scrollView, text, 25L)
        mediaPlayer?.setOnCompletionListener {
            it.release()
            mediaPlayer = null
            handler.postDelayed(onComplete, 800)
        }
    }

    /** ✨ Подсветка текущего элемента */
    private fun highlightUI(target: View) {
        clearHighlight()
        target.bringToFront()
        bind.novaContainer.bringToFront()

        highlightAnim = ValueAnimator.ofObject(
            ArgbEvaluator(),
            Color.parseColor("#0014E5FF"),
            Color.parseColor("#6614E5FF")
        ).apply {
            repeatCount = ValueAnimator.INFINITE
            repeatMode = ValueAnimator.REVERSE
            addUpdateListener {
                val color = it.animatedValue as Int
                target.backgroundTintList =
                    ColorStateList.valueOf(color)
            }
            start()
        }
        target.animate().scaleX(1.05f).scaleY(1.05f).setDuration(300).start()
    }

    private fun clearHighlight() {
        highlightAnim?.cancel()
    }

    /** 🔒 Скрытие всех элементов */
    private fun hideAllUI() {
        listOf(
            bind.formContainer,
            bind.emailInput,
            bind.passInput,
            bind.passInput2,
            bind.genderContainer,
            bind.btnContinue,
            bind.btnSkinLeft,
            bind.btnSkinRight
        ).forEach { it.visibility = View.GONE }
    }

    /** 🔓 Показ всех элементов (если пропустить обучение) */
    private fun showAllMainUI() {
        listOf(
            bind.formContainer,
            bind.emailInput,
            bind.passInput,
            bind.passInput2,
            bind.genderContainer,
            bind.btnContinue,
            bind.btnSkinLeft,
            bind.btnSkinRight
        ).forEach { it.visibility = View.VISIBLE }
    }

    private fun hideNovaSmooth() {
        bind.novaContainer.animate()
            .alpha(0f)
            .setDuration(600)
            .withEndAction {
                bind.novaContainer.visibility = View.GONE
                bind.novaContainer.alpha = 1f
            }.start()
    }

    private fun typewriterEffect(textView: TextView, scrollView: ScrollView, fullText: String, delay: Long) {
        textView.text = ""
        var index = 0
        val runnable = object : Runnable {
            override fun run() {
                if (index <= fullText.length) {
                    textView.text = fullText.substring(0, index)
                    index++
                    scrollView.post { scrollView.smoothScrollTo(0, textView.bottom) }
                    handler.postDelayed(this, delay)
                }
            }
        }
        handler.post(runnable)
    }

    /** 🕹️ Управление элементами */
    private fun enableUserControls() {
        bind.btnSkinLeft.setOnClickListener { onRegisterSkinBackClick() }
        bind.btnSkinRight.setOnClickListener { onRegisterSkinNextClick() }

        bind.boyButton.setOnClickListener {
            chooseSex = 1
            setGenderHighlight(true)
            onRegisterMaleSend(chooseSex)
        }

        bind.girlButton.setOnClickListener {
            chooseSex = 2
            setGenderHighlight(false)
            onRegisterMaleSend(chooseSex)
        }

        bind.btnContinue.setOnClickListener {
            val pass = bind.passInput.text.toString()
            val pass2 = bind.passInput2.text.toString()
            val mail = bind.emailInput.text.toString()
            if (pass.length > 5 && pass == pass2 && chooseSex != 0)
                onRegisterClick(pass, mail, chooseSex)
        }
    }

    /** 🔹 Подсветка выбранного пола */
    private fun setGenderHighlight(isMale: Boolean) {
        val active = Color.parseColor("#14E5FF")
        val inactive = Color.parseColor("#1A1A1A")

        if (isMale) {
            bind.boyButton.setBackgroundColor(active)
            bind.girlButton.setBackgroundColor(inactive)
        } else {
            bind.girlButton.setBackgroundColor(active)
            bind.boyButton.setBackgroundColor(inactive)
        }
    }

    /** 🧩 Скрытие окна регистрации плавно */
    override fun destroy() {
        activity.runOnUiThread {
            bind.root.animate().alpha(0f).setDuration(200).withEndAction {
                bind.root.visibility = View.GONE
            }.start()
        }
    }
}
