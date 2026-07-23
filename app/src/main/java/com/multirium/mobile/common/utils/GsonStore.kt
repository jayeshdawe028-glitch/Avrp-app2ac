package com.multirium.mobile.common.utils

import com.google.gson.Gson
import com.google.gson.GsonBuilder

object GsonStore {
    val gson: Gson = GsonBuilder().create()
}