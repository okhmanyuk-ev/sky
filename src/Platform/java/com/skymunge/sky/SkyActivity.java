package com.skymunge.sky;

import android.app.NativeActivity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;

import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
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

public class SkyActivity extends NativeActivity {
    static {
        System.loadLibrary("main");
        System.loadLibrary("fmod");
    }

    @Override
    public void onCreate(Bundle savedInstance) {
        super.onCreate(savedInstance);
        createSkyActivity();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        destroySkyActivity();
    }

    private native void createSkyActivity();
    private native void destroySkyActivity();

    // ------------------

    private BillingClient mBillingClient;
    private Map<String, SkuDetails> mSkuDetails = new HashMap<>();

    public void initializeBilling(final List products) {
        mBillingClient = BillingClient.newBuilder(this)
                .enablePendingPurchases()
                .setListener(new PurchasesUpdatedListener() {
                    @Override
                    public void onPurchasesUpdated(BillingResult billingResult, @Nullable List<Purchase> list) {
                        //
                    }
                })
                .build();

        mBillingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(BillingResult billingResult) {
                if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK)
                    return;

                querySkuDetails(products);
            }
            @Override
            public void onBillingServiceDisconnected() {
                // Try to restart the connection on the next request to
                // Google Play by calling the startConnection() method.
            }
        });
    }

    private void querySkuDetails(List products) {
        SkuDetailsParams params = SkuDetailsParams.newBuilder()
                .setSkusList(products)
                .setType(BillingClient.SkuType.INAPP)
                .build();

        mBillingClient.querySkuDetailsAsync(params,
                new SkuDetailsResponseListener() {
                    @Override
                    public void onSkuDetailsResponse(BillingResult billingResult, List<SkuDetails> skuDetailsList) {
                        if (billingResult.getResponseCode() != BillingClient.BillingResponseCode.OK)
                            return;

                        for (SkuDetails skuDetails : skuDetailsList) {
                            mSkuDetails.put(skuDetails.getSku(), skuDetails);
                        }
                    }
                });
    }

    public void purchase(String product) {
        BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
                .setSkuDetails(mSkuDetails.get(product))
                .build();

        mBillingClient.launchBillingFlow(this, billingFlowParams);
    }

}
