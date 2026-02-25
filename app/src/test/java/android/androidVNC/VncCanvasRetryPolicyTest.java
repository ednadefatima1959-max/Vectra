package android.androidVNC;

import org.junit.Test;

import java.io.IOException;

import static org.junit.Assert.assertEquals;

public class VncCanvasRetryPolicyTest {

    @Test
    public void shouldNotRetryWhenFailureIsNotRetryable() {
        VncCanvas.RetryAction action = VncCanvas.decideRetryAction(
                new RuntimeException("boom"),
                0,
                5,
                -1,
                -1
        );

        assertEquals(VncCanvas.RetryAction.NONE, action);
    }

    @Test
    public void shouldNotRetryWhenRetriesAreExhausted() {
        VncCanvas.RetryAction action = VncCanvas.decideRetryAction(
                new IOException("io"),
                5,
                5,
                9,
                0
        );

        assertEquals(VncCanvas.RetryAction.NONE, action);
    }

    @Test
    public void shouldRetryWithReducedLoadWhenQualityCanStillBeReduced() {
        VncCanvas.RetryAction action = VncCanvas.decideRetryAction(
                new IOException("io"),
                1,
                5,
                4,
                6
        );

        assertEquals(VncCanvas.RetryAction.RETRY_WITH_REDUCED_LOAD, action);
    }

    @Test
    public void shouldRetryWithoutReducedLoadWhenAlreadyAtMostConservativeParams() {
        VncCanvas.RetryAction action = VncCanvas.decideRetryAction(
                new ArrayIndexOutOfBoundsException("bounds"),
                2,
                5,
                9,
                0
        );

        assertEquals(VncCanvas.RetryAction.RETRY, action);
    }
}
