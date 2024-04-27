package com.etcs.dmi;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import androidx.annotation.Nullable;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import org.libsdl.app.SDL;

public class EVC extends Service {

    protected static Thread evcThread;

    public interface EVCCallBack
    {
        void onEVCstopped();
    }
    EVCCallBack callBack;

    public class EVCBinder extends Binder
    {
        public EVC evc = EVC.this;
    }
    EVCBinder binder = new EVCBinder();

    public EVC() {
    }

    @Override
    public void onCreate() {
        super.onCreate();
        evcThread = new Thread(new Runnable() {
            @Override
            public void run() {
                SDL.loadLibrary("evc");
                evcMain(getExternalFilesDir(null).getAbsolutePath());
            }
        });
        evcThread.start();
        setupNotificationChannel();
    }

    private void setupNotificationChannel() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O) return;

        String channelName = "ETCS";
        String channelDescription = "European Vital Computer status";
        int importance = NotificationManager.IMPORTANCE_LOW;

        NotificationChannel channel = new NotificationChannel("ETCS", channelName,importance);
        channel.setDescription(channelDescription);
        NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        manager.createNotificationChannel(channel);
    }

    private Notification buildNotification() {
        String contentText = "ETCS EVC running";

        Notification.Builder builder = new Notification.Builder(this);
        builder.setContentTitle("ETCS");
        builder.setContentText(contentText);
        builder.setSmallIcon(R.mipmap.ic_launcher);
        builder.setOngoing(true);

        builder.setPriority(Notification.PRIORITY_HIGH);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            builder.setChannelId("ETCS");
        }

        //Resources res = getResources();
        //Intent exitIntent = new Intent(this, EVC.class).setAction("com.etcs.evc_stop");
        //builder.addAction(android.R.drawable.ic_delete, "Exit", PendingIntent.getService(this, 0, exitIntent, 0));
        //builder.addAction(android.R.drawable.ic_delete, "Exit", PendingIntent.getService(this, ))

        return builder.build();
    }
    public native void evcMain(String filesDir);
    public native void evcStop();

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if ("com.etcs.evc_stop".equals(intent.getAction()))
        {
            ((NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE)).cancel(1234);
            evcStop();
            stopSelf();
            if (callBack!=null) callBack.onEVCstopped();
        }
        else
        {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                startForeground(1234, buildNotification(), ServiceInfo.FOREGROUND_SERVICE_TYPE_SPECIAL_USE);
            } else {
                startForeground(1234, buildNotification());
            }
        }
        return Service.START_NOT_STICKY;
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return binder;
    }
    @Override
    public boolean onUnbind(Intent intent) {
        ((NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE)).cancel(1234);
        evcStop();
        stopSelf();
        if (callBack!=null) callBack.onEVCstopped();
        return false;
    }
}