package com.multirium.mobile.core

object NeonRpc {

    @JvmStatic external fun nativeSendNeonControl(
        vehicleId: Int,
        modeByte: Byte,
        r: Byte,
        g: Byte,
        b: Byte,
        brightness: Byte,
        speed: Byte
    )

    @JvmStatic fun sendNeonControl(
        vehicleId: Int,
        modeByte: Byte,
        r: Byte,
        g: Byte,
        b: Byte,
        brightness: Byte,
        speed: Byte
    ) {
        nativeSendNeonControl(vehicleId, modeByte, r, g, b, brightness, speed)
    }
}
