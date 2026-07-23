package com.multirium.mobile.launcher.util;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonParseException;
import com.multirium.mobile.common.utils.GsonStore;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

public  class JsonUtilKt {
    public static boolean isJsonValid(String test) {
        if (test == null || test.trim().isEmpty()) {
            return false;
        }
        try {
            new JSONObject(test);
            return true;
        } catch (JSONException ex) {
            try {
                new JSONArray(test);
                return true;
            } catch (JSONException ex1) {
                return false;
            }
        }
    }
    public static String toStringJson(Object obj) {
        String json = GsonStore.INSTANCE.getGson().toJson(obj);
        return json;
    }
    public static <T> List<T> toListModel(String str, Class<T> cls) {
        if (!isJsonValid(str)) {
            return new ArrayList<>();
        }
        JsonArray asJsonArray = GsonStore.INSTANCE.getGson().fromJson(str, JsonElement.class).getAsJsonArray();
        Collection<T> arrayList = new ArrayList<>(asJsonArray.size());
        for (JsonElement fromJson : asJsonArray) {
            arrayList.add(GsonStore.INSTANCE.getGson().fromJson(fromJson, cls));
        }
        return new ArrayList<>(arrayList);
    }
    public static  <T> T toModel(String str, Class<T> cls) {

        if (isJsonValid(str)) {
            return GsonStore.INSTANCE.getGson().fromJson(str, cls);
        }
        throw new JsonParseException("Json is not valid");
    }
}
