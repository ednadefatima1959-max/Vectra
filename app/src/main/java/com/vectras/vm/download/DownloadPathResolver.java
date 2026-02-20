package com.vectras.vm.download;

import android.content.Context;
import android.os.Environment;

import androidx.annotation.NonNull;

import java.io.File;
import java.util.Locale;
import java.util.regex.Pattern;

public final class DownloadPathResolver {

    private static final Pattern INVALID_FILENAME_CHARS = Pattern.compile("[\\\\/:*?\"<>|\\p{Cntrl}]");

    private DownloadPathResolver() {
    }

    @NonNull
    public static ResolvedPaths resolve(@NonNull Context context, @NonNull String requestedName) {
        File baseDir = context.getExternalFilesDir(Environment.DIRECTORY_DOWNLOADS);
        if (baseDir == null) {
            baseDir = context.getFilesDir();
        }

        String safeFileName = sanitizeName(requestedName);
        File finalFile = new File(baseDir, safeFileName);
        File partialFile = new File(baseDir, safeFileName + ".part");
        return new ResolvedPaths(baseDir, safeFileName, finalFile, partialFile);
    }

    @NonNull
    public static String sanitizeName(@NonNull String rawName) {
        String input = rawName.trim();
        String preservedExtension = "";
        String lower = input.toLowerCase(Locale.US);

        if (lower.endsWith(".cvbi.zip")) {
            preservedExtension = ".cvbi.zip";
            input = input.substring(0, input.length() - preservedExtension.length());
        } else if (lower.endsWith(".cvbi")) {
            preservedExtension = ".cvbi";
            input = input.substring(0, input.length() - preservedExtension.length());
        }

        String sanitizedBase = INVALID_FILENAME_CHARS.matcher(input)
                .replaceAll("_")
                .replace('/', '_')
                .replace('\\', '_')
                .trim();
        sanitizedBase = trimDotsAndSpaces(sanitizedBase);

        if (sanitizedBase.isEmpty() || isReservedName(sanitizedBase)) {
            sanitizedBase = "download";
        }

        return sanitizedBase + preservedExtension;
    }

    @NonNull
    private static String trimDotsAndSpaces(@NonNull String value) {
        int start = 0;
        int end = value.length();

        while (start < end) {
            char c = value.charAt(start);
            if (c == ' ' || c == '.') {
                start++;
            } else {
                break;
            }
        }

        while (end > start) {
            char c = value.charAt(end - 1);
            if (c == ' ' || c == '.') {
                end--;
            } else {
                break;
            }
        }

        return value.substring(start, end);
    }

    private static boolean isReservedName(@NonNull String value) {
        String upper = value.toUpperCase(Locale.US);
        if (".".equals(upper) || "..".equals(upper)) {
            return true;
        }

        if ("CON".equals(upper) || "PRN".equals(upper) || "AUX".equals(upper) || "NUL".equals(upper)) {
            return true;
        }

        if (upper.matches("COM[1-9]") || upper.matches("LPT[1-9]")) {
            return true;
        }

        return false;
    }

    public static final class ResolvedPaths {
        @NonNull
        public final File baseDir;
        @NonNull
        public final String safeFileName;
        @NonNull
        public final File finalFile;
        @NonNull
        public final File partialFile;

        ResolvedPaths(@NonNull File baseDir,
                      @NonNull String safeFileName,
                      @NonNull File finalFile,
                      @NonNull File partialFile) {
            this.baseDir = baseDir;
            this.safeFileName = safeFileName;
            this.finalFile = finalFile;
            this.partialFile = partialFile;
        }
    }
}
