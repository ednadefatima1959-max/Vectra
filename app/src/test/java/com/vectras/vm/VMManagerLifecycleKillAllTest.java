package com.vectras.vm;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import com.vectras.vm.main.core.MainStartVM;
import com.vectras.vterm.Terminal;

import org.junit.After;
import org.junit.Test;

public class VMManagerLifecycleKillAllTest {

    @After
    public void tearDown() {
        MainStartVM.lastVMName = "";
        Terminal.qemuProcess = null;
        VMManager.killallqemuprocesses(null);
    }

    @Test
    public void killAllQemuProcesses_shouldResetResidualStartingStateAndAllowNewStart() {
        String vmId = "vm-residual-starting";

        assertTrue(VMManager.tryMarkVmStarting(vmId));
        assertFalse(VMManager.tryMarkVmStarting(vmId));

        MainStartVM.lastVMName = "";
        Terminal.qemuProcess = null;
        VMManager.killallqemuprocesses(null);

        assertTrue(VMManager.tryMarkVmStarting(vmId));
    }
}
