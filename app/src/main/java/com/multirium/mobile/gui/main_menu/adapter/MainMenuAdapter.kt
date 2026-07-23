package com.multirium.mobile.gui.main_menu.adapter


import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.multirium.mobile.databinding.MainMenuItemBinding
import com.multirium.mobile.gui.main_menu.obj.MapIcon

class MainMenuAdapter(var list: List<MapIcon>) : RecyclerView.Adapter<MainMenuAdapter.ViewHolder>() {
    lateinit var onItemClick: Function1<MapIcon, Unit>
    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val inflater = LayoutInflater.from(parent.context)
        val binding = MainMenuItemBinding.inflate(inflater, parent, false)
        return ViewHolder(binding)
    }

    override fun getItemCount(): Int {
        return list.size
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        with(holder.binding) {
            menuItemTitle.text = list[position].title
            mainMenuBtn.setImageResource(list[position].imageResource)
            mainMenuBtn.setOnClickListener {
                onItemClick.invoke(list[position])
            }
        }
    }
    class ViewHolder (val binding: MainMenuItemBinding) : RecyclerView.ViewHolder(binding.root)
}