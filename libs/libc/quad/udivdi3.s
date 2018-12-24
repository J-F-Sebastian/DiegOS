.file "udivdi3.s"

.globl udivdi3
.type memcpy,@function

.section .text

/*
 * uint64_t udivdi3 (uint64_t d, uint64_t u)
 *
 * STACK
 *
 * d[32:63] 4
 * d[0:31]  8
 * u[32:63] 12
 * u[0:31]  16
 */
udivdi3:

movl    4(%esp), %edx
movl    8(%esp), %eax
movl    12(%esp), %ecx
movl    16(%esp), %ebx

cmpl    %edx, %ecx
/* Test: if d[32:63] is greater than u[32:63] compute further */
jg      udiv_compute
/* Test: if d[32:63] is less than u[32:63] return 0 */
jl      udiv_zero
/* Test: if d[32:63] is equal to u[32:63] and they are null, compute further */
cmpl    $0x0, %edx
je      udiv_compute32
/* Test: if d[0:31] is greater than or equal to u[0:31] return 1, else 0 */
cmpl    %eax, %ebx
jl      udiv_zero
movl    $0x0, %edx
movl    $0x1, %eax
ret

udiv_zero:
movl    $0x0, %edx
movl    $0x0, %eax
ret

udiv_compute:

udiv_compute32:
div     %ebx
ret
