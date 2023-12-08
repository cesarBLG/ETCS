package com.etcs.dmi;

import android.app.NotificationManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.os.IBinder;
import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationManagerCompat;
import androidx.core.content.ContextCompat;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * A sample wrapper class that just calls SDLActivity
 */

public class DMI extends SDLActivity implements ServiceConnection
{
    native void DMIstop();

    EVC evc;
    boolean forceReplaceFiles=false;

    @Override
    protected void onDestroy() {
        DMIstop();

        if (evc != null)
        {
            ((NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE)).cancel(1234);
            evc.callBack = null;
            evc.evcStop();
            evc.stopSelf();
        }
        unbindService(this);

        super.onDestroy();

        finishAffinity();

        System.exit(0);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        /*if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED)
        {
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
        }*/
        try {
            SharedPreferences pref = getPreferences(Context.MODE_PRIVATE);
            int oldver = pref.getInt("ver", 0);
            PackageInfo packageInfo = getPackageManager().getPackageInfo(getPackageName(), 0);
            int versionCode = packageInfo.versionCode;
            SharedPreferences.Editor editor = pref.edit();
            editor.putInt("ver", versionCode);
            editor.apply();
            if (oldver != versionCode) forceReplaceFiles = true;
        } catch (PackageManager.NameNotFoundException e) {
        }
        copyFileOrDir("");
        super.onCreate(savedInstanceState);

        Intent serviceIntent = new Intent(this, EVC.class);
        startService(serviceIntent);
        if (!bindService(serviceIntent, this, 0))
            throw new RuntimeException("Error binding service");
    }

    private void copyFileOrDir(String path) {
        AssetManager assetManager = this.getAssets();
        String assets[] = null;
        try {
            assets = assetManager.list(path);
            if (assets.length == 0) {
                copyFile(path);
            } else {
                String fullPath = getExternalFilesDir(null) + "/" + path;
                File dir = new File(fullPath);
                if (!dir.exists())
                    dir.mkdir();
                for (int i = 0; i < assets.length; ++i) {
                    if (path.isEmpty()) copyFileOrDir(assets[i]);
                    else copyFileOrDir(path + "/" + assets[i]);
                }
            }
        } catch (IOException ex) {
            ex.printStackTrace();
        }
    }

    private void copyFile(String filename) {
        AssetManager assetManager = this.getAssets();

        InputStream in = null;
        OutputStream out = null;
        try {
            String newFileName = getExternalFilesDir(null) + "/" + filename;
            if (new File(newFileName).exists() && !forceReplaceFiles) return;
            in = assetManager.open(filename);
            out = new FileOutputStream(newFileName);

            byte[] buffer = new byte[1024];
            int read;
            while ((read = in.read(buffer)) != -1) {
                out.write(buffer, 0, read);
            }
            in.close();
            in = null;
            out.flush();
            out.close();
            out = null;
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    @Override
    public void onServiceConnected(ComponentName componentName, IBinder service) {
        evc = ((EVC.EVCBinder)service).evc;
        evc.callBack = new EVC.EVCCallBack() {
            @Override
            public void onEVCstopped() {
                finish();
            }
        };
    }

    @Override
    public void onServiceDisconnected(ComponentName componentName) {
    }
}