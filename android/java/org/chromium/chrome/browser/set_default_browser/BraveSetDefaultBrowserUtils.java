/**
 * Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.chromium.chrome.browser.set_default_browser;

import android.app.Activity;
import android.app.role.RoleManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.appcompat.app.AppCompatActivity;

import org.chromium.base.ContextUtils;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.app.BraveActivity;
import org.chromium.chrome.browser.onboarding.OnboardingPrefManager;
import org.chromium.chrome.browser.preferences.BravePreferenceKeys;
import org.chromium.chrome.browser.preferences.SharedPreferencesManager;
import org.chromium.chrome.browser.set_default_browser.OnBraveSetDefaultBrowserListener;
import org.chromium.chrome.browser.set_default_browser.SetDefaultBrowserActivity;
import org.chromium.chrome.browser.set_default_browser.SetDefaultBrowserBottomSheetFragment;
import org.chromium.components.embedder_support.util.UrlConstants;
import org.chromium.ui.widget.Toast;

public class BraveSetDefaultBrowserUtils {
    public static final int DEFAULT_BROWSER_ROLE_REQUEST_CODE = 36;
    public static final String ANDROID_SETUPWIZARD_PACKAGE_NAME = "com.google.android.setupwizard";
    public static final String ANDROID_PACKAGE_NAME = "android";
    public static final String BRAVE_BLOG_URL = "https://brave.com/privacy-features/";

    public static boolean isBottomSheetVisible;

    public static boolean isBraveSetAsDefaultBrowser(Context context) {
        Intent browserIntent =
                new Intent(Intent.ACTION_VIEW, Uri.parse(UrlConstants.HTTP_URL_PREFIX));
        ResolveInfo resolveInfo = context.getPackageManager().resolveActivity(
                browserIntent, supportsDefault() ? PackageManager.MATCH_DEFAULT_ONLY : 0);
        if (resolveInfo == null || resolveInfo.activityInfo == null
                || resolveInfo.activityInfo.packageName == null
                || ContextUtils.getApplicationContext() == null) {
            return false;
        }

        return resolveInfo.activityInfo.packageName.equals(
                       BraveActivity.BRAVE_PRODUCTION_PACKAGE_NAME)
                || resolveInfo.activityInfo.packageName.equals(
                        BraveActivity.BRAVE_BETA_PACKAGE_NAME)
                || resolveInfo.activityInfo.packageName.equals(
                        BraveActivity.BRAVE_NIGHTLY_PACKAGE_NAME);
    }

    public static void checkSetDefaultBrowserModal(AppCompatActivity activity) {
        if (!isBraveSetAsDefaultBrowser(activity) && !isBraveDefaultDontAsk()) {
            boolean shouldShowDefaultBrowserModalAfterP3A =
                    OnboardingPrefManager.getInstance().shouldShowDefaultBrowserModalAfterP3A();

            if (shouldShowDefaultBrowserModalAfterP3A) {
                Intent setDefaultBrowserIntent =
                        new Intent(activity, SetDefaultBrowserActivity.class);
                setDefaultBrowserIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
                activity.startActivity(setDefaultBrowserIntent);

                OnboardingPrefManager.getInstance().setShowDefaultBrowserModalAfterP3A(false);

            } else if (SharedPreferencesManager.getInstance().readInt(
                               BravePreferenceKeys.BRAVE_APP_OPEN_COUNT)
                    == 5) {
                showBraveSetDefaultBrowserDialog(activity);

            } else if (shouldShowBraveWasDefaultDialog()) {
                int braveWasDefaultCount = SharedPreferencesManager.getInstance().readInt(
                        BravePreferenceKeys.BRAVE_WAS_DEFAULT_ASK_COUNT);
                SharedPreferencesManager.getInstance().writeInt(
                        BravePreferenceKeys.BRAVE_WAS_DEFAULT_ASK_COUNT, braveWasDefaultCount + 1);
                showBraveSetDefaultBrowserDialog(activity);
            }

        } else if (isBraveSetAsDefaultBrowser(activity) && !wasBraveDefaultBefore()) {
            setBraveDefaultSuccess();
        }
    }

    public static void showBraveSetDefaultBrowserDialog(AppCompatActivity activity) {
        /* (Albert Wang): Default app settings didn't get added until API 24
         * https://developer.android.com/reference/android/provider/Settings#ACTION_MANAGE_DEFAULT_APPS_SETTINGS
         */
        if (isBraveSetAsDefaultBrowser(activity)) {
            Toast toast = Toast.makeText(
                    activity, R.string.brave_already_set_as_default_browser, Toast.LENGTH_LONG);
            toast.show();
            return;
        }

        int roleManagerOpenCount = SharedPreferencesManager.getInstance().readInt(
                BravePreferenceKeys.BRAVE_ROLE_MANAGER_DIALOG_COUNT);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q && roleManagerOpenCount < 2) {
            RoleManager roleManager = activity.getSystemService(RoleManager.class);

            if (roleManager.isRoleAvailable(RoleManager.ROLE_BROWSER)) {
                if (!roleManager.isRoleHeld(RoleManager.ROLE_BROWSER)) {
                    // save role manager open count as the second times onwards the dialog is shown,
                    // the system allows the user to click on "don't show again".
                    SharedPreferencesManager.getInstance().writeInt(
                            BravePreferenceKeys.BRAVE_ROLE_MANAGER_DIALOG_COUNT,
                            roleManagerOpenCount + 1);
                    activity.startActivityForResult(
                            roleManager.createRequestRoleIntent(RoleManager.ROLE_BROWSER),
                            DEFAULT_BROWSER_ROLE_REQUEST_CODE);
                }
            } else {
                showSetDefaultBottomSheet(activity);
            }

        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            showSetDefaultBottomSheet(activity);

        } else {
            ResolveInfo resolveInfo = getResolveInfo(activity);
            if (resolveInfo.activityInfo.packageName.equals(ANDROID_SETUPWIZARD_PACKAGE_NAME)
                    || resolveInfo.activityInfo.packageName.equals(ANDROID_PACKAGE_NAME)) {
                // (Albert Wang): From what I've experimented on 6.0,
                // default browser popup is in the middle of the screen for
                // these versions. So we shouldn't show the toast.
                openBraveBlog(activity);
            } else {
                Toast toast = Toast.makeText(
                        activity, R.string.brave_default_browser_go_to_settings, Toast.LENGTH_LONG);
                toast.show();
            }
        }
    }

    private static boolean supportsDefault() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.N;
    }

    private static ResolveInfo getResolveInfo(Activity activity) {
        Intent browserIntent =
                new Intent(Intent.ACTION_VIEW, Uri.parse(UrlConstants.HTTP_URL_PREFIX));

        return activity.getPackageManager().resolveActivity(
                browserIntent, supportsDefault() ? PackageManager.MATCH_DEFAULT_ONLY : 0);
    }

    private static void showSetDefaultBottomSheet(AppCompatActivity activity) {
        if (!isBottomSheetVisible) {
            isBottomSheetVisible = true;
            SetDefaultBrowserBottomSheetFragment bottomSheetDialog =
                    SetDefaultBrowserBottomSheetFragment.newInstance();

            int braveDefaultModalCount = SharedPreferencesManager.getInstance().readInt(
                    BravePreferenceKeys.BRAVE_SET_DEFAULT_BOTTOM_SHEET_COUNT);
            SharedPreferencesManager.getInstance().writeInt(
                    BravePreferenceKeys.BRAVE_SET_DEFAULT_BOTTOM_SHEET_COUNT,
                    braveDefaultModalCount + 1);

            bottomSheetDialog.show(
                    activity.getSupportFragmentManager(), "SetDefaultBrowserBottomSheetFragment");
        }
    }

    public static void openDefaultAppsSettings(Activity activity) {
        if (activity instanceof OnBraveSetDefaultBrowserListener) {
            ((OnBraveSetDefaultBrowserListener) activity).OnCheckDefaultResume();
        }

        Intent intent = new Intent(Settings.ACTION_MANAGE_DEFAULT_APPS_SETTINGS);
        // intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        activity.startActivity(intent);
    }

    private static void openBraveBlog(Activity activity) {
        if (activity instanceof OnBraveSetDefaultBrowserListener) {
            ((OnBraveSetDefaultBrowserListener) activity).OnCheckDefaultResume();
        }

        LayoutInflater inflater = activity.getLayoutInflater();
        View layout = inflater.inflate(R.layout.brave_set_default_browser_dialog,
                (ViewGroup) activity.findViewById(R.id.brave_set_default_browser_toast_container));

        Toast toast = new Toast(activity, layout);
        toast.setDuration(Toast.LENGTH_LONG);
        toast.setGravity(Gravity.TOP, 0, 40);
        toast.show();

        Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse(BRAVE_BLOG_URL));
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        activity.startActivity(intent);
    }

    private static boolean wasBraveDefaultBefore() {
        return SharedPreferencesManager.getInstance().readBoolean(
                BravePreferenceKeys.BRAVE_IS_DEFAULT, false);
    }

    private static boolean shouldShowBraveWasDefaultDialog() {
        int braveWasDefaultCount = SharedPreferencesManager.getInstance().readInt(
                BravePreferenceKeys.BRAVE_WAS_DEFAULT_ASK_COUNT);
        return braveWasDefaultCount < 2 && wasBraveDefaultBefore();
    }

    public static void setBraveDefaultSuccess() {
        SharedPreferencesManager.getInstance().writeBoolean(
                BravePreferenceKeys.BRAVE_IS_DEFAULT, true);
    }

    public static void setBraveDefaultDontAsk() {
        SharedPreferencesManager.getInstance().writeBoolean(
                BravePreferenceKeys.BRAVE_DEFAULT_DONT_ASK, true);
    }

    public static boolean isBraveDefaultDontAsk() {
        return SharedPreferencesManager.getInstance().readBoolean(
                BravePreferenceKeys.BRAVE_DEFAULT_DONT_ASK, false);
    }
}