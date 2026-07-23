package com.multirium.mobile.services;

import android.app.Activity;

public interface ActivityService {

    void showErrorMessage(String message, Activity activity);

    void showInfoMessage(String message, Activity activity);

    boolean isGameFileInstall(Activity activity, String filePath);

    void showBigMessage(String message, Activity activity);
}
