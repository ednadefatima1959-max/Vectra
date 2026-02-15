package com.vectras.vm.qemu;

import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

public class ExecutionBudgetPolicyTest {

    @Test
    public void throughputBudget_ClampsMinimumTo10() {
        ExecutionBudgetPolicy.CpuBudget budget = ExecutionBudgetPolicy.forProfile(VmProfile.THROUGHPUT, 4);

        assertTrue(budget.hasCpuTopology());
        assertEquals(10, budget.cpus());
    }

    @Test
    public void throughputBudget_ClampsMaximumTo23() {
        ExecutionBudgetPolicy.CpuBudget budget = ExecutionBudgetPolicy.forProfile(VmProfile.THROUGHPUT, 64);

        assertTrue(budget.hasCpuTopology());
        assertEquals(23, budget.cpus());
    }

    @Test
    public void throughputBudget_UsesAvailableProcessorsMinusOneInsideRange() {
        ExecutionBudgetPolicy.CpuBudget budget = ExecutionBudgetPolicy.forProfile(VmProfile.THROUGHPUT, 18);

        assertTrue(budget.hasCpuTopology());
        assertEquals(17, budget.cpus());
    }

    @Test
    public void nonThroughputProfiles_DoNotDeclareCpuTopology() {
        ExecutionBudgetPolicy.CpuBudget budget = ExecutionBudgetPolicy.forProfile(VmProfile.BALANCED, 18);

        assertFalse(budget.hasCpuTopology());
        assertEquals(0, budget.cpus());
    }
}
