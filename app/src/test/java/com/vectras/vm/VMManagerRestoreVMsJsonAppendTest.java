package com.vectras.vm;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import org.junit.Assert;
import org.junit.Test;

public class VMManagerRestoreVMsJsonAppendTest {

    @Test
    public void appendVmEntriesJson_shouldPreserveClosingBracketInsideStringPayload() {
        String baseJson = "[{\"vmID\":\"base\",\"title\":\"existing\"}]";
        String payloadWithBracket = "{\"vmID\":\"new\",\"title\":\"guest ] title\",\"path\":\"/tmp/a].img\"}";

        String result = VMManager.appendVmEntriesJson(baseJson, payloadWithBracket);

        JsonArray parsed = JsonParser.parseString(result).getAsJsonArray();
        Assert.assertEquals(2, parsed.size());

        JsonObject appended = parsed.get(1).getAsJsonObject();
        Assert.assertEquals("new", appended.get("vmID").getAsString());
        Assert.assertEquals("guest ] title", appended.get("title").getAsString());
        Assert.assertEquals("/tmp/a].img", appended.get("path").getAsString());
    }
}
