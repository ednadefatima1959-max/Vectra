.text
.globl model_s_ax_transform_x86_64
.type model_s_ax_transform_x86_64, @function
model_s_ax_transform_x86_64:
    movl %edi, %eax
    xorl %esi, %eax
    ret

.section .note.GNU-stack,"",@progbits
