package com.multirium.mobile.gui.util;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.text.Html;
import android.text.Spanned;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.Pair;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;

import androidx.core.content.ContextCompat;

import com.multirium.mobile.core.Samp;
import com.nvidia.devtech.NvEventQueueActivity;

import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.regex.Pattern;

import kotlin.Lazy;
import kotlin.LazyKt;
import kotlin.jvm.internal.Intrinsics;

public class Utils {
    public static final Utils INSTANCE = new Utils();
    static final String AB = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static SecureRandom rnd = new SecureRandom();

    private final Lazy screenSizeInternal$delegate = LazyKt.lazy(() -> Utils.INSTANCE.calculateScreenSize());

    private final Pair<Integer, Integer> getScreenSizeInternal() {
        return (Pair<Integer, Integer>) screenSizeInternal$delegate.getValue();
    }

    public final Pair<Integer, Integer> getScreenSize() {
        return getScreenSizeInternal();
    }

    public final Pair<Integer, Integer> calculateScreenSize() {
        DisplayMetrics displayMetrics = new DisplayMetrics();
        Object systemService = Samp.getActivity().getSystemService(Context.WINDOW_SERVICE);
        Intrinsics.checkNotNull(systemService, "null cannot be cast to non-null type android.view.WindowManager");
        ((WindowManager) systemService).getDefaultDisplay().getRealMetrics(displayMetrics);
        return new Pair<>(displayMetrics.widthPixels, displayMetrics.heightPixels);
    }

    public void fixInputKeyboard(ViewGroup viewGroup){
        FrameLayout.LayoutParams layoutParamss = (FrameLayout.LayoutParams)viewGroup.getLayoutParams();
        layoutParamss.bottomMargin = NvEventQueueActivity.getInstance().getSurfaceHeight() / 3;
        viewGroup.setLayoutParams(layoutParamss);
    }

    public final float getTextWidth(String str, float f, Typeface typeface) {
        Paint paint = new Paint();
        paint.setTextSize(f);
        paint.setTypeface(typeface);
        return paint.measureText(str);
    }

    public static void ShowLayout(View view, boolean isAnim) {
        if (view != null) {
            if (isAnim) {
                view.setAlpha(0.0f);
                fadeIn(view);
            } else {
                view.setAlpha(1.0f);
            }
            view.setVisibility(View.VISIBLE);
            view.invalidate();
        }
    }

    public static void HideLayout(View view, boolean isAnim) {
        if (view != null) {
            if (isAnim) {
                fadeOut(view);
                return;
            }
            view.setAlpha(0.0f);
            view.setVisibility(View.GONE);
            view.invalidate();
        }
    }

    private static void fadeIn(View view) {
        if (view != null) {
            view.animate().setDuration(150).setListener(new AnimatorListenerAdapter() {
                public void onAnimationEnd(Animator animation) {

                    super.onAnimationEnd(animation);
                }
            }).alpha(1.0f);
        }
    }

    private static void fadeOut(final View view) {
        if (view != null) {
            view.animate().setDuration(150).setListener(new AnimatorListenerAdapter() {
                public void onAnimationEnd(Animator animation) {
                    view.setVisibility(View.GONE);
                    super.onAnimationEnd(animation);
                }
            }).alpha(0.0f);
        }
    }

    public static Drawable getRes(Activity activity, int id) {
        return ContextCompat.getDrawable(activity.getApplicationContext(), id);
    }

    public static Spanned transfromColors(String str) {
        int i;
        LinkedList linkedList = new LinkedList();
        int i2 = 0;
        String str2 = str;
        int i3 = 0;
        for (int i4 = 0; i4 < str2.length(); i4++) {
            if (str2.charAt(i4) == '{' && (i = i4 + 7) < str2.length()) {
                StringBuilder sb = new StringBuilder();
                sb.append("#");
                int i5 = i4 + 1;
                sb.append(str2.substring(i5, i));
                linkedList.addLast(sb.toString());
                str2 = str2.substring(0, i5) + "repl" + i3 + str2.substring(i);
                i3++;
            }
        }
        Iterator it = linkedList.iterator();
        while (it.hasNext()) {
            String str3 = (String) it.next();
            try {
                int colorInt = Color.parseColor(str3);
                String hexColor = String.format("#%06X", (0xFFFFFF & colorInt));
                if (i2 == 0) {
                    str2 = str2.replaceAll(Pattern.quote("{repl" + i2 + "}"), "<font color='" + hexColor + "'>");
                } else {
                    str2 = str2.replaceAll(Pattern.quote("{repl" + i2 + "}"), "</font><font color='" + hexColor + "'>");
                }
            } catch (IllegalArgumentException e) {
                Log.e("ColorError", "Invalid color: " + str3);
                if (i2 == 0) {
                    str2 = str2.replaceAll(Pattern.quote("{repl" + i2 + "}"), "<font color='#FFFFFF'>");
                } else {
                    str2 = str2.replaceAll(Pattern.quote("{repl" + i2 + "}"), "</font><font color='#FFFFFF'>");
                }
            }
            i2++;
        }
        if (linkedList.size() >= 1) {
            str2 = str2 + "</font>";
        }
        return Html.fromHtml(str2.replaceAll(Pattern.quote("\n"), "<br>"));
    }
    public static ArrayList<String> fixFieldsForDialog(ArrayList<String> fields) {
        ArrayList<String> newList = new ArrayList<>();
        int max = 0;
        for (int i = 0; i < fields.size(); i++) {
            int len = fields.get(i).split("\t").length;
            if (len > max) {
                max = len;
            }
        }
        for (int i2 = 0; i2 < fields.size(); i2++) {
            StringBuilder field = new StringBuilder(fields.get(i2));
            for (int len2 = fields.get(i2).split("\t").length; len2 != max; len2++) {
                field.append("\\t ");
            }
            newList.add(field.toString());
        }
        return newList;
    }

    public static String randomString(int len) {
        StringBuilder sb = new StringBuilder(len);
        for (int i = 0; i < len; i++) {
            sb.append(AB.charAt(rnd.nextInt(AB.length())));
        }
        return sb.toString();
    }
}
