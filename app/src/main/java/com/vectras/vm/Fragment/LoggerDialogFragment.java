package com.vectras.vm.Fragment;

import android.app.Dialog;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;

import androidx.fragment.app.DialogFragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.view.Window;

import com.vectras.vm.R;
import com.vectras.vm.VectrasApp;
import com.vectras.vm.adapter.LogsAdapter;
import com.vectras.vm.core.LogcatRuntime;


public class LoggerDialogFragment extends DialogFragment {

    private LogsAdapter mLogAdapter;
    private RecyclerView logList;
    private LogcatRuntime logcatRuntime;
    private LogcatRuntime.Listener logListener;

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        final Dialog alertDialog = new Dialog(getActivity(), R.style.MainDialogTheme);
        alertDialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        alertDialog.getWindow().setBackgroundDrawable(new ColorDrawable(android.graphics.Color.TRANSPARENT));
        alertDialog.setContentView(R.layout.fragment_logs);
        LinearLayoutManager layoutManager = new LinearLayoutManager(VectrasApp.getApp());
        mLogAdapter = new LogsAdapter(layoutManager, VectrasApp.getApp());
        logList = (RecyclerView) alertDialog.findViewById(R.id.recyclerLog);
        logList.setAdapter(mLogAdapter);
        logList.setLayoutManager(layoutManager);
        mLogAdapter.scrollToLastPosition();
        logcatRuntime = LogcatRuntime.getInstance();
        logListener = appended -> {
            if (isAdded()) {
                requireActivity().runOnUiThread(mLogAdapter::scrollToLastPosition);
            }
        };
        logcatRuntime.addListener(logListener);
        logcatRuntime.acquire();
        alertDialog.show();
        return alertDialog;
    }
    @Override
    public void onDismiss(@androidx.annotation.NonNull android.content.DialogInterface dialog) {
        if (logcatRuntime != null && logListener != null) {
            logcatRuntime.removeListener(logListener);
            logcatRuntime.release();
        }
        super.onDismiss(dialog);
    }
}
