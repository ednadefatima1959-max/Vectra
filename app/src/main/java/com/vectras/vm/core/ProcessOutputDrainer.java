package com.vectras.vm.core;

import android.content.Context;
import android.util.Log;

import com.vectras.vm.audit.AuditEvent;
import com.vectras.vm.audit.AuditLedger;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;

import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Concurrent stdout/stderr drainer that never blocks on one side only.
 */
public class ProcessOutputDrainer {
    private static final String TAG = "ProcessOutputDrainer";
    private static final double IO_ERROR_LOG_REFILL_PER_SEC = 0.2d;
    private static final int IO_ERROR_LOG_BURST = 2;
    private static final double IO_ERROR_SUPPRESSED_LOG_REFILL_PER_SEC = 0.1d;
    private static final int IO_ERROR_SUPPRESSED_LOG_BURST = 1;
    public interface OutputLineConsumer {
        void onLine(String stream, String line);
    }

    public interface DrainAuditCallback {
        void onIoDrainFailure(String streamName, IOException exception);
    }

    private static final DrainAuditCallback NOOP_AUDIT_CALLBACK = (streamName, exception) -> {
    };

    private final ExecutorService streamExecutor = Executors.newFixedThreadPool(2);
    private final AtomicBoolean cancelled = new AtomicBoolean(false);
    private final TokenBucketRateLimiter ioErrorLogLimiter =
            new TokenBucketRateLimiter(IO_ERROR_LOG_REFILL_PER_SEC, IO_ERROR_LOG_BURST);
    private final TokenBucketRateLimiter ioErrorSuppressedLogLimiter =
            new TokenBucketRateLimiter(IO_ERROR_SUPPRESSED_LOG_REFILL_PER_SEC, IO_ERROR_SUPPRESSED_LOG_BURST);
    private volatile DrainAuditCallback drainAuditCallback = NOOP_AUDIT_CALLBACK;

    public ProcessOutputDrainer() {
    }

    public ProcessOutputDrainer(Context context, String vmId) {
        setVmAuditContext(context, vmId);
    }

    public ProcessOutputDrainer(DrainAuditCallback drainAuditCallback) {
        setDrainAuditCallback(drainAuditCallback);
    }

    public void setVmAuditContext(Context context, String vmId) {
        final String safeVmId = vmId == null ? "unknown" : vmId;
        if (context == null) {
            this.drainAuditCallback = NOOP_AUDIT_CALLBACK;
            return;
        }
        this.drainAuditCallback = (streamName, exception) -> AuditLedger.record(context, new AuditEvent(
                ProcessRuntimeOps.monoMs(),
                ProcessRuntimeOps.wallMs(),
                safeVmId,
                ProcessSupervisor.State.RUN.name(),
                ProcessSupervisor.State.DEGRADED.name(),
                "io_drain_failure",
                0,
                0L,
                0L,
                "log_and_continue"
        ));
    }

    public void setDrainAuditCallback(DrainAuditCallback drainAuditCallback) {
        this.drainAuditCallback = drainAuditCallback == null ? NOOP_AUDIT_CALLBACK : drainAuditCallback;
    }

    public void cancel() {
        cancelled.set(true);
    }

    public void drain(Process process, OutputLineConsumer consumer) throws InterruptedException {
        Future<?> out = streamExecutor.submit(() -> readStream("stdout", process.getInputStream(), consumer));
        Future<?> err = streamExecutor.submit(() -> readStream("stderr", process.getErrorStream(), consumer));

        try {
            waitFuture(out);
            waitFuture(err);
        } catch (InterruptedException e) {
            out.cancel(true);
            err.cancel(true);
            throw e;
        } catch (RuntimeException e) {
            out.cancel(true);
            err.cancel(true);
            Log.w(TAG, "drain failed", e);
            throw e;
        }
    }

    public void shutdown() {
        streamExecutor.shutdownNow();
        try {
            streamExecutor.awaitTermination(200, TimeUnit.MILLISECONDS);
        } catch (InterruptedException ignored) {
            Thread.currentThread().interrupt();
        }
    }

    private void readStream(String name, InputStream stream, OutputLineConsumer consumer) {
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(stream, StandardCharsets.UTF_8))) {
            String line;
            while (!cancelled.get() && (line = reader.readLine()) != null) {
                consumer.onLine(name, line);
            }
        } catch (IOException e) {
            Log.w(TAG, "readStream non-fatal failure on " + name
                    + " [" + e.getClass().getSimpleName() + "]: " + e.getMessage(), e);
            drainAuditCallback.onIoDrainFailure(name, e);
        }
    }

    private static void waitFuture(Future<?> future) throws InterruptedException {
        try {
            future.get();
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw e;
        } catch (ExecutionException e) {
            throw new IllegalStateException("stream drain failed", e.getCause());
        } catch (CancellationException e) {
            throw new IllegalStateException("stream drain cancelled", e);
        }
    }
}
