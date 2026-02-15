package com.vectras.vm.qemu;

public final class ExecutionBudgetPolicy {

    private static final int THROUGHPUT_MIN_CPUS = 10;
    private static final int THROUGHPUT_MAX_CPUS = 23;

    private ExecutionBudgetPolicy() {
        throw new AssertionError("ExecutionBudgetPolicy is a utility class and cannot be instantiated");
    }

    public static CpuBudget forProfile(VmProfile profile) {
        return forProfile(profile, Runtime.getRuntime().availableProcessors());
    }

    static CpuBudget forProfile(VmProfile profile, int availableProcessors) {
        int safeAvailableProcessors = Math.max(1, availableProcessors);
        if (profile == VmProfile.THROUGHPUT) {
            int requestedCpus = safeAvailableProcessors - 1;
            int clampedCpus = clamp(requestedCpus, THROUGHPUT_MIN_CPUS, THROUGHPUT_MAX_CPUS);
            return CpuBudget.withCpuTopology(clampedCpus);
        }
        return CpuBudget.withoutCpuTopology();
    }

    private static int clamp(int value, int min, int max) {
        if (value < min) {
            return min;
        }
        if (value > max) {
            return max;
        }
        return value;
    }

    public static final class CpuBudget {
        private final boolean hasCpuTopology;
        private final int cpus;

        private CpuBudget(boolean hasCpuTopology, int cpus) {
            this.hasCpuTopology = hasCpuTopology;
            this.cpus = cpus;
        }

        static CpuBudget withCpuTopology(int cpus) {
            return new CpuBudget(true, cpus);
        }

        static CpuBudget withoutCpuTopology() {
            return new CpuBudget(false, 0);
        }

        public boolean hasCpuTopology() {
            return hasCpuTopology;
        }

        public int cpus() {
            return cpus;
        }
    }
}
