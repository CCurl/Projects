Some thoughts on VGA output from a development board

What I understand about VGA:

There are 5 signals, 3 in and 2 out.

in: Red, Green, Blue (continuous analog 0.0-0.7)
out: VSync abd HSync (square 3.3 or 5.0)

Color voltages are 0.0v: black, 0.7: max

There is a ray that crosses the screen over and over. I will call that the "pos".

The pos starts in the upper left, goes straight across, then down a little, back the the left and then left to right again.
When it gets to the bottom, it goes back to the top and starts all over.

There are 2 signals from the VGA monitor that tell the computer where the pos is: VSync and HSync

VSync is high when the row pos is in the visible range (left to right)
HSync is high when the col pos is in the visible range (top to bottom)

The signals go like this:
|VL..VH| #ROWS 0 DO |HL..HH| .. {HL..HH} LOOP {VL..VH}
 V-Start (Each Row)  H-Start     H-End         V-End


Data is to be written to R/G/B while VH and HH  both are hign.

The computer can change the R/G/B voltages as quickily as it wants during that time. It usually uses what is generally referred to as a "pixel-clock" to specify how quickly the voltages are changed. The higher the pixel-clock, the higher the horizontal resolution.
