package com.vectras.vm.localization

import com.vectras.vm.network.NetworkEndpoints

/**
 * Represents a downloadable language module for the app.
 * Language modules are designed to be downloaded on-demand to save device resources.
 */
data class LanguageModule(
    val languageCode: String,
    val languageName: String,
    val nativeName: String,
    val downloadUrl: String,
    val version: Int,
    val sizeBytes: Long,
    val isBuiltIn: Boolean = false,
    var isDownloaded: Boolean = false,
    var downloadProgress: Int = 0
) {
    companion object {
        /**
         * Returns the list of supported language modules.
         * English is built-in and always available.
         * Other languages can be downloaded as modules.
         */
        fun getSupportedLanguages(): List<LanguageModule> = listOf(
            LanguageModule(
                languageCode = "en",
                languageName = "English",
                nativeName = "English",
                downloadUrl = "",
                version = 1,
                sizeBytes = 0,
                isBuiltIn = true,
                isDownloaded = true
            ),
            LanguageModule(
                languageCode = "pt",
                languageName = "Portuguese",
                nativeName = "Português",
                downloadUrl = NetworkEndpoints.languageModule("pt"),
                version = 1,
                sizeBytes = 50000,
                isBuiltIn = false
            ),
            LanguageModule(
                languageCode = "es",
                languageName = "Spanish",
                nativeName = "Español",
                downloadUrl = NetworkEndpoints.languageModule("es"),
                version = 1,
                sizeBytes = 50000,
                isBuiltIn = false
            ),
            LanguageModule(
                languageCode = "fr",
                languageName = "French",
                nativeName = "Français",
                downloadUrl = NetworkEndpoints.languageModule("fr"),
                version = 1,
                sizeBytes = 50000,
                isBuiltIn = false
            ),
            LanguageModule(
                languageCode = "de",
                languageName = "German",
                nativeName = "Deutsch",
                downloadUrl = NetworkEndpoints.languageModule("de"),
                version = 1,
                sizeBytes = 50000,
                isBuiltIn = false
            )
        )
        
        /**
         * Get language module by code
         */
        fun getByCode(code: String): LanguageModule? {
            return getSupportedLanguages().find { it.languageCode == code }
        }
    }
}
