# Toolchain Licenses (Termux ARM64 Orchestrator)

Este documento centraliza origem e licença dos componentes de toolchain exigidos no bootstrap local.

| Component | Distribution / Package | Source | License |
| --- | --- | --- | --- |
| JDK | `openjdk-21` (Termux package) | Termux package repository (`https://packages.termux.dev`) / OpenJDK upstream (`https://openjdk.org`) | GPL-2.0-only WITH Classpath-exception-2.0 |
| Android SDK (cmdline-tools / platform-tools / build-tools / platforms) | Android SDK packages instalados via `sdkmanager` | Google Android SDK repository (`https://dl.google.com/android/repository/`) | Android Software Development Kit License Agreement |
| Android NDK | `ndk;27.2.12479018` | Google Android SDK repository (`https://dl.google.com/android/repository/`) | Android NDK License (Android SDK License Agreement + third-party notices distribuídos no NDK) |
| CMake (Android SDK) | `cmake;3.22.1` | Google Android SDK repository (`https://dl.google.com/android/repository/`) | BSD-3-Clause (upstream CMake) + termos de distribuição do Android SDK |

## Observações

- As versões efetivas de SDK/NDK/CMake podem ser sobrescritas por variáveis de ambiente no bootstrap (`ANDROID_NDK_VERSION`, `ANDROID_CMAKE_VERSION`, etc.).
- O inventário executável com versão + hash + origem fica em `toolchain-manifests/toolchain-bom.json`.
