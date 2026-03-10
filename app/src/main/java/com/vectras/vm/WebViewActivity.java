package com.vectras.vm;

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.StrictMode;
import android.transition.TransitionManager;
import android.util.Log;
import android.view.View;
import android.webkit.CookieManager;
import android.webkit.WebResourceRequest;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import androidx.activity.OnBackPressedCallback;
import androidx.appcompat.app.AppCompatActivity;

import com.vectras.vm.databinding.ActivityWebViewBinding;
import com.vectras.vm.utils.UIUtils;

public class WebViewActivity extends AppCompatActivity {
    private static final String TAG = "WebViewActivity";
    ActivityWebViewBinding binding;
    String oringialDomain;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        UIUtils.edgeToEdge(this);
        binding = ActivityWebViewBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());
        UIUtils.setOnApplyWindowInsetsListener(binding.main);

        StrictMode.VmPolicy.Builder builder = new StrictMode.VmPolicy.Builder();
        StrictMode.setVmPolicy(builder.build());

        binding.webview.getSettings().setJavaScriptEnabled(false);
        CookieManager.getInstance().setAcceptCookie(true);
        binding.webview.getSettings().setBuiltInZoomControls(true);
        binding.webview.getSettings().setDisplayZoomControls(false);
        binding.webview.getSettings().setAllowFileAccess(false);
        binding.webview.getSettings().setAllowContentAccess(false);
        binding.webview.getSettings().setDatabaseEnabled(true);
        binding.webview.getSettings().setDomStorageEnabled(true);

        String intentUrl = getIntent().hasExtra("url") ? getIntent().getStringExtra("url") : null;
        String initialUrl = getSafeInitialUrl(intentUrl);
        oringialDomain = Uri.parse(initialUrl).getHost();
        updateJavaScriptPolicyForUrl(initialUrl);
        binding.webview.loadUrl(initialUrl);

        binding.webview.setWebViewClient(new WebViewClient() {
            @Override
            public boolean shouldOverrideUrlLoading(WebView view, WebResourceRequest request) {
                Uri url = request.getUrl();
                String scheme = url != null ? url.getScheme() : null;
                String domain = url != null ? url.getHost() : null;

                if (!isAllowedWebScheme(scheme) || domain == null) {
                    logBlockedNavigation(url, "invalid_scheme_or_host");
                    return true;
                }

                if (!domain.equals(oringialDomain)) {
                    Intent openInBrowser = new Intent();
                    openInBrowser.setAction(Intent.ACTION_VIEW);
                    openInBrowser.setData(url);
                    startActivity(openInBrowser);
                    logBlockedNavigation(url, "blocked_cross_origin");
                    return true;
                }

                updateJavaScriptPolicyForUrl(url.toString());
                return false;
            }
        });

        binding.webview.setOnScrollChangeListener((v, scrollX, scrollY, oldScrollX, oldScrollY) -> {
            if (scrollY > oldScrollY) {
                if (binding.btnClose.getVisibility() == View.VISIBLE) {
                    TransitionManager.beginDelayedTransition(binding.main);
                    binding.btnClose.setVisibility(View.GONE);
                }
            } else if (scrollY < oldScrollY) {
                if (binding.btnClose.getVisibility() == View.GONE) {
                    TransitionManager.beginDelayedTransition(binding.main);
                    binding.btnClose.setVisibility(View.VISIBLE);
                }
            }
        });

        binding.btnClose.setOnClickListener(v -> finish());

        getOnBackPressedDispatcher().addCallback(this, new OnBackPressedCallback(true) {
            @Override
            public void handleOnBackPressed() {
                if (binding.webview.canGoBack()) {
                    binding.webview.goBack();
                } else {
                    finish();
                }
            }
        });
    }

    private String getSafeInitialUrl(String incomingUrl) {
        if (isValidInitialUrl(incomingUrl)) {
            return incomingUrl.trim();
        }
        logBlockedNavigation(incomingUrl != null ? Uri.parse(incomingUrl) : null, "invalid_initial_url_fallback");
        return AppConfig.vectrasWebsite;
    }

    private boolean isValidInitialUrl(String rawUrl) {
        if (rawUrl == null || rawUrl.trim().isEmpty()) {
            return false;
        }
        Uri uri = Uri.parse(rawUrl.trim());
        String scheme = uri.getScheme();
        String host = uri.getHost();
        if (scheme == null || host == null) {
            return false;
        }
        if ("file".equalsIgnoreCase(scheme)
                || "javascript".equalsIgnoreCase(scheme)
                || "content".equalsIgnoreCase(scheme)
                || "data".equalsIgnoreCase(scheme)) {
            return false;
        }
        return isAllowedWebScheme(scheme);
    }

    private boolean isAllowedWebScheme(String scheme) {
        if (scheme == null) {
            return false;
        }
        if ("https".equalsIgnoreCase(scheme)) {
            return true;
        }
        return BuildConfig.DEBUG && "http".equalsIgnoreCase(scheme);
    }

    private void updateJavaScriptPolicyForUrl(String rawUrl) {
        Uri uri = Uri.parse(rawUrl);
        String host = uri.getHost();
        String appHost = Uri.parse(AppConfig.vectrasWebsite).getHost();
        boolean enableJs = host != null && (host.equals(oringialDomain) || host.equals(appHost));
        binding.webview.getSettings().setJavaScriptEnabled(enableJs);
    }

    private void logBlockedNavigation(Uri uri, String reason) {
        String scheme = uri != null ? uri.getScheme() : "null";
        String host = uri != null ? uri.getHost() : "null";
        Log.w(TAG, "Blocked web navigation. reason=" + reason + ", scheme=" + scheme + ", host=" + host);
    }
}
