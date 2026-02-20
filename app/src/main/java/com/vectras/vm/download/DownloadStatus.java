package com.vectras.vm.download;

import androidx.annotation.NonNull;

public final class DownloadStatus {

    public static final String QUEUED = "QUEUED";
    public static final String RUNNING = "RUNNING";
    public static final String PAUSED = "PAUSED";
    public static final String FAILED = "FAILED";
    public static final String CANCELED = "CANCELED";
    public static final String COMPLETED = "COMPLETED";
    public static final String HASH_MISMATCH = "HASH_MISMATCH";

    private DownloadStatus() {
    }

    @NonNull
    public static String sanitize(@NonNull String value) {
        switch (value) {
            case QUEUED:
            case RUNNING:
            case PAUSED:
            case FAILED:
            case CANCELED:
            case COMPLETED:
            case HASH_MISMATCH:
                return value;
            default:
                return FAILED;
        }
    }
}
