package com.multirium.mobile

import org.json.JSONObject

interface GUIWrapper {
    fun onShow(data: JSONObject?)
    fun onClose()
    fun receiveUIpacket(data: JSONObject?)
}
