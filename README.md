## Project title
MANGO-RUN
## Team members
Vivian Zhu Nancy Zou 

## Project description
We used an accelerometer to identify the user's physical jumps and crouches and reflected that on the monitor which showed a delightful CS107E version of the dinosaur game. The accelerometer was in a designed CAD case that was a belt around the waist. The dinosaur game included mango coins that could be collected and randomized obstacles for the user to jump on.

## Member contribution
Vivian: I worked on setting up the accelerometer with the i2c protocol, determining the detecting jump/squat algorithm, and the helping integrate the header files into the main graphics library. I tested the thresholds for jumps and squats by graphing the resultant data (the magnitude of the x,y,z) into the google sheets, and found the graph patterns and common trends. A trend I noticed was the clear value difference in the acceleration magnitude when the graph showed a peak. I helped create the score mechanisms and loaded the code on the SD card. We also worked together on the ending screen and adjusting parameters in graphics to fit the best seamless jumps. 

Nancy: I worked on the graphics side and I created the world for the game. I designed the characters and objects and I also implemented the mechanism for how the game would progress. I took inspiration from the original dino game and made it so the character stays in place while the world moves to the left. I implemented gravity in every frame of the game (and moved the character back up to ground when its position went below the ground) so that the jump would feel right. Detecting collisions was also tricky since only overlap between specific pixel were valid collisions. I also designed the CAD case, which took many rounds of testing.

## References
Cite any references/resources that inspired/influenced your project. 
If your submission incorporates external contributions such as adopting 
code/circuit/design from the work of others/AI, be sure to clearly
delineate what portion of the work was derivative and what portion is 
your original work.


https://github.com/hasanunlu/i2c_demo_for_HiFive1/blob/master/i2c_demo/i2c_demo.c 
We used this script for the structure of the i2c driver to interpret the accelerometer data in a bitwise fashion, and reading it appropriately. I replaced their functions with the awesome cs107e's i2c driver library and extrapolated the data to use. I also used it to establish an accelerometer initializatio function. 

## Self-evaluation
We were able to get the basic game working but it was definately trickier than we expected. At first, we thought we could easily detect the difference between a jump and a squat, but since the accelerometer doesn't detect direction and only acceleration, the line turned out to be quite blurry. We wrote the code for implementing a squating mechanism but when it came to testing, it didn't work very smoothly and some smaller jumps would be mistaken for squats. In the end, for the sake of user experience, we decided to not use the squat. Also, we made some changes to the original game plan, such as using the mangos as obsticals rather than coins you collect. 

The integration of our two parts (sensor and graphics) was very satisfying and we felt very proud of ourselves! It was very exciting to see the project slowly coming to life. Overall, we worked together very well and our strategy of working on different parts, then working together to integrate worked well. We spent that few days before the presentation together in Gates and it was really helpful having both of us there to test and debug. 


## Photos
You are encouraged to submit photos/videos of your project in action. 
Add the files and commit to your project repository to include along with your submission.
