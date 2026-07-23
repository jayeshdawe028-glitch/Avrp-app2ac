package com.multirium.mobile.gui.dialogs

import android.graphics.Typeface
import android.text.Spanned
import android.text.SpannedString
import androidx.core.content.res.ResourcesCompat
import com.multirium.mobile.R
import com.multirium.mobile.core.Samp.activity
import com.multirium.mobile.gui.util.Utils

class DialogTabListHelper(val content:String) {
    private val font: Typeface? = ResourcesCompat.getFont(activity, R.font.muller_medium)
    private val fontSize: Int = activity.getResources().getDimensionPixelSize(R.dimen._8sdp)

    var maxColumnWidths: MutableList<Float> = mutableListOf()

    var rows: MutableList<TabListRow> = mutableListOf()
    init {
        val comparable: Comparable<*>?


        val splitToRows: Iterable<String> = splitToRows()
        val it: Iterator<*> = splitToRows.iterator()
        var i = 0
        if (!it.hasNext()) {
            comparable = null
        } else {

            var valueOf: Comparable<Int> = Integer.valueOf(
                (it.next() as String).split("\t".toRegex()).size
            )
            while (it.hasNext()) {

                val valueOf2: Comparable<Int> = Integer.valueOf(
                    (it.next() as String).split("\t".toRegex()).size
                )
                if (valueOf.compareTo(valueOf2 as Int) < 0) {
                    valueOf = valueOf2
                }
            }
            comparable = valueOf
        }
        val num = comparable as Int?
        i = num ?: i
        val arrayList: MutableList<TabListRow> = mutableListOf()
        for (splitToColumns in splitToRows) {
            arrayList.add(TabListRow(splitToColumns(splitToColumns, i)))
        }
        rows = arrayList
        calculateMaxColumnWidths()
    }

    private fun splitToRows(): List<String> {
        return content.split("\n".toRegex())
    }

    private fun splitToColumns(str: String, i: Int): MutableList<Spanned> {

        val `split$default`: Iterable<String> = str.split("\t".toRegex())

        val arrayList: MutableList<Spanned> = mutableListOf()

        for (transfromColors in `split$default`) {
            arrayList.add(Utils.transfromColors(transfromColors))
        }
        val mutableList: MutableList<Spanned> = arrayList
        while (mutableList.size < i) {
            mutableList.add(SpannedString.valueOf(""))
        }
        return mutableList
    }

    private fun calculateMaxColumnWidths() {
        val comparable: Comparable<Int>?
        val it: Iterator<TabListRow> = rows.iterator()
        if (!it.hasNext()) {
            comparable = null
        } else {
            var valueOf: Comparable<Int> = it.next().columns.size
            while (it.hasNext()) {
                val valueOf2: Comparable<Int> = it.next().columns.size
                if (valueOf < (valueOf2 as Int)) {
                    valueOf = valueOf2
                }
            }
            comparable = valueOf
        }
        val num = comparable as Int?
        val intValue = num ?: 0
        val arrayList: MutableList<Float> = mutableListOf(intValue.toFloat())
        for (i in 0 until intValue) {
            arrayList.add(0.0f)
        }
        maxColumnWidths = arrayList
        for (columns in rows) {
            var i2 = 0
            for (next in columns.columns) {
                val i3 = i2 + 1

                val textWidth: Float = Utils.INSTANCE.getTextWidth(
                    (next as Spanned).toString(),
                    fontSize.toFloat(),
                    font
                )
                if (textWidth > maxColumnWidths[i2]) {
                    maxColumnWidths[i2] = textWidth
                }
                i2 = i3
            }
        }
    }
    companion object {

    }
}