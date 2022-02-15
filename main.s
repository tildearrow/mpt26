.include "2600.inc"

PAL=1
patLen=$60

m26_ptrL=$80
m26_ptrR=$82

m26_posL=$84
m26_posR=$85
m26_indexL=$86
m26_indexR=$87
m26_timeL=$88
m26_timeR=$89

.segment "CODE"

start:
  sei
  cld
  clv
  ldx #$ff
  txs
  lda #0
clear:
  sta VSYNC,x
  dex
  bmi clear
init:
  lda mpt26_offsLl
  sta m26_ptrL
  lda mpt26_offsLh
  sta m26_ptrL+1

  lda mpt26_offsRl
  sta m26_ptrR
  lda mpt26_offsRh
  sta m26_ptrR+1

  ldx #1
  stx m26_indexL
  stx m26_indexR
  stx m26_timeL
  stx m26_timeR
loop:
  lda #$02
  sta VBLANK
  sta WSYNC
  sta WSYNC
  sta WSYNC
  sta VSYNC
  sta WSYNC
  sta WSYNC
  lda #$00
  sta WSYNC
  sta VSYNC
.if .defined(PAL)
  lda #$35
.else
  lda #$29
.endif
  sta TIM64T
  jsr mpt26_playL
  jsr mpt26_playR
: lda INTIM
  sta WSYNC
  bne :-

  lda #$00
  sta VBLANK
.if .defined(PAL)
  lda #$13
.else
  lda #$10
.endif

  sta T1024T
: sta WSYNC
  lda INTIM
  bne :-
  
  lda #$02
  sta VBLANK
  lda #$15
  sta TIM64T
: lda INTIM
  sta WSYNC
  bne :-

  jmp loop

mpt26_playL:
  dec m26_timeL
  beq :+
  rts
: ldy m26_posL
  lda (m26_ptrL),y
  tax
  and #$20
  beq :+
  txa
  and #$0f
  sta AUDV0
  lda #1
  sta m26_timeL
  iny
  bvc postplayL
: txa
  and #$40
  bne :+
  txa
  and #$1f
  sta m26_timeL
  bvc :++
: txa
  and #$1f
  sta AUDF0
  sta PF1
  lda #1
  sta m26_timeL
: iny
  txa
  and #$80
  beq postplayL
  lda (m26_ptrL),y
  and #$0f
  sta AUDV0
  lda (m26_ptrL),y
  sta COLUPF
  lsr
  lsr
  lsr
  lsr
  sta AUDC0
  iny

postplayL:
  sty m26_posL
  txa
  bne :++
nextPatternL:
  lda #1
  sta m26_timeL
  ldx m26_indexL
  lda mpt26_offsLl,x
  sta m26_ptrL
  lda mpt26_offsLh,x
  ; check for end of song
  bne :+
  lda #0
  sta m26_indexL
  jmp nextPatternL
: sta m26_ptrL+1
  inx
  stx m26_indexL
  lda #0
  sta m26_posL
  jmp mpt26_playL

: rts

mpt26_playR:
  dec m26_timeR
  beq :+
  rts
: ldy m26_posR
  lda (m26_ptrR),y
  tax
  and #$20
  beq :+
  txa
  and #$0f
  sta AUDV1
  lda #1
  sta m26_timeR
  iny
  bvc postplayR
: txa
  and #$40
  bne :+
  txa
  and #$1f
  sta m26_timeR
  bvc :++
: txa
  and #$1f
  sta AUDF1
  sta PF2
  lda #1
  sta m26_timeR
: iny
  txa
  and #$80
  beq postplayR
  lda (m26_ptrR),y
  and #$0f
  sta AUDV1
  lda (m26_ptrR),y
  sta COLUBK
  lsr
  lsr
  lsr
  lsr
  sta AUDC1
  iny

postplayR:
  sty m26_posR
  txa
  bne :++
nextPatternR:
  lda #1
  sta m26_timeR
  ldx m26_indexR
  lda mpt26_offsRl,x
  sta m26_ptrR
  lda mpt26_offsRh,x
  ; check for end of song
  bne :+
  lda #0
  sta m26_indexR
  jmp nextPatternR
: sta m26_ptrR+1
  inx
  stx m26_indexR
  lda #0
  sta m26_posR
  jmp mpt26_playR

: rts

.segment "SONG"

.incbin "export/build/m26data.bin"

mpt26_offsLl:
.incbin "export/build/m26offLl.bin"
mpt26_offsLh:
.incbin "export/build/m26offLh.bin"

mpt26_offsRl:
.incbin "export/build/m26offRl.bin"
mpt26_offsRh:
.incbin "export/build/m26offRh.bin"

.segment "VECTOR"

.word start
