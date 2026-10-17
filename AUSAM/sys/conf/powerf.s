.data

	.globl	powfail

powfail:				/ entry at priority level 7
	0				/ halt - nought else to do
	br	powfail			/ loop just in case 'continued'
