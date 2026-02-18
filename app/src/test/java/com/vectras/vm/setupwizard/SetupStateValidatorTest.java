package com.vectras.vm.setupwizard;

import org.junit.Assert;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

public class SetupStateValidatorTest {

    @Test
    public void validStateJsonPassesSchemaValidation() {
        String json = "{\"version\":1,\"timestamp\":\"1739393939\",\"phase\":\"PREPARE\",\"stage_dir\":\"/root/.vectras-staging/1739393939\",\"message\":\"ok\"}";
        Assert.assertTrue(SetupStateValidator.isValidStateJson(json));
    }

    @Test
    public void invalidVersionFailsSchemaValidation() {
        String json = "{\"version\":2,\"timestamp\":\"1739393939\",\"phase\":\"PREPARE\",\"stage_dir\":\"/root/.vectras-staging/1739393939\",\"message\":\"ok\"}";
        Assert.assertFalse(SetupStateValidator.isValidStateJson(json));
    }

    @Test
    public void invalidStageDirFailsSchemaValidation() {
        Map<String, Object> state = new HashMap<>();
        state.put("version", 1);
        state.put("timestamp", "1739393939");
        state.put("phase", SetupStateValidator.PHASE_PREPARE);
        state.put("stage_dir", "/tmp/not-allowed");
        state.put("message", "bad");

        Assert.assertFalse(SetupStateValidator.isValidStateMap(state));
    }

    @Test
    public void failureTransitionPrepareToRolledBackIsAllowed() {
        Assert.assertTrue(SetupStateValidator.isValidTransition(
                SetupStateValidator.PHASE_PREPARE,
                SetupStateValidator.PHASE_ROLLED_BACK
        ));
    }

    @Test
    public void failureTransitionStageOkToRolledBackIsAllowed() {
        Assert.assertTrue(SetupStateValidator.isValidTransition(
                SetupStateValidator.PHASE_STAGE_OK,
                SetupStateValidator.PHASE_ROLLED_BACK
        ));
    }

    @Test
    public void invalidTransitionPromotedToRolledBackIsRejected() {
        Assert.assertFalse(SetupStateValidator.isValidTransition(
                SetupStateValidator.PHASE_PROMOTED,
                SetupStateValidator.PHASE_ROLLED_BACK
        ));
    }
}
