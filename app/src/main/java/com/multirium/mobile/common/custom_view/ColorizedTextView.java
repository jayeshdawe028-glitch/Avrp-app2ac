package com.multirium.mobile.common.custom_view;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.text.SpannableStringBuilder;
import android.text.style.ForegroundColorSpan;
import android.util.AttributeSet;
import android.widget.TextView;
import androidx.appcompat.widget.AppCompatTextView;
import com.multirium.mobile.R;
import kotlin.Metadata;
import kotlin.jvm.internal.DefaultConstructorMarker;
import kotlin.jvm.internal.Intrinsics;
import kotlin.text.MatchResult;
import kotlin.text.Regex;


public final class ColorizedTextView extends AppCompatTextView {
    public static final int $stable = 0;

     public ColorizedTextView(Context context) {
        this(context, (AttributeSet) null, 0, 6, (DefaultConstructorMarker) null);
        Intrinsics.checkNotNullParameter(context, "context");
    }

    public ColorizedTextView(Context context, AttributeSet attributeSet) {
        this(context, attributeSet, 0, 4, (DefaultConstructorMarker) null);
        Intrinsics.checkNotNullParameter(context, "context");
    }

    public ColorizedTextView(Context context, AttributeSet attributeSet, int i, int i2, DefaultConstructorMarker defaultConstructorMarker) {
        this(context, (i2 & 2) != 0 ? null : attributeSet, (i2 & 4) != 0 ? 16842884 : i);
    }

    public ColorizedTextView(Context context, AttributeSet attributeSet, int i) {
        super(context, attributeSet, i);
        Intrinsics.checkNotNullParameter(context, "context");
        if (attributeSet != null) {
            TypedArray obtainStyledAttributes = context.obtainStyledAttributes(attributeSet, R.styleable.ColorizedTextView);
            Intrinsics.checkNotNullExpressionValue(obtainStyledAttributes, "obtainStyledAttributes(...)");
            CharSequence text = obtainStyledAttributes.getText(R.styleable.ColorizedTextView_android_text);
            if (text != null) {
                setParsedText(text.toString());
            }
            obtainStyledAttributes.recycle();
        }
    }

    public void setText(CharSequence charSequence, BufferType bufferType) {
        String str;
        if (charSequence == null || (str = charSequence.toString()) == null) {
            str = "";
        }
        super.setText(parseTextWithColors(str), BufferType.SPANNABLE);
    }

    private final void setParsedText(String str) {
        super.setText(parseTextWithColors(str), BufferType.SPANNABLE);
    }

    private final SpannableStringBuilder parseTextWithColors(String str) {
        SpannableStringBuilder spannableStringBuilder = new SpannableStringBuilder();
        int currentTextColor = getCurrentTextColor();
        Regex regex = new Regex("\\{([A-Fa-f0-9]{6,8})\\}");
        int i = 0;
        while (true) {
            CharSequence charSequence = str;
            if (charSequence.length() > 0) {
                //MatchResult find$default = Regex.find$default(regex, charSequence, 0, 2, (Object) null);
                MatchResult find$default = regex.find(charSequence, 0);
                if (find$default == null) {
                    spannableStringBuilder.append(charSequence);
                    spannableStringBuilder.setSpan(new ForegroundColorSpan(currentTextColor), i, str.length() + i, 33);
                    break;
                }
                String substring = str.substring(0, find$default.getRange().getFirst());
                Intrinsics.checkNotNullExpressionValue(substring, "substring(...)");
                CharSequence charSequence2 = substring;
                if (charSequence2.length() > 0) {
                    spannableStringBuilder.append(charSequence2);
                    spannableStringBuilder.setSpan(new ForegroundColorSpan(currentTextColor), i, substring.length() + i, 33);
                    i += substring.length();
                }
                try {
                    currentTextColor = Color.parseColor('#' + find$default.getGroupValues().get(1));
                } catch (IllegalArgumentException unused) {
                    currentTextColor = getCurrentTextColor();
                }
                str = str.substring(find$default.getRange().getLast() + 1);
                Intrinsics.checkNotNullExpressionValue(str, "substring(...)");
            } else {
                break;
            }
        }
        return spannableStringBuilder;
    }
}
