package com.vectras.vm.download;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

public class DownloadWorkerEndpointValidationTest {

    @Test
    public void acceptsHttpsAllowlistedHost() {
        DownloadWorker.EndpointValidationOutcome outcome =
                DownloadWorker.validateDownloadEndpoint("https://raw.githubusercontent.com/owner/repo/file.img");

        assertTrue(outcome.allowed);
        assertEquals("None", outcome.reasonClass);
    }

    @Test
    public void rejectsNonHttpsScheme() {
        DownloadWorker.EndpointValidationOutcome outcome =
                DownloadWorker.validateDownloadEndpoint("http://raw.githubusercontent.com/owner/repo/file.img");

        assertFalse(outcome.allowed);
        assertEquals("NonHttpsScheme", outcome.reasonClass);
    }

    @Test
    public void rejectsHostOutsideAllowlist() {
        DownloadWorker.EndpointValidationOutcome outcome =
                DownloadWorker.validateDownloadEndpoint("https://example.com/rom.img");

        assertFalse(outcome.allowed);
        assertEquals("DisallowedEndpoint", outcome.reasonClass);
    }

    @Test
    public void rejectsMalformedEndpoint() {
        DownloadWorker.EndpointValidationOutcome outcome =
                DownloadWorker.validateDownloadEndpoint("://bad-url");

        assertFalse(outcome.allowed);
        assertEquals("IllegalArgumentException", outcome.reasonClass);
    }
}
