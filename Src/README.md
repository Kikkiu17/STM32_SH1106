# Font generation
"The Dot Factory" has been used to generate the fonts (http://www.eran.io/the-dot-factory-an-lcd-font-and-image-generator/). To generate a font, open the program, choose the font you like, write some characters
(that will be generated) and then change the following settings (button next to "Generate"): Flip/Rotate to 90°, Height (Y) to Fixed, Width (X) to Tightest. \
There is no need to generate a space character or separator, everything is handled in the library. Separator character is usually 1/2 the height in pages of the font, while the space character is double that.\
\
There are two fonts already generated that you can use, a big one (that contains only numbers) and a smaller one (that contains all letters, but no symbols). You can try using those before generating a custom
one. You have to check that all characters used are in the font, otherwise those that aren't won't be printed and undefined behavior can occur.

## How to add a font
After you have generated a font, you can copy all three structs (Bitmaps, Descriptors and Info) to the `font.c` file. You can change the FONT_INFO struct name to whatever you like, for example you can rename it 
from "microsoftYaHei_36ptFontInfo" to "YaHei_36pt".\
\
Then, in `font.h`, add the new font at the end of the file, by writing `extern const FONT_INFO font_name;`.

# Bitmap images
You can use any program to generate bitmaps for an image. For this project, I specifically used "LCD Assistant". The byte orientation has to be set to Vertical, with 8 pixels per byte.\
The background has to be white, while the visible part has to be black; the background won't be printed in the display, so if there's something already printed underneath the image area, it will be seen. If you
want to completely overwrite that area, draw a black rectangle, then the image.
