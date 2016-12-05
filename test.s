	.text
	.intel_syntax noprefix
	.def	 main;
	.scl	2;
	.type	32;
	.endef
	.globl	main
	.p2align	4, 0x90
main:                                   # @main
.Ltmp0:
.seh_proc main
# BB#0:                                 # %entry
	sub	rsp, 136
.Ltmp1:
	.seh_stackalloc 136
.Ltmp2:
	.seh_endprologue
	mov	dword ptr [rsp + 60], 0
	mov	qword ptr [rsp + 48], rdx
	mov	dword ptr [rsp + 44], ecx
	mov	dword ptr [rsp + 100], 10
	mov	dword ptr [rsp + 96], 10
	mov	dword ptr [rsp + 92], 10
	mov	dword ptr [rsp + 88], 10
	mov	ecx, 10
	movd	xmm0, ecx
	movd	xmm1, dword ptr [rsp + 96] # xmm1 = mem[0],zero,zero,zero
	punpckldq	xmm1, xmm0      # xmm1 = xmm1[0],xmm0[0],xmm1[1],xmm0[1]
	movd	xmm0, dword ptr [rsp + 92] # xmm0 = mem[0],zero,zero,zero
	movd	xmm2, dword ptr [rsp + 100] # xmm2 = mem[0],zero,zero,zero
	punpckldq	xmm2, xmm0      # xmm2 = xmm2[0],xmm0[0],xmm2[1],xmm0[1]
	punpckldq	xmm2, xmm1      # xmm2 = xmm2[0],xmm1[0],xmm2[1],xmm1[1]
	movaps	xmmword ptr [rsp + 64], xmm2
	movaps	xmmword ptr [rsp], xmm2
	movaps	xmmword ptr [rsp + 112], xmm2
	lea	rdx, [rsp + 16]
	mov	qword ptr [rsp + 104], rdx
	movaps	xmm0, xmmword ptr [rsp + 112]
	movaps	xmmword ptr [rsp + 16], xmm0
	xor	eax, eax
	add	rsp, 136
	ret
	.seh_handlerdata
	.text
.Ltmp3:
	.seh_endproc


