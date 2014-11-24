#include <iostream>
#include <math.h>
using namespace std;
#include <ncursesw/ncurses.h>

int main()
{
	WINDOW * curs_window = initscr();
	MEVENT * mouse_event;
	raw();
	curs_set(0);
	char cmd = 0;

	int row = 0;
	int col = 0;
	long double SIZEX = 0;
	long double SIZEY = 0;
	int MaxIters = 400;

	getmaxyx(stdscr, SIZEY, SIZEX);
	// getmaxyx(stdscr,row,col);
	// if (row < col)
	// 	SIZE = row;
	// else
	// 	SIZE = col;

	long double RIGHT 	= 1.0;
	long double LEFT 	= -2.0;
	long double TOP 	= 1.0;
	long double BOTTOM 	= -1.0;

	long double zoom = 1.0;

	short x, y, count;
	long double zr, zi, cr, ci;
	long double rsquared, isquared;
	long double centerX = 0.0;
	long double centerY = 0.0;

	while (1)
	{
		cmd = 0;
		cin >> cmd;
			if (cmd == 'a')
			{
				centerX += RIGHT/10.0;
				centerX += RIGHT/10.0;
			}
			else if (cmd == 'd')
			{
				centerX -= RIGHT/10.0;
				centerX -= RIGHT/10.0;
			}
			else if (cmd == 'w')
			{
				centerY -= TOP/10.0;
				centerY -= TOP/10.0;
			}
			else if (cmd == 's')
			{
				centerY += TOP/10.0;
				centerY += TOP/10.0;
			}
			else if (cmd == 'z')
			{ 	
				zoom += 0.1;
				RIGHT 	/= zoom;
				LEFT 	/= zoom;
				TOP 	/= zoom;
				BOTTOM 	/= zoom;
			}
			else if (cmd == 'x')
			{
				RIGHT *= zoom;
				LEFT *= zoom;
				TOP *= zoom;
				BOTTOM *= zoom;
			}
			else if (cmd == 'r')
			{
				zoom 	= 1.0;
				centerX = 0.0;
				centerY = 0.0;
				RIGHT 	= 1.0;
				LEFT 	= -2.0;
				TOP 	= 1.0;
				BOTTOM 	= -1.0;
			}
			else if (cmd == 'q')
			{
				endwin();
				return 0;
			}
		for (y = 0; y < SIZEY; ++y)
		{
			for (x = 0; x < SIZEX; ++x)
			{
				zr = 0.0;
				zi = 0.0;
				// cr = -0.12 + LEFT + x;
				// ci = 0.75 + TOP + y;
				cr = (LEFT/1.33 - centerX) + x * (RIGHT - LEFT) / SIZEX;
				ci = (TOP - centerY) + y * (BOTTOM - TOP) / SIZEY;

				rsquared = zr * zr;
				isquared = zi * zi;
				for (count = 0; rsquared + isquared <= 2.5 && count < MaxIters; ++count)
				{
					zi = zr * zi * 2;
					zi += ci;

					zr = rsquared - isquared; // Because i squared is always real, in the binomial expansion of the Im num Z
					zr += cr;

					rsquared = zr * zr;
					isquared = zi * zi;
				}
				if (rsquared + isquared <= 4.0)
				{
					addstr("*");
					move(y, x);
				}
				else
				{
					addstr(" ");
					move(y, x);
				}
			}
		}
		refresh();
		clear();
	}
	return 0;
}