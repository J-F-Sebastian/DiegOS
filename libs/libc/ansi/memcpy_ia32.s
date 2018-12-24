.file "memcpy.s"

.globl memcpy
.type memcpy,@function

.section .text

/*
 * void *memcpy (void *s1, const void *s2, size_t n)
 */
memcpy:

movl    %edi, %eax
movl    %esi, %edx
movl    4(%esp), %edi
movl    8(%esp), %esi

movl    %edi, %ecx
xorl    %esi, %ecx
andl    $3, %ecx
movl    12(%esp), %ecx
cld
jne     .Lunaligned

cmpl    $3, %ecx
jbe     .Lunaligned

testl   $3, %esi
je      1f
movsb
decl    %ecx
testl   $3, %esi
je      1f
movsb
decl    %ecx
testl   $3, %esi
je      1f
movsb
decl    %ecx
1:
movl    %ecx, %ebx
shrl    $2, %ecx
andl    $3, %ebx
rep     movsl
movl    %ebx, %ecx
rep     movsb

.Lend:
movl    %eax, %edi
movl    %edx, %esi
movl    4(%esp), %eax

ret

/* When we come here the pointers do not have the same
alignment or the length is too short.  No need to optimize for
aligned memory accesses. */
.Lunaligned:
shrl    $1, %ecx
jnc     1f
movsb
1:
shrl    $1, %ecx
jnc     2f
movsw
2:
rep
movsl
jmp     .Lend
ret
