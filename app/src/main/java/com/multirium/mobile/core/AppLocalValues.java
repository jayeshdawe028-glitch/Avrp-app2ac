package com.multirium.mobile.core;

import android.os.Environment;

import com.nvidia.devtech.NvUtil;

import java.util.HashMap;
import kotlin.Metadata;
import kotlin.jvm.JvmField;
import kotlin.jvm.JvmStatic;
import kotlin.jvm.internal.DefaultConstructorMarker;
import kotlin.jvm.internal.Intrinsics;
import org.jetbrains.annotations.NotNull;
import org.jetbrains.annotations.Nullable;

public final class AppLocalValues {
    @NotNull
    @JvmField
    public static final AppLocalValues instance = new AppLocalValues();
    @Nullable
    public HashMap<String, String> mAppLocalValues = new HashMap<>();

    public static AppLocalValues getInstance()
    {
        return instance;
    }

    public AppLocalValues() {
        setAppLocalValue("STORAGE_ROOT", Environment.getExternalStorageDirectory().getAbsolutePath());
    }

    public boolean hasAppLocalValue(@NotNull String key) {
        return this.mAppLocalValues.containsKey(key);
    }

    @Nullable
    public final String getAppLocalValue(@NotNull String key) {
        return mAppLocalValues.get(key);
    }

    public final void setAppLocalValue(@NotNull String str, @NotNull String str2) {
        Intrinsics.checkNotNullParameter(str, "key");
        Intrinsics.checkNotNullParameter(str2, "value");
        HashMap<String, String> hashMap = this.mAppLocalValues;
        if (hashMap != null) {
            hashMap.put(str, str2);
        }
    }
}
