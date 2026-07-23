package com.multirium.mobile.gui.util;

import android.content.Context;
import android.graphics.Color;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.style.ForegroundColorSpan;
import android.util.AttributeSet;

import androidx.appcompat.widget.AppCompatTextView;

public class ColorizedTextView extends AppCompatTextView {

    public ColorizedTextView(Context context) {
        super(context);
    }

    public ColorizedTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public ColorizedTextView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }
    public void setColorizedText(String fullText, String targetText, int color) {
        SpannableString spannable = new SpannableString(fullText);

        int start = fullText.indexOf(targetText);
        if (start >= 0) {
            spannable.setSpan(new ForegroundColorSpan(color), start,
                    start + targetText.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
        }

        setText(spannable);
    }
    public void setMultiColorText(String[][] textParts) {
        StringBuilder builder = new StringBuilder();
        for (String[] part : textParts) {
            builder.append(part[0]);
        }

        SpannableString spannable = new SpannableString(builder.toString());

        int cursor = 0;
        for (String[] part : textParts) {
            String text = part[0];
            int color = Color.parseColor(part[1]); // part[1] должен быть в формате "#RRGGBB"
            spannable.setSpan(new ForegroundColorSpan(color), cursor,
                    cursor + text.length(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
            cursor += text.length();
        }

        setText(spannable);
    }
}

