package com.vectras.vm.core;

import org.junit.Test;
import static org.junit.Assert.*;

/**
 * Unit tests for OptimizationStrategies class.
 */
public class OptimizationStrategiesTest {

    private static final double DELTA = 0.01;

    @Test
    public void testLoopUnroll() {
        int[] data = {1, 2, 3, 4, 5, 6, 7, 8};
        
        OptimizationStrategies.UnaryIntOperation doubleOp = new OptimizationStrategies.UnaryIntOperation() {
            @Override
            public int apply(int value) {
                return value * 2;
            }
        };

        OptimizationStrategies.loopUnroll(data, doubleOp, 4);

        for (int i = 0; i < data.length; i++) {
            assertEquals("Each element should be doubled", (i + 1) * 2, data[i]);
        }
    }

    @Test
    public void testLoopUnrollVariousSizes() {
        // Test unroll factor 2
        int[] data2 = {1, 2, 3};
        OptimizationStrategies.loopUnroll(data2, v -> v + 10, 2);
        assertArrayEquals(new int[]{11, 12, 13}, data2);

        // Test unroll factor 8
        int[] data8 = new int[10];
        for (int i = 0; i < 10; i++) data8[i] = i;
        OptimizationStrategies.loopUnroll(data8, v -> v * 2, 8);
        for (int i = 0; i < 10; i++) {
            assertEquals(i * 2, data8[i]);
        }

        // Edge cases
        OptimizationStrategies.loopUnroll(null, v -> v, 4);
        OptimizationStrategies.loopUnroll(new int[0], v -> v, 4);
    }

    @Test
    public void testLoopFusion() {
        int[] data = {1, 2, 3, 4, 5};
        
        OptimizationStrategies.UnaryIntOperation addOne = v -> v + 1;
        OptimizationStrategies.UnaryIntOperation multiplyTwo = v -> v * 2;

        OptimizationStrategies.loopFusion(data, addOne, multiplyTwo);

        // Should apply both operations: (v + 1) * 2
        assertArrayEquals(new int[]{4, 6, 8, 10, 12}, data);

        // Edge case
        OptimizationStrategies.loopFusion(null, addOne, multiplyTwo);
    }

    @Test
    public void testLoopTiling() {
        int rows = 4, cols = 4;
        int[] matrix = {
            1, 2, 3, 4,
            5, 6, 7, 8,
            9, 10, 11, 12,
            13, 14, 15, 16
        };

        OptimizationStrategies.loopTiling(matrix, rows, cols, v -> v * 2, 2);

        for (int i = 0; i < matrix.length; i++) {
            assertEquals((i + 1) * 2, matrix[i]);
        }

        // Edge case
        OptimizationStrategies.loopTiling(null, 4, 4, v -> v, 2);
    }

    @Test
    public void testObjectPool() {
        OptimizationStrategies.ObjectFactory<StringBuilder> factory = 
            new OptimizationStrategies.ObjectFactory<StringBuilder>() {
                @Override
                public StringBuilder create() {
                    return new StringBuilder();
                }
            };

        OptimizationStrategies.ObjectPool<StringBuilder> pool = 
            new OptimizationStrategies.ObjectPool<>(factory, 10);

        // Acquire and release
        StringBuilder sb1 = pool.acquire();
        assertNotNull(sb1);
        
        sb1.append("test");
        pool.release(sb1);

        StringBuilder sb2 = pool.acquire();
        assertNotNull(sb2);
    }

    @Test
    public void testAlignToCacheLine() {
        assertEquals(0, OptimizationStrategies.alignToCacheLine(0));
        assertEquals(64, OptimizationStrategies.alignToCacheLine(1));
        assertEquals(64, OptimizationStrategies.alignToCacheLine(64));
        assertEquals(128, OptimizationStrategies.alignToCacheLine(65));
        assertEquals(128, OptimizationStrategies.alignToCacheLine(100));
    }

    @Test
    public void testPadForFalseSharing() {
        int padded = OptimizationStrategies.padForFalseSharing(10);
        assertTrue("Should be padded", padded >= 10);
        
        // Should be multiple of cache line (64 bytes = 16 ints)
        assertTrue("Should align to cache line", (padded % 16) == 0);
    }

    @Test
    public void testFastPower() {
        assertEquals(1, OptimizationStrategies.fastPower(5, 0));
        assertEquals(5, OptimizationStrategies.fastPower(5, 1));
        assertEquals(25, OptimizationStrategies.fastPower(5, 2));
        assertEquals(125, OptimizationStrategies.fastPower(5, 3));
        assertEquals(1024, OptimizationStrategies.fastPower(2, 10));
        assertEquals(1, OptimizationStrategies.fastPower(1, 1000));
    }

    @Test
    public void testFastModPower() {
        assertEquals(1, OptimizationStrategies.fastModPower(5, 0, 13));
        assertEquals(5, OptimizationStrategies.fastModPower(5, 1, 13));
        assertEquals(12, OptimizationStrategies.fastModPower(5, 2, 13)); // 25 % 13 = 12
        assertEquals(8, OptimizationStrategies.fastModPower(5, 3, 13));  // 125 % 13 = 8
        assertEquals(4, OptimizationStrategies.fastModPower(2, 10, 12)); // 1024 % 12 = 4
    }

    @Test
    public void testMemoCache() {
        OptimizationStrategies.MemoCache<Integer, Integer> cache = 
            new OptimizationStrategies.MemoCache<>(3);

        cache.put(1, 10);
        cache.put(2, 20);
        cache.put(3, 30);

        assertTrue(cache.contains(1));
        assertEquals(10, (int) cache.get(1));
        assertEquals(20, (int) cache.get(2));
        assertEquals(30, (int) cache.get(3));

        // Add one more, should evict least recently used
        cache.put(4, 40);
        assertTrue(cache.contains(4));

        cache.clear();
        assertFalse(cache.contains(1));
    }

    @Test
    public void testConvertAoSToSoA() {
        // AoS: x0,y0,z0, x1,y1,z1
        int[] aos = {1, 2, 3, 4, 5, 6};
        int[][] soa = new int[3][2];

        OptimizationStrategies.convertAoSToSoA(aos, soa, 3);

        assertArrayEquals(new int[]{1, 4}, soa[0]); // x values
        assertArrayEquals(new int[]{2, 5}, soa[1]); // y values
        assertArrayEquals(new int[]{3, 6}, soa[2]); // z values

        // Edge cases
        OptimizationStrategies.convertAoSToSoA(null, soa, 3);
        OptimizationStrategies.convertAoSToSoA(aos, null, 3);
    }

    @Test
    public void testConvertSoAToAoS() {
        // SoA: x[], y[], z[]
        int[][] soa = {{1, 4}, {2, 5}, {3, 6}};
        int[] aos = new int[6];

        OptimizationStrategies.convertSoAToAoS(soa, aos, 3);

        assertArrayEquals(new int[]{1, 2, 3, 4, 5, 6}, aos);

        // Edge cases
        OptimizationStrategies.convertSoAToAoS(null, aos, 3);
        OptimizationStrategies.convertSoAToAoS(soa, null, 3);
    }

    @Test
    public void testMortonEncode() {
        assertEquals(0, OptimizationStrategies.mortonEncode(0, 0));
        
        // (1,0) -> 0b01 = 1
        assertEquals(1, OptimizationStrategies.mortonEncode(1, 0));
        
        // (0,1) -> 0b10 = 2
        assertEquals(2, OptimizationStrategies.mortonEncode(0, 1));
        
        // (1,1) -> 0b11 = 3
        assertEquals(3, OptimizationStrategies.mortonEncode(1, 1));
    }

    @Test
    public void testMortonDecode() {
        int[] coord = OptimizationStrategies.mortonDecode(0);
        assertArrayEquals(new int[]{0, 0}, coord);

        coord = OptimizationStrategies.mortonDecode(1);
        assertArrayEquals(new int[]{1, 0}, coord);

        coord = OptimizationStrategies.mortonDecode(2);
        assertArrayEquals(new int[]{0, 1}, coord);

        coord = OptimizationStrategies.mortonDecode(3);
        assertArrayEquals(new int[]{1, 1}, coord);
    }

    @Test
    public void testMortonRoundTrip() {
        for (int x = 0; x < 16; x++) {
            for (int y = 0; y < 16; y++) {
                int morton = OptimizationStrategies.mortonEncode(x, y);
                int[] decoded = OptimizationStrategies.mortonDecode(morton);
                assertEquals("X should match", x, decoded[0]);
                assertEquals("Y should match", y, decoded[1]);
            }
        }
    }

    @Test
    public void testStrengthReduction() {
        assertEquals(0, OptimizationStrategies.strengthReduction(0, 8));
        assertEquals(40, OptimizationStrategies.strengthReduction(5, 8));
        assertEquals(80, OptimizationStrategies.strengthReduction(10, 8));
        assertEquals(32, OptimizationStrategies.strengthReduction(1, 32));
        assertEquals(64, OptimizationStrategies.strengthReduction(2, 32));
    }

    @Test
    public void testCSECache() {
        OptimizationStrategies.CSECache cache = new OptimizationStrategies.CSECache(5);

        cache.put(10, 100);
        cache.put(20, 200);

        assertTrue(cache.contains(10));
        assertTrue(cache.contains(20));
        assertFalse(cache.contains(30));

        assertEquals(100, cache.get(10));
        assertEquals(200, cache.get(20));
        assertEquals(0, cache.get(30));
    }

    @Test
    public void testIsValueUsed() {
        assertTrue(OptimizationStrategies.isValueUsed(42));
        assertTrue(OptimizationStrategies.isValueUsed(-1));
        assertFalse(OptimizationStrategies.isValueUsed(0));
    }

    @Test
    public void testSelectStrategy() {
        // Small data
        OptimizationStrategies.Strategy s1 = OptimizationStrategies.selectStrategy(
            100, OptimizationStrategies.AccessPattern.SEQUENTIAL, 1.0);
        assertEquals(OptimizationStrategies.Strategy.LOOP_UNROLL, s1);

        // Large sequential
        OptimizationStrategies.Strategy s2 = OptimizationStrategies.selectStrategy(
            200000, OptimizationStrategies.AccessPattern.SEQUENTIAL, 1.0);
        assertEquals(OptimizationStrategies.Strategy.LOOP_TILING, s2);

        // High compute intensity
        OptimizationStrategies.Strategy s3 = OptimizationStrategies.selectStrategy(
            10000, OptimizationStrategies.AccessPattern.SEQUENTIAL, 20.0);
        assertEquals(OptimizationStrategies.Strategy.PARALLELIZE, s3);

        // Random access
        OptimizationStrategies.Strategy s4 = OptimizationStrategies.selectStrategy(
            10000, OptimizationStrategies.AccessPattern.RANDOM, 1.0);
        assertEquals(OptimizationStrategies.Strategy.SPATIAL_LOCALITY, s4);
    }

    @Test
    public void testCannotInstantiate() {
        try {
            java.lang.reflect.Constructor<?> constructor = OptimizationStrategies.class.getDeclaredConstructor();
            constructor.setAccessible(true);
            constructor.newInstance();
            fail("Should not be able to instantiate utility class");
        } catch (Exception e) {
            assertTrue(e.getCause() instanceof AssertionError);
        }
    }
}
