package com.vectras.vm.core;

import android.util.Log;

import com.vectras.vm.logger.VectrasStatus;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Registro local de orçamento de processos por feature/tag/caller.
 */
public final class ProcessBudgetRegistry {
    private static final String TAG = "ProcessBudgetRegistry";
    private static final int TOP_CONSUMERS_LIMIT = 5;

    private static final ConcurrentHashMap<String, AtomicInteger> COUNTERS = new ConcurrentHashMap<>();
    private static final ConcurrentHashMap<String, BudgetToken> TOKENS = new ConcurrentHashMap<>();
    private static final ConcurrentHashMap<String, String> TOKEN_BY_PROCESS_KEY = new ConcurrentHashMap<>();
    private static final AtomicLong TOKEN_SEQUENCE = new AtomicLong(1L);

    private ProcessBudgetRegistry() {
    }

    public static BudgetToken acquire(String feature, String tag, String caller, String vmId, long pid, int max) {
        String normalizedFeature = normalize(feature, "unknown_feature");
        String normalizedTag = normalize(tag, "unknown_tag");
        String normalizedCaller = normalize(caller, "unknown_caller");
        String normalizedVmId = normalize(vmId, "unknown");
        String key = compositeKey(normalizedFeature, normalizedTag, normalizedCaller);

        AtomicInteger counter = COUNTERS.computeIfAbsent(key, ignored -> new AtomicInteger(0));
        while (true) {
            int current = counter.get();
            if (current >= Math.max(1, max)) {
                logEvent(
                        "budget.acquire.deny",
                        normalizedFeature,
                        normalizedTag,
                        normalizedCaller,
                        normalizedVmId,
                        "",
                        pid,
                        current,
                        max
                );
                return null;
            }
            if (counter.compareAndSet(current, current + 1)) {
                int inUse = current + 1;
                String tokenId = "pbr-" + TOKEN_SEQUENCE.getAndIncrement();
                BudgetToken token = new BudgetToken(tokenId, key, normalizedFeature, normalizedTag, normalizedCaller, normalizedVmId, pid, max);
                TOKENS.put(tokenId, token);
                logEvent(
                        "budget.acquire.ok",
                        normalizedFeature,
                        normalizedTag,
                        normalizedCaller,
                        normalizedVmId,
                        tokenId,
                        pid,
                        inUse,
                        max
                );
                return token;
            }
        }
    }

    public static void bind(BudgetToken token, Process process, String vmId, long pid) {
        if (token == null) {
            return;
        }
        String normalizedVmId = normalize(vmId, token.vmId);
        long resolvedPid = pid > 0L ? pid : token.pid;
        token.vmId = normalizedVmId;
        token.pid = resolvedPid;

        if (process != null) {
            TOKEN_BY_PROCESS_KEY.put(processKey(process), token.tokenId);
        }

        AtomicInteger counter = COUNTERS.get(token.key);
        int inUse = counter == null ? 0 : counter.get();
        logEvent(
                "budget.bind.ok",
                token.feature,
                token.tag,
                token.caller,
                normalizedVmId,
                token.tokenId,
                resolvedPid,
                inUse,
                token.max
        );
    }

    public static void releaseByProcess(Process process, String vmId, long pid) {
        if (process == null) {
            logEvent("budget.release.noop", "vm_process", "register", "unknown_caller",
                    normalize(vmId, "unknown"), "", pid, 0, 0);
            return;
        }
        String tokenId = TOKEN_BY_PROCESS_KEY.remove(processKey(process));
        if (tokenId == null) {
            logEvent("budget.release.noop", "vm_process", "register", "unknown_caller",
                    normalize(vmId, "unknown"), "", pid, 0, 0);
            return;
        }
        releaseToken(tokenId, vmId, pid);
    }

    public static void release(BudgetToken token, String vmId, long pid) {
        if (token == null) {
            return;
        }
        TOKEN_BY_PROCESS_KEY.values().remove(token.tokenId);
        releaseToken(token.tokenId, vmId, pid);
    }

    private static void releaseToken(String tokenId, String vmId, long pid) {
        BudgetToken token = TOKENS.remove(tokenId);
        if (token == null) {
            logEvent("budget.release.noop", "vm_process", "register", "unknown_caller",
                    normalize(vmId, "unknown"), tokenId, pid, 0, 0);
            return;
        }

        AtomicInteger counter = COUNTERS.get(token.key);
        int inUse = 0;
        if (counter != null) {
            inUse = counter.decrementAndGet();
            if (inUse <= 0) {
                COUNTERS.remove(token.key, counter);
                inUse = Math.max(0, inUse);
            }
        }

        String normalizedVmId = normalize(vmId, token.vmId);
        long resolvedPid = pid > 0L ? pid : token.pid;
        logEvent(
                "budget.release.ok",
                token.feature,
                token.tag,
                token.caller,
                normalizedVmId,
                token.tokenId,
                resolvedPid,
                inUse,
                token.max
        );
    }

    public static String dumpState() {
        List<Map.Entry<String, AtomicInteger>> entries = new ArrayList<>(COUNTERS.entrySet());
        entries.sort(Comparator.comparingInt((Map.Entry<String, AtomicInteger> e) -> e.getValue().get()).reversed());

        StringBuilder out = new StringBuilder();
        out.append("ProcessBudgetRegistry{activeKeys=").append(entries.size())
                .append(", activeTokens=").append(TOKENS.size()).append("}");

        if (entries.isEmpty()) {
            out.append(" topConsumers=[]");
            return out.toString();
        }

        out.append(" topConsumers=[");
        int limit = Math.min(TOP_CONSUMERS_LIMIT, entries.size());
        for (int i = 0; i < limit; i++) {
            Map.Entry<String, AtomicInteger> entry = entries.get(i);
            if (i > 0) {
                out.append(", ");
            }
            out.append(entry.getKey()).append("=").append(entry.getValue().get());
        }
        out.append("]");
        return out.toString();
    }

    private static String compositeKey(String feature, String tag, String caller) {
        return feature + "|" + tag + "|" + caller;
    }

    private static String normalize(String value, String fallback) {
        if (value == null) {
            return fallback;
        }
        String trimmed = value.trim();
        return trimmed.isEmpty() ? fallback : trimmed;
    }

    private static String processKey(Process process) {
        return Integer.toHexString(System.identityHashCode(process));
    }

    private static void logEvent(String event,
                                 String feature,
                                 String tag,
                                 String caller,
                                 String vmId,
                                 String tokenId,
                                 long pid,
                                 int inUse,
                                 int max) {
        String line = String.format(Locale.US,
                "%s feature=%s tag=%s caller=%s vmId=%s tokenId=%s pid=%d inUse=%d max=%d",
                event,
                sanitize(feature),
                sanitize(tag),
                sanitize(caller),
                sanitize(vmId),
                sanitize(tokenId),
                pid,
                inUse,
                max);
        Log.i(TAG, line);
        VectrasStatus.logDebug(line);
    }

    private static String sanitize(String value) {
        if (value == null) {
            return "";
        }
        return value.replace(' ', '_');
    }

    public static final class BudgetToken {
        public final String tokenId;
        public final String key;
        public final String feature;
        public final String tag;
        public final String caller;
        public volatile String vmId;
        public volatile long pid;
        public final int max;

        private BudgetToken(String tokenId,
                            String key,
                            String feature,
                            String tag,
                            String caller,
                            String vmId,
                            long pid,
                            int max) {
            this.tokenId = tokenId;
            this.key = key;
            this.feature = feature;
            this.tag = tag;
            this.caller = caller;
            this.vmId = vmId;
            this.pid = pid;
            this.max = max;
        }
    }
}
