package com.vectras.vm.core;

import com.vectras.vm.VMManager;

import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Lightweight process-start budget registry used to reserve start slots before spawning subprocesses.
 */
public final class ProcessBudgetRegistry {
    private static final Object LOCK = new Object();
    private static final ConcurrentHashMap<String, SlotToken> TOKENS = new ConcurrentHashMap<>();

    private ProcessBudgetRegistry() {
    }

    public static SlotToken tryAcquireSlot(String feature, String subFeature, String origin, String vmId) {
        synchronized (LOCK) {
            int active = VMManager.getActiveSupervisedVmProcessCount();
            int max = VMManager.getMaxSupervisedVmProcesses();
            int unboundReservations = 0;
            for (SlotToken token : TOKENS.values()) {
                if (!token.isReleased() && !token.isBound()) {
                    unboundReservations++;
                }
            }
            if (active + unboundReservations >= max) {
                return null;
            }
            SlotToken token = new SlotToken(feature, subFeature, origin, vmId);
            TOKENS.put(token.id, token);
            return token;
        }
    }

    public static void bindProcess(SlotToken token, Process process) {
        if (token == null || process == null || token.isReleased()) {
            return;
        }
        token.boundProcess = process;
        token.bound.set(true);
    }

    public static void releaseSlot(SlotToken token, String reason) {
        if (token == null) {
            return;
        }
        if (token.released.compareAndSet(false, true)) {
            token.releaseReason = reason;
            TOKENS.remove(token.id);
        }
    }

    public static final class SlotToken {
        private final String id;
        public final String feature;
        public final String subFeature;
        public final String origin;
        public final String vmId;
        private final AtomicBoolean bound = new AtomicBoolean(false);
        private final AtomicBoolean released = new AtomicBoolean(false);
        private volatile Process boundProcess;
        private volatile String releaseReason;

        private SlotToken(String feature, String subFeature, String origin, String vmId) {
            this.id = UUID.randomUUID().toString();
            this.feature = feature;
            this.subFeature = subFeature;
            this.origin = origin;
            this.vmId = vmId;
        }

        private boolean isBound() {
            return bound.get();
        }

        private boolean isReleased() {
            return released.get();
        }
    }
}

