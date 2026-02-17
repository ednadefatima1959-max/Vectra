package com.vectras.vm.core;

import android.content.Context;

import com.termux.app.TermuxService;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public class ProotCommandBuilder {

    private final Context context;
    private final String rootfsPath;
    private final String workDir;
    private String display;
    private String pulseServer;
    private String xdgRuntimeDir;
    private String path;

    public ProotCommandBuilder(Context context, String rootfsPath, String workDir) {
        this.context = context;
        this.rootfsPath = rootfsPath;
        this.workDir = workDir;
    }

    public ProotCommandBuilder setDisplay(String display) {
        this.display = display;
        return this;
    }

    public ProotCommandBuilder setPulseServer(String pulseServer) {
        this.pulseServer = pulseServer;
        return this;
    }

    public ProotCommandBuilder setXdgRuntimeDir(String xdgRuntimeDir) {
        this.xdgRuntimeDir = xdgRuntimeDir;
        return this;
    }

    public ProotCommandBuilder setPath(String path) {
        this.path = path;
        return this;
    }

    public void applyOptionalEnvironment(Map<String, String> environment) {
        putIfNotEmpty(environment, "DISPLAY", display);
        putIfNotEmpty(environment, "PULSE_SERVER", pulseServer);
        putIfNotEmpty(environment, "XDG_RUNTIME_DIR", xdgRuntimeDir);
        putIfNotEmpty(environment, "PATH", path);
    }

    public List<String> buildCommand() {
        String filesDir = context.getFilesDir().getAbsolutePath();
        String devShmBind = filesDir + "/distro/root:/dev/shm";
        String tmpBind = filesDir + "/usr/tmp:/tmp";

        List<String> command = new ArrayList<>();
        command.add(TermuxService.PREFIX_PATH + "/bin/proot");
        command.add("--kill-on-exit");
        command.add("--link2symlink");
        command.add("-0");
        command.add("-r");
        command.add(rootfsPath);
        command.add("-b");
        command.add("/dev");
        command.add("-b");
        command.add("/proc");
        command.add("-b");
        command.add("/sys");
        command.add("-b");
        command.add("/sdcard");
        command.add("-b");
        command.add("/storage");
        command.add("-b");
        command.add("/data");
        command.add("-b");
        command.add(devShmBind);
        command.add("-b");
        command.add(tmpBind);
        command.add("-w");
        command.add(workDir);
        command.add("/bin/sh");
        command.add("--login");

        return command;
    }

    private static void putIfNotEmpty(Map<String, String> environment, String key, String value) {
        if (value != null && !value.trim().isEmpty()) {
            environment.put(key, value);
        }
    }
}
