package com.vectras.vm.core;

/**
 * LowLevelAsm: low-level helpers with ASM-style naming and flat-array access.
 *
 * <p>Designed to minimize GC and overhead by operating on primitive arrays
 * with explicit offsets, avoiding object allocations and named wrappers.</p>
 */
public final class LowLevelAsm {

    // ========== Fixed-point constants ==========
    public static final int FIXED_POINT_BITS = 16;
    public static final int FIXED_POINT_SCALE = 1 << FIXED_POINT_BITS;
    public static final int FIXED_PI = (int) (Math.PI * FIXED_POINT_SCALE);
    public static final int FIXED_TWO_PI = (int) (2.0 * Math.PI * FIXED_POINT_SCALE);

    private static final int SINE_TABLE_SIZE = 256;
    private static final short[] SINE_TABLE = new short[SINE_TABLE_SIZE];
    private static final byte[] LOG2_TABLE = new byte[256];

    static {
        for (int i = 0; i < SINE_TABLE_SIZE; i++) {
            double angle = (i * Math.PI / 2.0) / SINE_TABLE_SIZE;
            SINE_TABLE[i] = (short) Math.round(Math.sin(angle) * 32767.0);
        }

        for (int i = 1; i < 256; i++) {
            LOG2_TABLE[i] = (byte) Math.round(Math.log(i) / Math.log(2) * 16);
        }
        LOG2_TABLE[0] = 0;
    }

    private LowLevelAsm() {
        throw new AssertionError("LowLevelAsm is a utility class and cannot be instantiated");
    }

    // ========== Vec2 packed ops ==========

    public static int asmVec2Pack(int x, int y) {
        return ((y & 0xFFFF) << 16) | (x & 0xFFFF);
    }

    public static int asmVec2X(int vec) {
        return (short) (vec & 0xFFFF);
    }

    public static int asmVec2Y(int vec) {
        return (short) (vec >>> 16);
    }

    public static int asmVec2AddSat(int a, int b) {
        int ax = asmVec2X(a);
        int ay = asmVec2Y(a);
        int bx = asmVec2X(b);
        int by = asmVec2Y(b);
        return asmVec2Pack(asmClampShort(ax + bx), asmClampShort(ay + by));
    }

    public static int asmVec2Dot(int a, int b) {
        int ax = asmVec2X(a);
        int ay = asmVec2Y(a);
        int bx = asmVec2X(b);
        int by = asmVec2Y(b);
        return ax * bx + ay * by;
    }

    public static int asmVec2Mag2(int vec) {
        return asmVec2Dot(vec, vec);
    }

    // ========== Matrix ops with offsets ==========

    public static void asmMat4Mul(short[] m, int mOffset, short[] v, int vOffset, int[] out, int outOffset) {
        for (int i = 0; i < 4; i++) {
            int row = mOffset + (i << 2);
            int sum = 0;
            sum += m[row] * v[vOffset];
            sum += m[row + 1] * v[vOffset + 1];
            sum += m[row + 2] * v[vOffset + 2];
            sum += m[row + 3] * v[vOffset + 3];
            out[outOffset + i] = sum >> FIXED_POINT_BITS;
        }
    }

    public static void asmMat4Transpose(short[] m, int offset) {
        swap(m, offset + 1, offset + 4);
        swap(m, offset + 2, offset + 8);
        swap(m, offset + 3, offset + 12);
        swap(m, offset + 6, offset + 9);
        swap(m, offset + 7, offset + 13);
        swap(m, offset + 11, offset + 14);
    }

    private static void swap(short[] m, int i, int j) {
        short tmp = m[i];
        m[i] = m[j];
        m[j] = tmp;
    }

    // ========== Fast trig/log ==========

    public static int asmFastSineFixed(int angleFixed) {
        int angle = angleFixed % FIXED_TWO_PI;
        if (angle < 0) angle += FIXED_TWO_PI;

        int quadrant = (angle * 4) / FIXED_TWO_PI;
        int tableAngle;
        boolean negate = false;

        switch (quadrant) {
            case 0:
                tableAngle = (angle * SINE_TABLE_SIZE * 2) / FIXED_PI;
                break;
            case 1:
                tableAngle = ((FIXED_PI - angle) * SINE_TABLE_SIZE * 2) / FIXED_PI;
                break;
            case 2:
                tableAngle = ((angle - FIXED_PI) * SINE_TABLE_SIZE * 2) / FIXED_PI;
                negate = true;
                break;
            default:
                tableAngle = ((FIXED_TWO_PI - angle) * SINE_TABLE_SIZE * 2) / FIXED_PI;
                negate = true;
                break;
        }

        if (tableAngle < 0) tableAngle = 0;
        if (tableAngle >= SINE_TABLE_SIZE) tableAngle = SINE_TABLE_SIZE - 1;
        int result = SINE_TABLE[tableAngle];
        return negate ? -result : result;
    }

    public static int asmFastCosineFixed(int angleFixed) {
        return asmFastSineFixed(angleFixed + (FIXED_PI >> 1));
    }

    public static int asmFastLog2Fixed(int x) {
        if (x <= 0) return Integer.MIN_VALUE;

        int msb = 31 - Integer.numberOfLeadingZeros(x);
        int tableIdx;
        if (msb >= 8) {
            tableIdx = (x >>> (msb - 7)) & 0xFF;
        } else {
            tableIdx = (x << (7 - msb)) & 0xFF;
        }

        int intPart = (msb - FIXED_POINT_BITS) << FIXED_POINT_BITS;
        int fracPart = (LOG2_TABLE[tableIdx] & 0xFF) << (FIXED_POINT_BITS - 4);
        return intPart + fracPart;
    }

    // ========== Small helpers ==========

    public static int asmClampShort(int value) {
        if (value < Short.MIN_VALUE) return Short.MIN_VALUE;
        if (value > Short.MAX_VALUE) return Short.MAX_VALUE;
        return value;
    }
}
