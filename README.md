# Life-Launch

This is meant for the ESP-32 Cheap Yellow Display (320x240 pixels).
The slider image was designed on Canva (320x80 pixels) for a range from 20-100 ft. 
Use load_image.ino to upload the image of the slider to the RAM, change line 92 to modify the link of the slider image (I used imgur for image link).
Upload display.ino to the ESP-32 Display, this loads slider image from RAM, calculates angle based on distance selected, and draws an arrow showing which direction to move the launcher to get the required angle. Displays green and buzzes when launcher is ready.