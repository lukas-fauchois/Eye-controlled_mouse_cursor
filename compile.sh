#bin/bash
g++  -I/home/lukas/Documents/IMA4/IHM/opencvfiles/include/opencv -I/home/lukas/Documents/IMA4/IHM/opencvfiles/include/  -L/home/lukas/Documents/IMA4/IHM/opencvfiles/lib -g -o tpIHM  main.cpp -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_video -lopencv_contrib -lopencv_objdetect -lX11
