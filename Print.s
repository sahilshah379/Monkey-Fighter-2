; Print.s
; Student names: Sahil Shah, Ankit Patel
; Last modification date: change this to the last modification date or look very silly
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
; R0=0,    then output "0"
; R0=3,    then output "3"
; R0=89,   then output "89"
; R0=123,  then output "123"
; R0=9999, then output "9999"
; R0=4294967295, then output "4294967295"

Num EQU 0
LCD_OutDec
    PUSH {R1, R2, R3, R4, R5, R6, R11, LR}
    
    SUB SP, #8
    MOV R11, SP
    STR R0, [R11, #Num]
    LDR R0, [R11, #Num]
    
    CMP R0, #10
    BHS elsedec
    MOV R1, R0
    ADD R0, #0x30
    PUSH {R0, R1, R2, R3, R4, R5, R6, LR}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5, R6, LR}
    B donedec
    
elsedec
    MOV R1, #10 
    MOV R4, R0 ; R4 saves copy of R0
    UDIV R0, R1 ; R0 /= 10
    BL LCD_OutDec
    MOV R0, R4 ; restores R0
    UDIV R2, R0, R1 ; R2=R0/R1
    MUL R3, R2, R1 ; R3=(R0/R1)*R1
    SUB R3, R0, R3 ; R3=R0%R1
    MOV R0, R3
    
    ADD R0, #0x30
    PUSH {R0, R1, R2, R3, R4, R5, R6, LR}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5, R6, LR}
    
    
donedec
    ADD SP, #8
    POP {R1, R2, R3, R4, R5, R6, R11, PC}
      ;BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000"
;       R0=3,    then output "0.003"
;       R0=89,   then output "0.089"
;       R0=123,  then output "0.123"
;       R0=9999, then output "9.999"
;       R0>9999, then output "*.***"
; Invariables: This function must not permanently modify registers R4 to R11

var1    EQU 0
var2    EQU 4
var3    EQU 8
var4    EQU 12
link    EQU 16

LCD_OutFix
    PUSH {R1, R2, R3, R4, R5, R6, R11, LR}
    SUB SP, #24
    MOV R11, SP
    STR LR, [R11, #link]

    MOV R1, #1 ; R1 is len
    STR R1, [R11, #var1]
    LDR R1, [R11, #var1]
    MOV R2, R0 ; R2 is c
    STR R2, [R11, #var2]
    LDR R2, [R11, #var2]
lenloop
    CMP R2, #10
    BLO lenloopdone
    MOV R3, #10
    UDIV R2, R3 ; c /= 10
    ADD R1, #1
    B lenloop
    
lenloopdone
    CMP R1, #3
    BGT len_is_4ormore
    
    MOV R4, R0 ; R4 will save a copy of R0 (n)
    MOV R0, #0x30
    
    PUSH {R0, R1, R2, R3, R4, R5}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5}
    
    MOV R0, #0x2E
    
    PUSH {R0, R1, R2, R3, R4, R5}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5}
    
    MOV R5, #3
    SUBS R5, R5, R1 ; R5 <- R5 - R1 and b is R5
    STR R5, [R11, #var3]
    LDR R5, [R11, #var3]
    
print0train
    CMP R5, #0
    BEQ print0traindone
    
    MOV R0, #0x30
    
    PUSH {R0, R1, R2, R3, R4, R5}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5}
    
    SUBS R5, #1
    B print0train
    
print0traindone

    MOV R0, R4 ; reload copy of R0 (n)
    BL LCD_OutDec
    
    B donewiththisfunction
    
len_is_4ormore
    CMP R1, #4
    BNE lenisbiglol
    
    MOV R3, R0 ; R3 is now R0 and thus n
    
    MOV R0, #1000
    UDIV R0, R3, R0 ; R0 = R3 / 1000
    STR R0, [R11, #var4]
    LDR R0, [R11, #var4]
    ADD R0, #0x30
    
    PUSH {R0, R1, R2, R3, R4, R5}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5}
    
    MOV R0, #0x2E
    PUSH {R0, R1, R2, R3, R4, R5}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5}
    
    MOV R4, #100
    UDIV R0, R3, R4 ; R0 = R3 / R4
    MOV R1, #10
    BL modulus ; R2 is x
    MOV R0, R2
    ADD R0, #0x30
    PUSH {R0, R1, R2, R3, R4, R5}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5}
    
    
    MOV R4, #10
    UDIV R0, R3, R4
    MOV R1, #10
    BL modulus ; R2 is x
    MOV R0, R2
    ADD R0, #0x30
    PUSH {R0, R1, R2, R3, R4, R5}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5}
    
    MOV R0, R3
    MOV R1, #10
    BL modulus ; R2 is x
    MOV R0, R2
    
    ADD R0, #0x30
    PUSH {R0, R1, R2, R3, R4, R5}
    BL ST7735_OutChar
    POP {R0, R1, R2, R3, R4, R5}
    
    
    B donewiththisfunction
    
lenisbiglol
    MOV R0, #0x2A
    BL ST7735_OutChar
    MOV R0, #0x2E
    BL ST7735_OutChar
    MOV R0, #0x2A
    BL ST7735_OutChar
    MOV R0, #0x2A
    BL ST7735_OutChar
    MOV R0, #0x2A
    BL ST7735_OutChar
        
    
donewiththisfunction
    LDR LR, [R11, #link]
    ADD SP, #24
    POP {R1, R2, R3, R4, R5, R6, R11, LR}    
     
modulus
    ; R0 and R1 are inputs, R2 is the output
    ; return R2 = R0 % R1
    PUSH {R3, LR}
    UDIV R2, R0, R1 ; R2 = R0/R1
    MUL R2, R2, R1 ; R2 = (R0/R1)*R1
    SUB R2, R0, R2 ; R2 = R0 % R1
    POP {R3, PC}
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
