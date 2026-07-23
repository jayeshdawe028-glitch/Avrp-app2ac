package com.multirium.mobile.gui.dialogs

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.multirium.cr.gui.dialogs.SelectedItem
import com.multirium.mobile.R
import com.multirium.mobile.databinding.DialogStyleTablistItemBinding
import kotlin.jvm.internal.Intrinsics

class DialogStyleTabListAdapter : RecyclerView.Adapter<DialogStyleTabListAdapter.ViewHolder>(){
    private var data = DialogTabListHelper("")
    lateinit var onDoubleClick: Function1<SelectedItem?, Unit>

    var selectedPos: Int = 0

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val inflater = LayoutInflater.from(parent.context)
        val binding = DialogStyleTablistItemBinding.inflate(inflater, parent, false)
        return ViewHolder(this, binding)
    }

    override fun getItemCount(): Int {
        return data.rows.size
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val tabListRow = data.rows[position]
        val binding: DialogStyleTablistItemBinding = holder.binding
        if (position == this.selectedPos) {
            binding.container.setBackgroundResource(R.drawable.dialog_list_item_active_bg)
        } else {
            binding.container.setBackgroundResource(R.drawable.dialog_list_item_passive_bg)
        }
        var i2 = 0
        for (next in mutableListOf(
            binding.column1Text,
            binding.column2Text,
            binding.column3Text,
            binding.column4Text
        )) {
            val i3 = i2 + 1

            val textView = next as TextView
            if (i2 < tabListRow.columns.size) {
                textView.width = data.maxColumnWidths[i2].toInt()
                textView.text = tabListRow.columns[i2]
                textView.visibility = View.VISIBLE
            } else {
                textView.visibility = View.GONE
            }
            i2 = i3
        }
    }

    fun getSelectedListItem(): SelectedItem {
        val selectedItem = SelectedItem("", 0)
        selectedItem.listitem = selectedPos
        selectedItem.inputText = data.rows[selectedPos].columns[0].toString()
        return selectedItem
    }

    fun configureNew(dialogTabListHelper: DialogTabListHelper?) {
        Intrinsics.checkNotNullParameter(dialogTabListHelper, "newData")
        this.data = dialogTabListHelper!!
        notifyDataSetChanged()
    }

    class ViewHolder (val dialogStyleTabListAdapter: DialogStyleTabListAdapter, val binding: DialogStyleTablistItemBinding) : RecyclerView.ViewHolder(binding.root){
        init {
            binding.container.setOnClickListener {
                if (dialogStyleTabListAdapter.selectedPos == getBindingAdapterPosition()) {

                    if (dialogStyleTabListAdapter.onDoubleClick != null) {
                        dialogStyleTabListAdapter.onDoubleClick.invoke(dialogStyleTabListAdapter.getSelectedListItem())
                        return@setOnClickListener
                    }
                    return@setOnClickListener
                }
                val `access$getSelectedPos$p` = dialogStyleTabListAdapter.selectedPos
                dialogStyleTabListAdapter.selectedPos = getBindingAdapterPosition()
                dialogStyleTabListAdapter.notifyItemChanged(dialogStyleTabListAdapter.selectedPos)
                if (`access$getSelectedPos$p` != -1) {
                    dialogStyleTabListAdapter.notifyItemChanged(`access$getSelectedPos$p`)
                }
            }
        }
    }
}