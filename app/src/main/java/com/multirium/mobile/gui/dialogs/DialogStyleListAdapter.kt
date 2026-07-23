package com.multirium.mobile.gui.dialogs

import android.text.Spanned
import android.view.LayoutInflater
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.multirium.cr.gui.dialogs.SelectedItem
import com.multirium.mobile.R
import com.multirium.mobile.databinding.DialogStyleListItemBinding

class DialogStyleListAdapter : RecyclerView.Adapter<DialogStyleListAdapter.ViewHolder>() {

    private var list: MutableList<Spanned?> = mutableListOf()
    lateinit var onDoubleClick: Function1<SelectedItem?, Unit>

    var selectedPos: Int = 0

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val inflater = LayoutInflater.from(parent.context)
        val binding = DialogStyleListItemBinding.inflate(inflater, parent, false)
        return ViewHolder(this,binding)
    }

    override fun getItemCount(): Int {
        return list.size
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {

        val spanned = list[position] as Spanned
        val binding: DialogStyleListItemBinding = holder.binding
        if (position == this.selectedPos) {
            binding.text.setBackgroundResource(R.drawable.dialog_list_item_active_bg)
        } else {
            binding.text.setBackgroundResource(R.drawable.dialog_list_item_passive_bg);
        }
        binding.text.text = spanned
    }
    fun getSelectedListItem(): SelectedItem {
        return SelectedItem(list[selectedPos].toString(), selectedPos)
    }

    fun configureNew(list2: MutableList<Spanned?>) {
        this.list = list2
        notifyDataSetChanged()
    }

    class ViewHolder (val dialogStyleListAdapter: DialogStyleListAdapter, val binding: DialogStyleListItemBinding) : RecyclerView.ViewHolder(binding.root){
        init {
            binding.root.setOnClickListener {
                if (dialogStyleListAdapter.selectedPos == getBindingAdapterPosition()) {
                    if (dialogStyleListAdapter.onDoubleClick != null) {
                        dialogStyleListAdapter.onDoubleClick.invoke(dialogStyleListAdapter.getSelectedListItem())
                        return@setOnClickListener
                    }
                    return@setOnClickListener
                }
                val lastpos = dialogStyleListAdapter.selectedPos
                dialogStyleListAdapter.selectedPos = getBindingAdapterPosition()
                dialogStyleListAdapter.notifyItemChanged(dialogStyleListAdapter.selectedPos)
                if (dialogStyleListAdapter.selectedPos != -1) {
                    dialogStyleListAdapter.notifyItemChanged(lastpos)
                }
            }
        }
    }
}