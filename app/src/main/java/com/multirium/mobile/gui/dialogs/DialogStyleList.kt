package com.multirium.mobile.gui.dialogs

import android.text.Spanned
import android.view.View
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.DialogStyleListBinding
import com.multirium.mobile.gui.NativeGui
import com.multirium.mobile.gui.util.Utils
import kotlin.jvm.internal.Reflection

class DialogStyleList(var caption: String, var content: String, var leftBtnText: String, var rightBtnText: String) : NativeGui<DialogStyleListBinding>(Reflection.getOrCreateKotlinClass(DialogStyleListBinding::class.java), false, false,true, true){
    val adapter: DialogStyleListAdapter = DialogStyleListAdapter()
    init {
        run {
            (getBinding() as DialogStyleListBinding).caption.text = Utils.transfromColors(this.caption)
            (getBinding() as DialogStyleListBinding).leftButtText.text = Utils.transfromColors(this.leftBtnText)
            (getBinding() as DialogStyleListBinding).leftButton.setOnClickListener {
                Dialog.INSTANCE.sendResponse(1, adapter.getSelectedListItem().listitem, adapter.getSelectedListItem().inputText)
            }
            if (this.rightBtnText.isEmpty()) {
                (getBinding() as DialogStyleListBinding).rightButton.visibility = View.GONE
            } else {
                (getBinding() as DialogStyleListBinding).rightButtText.text = Utils.transfromColors(this.rightBtnText)
                (getBinding() as DialogStyleListBinding).rightButton.setOnClickListener{
                    Dialog.INSTANCE.sendResponse(0, adapter.getSelectedListItem().listitem, adapter.getSelectedListItem().inputText)
                }

            }
            adapter.onDoubleClick =  {
                if (it != null) {
                    Dialog.INSTANCE.sendResponse(1, it.listitem, it.inputText)
                }
            }
            (getBinding() as DialogStyleListBinding).recycler.adapter = adapter
            (getBinding() as DialogStyleListBinding).recycler.layoutManager = LinearLayoutManager(activity, RecyclerView.VERTICAL, false)

            val strings: List<String> = content.split("\n".toRegex())

            val arrayList: MutableList<Spanned?> = mutableListOf()
            for (transfromColors in strings) {
                arrayList.add(Utils.transfromColors(transfromColors))
            }
            this.adapter.configureNew(arrayList)
        }
    }
    override fun receivePacket(type: Int, data: String?) {
        TODO("Not yet implemented")
    }
}