package com.vectras.vm.main;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class MainUiStateViewModel extends ViewModel {
    private final MutableLiveData<Boolean> searchReady = new MutableLiveData<>(false);

    public LiveData<Boolean> getSearchReady() {
        return searchReady;
    }

    public void setSearchReady(boolean isReady) {
        searchReady.setValue(isReady);
    }
}
