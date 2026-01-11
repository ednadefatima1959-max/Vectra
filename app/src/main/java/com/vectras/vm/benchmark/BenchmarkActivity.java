package com.vectras.vm.benchmark;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.widget.Toolbar;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import com.vectras.vm.AppConfig;
import com.vectras.vm.R;
import com.vectras.vm.utils.FileUtils;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * BenchmarkActivity - UI for running and viewing benchmark tests.
 * 
 * Provides a professional engineering interface for:
 * - Running the 79-metric benchmark suite
 * - Viewing formal engineering metrics with SI units (MB/s, MFLOPS, ns, etc.)
 * - Displaying device specifications (CPU, RAM, architecture)
 * - Exporting and sharing results
 */
public class BenchmarkActivity extends AppCompatActivity {
    private static final String TAG = "BenchmarkActivity";
    
    // UI Elements
    private TextView tvTotalScore;
    private TextView tvScoreStatus;
    private TextView tvCpuSingleScore;
    private TextView tvCpuMultiScore;
    private TextView tvMemoryScore;
    private TextView tvStorageScore;
    private TextView tvIntegrityScore;
    private TextView tvEmulationScore;
    private LinearLayout layoutCategoryScores;
    private LinearLayout layoutProgress;
    private TextView tvProgressText;
    private LinearLayout btnRunBenchmark;
    private LinearLayout btnViewDetails;
    private LinearLayout btnExportResults;
    private LinearLayout btnShareResults;
    
    // Data
    private VectraBenchmark.BenchmarkResult[] lastResults;
    private final ExecutorService executor = Executors.newSingleThreadExecutor();
    private final Handler mainHandler = new Handler(Looper.getMainLooper());
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_benchmark);
        
        setupToolbar();
        initViews();
        setupListeners();
    }
    
    private void setupToolbar() {
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        Objects.requireNonNull(getSupportActionBar()).setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setDisplayShowHomeEnabled(true);
        toolbar.setTitle(getString(R.string.benchmark));
    }
    
    private void initViews() {
        tvTotalScore = findViewById(R.id.tvTotalScore);
        tvScoreStatus = findViewById(R.id.tvScoreStatus);
        tvCpuSingleScore = findViewById(R.id.tvCpuSingleScore);
        tvCpuMultiScore = findViewById(R.id.tvCpuMultiScore);
        tvMemoryScore = findViewById(R.id.tvMemoryScore);
        tvStorageScore = findViewById(R.id.tvStorageScore);
        tvIntegrityScore = findViewById(R.id.tvIntegrityScore);
        tvEmulationScore = findViewById(R.id.tvEmulationScore);
        layoutCategoryScores = findViewById(R.id.layoutCategoryScores);
        layoutProgress = findViewById(R.id.layoutProgress);
        tvProgressText = findViewById(R.id.tvProgressText);
        btnRunBenchmark = findViewById(R.id.btnRunBenchmark);
        btnViewDetails = findViewById(R.id.btnViewDetails);
        btnExportResults = findViewById(R.id.btnExportResults);
        btnShareResults = findViewById(R.id.btnShareResults);
    }
    
    private void setupListeners() {
        btnRunBenchmark.setOnClickListener(v -> runBenchmark());
        btnViewDetails.setOnClickListener(v -> showDetailedResults());
        btnExportResults.setOnClickListener(v -> exportResults());
        btnShareResults.setOnClickListener(v -> shareResults());
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (item.getItemId() == android.R.id.home) {
            finish();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
    private void runBenchmark() {
        // Show progress
        layoutProgress.setVisibility(View.VISIBLE);
        btnRunBenchmark.setEnabled(false);
        tvScoreStatus.setText(getString(R.string.running_benchmark));
        
        // Run benchmark in background
        executor.execute(() -> {
            try {
                // Run the benchmark
                VectraBenchmark.BenchmarkResult[] results = VectraBenchmark.runAllBenchmarks();
                lastResults = results;
                
                // Get device specifications
                VectraBenchmark.DeviceSpecification deviceSpec = VectraBenchmark.getDeviceSpecification();
                
                // Update UI on main thread
                mainHandler.post(() -> {
                    updateScoreDisplay(results, deviceSpec);
                    layoutProgress.setVisibility(View.GONE);
                    btnRunBenchmark.setEnabled(true);
                    tvScoreStatus.setText(getString(R.string.benchmark_complete));
                    
                    // Show result buttons
                    btnViewDetails.setVisibility(View.VISIBLE);
                    btnExportResults.setVisibility(View.VISIBLE);
                    btnShareResults.setVisibility(View.VISIBLE);
                    layoutCategoryScores.setVisibility(View.VISIBLE);
                });
                
            } catch (Exception e) {
                mainHandler.post(() -> {
                    layoutProgress.setVisibility(View.GONE);
                    btnRunBenchmark.setEnabled(true);
                    Toast.makeText(this, "Benchmark failed: " + e.getMessage(), Toast.LENGTH_LONG).show();
                });
            }
        });
    }
    
    private void updateScoreDisplay(VectraBenchmark.BenchmarkResult[] results, 
                                     VectraBenchmark.DeviceSpecification deviceSpec) {
        // Display device info instead of arbitrary score
        String deviceInfo = deviceSpec.cpuCores + " cores @ " + deviceSpec.getFormattedCpuFreq();
        tvTotalScore.setText(String.valueOf(results.length));
        tvTotalScore.setTextSize(android.util.TypedValue.COMPLEX_UNIT_SP, 32);
        
        // Calculate category summaries with real metrics
        String cpuSingleSummary = getCategorySummary(results, "CPU Single-threaded");
        String cpuMultiSummary = getCategorySummary(results, "CPU Multi-threaded");
        String memorySummary = getCategorySummary(results, "Memory");
        String storageSummary = getCategorySummary(results, "Storage");
        String integritySummary = getCategorySummary(results, "Integrity");
        String emulationSummary = getCategorySummary(results, "Emulation");
        
        tvCpuSingleScore.setText(cpuSingleSummary);
        tvCpuMultiScore.setText(cpuMultiSummary);
        tvMemoryScore.setText(memorySummary);
        tvStorageScore.setText(storageSummary);
        tvIntegrityScore.setText(integritySummary);
        tvEmulationScore.setText(emulationSummary);
    }
    
    /**
     * Get a representative metric value for a category.
     */
    private String getCategorySummary(VectraBenchmark.BenchmarkResult[] results, String category) {
        // Find the first result in this category and use its formatted value
        for (VectraBenchmark.BenchmarkResult r : results) {
            if (r != null && category.equals(r.category())) {
                return r.formattedValue();
            }
        }
        return "N/A";
    }
    
    private void showDetailedResults() {
        if (lastResults == null) {
            Toast.makeText(this, "No results to display", Toast.LENGTH_SHORT).show();
            return;
        }
        
        String report = VectraBenchmark.formatReport(lastResults);
        
        // Create a scrollable text view for the detailed results
        TextView messageView = new TextView(this);
        messageView.setText(report);
        messageView.setTextIsSelectable(true);
        messageView.setTypeface(android.graphics.Typeface.MONOSPACE);
        messageView.setTextSize(9f);
        messageView.setPadding(16, 16, 16, 16);
        
        android.widget.ScrollView scrollView = new android.widget.ScrollView(this);
        scrollView.addView(messageView);
        
        new MaterialAlertDialogBuilder(this)
                .setTitle(getString(R.string.detailed_results))
                .setView(scrollView)
                .setPositiveButton(android.R.string.ok, null)
                .show();
    }
    
    private void exportResults() {
        if (lastResults == null) {
            Toast.makeText(this, "No results to export", Toast.LENGTH_SHORT).show();
            return;
        }
        
        executor.execute(() -> {
            try {
                String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(new Date());
                String fileName = "vectras_benchmark_" + timestamp + ".txt";
                File exportDir = new File(AppConfig.maindirpath);
                if (!exportDir.exists()) {
                    exportDir.mkdirs();
                }
                File exportFile = new File(exportDir, fileName);
                
                // Use detailed report format
                String report = VectraBenchmark.formatDetailedReport(lastResults);
                FileUtils.writeToFile(exportDir.getAbsolutePath(), fileName, report);
                
                mainHandler.post(() -> {
                    Toast.makeText(this, 
                            getString(R.string.results_exported, exportFile.getAbsolutePath()), 
                            Toast.LENGTH_LONG).show();
                });
                
            } catch (Exception e) {
                mainHandler.post(() -> {
                    Toast.makeText(this, getString(R.string.export_failed) + ": " + e.getMessage(), 
                            Toast.LENGTH_LONG).show();
                });
            }
        });
    }
    
    private void shareResults() {
        if (lastResults == null) {
            Toast.makeText(this, "No results to share", Toast.LENGTH_SHORT).show();
            return;
        }
        
        String report = VectraBenchmark.formatReport(lastResults);
        
        Intent shareIntent = new Intent(Intent.ACTION_SEND);
        shareIntent.setType("text/plain");
        shareIntent.putExtra(Intent.EXTRA_SUBJECT, "Vectras VM Professional Benchmark Results");
        shareIntent.putExtra(Intent.EXTRA_TEXT, report);
        startActivity(Intent.createChooser(shareIntent, getString(R.string.share_results)));
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        executor.shutdown();
    }
}
