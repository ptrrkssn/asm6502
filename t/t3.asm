	ORG $100
FOO:
FIE:	EQU 10+20
	LDA #$10
	RTS
BAR:
	DW FOO
	DW FIE
	END
