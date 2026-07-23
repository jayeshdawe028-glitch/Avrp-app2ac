package com.multirium.mobile.launcher.web

import java.io.IOException
import java.net.InetSocketAddress
import java.net.Socket

object ServerChecker {
    /**
     * Server isServerAvailable checker
     *
     * @param ip       IP-адрес сервера, например "777.77.777.777"
     * @param port     Порт сервера, например 7777
     * @param timeout  Таймаут подключения в миллисекундах, например 3000 (3 секунды)
     * @return true, если сервер доступен, false в противном случае
     */
    fun isServerAvailable(ip: String?, port: Int, timeout: Int): Boolean {
        try {
            Socket().use { socket ->
                socket.connect(InetSocketAddress(ip, port), timeout)
                return true
            }
        } catch (ex: IOException) {
            return false
        }
    }
}