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
import java.util.List;

public class SkyActivity extends NativeActivity {
    static {
        System.loadLibrary("fmod");
    }

   // private BillingClient billingClient;

    @Override
    public void onCreate(Bundle savedInstance) {
        super.onCreate(savedInstance);
      //  initSkyActivity();
    }

    private static native void initSkyActivity();

    /*private void setupBillingClient() {
        billingClient = BillingClient.newBuilder(this)
                .enablePendingPurchases()
                .setListener(new PurchasesUpdatedListener() {
                    @Override
                    public void onPurchasesUpdated(BillingResult billingResult, @Nullable List<Purchase> list) {
                        //
                    }
                })
                .build();

        billingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(BillingResult billingResult) {
                int response = billingResult.getResponseCode();
                if (response == BillingClient.BillingResponseCode.OK) {
                    setupSkuList();
                }
            }
            @Override
            public void onBillingServiceDisconnected() {
                // Try to restart the connection on the next request to
                // Google Play by calling the startConnection() method.
            }
        });
    }

    private void setupSkuList() {
        List<String> skuList = new ArrayList<>();
        skuList.add("rubies.001");

        SkuDetailsParams params = SkuDetailsParams.newBuilder()
                .setSkusList(skuList)
                .setType(BillingClient.SkuType.INAPP)
                .build();

        billingClient.querySkuDetailsAsync(params,
                new SkuDetailsResponseListener() {
                    @Override
                    public void onSkuDetailsResponse(BillingResult billingResult, List<SkuDetails> skuDetailsList) {
                        int response = billingResult.getResponseCode();
                        if (response == BillingClient.BillingResponseCode.OK) {


                            for (SkuDetails skuDetails : skuDetailsList) {
                                String sku = skuDetails.getSku();
                                String price = skuDetails.getPrice();

                                launchBilling(skuDetails);

                                break;
                            }




                        }
                    }
                });
    }

    private void launchBilling(SkuDetails skuDetails) {
        BillingFlowParams billingFlowParams = BillingFlowParams.newBuilder()
                .setSkuDetails(skuDetails)
                .build();

        billingClient.launchBillingFlow(this, billingFlowParams);
    }*/

}
