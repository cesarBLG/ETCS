package com.etcs.dmi;

import org.libsdl.app.SDL;

public class EVC implements Runnable {
    @Override
    public void run() {
        SDL.loadLibrary("evc");
        evcMain(DMI.singleton.getExternalFilesDir(null).getAbsolutePath());
        if (DMI.singleton != null && !DMI.singleton.isFinishing()) {
            DMI.singleton.evc = null;
            DMI.singleton.evcthread = null;
            DMI.singleton.DMIstop();
            DMI.singleton.finish();
        }
    }
    public native void evcMain(String filesDir);
    public native void evcStop();
}