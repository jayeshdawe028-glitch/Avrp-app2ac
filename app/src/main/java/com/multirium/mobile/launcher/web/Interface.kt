package com.multirium.mobile.launcher.web

import retrofit2.Call
import retrofit2.http.GET

interface Interface {
    @get:GET("https://wh27264.web2.maze-tech.ru/servers.json")
    val servers: Call<List<ServerInfoResponse?>?>?

    @get:GET("https://wh27264.web2.maze-tech.ru/news.json")
    val news: Call<List<NewsInfoResponse?>?>?
}
