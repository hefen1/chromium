// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.device.battery;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Build;
import android.util.Log;

import org.chromium.base.VisibleForTesting;
import org.chromium.mojom.device.BatteryStatus;

/**
 * Data source for battery status information. This class registers for battery status notifications
 * from the system and calls the callback passed on construction whenever a notification is
 * received.
 */
class BatteryStatusManager {

    private static final String TAG = "BatteryStatusManager";

    interface BatteryStatusCallback {
        void onBatteryStatusChanged(BatteryStatus batteryStatus);
    }

    // A reference to the application context in order to acquire the SensorService.
    private final Context mAppContext;
    private final BatteryStatusCallback mCallback;
    private final IntentFilter mFilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            BatteryStatusManager.this.onReceive(intent);
        }
    };

    // This is to workaround a Galaxy Nexus bug, see the comment in the constructor.
    private final boolean mIgnoreBatteryPresentState;

    // Only used in L (API level 21 and higher).
    private BatteryManager mLollipopBatteryManager;

    private boolean mEnabled = false;

    private BatteryStatusManager(
            Context context, BatteryStatusCallback callback, boolean ignoreBatteryPresentState,
            BatteryManager batteryManager) {
        mAppContext = context.getApplicationContext();
        mCallback = callback;
        mIgnoreBatteryPresentState = ignoreBatteryPresentState;
        mLollipopBatteryManager = batteryManager;
    }

    BatteryStatusManager(Context context, BatteryStatusCallback callback) {
        // BatteryManager.EXTRA_PRESENT appears to be unreliable on Galaxy Nexus,
        // Android 4.2.1, it always reports false. See http://crbug.com/384348.
        this(context, callback, Build.MODEL.equals("Galaxy Nexus"),
             Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP ? new BatteryManager()
                                                                   : null);
    }

    /**
     * Creates a BatteryStatusManager without the Galaxy Nexus workaround for consistency in
     * testing.
     */
    static BatteryStatusManager createBatteryStatusManagerForTesting(
            Context context,
            BatteryStatusCallback callback,
            BatteryManager batteryManager) {
        return new BatteryStatusManager(context, callback, false, batteryManager);
    }

    /**
     * Starts listening for intents.
     * @return True on success.
     */
    boolean start() {
        if (!mEnabled && mAppContext.registerReceiver(mReceiver, mFilter) != null) {
            // success
            mEnabled = true;
        }
        return mEnabled;
    }

    /**
     * Stops listening to intents.
     */
    void stop() {
        if (mEnabled) {
            mAppContext.unregisterReceiver(mReceiver);
            mEnabled = false;
        }
    }

    @VisibleForTesting
    void onReceive(Intent intent) {
        if (!intent.getAction().equals(Intent.ACTION_BATTERY_CHANGED)) {
            Log.e(TAG, "Unexpected intent.");
            return;
        }

        boolean present = mIgnoreBatteryPresentState
                ? true
                : intent.getBooleanExtra(BatteryManager.EXTRA_PRESENT, false);
        int pluggedStatus = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);

        if (!present || pluggedStatus == -1) {
            // No battery or no plugged status: return default values.
            mCallback.onBatteryStatusChanged(new BatteryStatus());
            return;
        }

        int current = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
        int max = intent.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
        double level = (double) current / (double) max;
        if (level < 0 || level > 1) {
            // Sanity check, assume default value in this case.
            level = 1.0;
        }

        // Currently Android (below L) does not provide charging/discharging time, as a work-around
        // we could compute it manually based on the evolution of level delta.
        // TODO(timvolodine): add proper projection for chargingTime, dischargingTime
        // (see crbug.com/401553).
        boolean charging = pluggedStatus != 0;
        int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
        boolean batteryFull = status == BatteryManager.BATTERY_STATUS_FULL;
        double chargingTimeSeconds = (charging && batteryFull) ? 0 : Double.POSITIVE_INFINITY;
        double dischargingTimeSeconds = Double.POSITIVE_INFINITY;

        if (mLollipopBatteryManager != null) {
            // On Lollipop we can provide a better estimate for chargingTime and dischargingTime.
            double remainingCapacityRatio = mLollipopBatteryManager.getIntProperty(
                    BatteryManager.BATTERY_PROPERTY_CAPACITY) / 100.0;
            double batteryCapacityMicroAh = mLollipopBatteryManager.getIntProperty(
                    BatteryManager.BATTERY_PROPERTY_CHARGE_COUNTER);
            double averageCurrentMicroA = mLollipopBatteryManager.getIntProperty(
                    BatteryManager.BATTERY_PROPERTY_CURRENT_AVERAGE);

            if (charging) {
                if (chargingTimeSeconds == Double.POSITIVE_INFINITY && averageCurrentMicroA > 0) {
                    double chargeFromEmptyHours = batteryCapacityMicroAh / averageCurrentMicroA;
                    chargingTimeSeconds =
                            Math.ceil((1 - remainingCapacityRatio) * chargeFromEmptyHours * 3600.0);
                }
            } else {
                if (averageCurrentMicroA < 0) {
                    double dischargeFromFullHours = batteryCapacityMicroAh / -averageCurrentMicroA;
                    dischargingTimeSeconds =
                            Math.floor(remainingCapacityRatio * dischargeFromFullHours * 3600.0);
                }
            }
        }

        BatteryStatus batteryStatus = new BatteryStatus();
        batteryStatus.charging = charging;
        batteryStatus.chargingTime = chargingTimeSeconds;
        batteryStatus.dischargingTime = dischargingTimeSeconds;
        batteryStatus.level = level;
        mCallback.onBatteryStatusChanged(batteryStatus);
    }
}
