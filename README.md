Once you have downloaded the Zip file, it should be extracted into your Arduino Libraries folder and renamed remove the extra "-master" from the folder name.

For full instructions see the Wiki icon on the right.

NOTES
=====
1) Make sure you know how many characters fit across your message area, this will be usefull when planning your display.	
2) If you want your display to start without any visible text, insert "display width" of spaces at the start of the array.	
3) If your display area is taller than a single character height any partially rendered character at the right hand side
   will be skipped and the rendering will continue with the next whole character on the left hand side a "line" below.
   This is mainly useful for a SCROLL_UP/DOWN display. If multiple lines are required for a SCROLL_LEFT display it is
   neccessary to use two instances of cLEDText placed one over the other with their own text arrays.	
4) If you want left-right readable text when using SCROLL_RIGHT reverse the characters in your text array.	
5) To just keep rendering static text, use SetText() and UpdateText() at the same time and ignore point 2).		
