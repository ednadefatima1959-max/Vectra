package com.vectras.vm.Fragment;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.vectras.vm.R;
import com.vectras.vm.VectrasApp;
import com.vectras.vm.adapter.LogsAdapter;
import com.vectras.vm.core.LogcatRuntime;


public class LoggerFragment extends Fragment {

    View view;
    private LogsAdapter mLogAdapter;
    private RecyclerView logList;
    private LogcatRuntime logcatRuntime;
    private LogcatRuntime.Listener logListener;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO Auto-generated method stub

        view = inflater.inflate(R.layout.fragment_logs, container, false);
        LinearLayoutManager layoutManager = new LinearLayoutManager(VectrasApp.getApp());
        mLogAdapter = new LogsAdapter(layoutManager, VectrasApp.getApp());
        logList = (RecyclerView) view.findViewById(R.id.recyclerLog);
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

        return view;
    }

    @Override
    public void onDestroyView() {
        if (logcatRuntime != null && logListener != null) {
            logcatRuntime.removeListener(logListener);
            logcatRuntime.release();
        }
        super.onDestroyView();
    }

}
