package com.multirium.mobile.gui.main_menu.adapter

import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.multirium.mobile.R
import com.multirium.mobile.databinding.SettingsCategoryItemBinding
import com.multirium.mobile.gui.main_menu.obj.SettingsCategory
import com.multirium.mobile.common.EasyAnimation.INSTANCE
import kotlin.jvm.internal.Intrinsics

class SettingsCategoryAdapter : RecyclerView.Adapter<SettingsCategoryAdapter.ViewHolder>() {
    var list: MutableList<SettingsCategory> = mutableListOf()
    class ViewHolder (val binding: SettingsCategoryItemBinding) : RecyclerView.ViewHolder(binding.root)

    lateinit var onClick: Function2<String, Int, Unit>
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val inflater = LayoutInflater.from(parent.context)
        val binding = SettingsCategoryItemBinding.inflate(inflater, parent, false)
        return ViewHolder(binding)
    }

    override fun getItemCount(): Int {
        return list.size
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        with(holder.binding){
            settingsCategoryText.text = list[position].name
            if (list[position].isActive) {
                settingsCategoryItem.setBackgroundResource(R.drawable.settings_menu_category_active_bg)
            } else {
                settingsCategoryItem.setBackgroundResource(R.drawable.settings_menu_category_inactive_bg)
            }
            INSTANCE.setOnClickListenerWithAnim(settingsCategoryItem){
                list[0].isActive = true
                val arrayList: MutableList<SettingsCategory> = mutableListOf()
                for (next in list) {
                    if (!Intrinsics.areEqual(next, list)) {
                        arrayList.add(next)
                    }
                }
                for (active in arrayList) {
                    active.isActive = false
                }

                if (onClick != null) {
                    onClick.invoke(list[position].name, position)
                }
                notifyDataSetChanged()
            }
        }
    }
    fun addAll(list: List<SettingsCategory>) {
        this.list.clear()
        this.list.addAll(list)
        notifyDataSetChanged()
    }
}