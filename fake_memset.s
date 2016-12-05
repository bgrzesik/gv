	.text
	.intel_syntax noprefix
	.def	 sprintf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,sprintf
	.globl	sprintf
	.p2align	4, 0x90
sprintf:                                # @sprintf
.Ltmp0:
.seh_proc sprintf
# BB#0:                                 # %entry
	sub	rsp, 72
.Ltmp1:
	.seh_stackalloc 72
.Ltmp2:
	.seh_endprologue
	mov	qword ptr [rsp + 104], r9
	mov	qword ptr [rsp + 96], r8
	mov	qword ptr [rsp + 64], rdx
	mov	qword ptr [rsp + 56], rcx
	lea	rcx, [rsp + 96]
	mov	qword ptr [rsp + 40], rcx
	mov	rdx, qword ptr [rsp + 64]
	mov	r8, qword ptr [rsp + 56]
	xor	eax, eax
	mov	r9d, eax
	mov	qword ptr [rsp + 32], rcx # 8-byte Spill
	mov	rcx, r8
	mov	r8, r9
	mov	r9, qword ptr [rsp + 32] # 8-byte Reload
	call	_vsprintf_l
	mov	dword ptr [rsp + 52], eax
	add	rsp, 72
	ret
	.seh_handlerdata
	.section	.text,"xr",discard,sprintf
.Ltmp3:
	.seh_endproc

	.def	 vsprintf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,vsprintf
	.globl	vsprintf
	.p2align	4, 0x90
vsprintf:                               # @vsprintf
.Ltmp4:
.seh_proc vsprintf
# BB#0:                                 # %entry
	sub	rsp, 72
.Ltmp5:
	.seh_stackalloc 72
.Ltmp6:
	.seh_endprologue
	mov	qword ptr [rsp + 64], r8
	mov	qword ptr [rsp + 56], rdx
	mov	qword ptr [rsp + 48], rcx
	mov	rdx, qword ptr [rsp + 64]
	mov	r8, qword ptr [rsp + 56]
	mov	rax, rsp
	mov	qword ptr [rax + 32], rdx
	xor	r9d, r9d
                                        # 
	mov	rdx, -1
	call	_vsnprintf_l
	nop
	add	rsp, 72
	ret
	.seh_handlerdata
	.section	.text,"xr",discard,vsprintf
.Ltmp7:
	.seh_endproc

	.def	 _snprintf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_snprintf
	.globl	_snprintf
	.p2align	4, 0x90
_snprintf:                              # @_snprintf
.Ltmp8:
.seh_proc _snprintf
# BB#0:                                 # %entry
	sub	rsp, 88
.Ltmp9:
	.seh_stackalloc 88
.Ltmp10:
	.seh_endprologue
	mov	qword ptr [rsp + 120], r9
	mov	qword ptr [rsp + 80], r8
	mov	qword ptr [rsp + 72], rdx
	mov	qword ptr [rsp + 64], rcx
	lea	rcx, [rsp + 120]
	mov	qword ptr [rsp + 48], rcx
	mov	r8, qword ptr [rsp + 80]
	mov	rdx, qword ptr [rsp + 72]
	mov	r9, qword ptr [rsp + 64]
	mov	qword ptr [rsp + 40], rcx # 8-byte Spill
	mov	rcx, r9
	mov	r9, qword ptr [rsp + 40] # 8-byte Reload
	call	_vsnprintf
	mov	dword ptr [rsp + 60], eax
	add	rsp, 88
	ret
	.seh_handlerdata
	.section	.text,"xr",discard,_snprintf
.Ltmp11:
	.seh_endproc

	.def	 _vsnprintf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_vsnprintf
	.globl	_vsnprintf
	.p2align	4, 0x90
_vsnprintf:                             # @_vsnprintf
.Ltmp12:
.seh_proc _vsnprintf
# BB#0:                                 # %entry
	sub	rsp, 72
.Ltmp13:
	.seh_stackalloc 72
.Ltmp14:
	.seh_endprologue
	mov	qword ptr [rsp + 64], r9
	mov	qword ptr [rsp + 56], r8
	mov	qword ptr [rsp + 48], rdx
	mov	qword ptr [rsp + 40], rcx
	mov	rdx, qword ptr [rsp + 64]
	mov	r8, qword ptr [rsp + 56]
	mov	r9, qword ptr [rsp + 48]
	mov	rax, rsp
	mov	qword ptr [rax + 32], rdx
	xor	r10d, r10d
	mov	eax, r10d
	mov	rdx, r9
	mov	r9, rax
	call	_vsnprintf_l
	nop
	add	rsp, 72
	ret
	.seh_handlerdata
	.section	.text,"xr",discard,_vsnprintf
.Ltmp15:
	.seh_endproc

	.def	 main;
	.scl	2;
	.type	32;
	.endef
	.text
	.globl	main
	.p2align	4, 0x90
main:                                   # @main
.Ltmp16:
.seh_proc main
# BB#0:                                 # %entry
	push	rbx
.Ltmp17:
	.seh_pushreg 3
	mov	eax, 4240
	call	__chkstk
	sub	rsp, rax
.Ltmp18:
	.seh_stackalloc 4240
.Ltmp19:
	.seh_endprologue
	mov	dword ptr [rsp + 4220], 0
	mov	qword ptr [rsp + 4208], rdx
	mov	dword ptr [rsp + 4204], ecx
	lea	rcx, [rip + "??_C@_0BL@FBIMDKDC@Starting?4?4?4?5?$CIfake_memset?$CJ?6?$AA@"]
	call	printf
	mov	dword ptr [rsp + 60], eax # 4-byte Spill
	call	clock
	mov	dword ptr [rsp + 92], eax
	mov	dword ptr [rsp + 4200], 0
.LBB4_1:                                # %for.cond
                                        # =>This Inner Loop Header: Depth=1
	cmp	dword ptr [rsp + 4200], 1000000
	jge	.LBB4_4
# BB#2:                                 # %for.body
                                        #   in Loop: Header=BB4_1 Depth=1
	mov	edx, 4294967295
	mov	eax, 4096
	mov	r8d, eax
	lea	rcx, [rsp + 96]
	call	fake_memset
	mov	qword ptr [rsp + 48], rax # 8-byte Spill
# BB#3:                                 # %for.inc
                                        #   in Loop: Header=BB4_1 Depth=1
	mov	eax, dword ptr [rsp + 4200]
	add	eax, 1
	mov	dword ptr [rsp + 4200], eax
	jmp	.LBB4_1
.LBB4_4:                                # %for.end
	call	clock
	mov	dword ptr [rsp + 88], eax
	mov	ecx, dword ptr [rsp + 92]
	sub	eax, ecx
	lea	rcx, [rip + "??_C@_0BG@DJECEBNJ@Test?5took?5?$CFld?5cycles?6?$AA@"]
	mov	edx, eax
	call	printf
	movzx	edx, byte ptr [rsp + 1120]
	sub	edx, 255
	sete	r8b
	movzx	r9d, r8b
	lea	rcx, [rip + "??_C@_0N@KEJCBMPH@Success?3?5?$CFd?6?$AA@"]
	mov	dword ptr [rsp + 44], edx # 4-byte Spill
	mov	edx, r9d
	mov	dword ptr [rsp + 40], eax # 4-byte Spill
	call	printf
	xorps	xmm0, xmm0
	movaps	xmmword ptr [rsp + 64], xmm0
	mov	dword ptr [rsp + 4236], 0
	mov	dword ptr [rsp + 4232], 7
	lea	rcx, [rsp + 64]
	mov	qword ptr [rsp + 4224], rcx
	mov	edx, dword ptr [rsp + 4232]
	mov	ecx, dword ptr [rsp + 4236]
	mov	dword ptr [rsp + 36], eax # 4-byte Spill
	mov	eax, edx
	#APP
	cpuid
	#NO_APP
	mov	dword ptr [rsp + 64], eax
	mov	dword ptr [rsp + 68], ebx
	mov	dword ptr [rsp + 72], ecx
	mov	dword ptr [rsp + 76], edx
	movsxd	r10, dword ptr [rsp + 68]
	movabs	r11, 68719476736
	and	r10, r11
	lea	rcx, [rip + "??_C@_03PMGGPEJJ@?$CFd?6?$AA@"]
	mov	rdx, r10
	call	printf
	xor	r9d, r9d
	mov	dword ptr [rsp + 32], eax # 4-byte Spill
	mov	eax, r9d
	add	rsp, 4240
	pop	rbx
	ret
	.seh_handlerdata
	.text
.Ltmp20:
	.seh_endproc

	.def	 printf;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,printf
	.globl	printf
	.p2align	4, 0x90
printf:                                 # @printf
.Ltmp21:
.seh_proc printf
# BB#0:                                 # %entry
	sub	rsp, 72
.Ltmp22:
	.seh_stackalloc 72
.Ltmp23:
	.seh_endprologue
	mov	qword ptr [rsp + 104], r9
	mov	qword ptr [rsp + 96], r8
	mov	qword ptr [rsp + 88], rdx
	mov	qword ptr [rsp + 64], rcx
	lea	rcx, [rsp + 88]
	mov	qword ptr [rsp + 48], rcx
	mov	rdx, qword ptr [rsp + 64]
	mov	eax, 1
	mov	qword ptr [rsp + 40], rcx # 8-byte Spill
	mov	ecx, eax
	mov	qword ptr [rsp + 32], rdx # 8-byte Spill
	call	__acrt_iob_func
	xor	ecx, ecx
	mov	r8d, ecx
	mov	rcx, rax
	mov	rdx, qword ptr [rsp + 32] # 8-byte Reload
	mov	r9, qword ptr [rsp + 40] # 8-byte Reload
	call	_vfprintf_l
	mov	dword ptr [rsp + 60], eax
	add	rsp, 72
	ret
	.seh_handlerdata
	.section	.text,"xr",discard,printf
.Ltmp24:
	.seh_endproc

	.def	 fake_memset;
	.scl	3;
	.type	32;
	.endef
	.text
	.p2align	4, 0x90
fake_memset:                            # @fake_memset
.Ltmp25:
.seh_proc fake_memset
# BB#0:                                 # %entry
	push	rdi
.Ltmp26:
	.seh_pushreg 7
	sub	rsp, 48
.Ltmp27:
	.seh_stackalloc 48
.Ltmp28:
	.seh_endprologue
	mov	qword ptr [rsp + 16], r8
	mov	dword ptr [rsp + 12], edx
	mov	qword ptr [rsp], rcx
	mov	r8, qword ptr [rsp + 16]
	mov	al, byte ptr [rsp + 12]
	mov	qword ptr [rsp + 40], r8
	mov	byte ptr [rsp + 39], al
	mov	qword ptr [rsp + 24], rcx
	mov	al, byte ptr [rsp + 39]
	mov	r8, qword ptr [rsp + 40]
	mov	rdi, rcx
	mov	rcx, r8
	#APP
	rep
	stosb	byte ptr es:[rdi], al
	#NO_APP
	mov	rax, qword ptr [rsp]
	add	rsp, 48
	pop	rdi
	ret
	.seh_handlerdata
	.text
.Ltmp29:
	.seh_endproc

	.def	 _vsprintf_l;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_vsprintf_l
	.globl	_vsprintf_l
	.p2align	4, 0x90
_vsprintf_l:                            # @_vsprintf_l
.Ltmp30:
.seh_proc _vsprintf_l
# BB#0:                                 # %entry
	sub	rsp, 72
.Ltmp31:
	.seh_stackalloc 72
.Ltmp32:
	.seh_endprologue
	mov	qword ptr [rsp + 64], r9
	mov	qword ptr [rsp + 56], r8
	mov	qword ptr [rsp + 48], rdx
	mov	qword ptr [rsp + 40], rcx
	mov	rdx, qword ptr [rsp + 64]
	mov	r9, qword ptr [rsp + 56]
	mov	r8, qword ptr [rsp + 48]
	mov	rax, rsp
	mov	qword ptr [rax + 32], rdx
	mov	rdx, -1
	call	_vsnprintf_l
	nop
	add	rsp, 72
	ret
	.seh_handlerdata
	.section	.text,"xr",discard,_vsprintf_l
.Ltmp33:
	.seh_endproc

	.def	 _vsnprintf_l;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_vsnprintf_l
	.globl	_vsnprintf_l
	.p2align	4, 0x90
_vsnprintf_l:                           # @_vsnprintf_l
.Ltmp34:
.seh_proc _vsnprintf_l
# BB#0:                                 # %entry
	sub	rsp, 152
.Ltmp35:
	.seh_stackalloc 152
.Ltmp36:
	.seh_endprologue
	mov	rax, qword ptr [rsp + 192]
	mov	qword ptr [rsp + 144], rax
	mov	qword ptr [rsp + 136], r9
	mov	qword ptr [rsp + 128], r8
	mov	qword ptr [rsp + 120], rdx
	mov	qword ptr [rsp + 112], rcx
	mov	rax, qword ptr [rsp + 144]
	mov	rcx, qword ptr [rsp + 136]
	mov	r9, qword ptr [rsp + 128]
	mov	r8, qword ptr [rsp + 120]
	mov	rdx, qword ptr [rsp + 112]
	mov	qword ptr [rsp + 96], rax # 8-byte Spill
	mov	qword ptr [rsp + 88], rcx # 8-byte Spill
	mov	qword ptr [rsp + 80], r9 # 8-byte Spill
	mov	qword ptr [rsp + 72], r8 # 8-byte Spill
	mov	qword ptr [rsp + 64], rdx # 8-byte Spill
	call	__local_stdio_printf_options
	mov	rax, qword ptr [rax]
	or	rax, 1
	mov	rcx, rax
	mov	rdx, qword ptr [rsp + 64] # 8-byte Reload
	mov	r8, qword ptr [rsp + 72] # 8-byte Reload
	mov	r9, qword ptr [rsp + 80] # 8-byte Reload
	mov	rax, qword ptr [rsp + 88] # 8-byte Reload
	mov	qword ptr [rsp + 32], rax
	mov	rax, qword ptr [rsp + 96] # 8-byte Reload
	mov	qword ptr [rsp + 40], rax
	call	__stdio_common_vsprintf
	mov	dword ptr [rsp + 108], eax
	cmp	dword ptr [rsp + 108], 0
	jge	.LBB8_2
# BB#1:                                 # %cond.true
	mov	eax, 4294967295
	mov	dword ptr [rsp + 60], eax # 4-byte Spill
	jmp	.LBB8_3
.LBB8_2:                                # %cond.false
	mov	eax, dword ptr [rsp + 108]
	mov	dword ptr [rsp + 60], eax # 4-byte Spill
.LBB8_3:                                # %cond.end
	mov	eax, dword ptr [rsp + 60] # 4-byte Reload
	add	rsp, 152
	ret
	.seh_handlerdata
	.section	.text,"xr",discard,_vsnprintf_l
.Ltmp37:
	.seh_endproc

	.def	 __local_stdio_printf_options;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,__local_stdio_printf_options
	.globl	__local_stdio_printf_options
	.p2align	4, 0x90
__local_stdio_printf_options:           # @__local_stdio_printf_options
.Ltmp38:
.seh_proc __local_stdio_printf_options
# BB#0:                                 # %entry
.Ltmp39:
	.seh_endprologue
	lea	rax, [rip + __local_stdio_printf_options._OptionsStorage]
	ret
	.seh_handlerdata
	.section	.text,"xr",discard,__local_stdio_printf_options
.Ltmp40:
	.seh_endproc

	.def	 _vfprintf_l;
	.scl	2;
	.type	32;
	.endef
	.section	.text,"xr",discard,_vfprintf_l
	.globl	_vfprintf_l
	.p2align	4, 0x90
_vfprintf_l:                            # @_vfprintf_l
.Ltmp41:
.seh_proc _vfprintf_l
# BB#0:                                 # %entry
	sub	rsp, 104
.Ltmp42:
	.seh_stackalloc 104
.Ltmp43:
	.seh_endprologue
	mov	qword ptr [rsp + 96], r9
	mov	qword ptr [rsp + 88], r8
	mov	qword ptr [rsp + 80], rdx
	mov	qword ptr [rsp + 72], rcx
	mov	rdx, qword ptr [rsp + 96]
	mov	r9, qword ptr [rsp + 88]
	mov	r8, qword ptr [rsp + 80]
	mov	qword ptr [rsp + 64], rcx # 8-byte Spill
	mov	qword ptr [rsp + 56], rdx # 8-byte Spill
	mov	qword ptr [rsp + 48], r9 # 8-byte Spill
	mov	qword ptr [rsp + 40], r8 # 8-byte Spill
	call	__local_stdio_printf_options
	mov	rcx, qword ptr [rax]
	mov	rax, rsp
	mov	rdx, qword ptr [rsp + 56] # 8-byte Reload
	mov	qword ptr [rax + 32], rdx
	mov	rdx, qword ptr [rsp + 64] # 8-byte Reload
	mov	r8, qword ptr [rsp + 40] # 8-byte Reload
	mov	r9, qword ptr [rsp + 48] # 8-byte Reload
	call	__stdio_common_vfprintf
	nop
	add	rsp, 104
	ret
	.seh_handlerdata
	.section	.text,"xr",discard,_vfprintf_l
.Ltmp44:
	.seh_endproc

	.section	.rdata,"dr",discard,"??_C@_0BL@FBIMDKDC@Starting?4?4?4?5?$CIfake_memset?$CJ?6?$AA@"
	.globl	"??_C@_0BL@FBIMDKDC@Starting?4?4?4?5?$CIfake_memset?$CJ?6?$AA@" # @"\01??_C@_0BL@FBIMDKDC@Starting?4?4?4?5?$CIfake_memset?$CJ?6?$AA@"
"??_C@_0BL@FBIMDKDC@Starting?4?4?4?5?$CIfake_memset?$CJ?6?$AA@":
	.asciz	"Starting... (fake_memset)\n"

	.section	.rdata,"dr",discard,"??_C@_0BG@DJECEBNJ@Test?5took?5?$CFld?5cycles?6?$AA@"
	.globl	"??_C@_0BG@DJECEBNJ@Test?5took?5?$CFld?5cycles?6?$AA@" # @"\01??_C@_0BG@DJECEBNJ@Test?5took?5?$CFld?5cycles?6?$AA@"
"??_C@_0BG@DJECEBNJ@Test?5took?5?$CFld?5cycles?6?$AA@":
	.asciz	"Test took %ld cycles\n"

	.section	.rdata,"dr",discard,"??_C@_0N@KEJCBMPH@Success?3?5?$CFd?6?$AA@"
	.globl	"??_C@_0N@KEJCBMPH@Success?3?5?$CFd?6?$AA@" # @"\01??_C@_0N@KEJCBMPH@Success?3?5?$CFd?6?$AA@"
"??_C@_0N@KEJCBMPH@Success?3?5?$CFd?6?$AA@":
	.asciz	"Success: %d\n"

	.section	.rdata,"dr",discard,"??_C@_03PMGGPEJJ@?$CFd?6?$AA@"
	.globl	"??_C@_03PMGGPEJJ@?$CFd?6?$AA@" # @"\01??_C@_03PMGGPEJJ@?$CFd?6?$AA@"
"??_C@_03PMGGPEJJ@?$CFd?6?$AA@":
	.asciz	"%d\n"

	.lcomm	__local_stdio_printf_options._OptionsStorage,8,8 # @__local_stdio_printf_options._OptionsStorage

