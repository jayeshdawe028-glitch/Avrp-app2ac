package com.multirium.mobile.gui.util

class GtaCordsData(f: Float, f2: Float, f3: Float, f4: Float, i: Int, i2: Int) {
    var height: Float = 0f
    var screenHeight: Int = 0
    var screenWidth: Int = 0
    var width: Float = 0f
    var x: Float = 0f
    var y: Float = 0f

    init {
        this.x = f
        this.y = f2
        this.width = f3
        this.height = f4
        this.screenWidth = i
        this.screenHeight = i2
    }
}
