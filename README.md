## Project title
MANGO RUN
## Team members
Nancy Zou Vivian Zhu
## Project description

We used an accelerometer to identify the user's physical jumps and crouches and reflected that on the monitor which showed a delightful CS107E version of the d…We used an accelerometer to identify the user's physical jumps and crouches and reflected that on the monitor which showed a delightful CS107E version of the dinosaur game. The accelerometer was in a designed CAD case that was a belt around the waist. The dinosaur game included mango coins that could be collected and randomized obstacles for the user to jump on.

## Member contribution
A short description of the work performed by each member of the team.

Vivian: I worked on setting up the accelerometer with the i2c protocol, determining the detecting jump/squat algorithm, and the helping integrate the header files into the main graphics library. I tested the thresholds for jumps and squats by graphing the resultant data (the magnitude of the x,y,z) into the google sheets, and found the graph patterns and common trends. A trend I noticed was the clear value difference in the acceleration magnitude when the graph showed a peak. I helped create the score mechanisms and loaded the code on the SD card. We also worked together on the ending screen and adjusting parameters in graphics to fit the best seamless jumps. 

## References
Cite any references/resources that inspired/influenced your project. 
If your submission incorporates external contributions such as adopting 
code/circuit/design from the work of others/AI, be sure to clearly
delineate what portion of the work was derivative and what portion is 
your original work.


https://github.com/hasanunlu/i2c_demo_for_HiFive1/blob/master/i2c_demo/i2c_demo.c 
We used this script for the structure of the i2c driver to interpret the accelerometer data in a bitwise fashion, and reading it appropriately. I replaced their functions with the awesome cs107e's i2c driver library and extrapolated the data to use. I also used it to establish an accelerometer initializatio function. 

## Self-evaluation
How well was your team able to execute on the plan in your proposal?  
Any trying or heroic moments you would like to share? Of what are you particularly proud:
the effort you put into it? the end product? the process you followed?
what you learned along the way? Tell us about it!



## Photos
You are encouraged to submit photos/videos of your project in action. 
Add the files and commit to your project repository to include along with your submission.
