package com.vectras.vm.core;

import org.junit.Assert;
import org.junit.Test;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.concurrent.atomic.AtomicInteger;

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

    private static class IOExceptionProcess extends Process {
        @Override public OutputStream getOutputStream() { return OutputStream.nullOutputStream(); }
        @Override public InputStream getInputStream() { return new ThrowingInputStream(); }
        @Override public InputStream getErrorStream() { return new ThrowingInputStream(); }
        @Override public int waitFor() { return 0; }
        @Override public int exitValue() { return 0; }
        @Override public void destroy() {}
    }

    private static class ThrowingInputStream extends InputStream {
        @Override
        public int read() throws java.io.IOException {
            throw new java.io.IOException("forced");
        }
    }

}
