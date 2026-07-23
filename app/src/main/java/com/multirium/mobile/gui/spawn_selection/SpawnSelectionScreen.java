package com.multirium.mobile.gui.spawn_selection;

import android.app.Activity;
import android.view.LayoutInflater;
import android.view.ViewGroup;

import androidx.recyclerview.widget.LinearLayoutManager;

import com.multirium.mobile.R;
import com.multirium.mobile.GUIManager;
import com.multirium.mobile.GUIWrapper;
import com.multirium.mobile.databinding.SpawnSelectionLayoutBinding;

import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

public class SpawnSelectionScreen implements GUIWrapper {
    private SpawnSelectionLayoutBinding binding;
    private Activity activity;
    private ViewGroup root;
    public static SpawnSelectionScreen newInstance(Activity activity, ViewGroup root) {
        SpawnSelectionScreen screen = new SpawnSelectionScreen();

        screen.activity = activity;
        screen.root = root;
        screen.binding = SpawnSelectionLayoutBinding.inflate(LayoutInflater.from(activity), root, false);

        return screen;
    }
    @Override
    public void onShow(JSONObject data) {
        activity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (binding.getRoot().getParent() == null) {
                    root.addView(binding.getRoot());
                }
            }
        });
    }

    @Override
    public void onClose() {
        activity.runOnUiThread(() -> {
            if (binding.getRoot().getParent() != null) {
                root.removeView(binding.getRoot());
            }
        });
    }
    private static void toggle(int id, int toggle) {
        JSONObject data = new JSONObject();
        try {
            data.put("o", toggle);
        } catch (Exception e) {
            e.printStackTrace();
        }

        GUIManager.receiveUIpacket(id, data);
    }
    @Override
    public void receiveUIpacket(JSONObject data) {}
}
