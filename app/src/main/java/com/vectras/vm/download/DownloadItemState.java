package com.vectras.vm.download;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class DownloadItemState {

    @NonNull
    public String id;
    @NonNull
    public String url;
    @Nullable
    public String partialPath;
    @Nullable
    public String finalPath;
    public long bytesDownloaded;
    public long totalBytes;
    @Nullable
    public String expectedSha256;
    @NonNull
    public String status;
    public long updatedAt;

    public DownloadItemState(@NonNull String id,
                             @NonNull String url,
                             @Nullable String partialPath,
                             @Nullable String finalPath,
                             long bytesDownloaded,
                             long totalBytes,
                             @Nullable String expectedSha256,
                             @NonNull String status,
                             long updatedAt) {
        this.id = id;
        this.url = url;
        this.partialPath = partialPath;
        this.finalPath = finalPath;
        this.bytesDownloaded = bytesDownloaded;
        this.totalBytes = totalBytes;
        this.expectedSha256 = expectedSha256;
        this.status = DownloadStatus.sanitize(status);
        this.updatedAt = updatedAt;
    }
}
