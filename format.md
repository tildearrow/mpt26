# format for converted file

every pattern is converted to a string of values, which represent frequency, control (type) and volume (and duration).

the string is read sequentially as follows:

0: bit 7: change V/C
   bit 6: change F
   bit 0-4: freq/length

if change F is set, bits 0-4 are interpreted as frequency change.
if all bits are zero, this marks the end of the pattern.

if change V/C is set, the next byte is read:

1: bit 4-7: control (type)
   bit 0-3: volume
