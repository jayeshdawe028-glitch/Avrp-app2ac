package com.multirium.mobile.gui.util

object GtaCordsDataKt {
    @JvmStatic
    val INSTANCE: GtaCordsDataKt = GtaCordsDataKt
    fun convertCordsToGta(gtaCordsData: GtaCordsData): GtaCordsData {
        val gtaCordsData2 =
            GtaCordsData(0.0f, 0.0f, 0.0f, 0.0f, 0, 0)
        gtaCordsData2.x =
            (gtaCordsData.x + (gtaCordsData.width / 2.0f)) / (gtaCordsData.screenWidth.toFloat()) * 640.0f
        gtaCordsData2.y =
            (gtaCordsData.y + (gtaCordsData.height / 2.2f)) / (gtaCordsData.screenHeight.toFloat()) * 480.0f
        gtaCordsData2.width =
            (gtaCordsData.width / (2f)) / (gtaCordsData.screenWidth.toFloat()) * 640.0f
        gtaCordsData2.height =
            (gtaCordsData.height / 3.333f) / (gtaCordsData.screenHeight.toFloat()) * 480.0f
        return gtaCordsData2
    }

}
