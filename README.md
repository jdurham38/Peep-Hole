Building out my custom ring camera that is apartment friendly. 

My main purpose for this is because nearly every door in my apartment building has a peep hole... exluding mine. 
So i was like ya know what let me take a crack at building a custom ring camera.

My main goals for this are as follows.

1. well obviosuly a camera lol
2. the camera cannot be wired into a wall outlet so i need a sustainable power source that I will not have to replace or recharge every day.
3. I do not need to record any video so I do not need a database to store mp4
4. I do not need it on at all times which is where the motion sensor comes in.

hardware used

- 3 6v solar panels
- esp-32 cam w/ wifi & BT
- PIR Sensor
- 12v lithium ion battery


Most of the code is within the main.cpp file but could most likely be consolidated into more modular files. 

Two header files, 1 for the registers and the other for the simple http web server.

- Quick note I want to use a secure web server for the camera but I am still debating since the camera is only acting as a peep hole for my apartment door.
- Another note, yes I will be changing the server address to hide my internet creds and on this topic I'll most likely use https lol.


  Pictures to come!!!
