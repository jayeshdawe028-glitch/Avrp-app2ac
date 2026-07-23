package com.multirium.mobile.gui.auth

import android.view.View
import com.multirium.mobile.R
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.SpawnSelectorBinding
import com.multirium.mobile.gui.NativeGui
import kotlin.jvm.internal.Reflection

class SpawnSelect(
    val isOrg: Int,
    val isHome: Int,
    val isFamily: Int,
    val isLastPos: Int
) : NativeGui<SpawnSelectorBinding>(
    Reflection.getOrCreateKotlinClass(SpawnSelectorBinding::class.java),
    false, false, true, true
) {

    private var spawnid = 0
    private val bind: SpawnSelectorBinding = getBinding() as SpawnSelectorBinding
    external fun SendSpawn(spawnid: Int)

    init {
        activity.runOnUiThread {

            // 🔒 Замки недоступных
            bind.orgLock.visibility = if (isOrg == 0) View.VISIBLE else View.GONE
            bind.homeLock.visibility = if (isHome == 0) View.VISIBLE else View.GONE
            bind.familyLock.visibility = if (isFamily == 0) View.VISIBLE else View.GONE
            bind.posLock.visibility = if (isLastPos == 0) View.VISIBLE else View.GONE

            // 🖼️ Карты соответствия — обычные и выбранные картинки
            val defaultImages = mapOf(
                1 to R.drawable.spawnvokzal,
                2 to R.drawable.spawnorg,
                3 to R.drawable.spawnhome,
                4 to R.drawable.spawnfam,
                5 to R.drawable.spawnlast
            )

            val selectedImages = mapOf(
                1 to R.drawable.selectvokzal,
                2 to R.drawable.selectorg,
                3 to R.drawable.selecthouse,
                4 to R.drawable.selectfam,
                5 to R.drawable.selectlast
            )

            // Все кнопки спавна
            val buttons = listOf(
                bind.vokzalBtn to 1,
                bind.orgBtn to 2,
                bind.homeBtn to 3,
                bind.familyHomeBtn to 4,
                bind.posBtn to 5
            )

            // Обработчики нажатий
            buttons.forEach { (button, id) ->
                button.setOnClickListener {
                    // Проверяем доступность
                    if (
                        (id == 2 && isOrg == 0) ||
                        (id == 3 && isHome == 0) ||
                        (id == 4 && isFamily == 0) ||
                        (id == 5 && isLastPos == 0)
                    ) return@setOnClickListener

                    // Возвращаем всем кнопкам обычные картинки
                    buttons.forEach { (b, bid) ->
                        b.setImageResource(defaultImages[bid]!!)
                    }

                    // Устанавливаем выбранной её выделенную картинку
                    button.setImageResource(selectedImages[id]!!)

                    spawnid = id
                }
            }

            // 🚀 Кнопка входа
            bind.spawnBtn.setOnClickListener {
                if (spawnid != 0) SendSpawn(spawnid)
            }
        }
    }

    override fun receivePacket(type: Int, data: String?) {}
}
