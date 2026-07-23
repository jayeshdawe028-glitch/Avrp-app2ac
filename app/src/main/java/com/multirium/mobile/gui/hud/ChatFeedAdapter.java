package com.multirium.mobile.gui.hud;

import android.app.Activity;
import android.text.Spanned;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.multirium.mobile.R;
import com.multirium.mobile.gui.util.Utils;

import java.util.ArrayList;

public class ChatFeedAdapter extends RecyclerView.Adapter<ChatFeedAdapter.VH> {

    private final Activity activity;
    private RecyclerView recyclerView;

    private final ArrayList<ChatLine> items = new ArrayList<>();
    private int maxLines = 6;

    private boolean userScrolling = false;
    private boolean stickToTop = true; // т.к. reverseLayout=true, "низ" для нас это position=0

    public ChatFeedAdapter(Activity activity) {
        this.activity = activity;
        setHasStableIds(true);
    }

    public void attach(@NonNull RecyclerView rv) {
        this.recyclerView = rv;

        LinearLayoutManager lm = new LinearLayoutManager(activity);
        lm.setOrientation(RecyclerView.VERTICAL);
        lm.setReverseLayout(true);
        lm.setStackFromEnd(false);

        rv.setLayoutManager(lm);
        rv.setItemAnimator(null);
        rv.setAdapter(this);

        // Скроллбар + поведение скролла
        rv.setVerticalScrollBarEnabled(true);
        rv.setScrollbarFadingEnabled(false);
        rv.setOverScrollMode(View.OVER_SCROLL_ALWAYS);

        rv.addOnScrollListener(new RecyclerView.OnScrollListener() {
            @Override
            public void onScrollStateChanged(@NonNull RecyclerView recyclerView, int newState) {
                userScrolling = newState == RecyclerView.SCROLL_STATE_DRAGGING
                        || newState == RecyclerView.SCROLL_STATE_SETTLING;

                // когда остановился — пересчитаем "мы на самом верху (0)?"
                if (newState == RecyclerView.SCROLL_STATE_IDLE) updateStickState();
            }

            @Override
            public void onScrolled(@NonNull RecyclerView recyclerView, int dx, int dy) {
                updateStickState();
            }

            private void updateStickState() {
                RecyclerView.LayoutManager m = rv.getLayoutManager();
                if (!(m instanceof LinearLayoutManager)) return;
                int first = ((LinearLayoutManager) m).findFirstCompletelyVisibleItemPosition();
                // при reverseLayout=true "последние сообщения" = позиция 0
                stickToTop = (first == 0);
            }
        });
    }

    public void setMaxLines(int maxLines) {
        this.maxLines = Math.max(1, maxLines);
        trim();
        notifyDataSetChanged();
    }

    public void push(int type, String time, String text) {
        items.add(0, new ChatLine(type, time, text));
        trim();
        notifyDataSetChanged();

        // Автоскролл только если пользователь не листает и мы "на последнем сообщении" (position=0)
        if (recyclerView != null && !userScrolling && stickToTop) {
            recyclerView.scrollToPosition(0);
        }
    }

    public void clear() {
        items.clear();
        notifyDataSetChanged();
    }

    private void trim() {
        while (items.size() > maxLines) items.remove(items.size() - 1);
    }

    @Override
    public long getItemId(int position) {
        ChatLine l = items.get(position);
        return (l.type + "|" + l.time + "|" + l.text).hashCode();
    }

    @NonNull
    @Override
    public VH onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View v = LayoutInflater.from(parent.getContext()).inflate(R.layout.item_chat_feed, parent, false);
        return new VH(v);
    }

    @Override
    public void onBindViewHolder(@NonNull VH h, int position) {
        ChatLine line = items.get(position);

        h.time.setText(line.time);

        Spanned sp = Utils.transfromColors(line.text == null ? "" : line.text);
        h.text.setText(sp);

        h.typeBar.setBackgroundResource(R.drawable.bg_chat_type_bar);
    }

    @Override
    public int getItemCount() {
        return items.size();
    }

    static final class ChatLine {
        final int type;
        final String time;
        final String text;

        ChatLine(int type, String time, String text) {
            this.type = type;
            this.time = time;
            this.text = text;
        }
    }

    static final class VH extends RecyclerView.ViewHolder {
        final View typeBar;
        final TextView time;
        final TextView text;

        VH(@NonNull View itemView) {
            super(itemView);
            typeBar = itemView.findViewById(R.id.chat_type_bar);
            time = itemView.findViewById(R.id.chat_time);
            text = itemView.findViewById(R.id.chat_text);
        }
    }
}
