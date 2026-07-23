package com.multirium.mobile.gui.pausemenu

import android.graphics.Matrix
import android.view.ScaleGestureDetector
import android.view.ScaleGestureDetector.SimpleOnScaleGestureListener
import android.view.View
import android.view.View.OnTouchListener
import android.widget.ImageView
import android.widget.LinearLayout
import androidx.constraintlayout.widget.ConstraintLayout
import com.multirium.mobile.R
import com.multirium.mobile.core.Samp
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.ControlSettingsLayoutBinding
import com.multirium.mobile.gui.NativeGui
import com.multirium.mobile.gui.main_menu.obj.ButtonInfo
import com.multirium.mobile.gui.main_menu.obj.OnFootControlButton
import com.multirium.mobile.gui.main_menu.obj.OnVehicleControlButton
import com.multirium.mobile.gui.main_menu.utils.SettingGameButtonUtil
import com.multirium.mobile.gui.main_menu.utils.SettingGameButtonUtil.updateGameButton
import com.multirium.mobile.common.EasyAnimation
import com.multirium.mobile.common.EasyAnimation.INSTANCE
import com.multirium.mobile.gui.util.GtaCordsData
import com.multirium.mobile.gui.util.GtaCordsDataKt.INSTANCE
import com.multirium.mobile.gui.util.GtaCordsDataKt.convertCordsToGta
import com.multirium.mobile.launcher.util.JsonUtilKt
import kotlin.jvm.internal.Reflection

class SettingsControl : NativeGui<ControlSettingsLayoutBinding>(
    Reflection.getOrCreateKotlinClass(
        ControlSettingsLayoutBinding::class.java), false, false, true, false) {
    val bind = getBinding() as ControlSettingsLayoutBinding
    var scaleGestureDetector: ScaleGestureDetector = ScaleGestureDetector(activity, ScaleListener())
    var createdButtons: MutableList<View> = mutableListOf()
    lateinit var exitCallback: Function0<Unit>
    init {
        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(bind.exitBtn){
            if(exitCallback != null)
                exitCallback.invoke()
        }
    }
    fun showOnFootControl(){
        val onFootControlButton = JsonUtilKt.toModel(SettingGameButtonUtil.INSTANCE.loadJsonFromFile(SettingGameButtonUtil.ON_FOOT_CONTROL_BUTTON), OnFootControlButton::class.java) as OnFootControlButton
        createButton(onFootControlButton.enterAim, R.drawable.control_settings_aim_btn)
        createButton(onFootControlButton.enterCar, R.drawable.control_settings_enter_car_btn)
        createButton(onFootControlButton.fist, R.drawable.control_settings_fist_btn)
        createButton(onFootControlButton.sprint, R.drawable.control_settings_sprint_btn)
        createButton(onFootControlButton.voice, R.drawable.control_settings_voice_btn)
        createButton(onFootControlButton.fireGun, R.drawable.control_settings_fire_btn)
        if (onFootControlButton.fireGun2 != null)
            createButton(onFootControlButton.fireGun2, R.drawable.control_settings_fire_btn)

        EasyAnimation.INSTANCE.setOnClickListenerWithAnim(bind.saveBtn){
            updateGameButton(
                onFootControlButton.enterAim.id,
                onFootControlButton.enterAim.x,
                onFootControlButton.enterAim.y,
                onFootControlButton.enterAim.width,
                onFootControlButton.enterAim.height
            )
            updateGameButton(
                onFootControlButton.exitAim.id,
                onFootControlButton.enterAim.x,
                onFootControlButton.enterAim.y,
                onFootControlButton.enterAim.width,
                onFootControlButton.enterAim.height
            )
            updateGameButton(
                onFootControlButton.sprint.id,
                onFootControlButton.sprint.x,
                onFootControlButton.sprint.y,
                onFootControlButton.sprint.width,
                onFootControlButton.sprint.height
            )
            updateGameButton(
                onFootControlButton.voice.id,
                onFootControlButton.voice.x,
                onFootControlButton.voice.y,
                onFootControlButton.voice.width,
                onFootControlButton.voice.height
            )
            updateGameButton(
                onFootControlButton.enterCar.id,
                onFootControlButton.enterCar.x,
                onFootControlButton.enterCar.y,
                onFootControlButton.enterCar.width,
                onFootControlButton.enterCar.height
            )
            updateGameButton(
                onFootControlButton.fist.id,
                onFootControlButton.fist.x,
                onFootControlButton.fist.y,
                onFootControlButton.fist.width,
                onFootControlButton.fist.height
            )
            updateGameButton(
                onFootControlButton.fireGun.id,
                onFootControlButton.fireGun.x,
                onFootControlButton.fireGun.y,
                onFootControlButton.fireGun.width,
                onFootControlButton.fireGun.height
            )
            updateGameButton(
                onFootControlButton.fireGun2.id,
                onFootControlButton.fireGun2.x,
                onFootControlButton.fireGun2.y,
                onFootControlButton.fireGun2.width,
                onFootControlButton.fireGun2.height
            )
            SettingGameButtonUtil.INSTANCE.saveJsonToFile(
                JsonUtilKt.toStringJson(
                    onFootControlButton
                ), SettingGameButtonUtil.ON_FOOT_CONTROL_BUTTON
            )
            removeAllButtons()
            if(exitCallback != null) exitCallback.invoke()
        }
    }
    fun showOnVehicleControl() {
        activity.runOnUiThread {
            val onVehicleControlButton =
                JsonUtilKt.toModel(
                    SettingGameButtonUtil.INSTANCE.loadJsonFromFile(SettingGameButtonUtil.ON_VEHICLE_CONTROL_BUTTON),
                    OnVehicleControlButton::class.java
                ) as OnVehicleControlButton
            createButton(onVehicleControlButton.brake, R.drawable.control_settings_brake_btn)
            createButton(
                onVehicleControlButton.acceleration,
                R.drawable.control_settings_acceleration_btn
            )
            createButton(
                onVehicleControlButton.handBrake,
                R.drawable.control_settings_hand_brake_btn
            )
            createButton(
                onVehicleControlButton.enterCar,
                R.drawable.control_settings_enter_car_btn
            )
            createButton(onVehicleControlButton.horn, R.drawable.control_settings_horn_btn)
            createButton(
                onVehicleControlButton.leftArrow,
                R.drawable.control_settings_left_arrow_btn
            )
            createButton(
                onVehicleControlButton.rightArrow,
                R.drawable.control_settings_right_arrow_btn
            )
            createButton(onVehicleControlButton.nitro, R.drawable.control_settings_nitro_btn)
            EasyAnimation.INSTANCE.setOnClickListenerWithAnim(bind.saveBtn) {
                updateGameButton(
                    onVehicleControlButton.brake.id,
                    onVehicleControlButton.brake.x,
                    onVehicleControlButton.brake.y,
                    onVehicleControlButton.brake.width,
                    onVehicleControlButton.brake.height
                )
                updateGameButton(
                    onVehicleControlButton.acceleration.id,
                    onVehicleControlButton.acceleration.x,
                    onVehicleControlButton.acceleration.y,
                    onVehicleControlButton.acceleration.width,
                    onVehicleControlButton.acceleration.height
                )
                updateGameButton(
                    onVehicleControlButton.enterCar.id,
                    onVehicleControlButton.enterCar.x,
                    onVehicleControlButton.enterCar.y,
                    onVehicleControlButton.enterCar.width,
                    onVehicleControlButton.enterCar.height
                )
                updateGameButton(
                    onVehicleControlButton.horn.id,
                    onVehicleControlButton.horn.x,
                    onVehicleControlButton.horn.y,
                    onVehicleControlButton.horn.width,
                    onVehicleControlButton.horn.height
                )
                updateGameButton(
                    onVehicleControlButton.handBrake.id,
                    onVehicleControlButton.handBrake.x,
                    onVehicleControlButton.handBrake.y,
                    onVehicleControlButton.handBrake.width,
                    onVehicleControlButton.handBrake.height
                )
                updateGameButton(
                    onVehicleControlButton.leftArrow.id,
                    onVehicleControlButton.leftArrow.x,
                    onVehicleControlButton.leftArrow.y,
                    onVehicleControlButton.leftArrow.width,
                    onVehicleControlButton.leftArrow.height
                )
                updateGameButton(
                    onVehicleControlButton.rightArrow.id,
                    onVehicleControlButton.rightArrow.x,
                    onVehicleControlButton.rightArrow.y,
                    onVehicleControlButton.rightArrow.width,
                    onVehicleControlButton.rightArrow.height
                )
                SettingGameButtonUtil.INSTANCE.saveJsonToFile(
                    JsonUtilKt.toStringJson(onVehicleControlButton),
                    SettingGameButtonUtil.ON_VEHICLE_CONTROL_BUTTON
                )
                removeAllButtons()
                if(exitCallback != null) exitCallback.invoke()

            }
        }
    }
    fun createButton(buttonInfo: ButtonInfo, i: Int) {
        activity.runOnUiThread {
            bind.root.post(Runnable {
                val convertAbsoluteToPercentage =
                    convertAbsoluteToPercentage(buttonInfo.x, buttonInfo.y)
                val convertPercentageToScreen =
                    convertPercentageToScreen(
                        convertAbsoluteToPercentage.component1(),
                        convertAbsoluteToPercentage.component2(),
                        bind.root.width,
                        bind.root.height
                    )
                val floatValue = convertPercentageToScreen.component1()
                val floatValue2 = convertPercentageToScreen.component2()
                val linearLayout = LinearLayout(Samp.activity)
                val layoutParams =
                    ConstraintLayout.LayoutParams(0, 0)
                layoutParams.width =
                    convertToSize(buttonInfo.width, bind.root.width).toInt()
                layoutParams.height =
                    convertToSize(buttonInfo.width, bind.root.width).toInt()
                linearLayout.layoutParams = layoutParams
                linearLayout.setBackgroundResource(R.drawable.control_settings_contour_inactive)
                linearLayout.orientation = LinearLayout.VERTICAL
                val imageView = ImageView(activity)
                imageView.layoutParams = LinearLayout.LayoutParams(-1, -2)
                imageView.adjustViewBounds = true
                imageView.setImageResource(i)
                linearLayout.addView(imageView)
                bind.root.addView(linearLayout)
                linearLayout.post {
                    linearLayout.x =
                        floatValue - ((linearLayout.width / 2).toFloat())
                    linearLayout.y =
                        floatValue2 - ((linearLayout.height / 3.8).toFloat())
                    linearLayout.layoutParams.width =
                        convertToSize(
                            buttonInfo.width,
                            bind.root.width
                        ).toInt()
                    linearLayout.layoutParams.height =
                        convertToSize(
                            buttonInfo.width,
                            bind.root.width
                        ).toInt()
                }
                setDragListener(linearLayout, buttonInfo)
                createdButtons.add(linearLayout)
            })
        }
    }
    private fun convertToSize(f: Float, i: Int): Float {
        return (i.toFloat()) * (((f / 0.96f) * (2f)) / (640f))
    }

    private fun setDragListener(linearLayout: LinearLayout, buttonInfo: ButtonInfo) {
        linearLayout.setOnTouchListener(OnTouchListener { view, motionEvent ->
            if (scaleGestureDetector != null) {
                scaleGestureDetector.onTouchEvent(motionEvent)
            }
            linearLayout.layoutParams.width =
                convertToSize(buttonInfo.width, bind.root.getWidth()).toInt()
            linearLayout.layoutParams.height =
                convertToSize(buttonInfo.width, bind.root.getWidth()).toInt()
            linearLayout.requestLayout()
            val action = motionEvent.action
            if (action == 0) {
                view.setBackgroundResource(R.drawable.control_settings_contour_active)
                true
            } else if (action == 1) {
                view.setBackgroundResource(R.drawable.control_settings_contour_inactive)
                true
            } else if (action != 2) {
                false
            } else {
                if (SettingsControl.lastCurrent >= System.currentTimeMillis()) {
                    return@OnTouchListener true
                }
                view.x = motionEvent.rawX - ((view.width / 2).toFloat())
                view.y = motionEvent.rawY - ((view.height / 2).toFloat())

                /*ConvertViewCoordsToGta.Data convertCoordsToGta = ConvertViewCoordsToGta.INSTANCE.convertCoordsToGta(new ConvertViewCoordsToGta.Data(view.getX(), view.getY(), (float) view.getWidth(), (float) view.getHeight()));
                    buttonInfo.setX(convertCoordsToGta.getX());
                    buttonInfo.setY(convertCoordsToGta.getY());*/
                val convertCordsToGta = convertCordsToGta(
                    GtaCordsData(
                        view.x,
                        view.y,
                        view.width.toFloat(),
                        view.height.toFloat(),
                        bind.root.getWidth(),
                        bind.root.getHeight()
                    )
                )
                buttonInfo.x = convertCordsToGta.x
                buttonInfo.y = convertCordsToGta.y

                SettingsControl.buttonSelect = buttonInfo
                SettingsControl.btnSelectLayout = linearLayout
                true
            }
        })
    }

    fun removeAllButtons() {
        for (removeView in this.createdButtons) {
            bind.root.removeView(removeView)
        }
        this.createdButtons.clear()
    }

    private fun convertAbsoluteToPercentage(f: Float, f2: Float): Pair<Float, Float> {
        return f / 640.0f to f2 / 480.0f
    }

    private fun convertPercentageToScreen(
        f: Float,
        f2: Float,
        i: Int,
        i2: Int
    ): Pair<Float, Float> {
        return f * (i.toFloat()) to f2 * (i2.toFloat())
    }
    companion object {

        @JvmStatic
        var LastScale: Float = 1.0f

        @JvmStatic
        var btnSelectLayout: LinearLayout? = null

        @JvmStatic
        var buttonSelect: ButtonInfo? = null

        @JvmStatic
        var lastCurrent: Long = 0

        @JvmStatic
        var matrix: Matrix = Matrix()

        @JvmStatic
        var scale: Float = 1.0f


    }

    override fun receivePacket(type: Int, data: String?) {
        TODO("Not yet implemented")
    }
    class ScaleListener : SimpleOnScaleGestureListener() {
        override fun onScale(scaleGestureDetector: ScaleGestureDetector): Boolean {
            val lastScale = LastScale

            scale *= scaleGestureDetector.scaleFactor
            scale = 0.1f.coerceAtLeast(scale.coerceAtMost(5.0f))
            matrix.setScale(scale, scale)
            if (buttonSelect == null) {
                return true
            }
            if (lastScale < scale) {
                val buttonSelects = buttonSelect

                val buttonSelect2 = buttonSelect

                if (buttonSelects != null) {
                    if (buttonSelect2 != null) {
                        buttonSelects.width = 12.0f.coerceAtLeast(buttonSelect2.width + 1.0f)
                    }
                }
                val buttonSelect3 = buttonSelect

                val buttonSelect4 = buttonSelect

                if (buttonSelect3 != null) {
                    if (buttonSelect4 != null) {
                        buttonSelect3.height = 12.0f.coerceAtLeast(buttonSelect4.height + 1.0f)
                    }
                }
            } else {
                val buttonSelect5 = buttonSelect

                val buttonSelect6 = buttonSelect

                if (buttonSelect5 != null) {
                    if (buttonSelect6 != null) {
                        buttonSelect5.width = 12.0f.coerceAtLeast(buttonSelect6.width - 1.0f)
                    }
                }
                val buttonSelect7 = buttonSelect

                val buttonSelect8 = buttonSelect

                if (buttonSelect7 != null) {
                    if (buttonSelect8 != null) {
                        buttonSelect7.height = 12.0f.coerceAtLeast(buttonSelect8.height - 1.0f)
                    }
                }
            }
            LastScale = scale
            lastCurrent = System.currentTimeMillis() + (1000L)
            return true
        }
    }
}