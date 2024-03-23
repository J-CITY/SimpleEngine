package com.oxrapp.glesapp;

import android.content.res.AssetManager;
import android.os.Bundle;

public class OXRAppMainActivity extends android.app.NativeActivity {
  static {
    System.loadLibrary("openxr_loader");
    System.loadLibrary("openxr_app");
  }

    private native void CreateObjectNative(AssetManager assetManager, String pathToInternalDir);
    private native void DeleteObjectNative();

    @Override
    public void onCreate(Bundle savedInstanceState) {
        AssetManager assetManager = getAssets();
        String pathToInternalDir = getFilesDir().getAbsolutePath();
        CreateObjectNative(assetManager, pathToInternalDir);
        super.onCreate(savedInstanceState);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        // We are exiting the activity, let's delete the native objects
        DeleteObjectNative();

    }
}
