package com.vectras.vm.setupwizard;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import java.lang.reflect.Type;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public final class SetupStateValidator {

    public static final String PHASE_PREPARE = "PREPARE";
    public static final String PHASE_STAGE_OK = "STAGE_OK";
    public static final String PHASE_PROMOTED = "PROMOTED";
    public static final String PHASE_ROLLED_BACK = "ROLLED_BACK";

    private static final Set<String> SUPPORTED_PHASES = new HashSet<>(Arrays.asList(
            PHASE_PREPARE,
            PHASE_STAGE_OK,
            PHASE_PROMOTED,
            PHASE_ROLLED_BACK
    ));

    private SetupStateValidator() {
    }

    public static boolean isValidStateJson(String json) {
        if (json == null || json.trim().isEmpty()) {
            return false;
        }

        try {
            Type mapType = new TypeToken<Map<String, Object>>() { }.getType();
            Map<String, Object> state = new Gson().fromJson(json, mapType);
            return isValidStateMap(state);
        } catch (Exception ignored) {
            return false;
        }
    }

    public static boolean isValidStateMap(Map<String, Object> state) {
        if (state == null) {
            return false;
        }

        Object version = state.get("version");
        if (!(version instanceof Number) || ((Number) version).intValue() != 1) {
            return false;
        }

        Object timestamp = state.get("timestamp");
        if (!(timestamp instanceof String) || ((String) timestamp).trim().isEmpty()) {
            return false;
        }

        Object phase = state.get("phase");
        if (!(phase instanceof String) || !SUPPORTED_PHASES.contains(phase)) {
            return false;
        }

        Object stageDir = state.get("stage_dir");
        if (!(stageDir instanceof String) || !((String) stageDir).startsWith("/root/.vectras-staging/")) {
            return false;
        }

        Object message = state.get("message");
        return message instanceof String;
    }

    public static boolean isValidTransition(String fromPhase, String toPhase) {
        if (!SUPPORTED_PHASES.contains(fromPhase) || !SUPPORTED_PHASES.contains(toPhase)) {
            return false;
        }

        if (PHASE_PREPARE.equals(fromPhase)) {
            return PHASE_STAGE_OK.equals(toPhase) || PHASE_ROLLED_BACK.equals(toPhase);
        }

        if (PHASE_STAGE_OK.equals(fromPhase)) {
            return PHASE_PROMOTED.equals(toPhase) || PHASE_ROLLED_BACK.equals(toPhase);
        }

        if (PHASE_PROMOTED.equals(fromPhase)) {
            return false;
        }

        return false;
    }
}
