package com.multirium.mobile.gui.main_menu.adapter

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.SeekBar
import android.widget.SeekBar.OnSeekBarChangeListener
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.multirium.mobile.R
import com.multirium.mobile.databinding.SettingsMenuItemBinding
import com.multirium.mobile.gui.main_menu.obj.SettingParamId
import com.multirium.mobile.gui.main_menu.obj.SettingsType
import com.multirium.mobile.gui.main_menu.obj.SettingDeep
import com.multirium.mobile.gui.main_menu.obj.SettingParam
import com.multirium.mobile.common.EasyAnimation.INSTANCE
import kotlin.jvm.internal.Intrinsics

class SettingsParamsAdapter(var width: Int, var height: Int): RecyclerView.Adapter<SettingsParamsAdapter.ViewHolder>()  {
    var list: MutableList<SettingParam> = mutableListOf()
    class ViewHolder (val binding: SettingsMenuItemBinding) : RecyclerView.ViewHolder(binding.root)


    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val inflater = LayoutInflater.from(parent.context)
        val binding = SettingsMenuItemBinding.inflate(inflater, parent, false)
        return ViewHolder(binding)
    }

    override fun getItemCount(): Int {
        return list.size
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        with(holder.binding) {
            val settingParam = list[position]
            settingsMenuSeekBar.setOnSeekBarChangeListener(null as OnSeekBarChangeListener?)
            settingsMenuTitle.visibility = View.VISIBLE
            settingsMenuTitle.text = settingParam.title
            settingsMenuItem.post {
                var minValue: Int
                var maxValue: Int
                val layoutParams: ViewGroup.LayoutParams = settingsMenuItem.layoutParams
                val layoutParams2 = layoutParams as RecyclerView.LayoutParams
                layoutParams2.setMargins(0, 0, (((width / 100).toDouble()) * 1.51).toInt(), (((height / 100).toDouble()) * 2.857).toInt())
                layoutParams2.width = -1
                when (settingParam.type.ordinal) {
                    SettingsType.BUTTON.ordinal -> {
                        settingsMenuItem.setBackgroundResource(R.drawable.settings_menu_small_param_bg)
                        layoutParams2.height = (((height / 100).toDouble()) * 9.56).toInt()


                        for (visibility in listOf(settingsMenuCheckboxFirst, settingsMenuLabelFirst, settingsMenuCheckboxSecond, settingsMenuLabelSecond, settingsMenuCheckboxThird, settingsMenuLabelThird, settingsMenuCheckboxFour, settingsMenuLabelFour, settingsMenuSeekBar, settingsMenuProgress)) {
                            visibility.visibility = View.GONE
                        }
                        settingsMenuArrow.visibility = View.VISIBLE
                        INSTANCE.setOnClickListenerWithAnim(settingsMenuItem){
                            val settingDeep: SettingDeep = settingParam.deeps.firstOrNull() as SettingDeep

                            if (settingDeep != null && settingDeep.callback != null) {
                                settingDeep.callback!!.invoke()
                            }
                        }
                    }
                    SettingsType.CHECKBOX.ordinal -> {
                        settingsMenuItem.setBackgroundResource(R.drawable.settings_menu_medium_param_bg)
                        layoutParams2.height =
                            (((height / 100).toDouble()) * 18.55).toInt()
                        val imageView5: ImageView = settingsMenuArrow

                        val seekBar2: SeekBar = settingsMenuSeekBar

                        val textView6: TextView = settingsMenuProgress

                        for (visibility2 in listOf(imageView5, seekBar2, textView6)) {
                            visibility2.visibility = View.GONE
                        }
                        val listOf: List<Pair<ImageView, TextView>> = listOf(
                            Pair(
                                settingsMenuCheckboxFirst,
                                settingsMenuLabelFirst
                            ),
                            Pair(
                                settingsMenuCheckboxSecond,
                                settingsMenuLabelSecond
                            ),
                            Pair(
                                settingsMenuCheckboxThird,
                                settingsMenuLabelThird
                            ),
                            Pair(
                                settingsMenuCheckboxFour,
                                settingsMenuLabelFour
                            )
                        )
                        for (pair in listOf) {
                            (pair.first).visibility = View.GONE
                            (pair.second).visibility = View.GONE
                        }
                        var i3 = 0
                        for (next in settingParam.deeps) {
                            val i4 = i3 + 1

                            val settingDeep = next
                            val first: Any = (listOf[i3]).first
                            val imageView6 = first as ImageView
                            val second: Any = (listOf[i3]).second
                            val textView7 = second as TextView
                            imageView6.visibility = View.VISIBLE
                            textView7.visibility = View.VISIBLE
                            textView7.text = settingDeep.name
                            if (settingDeep.isActive) {
                                imageView6.setImageResource(R.drawable.settings_menu_checkbox_active)
                            } else {
                                imageView6.setImageResource(R.drawable.settings_menu_checkbox_inactive)
                            }
                            INSTANCE.setOnClickListenerWithAnim(imageView6){
                                settingDeep.isActive = true

                                val arrayList: MutableList<SettingDeep> = mutableListOf()
                                for (next in settingParam.deeps) {
                                    if (!Intrinsics.areEqual(
                                            next,
                                            settingDeep
                                        )
                                    ) {
                                        arrayList.add(next)
                                    }
                                }
                                for (active in arrayList) {
                                    active.isActive = false
                                }

                                if (settingDeep.callback != null) {
                                    settingDeep.callback!!.invoke()
                                }
                                notifyItemChanged(position)
                            }
                            i3 = i4
                        }
                    }
                    SettingsType.SEEKBAR.ordinal -> {
                        settingsMenuItem.setBackgroundResource(R.drawable.settings_menu_medium_param_bg)
                        layoutParams2.height = (((height / 100).toDouble()) * 14.35).toInt()
                        val settingDeep2 = settingParam.deeps.firstOrNull()
                        if (settingDeep2 != null) {
                            if (settingDeep2.isNoPercent) {
                                settingsMenuProgress.text = settingDeep2.value.toString()
                            } else {
                                val textView8: TextView = settingsMenuProgress
                                val sb = StringBuilder()

                                sb.append(settingDeep2.value)
                                sb.append('%')
                                textView8.text = sb.toString()
                            }
                        }
                        if (settingDeep2 != null) {
                            settingsMenuSeekBar.progress = settingDeep2.value
                        }
                        val imageView7: ImageView = settingsMenuCheckboxFirst

                        val textView9: TextView = settingsMenuLabelFirst

                        val imageView8: ImageView = settingsMenuCheckboxSecond
                        val textView10: TextView = settingsMenuLabelSecond
                        val imageView9: ImageView = settingsMenuCheckboxThird
                        val textView11: TextView = settingsMenuLabelThird
                        val imageView10: ImageView = settingsMenuCheckboxFour
                        val textView12: TextView = settingsMenuLabelFour
                        val imageView11: ImageView = settingsMenuArrow

                        for (visibility3 in listOf(
                            imageView7,
                            textView9,
                            imageView8,
                            textView10,
                            imageView9,
                            textView11,
                            imageView10,
                            textView12,
                            imageView11
                        )) {
                            visibility3.visibility = View.GONE
                        }
                        val seekBar3: SeekBar = settingsMenuSeekBar
                        val textView13: TextView = settingsMenuProgress
                        for (visibility4 in listOf(seekBar3, textView13)) {
                            visibility4.visibility = View.VISIBLE
                        }
                        if (!(settingDeep2 == null)) {
                            settingsMenuSeekBar.setMax(settingDeep2.maxValue)
                        }
                        if (!(settingDeep2 == null)) {
                            settingsMenuSeekBar.setMin(settingDeep2.minValue)
                        }
                        settingsMenuSeekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
                            override fun onProgressChanged(seekBar: SeekBar, progress: Int, fromUser: Boolean) {


                                if (!(settingDeep2 == null )) {
                                    settingDeep2.value = progress
                                    settingDeep2.callback!!.invoke()
                                }

                                if (settingDeep2 != null) {
                                    val noPercent: Boolean = settingDeep2.isNoPercent

                                    if (noPercent) {
                                        settingsMenuProgress.text = progress.toString()
                                        return
                                    }
                                    val textView = settingsMenuProgress
                                    val sb = java.lang.StringBuilder()
                                    sb.append(progress)
                                    sb.append('%')
                                    textView.text = sb.toString()
                                }

                            }

                            override fun onStartTrackingTouch(seekBar: SeekBar?) {

                            }

                            override fun onStopTrackingTouch(seekBar: SeekBar?) {

                            }
                        })
                    }
                }
            }
        }
    }
    fun addAll(list2: List<SettingParam>) {
        list.clear()
        list.addAll(list2)
        notifyDataSetChanged()
    }
}