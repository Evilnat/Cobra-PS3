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
CALL(0x27C5E8)
std       %r26, 0x68(%r1)
mr %r26, %r3
cmplwi cr7, %r3, 1
bgt cr7,4f
addi      %r4, %r1, 0x70
lis 	  %r3, dev_rebug_stage@highest   
ori 	  %r3, %r3, dev_rebug_stage@higher  
rldicr    %r3, %r3,32,31 
oris      %r3, %r3,dev_rebug_stage@h   
ori       %r3, %r3,dev_rebug_stage@l   
CALL(cellFsStat_symbol)
cmpwi cr7, %r3, 0
bne cr7, 2f
bl main
b 3f

2:
lis %r3, 0x8000
sldi %r3, %r3, 32
oris %r3, %r3, 0x7f
li %r4, 0
lis %r5, 0x1
CALL(memset_symbol)
b 4f

3:
lis       %r0, 0x80000000007f0000@highest
ori 	  %r0, %r0, 0x80000000007f0000@higher  
rldicr    %r0, %r0,32,31 
oris      %r0, %r0,0x80000000007f0000@h   
ori       %r0, %r0,0x80000000007f0000@l   
mtctr %r0
bctrl

4:
mr %r3, %r26
ld       %r26, 0x68(%r1)
ld        %r0, 0x110(%r1)
addi      %r1, %r1, 0x100
mtlr      %r0
blr


dev_rebug_stage:
	.string "/dev_flash/rebug/cobra/stage2.dex"

