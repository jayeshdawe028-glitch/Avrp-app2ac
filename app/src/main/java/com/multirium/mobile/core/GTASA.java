package com.multirium.mobile.core;

import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.KeyEvent;

import com.joom.paranoid.Obfuscate;
import com.wardrumstudios.utils.WarMedia;

@Obfuscate
public class GTASA extends WarMedia {


    static String vmVersion;
    private boolean once = false;

    static {
        vmVersion = null;
        try {
            vmVersion = System.getProperty("java.vm.version");
            //System.out.println("vmVersion " + vmVersion);
            System.loadLibrary("ImmEmulatorJ");
        }
        catch (ExceptionInInitializerError | UnsatisfiedLinkError ignored) {
        }
        System.loadLibrary("GTASA");
        //System.loadLibrary("bass");
        System.loadLibrary("multirium-client");
    }

    public boolean ServiceAppCommand(String str, String str2)
    {
        return false;
    }

    public int ServiceAppCommandValue(String str, String str2)
    {
        return 0;
    }

    public void onActivityResult(int i, int i2, Intent intent) {
        super.onActivityResult(i, i2, intent);
    }

    public void onConfigurationChanged(Configuration configuration) {
        super.onConfigurationChanged(configuration);
    }

    public void onCreate(Bundle bundle) {
        if(!once)
        {
            once = true;
        }

        System.out.println("GTASA onCreate");

        super.onCreate(bundle);
    }

    public void onDestroy() {
        System.out.println("GTASA onDestroy");
        super.onDestroy();
    }

    public boolean onKeyDown(int i, KeyEvent keyEvent)
    {
        return super.onKeyDown(i, keyEvent);
    }

    public void onPause() {
        System.out.println("GTASA onPause");
        super.onPause();
    }

    public void onRestart() {
        System.out.println("GTASA onRestart");
        super.onRestart();
    }

    public void onResume() {
        System.out.println("GTASA onResume");
        super.onResume();
    }

    public void onStart() {
        System.out.println("GTASA onStart");
        super.onStart();
    }

    public void onStop() {
        System.out.println("GTASA onStop");
        super.onStop();
    }
}