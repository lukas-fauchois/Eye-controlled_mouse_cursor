#include <X11/Xlib.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "opencv2/opencv.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/objdetect/objdetect.hpp"

using namespace cv;

String face_cascade = "./haarcascade_frontalface_alt.xml";
String eyes_cascade = "./haarcascade_eye_tree_eyeglasses.xml";

CascadeClassifier cascade_frontalface;
CascadeClassifier cascade_eyes; //pour les yeux

void click (Display *display, int button)
{
  // Create and setting up the event
  XEvent event;
  memset (&event, 0, sizeof (event));
  event.xbutton.button = button;
  event.xbutton.same_screen = True;
  event.xbutton.subwindow = DefaultRootWindow (display);
  while (event.xbutton.subwindow)
    {
      event.xbutton.window = event.xbutton.subwindow;
      XQueryPointer (display, event.xbutton.window,
		     &event.xbutton.root, &event.xbutton.subwindow,
		     &event.xbutton.x_root, &event.xbutton.y_root,
		     &event.xbutton.x, &event.xbutton.y,
		     &event.xbutton.state);
    }
  // Press
  event.type = ButtonPress;
  if (XSendEvent (display, PointerWindow, True, ButtonPressMask, &event) == 0)
    fprintf (stdout, "Error to send the event!\n");
  XFlush (display);
  usleep (1);
  // Release
  event.type = ButtonRelease;
  if (XSendEvent (display, PointerWindow, True, ButtonReleaseMask, &event) == 0)
    fprintf (stdout, "Error to send the event!\n");
  XFlush (display);
  usleep (1);
}

int main(int, char**)
{
    //Taille du frame
    int frame_width = 240;
    int frame_height = 180;

    //Choisir l'écran
    Display* dpy = XOpenDisplay(0);
    int scr = XDefaultScreen(dpy);
    Window root_window = XRootWindow(dpy, scr);

    //Calculer la taille de l'ecran
    int height = DisplayHeight(dpy, scr);
    int width  = DisplayWidth(dpy, scr);
    //std::cout << "Screen size : " << width << "x" << height << std::endl;

    //Initialiser la position du curseur au milieu de l'écran
    int c, cursor_x, cursor_y;
    cursor_x = width/2;
    cursor_y = height/2;
    int posx = cursor_x;
    c = 0;

    //Déclaration de certaines variables : abscisse des deux yeux et centre entre les deux
    int center, left_x, right_x;

    Point cursor_point(frame_width/2, frame_height/2);
    vector<Mat> images;
    vector<int> labels;

    if(!cascade_frontalface.load(face_cascade)){printf("--(!)Error loading face(!)--"); return -1;};
    if(!cascade_eyes.load(eyes_cascade)){printf("--(!)Error loading eyes(!)--"); return -1;};

    VideoCapture cap(0);
    if(!cap.isOpened())
        return -1;

    Mat grImage;

    //Variable permettant le calcul de l'inclinaison de la tête grâce à la position des yeux
    int radius;
    //Nombre d'yeux ouverts
    int open_eye;

    namedWindow("Face and eyes",1);
    for(;;)
    {
        Mat frame;
        Mat small_frame;
        Size size(frame_width,frame_height);

        cap >> frame; // get a new frame from camera

        resize(frame, small_frame, size); //conversion du frame calculé en un plus petit frame pour améliorer la fluidité

        cvtColor(small_frame, grImage, CV_BGR2GRAY);
        vector< Rect_<int> > faces;
        cascade_frontalface.detectMultiScale(grImage, faces);

        for(int i = 0; i < faces.size(); i++) {
          Rect face_i = faces[i];
          rectangle(small_frame, face_i, CV_RGB(0,255,0), 1);

          Point center_face(faces[i].x + faces[i].width*0.5 , faces[i].y + faces[i].height*0.5);

          int min = frame_height*0.475;
          int max = frame_height*0.525;

          if(center_face.y > max){  //si la tête se penche vers le bas
            cursor_point.y += 10;
            cursor_y += 15;
          }
          else if(center_face.y < min){
            cursor_point.y -= 10;
            cursor_y -= 15;
          }

          vector< Rect_<int> > eyes;

          cascade_eyes.detectMultiScale(grImage, eyes);

          if(eyes.size() == 2){
            open_eye = eyes.size();

            if(eyes[1].x < eyes[2].x){
              Point left (faces[i].x + eyes[1].x + eyes[1].width*0.5 , faces[i].y + eyes[1].y + eyes[1].height*0.5);
              Point right (faces[i].x + eyes[2].x + eyes[2].width*0.5 , faces[i].y + eyes[2].y + eyes[2].height*0.5);
              left_x = left.x;
              right_x = right.x;
              center = left_x + (right_x - left_x)*0.5;
              radius = left.y - right.y;
            }
            else{
              Point right (faces[i].x + eyes[1].x + eyes[1].width*0.5 , faces[i].y + eyes[1].y + eyes[1].height*0.5);
              Point left (faces[i].x + eyes[2].x + eyes[2].width*0.5 , faces[i].y + eyes[2].y + eyes[2].height*0.5);
              left_x = left.x;
              right_x = right.x;
              center = left_x + (right_x - left_x)*0.5;
              radius = left.y - right.y;
            }
          }

          if(radius > 3){
            cursor_point.x += 10;
            cursor_x += 20;
          }
          else if(radius < -3){
            cursor_point.x -= 10;
            cursor_x -= 20;
          }

          if((open_eye == 2) && (eyes.size()) == 1){
            open_eye = 1;
            if(faces[i].x + eyes[1].x + eyes[1].width*0.5 < center) { click(dpy, Button1); std::cout << "Left click" << std::endl; }
            else { click(dpy, Button3); std::cout << "Right click" << std::endl; }
          }

          for(int j = 0; j < eyes.size(); j++){
            Rect eyes_j = eyes[j];
            rectangle(small_frame, eyes_j, CV_RGB(0, 0, 255), 1);
          }
	       }

        XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, cursor_x, cursor_y);
        XFlush(dpy);
        circle(small_frame, cursor_point, 2, CV_RGB(255, 0, 0), 4, 8, 0 );


        imshow("Face and eyes", small_frame);
        if(waitKey(30) >= 0) break;
    }

    return 0;
}
