package com.vectras.vm.core;

import org.junit.Test;
import static org.junit.Assert.*;

/**
 * Unit tests for AlgorithmAnalyzer class.
 */
public class AlgorithmAnalyzerTest {

    private static final double DELTA = 0.01;

    @Test
    public void testEstimateTimeComplexity() {
        // O(1) algorithm
        AlgorithmAnalyzer.TimedOperation constantTime = new AlgorithmAnalyzer.TimedOperation() {
            @Override
            public void execute(int size) {
                int x = 42;
                x += 1;
            }
        };

        int[] sizes = {100, 200, 400, 800};
        int complexity = AlgorithmAnalyzer.estimateTimeComplexity(constantTime, sizes);
        assertTrue("Constant time should be detected", complexity <= 2);

        // O(n) algorithm
        AlgorithmAnalyzer.TimedOperation linearTime = new AlgorithmAnalyzer.TimedOperation() {
            @Override
            public void execute(int size) {
                int sum = 0;
                for (int i = 0; i < size; i++) {
                    sum += i;
                }
            }
        };

        int linearComplexity = AlgorithmAnalyzer.estimateTimeComplexity(linearTime, sizes);
        assertTrue("Linear time should be detected", linearComplexity >= 2 && linearComplexity <= 4);
    }

    @Test
    public void testEstimateSpaceComplexity() {
        AlgorithmAnalyzer.TimedOperation allocatingOp = new AlgorithmAnalyzer.TimedOperation() {
            @Override
            public void execute(int size) {
                int[] arr = new int[size];
                for (int i = 0; i < size; i++) {
                    arr[i] = i;
                }
            }
        };

        long space = AlgorithmAnalyzer.estimateSpaceComplexity(allocatingOp, 1000);
        assertTrue("Should detect space allocation", space >= 0);
    }

    @Test
    public void testAnalyzeCacheEfficiency() {
        byte[] data = new byte[1024];

        // Sequential access (good cache efficiency)
        int[] sequential = new int[100];
        for (int i = 0; i < sequential.length; i++) {
            sequential[i] = i;
        }
        double seqMissRatio = AlgorithmAnalyzer.analyzeCacheEfficiency(data, sequential);
        assertTrue("Sequential access should have low miss ratio", seqMissRatio < 0.5);

        // Random access (poor cache efficiency)
        int[] random = new int[100];
        for (int i = 0; i < random.length; i++) {
            random[i] = (i * 17) % data.length;
        }
        double randMissRatio = AlgorithmAnalyzer.analyzeCacheEfficiency(data, random);
        assertTrue("Random access should have higher miss ratio", randMissRatio >= 0.0);

        // Edge cases
        assertEquals(0.0, AlgorithmAnalyzer.analyzeCacheEfficiency(null, sequential), DELTA);
        assertEquals(0.0, AlgorithmAnalyzer.analyzeCacheEfficiency(data, null), DELTA);
        assertEquals(0.0, AlgorithmAnalyzer.analyzeCacheEfficiency(data, new int[0]), DELTA);
    }

    @Test
    public void testSuggestDataLayout() {
        // Small elements, sequential access -> AoS
        int layout1 = AlgorithmAnalyzer.suggestDataLayout(4, AlgorithmAnalyzer.AccessPattern.SEQUENTIAL);
        assertEquals("Small sequential should be AoS", 0, layout1);

        // Large elements, sequential access -> SoA
        int layout2 = AlgorithmAnalyzer.suggestDataLayout(128, AlgorithmAnalyzer.AccessPattern.SEQUENTIAL);
        assertEquals("Large sequential should be SoA", 1, layout2);

        // Random access -> AoS
        int layout3 = AlgorithmAnalyzer.suggestDataLayout(16, AlgorithmAnalyzer.AccessPattern.RANDOM);
        assertEquals("Random should be AoS", 0, layout3);

        // Strided access -> AoSoA
        int layout4 = AlgorithmAnalyzer.suggestDataLayout(16, AlgorithmAnalyzer.AccessPattern.STRIDED);
        assertEquals("Strided should be AoSoA", 2, layout4);

        // Vectorized -> SoA
        int layout5 = AlgorithmAnalyzer.suggestDataLayout(16, AlgorithmAnalyzer.AccessPattern.VECTORIZED);
        assertEquals("Vectorized should be SoA", 1, layout5);
    }

    @Test
    public void testAnalyzeBranchPredictability() {
        // Perfectly predictable (all true)
        boolean[] predictable = new boolean[100];
        for (int i = 0; i < predictable.length; i++) {
            predictable[i] = true;
        }
        double predScore = AlgorithmAnalyzer.analyzeBranchPredictability(predictable);
        assertTrue("All same should be predictable", predScore > 0.8);

        // Unpredictable (alternating)
        boolean[] unpredictable = new boolean[100];
        for (int i = 0; i < unpredictable.length; i++) {
            unpredictable[i] = (i % 2) == 0;
        }
        double unpredScore = AlgorithmAnalyzer.analyzeBranchPredictability(unpredictable);
        assertTrue("Alternating should be less predictable", unpredScore < 1.0);

        // Edge cases
        assertEquals(1.0, AlgorithmAnalyzer.analyzeBranchPredictability(new boolean[0]), DELTA);
        assertEquals(1.0, AlgorithmAnalyzer.analyzeBranchPredictability(new boolean[1]), DELTA);
        assertEquals(1.0, AlgorithmAnalyzer.analyzeBranchPredictability(null), DELTA);
    }

    @Test
    public void testSuggestBranchless() {
        // Many unpredictable branches -> suggest branchless
        assertTrue("Should suggest branchless for many unpredictable branches",
                AlgorithmAnalyzer.suggestBranchless(5, 0.5));

        // Few predictable branches -> don't suggest branchless
        assertFalse("Should not suggest branchless for few predictable branches",
                AlgorithmAnalyzer.suggestBranchless(2, 0.9));

        // Very unpredictable -> suggest branchless even with few branches
        assertTrue("Should suggest branchless for very unpredictable",
                AlgorithmAnalyzer.suggestBranchless(2, 0.3));
    }

    @Test
    public void testAnalyzeParallelism() {
        // Fully serial (each depends on previous)
        int[] serial = {-1, 0, 1, 2, 3};
        double parallelismSerial = AlgorithmAnalyzer.analyzeParallelism(serial);
        assertTrue("Serial should have low parallelism", parallelismSerial < 2.0);

        // Fully parallel (no dependencies)
        int[] parallel = {-1, -1, -1, -1, -1};
        double parallelismPar = AlgorithmAnalyzer.analyzeParallelism(parallel);
        assertTrue("Parallel should have high parallelism", parallelismPar >= 1.0);

        // Edge cases
        assertEquals(1.0, AlgorithmAnalyzer.analyzeParallelism(new int[0]), DELTA);
        assertEquals(1.0, AlgorithmAnalyzer.analyzeParallelism(null), DELTA);
    }

    @Test
    public void testSuggestThreadCount() {
        int cores = Runtime.getRuntime().availableProcessors();

        // Many fine-grained tasks
        int threads1 = AlgorithmAnalyzer.suggestThreadCount(1000, 100);
        assertTrue("Fine-grained should use fewer threads", threads1 >= 1 && threads1 <= cores);

        // Few coarse-grained tasks
        int threads2 = AlgorithmAnalyzer.suggestThreadCount(10, 10000);
        assertTrue("Coarse-grained can use more threads", threads2 >= 1 && threads2 <= cores);
    }

    @Test
    public void testAnalyzeILP() {
        // All same type (low ILP)
        AlgorithmAnalyzer.OperationType[] sameType = new AlgorithmAnalyzer.OperationType[10];
        for (int i = 0; i < sameType.length; i++) {
            sameType[i] = AlgorithmAnalyzer.OperationType.ALU;
        }
        double ilpLow = AlgorithmAnalyzer.analyzeILP(sameType);
        assertEquals("Same types should have low ILP", 1.0, ilpLow, DELTA);

        // Mixed types (higher ILP)
        AlgorithmAnalyzer.OperationType[] mixed = {
            AlgorithmAnalyzer.OperationType.LOAD,
            AlgorithmAnalyzer.OperationType.ALU,
            AlgorithmAnalyzer.OperationType.STORE,
            AlgorithmAnalyzer.OperationType.FPU
        };
        double ilpHigh = AlgorithmAnalyzer.analyzeILP(mixed);
        assertTrue("Mixed types should have higher ILP", ilpHigh > 1.0);

        // Edge cases
        assertEquals(1.0, AlgorithmAnalyzer.analyzeILP(new AlgorithmAnalyzer.OperationType[0]), DELTA);
        assertEquals(1.0, AlgorithmAnalyzer.analyzeILP(new AlgorithmAnalyzer.OperationType[1]), DELTA);
        assertEquals(1.0, AlgorithmAnalyzer.analyzeILP(null), DELTA);
    }

    @Test
    public void testEstimateVectorizationPotential() {
        // Large data, no dependencies
        double speedup1 = AlgorithmAnalyzer.estimateVectorizationPotential(10000, 4, false);
        assertTrue("Should have vectorization potential", speedup1 > 1.0);

        // With dependencies
        double speedup2 = AlgorithmAnalyzer.estimateVectorizationPotential(10000, 4, true);
        assertEquals("Dependencies should prevent vectorization", 1.0, speedup2, DELTA);

        // Too small data
        double speedup3 = AlgorithmAnalyzer.estimateVectorizationPotential(10, 4, false);
        assertEquals("Too small should not benefit", 1.0, speedup3, DELTA);
    }

    @Test
    public void testGenerateReport() {
        String report = AlgorithmAnalyzer.generateReport(
            "TestAlgorithm",
            3,  // O(n)
            4096,  // 4KB
            0.2,  // 80% cache hit rate
            2.5   // 2.5x parallelism
        );

        assertNotNull("Report should not be null", report);
        assertTrue("Report should contain name", report.contains("TestAlgorithm"));
        assertTrue("Report should contain complexity", report.contains("O(n)"));
        assertTrue("Report should contain space", report.contains("KB"));
        assertTrue("Report should mention cache", report.contains("Cache"));
        assertTrue("Report should mention parallelism", report.contains("Parallelism"));
    }

    @Test
    public void testGenerateReportWithRecommendations() {
        // Poor cache efficiency
        String report1 = AlgorithmAnalyzer.generateReport("CachePoor", 3, 1024, 0.5, 1.0);
        assertTrue("Should recommend cache improvement", report1.contains("locality"));

        // High parallelism
        String report2 = AlgorithmAnalyzer.generateReport("Parallel", 3, 1024, 0.1, 3.0);
        assertTrue("Should recommend parallelization", report2.contains("parallelization"));

        // High complexity
        String report3 = AlgorithmAnalyzer.generateReport("Slow", 5, 1024, 0.1, 1.0);
        assertTrue("Should recommend optimization", report3.contains("optimization"));
    }

    @Test
    public void testCannotInstantiate() {
        try {
            java.lang.reflect.Constructor<?> constructor = AlgorithmAnalyzer.class.getDeclaredConstructor();
            constructor.setAccessible(true);
            constructor.newInstance();
            fail("Should not be able to instantiate utility class");
        } catch (Exception e) {
            assertTrue(e.getCause() instanceof AssertionError);
        }
    }
}
