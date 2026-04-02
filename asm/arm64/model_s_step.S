.text
.global model_s_ax_transform_arm64
.type model_s_ax_transform_arm64, %function
model_s_ax_transform_arm64:
    eor w0, w0, w1
    ret

.section .note.GNU-stack,"",@progbits
