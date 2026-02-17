package com.vectras.qemu.utils;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.ContentResolver;
import android.content.Context;
import android.net.Uri;
import android.os.ParcelFileDescriptor;

import org.junit.Test;

import java.io.File;

public class FileUtilsOpenModeTest {

    @Test
    public void openModeWrappers_shouldMatchVmCanonicalContract() {
        assertEquals(com.vectras.vm.utils.FileUtils.resolveContentOpenMode("content://disk.qcow2", "r"),
                FileUtils.resolveContentOpenMode("content://disk.qcow2", "r"));
        assertEquals(com.vectras.vm.utils.FileUtils.resolveContentOpenMode("content://disk.iso", "rw"),
                FileUtils.resolveContentOpenMode("content://disk.iso", "rw"));
        assertEquals(com.vectras.vm.utils.FileUtils.resolveContentOpenMode("content://disk.qcow2", "wa"),
                FileUtils.resolveContentOpenMode("content://disk.qcow2", "wa"));

        assertEquals(com.vectras.vm.utils.FileUtils.resolveParcelOpenMode("/storage/emulated/0/vm/disk.ISO", "rw"),
                FileUtils.resolveParcelOpenMode("/storage/emulated/0/vm/disk.ISO", "rw"));
        assertEquals(com.vectras.vm.utils.FileUtils.resolveParcelOpenMode("/storage/emulated/0/vm/disk.qcow2", ""),
                FileUtils.resolveParcelOpenMode("/storage/emulated/0/vm/disk.qcow2", ""));
    }

    @Test
    public void getFdWrapper_overloadWithoutBackendMode_shouldMatchVmBehavior() throws Exception {
        Context context = mock(Context.class);
        ContentResolver resolver = mock(ContentResolver.class);
        when(context.getContentResolver()).thenReturn(resolver);

        Uri uri = Uri.parse("content://disk.qcow2");
        File tempFile = File.createTempFile("vectras-qemu-fd", ".img");
        ParcelFileDescriptor pfd = ParcelFileDescriptor.open(tempFile, ParcelFileDescriptor.MODE_READ_WRITE);
        when(resolver.openFileDescriptor(uri, "rw")).thenReturn(pfd);

        int fdWithoutMode = FileUtils.get_fd(context, "content://disk.qcow2");
        int fdWithNullMode = FileUtils.get_fd(context, "content://disk.qcow2", null);

        assertTrue(fdWithoutMode > 0);
        assertEquals(fdWithoutMode, fdWithNullMode);
        verify(resolver, times(2)).openFileDescriptor(uri, "rw");

        pfd.close();
    }
}
