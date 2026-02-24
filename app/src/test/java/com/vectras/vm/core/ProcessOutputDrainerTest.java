package com.vectras.vm.core;

import org.junit.Assert;
import org.junit.Test;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;

public class ProcessOutputDrainerTest {
    @Test
    public void shouldDrainStdoutAndStderrWithoutDeadlock() throws Exception {
        Process fake = new FakeProcess("o1\no2\n", "e1\ne2\n");
        ProcessOutputDrainer drainer = new ProcessOutputDrainer();
        AtomicInteger out = new AtomicInteger();
        AtomicInteger err = new AtomicInteger();

        drainer.drain(fake, (stream, line) -> {
            if ("stderr".equals(stream)) err.incrementAndGet();
            else out.incrementAndGet();
        });
        drainer.shutdown();

        Assert.assertEquals(2, out.get());
        Assert.assertEquals(2, err.get());
    }




    @Test
    public void shouldDrainWhenStderrFloods() throws Exception {
        int stderrLines = 2000;
        StringBuilder stderr = new StringBuilder();
        for (int i = 0; i < stderrLines; i++) {
            stderr.append("e").append(i).append("\n");
        }
        Process fake = new FakeProcess("o1\n", stderr.toString());
        ProcessOutputDrainer drainer = new ProcessOutputDrainer();
        AtomicInteger out = new AtomicInteger();
        AtomicInteger err = new AtomicInteger();

        try {
            drainer.drain(fake, (stream, line) -> {
                if ("stderr".equals(stream)) err.incrementAndGet();
                else out.incrementAndGet();
            });
        } finally {
            drainer.shutdown();
        }

        Assert.assertEquals(1, out.get());
        Assert.assertEquals(stderrLines, err.get());
    }


    @Test
    public void shouldInvokeAuditCallbackOnIoException() throws Exception {
        Process fake = new IOExceptionProcess();
        AtomicInteger callbacks = new AtomicInteger();
        ProcessOutputDrainer drainer = new ProcessOutputDrainer((stream, exception) -> callbacks.incrementAndGet());

        try {
            drainer.drain(fake, (stream, line) -> { });
        } finally {
            drainer.shutdown();
        }

        Assert.assertEquals(2, callbacks.get());
    }

    @Test(expected = IllegalStateException.class)
    public void shouldPropagateWorkerFailure() throws Exception {
        Process fake = new FakeProcess("ok\n", "err\n");
        ProcessOutputDrainer drainer = new ProcessOutputDrainer();
        try {
            drainer.drain(fake, (stream, line) -> {
                if ("stdout".equals(stream)) {
                    throw new IllegalStateException("boom");
                }
            });
        } finally {
            drainer.shutdown();
        }
    }

    @Test
    public void shouldHandleIOExceptionFromReadWithoutCrashingAndReportError() throws Exception {
        CountingErrorReporter reporter = new CountingErrorReporter();
        ProcessOutputDrainer drainer = new ProcessOutputDrainer(reporter);
        Process fake = new DualStreamProcess(
                new FailingInputStream(new byte[]{'o', 'k', '\n'}, 2),
                new ByteArrayInputStream(new byte[0])
        );

        try {
            drainer.drain(fake, "vm-audit-ctx", (stream, line) -> { });
        } finally {
            drainer.shutdown();
        }

        Assert.assertEquals(1, reporter.acceptedErrors.get());
        Assert.assertEquals(0, reporter.suppressedErrors.get());
        Assert.assertEquals("stdout", reporter.lastStream);
        Assert.assertEquals("vm-audit-ctx", reporter.lastVmContext);
    }

    @Test
    public void shouldRateLimitMultipleDrainReadFailures() throws Exception {
        CountingErrorReporter reporter = new CountingErrorReporter();
        ProcessOutputDrainer drainer = new ProcessOutputDrainer(reporter);

        try {
            for (int i = 0; i < 10; i++) {
                Process fake = new DualStreamProcess(
                        new FailingInputStream(new byte[]{'x', '\n'}, 1),
                        new ByteArrayInputStream(new byte[0])
                );
                drainer.drain(fake, "vm-rate-limit", (stream, line) -> { });
            }
        } finally {
            drainer.shutdown();
        }

        Assert.assertTrue("accepted should be limited", reporter.acceptedErrors.get() <= 2);
        Assert.assertTrue("suppressed should be limited", reporter.suppressedErrors.get() <= 1);
        Assert.assertTrue("some errors must be observable", reporter.acceptedErrors.get() >= 1);
    }


    @Test
    public void shouldCancelBlockedDrainWithoutFatalErrorAndWithoutLeakedThread() throws Exception {
        BlockingUntilCloseInputStream blockingStdout = new BlockingUntilCloseInputStream();
        Process fake = new DualStreamProcess(blockingStdout, new ByteArrayInputStream(new byte[0]));
        CountingErrorReporter reporter = new CountingErrorReporter();
        ProcessOutputDrainer drainer = new ProcessOutputDrainer(reporter);
        AtomicReference<Throwable> failure = new AtomicReference<>();
        CountDownLatch finished = new CountDownLatch(1);

        Thread drainThread = new Thread(() -> {
            try {
                drainer.drain(fake, "vm-cancel", (stream, line) -> { });
            } catch (Throwable t) {
                failure.set(t);
            } finally {
                finished.countDown();
            }
        }, "process-drain-test-thread");

        try {
            drainThread.start();
            Assert.assertTrue("drain should reach blocking read",
                    blockingStdout.awaitFirstRead(500, TimeUnit.MILLISECONDS));

            drainer.cancel();

            Assert.assertTrue("drain should finish after cancel", finished.await(500, TimeUnit.MILLISECONDS));
            drainThread.join(500);
            Assert.assertFalse("drain thread must not remain alive", drainThread.isAlive());
            Assert.assertNull("cancel should not fail drain", failure.get());
            Assert.assertEquals("cancelled read should not be treated as fatal", 0, reporter.acceptedErrors.get());
            Assert.assertEquals("cancelled read should not be rate-limited fatal", 0, reporter.suppressedErrors.get());
        } finally {
            drainer.shutdown();
        }
    }

    private static class FakeProcess extends Process {
        private final InputStream stdout;
        private final InputStream stderr;

        FakeProcess(String stdout, String stderr) {
            this.stdout = new ByteArrayInputStream(stdout.getBytes());
            this.stderr = new ByteArrayInputStream(stderr.getBytes());
        }

        @Override public OutputStream getOutputStream() { return OutputStream.nullOutputStream(); }
        @Override public InputStream getInputStream() { return stdout; }
        @Override public InputStream getErrorStream() { return stderr; }
        @Override public int waitFor() { return 0; }
        @Override public int exitValue() { return 0; }
        @Override public void destroy() {}
    }

    private static class DualStreamProcess extends Process {
        private final InputStream stdout;
        private final InputStream stderr;

        DualStreamProcess(InputStream stdout, InputStream stderr) {
            this.stdout = stdout;
            this.stderr = stderr;
        }

        @Override public OutputStream getOutputStream() { return OutputStream.nullOutputStream(); }
        @Override public InputStream getInputStream() { return stdout; }
        @Override public InputStream getErrorStream() { return stderr; }
        @Override public int waitFor() { return 0; }
        @Override public int exitValue() { return 0; }
        @Override public void destroy() {}
    }

    private static class FailingInputStream extends InputStream {
        private final byte[] data;
        private final int failAtReadCount;
        private int index;
        private int readCount;

        FailingInputStream(byte[] data, int failAtReadCount) {
            this.data = data;
            this.failAtReadCount = failAtReadCount;
        }

        @Override
        public int read() throws IOException {
            readCount++;
            if (readCount >= failAtReadCount) {
                throw new IOException("forced-read-failure-" + failAtReadCount);
            }
            if (index >= data.length) {
                return -1;
            }
            return data[index++];
        }
    }

    private static class BlockingUntilCloseInputStream extends InputStream {
        private final Object lock = new Object();
        private final CountDownLatch firstReadLatch = new CountDownLatch(1);
        private boolean closed;

        @Override
        public int read() throws IOException {
            firstReadLatch.countDown();
            synchronized (lock) {
                while (!closed) {
                    try {
                        lock.wait();
                    } catch (InterruptedException e) {
                        Thread.currentThread().interrupt();
                        throw new IOException("interrupted", e);
                    }
                }
            }
            throw new IOException("stream closed");
        }

        @Override
        public void close() {
            synchronized (lock) {
                closed = true;
                lock.notifyAll();
            }
        }

        boolean awaitFirstRead(long timeout, TimeUnit unit) throws InterruptedException {
            return firstReadLatch.await(timeout, unit);
        }
    }

    private static class CountingErrorReporter implements ProcessOutputDrainer.ErrorReporter {
        private final AtomicInteger acceptedErrors = new AtomicInteger();
        private final AtomicInteger suppressedErrors = new AtomicInteger();
        private volatile String lastStream;
        private volatile String lastVmContext;

        @Override
        public void onReadError(String stream, String vmContext, IOException error) {
            acceptedErrors.incrementAndGet();
            lastStream = stream;
            lastVmContext = vmContext;
        }

        @Override
        public void onReadErrorSuppressed(String stream, String vmContext, IOException error) {
            suppressedErrors.incrementAndGet();
            lastStream = stream;
            lastVmContext = vmContext;
        }
    }
}
