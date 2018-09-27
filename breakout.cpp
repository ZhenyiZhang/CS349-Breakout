// CS 349 Fall 2018
// A1: Breakout code sample
// You may use any or all of this code in your assignment!
// See makefile for compiling instructions

#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include <stdlib.h>
#include <string>
#include <cmath>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;



// X11 structures
Display* display;
Window window;
Colormap cm;


// fixed frames per second animation
int FPS = 45;

//some variables
int brick_width = 128;
int brick_height = 50;
int brick_row = 6;
int brick_per_row = 1280 / brick_width;
int brick_num = brick_row * brick_per_row;

//indicator for start menu
bool game_start = true;
//indicator for game over menu
bool game_finish = false;
bool win = false;

//how many pixels the ball pass through each time
int speed;

//direction of ball when ball meets bricks
int updir = 1;
int downdir = 2;
int leftdir = 3;
int rightdir = 4;
int away = 5;

//ballsize
int bsize = 20;


// get current time
unsigned long now() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}


class brick {
public:
	//coordinates
	int x;
	int y;

	//color of the brick
	int r;
	int g;
	int b;

	//decide weather ball meets the brick and the direction the ball came from
	int touch (int posx, int posy) {
		if(posx - bsize / 2 <= x + brick_width && posx +  bsize/2 > x + brick_width
			&& posy >= y && posy <= y + brick_height) {
				return rightdir;
			}
			else if(posx +  bsize / 2 >= x && posx - bsize/2 < x
				&& posy >= y && posy <= y + brick_height) {
					return leftdir;
				}
				else if(posy -  bsize / 2 <= y + brick_height && posy +  bsize / 2 > y + brick_height
					&&  posx >= x && posx <= x + brick_width) {
						return downdir;
					}
					else if(posy + bsize/2 >= y && posy - bsize/2 < y && posx >= x && posx <= x + brick_width) {
						return updir;
					}
					else {
						return away;
					}
				}
			};

		//where we store all the bricks
		vector<brick> brick_list;

		// entry point
		int main(int argc, char *argv[]) {

			// create window
			display = XOpenDisplay("");
			if (display == NULL) exit (-1);
			int screennum = DefaultScreen(display);
			long background = WhitePixel(display, screennum);
			long foreground = BlackPixel(display, screennum);
			cm = XDefaultColormap(display, screennum);
			window = XCreateSimpleWindow(display, DefaultRootWindow(display),
			10, 10, 1280, 800, 2, foreground, background);

			// set events to monitor and display window
			XSelectInput(display, window, ButtonPressMask | KeyPressMask);
			XMapRaised(display, window);
			XFlush(display);

			//build all the bricks
			int x_count = brick_width;
			int y_count = brick_height;
			for(int i = 0; i < brick_row; i++) {
				while(x_count < 1280 - brick_width){
					brick temp;
					temp.x = x_count;
					temp.y = y_count;
					temp.r = rand() % 90000+ 100;
					temp.g = rand() % 90000+ 100;
					temp.b = rand() % 90000+ 100;
					brick_list.push_back(temp);
					x_count += brick_width;
				}
				y_count += brick_height;
				x_count = brick_width;
			}
			y_count = brick_height;
			// ball postition, size, and velocity
			XPoint ballPos;
			ballPos.x = 20;
			ballPos.y = 20;
			int ballSize = bsize;

			XPoint ballDir;
			ballDir.x = 5;
			ballDir.y = 5;

			//block position, size
			XPoint rectPos;
			rectPos.x = 640;
			rectPos.y = 700;

			//command line instructions
			if(argc == 3) {
				FPS = atoi(argv[1]);
				ballDir.x = atoi(argv[2]);
				ballDir.y = atoi(argv[2]);
				if(FPS < 10 || FPS > 60) {
					cout << "FPS RANGE 10-60!" << endl;
					return 0;
				}
				if(ballDir.x < 1 || ballDir.x > 15) {
					cout << "SPEED RANGE 1-15!" << endl;
					return 0;
				}
			}
			speed = ballDir.x * ballDir.x;

			// create gc for drawing
			GC gc = XCreateGC(display, window, 0, 0);
			// GC gc2 = XCreateGC(display, window, 0, 0);
			XWindowAttributes w;
			XGetWindowAttributes(display, window, &w);

			// save time of last window paint
			unsigned long lastRepaint = 0;

			// event handle for current event
			XEvent event;



			gc = XCreateGC(display, window, 0, 0);

			// DOUBLE BUFFER
			// create bimap (pximap) to us a other buffer
			int depth = DefaultDepth(display, DefaultScreen(display));
			Pixmap	buffer = XCreatePixmap(display, window, w.width, w.height, depth);

			bool useBuffer = true;


			//user cannot resize the window
			XSizeHints *hints = XAllocSizeHints();
			hints->flags = PMinSize|PMaxSize;
			hints->max_width = w.width;
			hints->max_height = w.height;
			hints->min_width = w.width;
			hints->min_height = w.height;
			XSetWMNormalHints(display,window,hints);
			XFree(hints);

			// event loop
			while ( true ) {
				int brick_per_row = 1280 / brick_width;
				int brick_num = brick_row * brick_per_row;

				//game winning
				if(brick_list.size() == 0) {
					brick_list.clear();
					win = true;
					ballPos.x = 20;
					ballPos.y = 20;
					ballDir.x = atoi(argv[2]);
					ballDir.y = atoi(argv[2]);
				}

				// vector<int> v;
				gc = XCreateGC(display, window, 0, 0);
				// process if we have any events
				if (XPending(display) > 0) {

					XNextEvent( display, &event );

					switch ( event.type ) {

						// mouse button press
						case ButtonPress:
						useBuffer = !useBuffer;
						cout << "double buffer " << useBuffer << endl;
						break;

						// any keypress
						case KeyPress:
						KeySym key;
						char text[10];
						int i = XLookupString( (XKeyEvent*)&event, text, 10, &key, 0 );

						// move right
						if ( i == 1 && text[0] == 'd' ) {
							rectPos.x += 18;
							if(ballDir.x >=8) {
								rectPos.x += 25;
							}
						}

						//restart the game
						if ( i == 1 && text[0] == 'e' ) {
							gc = XCreateGC(display, window, 0, 0);
							brick_row = 1;
							brick_width = 400;
							brick_list.clear();
							x_count = brick_width;
							y_count = brick_height;
							for(int i = 0; i < brick_row; i++) {
								while(x_count < 1280 - brick_width){
									brick temp;
									temp.x = x_count;
									temp.y = y_count;
									temp.r = rand() % 90000+ 100;
									temp.g = rand() % 90000+ 100;
									temp.b = rand() % 90000+ 100;
									brick_list.push_back(temp);
									x_count += brick_width;
								}
								y_count += brick_height;
								x_count = brick_width;
							}
							y_count = 0;
						}

						// move left
						if ( i == 1 && text[0] == 'a' ) {
							rectPos.x -= 18;
							if(ballDir.x >=8) {
								rectPos.x -= 25;
							}
						}

						// quit game
						if ( i == 1 && text[0] == 'q' ) {
							XCloseDisplay(display);
							exit(0);
						}

						//to test when all bricks are destroyed
						if ( i == 1 && text[0] == 't' ) {
							brick_list.clear();
						}

						//begin the game
						if ( i == 1 && text[0] == 'b' ) {
							game_start = false;
							gc = XCreateGC(display, window, 0, 0);
						}

						//restart the game
						if ( i == 1 && text[0] == 'r' ) {
							gc = XCreateGC(display, window, 0, 0);
							game_finish = false;
							win = false;
							brick_list.clear();
							x_count = brick_width;
							y_count = brick_height;
							for(int i = 0; i < brick_row; i++) {
								while(x_count < 1280 - brick_width){
									brick temp;
									temp.x = x_count;
									temp.y = y_count;
									temp.r = rand() % 90000+ 100;
									temp.g = rand() % 90000+ 100;
									temp.b = rand() % 90000+ 100;
									brick_list.push_back(temp);
									x_count += brick_width;
								}
								y_count += brick_height;
								x_count = brick_width;
							}
							y_count = 0;
						}

						break;
					}
				}

				unsigned long end = now();	// get current time in microsecond

				if (end - lastRepaint > 1000000 / FPS) {

					Pixmap pixmap;

					if (useBuffer) {

						pixmap = buffer;
						// draw into the buffer
						// note that a window and a pixmap are “drawables”
						XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
						XFillRectangle(display, pixmap, gc,
							0, 0, w.width, w.height);
						} else {
							pixmap = window;
							// clear background
							XClearWindow(display, pixmap);
						}

						//game start interface
						if(game_start) {
							string info_death = "Press [B] to begin BREAKOUT!";
							XColor color_cell;
							color_cell.red = rand() % 90000+ 1;
							color_cell.green = rand() % 90000+ 1;
							color_cell.blue = rand() % 90000 + 1;
							color_cell.flags = DoRed | DoGreen | DoBlue;
							XAllocColor(display, cm, &color_cell);
							XSetForeground(display, gc, color_cell.pixel);
							XDrawString(display, window, gc,
								580, 400, info_death.c_str(), info_death.length());
								XFlush( display );
								continue;
							}


							if(game_finish || win) {
								string info_death;
								if(win) {
									info_death = "You Win! Press [R] Play Again";
								} else {
									info_death = "Press [R] to Play Again";
								}
								XColor color_cell;
								color_cell.red = rand() % 90000+ 1;
								color_cell.green = rand() % 90000+ 1;
								color_cell.blue = rand() % 90000 + 1;
								color_cell.flags = DoRed | DoGreen | DoBlue;
								XAllocColor(display, cm, &color_cell);
								XSetForeground(display, gc, color_cell.pixel);
								XDrawString(display, window, gc,
									580, 400, info_death.c_str(), info_death.length());
									XFlush( display );
									continue;
								}

								//to destroy bricks when ball meet them
								for(vector<brick>::iterator it = brick_list.begin(); it != brick_list.end(); ++it) {
									if(brick_list.size() == 0) break;
									if(it->touch(ballPos.x,ballPos.y) == rightdir || it->touch(ballPos.x,ballPos.y) == leftdir) {
										brick_list.erase(it);
										if(brick_list.size() == 0) break;
										ballDir.x = - ballDir.x;
										it = brick_list.begin();
										continue;
									} else if(it->touch(ballPos.x,ballPos.y) == updir || it->touch(ballPos.x,ballPos.y) == downdir) {
										brick_list.erase(it);
										if(brick_list.size() == 0) break;
										ballDir.y = - ballDir.y;
										it = brick_list.begin();
										continue;
									}
									XColor color_cell;
									color_cell.red = it->r;
									color_cell.green = it->g;
									color_cell.blue = it->b;
									color_cell.flags = DoRed | DoGreen | DoBlue;
									XAllocColor(display, cm, &color_cell);
									XSetForeground(display, gc, color_cell.pixel);
									XDrawRectangle(display, pixmap, gc, it->x, it->y, brick_width - 3, brick_height -3);
									XFillRectangle(display, pixmap, gc, it->x, it->y, brick_width - 3 , brick_height - 3);
								}

								gc = XCreateGC(display, window, 0, 0);

								// draw rectangle
								int recwidth = 150;
								int recheight = 10;
								XFillRectangle(display, pixmap, gc, rectPos.x, rectPos.y, recwidth, recheight);

								//print infos
								string info_fps = "FPS " + to_string(FPS);
								XDrawString(display, pixmap, gc,
									50, 750, info_fps.c_str(), info_fps.length());
									string info_score = "Score ";
									int score = (brick_num - brick_list.size()) * 10;
									info_score = info_score + to_string(score);
									XDrawString(display, pixmap, gc,
										50, 770, info_score.c_str(), info_score.length());
										string info_button = "[Move Left: A] [Move Right: D] [Quit: Q] [Easy Mode: E] [Restart: R]";
										XDrawString(display, pixmap, gc,
											50, 790, info_button.c_str(), info_button.length());
											string info_developer = "Name: Zhenyi Zhang User ID: z528zhan";
											XDrawString(display, pixmap, gc,
												1000, 790, info_developer.c_str(), info_developer.length());

												// draw ball from centre
												XFillArc(display, pixmap, gc,
													ballPos.x - ballSize/2,
													ballPos.y - ballSize/2,
													ballSize, ballSize,
													0, 360*64);


													// update ball position
													ballPos.x += ballDir.x;
													ballPos.y += ballDir.y;

													//ball meets line
													if ((ballPos.y + ballSize/2 >= rectPos.y)
													&& (ballPos.y + ballSize/2 <= rectPos.y + 10)
													&& ballPos.x >= rectPos.x
													&& ballPos.x <= rectPos.x + recwidth) {
														int direc = rand() % 10 + 0;
														if(direc % 2 == 0) {
															ballDir.y = -(rand() % (int)(sqrt(speed)+1) + (atoi(argv[2])/2));
															if(ballDir.x > 0) {
																ballDir.x = (int)sqrt(speed - ballDir.y * ballDir.y);
															} else {
																ballDir.x = -(int)sqrt(speed - ballDir.y * ballDir.y);
															}
															ballPos.y -= 10;
														} else {
															ballDir.y = -ballDir.y;
															ballDir.x = -ballDir.x;
															ballPos.y -= 10;
														}
													}

													// bounce ball
													if (ballPos.x + ballSize/2 > w.width ||
														ballPos.x - ballSize/2 < 0) {
															if(ballPos.y + ballSize/2 > rectPos.y) {
																ballPos.x = 20;
																ballPos.y = 20;
																ballDir.x = atoi(argv[2]);
																ballDir.y = atoi(argv[2]);
																game_finish = true;
																continue;
															}
															ballDir.x = -ballDir.x;
														}
														if (ballPos.y + ballSize/2 > w.height ||
															ballPos.y - ballSize/2 < 0) {
																if(ballPos.y + ballSize/2 > rectPos.y){
																	ballPos.x = 20;
																	ballPos.y = 20;
																	ballDir.x = atoi(argv[2]);
																	ballDir.y = atoi(argv[2]);
																	game_finish = true;
																	continue;
																}
																ballDir.y = -ballDir.y;

															}


															// copy buffer to window
															if (useBuffer) {
																XCopyArea(display, pixmap, window, gc,
																	0, 0, 1280, 800,  // region of pixmap to copy
																	0, 0); // position to put top left corner of pixmap in window
																}

																XFlush( display );

																lastRepaint = now(); // remember when the paint happened
															}

															// IMPORTANT: sleep for a bit to let other processes work
															if (XPending(display) == 0) {
																usleep(1000000 / FPS - (now() - lastRepaint));
															}
														}
														XCloseDisplay(display);
													}
