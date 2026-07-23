package com.multirium.mobile.gui.keyboard
// TODO: by viski
import android.annotation.SuppressLint
import android.graphics.drawable.Drawable
import android.os.Handler
import android.os.Looper
import android.text.Editable
import android.text.TextWatcher
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import androidx.constraintlayout.widget.ConstraintLayout
import androidx.constraintlayout.widget.ConstraintSet
import androidx.core.view.isVisible
import com.joom.paranoid.Obfuscate
import com.multirium.mobile.R
import com.multirium.mobile.common.EasyAnimation
import com.multirium.mobile.core.Samp
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.SampKeyboardLayoutBinding
import com.multirium.mobile.gui.keyboard.adapter.CommandAdapter
import com.multirium.mobile.gui.util.CustomEditText
import com.multirium.mobile.gui.util.Utils
import org.ini4j.Config
import java.io.UnsupportedEncodingException
import kotlin.jvm.internal.Intrinsics
import kotlin.math.max
import kotlin.math.min


// TODO: by viski
class SAMPKeyboard(var customEditText: CustomEditText?) {
    var binding: SampKeyboardLayoutBinding =
        SampKeyboardLayoutBinding.inflate(activity.layoutInflater)
    private var currentLayout = "ru"
    val repeatUpdateHandler: Handler
    private val repeatUpdateTask: Runnable
    val repeatInterval: Long
    private var isCaps = true
    private val initialDelay: Long
    var currentCommand: String? = null
    lateinit var onClickEnter: Function0<Unit>

    private val messageHistory: MutableList<String> = mutableListOf()

    var currentHistoryIndex = -1

    val commandAdapter: CommandAdapter
    var isLockedUpperCase: Boolean = false
    var isUpperCase: Boolean = false

    fun searchCommands(str: String): MutableList<CommandInfo>? {
        val arrayList: MutableList<CommandInfo> = ArrayList()

        for (next in StaticServerCommands.INSTANCE.staticServerCommands) {
            val commandInfo = next
            val charSequence: CharSequence = str

            if (commandInfo.name.contains(charSequence, ignoreCase = true) ||
                commandInfo.description.contains(charSequence, ignoreCase = true) ||
                commandInfo.props.contains(charSequence, ignoreCase = true)) {

                arrayList.add(commandInfo)
            }
        }

        return arrayList
    }

    fun clear() {
        this.customEditText?.text?.clear()
        binding.keyboardInput.text.clear()
    }

    val view: ViewGroup
        get() {
            val root = binding.root
            Intrinsics.checkNotNullExpressionValue(root, "getRoot(...)")
            return root
        }

    private fun applyEnglishLayout() {
        val sampKeyboardLayoutBinding = this.binding
        val constraintSet = ConstraintSet()
        constraintSet.clone(sampKeyboardLayoutBinding.keyboardBg)
        val button = sampKeyboardLayoutBinding.btn1
        val button2 = sampKeyboardLayoutBinding.btn2
        val button3 = sampKeyboardLayoutBinding.btn3
        val button4 = sampKeyboardLayoutBinding.btn4
        val button5 = sampKeyboardLayoutBinding.btn5
        val button6 = sampKeyboardLayoutBinding.btn6
        val button7 = sampKeyboardLayoutBinding.btn7
        val button8 = sampKeyboardLayoutBinding.btn8
        val button9 = sampKeyboardLayoutBinding.btn9
        val button10 = sampKeyboardLayoutBinding.btn10
        val button11 = sampKeyboardLayoutBinding.btn11
        val button12 = sampKeyboardLayoutBinding.btn12
        val button13 = sampKeyboardLayoutBinding.btn13
        val button14 = sampKeyboardLayoutBinding.btn14
        val button15 = sampKeyboardLayoutBinding.btn15
        val button16 = sampKeyboardLayoutBinding.btn16
        val button17 = sampKeyboardLayoutBinding.btn17
        val button18 = sampKeyboardLayoutBinding.btn18
        val button19 = sampKeyboardLayoutBinding.btn19
        val button20 = sampKeyboardLayoutBinding.btn20
        val button21 = sampKeyboardLayoutBinding.btn21
        val button22 = sampKeyboardLayoutBinding.btn22
        val imageButton = sampKeyboardLayoutBinding.btn23
        val button23 = sampKeyboardLayoutBinding.btn24
        val button24 = sampKeyboardLayoutBinding.btn25
        val button25 = sampKeyboardLayoutBinding.btn26
        val button26 = sampKeyboardLayoutBinding.btn27
        val button27 = sampKeyboardLayoutBinding.btn28
        val button28 = sampKeyboardLayoutBinding.btn29
        val button29 = sampKeyboardLayoutBinding.btn30
        val button30 = sampKeyboardLayoutBinding.btn31
        val button31 = sampKeyboardLayoutBinding.btn32
        val imageButton2 = sampKeyboardLayoutBinding.btn33
        val button32 = sampKeyboardLayoutBinding.btn34
        val button33 = sampKeyboardLayoutBinding.btn35
        val imageButton3 = sampKeyboardLayoutBinding.btn36
        val imageButton4 = sampKeyboardLayoutBinding.btn37
        val button34 = sampKeyboardLayoutBinding.btn38
        val button35 = sampKeyboardLayoutBinding.btn39
        val imageButton5 = sampKeyboardLayoutBinding.btn40
        clearConstraints(
            constraintSet,
            button,
            button2,
            button3,
            button4,
            button5,
            button6,
            button7,
            button8,
            button9,
            button10,
            button11,
            button12,
            button13,
            button14,
            button15,
            button16,
            button17,
            button18,
            button19,
            button20,
            button21,
            button22,
            imageButton,
            button23,
            button24,
            button25,
            button26,
            button27,
            button28,
            button29,
            button30,
            button31,
            imageButton2,
            button32,
            button33,
            imageButton3,
            imageButton4,
            button34,
            button35,
            imageButton5
        )
        constraintSet.applyTo(sampKeyboardLayoutBinding.keyboardBg)
        resetBias()
        sampKeyboardLayoutBinding.btn11.visibility = View.INVISIBLE
        sampKeyboardLayoutBinding.btn21.visibility = View.INVISIBLE
        sampKeyboardLayoutBinding.btn22.visibility = View.INVISIBLE
        sampKeyboardLayoutBinding.btn31.visibility = View.INVISIBLE
        sampKeyboardLayoutBinding.btn32.visibility = View.INVISIBLE
        sampKeyboardLayoutBinding.btn1.text = "q"
        sampKeyboardLayoutBinding.btn2.text = "w"
        sampKeyboardLayoutBinding.btn3.text = "e"
        sampKeyboardLayoutBinding.btn4.text = "r"
        sampKeyboardLayoutBinding.btn5.text = "t"
        sampKeyboardLayoutBinding.btn6.text = "y"
        sampKeyboardLayoutBinding.btn7.text = "u"
        sampKeyboardLayoutBinding.btn8.text = "i"
        sampKeyboardLayoutBinding.btn9.text = "o"
        sampKeyboardLayoutBinding.btn10.text = "p"
        sampKeyboardLayoutBinding.btn11.text = "Х"
        sampKeyboardLayoutBinding.btn12.text = "a"
        sampKeyboardLayoutBinding.btn13.text = "s"
        sampKeyboardLayoutBinding.btn14.text = "d"
        sampKeyboardLayoutBinding.btn15.text = "f"
        sampKeyboardLayoutBinding.btn16.text = "g"
        sampKeyboardLayoutBinding.btn17.text = "h"
        sampKeyboardLayoutBinding.btn18.text = "j"
        sampKeyboardLayoutBinding.btn19.text = "k"
        sampKeyboardLayoutBinding.btn20.text = "l"
        sampKeyboardLayoutBinding.btn21.text = "Ж"
        sampKeyboardLayoutBinding.btn22.text = "Э"
        sampKeyboardLayoutBinding.btn24.text = "z"
        sampKeyboardLayoutBinding.btn25.text = "x"
        sampKeyboardLayoutBinding.btn26.text = "c"
        sampKeyboardLayoutBinding.btn27.text = "v"
        sampKeyboardLayoutBinding.btn28.text = "b"
        sampKeyboardLayoutBinding.btn29.text = "n"
        sampKeyboardLayoutBinding.btn30.text = "m"
        sampKeyboardLayoutBinding.btn31.text = "Б"
        sampKeyboardLayoutBinding.btn32.text = "Ю"
        sampKeyboardLayoutBinding.btn34.text = "/"
        sampKeyboardLayoutBinding.btn35.text = ","
        sampKeyboardLayoutBinding.btn38.text = Config.DEFAULT_GLOBAL_SECTION_NAME
        sampKeyboardLayoutBinding.btn39.text = "!"
        val constraintSet2 = ConstraintSet()
        constraintSet2.clone(binding.keyboardBg)
        val sampKeyboardLayoutBinding2 = this.binding
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn1.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn1.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn2.id,
            6,
            sampKeyboardLayoutBinding2.btn1.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn2.id,
            7,
            sampKeyboardLayoutBinding2.btn3.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn2.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn3.id,
            6,
            sampKeyboardLayoutBinding2.btn2.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn3.id,
            7,
            sampKeyboardLayoutBinding2.btn4.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn3.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn4.id,
            6,
            sampKeyboardLayoutBinding2.btn3.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn4.id,
            7,
            sampKeyboardLayoutBinding2.btn5.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn4.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn5.id,
            6,
            sampKeyboardLayoutBinding2.btn4.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn5.id,
            7,
            sampKeyboardLayoutBinding2.btn6.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn5.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn6.id,
            6,
            sampKeyboardLayoutBinding2.btn5.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn6.id,
            7,
            sampKeyboardLayoutBinding2.btn7.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn6.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn7.id,
            6,
            sampKeyboardLayoutBinding2.btn6.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn7.id,
            7,
            sampKeyboardLayoutBinding2.btn8.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn7.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn8.id,
            6,
            sampKeyboardLayoutBinding2.btn7.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn8.id,
            7,
            sampKeyboardLayoutBinding2.btn9.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn8.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn9.id,
            6,
            sampKeyboardLayoutBinding2.btn8.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn9.id,
            7,
            sampKeyboardLayoutBinding2.btn10.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn9.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn10.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn10.id,
            3,
            sampKeyboardLayoutBinding2.keyboardInput.id,
            4
        )
        constraintSet2.clear(sampKeyboardLayoutBinding2.btn10.id, 6)
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn12.id, 6, 0, 6)
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn12.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn12.id,
            3,
            sampKeyboardLayoutBinding2.btn1.id,
            4
        )
        constraintSet2.setHorizontalBias(sampKeyboardLayoutBinding2.btn12.id, 0.06f)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn13.id,
            6,
            sampKeyboardLayoutBinding2.btn12.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn13.id,
            7,
            sampKeyboardLayoutBinding2.btn14.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn13.id,
            3,
            sampKeyboardLayoutBinding2.btn2.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn14.id,
            6,
            sampKeyboardLayoutBinding2.btn13.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn14.id,
            7,
            sampKeyboardLayoutBinding2.btn15.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn14.id,
            3,
            sampKeyboardLayoutBinding2.btn3.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn15.id,
            6,
            sampKeyboardLayoutBinding2.btn14.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn15.id,
            7,
            sampKeyboardLayoutBinding2.btn16.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn15.id,
            3,
            sampKeyboardLayoutBinding2.btn4.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn16.id,
            6,
            sampKeyboardLayoutBinding2.btn15.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn16.id,
            7,
            sampKeyboardLayoutBinding2.btn17.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn16.id,
            3,
            sampKeyboardLayoutBinding2.btn5.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn17.id,
            6,
            sampKeyboardLayoutBinding2.btn16.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn17.id,
            7,
            sampKeyboardLayoutBinding2.btn18.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn17.id,
            3,
            sampKeyboardLayoutBinding2.btn6.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn18.id,
            6,
            sampKeyboardLayoutBinding2.btn17.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn18.id,
            7,
            sampKeyboardLayoutBinding2.btn19.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn18.id,
            3,
            sampKeyboardLayoutBinding2.btn7.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn19.id,
            6,
            sampKeyboardLayoutBinding2.btn18.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn19.id,
            7,
            sampKeyboardLayoutBinding2.btn20.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn19.id,
            3,
            sampKeyboardLayoutBinding2.btn8.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn20.id, 6, 0, 6)
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn20.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn20.id,
            3,
            sampKeyboardLayoutBinding2.btn9.id,
            4
        )
        constraintSet2.setHorizontalBias(sampKeyboardLayoutBinding2.btn20.id, 0.94f)
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn23.id, 6, 0, 6)
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn23.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn23.id,
            3,
            sampKeyboardLayoutBinding2.btn12.id,
            4
        )
        constraintSet2.setHorizontalBias(sampKeyboardLayoutBinding2.btn23.id, 0.05f)
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn24.id, 6, 0, 6)
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn24.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn24.id,
            3,
            sampKeyboardLayoutBinding2.btn13.id,
            4
        )
        constraintSet2.setHorizontalBias(sampKeyboardLayoutBinding2.btn24.id, 0.17f)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn25.id,
            6,
            sampKeyboardLayoutBinding2.btn24.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn25.id,
            7,
            sampKeyboardLayoutBinding2.btn26.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn25.id,
            3,
            sampKeyboardLayoutBinding2.btn14.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn26.id,
            6,
            sampKeyboardLayoutBinding2.btn25.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn26.id,
            7,
            sampKeyboardLayoutBinding2.btn27.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn26.id,
            3,
            sampKeyboardLayoutBinding2.btn15.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn27.id,
            6,
            sampKeyboardLayoutBinding2.btn26.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn27.id,
            7,
            sampKeyboardLayoutBinding2.btn28.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn27.id,
            3,
            sampKeyboardLayoutBinding2.btn16.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn28.id,
            6,
            sampKeyboardLayoutBinding2.btn27.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn28.id,
            7,
            sampKeyboardLayoutBinding2.btn29.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn28.id,
            3,
            sampKeyboardLayoutBinding2.btn17.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn29.id,
            6,
            sampKeyboardLayoutBinding2.btn28.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn29.id,
            7,
            sampKeyboardLayoutBinding2.btn30.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn29.id,
            3,
            sampKeyboardLayoutBinding2.btn18.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn30.id,
            6,
            sampKeyboardLayoutBinding2.btn29.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn30.id,
            7,
            sampKeyboardLayoutBinding2.btn33.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn30.id,
            3,
            sampKeyboardLayoutBinding2.btn19.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn33.id, 6, 0, 6)
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn33.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn33.id,
            3,
            sampKeyboardLayoutBinding2.btn20.id,
            4
        )
        constraintSet2.setHorizontalBias(sampKeyboardLayoutBinding2.btn33.id, 0.965f)
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn34.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn34.id,
            3,
            sampKeyboardLayoutBinding2.btn23.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn35.id,
            6,
            sampKeyboardLayoutBinding2.btn34.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn35.id,
            7,
            sampKeyboardLayoutBinding2.btn36.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn35.id,
            3,
            sampKeyboardLayoutBinding2.btn24.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn36.id,
            6,
            sampKeyboardLayoutBinding2.btn35.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn36.id,
            7,
            sampKeyboardLayoutBinding2.btn37.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn36.id,
            3,
            sampKeyboardLayoutBinding2.btn25.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn37.id,
            6,
            sampKeyboardLayoutBinding2.btn36.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn37.id,
            7,
            sampKeyboardLayoutBinding2.btn38.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn37.id,
            3,
            sampKeyboardLayoutBinding2.btn26.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn38.id,
            6,
            sampKeyboardLayoutBinding2.btn37.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn38.id,
            7,
            sampKeyboardLayoutBinding2.btn39.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn38.id,
            3,
            sampKeyboardLayoutBinding2.btn27.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn39.id,
            6,
            sampKeyboardLayoutBinding2.btn38.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn39.id,
            7,
            sampKeyboardLayoutBinding2.btn40.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn39.id,
            3,
            sampKeyboardLayoutBinding2.btn28.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn40.id,
            6,
            sampKeyboardLayoutBinding2.btn39.id,
            7
        )
        constraintSet2.connect(sampKeyboardLayoutBinding2.btn40.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding2.btn40.id,
            3,
            sampKeyboardLayoutBinding2.btn29.id,
            4
        )
        constraintSet2.applyTo(binding.keyboardBg)
        setupButtonListeners()
        this.isLockedUpperCase = true
    }

    private fun applyRussianLayout() {
        val sampKeyboardLayoutBinding = this.binding
        val constraintSet = ConstraintSet()
        constraintSet.clone(sampKeyboardLayoutBinding.keyboardBg)
        val button = sampKeyboardLayoutBinding.btn1
        val button2 = sampKeyboardLayoutBinding.btn2
        val button3 = sampKeyboardLayoutBinding.btn3
        val button4 = sampKeyboardLayoutBinding.btn4
        val button5 = sampKeyboardLayoutBinding.btn5
        val button6 = sampKeyboardLayoutBinding.btn6
        val button7 = sampKeyboardLayoutBinding.btn7
        val button8 = sampKeyboardLayoutBinding.btn8
        val button9 = sampKeyboardLayoutBinding.btn9
        val button10 = sampKeyboardLayoutBinding.btn10
        val button11 = sampKeyboardLayoutBinding.btn11
        val button12 = sampKeyboardLayoutBinding.btn12
        val button13 = sampKeyboardLayoutBinding.btn13
        val button14 = sampKeyboardLayoutBinding.btn14
        val button15 = sampKeyboardLayoutBinding.btn15
        val button16 = sampKeyboardLayoutBinding.btn16
        val button17 = sampKeyboardLayoutBinding.btn17
        val button18 = sampKeyboardLayoutBinding.btn18
        val button19 = sampKeyboardLayoutBinding.btn19
        val button20 = sampKeyboardLayoutBinding.btn20
        val button21 = sampKeyboardLayoutBinding.btn21
        val button22 = sampKeyboardLayoutBinding.btn22
        val imageButton = sampKeyboardLayoutBinding.btn23
        val button23 = sampKeyboardLayoutBinding.btn24
        val button24 = sampKeyboardLayoutBinding.btn25
        val button25 = sampKeyboardLayoutBinding.btn26
        val button26 = sampKeyboardLayoutBinding.btn27
        val button27 = sampKeyboardLayoutBinding.btn28
        val button28 = sampKeyboardLayoutBinding.btn29
        val button29 = sampKeyboardLayoutBinding.btn30
        val button30 = sampKeyboardLayoutBinding.btn31
        val button31 = sampKeyboardLayoutBinding.btn32
        val imageButton2 = sampKeyboardLayoutBinding.btn33
        val button32 = sampKeyboardLayoutBinding.btn34
        val button33 = sampKeyboardLayoutBinding.btn35
        val imageButton3 = sampKeyboardLayoutBinding.btn36
        val imageButton4 = sampKeyboardLayoutBinding.btn37
        val button34 = sampKeyboardLayoutBinding.btn38
        val button35 = sampKeyboardLayoutBinding.btn39
        val imageButton5 = sampKeyboardLayoutBinding.btn40
        clearConstraints(
            constraintSet,
            button,
            button2,
            button3,
            button4,
            button5,
            button6,
            button7,
            button8,
            button9,
            button10,
            button11,
            button12,
            button13,
            button14,
            button15,
            button16,
            button17,
            button18,
            button19,
            button20,
            button21,
            button22,
            imageButton,
            button23,
            button24,
            button25,
            button26,
            button27,
            button28,
            button29,
            button30,
            button31,
            imageButton2,
            button32,
            button33,
            imageButton3,
            imageButton4,
            button34,
            button35,
            imageButton5
        )
        constraintSet.applyTo(sampKeyboardLayoutBinding.keyboardBg)
        resetBias()
        sampKeyboardLayoutBinding.btn11.visibility = View.VISIBLE
        sampKeyboardLayoutBinding.btn21.visibility = View.VISIBLE
        sampKeyboardLayoutBinding.btn22.visibility = View.VISIBLE
        sampKeyboardLayoutBinding.btn31.visibility = View.VISIBLE
        sampKeyboardLayoutBinding.btn32.visibility = View.VISIBLE
        sampKeyboardLayoutBinding.btn1.text = "й"
        sampKeyboardLayoutBinding.btn2.text = "ц"
        sampKeyboardLayoutBinding.btn3.text = "у"
        sampKeyboardLayoutBinding.btn4.text = "к"
        sampKeyboardLayoutBinding.btn5.text = "е"
        sampKeyboardLayoutBinding.btn6.text = "н"
        sampKeyboardLayoutBinding.btn7.text = "г"
        sampKeyboardLayoutBinding.btn8.text = "ш"
        sampKeyboardLayoutBinding.btn9.text = "щ"
        sampKeyboardLayoutBinding.btn10.text = "з"
        sampKeyboardLayoutBinding.btn11.text = "х"
        sampKeyboardLayoutBinding.btn12.text = "ф"
        sampKeyboardLayoutBinding.btn13.text = "ы"
        sampKeyboardLayoutBinding.btn14.text = "в"
        sampKeyboardLayoutBinding.btn15.text = "а"
        sampKeyboardLayoutBinding.btn16.text = "п"
        sampKeyboardLayoutBinding.btn17.text = "р"
        sampKeyboardLayoutBinding.btn18.text = "о"
        sampKeyboardLayoutBinding.btn19.text = "л"
        sampKeyboardLayoutBinding.btn20.text = "д"
        sampKeyboardLayoutBinding.btn21.text = "ж"
        sampKeyboardLayoutBinding.btn22.text = "э"
        sampKeyboardLayoutBinding.btn24.text = "я"
        sampKeyboardLayoutBinding.btn25.text = "ч"
        sampKeyboardLayoutBinding.btn26.text = "с"
        sampKeyboardLayoutBinding.btn27.text = "м"
        sampKeyboardLayoutBinding.btn28.text = "и"
        sampKeyboardLayoutBinding.btn29.text = "т"
        sampKeyboardLayoutBinding.btn30.text = "ь"
        sampKeyboardLayoutBinding.btn31.text = "б"
        sampKeyboardLayoutBinding.btn32.text = "ю"
        //sampKeyboardLayoutBinding.btn34.setText(RemoteSettings.FORWARD_SLASH_STRING);
        sampKeyboardLayoutBinding.btn34.text = "/"
        // sampKeyboardLayoutBinding.btn35.setText(StringUtils.COMMA);
        sampKeyboardLayoutBinding.btn35.text = ","
        sampKeyboardLayoutBinding.btn38.text = Config.DEFAULT_GLOBAL_SECTION_NAME
        // sampKeyboardLayoutBinding.btn38.setText("?");
        sampKeyboardLayoutBinding.btn39.text = "!"
        val constraintSet2 = ConstraintSet()
        constraintSet2.clone(sampKeyboardLayoutBinding.keyboardBg)
        constraintSet2.connect(sampKeyboardLayoutBinding.btn1.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn1.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn2.id,
            6,
            sampKeyboardLayoutBinding.btn1.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn2.id,
            7,
            sampKeyboardLayoutBinding.btn3.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn2.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn3.id,
            6,
            sampKeyboardLayoutBinding.btn2.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn3.id,
            7,
            sampKeyboardLayoutBinding.btn4.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn3.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn4.id,
            6,
            sampKeyboardLayoutBinding.btn3.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn4.id,
            7,
            sampKeyboardLayoutBinding.btn5.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn4.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn5.id,
            6,
            sampKeyboardLayoutBinding.btn4.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn5.id,
            7,
            sampKeyboardLayoutBinding.btn6.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn5.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn6.id,
            6,
            sampKeyboardLayoutBinding.btn5.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn6.id,
            7,
            sampKeyboardLayoutBinding.btn7.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn6.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn7.id,
            6,
            sampKeyboardLayoutBinding.btn6.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn7.id,
            7,
            sampKeyboardLayoutBinding.btn8.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn7.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn8.id,
            6,
            sampKeyboardLayoutBinding.btn7.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn8.id,
            7,
            sampKeyboardLayoutBinding.btn9.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn8.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn9.id,
            6,
            sampKeyboardLayoutBinding.btn8.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn9.id,
            7,
            sampKeyboardLayoutBinding.btn10.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn9.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn10.id,
            6,
            sampKeyboardLayoutBinding.btn9.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn10.id,
            7,
            sampKeyboardLayoutBinding.btn11.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn10.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn11.id,
            6,
            sampKeyboardLayoutBinding.btn10.id,
            7
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn11.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn11.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn12.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn12.id,
            3,
            sampKeyboardLayoutBinding.btn1.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn13.id,
            6,
            sampKeyboardLayoutBinding.btn12.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn13.id,
            7,
            sampKeyboardLayoutBinding.btn14.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn13.id,
            3,
            sampKeyboardLayoutBinding.btn2.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn14.id,
            6,
            sampKeyboardLayoutBinding.btn13.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn14.id,
            7,
            sampKeyboardLayoutBinding.btn15.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn14.id,
            3,
            sampKeyboardLayoutBinding.btn3.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn15.id,
            6,
            sampKeyboardLayoutBinding.btn14.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn15.id,
            7,
            sampKeyboardLayoutBinding.btn16.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn15.id,
            3,
            sampKeyboardLayoutBinding.btn4.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn16.id,
            6,
            sampKeyboardLayoutBinding.btn15.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn16.id,
            7,
            sampKeyboardLayoutBinding.btn17.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn16.id,
            3,
            sampKeyboardLayoutBinding.btn5.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn17.id,
            6,
            sampKeyboardLayoutBinding.btn16.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn17.id,
            7,
            sampKeyboardLayoutBinding.btn18.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn17.id,
            3,
            sampKeyboardLayoutBinding.btn6.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn18.id,
            6,
            sampKeyboardLayoutBinding.btn17.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn18.id,
            7,
            sampKeyboardLayoutBinding.btn19.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn18.id,
            3,
            sampKeyboardLayoutBinding.btn7.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn19.id,
            6,
            sampKeyboardLayoutBinding.btn18.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn19.id,
            7,
            sampKeyboardLayoutBinding.btn20.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn19.id,
            3,
            sampKeyboardLayoutBinding.btn8.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn20.id,
            6,
            sampKeyboardLayoutBinding.btn19.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn20.id,
            7,
            sampKeyboardLayoutBinding.btn21.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn20.id,
            3,
            sampKeyboardLayoutBinding.btn9.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn21.id,
            6,
            sampKeyboardLayoutBinding.btn20.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn21.id,
            7,
            sampKeyboardLayoutBinding.btn22.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn21.id,
            3,
            sampKeyboardLayoutBinding.btn10.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn22.id,
            6,
            sampKeyboardLayoutBinding.btn21.id,
            7
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn22.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn22.id,
            3,
            sampKeyboardLayoutBinding.btn11.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn23.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn23.id,
            3,
            sampKeyboardLayoutBinding.btn12.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn24.id,
            6,
            sampKeyboardLayoutBinding.btn23.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn24.id,
            7,
            sampKeyboardLayoutBinding.btn25.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn24.id,
            3,
            sampKeyboardLayoutBinding.btn13.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn25.id,
            6,
            sampKeyboardLayoutBinding.btn24.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn25.id,
            7,
            sampKeyboardLayoutBinding.btn26.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn25.id,
            3,
            sampKeyboardLayoutBinding.btn14.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn26.id,
            6,
            sampKeyboardLayoutBinding.btn25.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn26.id,
            7,
            sampKeyboardLayoutBinding.btn27.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn26.id,
            3,
            sampKeyboardLayoutBinding.btn15.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn27.id,
            6,
            sampKeyboardLayoutBinding.btn26.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn27.id,
            7,
            sampKeyboardLayoutBinding.btn28.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn27.id,
            3,
            sampKeyboardLayoutBinding.btn16.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn28.id,
            6,
            sampKeyboardLayoutBinding.btn27.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn28.id,
            7,
            sampKeyboardLayoutBinding.btn29.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn28.id,
            3,
            sampKeyboardLayoutBinding.btn17.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn29.id,
            6,
            sampKeyboardLayoutBinding.btn28.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn29.id,
            7,
            sampKeyboardLayoutBinding.btn30.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn29.id,
            3,
            sampKeyboardLayoutBinding.btn18.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn30.id,
            6,
            sampKeyboardLayoutBinding.btn29.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn30.id,
            7,
            sampKeyboardLayoutBinding.btn31.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn30.id,
            3,
            sampKeyboardLayoutBinding.btn19.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn31.id,
            6,
            sampKeyboardLayoutBinding.btn30.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn31.id,
            7,
            sampKeyboardLayoutBinding.btn32.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn31.id,
            3,
            sampKeyboardLayoutBinding.btn20.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn32.id,
            6,
            sampKeyboardLayoutBinding.btn31.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn32.id,
            7,
            sampKeyboardLayoutBinding.btn33.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn32.id,
            3,
            sampKeyboardLayoutBinding.btn21.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn33.id,
            6,
            sampKeyboardLayoutBinding.btn32.id,
            7
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn33.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn33.id,
            3,
            sampKeyboardLayoutBinding.btn22.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn34.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn34.id,
            3,
            sampKeyboardLayoutBinding.btn23.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn35.id,
            6,
            sampKeyboardLayoutBinding.btn34.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn35.id,
            7,
            sampKeyboardLayoutBinding.btn36.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn35.id,
            3,
            sampKeyboardLayoutBinding.btn24.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn36.id,
            6,
            sampKeyboardLayoutBinding.btn35.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn36.id,
            7,
            sampKeyboardLayoutBinding.btn37.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn36.id,
            3,
            sampKeyboardLayoutBinding.btn25.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn37.id,
            6,
            sampKeyboardLayoutBinding.btn36.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn37.id,
            7,
            sampKeyboardLayoutBinding.btn38.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn37.id,
            3,
            sampKeyboardLayoutBinding.btn28.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn38.id,
            6,
            sampKeyboardLayoutBinding.btn37.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn38.id,
            7,
            sampKeyboardLayoutBinding.btn39.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn38.id,
            3,
            sampKeyboardLayoutBinding.btn31.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn39.id,
            6,
            sampKeyboardLayoutBinding.btn38.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn39.id,
            7,
            sampKeyboardLayoutBinding.btn40.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn39.id,
            3,
            sampKeyboardLayoutBinding.btn32.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn40.id,
            6,
            sampKeyboardLayoutBinding.btn39.id,
            7
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn40.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn40.id,
            3,
            sampKeyboardLayoutBinding.btn33.id,
            4
        )
        constraintSet2.applyTo(sampKeyboardLayoutBinding.keyboardBg)
        setupButtonListeners()
    }

    private fun resetBias() {
        val constraintSet = ConstraintSet()
        constraintSet.clone(binding.keyboardBg)
        val sampKeyboardLayoutBinding = this.binding
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn1.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn2.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn3.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn4.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn5.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn6.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn7.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn8.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn9.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn10.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn11.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn12.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn13.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn14.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn15.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn16.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn17.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn18.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn19.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn20.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn21.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn22.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn23.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn24.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn25.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn26.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn27.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn28.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn29.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn30.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn31.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn32.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn33.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn34.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn35.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn36.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn37.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn38.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn39.id, 0.0f)
        constraintSet.setHorizontalBias(sampKeyboardLayoutBinding.btn40.id, 0.0f)
        constraintSet.applyTo(binding.keyboardBg)
    }

    private fun applyNumbersLayout() {
        val sampKeyboardLayoutBinding = this.binding
        val constraintSet = ConstraintSet()
        constraintSet.clone(sampKeyboardLayoutBinding.keyboardBg)
        val button = sampKeyboardLayoutBinding.btn1
        val button2 = sampKeyboardLayoutBinding.btn2
        val button3 = sampKeyboardLayoutBinding.btn3
        val button4 = sampKeyboardLayoutBinding.btn4
        val button5 = sampKeyboardLayoutBinding.btn5
        val button6 = sampKeyboardLayoutBinding.btn6
        val button7 = sampKeyboardLayoutBinding.btn7
        val button8 = sampKeyboardLayoutBinding.btn8
        val button9 = sampKeyboardLayoutBinding.btn9
        val button10 = sampKeyboardLayoutBinding.btn10
        val button11 = sampKeyboardLayoutBinding.btn11
        val button12 = sampKeyboardLayoutBinding.btn12
        val button13 = sampKeyboardLayoutBinding.btn13
        val button14 = sampKeyboardLayoutBinding.btn14
        val button15 = sampKeyboardLayoutBinding.btn15
        val button16 = sampKeyboardLayoutBinding.btn16
        val button17 = sampKeyboardLayoutBinding.btn17
        val button18 = sampKeyboardLayoutBinding.btn18
        val button19 = sampKeyboardLayoutBinding.btn19
        val button20 = sampKeyboardLayoutBinding.btn20
        val button21 = sampKeyboardLayoutBinding.btn21
        val button22 = sampKeyboardLayoutBinding.btn22
        val imageButton = sampKeyboardLayoutBinding.btn23
        val button23 = sampKeyboardLayoutBinding.btn24
        val button24 = sampKeyboardLayoutBinding.btn25
        val button25 = sampKeyboardLayoutBinding.btn26
        val button26 = sampKeyboardLayoutBinding.btn27
        val button27 = sampKeyboardLayoutBinding.btn28
        val button28 = sampKeyboardLayoutBinding.btn29
        val button29 = sampKeyboardLayoutBinding.btn30
        val button30 = sampKeyboardLayoutBinding.btn31
        val button31 = sampKeyboardLayoutBinding.btn32
        val imageButton2 = sampKeyboardLayoutBinding.btn33
        val button32 = sampKeyboardLayoutBinding.btn34
        val button33 = sampKeyboardLayoutBinding.btn35
        val imageButton3 = sampKeyboardLayoutBinding.btn36
        val imageButton4 = sampKeyboardLayoutBinding.btn37
        val button34 = sampKeyboardLayoutBinding.btn38
        val button35 = sampKeyboardLayoutBinding.btn39
        val imageButton5 = sampKeyboardLayoutBinding.btn40
        clearConstraints(
            constraintSet,
            button,
            button2,
            button3,
            button4,
            button5,
            button6,
            button7,
            button8,
            button9,
            button10,
            button11,
            button12,
            button13,
            button14,
            button15,
            button16,
            button17,
            button18,
            button19,
            button20,
            button21,
            button22,
            imageButton,
            button23,
            button24,
            button25,
            button26,
            button27,
            button28,
            button29,
            button30,
            button31,
            imageButton2,
            button32,
            button33,
            imageButton3,
            imageButton4,
            button34,
            button35,
            imageButton5
        )
        constraintSet.applyTo(sampKeyboardLayoutBinding.keyboardBg)
        resetBias()
        sampKeyboardLayoutBinding.btn11.visibility = View.INVISIBLE
        sampKeyboardLayoutBinding.btn22.visibility = View.INVISIBLE
        sampKeyboardLayoutBinding.btn21.visibility = View.VISIBLE
        sampKeyboardLayoutBinding.btn31.visibility = View.VISIBLE
        sampKeyboardLayoutBinding.btn32.visibility = View.VISIBLE
        sampKeyboardLayoutBinding.btn1.text = "1"
        sampKeyboardLayoutBinding.btn2.text = "2"
        sampKeyboardLayoutBinding.btn3.text = "3"
        sampKeyboardLayoutBinding.btn4.text = "4"
        sampKeyboardLayoutBinding.btn5.text = "5"
        sampKeyboardLayoutBinding.btn6.text = "6"
        sampKeyboardLayoutBinding.btn7.text = "7"
        sampKeyboardLayoutBinding.btn8.text = "8"
        sampKeyboardLayoutBinding.btn9.text = "9"
        sampKeyboardLayoutBinding.btn10.text = "0"
        sampKeyboardLayoutBinding.btn11.text = "Х"
        sampKeyboardLayoutBinding.btn12.text = "@"
        sampKeyboardLayoutBinding.btn13.text = "#"
        sampKeyboardLayoutBinding.btn14.text = "$"
        sampKeyboardLayoutBinding.btn15.text = "%"
        sampKeyboardLayoutBinding.btn16.text = "“"
        sampKeyboardLayoutBinding.btn17.text = "*"
        sampKeyboardLayoutBinding.btn18.text = "("
        sampKeyboardLayoutBinding.btn19.text = ")"
        sampKeyboardLayoutBinding.btn20.text = "-"
        sampKeyboardLayoutBinding.btn21.text = "_"
        sampKeyboardLayoutBinding.btn22.text = "Э"
        sampKeyboardLayoutBinding.btn24.text = "."
        sampKeyboardLayoutBinding.btn25.text = ":"
        sampKeyboardLayoutBinding.btn26.text = ";"
        sampKeyboardLayoutBinding.btn27.text = "+"
        sampKeyboardLayoutBinding.btn28.text = "="
        sampKeyboardLayoutBinding.btn29.text = "<"
        sampKeyboardLayoutBinding.btn30.text = ">"
        sampKeyboardLayoutBinding.btn31.text = "["
        sampKeyboardLayoutBinding.btn32.text = "]"
        sampKeyboardLayoutBinding.btn34.text = "/"
        sampKeyboardLayoutBinding.btn35.text = ","
        sampKeyboardLayoutBinding.btn38.text = Config.DEFAULT_GLOBAL_SECTION_NAME
        sampKeyboardLayoutBinding.btn39.text = "!"
        val constraintSet2 = ConstraintSet()
        constraintSet2.clone(sampKeyboardLayoutBinding.keyboardBg)
        constraintSet2.connect(sampKeyboardLayoutBinding.btn1.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn1.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn2.id,
            6,
            sampKeyboardLayoutBinding.btn1.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn2.id,
            7,
            sampKeyboardLayoutBinding.btn3.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn2.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn3.id,
            6,
            sampKeyboardLayoutBinding.btn2.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn3.id,
            7,
            sampKeyboardLayoutBinding.btn4.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn3.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn4.id,
            6,
            sampKeyboardLayoutBinding.btn3.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn4.id,
            7,
            sampKeyboardLayoutBinding.btn5.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn4.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn5.id,
            6,
            sampKeyboardLayoutBinding.btn4.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn5.id,
            7,
            sampKeyboardLayoutBinding.btn6.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn5.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn6.id,
            6,
            sampKeyboardLayoutBinding.btn5.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn6.id,
            7,
            sampKeyboardLayoutBinding.btn7.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn6.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn7.id,
            6,
            sampKeyboardLayoutBinding.btn6.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn7.id,
            7,
            sampKeyboardLayoutBinding.btn8.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn7.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn8.id,
            6,
            sampKeyboardLayoutBinding.btn7.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn8.id,
            7,
            sampKeyboardLayoutBinding.btn9.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn8.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn9.id,
            6,
            sampKeyboardLayoutBinding.btn8.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn9.id,
            7,
            sampKeyboardLayoutBinding.btn10.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn9.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn10.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn10.id,
            3,
            sampKeyboardLayoutBinding.keyboardInput.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn12.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn12.id,
            3,
            sampKeyboardLayoutBinding.btn1.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn13.id,
            6,
            sampKeyboardLayoutBinding.btn12.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn13.id,
            7,
            sampKeyboardLayoutBinding.btn14.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn13.id,
            3,
            sampKeyboardLayoutBinding.btn2.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn14.id,
            6,
            sampKeyboardLayoutBinding.btn13.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn14.id,
            7,
            sampKeyboardLayoutBinding.btn15.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn14.id,
            3,
            sampKeyboardLayoutBinding.btn3.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn15.id,
            6,
            sampKeyboardLayoutBinding.btn14.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn15.id,
            7,
            sampKeyboardLayoutBinding.btn16.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn15.id,
            3,
            sampKeyboardLayoutBinding.btn4.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn16.id,
            6,
            sampKeyboardLayoutBinding.btn15.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn16.id,
            7,
            sampKeyboardLayoutBinding.btn17.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn16.id,
            3,
            sampKeyboardLayoutBinding.btn5.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn17.id,
            6,
            sampKeyboardLayoutBinding.btn16.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn17.id,
            7,
            sampKeyboardLayoutBinding.btn18.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn17.id,
            3,
            sampKeyboardLayoutBinding.btn6.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn18.id,
            6,
            sampKeyboardLayoutBinding.btn17.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn18.id,
            7,
            sampKeyboardLayoutBinding.btn19.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn18.id,
            3,
            sampKeyboardLayoutBinding.btn7.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn19.id,
            6,
            sampKeyboardLayoutBinding.btn18.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn19.id,
            7,
            sampKeyboardLayoutBinding.btn20.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn19.id,
            3,
            sampKeyboardLayoutBinding.btn8.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn20.id,
            6,
            sampKeyboardLayoutBinding.btn19.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn20.id,
            7,
            sampKeyboardLayoutBinding.btn21.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn20.id,
            3,
            sampKeyboardLayoutBinding.btn9.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn21.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn21.id,
            3,
            sampKeyboardLayoutBinding.btn10.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn23.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn23.id,
            3,
            sampKeyboardLayoutBinding.btn12.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn24.id,
            6,
            sampKeyboardLayoutBinding.btn23.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn24.id,
            7,
            sampKeyboardLayoutBinding.btn25.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn24.id,
            3,
            sampKeyboardLayoutBinding.btn13.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn25.id,
            6,
            sampKeyboardLayoutBinding.btn24.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn25.id,
            7,
            sampKeyboardLayoutBinding.btn26.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn25.id,
            3,
            sampKeyboardLayoutBinding.btn14.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn26.id,
            6,
            sampKeyboardLayoutBinding.btn25.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn26.id,
            7,
            sampKeyboardLayoutBinding.btn27.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn26.id,
            3,
            sampKeyboardLayoutBinding.btn15.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn27.id,
            6,
            sampKeyboardLayoutBinding.btn26.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn27.id,
            7,
            sampKeyboardLayoutBinding.btn28.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn27.id,
            3,
            sampKeyboardLayoutBinding.btn16.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn28.id,
            6,
            sampKeyboardLayoutBinding.btn27.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn28.id,
            7,
            sampKeyboardLayoutBinding.btn29.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn28.id,
            3,
            sampKeyboardLayoutBinding.btn17.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn29.id,
            6,
            sampKeyboardLayoutBinding.btn28.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn29.id,
            7,
            sampKeyboardLayoutBinding.btn30.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn29.id,
            3,
            sampKeyboardLayoutBinding.btn18.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn30.id,
            6,
            sampKeyboardLayoutBinding.btn29.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn30.id,
            7,
            sampKeyboardLayoutBinding.btn31.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn30.id,
            3,
            sampKeyboardLayoutBinding.btn19.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn31.id,
            6,
            sampKeyboardLayoutBinding.btn30.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn31.id,
            7,
            sampKeyboardLayoutBinding.btn32.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn31.id,
            3,
            sampKeyboardLayoutBinding.btn20.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn32.id,
            6,
            sampKeyboardLayoutBinding.btn31.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn32.id,
            7,
            sampKeyboardLayoutBinding.btn33.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn32.id,
            3,
            sampKeyboardLayoutBinding.btn21.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn33.id,
            6,
            sampKeyboardLayoutBinding.btn32.id,
            7
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn33.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn33.id,
            3,
            sampKeyboardLayoutBinding.btn21.id,
            4
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn34.id, 6, 0, 6)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn34.id,
            3,
            sampKeyboardLayoutBinding.btn23.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn35.id,
            6,
            sampKeyboardLayoutBinding.btn34.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn35.id,
            7,
            sampKeyboardLayoutBinding.btn36.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn35.id,
            3,
            sampKeyboardLayoutBinding.btn24.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn36.id,
            6,
            sampKeyboardLayoutBinding.btn35.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn36.id,
            7,
            sampKeyboardLayoutBinding.btn37.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn36.id,
            3,
            sampKeyboardLayoutBinding.btn25.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn37.id,
            6,
            sampKeyboardLayoutBinding.btn36.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn37.id,
            7,
            sampKeyboardLayoutBinding.btn38.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn37.id,
            3,
            sampKeyboardLayoutBinding.btn28.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn38.id,
            6,
            sampKeyboardLayoutBinding.btn37.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn38.id,
            7,
            sampKeyboardLayoutBinding.btn39.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn38.id,
            3,
            sampKeyboardLayoutBinding.btn31.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn39.id,
            6,
            sampKeyboardLayoutBinding.btn38.id,
            7
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn39.id,
            7,
            sampKeyboardLayoutBinding.btn40.id,
            6
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn39.id,
            3,
            sampKeyboardLayoutBinding.btn32.id,
            4
        )
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn40.id,
            6,
            sampKeyboardLayoutBinding.btn39.id,
            7
        )
        constraintSet2.connect(sampKeyboardLayoutBinding.btn40.id, 7, 0, 7)
        constraintSet2.connect(
            sampKeyboardLayoutBinding.btn40.id,
            3,
            sampKeyboardLayoutBinding.btn33.id,
            4
        )
        constraintSet2.applyTo(sampKeyboardLayoutBinding.keyboardBg)
        setupButtonListeners()
    }

    private fun setupButtonListeners() {
        for (button in listOf(
            binding.btn1,
            binding.btn2,
            binding.btn3,
            binding.btn4,
            binding.btn5,
            binding.btn6,
            binding.btn7,
            binding.btn8,
            binding.btn9,
            binding.btn10,
            binding.btn11,
            binding.btn12,
            binding.btn13,
            binding.btn14,
            binding.btn15,
            binding.btn16,
            binding.btn17,
            binding.btn18,
            binding.btn19,
            binding.btn20,
            binding.btn21,
            binding.btn22,
            binding.btn24,
            binding.btn25,
            binding.btn26,
            binding.btn27,
            binding.btn28,
            binding.btn29,
            binding.btn30,
            binding.btn31,
            binding.btn32,
            binding.btn34,
            binding.btn35, binding.btn38, binding.btn39
        )) {
            button.setOnTouchListener { view: View, motionEvent: MotionEvent ->
                val result = setupButton(view, button, motionEvent)
                if (motionEvent.action == MotionEvent.ACTION_UP) {
                    view.performClick()
                }
                result
            }
        }
        binding.btn33.setOnTouchListener { view: View, motionEvent: MotionEvent ->
            val result = this.setupButtonDelete(view, motionEvent)
            if (motionEvent.action == MotionEvent.ACTION_UP) {
                view.performClick()
            }
            result
        }
        binding.btn23.setOnClickListener { view: View ->
            this.setupButtonShift(
                view
            )
        }
        binding.btn37.setOnTouchListener { view: View, motionEvent: MotionEvent ->
            when (motionEvent.action) {
                MotionEvent.ACTION_DOWN -> {
                    this.addSpaceAtCursor()
                    view.setBackgroundResource(R.drawable.samp_keyboard_active)
                    return@setOnTouchListener true
                }
                MotionEvent.ACTION_UP -> {
                    view.setBackgroundResource(0)
                    return@setOnTouchListener true
                }
                MotionEvent.ACTION_CANCEL -> {
                    view.setBackgroundResource(0)
                    return@setOnTouchListener true
                }
            }
            false
        }
        binding.btn40.setOnClickListener { view: View ->
            this.setupButtonSend(
                view
            )
        }
        binding.historyNext.setOnClickListener { view: View ->
            this.setupButtonHistoryUp(
                view
            )
        }
        binding.historyPrevious.setOnClickListener { view: View ->
            this.setupButtonHistoryDown(
                view
            )
        }
    }

    private fun setupButtonHistoryUp(view: View) {
        this.views = view
        this.drawable = view.background

        val editText = binding.keyboardInput

        /*val i = this.idSelectedHistoryTexts + 1
        if (i in HistoryTexts.indices && !HistoryTexts[i].isNullOrEmpty()) {
            this.idSelectedHistoryTexts = i
            editText.setText(HistoryTexts[i])
            editText.setSelection(editText.text.toString().length)
            editText.requestFocus()
        }*/

        if (currentHistoryIndex < messageHistory.size - 1) {
            currentHistoryIndex++
            editText.setText(messageHistory[currentHistoryIndex])
            editText.setSelection(messageHistory[currentHistoryIndex].length)
        }

        view.setBackgroundResource(R.drawable.samp_keyboard_active)
        Handler(Looper.getMainLooper()).postDelayed({
            views?.background = drawable
        }, 25)
    }

    private fun setupButtonHistoryDown(view: View) {
        this.views = view
        this.drawable = view.background

        val editText = binding.keyboardInput

        if (currentHistoryIndex > 0) {
            currentHistoryIndex -= 1
            editText.setText(messageHistory[currentHistoryIndex])
            editText.setSelection(messageHistory[currentHistoryIndex].length)
        } else {
            editText.setText("")
            currentHistoryIndex = -1
        }
        /*
                val i = this.idSelectedHistoryTexts - 1
                if (i >= -1) {
                    if (i == -1) {
                        editText.setText("")
                        this.idSelectedHistoryTexts = i
                    } else if (i in HistoryTexts.indices && !HistoryTexts[i].isNullOrEmpty()) {
                        this.idSelectedHistoryTexts = i
                        editText.setText(HistoryTexts[i])
                        editText.setSelection(editText.text.toString().length)
                    }
                }
        */
        view.setBackgroundResource(R.drawable.samp_keyboard_active)
        Handler(Looper.getMainLooper()).postDelayed({
            views?.background = drawable
        }, 25)
    }
    fun setupButtonSend(view: View) {
        this.views = view
        this.drawable = view.background

        var message = binding.keyboardInput.text.toString().trim()

        currentCommand?.let { command ->
            message = "$command $message"
        }

        val charSequence: CharSequence = message
        currentCommand = null

        onClickEnter.invoke()

        customEditText?.setText(charSequence);
        customEditText?.setSelection(message.length);

        if (message.isNotEmpty()) {
            messageHistory.add(0, message)
            currentHistoryIndex = -1
        }

        binding.keyboardInput.text.clear()

        view.setBackgroundResource(R.drawable.samp_keyboard_active)
        Handler(Looper.getMainLooper()).postDelayed({
            view.background =
                drawable
        }, 25)
    }

    private fun setupButtonSpace(view: View) {
        this.addSpaceAtCursor()
        this.views = view
        this.drawable = view.background
        view.setBackgroundResource(R.drawable.samp_keyboard_active)
        Handler(Looper.getMainLooper()).postDelayed({
            views?.background = drawable
        }, 25)
    }

    private fun setupButton(view: View, button: Button, motionEvent: MotionEvent): Boolean {
        when (motionEvent.action) {
            MotionEvent.ACTION_DOWN -> {
                this.onButtonClick(button)
                view.setBackgroundResource(R.drawable.samp_keyboard_active)
                return true
            }
            MotionEvent.ACTION_UP -> {
                view.setBackgroundResource(0)
                return true
            }
            MotionEvent.ACTION_CANCEL -> {
                view.setBackgroundResource(0)
                return true
            }
        }
        return false
    }

    private fun addSpaceAtCursor() {
        val selectionStart = binding.keyboardInput.selectionStart
        if (selectionStart >= 0) {
            binding.keyboardInput.text.insert(selectionStart, " ")
        }
    }

    private fun onButtonClick(button: Button) {
        val min = min(
            binding.keyboardInput.selectionStart.toDouble(),
            binding.keyboardInput.selectionEnd.toDouble()
        ).toInt()
        val max = max(
            binding.keyboardInput.selectionStart.toDouble(),
            binding.keyboardInput.selectionEnd.toDouble()
        ).toInt()
        val obj = button.text.toString()
        if (min != max) {
            binding.keyboardInput.text.delete(min, max)
        }
        binding.keyboardInput.text.insert(min, obj)
        binding.keyboardInput.setSelection(min + obj.length)
    }

    private fun setupButtonDelete(view: View, motionEvent: MotionEvent): Boolean {
        // Intrinsics.checkNotNullParameter(this, "this$0")
        this.views = view
        this.drawable = view.background

        val action = motionEvent.action
        if (action == MotionEvent.ACTION_DOWN) {
            binding.btn33.isPressed = true
            this.deleteTextAtCursor()
            repeatUpdateHandler.postDelayed(this.repeatUpdateTask, this.initialDelay)
            view.setBackgroundResource(R.drawable.samp_keyboard_active)
            return true
        } else if (action == MotionEvent.ACTION_UP || action == MotionEvent.ACTION_CANCEL) {
            binding.btn33.isPressed = false
            repeatUpdateHandler.removeCallbacks(this.repeatUpdateTask)
            view.background = null
            return true
        }
        return false
    }

    private var views: View? = null
    private var drawable: Drawable? = null

    init {
        activity.addContentView(binding.root, ConstraintLayout.LayoutParams(-1, -1))

        Utils.HideLayout(binding.root, true)

        val commandAdapter2 = CommandAdapter()
        this.commandAdapter = commandAdapter2
        binding.commandRc.adapter = commandAdapter2
        commandAdapter2.setOnCommandClick { commandInfo: CommandInfo ->
            Intrinsics.checkNotNullParameter(commandInfo, "commandInfo")
            binding.followingText.text = commandInfo.props
            val editText = binding.keyboardInput
            Intrinsics.checkNotNullExpressionValue(editText, "input")
            editText.setText(commandInfo.name)
            editText.setSelection(commandInfo.name.length)
            Unit
        }
        binding.keyboardInput.addTextChangedListener(object : TextWatcher {
            override fun beforeTextChanged(s: CharSequence, start: Int, count: Int, after: Int) {}

            override fun onTextChanged(s: CharSequence, start: Int, before: Int, count: Int) {}

            override fun afterTextChanged(editable: Editable) {
                searchCommands(editable.toString())?.let { commandAdapter.submitList(it) }
                if (isLockedUpperCase && !isCaps) {
                    isUpperCase = false
                    updateKeyTexts()
                    binding.btn23.setImageResource(R.drawable.samp_keyboard_shift_inactive)
                }
                if(customEditText == null) {
                    binding.keyboardInput.post {
                        updateTextAndPosition(editable.toString())
                    }
                }
            }
        })

        this.currentHistoryIndex = -1
        this.initialDelay = 500
        this.repeatInterval = 50
        this.repeatUpdateHandler = Handler(Looper.getMainLooper())
        this.repeatUpdateTask = object : Runnable {
            override fun run() {
                if (binding.btn33.isPressed) {
                    deleteTextAtCursor()
                    repeatUpdateHandler.postDelayed(this, repeatInterval)
                }
            }
        }
        switchLanguage(this.currentLayout)
        setupLanguageToggleButton()
        setupButtonListeners()
        toggleCapsLock()
        binding.keyboardInput.showSoftInputOnFocus = false
        binding.exitZone.setOnClickListener { v: View? ->
            setUIVisible(false)
        }
    }

    fun destroy() {
        repeatUpdateHandler.removeCallbacksAndMessages(null)
    }

    private fun setupButtonShift(view: View) {
        this.toggleCapsLock()
        this.views = view
        this.drawable = view.background
        view.setBackgroundResource(R.drawable.samp_keyboard_active)
        Handler(Looper.getMainLooper()).postDelayed({
            views?.background = drawable
        }, 25)
    }

    private fun toggleCapsLock() {
        this.isCaps = !this.isCaps
        updateKeyTexts()
    }

    private fun updateKeyTexts() {
        val buttons = listOf(
            binding.btn1, binding.btn2, binding.btn3, binding.btn4,
            binding.btn5, binding.btn6, binding.btn7, binding.btn8,
            binding.btn9, binding.btn10, binding.btn11, binding.btn12,
            binding.btn13, binding.btn14, binding.btn15, binding.btn16,
            binding.btn17, binding.btn18, binding.btn19, binding.btn20,
            binding.btn21, binding.btn22, binding.btn24, binding.btn25,
            binding.btn26, binding.btn27, binding.btn28, binding.btn29,
            binding.btn30, binding.btn31, binding.btn32
        )

        buttons.forEach { button ->
            button.isAllCaps = isCaps
            button.text = if (isCaps) button.text.toString().uppercase() else button.text.toString().lowercase()
        }

        binding.btn23.setImageResource(
            if (isCaps) R.drawable.samp_keyboard_shift_active
            else R.drawable.samp_keyboard_shift_inactive
        )
    }

    fun deleteTextAtCursor() {
        val selectionStart = binding.keyboardInput.selectionStart
        val selectionEnd = binding.keyboardInput.selectionEnd
        if (selectionStart != selectionEnd) {
            binding.keyboardInput.text.delete(
                min(
                    selectionStart.toDouble(),
                    selectionEnd.toDouble()
                ).toInt(), max(selectionStart.toDouble(), selectionEnd.toDouble()).toInt()
            )
        } else if (selectionStart > 0) {
            binding.keyboardInput.text.delete(selectionStart - 1, selectionStart)
        }
    }

    private fun setupLanguageToggleButton() {
        binding.btn36.setOnClickListener {
            currentLayout = when (currentLayout) {
                "en" -> {
                    applyNumbersLayout()
                    "num"
                }
                "ru" -> {
                    applyEnglishLayout()
                    "en"
                }
                "num" -> {
                    applyRussianLayout()
                    "ru"
                }
                else -> {
                    applyRussianLayout()
                    "ru"
                }
            }
        }
    }

    private fun clearConstraints(constraintSet: ConstraintSet, vararg viewArr: View) {
        for (view in viewArr) {
            constraintSet.clear(view.id, 6)
            constraintSet.clear(view.id, 7)
            constraintSet.clear(view.id, 3)
            constraintSet.clear(view.id, 4)
            constraintSet.clear(view.id, 1)
            constraintSet.clear(view.id, 2)
            constraintSet.clear(view.id, 5)
        }
    }

    private fun switchLanguage(str: String) {
        val hashCode = str.hashCode()
        if (hashCode != 3241) {
            if (hashCode != 3651) {
                if (hashCode == 109446 && str == "num") {
                    applyNumbersLayout()
                }
            } else if (str == "ru") {
                applyRussianLayout()
            }
        } else if (str == "en") {
            applyEnglishLayout()
        }
    }

    fun show() {
        binding.root.visibility = View.VISIBLE
    }

    fun request() {
        binding.root.requestLayout()
    }

    fun hide() {
        binding.root.visibility = View.GONE
    }

    fun setUIVisible(toggle: Boolean) {
        customEditText?.isKeyBoardShowS = toggle

        println("\n[toggle] SAMP KEYBOARD: $toggle")

        if (toggle) {
            Utils.ShowLayout(binding.root, true)
            Utils.ShowLayout(binding.sampKeyboardLayout, true)
            return
        }
        Utils.HideLayout(binding.root, true)
        Utils.HideLayout(binding.sampKeyboardLayout, true)

        /* activity.runOnUiThread {
             println("\n[toggle] SAMP KEYBOARD: $toggle")

             if (toggle) {
                 EasyAnimation.slideWithFade(
                     binding.root,
                     EasyAnimation.SlideDirection.SCREEN_TO_DEFAULT,
                     0L,
                     { show() },
                     { request() }
                 )
                 return@runOnUiThread
             }
             EasyAnimation.slideWithFade(
                 binding.root,
                 EasyAnimation.SlideDirection.OUT_SCREEN_DOWN,
                 0L,
                 null,
                 { hide() }
              )
         }*/
    }

    fun updateTextAndPosition(str: String) {
        val editText = binding.keyboardInput
        val textView = binding.followingText

        val regex = Regex("\\s+")
        val list = regex.split(str.trim().toString())
            .filter { it.isNotEmpty() }

        if (list.isNotEmpty()) {
            val firstWord = list.first()
            val remainingWords = if (list.size > 1) list.drop(1) else emptyList()

            val commandInfo = StaticServerCommands.INSTANCE.staticServerCommands.find { it.name == firstWord }

            if (commandInfo != null) {
                var extractParams = extractParams(commandInfo.props)

                if (remainingWords.isNotEmpty()) {
                    extractParams = extractParams.drop(remainingWords.size)
                }

                if (extractParams.isNotEmpty()) {
                    val ok = extractParams.joinToString(
                        separator = " ",
                        prefix = "",
                        postfix = "",
                        transform = { "[${it}]" }
                    )
                    textView.text = ok
                    textView.visibility = View.VISIBLE
                } else {
                    textView.visibility = View.GONE
                }
            } else {
                textView.visibility = View.GONE
            }
        } else {
            textView.visibility = View.GONE
        }

        updateTextViewPosition(editText.selectionStart)
    }

    private fun extractParams(str: String): List<String> {
        val regex = Regex("\\[([^\\]]+)\\]")
        return regex.findAll(str)
            .map { it.groupValues[1] }
            .toList()
    }

    @SuppressLint("UseKtx")
    private fun updateTextViewPosition(cursorPosition: Int) {
        val editText = binding.keyboardInput
        val textView = binding.followingText

        if (cursorPosition > 0 && textView.isVisible) {
            val layout = editText.layout
            val lineForOffset = layout.getLineForOffset(cursorPosition)
            val lineBaseline = layout.getLineBaseline(lineForOffset)
            val lineAscent = layout.getLineAscent(lineForOffset)
            var primaryHorizontal = layout.getPrimaryHorizontal(cursorPosition)
            val verticalOffset = (editText.paddingTop + lineBaseline + lineAscent).toFloat()
            val text = editText.text

            if (text?.isNotEmpty() == true && text.last() != ' ') {
                primaryHorizontal += layout.paint.measureText(" ")
            }

            textView.translationX = editText.paddingLeft + primaryHorizontal
            textView.translationY = verticalOffset
        }
    }

    fun cleanup() {
        // binding.keyboardInput.removeTextChangedListener()
        repeatUpdateHandler.removeCallbacksAndMessages(null)
    }
}