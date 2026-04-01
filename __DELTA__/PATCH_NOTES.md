# PATCH NOTES

- BUG1: ASM paths corrected in sources_rmr_core.cmake.
- BUG2: Android-only group ensured via core-minus-host partition.
- BUG3: root CMake option help strings added.
- BUG4: bitraf.c kept out of core list and compiled only as dedicated static lib.
- BUG5: vectras-ci workflow rebuilt as valid YAML workflow.
- BUG6: build-zip updated to actions v4.
- BUG7: android-arm642 now builds librmr.a then links demo.
- TAREFA C/H: engine-ci now runs manifest check and invariant validation using tools/invariant_golden.txt.
- TAREFA F/G: android-arm64-build cache gradle; zipdrop validates SHA256 manifest.
