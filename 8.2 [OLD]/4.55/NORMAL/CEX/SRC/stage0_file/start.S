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

#include <lv2/macros.h.S>
#include <lv2/symbols.h>

#define CALL(addr) \
	MEM_BASE(%r0); \
	LOAD_LABEL(%r0, %r0, addr); \
	mtctr	%r0; \
	bctrl;

	.text
	.section .start,"ax",@progbits
	.align 3

	.global _start
	
_start:

stdu      %r1, -0x100(%r1)
mflr      %r0
std       %r0, 0x110(%r1)
stdu %r1, -0xf0(%r1)
CALL(0x2767DC)
std       %r26, 0x68(%r1)
mr %r26, %r3
cmplwi cr7, %r3, 1
bgt cr7,3f

2:
	std 	%rtoc, 0x28(%sp)
	clrrdi 	%rtoc, %rtoc, 32
	oris 	%rtoc, %rtoc, __toc@h
	ori 	%rtoc, %rtoc, __toc@l
	bl 	main
	ld	%rtoc, 0x28(%sp)

3:
mr %r3, %r26
ld       %r26, 0x68(%r1)
ld        %r0, 0x110(%r1)
addi      %r1, %r1, 0x100
mtlr      %r0
blr

