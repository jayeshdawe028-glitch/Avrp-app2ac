package com.multirium.mobile.launcher.web;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class NewsInfoResponse {

    @Expose
    @SerializedName("imageurl")
    private final String imageUrl;

    @Expose
    @SerializedName("title")
    private final String title;

    @Expose
    @SerializedName("url")
    private final String url;

    public NewsInfoResponse(String imageUrl, String title, String url) {
        this.imageUrl = imageUrl;
        this.title = title;
        this.url = url;
    }

    public String getImageUrl() {
        return imageUrl;
    }

    public String getTitle() {
        return title;
    }

    public String getUrl() {
        return url;
    }
}