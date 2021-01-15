package com.dreamskies.sky;

import android.app.NativeActivity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.renderscript.ScriptGroup;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.TextView;
import android.provider.Settings.Secure;

import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.ConsumeParams;
import com.android.billingclient.api.ConsumeResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.SkuDetails;
import com.android.billingclient.api.SkuDetailsParams;
import com.android.billingclient.api.SkuDetailsResponseListener;

import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

public class SkyActivity extends NativeActivity {
    static {
        System.loadLibrary("fmod");
    }

    @Override
    public void onCreate(Bundle savedInstance) {
        super.onCreate(savedInstance);
        createSkyActivity();
        initKeyboard();
        hideSystemUI();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        destroySkyActivity();
    }

    private native void createSkyActivity();
    private native void destroySkyActivity();

    private native void onConsume(String id);

    // keyboard

    private EditText mEditText;

    private native void onKeyboardTextChanged(String text);
    private native void onKeyboardEnterPressed();

    private void initKeyboard() {
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);

        mEditText = new EditText(this);
        mEditText.setFocusable(true);
        mEditText.setFocusableInTouchMode(true);
        mEditText.setSingleLine();
        mEditText.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
                // nothing
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
                onKeyboardTextChanged(s.toString());
            }

            @Override
            public void afterTextChanged(Editable s) {
                // nothing
            }
        });
        mEditText.setOnEditorActionListener((v, actionId, event) -> {
            if (actionId == EditorInfo.IME_ACTION_DONE) {
                onKeyboardEnterPressed();
                return true;
            }
            return false;
        });
        ViewGroup.LayoutParams layoutParams = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        addContentView(mEditText, layoutParams);
    }

    public void showKeyboard() {
        InputMethodManager inputMethodManager = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
        inputMethodManager.showSoftInput(mEditText, InputMethodManager.SHOW_FORCED);
    }

    public void hideKeyboard() {
        InputMethodManager inputMethodManager = (InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE);
        inputMethodManager.hideSoftInputFromWindow(mEditText.getWindowToken(), 0);
    }

    public void setKeyboardText(final String text) {
        runOnUiThread(() -> {
            mEditText.setText(text);
            mEditText.setSelection(text.length());
        });
    }

    // billing

    private BillingClient mBillingClient;
    private Map<String, SkuDetails> mSkuDetails = new HashMap<>();

    public void initializeBilling(final List products) {
        PurchasesUpdatedListener purchasesUpdatedListener = (billingResult, purchases) -> {
            if (purchases == null)
                return;

            if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK)
                return;

            consume(purchases);
        };

        mBillingClient = BillingClient.newBuilder(this)
                .enablePendingPurchases()
                .setListener(purchasesUpdatedListener)
                .build();

        BillingClientStateListener billingClientStateListener = new BillingClientStateListener() {
                @Override
                public void onBillingSetupFinished(BillingResult billingResult) {
                    if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK)
                        return;

                    querySkuDetails(products);
                    consumeEstimatedPurchases();
                }
                @Override
                public void onBillingServiceDisconnected() {
                    // Try to restart the connection on the next request to
                    // Google Play by calling the startConnection() method.
                }
            };

        mBillingClient.startConnection(billingClientStateListener);
    }

    private void querySkuDetails(List products) {
        SkuDetailsParams params = SkuDetailsParams.newBuilder()
                .setSkusList(products)
                .setType(BillingClient.SkuType.INAPP)
                .build();

        SkuDetailsResponseListener skuDetailsResponseListener = (billingResult, skuDetailsList) -> {
            if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK)
                return;

            for (SkuDetails skuDetails : skuDetailsList) {
                mSkuDetails.put(skuDetails.getSku(), skuDetails);
            }
        };

        mBillingClient.querySkuDetailsAsync(params, skuDetailsResponseListener);
    }

    private void consumeEstimatedPurchases() {
        Purchase.PurchasesResult purchasesResult = mBillingClient.queryPurchases(BillingClient.SkuType.INAPP);
        consume(purchasesResult.getPurchasesList());
    }

    public void purchase(String product) {
        BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
                .setSkuDetails(mSkuDetails.get(product))
                .build();

        mBillingClient.launchBillingFlow(this, billingFlowParams);
    }

    public void consume(List<Purchase> purchases) {
        for (Purchase purchase : purchases) {
            consume(purchase);
        }
    }

    public void consume(final Purchase purchase) {
        ConsumeParams consumeParams = ConsumeParams.newBuilder()
                .setPurchaseToken(purchase.getPurchaseToken())
                .build();

        ConsumeResponseListener consumeResponseListener = (billingResult, outToken) -> {
            if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK)
                return;

            onConsume(purchase.getSku());
        };

        mBillingClient.consumeAsync(consumeParams, consumeResponseListener);
    }

    // ------------

    public String getUUID() {
        return Secure.getString(getContentResolver(), Secure.ANDROID_ID);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);

        if (!hasFocus)
            return;

        hideSystemUI();
    }

    private void hideSystemUI() {
        View decorView = getWindow().getDecorView();
        decorView.setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE |
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
            View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_FULLSCREEN);
    }
}
