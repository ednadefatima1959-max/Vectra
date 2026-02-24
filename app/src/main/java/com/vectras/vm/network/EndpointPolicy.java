package com.vectras.vm.network;

import android.net.Uri;

import java.util.Collections;
import java.util.EnumMap;
import java.util.LinkedHashSet;
import java.util.Map;
import java.util.Set;

public final class EndpointPolicy {

    public enum Feature {
        ROM_API,
        GITHUB_API,
        LANG_MODULES,
        EXTERNAL_ACTIONS
    }

    private static final Map<Feature, Set<String>> API_ALLOWLIST = new EnumMap<>(Feature.class);
    private static final Map<Feature, Set<String>> ACTION_VIEW_ALLOWLIST = new EnumMap<>(Feature.class);

    static {
        registerApi(Feature.ROM_API, "https://go.anbui.ovh/egg/");
        registerApi(Feature.GITHUB_API, "https://api.github.com/users/");
        registerApi(Feature.LANG_MODULES, "https://raw.githubusercontent.com/rafaelmeloreisnovo/Vectras-VM-Android/main/resources/lang/");

        registerActionView(Feature.GITHUB_API, "https://github.com/");
        registerActionView(Feature.EXTERNAL_ACTIONS, "https://github.com/termux/termux-app/releases");
        registerActionView(Feature.EXTERNAL_ACTIONS, "https://raw.githubusercontent.com/AnBui2004/termux/refs/heads/main/installpulseaudio.sh");
    }

    private EndpointPolicy() {
    }

    public static boolean isAllowedApi(Feature feature, String endpoint) {
        return isAllowed(feature, endpoint, API_ALLOWLIST);
    }

    public static boolean isAllowedActionView(Feature feature, String endpoint) {
        return isAllowed(feature, endpoint, ACTION_VIEW_ALLOWLIST);
    }

    public static String requireAllowedApi(Feature feature, String endpoint) {
        if (!isAllowedApi(feature, endpoint)) {
            throw new IllegalArgumentException("Endpoint blocked by API policy: " + endpoint);
        }
        return endpoint;
    }

    public static String requireAllowedActionView(Feature feature, String endpoint) {
        if (!isAllowedActionView(feature, endpoint)) {
            throw new IllegalArgumentException("Endpoint blocked by ACTION_VIEW policy: " + endpoint);
        }
        return endpoint;
    }

    private static void registerApi(Feature feature, String endpointPrefix) {
        register(feature, endpointPrefix, API_ALLOWLIST);
    }

    private static void registerActionView(Feature feature, String endpointPrefix) {
        register(feature, endpointPrefix, ACTION_VIEW_ALLOWLIST);
    }

    private static void register(Feature feature, String endpointPrefix, Map<Feature, Set<String>> target) {
        target.computeIfAbsent(feature, key -> new LinkedHashSet<>()).add(endpointPrefix);
    }

    private static boolean isAllowed(Feature feature, String endpoint, Map<Feature, Set<String>> source) {
        if (endpoint == null || endpoint.trim().isEmpty()) {
            return false;
        }

        Set<String> allowed = source.getOrDefault(feature, Collections.emptySet());
        if (allowed.isEmpty()) {
            return false;
        }

        String normalized = Uri.parse(endpoint).toString();
        for (String prefix : allowed) {
            if (normalized.startsWith(prefix)) {
                return true;
            }
        }
        return false;
    }
}
