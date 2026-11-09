/* setjmp/longjmp da MN1 (o32, sem FPU): ra, sp, s0-s8 e gp. O jmp_buf da glibc e maior que isso. */
    .set noreorder
    .text
    .globl _setjmp
    .globl setjmp
    .type _setjmp,@function
_setjmp:
setjmp:
    sw $ra, 0($a0)
    sw $sp, 4($a0)
    sw $s0, 8($a0)
    sw $s1, 12($a0)
    sw $s2, 16($a0)
    sw $s3, 20($a0)
    sw $s4, 24($a0)
    sw $s5, 28($a0)
    sw $s6, 32($a0)
    sw $s7, 36($a0)
    sw $fp, 40($a0)
    sw $gp, 44($a0)
    jr $ra
    move $v0, $zero

    .globl longjmp
    .globl __longjmp_chk
    .type longjmp,@function
longjmp:
__longjmp_chk:
    lw $ra, 0($a0)
    lw $sp, 4($a0)
    lw $s0, 8($a0)
    lw $s1, 12($a0)
    lw $s2, 16($a0)
    lw $s3, 20($a0)
    lw $s4, 24($a0)
    lw $s5, 28($a0)
    lw $s6, 32($a0)
    lw $s7, 36($a0)
    lw $fp, 40($a0)
    lw $gp, 44($a0)
    bnez $a1, 1f
    move $v0, $a1
    li $v0, 1
1:  jr $ra
    nop
