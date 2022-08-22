# StefFly-Rotary-Mouse
Arduino file(s) for the SteFly rotary encoder for OpenVario

Work here is based on the .ino files provided by Stefan Langer at https://www.stefly.aero/2019/11/28/custom-user-input-options-for-stefly-rotary-module/

I have adapted (and slightly cleaned up) the original .ino file for rotary plus mouse. 

Adaptation is primarily around debouncing the rotary encoders. I have changed debouncing to use milliseconds instead of simple delays (which didn't help
a lot on my system). Change `debounceTime`to a different value if you are unhappy with mouse movements. The Logic for button presses is untouch and may
benefit from a similar logic.

Changed behaviour of "M" button:
In case move movements are active and M is pressed, mouse mode is suspended. I.e. normal "menu moves" apply while in the menu. Pressing "X" will restore
mouse mode.

A simple linear mouse acceleration logic has been added to allow for faster mouse movements based on Rotary Knob speed. It is based on a simple
linear function defined by the min time/max move and max time/min move points on a simple linear graph (see function `calculateAcceleration` for details.

Cleanup covers mostly code fragments oviously left over from some previous versions of the file(s) and compression of the main if-then-else logic around
mouse active in the main loop. Still mre cleanup to do - mainly moving all encoder specific stuff into Encoder class and add more methods to encoder class
if needed.

Use at your own risk.
