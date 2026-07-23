package com.multirium.mobile.launcher.web;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class ServerInfoResponse {

    @Expose
    @SerializedName("serverId")
    public final int serverId;

    @Expose
    @SerializedName("ip")
    public final String ip;

    @Expose
    @SerializedName("port")
    public final String port;

    @Expose
    @SerializedName("serverName")
    public final String serverName;

    @Expose
    @SerializedName("onlinePlayers")
    public final int onlinePlayers;

    @Expose
    @SerializedName("maxPlayers")
    public final int maxPlayers;

    @Expose
    @SerializedName("isAvailable")
    public final boolean isAvailable;

    @Expose
    @SerializedName("description")
    public final String description;

    public ServerInfoResponse(int serverId, String ip, String port, String serverName, int onlinePlayers, 
                              int maxPlayers, boolean isAvailable, String description) {
        this.serverId = serverId;
        this.ip = ip;
        this.port = port;
        this.serverName = serverName;
        this.onlinePlayers = onlinePlayers;
        this.maxPlayers = maxPlayers;
        this.isAvailable = isAvailable;
        this.description = description;
    }

}