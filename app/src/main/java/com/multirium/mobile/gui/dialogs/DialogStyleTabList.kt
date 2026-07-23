package com.multirium.mobile.gui.dialogs

import android.view.View
import android.widget.TextView
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.databinding.DialogStyleTablistBinding
import com.multirium.mobile.gui.NativeGui
import com.multirium.mobile.gui.util.Utils
import kotlin.jvm.internal.Reflection


class DialogStyleTabList(var caption: String, var content: String, var leftBtnText: String, var rightBtnText: String, var withHeaders: Boolean) : NativeGui<DialogStyleTablistBinding>(Reflection.getOrCreateKotlinClass(DialogStyleTablistBinding::class.java), false, false,true, true) {
    private var adapter: DialogStyleTabListAdapter = DialogStyleTabListAdapter()
    init {
        val dialogStyleTabListAdapter = DialogStyleTabListAdapter()
        this.adapter = dialogStyleTabListAdapter
        if (!withHeaders) {
            (getBinding() as DialogStyleTablistBinding).headersContainer.visibility = View.GONE
        }
        (getBinding() as DialogStyleTablistBinding).caption.text = Utils.transfromColors(this.caption)
        (getBinding() as DialogStyleTablistBinding).leftButtText.text = Utils.transfromColors(this.leftBtnText)
        (getBinding() as DialogStyleTablistBinding).leftButton.setOnClickListener {
            Dialog.INSTANCE.sendResponse(1, adapter.getSelectedListItem().listitem, adapter.getSelectedListItem().inputText)
        }
        if (this.rightBtnText.isEmpty()) {
            (getBinding() as DialogStyleTablistBinding).rightButton.visibility = View.GONE
        } else {
            (getBinding() as DialogStyleTablistBinding).rightButtText.text = Utils.transfromColors(this.rightBtnText)
            (getBinding() as DialogStyleTablistBinding).rightButton.setOnClickListener {
                Dialog.INSTANCE.sendResponse(0, adapter.getSelectedListItem().listitem, adapter.getSelectedListItem().inputText)
            }
        }
        dialogStyleTabListAdapter.onDoubleClick = {
            Dialog.INSTANCE.sendResponse(1, adapter.getSelectedListItem().listitem, adapter.getSelectedListItem().inputText)
        }
        (getBinding() as DialogStyleTablistBinding).recycler.adapter =
            dialogStyleTabListAdapter
        (getBinding() as DialogStyleTablistBinding).recycler.layoutManager =
            LinearLayoutManager(activity, RecyclerView.VERTICAL, false)
        val dialogTabListHelper = DialogTabListHelper(this.content)
        if (withHeaders) {
            fillHeaders(dialogTabListHelper)
            dialogTabListHelper.rows.removeAt(0)
        }
        dialogStyleTabListAdapter.configureNew(dialogTabListHelper)
    }

    private fun fillHeaders(dialogTabListHelper: DialogTabListHelper) {
        val tabListRow = dialogTabListHelper.rows[0]
        var i = 0
        for (next in mutableListOf(
            (getBinding() as DialogStyleTablistBinding).header1,
            (getBinding() as DialogStyleTablistBinding).header2,
            (getBinding() as DialogStyleTablistBinding).header3,
            (getBinding() as DialogStyleTablistBinding).header4
        )) {
            val i2 = i + 1

            val textView = next as TextView
            if (i < tabListRow.columns.size) {
                textView.width = dialogTabListHelper.maxColumnWidths[i].toInt()
                textView.text = tabListRow.columns[i]
                textView.visibility = View.VISIBLE
            } else {
                textView.visibility = View.GONE
            }
            i = i2
        }
    }

    override fun receivePacket(type: Int, data: String?) {
        TODO("Not yet implemented")
    }
}