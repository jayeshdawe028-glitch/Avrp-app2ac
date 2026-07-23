package com.multirium.mobile.launcher;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.constraintlayout.widget.ConstraintLayout;

import com.multirium.mobile.R;

import java.util.Locale;

public class CreateCharacterController {

    public interface CreateCb {
        void onSuccess(String finalNick, int finalSkin);
        void onError(String message);
    }

    public interface NickCheckCb {
        void onResult(boolean exists);
        void onError(String message);
    }
    public interface Host {
        void playClickSound();
        void buttonClick(View v);
        void hapticTick(View v);

        void forcedAnim(long durationMs, FloatUpdate update, EndCb end);

        void setLauncherSkin(int skinId);

        boolean isSlotEmpty(int slot);

        String getSlotNick(int slot);
        void setSlotNick(int slot, String nick);

        int getSlotSkin(int slot);
        void setSlotSkin(int slot, int skinId);

        void setBindStatus(int slot, boolean bound);

        void ensureUniqueNicks();
        void updateSlotsListUI();
        void applySelectedSlotToUIAndClient(boolean fromUser);
        void requestCreateCharacter(int slot, String nick, int skin, int sex, CreateCb cb);
        void checkNickExists(String nick, NickCheckCb cb);
        void toast(String text);
        void hideSystemUI();
    }

    public interface FloatUpdate { void update(float f); }
    public interface EndCb { void end(); }

    private final Activity activity;
    private final ConstraintLayout mainLayout;
    private final Host host;

    private View root;

    private TextView btnCreate;
    private View btnMale, btnFemale;
    private View btnNext, btnPrev;
    private View btnDiceName, btnDiceSurname;
    private View btnExit;

    private EditText etName, etSurname;

    private ImageView ivNameStatus, ivSurnameStatus;

    private int slot = 1;
    private boolean isMale = true;

    // Подгони под свои (или любые нормальные)
    private final int[] maleSkins = new int[]{ 29, 2, 30, 46, 5, 6, 20 };
    private final int[] femaleSkins = new int[]{ 11, 12, 13, 31, 65, 64, 9 };
    private int skinIndex = 0;

    private boolean creating = false;
    private CharSequence createBtnText;

    public CreateCharacterController(Activity activity, ConstraintLayout mainLayout, Host host) {
        this.activity = activity;
        this.mainLayout = mainLayout;
        this.host = host;

        init();
    }

    private void init() {
        root = activity.getLayoutInflater().inflate(R.layout.create_character_layout, null);
        activity.addContentView(root, new ConstraintLayout.LayoutParams(-1, -1));
        root.setVisibility(View.GONE);
        root.setAlpha(1f);

        // перехват тапов, чтобы не пробивало в игру
        root.setClickable(true);
        root.setFocusable(true);
        root.setFocusableInTouchMode(true);
        root.setOnTouchListener((v, e) -> true);

        btnCreate = root.findViewById(R.id.create_char_button);
        if (btnCreate != null) createBtnText = btnCreate.getText();
        btnMale = root.findViewById(R.id.male_sex_button);
        btnFemale = root.findViewById(R.id.female_sex_button);
        btnNext = root.findViewById(R.id.next_btn);
        btnPrev = root.findViewById(R.id.previous_btn);
        btnDiceName = root.findViewById(R.id.random_name_button);
        btnDiceSurname = root.findViewById(R.id.random_surname_button);
        btnExit = root.findViewById(R.id.exitBtn);

        etName = root.findViewById(R.id.input_name);
        etSurname = root.findViewById(R.id.input_surname);

        ivNameStatus = root.findViewById(R.id.input_name_status);
        ivSurnameStatus = root.findViewById(R.id.input_surname_status);

        // exit (по желанию)
        if (btnExit != null) {
            btnExit.setVisibility(View.VISIBLE);
            btnExit.setOnClickListener(v -> {
                click(v);
                hide();
            });
        }

        if (btnMale != null) btnMale.setOnClickListener(v -> { click(v); setSex(true); });
        if (btnFemale != null) btnFemale.setOnClickListener(v -> { click(v); setSex(false); });

        if (btnNext != null) btnNext.setOnClickListener(v -> { click(v); stepSkin(+1); });
        if (btnPrev != null) btnPrev.setOnClickListener(v -> { click(v); stepSkin(-1); });

        if (btnDiceName != null) btnDiceName.setOnClickListener(v -> {
            click(v);
            generateUniqueNickSuggestion(10);
        });

        if (btnDiceSurname != null) btnDiceSurname.setOnClickListener(v -> {
            click(v);
            generateUniqueNickSuggestion(10);
        });

        TextWatcher tw = new TextWatcher() {
            @Override public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
            @Override public void onTextChanged(CharSequence s, int start, int before, int count) { updateCreateButtonState(); }
            @Override public void afterTextChanged(Editable s) {}
        };
        if (etName != null) etName.addTextChangedListener(tw);
        if (etSurname != null) etSurname.addTextChangedListener(tw);

        if (btnCreate != null) btnCreate.setOnClickListener(v -> {
            click(v);
            onCreatePressed();
        });

        // дефолт
        setSex(true);
        updateCreateButtonState();
    }

    private void click(View v) {
        host.playClickSound();
        host.buttonClick(v);
        host.hapticTick(v);
    }

    public boolean isVisible() {
        return root != null && root.getVisibility() == View.VISIBLE;
    }

    public void show(int slot) {
        this.slot = clampSlot(slot);

        creating = false;
        if (btnCreate != null && createBtnText != null) btnCreate.setText(createBtnText);
        // сброс полей
        if (etName != null) etName.setText("");
        if (etSurname != null) etSurname.setText("");
        // дефолт пол/скин
        isMale = true;
        skinIndex = 0;
        applySkinToNative();
        updateSexButtons();

        updateCreateButtonState();

        if (mainLayout != null) mainLayout.setVisibility(View.GONE);

        root.setVisibility(View.VISIBLE);
        root.setAlpha(0f);

        // автогенерация ника
        generateUniqueNickSuggestion(10);

        host.forcedAnim(180, f -> root.setAlpha(f), () -> host.hideSystemUI());
    }

    void show(int slot, String prefillFirst, String prefillLast) {
        this.slot = clampSlot(slot);

        creating = false;
        if (btnCreate != null && createBtnText != null) btnCreate.setText(createBtnText);

        if (etName != null) etName.setText(prefillFirst != null ? prefillFirst : "");
        if (etSurname != null) etSurname.setText(prefillLast != null ? prefillLast : "");

        // дефолт пол/скин
        isMale = true;
        skinIndex = 0;
        applySkinToNative();

        updateCreateButtonState();

        if (mainLayout != null) mainLayout.setVisibility(View.GONE);

        root.setVisibility(View.VISIBLE);
        root.setAlpha(0f);

        host.forcedAnim(180, f -> root.setAlpha(f), () -> host.hideSystemUI());
    }


    public void hide() {
        if (root == null) return;

        final float a0 = root.getAlpha();
        host.forcedAnim(160, f -> root.setAlpha(a0 * (1f - f)), () -> {
            root.setVisibility(View.GONE);
            root.setAlpha(1f);
            if (mainLayout != null) mainLayout.setVisibility(View.VISIBLE);
            host.hideSystemUI();
        });
    }

    private void setSex(boolean male) {
        isMale = male;
        skinIndex = 0;
        applySkinToNative();
        updateSexButtons();
    }

    private void updateSexButtons() {
        if (btnMale != null) {
            btnMale.setBackgroundResource(isMale ? R.drawable.create_char_male_sex_bg : R.drawable.create_char_inactive_sex_bg);
        }
        if (btnFemale != null) {
            btnFemale.setBackgroundResource(!isMale ? R.drawable.create_char_female_sex_bg : R.drawable.create_char_inactive_sex_bg);
        }
    }

    private void stepSkin(int dir) {
        int[] arr = isMale ? maleSkins : femaleSkins;
        if (arr.length == 0) return;

        skinIndex += dir;
        if (skinIndex < 0) skinIndex = arr.length - 1;
        if (skinIndex >= arr.length) skinIndex = 0;

        applySkinToNative();
    }

    private int getSkin() {
        int[] arr = isMale ? maleSkins : femaleSkins;
        if (arr.length == 0) return 94;
        if (skinIndex < 0) skinIndex = 0;
        if (skinIndex >= arr.length) skinIndex = arr.length - 1;
        return arr[skinIndex];
    }

    private void applySkinToNative() {
        try { host.setLauncherSkin(getSkin()); } catch (Throwable ignored) {}
    }

    private void updateCreateButtonState() {
        if (btnCreate == null) return;

        String first = (etName != null) ? safe(etName.getText().toString()) : "";
        String last  = (etSurname != null) ? safe(etSurname.getText().toString()) : "";

        boolean nameOk = nickPartOk(first);
        boolean surnameOk = nickPartOk(last);

        if (ivNameStatus != null) ivNameStatus.setVisibility(nameOk ? View.VISIBLE : View.GONE);
        if (ivSurnameStatus != null) ivSurnameStatus.setVisibility(surnameOk ? View.VISIBLE : View.GONE);

        boolean ok = nameOk && surnameOk && !creating;

        btnCreate.setEnabled(ok);
        btnCreate.setAlpha(ok ? 1f : 0.55f);

        // если у тебя drawable меняется по enabled — можно так:
        // btnCreate.setBackgroundResource(ok ? R.drawable.create_char_active_button : R.drawable.create_char_inactive_button);
    }

    private void onCreatePressed() {
        if (creating) return;

        String first = (etName != null) ? safe(etName.getText().toString()) : "";
        String last  = (etSurname != null) ? safe(etSurname.getText().toString()) : "";

        if (!nickPartOk(first)) { host.toast("Имя: только латиница, минимум 3 символа"); return; }
        if (!nickPartOk(last))  { host.toast("Фамилия: только латиница, минимум 3 символа"); return; }

        String nick = first + "_" + last;
        int skin = getSkin();
        int sex = isMale ? 0 : 1;

        // защита от дубля по слотам (локально)
        for (int i = 1; i <= 3; i++) {
            if (i == slot) continue;
            String other = safe(host.getSlotNick(i));
            if (!other.isEmpty() && nick.equalsIgnoreCase(other)) {
                host.toast("Этот ник уже используется в другом слоте");
                return;
            }
        }

        // UI: блокируем кнопку пока ждём сервер
        creating = true;
        if (btnCreate != null) {
            btnCreate.setEnabled(false);
            btnCreate.setAlpha(0.55f);
            btnCreate.setText("Создание...");
        }

        host.requestCreateCharacter(slot, nick, skin, sex, new CreateCb() {
            @Override public void onSuccess(String finalNick, int finalSkin) {
                creating = false;
                if (btnCreate != null) {
                    btnCreate.setEnabled(true);
                    btnCreate.setAlpha(1f);
                    if (createBtnText != null) btnCreate.setText(createBtnText);
                }
                hide();
            }

            @Override public void onError(String message) {
                creating = false;
                if (btnCreate != null) {
                    btnCreate.setEnabled(true);
                    if (createBtnText != null) btnCreate.setText(createBtnText);
                    updateCreateButtonState();
                }
                host.toast(message != null && !message.isEmpty() ? message : "Не удалось создать персонажа");
            }
        });
    }

    private boolean nickPartOk(String s) {
        if (s == null) return false;
        String t = safe(s);
        if (t.length() < 3) return false;
        if (t.length() > 12) return false;

        for (int i = 0; i < t.length(); i++) {
            char c = t.charAt(i);
            boolean ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
            if (!ok) return false;
        }
        return true;
    }


    // ===================== RANDOM NICK PARTS =====================
    // Generate by symbols (letters+digits) to avoid collisions with huge online.
    private static final char[] NICK_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789".toCharArray();

    private String randomPart(int minLen, int maxLen) {
        int len = minLen + (int)(Math.random() * Math.max(1, (maxLen - minLen + 1)));
        if (len < minLen) len = minLen;
        if (len > maxLen) len = maxLen;

        StringBuilder sb = new StringBuilder(len);
        // first char: letter for nicer look
        char first = (char) ('A' + (int)(Math.random() * 26));
        sb.append(first);

        for (int i = 1; i < len; i++) {
            sb.append(NICK_CHARS[(int)(Math.random() * NICK_CHARS.length)]);
        }
        return sb.toString();
    }

    private void generateUniqueNickSuggestion(int triesLeft) {
        if (triesLeft <= 0) {
            host.toast("Не удалось подобрать свободный ник");
            return;
        }

        String first = randomPart(5, 10);
        String last  = randomPart(5, 10);
        final String nick = first + "_" + last;

        // local slot dup protection
        for (int i = 1; i <= 3; i++) {
            if (i == slot) continue;
            String other = safe(host.getSlotNick(i));
            if (!other.isEmpty() && nick.equalsIgnoreCase(other)) {
                generateUniqueNickSuggestion(triesLeft - 1);
                return;
            }
        }

        if (etName != null) etName.setText(first);
        if (etSurname != null) etSurname.setText(last);
        updateCreateButtonState();

        host.checkNickExists(nick, new NickCheckCb() {
            @Override public void onResult(boolean exists) {
                if (exists) {
                    host.toast("Ник занят, генерирую другой…");
                    generateUniqueNickSuggestion(triesLeft - 1);
                }
            }
            @Override public void onError(String message) {
                // if check failed, still keep generated nick, user can try create and server will validate
                if (message != null && !message.isEmpty()) host.toast(message);
            }
        });
    }
    private int clampSlot(int slot) { return Math.max(1, Math.min(3, slot)); }

    private String safe(String s) { return s == null ? "" : s.trim(); }
}
