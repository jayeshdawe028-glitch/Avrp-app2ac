package com.multirium.mobile

import android.app.Application
import com.liulishuo.filedownloader.FileDownloader

class App : Application() {
    override fun onCreate() {
        super.onCreate()
        FileDownloader.setup(this)
    }
}