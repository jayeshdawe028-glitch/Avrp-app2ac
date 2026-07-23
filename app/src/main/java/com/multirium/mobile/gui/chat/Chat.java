package com.multirium.mobile.gui.chat;

import static com.multirium.mobile.gui.main_menu.utils.SettingMenuUtil.getValueSetting;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.text.Spanned;
import android.util.Log;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.TextView;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.joom.paranoid.Obfuscate;
import com.multirium.mobile.R;
import com.multirium.mobile.core.Samp;
import com.multirium.mobile.gui.keyboard.SAMPKeyboard;
import com.multirium.mobile.gui.main_menu.obj.SettingCategory;
import com.multirium.mobile.gui.util.Utils;
import com.multirium.mobile.databinding.ChatBinding;

import org.jetbrains.annotations.NotNull;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Pattern;
import androidx.annotation.NonNull;

import kotlin.Unit;

@Obfuscate
public class Chat {
    private final Activity activity;
    public boolean isShow;

    native void nativeInit();
    public native void nativeSendChatMessage(byte str[]);

    public ChatBinding binding;
    ChatAdapter chatadapter;
    ArrayList<Spanned> chat_lines = new ArrayList<>();
    public ViewGroup viewGroup = null;

    SAMPKeyboard sampKeyboard = null;

    private boolean userScrolling = false;
    private boolean stickToBottom = true;
    private LinearLayoutManager layoutManager;

    private static final LinkedHashMap<String, String> CHAT_MACROS = new LinkedHashMap<>();

    static {
        CHAT_MACROS.put("c_server", "{20B2AA}");
        CHAT_MACROS.put("c_s", "{FF0000}");
        CHAT_MACROS.put("c_b", "{FFFFFF}");
        CHAT_MACROS.put("c_i", "{32CD32}");
        CHAT_MACROS.put("c_m", "{009900}");
        CHAT_MACROS.put("c_red", "{FF0000}");
        CHAT_MACROS.put("c_ye", "{FFFF00}");
        CHAT_MACROS.put("c_white", "{FFFFFF}");
        CHAT_MACROS.put("adm_log", "{A9A9A9}");

        CHAT_MACROS.put("green_palka", "{32CD32}| {FFFFFF}");
        CHAT_MACROS.put("ye_palka", "{FFFF00}| {FFFFFF}");
        CHAT_MACROS.put("red_palka", "{FF0000}| {FFFFFF}");
        CHAT_MACROS.put("blue_palka", "{00CED1}| {FFFFFF}");

        CHAT_MACROS.put("debug", "{008B8B}[DEBUG] {FFFFFF}");
        CHAT_MACROS.put("error", "{FF0000}[ERROR] {FFFFFF}");
        CHAT_MACROS.put("warning", "{FFA500}[WARNING] {FFFFFF}");
        CHAT_MACROS.put("rpc", "{F0E68C}[RPC] {FFFFFF}");

        CHAT_MACROS.put("f_1", "{FFFFFF}");
        CHAT_MACROS.put("f_2", "{CD5C5C}");
        CHAT_MACROS.put("f_3", "{FF0000}");
        CHAT_MACROS.put("f_4", "{8B0000}");
        CHAT_MACROS.put("f_5", "{FFC0CB}");
        CHAT_MACROS.put("f_6", "{FFA07A}");
        CHAT_MACROS.put("f_7", "{FFFF00}");
        CHAT_MACROS.put("f_8", "{FFD700}");
        CHAT_MACROS.put("f_9", "{EE82EE}");
        CHAT_MACROS.put("f_10", "{800080}");
        CHAT_MACROS.put("f_11", "{808080}");
        CHAT_MACROS.put("f_12", "{C0C0C0}");
        CHAT_MACROS.put("f_13", "{00FF00}");
        CHAT_MACROS.put("f_14", "{008000}");
        CHAT_MACROS.put("f_15", "{228B22}");
        CHAT_MACROS.put("f_16", "{1E90FF}");
        CHAT_MACROS.put("f_17", "{00BFFF}");

        CHAT_MACROS.put("i_2", "{FFFFFF}");

        CHAT_MACROS.put("wh", "{FFFFFF}");
        CHAT_MACROS.put("white", "{FFFFFF}");
        CHAT_MACROS.put("w", "{FFFFFF}");

        CHAT_MACROS.put("rd", "{FF0000}");
        CHAT_MACROS.put("red", "{FF0000}");
        CHAT_MACROS.put("r", "{FF0000}");

        CHAT_MACROS.put("ye", "{FFFF00}");
        CHAT_MACROS.put("yellow", "{FFFF00}");
        CHAT_MACROS.put("y", "{FFFF00}");

        CHAT_MACROS.put("gn", "{32CD32}");
        CHAT_MACROS.put("green", "{32CD32}");
        CHAT_MACROS.put("g", "{32CD32}");

        CHAT_MACROS.put("bl", "{1E90FF}");
        CHAT_MACROS.put("blue", "{1E90FF}");
        CHAT_MACROS.put("b", "{1E90FF}");

        CHAT_MACROS.put("lb", "{00BFFF}");
        CHAT_MACROS.put("lightblue", "{00BFFF}");

        CHAT_MACROS.put("db", "{00008B}");
        CHAT_MACROS.put("darkblue", "{00008B}");

        CHAT_MACROS.put("aq", "{20B2AA}");
        CHAT_MACROS.put("aqua", "{20B2AA}");
        CHAT_MACROS.put("cyan", "{00CED1}");

        CHAT_MACROS.put("or", "{FFA500}");
        CHAT_MACROS.put("orange", "{FFA500}");

        CHAT_MACROS.put("pi", "{FFC0CB}");
        CHAT_MACROS.put("pink", "{FFC0CB}");

        CHAT_MACROS.put("pu", "{800080}");
        CHAT_MACROS.put("purple", "{800080}");

        CHAT_MACROS.put("gy", "{808080}");
        CHAT_MACROS.put("gray", "{808080}");
        CHAT_MACROS.put("grey", "{808080}");

        CHAT_MACROS.put("sv", "{C0C0C0}");
        CHAT_MACROS.put("silver", "{C0C0C0}");

        CHAT_MACROS.put("gold", "{FFD700}");
        CHAT_MACROS.put("lime", "{00FF00}");

        CHAT_MACROS.put("g_palka", "{32CD32}| {FFFFFF}");
        CHAT_MACROS.put("y_palka", "{FFFF00}| {FFFFFF}");
        CHAT_MACROS.put("r_palka", "{FF0000}| {FFFFFF}");
        CHAT_MACROS.put("b_palka", "{00CED1}| {FFFFFF}");
    }

    public Chat(Activity activity) {
        this.activity = activity;

        binding = ChatBinding.inflate(LayoutInflater.from(activity));
        viewGroup = binding.getRoot();
        Samp.getInstance().getBackUILayout().addView(viewGroup, -1, -1);

        nativeInit();

        layoutManager = new LinearLayoutManager(activity);
        layoutManager.setStackFromEnd(true);
        binding.chatRecycler.setLayoutManager(layoutManager);

        binding.chatRecycler.setVerticalScrollBarEnabled(true);
        binding.chatRecycler.setScrollbarFadingEnabled(false);
        binding.chatRecycler.setOverScrollMode(View.OVER_SCROLL_ALWAYS);

        binding.chatRecycler.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrollStateChanged(@NonNull RecyclerView recyclerView, int newState) {
                userScrolling = newState == RecyclerView.SCROLL_STATE_DRAGGING
                        || newState == RecyclerView.SCROLL_STATE_SETTLING;

                if (newState == RecyclerView.SCROLL_STATE_IDLE) updateStickBottom();
            }

            @Override
            public void onScrolled(@NonNull RecyclerView recyclerView, int dx, int dy) {
                updateStickBottom();
            }

            private void updateStickBottom() {
                int lastCompletely = layoutManager.findLastCompletelyVisibleItemPosition();
                int total = chatadapter != null ? chatadapter.getItemCount() : 0;
                stickToBottom = total == 0 || lastCompletely >= total - 1;
            }
        });

        chatadapter = new ChatAdapter(activity, chat_lines);
        binding.chatRecycler.setAdapter(chatadapter);
        binding.chatRecycler.setClickable(true);
        binding.chatRecycler.setFocusable(true);

        chatadapter.onClick = this::ChatClick;

        binding.chatInputLayout.setVisibility(View.GONE);
        binding.chatInput.setShowSoftInputOnFocus(false);

        binding.chatInput.setOnEditorActionListener((v, actionId, event) -> {
            if (actionId == EditorInfo.IME_ACTION_SEND) {
                try {
                    nativeSendChatMessage(binding.chatInput.getText().toString().getBytes("windows-1251"));
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
                toggleKeyboard(false);
                return true;
            }
            return false;
        });

        isShow = false;

        initKeyboard();
    }

    public void ChatClickFromHud() {
        activity.runOnUiThread(() -> toggleKeyboard(binding.chatInputLayout.getVisibility() != View.VISIBLE));
    }

    private void pushToHudChat(int type, String msg) {
        try {
            Samp s = Samp.getInstance();
            if (s != null) s.pushHudChat(type, msg);
        } catch (Throwable ignored) { }
    }

    private String preprocessChatColors(String s) {
        if (s == null) return "";

        String out = s;

        for (Map.Entry<String, String> e : CHAT_MACROS.entrySet()) {
            String key = e.getKey();
            String val = e.getValue();

            out = out.replace("\"\"" + key + "\"", val);
            out = out.replace("\"" + key + "\"", val);
            out = out.replace("'" + key + "'", val);

            out = out.replaceAll("(?i)(?<![A-Za-z0-9_])" + Pattern.quote(key) + "(?![A-Za-z0-9_])", val);
        }
        return out;
    }

    private void initKeyboard() {
        sampKeyboard = new SAMPKeyboard(null);

        ViewGroup keyboardView = sampKeyboard.getBinding().getRoot();
        @SuppressLint("ResourceType") ViewGroup rootContent = activity.findViewById(16908290);

        float maxElevation = 0f;
        for (int i = 0; i < rootContent.getChildCount(); i++) {
            float e = rootContent.getChildAt(i).getElevation();
            if (e > maxElevation) maxElevation = e;
        }
        keyboardView.setElevation(maxElevation + 1.0f);

        if (rootContent.indexOfChild(keyboardView) == -1) {
            rootContent.addView(keyboardView, new ViewGroup.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT
            ));
        }

        sampKeyboard.onClickEnter = new kotlin.jvm.functions.Function0<Unit>() {
            @Override
            public Unit invoke() {
                String text = sampKeyboard.getBinding().keyboardInput.getText().toString();
                sampKeyboard.clear();
                toggleKeyboard(false);

                try {
                    nativeSendChatMessage(text.getBytes("windows-1251"));
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
                return null;
            }
        };
    }

    public void ToggleChat(boolean toggle) {
        activity.runOnUiThread(() -> binding.chatRecycler.setVisibility(toggle ? View.VISIBLE : View.GONE));
    }

    public void AddChatMessage(String msg, int textcolor) {
        activity.runOnUiThread(() -> {
            String fixedMsg = preprocessChatColors(msg);

            String color = String.format("%X", textcolor, fixedMsg.hashCode());
            String temp = color.substring(0, 6);

            chatadapter.addItem("{" + temp + "}" + fixedMsg);

            pushToHudChat(0, fixedMsg);
        });
    }

    public void ToggleChatInput(boolean toggle) {
        activity.runOnUiThread(() -> {
            if (toggle) {
                binding.chatInputLayout.setVisibility(View.VISIBLE);
            } else {
                binding.chatInputLayout.setVisibility(View.GONE);
                binding.chatInput.getText().clear();
            }
        });
    }

    void toggleKeyboard(boolean toggle) {
        int type_keyboard = getValueSetting(SettingCategory.PERSONALIZATION, "Выбор клавиатуры");

        switch (type_keyboard) {
            case 1: {
                sampKeyboard.setUIVisible(toggle);
                break;
            }
            case 2: {
                binding.chatInput.requestFocus();

                InputMethodManager imm = (InputMethodManager) activity.getSystemService(Context.INPUT_METHOD_SERVICE);
                ToggleChatInput(toggle);
                if (toggle) imm.showSoftInput(binding.chatInput, InputMethodManager.SHOW_IMPLICIT);
                else imm.hideSoftInputFromWindow(binding.chatInput.getWindowToken(), 0);
                break;
            }
            default:
                Log.d("Multirium.chat", "Error type keyboard");
        }
    }

    public void ChatClick() {
        activity.runOnUiThread(() -> toggleKeyboard(binding.chatInputLayout.getVisibility() != View.VISIBLE));
    }

    public class ChatAdapter extends RecyclerView.Adapter<ChatAdapter.ViewHolder> {

        private final LayoutInflater inflater;
        private final List<Spanned> chat_lines;

        public Runnable onClick;

        ChatAdapter(Context context, List<Spanned> chat_lines) {
            this.chat_lines = chat_lines;
            this.inflater = LayoutInflater.from(context);
        }

        @NotNull
        @Override
        public ChatAdapter.ViewHolder onCreateViewHolder(@NotNull ViewGroup parent, int viewType) {
            View view = inflater.inflate(R.layout.chatline, parent, false);
            return new ChatAdapter.ViewHolder(view);
        }

        @Override
        public void onBindViewHolder(ChatAdapter.ViewHolder holder, int position) {
            holder.chat_line_text.setText(chat_lines.get(position));
            holder.chat_line_shadow.setText(chat_lines.get(position).toString());

            holder.chat_line_shadow.setTextSize(TypedValue.COMPLEX_UNIT_PX, 27);
            holder.chat_line_text.setTextSize(TypedValue.COMPLEX_UNIT_PX, 27);
        }

        @Override
        public int getItemCount() {
            return chat_lines.size();
        }

        public class ViewHolder extends RecyclerView.ViewHolder {
            final TextView chat_line_text;
            final TextView chat_line_shadow;

            ViewHolder(View view) {
                super(view);
                chat_line_text = view.findViewById(R.id.chat_line_text);
                chat_line_shadow = view.findViewById(R.id.chat_line_shadow);

                view.setOnClickListener(v -> {
                    if (onClick != null) onClick.run();
                });
            }
        }

        public void addItem(String item) {
            if (this.chat_lines.size() > 40) {
                this.chat_lines.remove(0);
                notifyItemRemoved(0);
            }

            String fixed = preprocessChatColors(item);
            this.chat_lines.add(Utils.transfromColors(" " + fixed + " "));
            notifyItemInserted(this.chat_lines.size() - 1);

            // ✅ Автоскролл только если пользователь НЕ листает и он был внизу
            if (!userScrolling && stickToBottom) {
                binding.chatRecycler.scrollToPosition(this.chat_lines.size() - 1);
            }
        }
    }
}
