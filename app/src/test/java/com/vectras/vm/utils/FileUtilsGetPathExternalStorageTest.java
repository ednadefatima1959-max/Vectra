package com.vectras.vm.utils;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;

import android.content.Context;
import android.net.Uri;
import android.os.Environment;

import androidx.test.core.app.ApplicationProvider;

import org.junit.After;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.robolectric.RobolectricTestRunner;

import java.io.File;
import java.io.IOException;

@RunWith(RobolectricTestRunner.class)
public class FileUtilsGetPathExternalStorageTest {

    private File createdFile;

    @After
    public void cleanup() {
        if (createdFile != null && createdFile.exists()) {
            createdFile.delete();
        }
    }

    @Test
    public void getPath_externalStorageDocument_withEmptyResolvedPath_shouldReturnNull() {
        Context context = ApplicationProvider.getApplicationContext();
        Uri uri = Uri.parse("content://com.android.externalstorage.documents/document/primary:vectras_missing_file.img");

        String resolvedPath = FileUtils.getPath(context, uri);

        assertNull(resolvedPath);
    }

    @Test
    public void getPath_externalStorageDocument_withValidResolvedPath_shouldReturnPath() throws IOException {
        Context context = ApplicationProvider.getApplicationContext();
        String relativeName = "vectras_existing_file.img";
        createdFile = new File(Environment.getExternalStorageDirectory(), relativeName);
        File parent = createdFile.getParentFile();
        if (parent != null && !parent.exists()) {
            parent.mkdirs();
        }
        createdFile.createNewFile();

        Uri uri = Uri.parse("content://com.android.externalstorage.documents/document/primary:" + relativeName);

        String resolvedPath = FileUtils.getPath(context, uri);

        assertEquals(createdFile.getAbsolutePath(), resolvedPath);
    }
}
