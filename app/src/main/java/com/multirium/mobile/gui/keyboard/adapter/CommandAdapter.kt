package com.multirium.mobile.gui.keyboard.adapter

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.multirium.mobile.R
import com.multirium.mobile.common.EasyAnimation
import com.multirium.mobile.databinding.KeyboardCommandItemBinding
import com.multirium.mobile.gui.keyboard.CommandInfo
import kotlin.jvm.internal.Intrinsics

class CommandAdapter : RecyclerView.Adapter<CommandAdapter.CommandViewHolder>() {
    private val commands: MutableList<CommandInfo> = ArrayList()
    var onCommandClick: Function1<CommandInfo, Unit>? = null
        private set

    fun setOnCommandClick(function1: (CommandInfo) -> Unit) {
        this.onCommandClick = function1
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): CommandViewHolder {
        val view = LayoutInflater.from(parent.context)
            .inflate(R.layout.keyboard_command_item, parent, false)
        return CommandViewHolder(view)
    }

    override fun onBindViewHolder(holder: CommandViewHolder, position: Int) {
        val commandInfo = commands[position]
        val binding = holder.binding

        if (commands.size == 1) {
            binding.command.text = commandInfo.name + " - " + commandInfo.description
        } else {
            binding.command.text = commandInfo.name
        }

        val commandItem = binding.commandItem
        EasyAnimation.`animateClick$default`(
            EasyAnimation.INSTANCE, commandItem, 0, null,
            CommandClickListener(this, commandInfo), 3, null
        )
    }

    override fun getItemCount(): Int {
        return commands.size
    }

    fun submitList(list: List<CommandInfo>) {
        Intrinsics.checkNotNullParameter(list, "list")
        commands.clear()
        commands.addAll(list)
        notifyDataSetChanged()
    }

    class CommandViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        internal val binding: KeyboardCommandItemBinding =
            KeyboardCommandItemBinding.bind(itemView)
    }

    private class CommandClickListener(
        private val adapter: CommandAdapter,
        private val commandInfo: CommandInfo
    ) :
        Function0<Unit> {
        override fun invoke() {
            adapter.commands.clear()
            adapter.commands.add(commandInfo)
            adapter.notifyDataSetChanged()

            val clickListener = adapter.onCommandClick
            clickListener?.invoke(commandInfo)
            return Unit
        }
    }
}