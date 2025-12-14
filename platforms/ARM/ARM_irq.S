/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * ARM_irq IRQ service routine
 */

#include "arm.h"

.text
.global ARM_irq
.global ARM_fiq

.func ARM_irq
ARM_irq:
/* adjust lr */
SUB	r14, r14, #4
/* save context */
STMFD r13!, {r0-r3, r12, r14}
B	ARM_irq_service
/* restore context, moving lr into pc and restoring the cpsr too */
LDMFD	r13!, {r0-r3, r12, pc}^
.endfunc

.func ARM_fiq
ARM_fiq:
/* adjust lr */
SUB	r14, r14, #4
/* save context */
STMFD r13!, {r0-r3, r12, r14}
B	ARM_fiq_service
/* restore context, moving lr into pc and restoring the cpsr too */
LDMFD	r13!, {r0-r3, r12, pc}^
.endfunc
