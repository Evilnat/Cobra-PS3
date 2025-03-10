/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
// Ported by Evilnat from 3141card's ps2_netemu research, all credits goes to him

	.text
	.section .start,"ax",@progbits
	.align 3

	.global _sm_get_temperature
	
_sm_get_temperature:

	stdu 	%sp, -0xA0(%sp)
	mflr    %r0
	std     %r31, 0x98(%sp)
	addi    %r31, %r3, 0x2980
	std     %r27, 0x78(%sp)
	std     %r28, 0x80(%sp)
	std     %r29, 0x88(%sp)
	std     %r30, 0x90(%sp)
	std     %r0, 0xB0(%sp)
	lbz     %r10, 0(%r31)
	mr      %r29, %r3
	mr      %r28, %r5
	mr      %r27, %r4
	cmpwi   cr7, %r10, 0
	bne     cr7, 3f
	addi    %r30, %r3, 0x2990
	li      %r0, 0x10
	li      %r9, 8
	li      %r11, 1
	stb     %r11, 0(%r31)
	stb     %r0, 1(%r31)
	stw     %r9, 4(%r31)
	li      %r0, 0xD
	li      %r9, 0
	sth     %r0, 8(%r31)
	sth     %r9, 0xA(%r31)
	stw     %r10, 0xC(%r31)
	sth     %r10, 2(%r31)
	li      %r4, 0
	li      %r5, 8
	mr      %r3, %r30
	bl 		-0xD6538
	nop
	stb     %r11, 0(%r30)
	stb     %r27, 1(%r30)
	lbz     %r0, 0(%r31)
	cmpwi   cr7, %r0, 0	
	beq     cr7, 3f
	li      %r3, 2
	mr      %r4, %r31
	li      %r5, 0x18
	bl 		-0xD6E90
	lbz     %r0, 0(%r31)
	cmpwi   cr7, %r0, 0
	beq     cr7, 2f

1:
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	mr      %r31, %r31
	lbz     %r0, 0(%r31)
	cmpwi   cr7, %r0, 0
	bne     cr7, 1b

2:
	lhz     %r0, 0x3194(%r29)
	sth     %r0, 0(%r28)
	li      %r3, 0
	stb     %r3, 0(%r31)
	ld      %r0, 0xB0(%sp)
	ld      %r27, 0x78(%sp)
	ld      %r28, 0x80(%sp)
	mtlr    %r0
	ld      %r29, 0x88(%sp)
	ld      %r30, 0x90(%sp)
	ld      %r31, 0x98(%sp)
	addi    %sp, %sp, 0xA0
	blr

3:
	li 		%r3, 1
	bl 		-0xDD5B4

	.long 0x00

.global _draw_temperature
	
_draw_temperature:

	stdu 	%sp, -0x130(%sp)
	mflr    %r0
	std     %r27, 0x108(%sp)
	ld 	 	%r27, -0x3378(%r2)
	std 	%r25, 0xF8(%sp)
	std 	%r29, 0x118(%sp)
	addi 	%r29, %sp, 0x70
	std 	%r0, 0x140(%sp)
	std 	%r26, 0x100(%sp)
	std 	%r28, 0x110(%sp)
	lis		%r26, 0x241
	ori		%r26, %r26, 0xE30C
	lbz		%r9, 0(%r26)
	cmpwi   cr7, %r9, 0
	addi    %r9, %r9, -1
	beq     cr7, 2f
	stb 	%r9, 0(%r26)

1:
	lbz       %r6, 1(%r26)
	lbz       %r7, 2(%r26)
	lis       %r5, 0x1E
	ori       %r5, %r5, 0xF124
	li        %r4, 0x80
	mr        %r3, %r29
	bl        -0xD5970
	nop
	ld        %r28, -0x3498(%r2)
	mr        %r4, %r29
	mr        %r3, %r28
	bl        0x8F400
	nop
	li        %r7, 0x20 // Y axis
	li        %r6, 0x40 // X axis
	clrldi    %r5, %r3, 32
	li        %r4, 0
	li        %r3, 0
	bl        0x85EF4
	nop
	ld        %r0, 0x140(%sp)
	ld        %r25, 0xF8(%sp)
	ld        %r26, 0x100(%sp)
	mtlr      %r0
	ld        %r27, 0x108(%sp)
	ld        %r28, 0x110(%sp)
	ld        %r29, 0x118(%sp)
	addi      %sp, %sp, 0x130
	blr	

2:
	li      %r0, 0x3E # '>'
	stb     %r0, 0(%r26)
	mr      %r3, %r27
	li      %r4, 0
	addi    %r5, %r26, 1
	bl      -0x20C
	nop
	mr      %r3, %r27
	li      %r4, 1
	addi    %r5, %r26, 2
	bl      -0x220
	nop
	b       1b

temp_string:
	.string "[ps2_netemu.self]\nCELL: %02d °C\nRSX:   %02d °C"
