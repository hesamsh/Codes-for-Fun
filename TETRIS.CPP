//------------------------------<TETRIS GAME>--------------------------------
//-------------------<Programmed by Hesam Shams in C++>----------------------
//-------------------------------<Year 2005>---------------------------------
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>
#include <conio.h>

#define SPEED 300
#define Min_X 50
#define Min_Y 1
#define ESC 1
#define P_KEY 25
#define S_KEY 31
#define SPACE 57
#define Up_Arrow 72
#define Down_Arrow  80
#define Left_Arrow  75
#define Right_Arrow 77
#define VideoSeg 0xB800
#define KEYB  0x09
#define keyb_port 0x60
#define __CPPARGS ...

unsigned int sound_on = 1;
unsigned char scancode = 0;
unsigned int lines = 0;
unsigned int level = 1;
unsigned long score = 0;

void initialize(void);
void clearup(void);
void sound_effect(int fx);
char readch(int x, int y);
void writech(int x, int y, char ch);
void interrupt keyb_routine(__CPPARGS);
void interrupt ( *oldkeybhandler)(__CPPARGS);

//--------------------<tetraminoe Class Interface Below>----------------------
class tetraminoe {
  public:
	tetraminoe(unsigned int a[4]);
	void go_left(void);
	void go_right(void);
	void go_up(void);
	void go_down(void);
	void rotate_clockwise(void);
	void rotate_anticlockwise(void);
	void show_tetraminoe(void);
	void clear_tetraminoe(void);
	int check_crash(void);
  private:
	int shape[4];
	int orientation,
	x_Pos, y_Pos;
};//----------------<member function of tetraminoe class below>---------------
tetraminoe::tetraminoe(unsigned int a[4]) { //the constructor
   shape[0] = a[0];
   shape[1] = a[1];
   shape[2] = a[2];
   shape[3] = a[3];
   orientation = 0;
   x_Pos = Min_X + 6;
   y_Pos = Min_Y;
}
void tetraminoe::go_left(void) {
   x_Pos -= 2;
}
void tetraminoe::go_right(void) {
   x_Pos += 2;
}
void tetraminoe::go_up(void) {
   y_Pos--;
}
void tetraminoe::go_down(void) {
   y_Pos++;
}
void tetraminoe::rotate_clockwise(void) {
   orientation--;
   if (orientation < 0) orientation = 0;
}
void tetraminoe::rotate_anticlockwise(void) {
   orientation++;
   if (orientation >= 4) orientation = 0;
}
int tetraminoe::check_crash(void) {
   int x, y, i = 0;

   for (y = y_Pos; y < y_Pos + 4; y++)
   for (x = x_Pos; x < x_Pos + 8; x+=2) {
	  if (shape[orientation] & (0x8000 >> i))//if current cell of tetraminoe is full
//and related characters on the screen are blank then return 0
		 if ((readch(x, y) != ' ') || (readch(x + 1, y) != ' '))
			return 0;
	  i++;
   }
  return 1;
}
void tetraminoe::show_tetraminoe(void) {
   int k = 0, i;
   for (i = 1; i <= 16; i++) {
	  if (shape[orientation] & 0x8000 >> (i-1)) {//if current cell of tetraminoe
//is full then print a square (chr(219)*2) at current location on screen
		 writech(x_Pos + (((i-1) % 4) << 1), y_Pos + k, 'Û');
		 writech(x_Pos + (((i-1) % 4) << 1) + 1, y_Pos + k, 'Û');
	  }
	  if (!(i % 4)) k++;//go one line down for each 4 characters
   }
}//---Clears-Tetraminoe-from-screen------< clear_tetraminoe below >-----------
void tetraminoe::clear_tetraminoe(void) {
   int k = 0, i;
   for (i = 1; i <= 16; i++) {//same algorithm with show function
//but writes two blanks instead of a square on screen
	  if (shape[orientation] & 0x8000>>(i-1)) {
		 writech(x_Pos + (((i-1) % 4) << 1), y_Pos + k, ' ');
		 writech(x_Pos + (((i-1) % 4) << 1) + 1, y_Pos + k, ' ');
	  }
	  if (!(i % 4)) k++;//go one line down for each 4 characters
   }
}//----------------------------<Functions Below>------------------------------
void writech(int x, int y, char ch) {
   pokeb(VideoSeg, ( (((y-1)<<6)+((y-1)<<4)+x-1) << 1) + 1,0x9);
   pokeb(VideoSeg, (((y-1)<<6)+((y-1)<<4)+x-1) << 1, ch);
}
char readch(int x, int y) {
   return peekb(VideoSeg,(((y-1)<<6)+((y-1)<<4)+x-1) << 1);
}
void sound_effect(int fx) {
  int i;
  if (sound_on) {
     switch (fx) {
	    case 1: for (i = 400; i > 300; i-=30) {sound(i);delay(2);nosound();}break;
	    case 2: for (i = 300; i < 500; i+=20) {sound(i);delay(20);nosound();}break;
	    case 3: for (i = 500; i >= 200; i-=50) {sound(i);delay(50);nosound();}break;
	    case 4: sound(300); delay(400); nosound(); sound(400);delay(200);nosound();
				for (i = 0; i < 10; i++) {sound(500);delay(40);nosound();delay(5);}break;
	 }//switch
  }//if sound is on
}//----deletes-complited-lines-----------< checkline below >------------------
//-----adjusts-line-level-and-score-------------------------------------------
void checkline(void) {
  int x, y, j, LineOK = 1, LinesDeleted = 0;
  for (y = Min_Y + 20; y >= Min_Y; y--) {
	  for (x = Min_X; x < Min_X + 20; x++)
		  if (readch(x,y) == ' ') LineOK = 0;
	  if (LineOK) {//---y-is-now-at-the-completed-line------------------------
	  //------delete-line------------------------------------------------------
		 for (x = Min_X; x < Min_X + 20; x++) writech(x, y, ' ');
		 for (j = y - 2; j >= 1; j--) //--------drop-lines-by-one-----------------
			 movedata(VideoSeg, ( (j<<6) + (j<<4) + Min_X-1) << 1, VideoSeg, ( ((j+1)<<6) + ((j+1)<<4) + Min_X-1) << 1, 40);
		 //since-all-lines-went-down-by-one-we-must-check-current-line-again------
		 y++;
		 lines++;
		 gotoxy(Min_X - 18, Min_Y + 7); printf("%d",lines);
		 if ((lines != 0) && !(lines % 20)) {//increase-level-for-each-20-lines
			level++; if (level > 10) level = 10;  //-----maximum-10-levels----
			gotoxy(Min_X - 18, Min_Y + 4); printf("%d",level);
		 }//if
		 LinesDeleted++;
	 }//-----if-line-is-completed
	 LineOK = 1;
  }//-----check-for-each-line-from-bottom-to-top------------------------------
  if (LinesDeleted) {
	 score += LinesDeleted*LinesDeleted*1000;
	 sound_effect(3);
  }//if
  gotoxy(Min_X - 18, Min_Y + 10); printf("%lu", score);
}//-----Initalize-keyboard-interrupt-and-draw-play-area-----------------------
void initialize() {
  int i;
  oldkeybhandler = getvect(KEYB);
  setvect(KEYB, keyb_routine);
  textattr(0x7f); clrscr(); textattr(0x07); _setcursortype(_NOCURSOR);
  for (i = 1; i <= 24; i++) {
	 gotoxy(1, i); cprintf("                           ³\n");
  }
	 gotoxy(1,24); cprintf("ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n");
  for (i = Min_Y; i<= Min_Y + 20; i++) {
	 gotoxy(Min_X - 1, i);cprintf("³                    ³\n");
  }
  gotoxy(Min_X - 1, Min_Y + 21);cprintf("ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n");
  textattr(0x0e);
  gotoxy(1,1);cprintf("      TETRIS GAME by \n");
  gotoxy(1,2);cprintf("        Hesam Shams\n");
  gotoxy(1,3);cprintf("             &  \n");
  gotoxy(1,4);cprintf("       Parisa Majlesi   \n");
  gotoxy(1,7);cprintf("        ---<KEYS>---");
  gotoxy(1,9);cprintf("             up");
  gotoxy(1,10);cprintf("        <left right>");
  gotoxy(1,11);cprintf("            down");
  gotoxy(1,13);cprintf("  Space : drops piece");
  gotoxy(1,14);cprintf("  S : toggles sound on/off");
  gotoxy(1,15);cprintf("  P : toggles pause");
  gotoxy(1,16);cprintf("  ESC : quits game");
  gotoxy(1,22);cprintf("         GOOD LUCK!");
  textattr(0x7f);
  gotoxy(Min_X - 18, Min_Y + 3);cprintf("LEVEL:");
  gotoxy(Min_X - 18, Min_Y + 4);printf("%d", level);
  gotoxy(Min_X - 18, Min_Y + 6);cprintf("LINES:");
  gotoxy(Min_X - 18, Min_Y + 7);printf("%d", lines);
  gotoxy(Min_X - 18, Min_Y + 9);cprintf("SCORE:");
  gotoxy(Min_X - 18, Min_Y + 10);printf("%d", score);
  gotoxy(Min_X - 18, Min_Y + 12);cprintf("NEXT");
  gotoxy(Min_X - 18, Min_Y + 18);cprintf("Sound on");
  randomize();
}
void clearup(void) {
  char name[10],highest_name[10];unsigned long highest_score = 0;
  FILE *score_file;
  setvect(KEYB, oldkeybhandler);  //release keyboard interrupt vector
  textattr(0x9f);
  gotoxy(Min_X + 2,Min_Y + 9); cprintf("ÚÄÄÄÄÄÄÄÄÄÄÄÄ¿");
  gotoxy(Min_X + 2,Min_Y + 10);cprintf("³ GAME OVER! ³");
  gotoxy(Min_X + 2,Min_Y + 11);cprintf("ÀÄÄÄÄÄÄÄÄÄÄÄÄÙ");
  sound_effect(4);
  getch();
  clrscr();
  exit(1);
}
void interrupt keyb_routine(__CPPARGS)
{
   scancode = inp(keyb_port);
   outportb(0x20, 0x20);
}
//----------------------------<MAIN PROGRAM>----------------------------------
void main() {
  unsigned int counter1 = 0, counter2 = 0, select, next_select, new_tetraminoe = 0, k;
  tetraminoe *Tetraminoe;
  unsigned int array [7][4] = {{0x2230, 0x1700, 0x6220, 0x0740},
/*Here are the 7 Tetraminoes*/ {0x2260, 0x0710, 0x3220, 0x4700},
/*each having 4 frames*/       {0x2310, 0x0360, 0x2310, 0x0360},
							   {0x1320, 0x0630, 0x1320, 0x0630},
							   {0x6600, 0x6600, 0x6600, 0x6600},
							   {0x2222, 0x0f00, 0x2222, 0x0f00},
							   {0x2700, 0x2620, 0x0720, 0x2320}};
  initialize();
  select = random(7);
  next_select = random(7);

  do {//-------------------------<The Game Loop>------------------------------
	 if (new_tetraminoe) {
		delete Tetraminoe;
		switch (select) {
			case 0: Tetraminoe = new tetraminoe(array[0]);break;/*L_shaped1*/
			case 1: Tetraminoe = new tetraminoe(array[1]);break;/*L_shaped2*/
			case 2: Tetraminoe = new tetraminoe(array[2]);break;/*Z_shaped1*/
			case 3: Tetraminoe = new tetraminoe(array[3]);break;/*Z_shaped2*/
			case 4: Tetraminoe = new tetraminoe(array[4]);break;/*Cube_shaped*/
			case 5: Tetraminoe = new tetraminoe(array[5]);break;/*Rod_shaped*/
			case 6: Tetraminoe = new tetraminoe(array[6]);break;/*Tshaped*/
		}//select tetraminoe switch
		textattr(0x79);
		switch (next_select) {
			case 0: gotoxy(Min_X - 18, Min_Y + 13);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 14);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 15);cprintf("  ÛÛÛÛ");
					gotoxy(Min_X - 18, Min_Y + 16);cprintf("      ");break;
			case 1: gotoxy(Min_X - 18, Min_Y + 13);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 14);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 15);cprintf("ÛÛÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 16);cprintf("      ");break;
			case 2: gotoxy(Min_X - 18, Min_Y + 13);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 14);cprintf("  ÛÛÛÛ");
					gotoxy(Min_X - 18, Min_Y + 15);cprintf("    ÛÛ");
					gotoxy(Min_X - 18, Min_Y + 16);cprintf("      ");break;
			case 3: gotoxy(Min_X - 18, Min_Y + 13);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 14);cprintf("ÛÛÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 15);cprintf("ÛÛ    ");
					gotoxy(Min_X - 18, Min_Y + 16);cprintf("      ");break;
			case 4: gotoxy(Min_X - 18, Min_Y + 13);cprintf("ÛÛÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 14);cprintf("ÛÛÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 15);cprintf("      ");
					gotoxy(Min_X - 18, Min_Y + 16);cprintf("      ");break;
			case 5: gotoxy(Min_X - 18, Min_Y + 13);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 14);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 15);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 16);cprintf("  ÛÛ  ");break;
			case 6: gotoxy(Min_X - 18, Min_Y + 13);cprintf("  ÛÛ  ");
					gotoxy(Min_X - 18, Min_Y + 14);cprintf("ÛÛÛÛÛÛ");
					gotoxy(Min_X - 18, Min_Y + 15);cprintf("      ");
					gotoxy(Min_X - 18, Min_Y + 16);cprintf("      ");break;
		}//switch next_select
		if (!Tetraminoe->check_crash()) clearup();//GAME IS OVER
		new_tetraminoe = 0; //to initialize a tetraminoe once
	 }//if new_tetraminoe
//----------------Control-scan-code-for-a-pressed-key-------------------------
	 switch (scancode) {
		case Left_Arrow: if (!(counter1%20)) { Tetraminoe->go_left();
				if (!Tetraminoe->check_crash()) Tetraminoe->go_right();} break;

		case Right_Arrow: if (!(counter1%20)) { Tetraminoe->go_right();
				if (!Tetraminoe->check_crash()) Tetraminoe->go_left();} break;

		case Up_Arrow: if (!(counter1%25)) { Tetraminoe->rotate_anticlockwise();
				if (!Tetraminoe->check_crash()) Tetraminoe->rotate_clockwise();
				else sound_effect(2);} break;

		case Down_Arrow: if (!(counter1%10)) { Tetraminoe->go_down();
				if (!Tetraminoe->check_crash()) Tetraminoe->go_up();
				else {score += 10;sound_effect(1);}} break;

		case SPACE: while (Tetraminoe->check_crash()) {Tetraminoe->go_down();score += 10;}
					Tetraminoe->go_up(); break;

		case S_KEY: sound_on ^= 1; gotoxy(Min_X - 12, Min_Y + 18);
					sound_on ? printf("on ") : printf("off");delay(150); break;

		case P_KEY: delay(500); scancode = 0;
					gotoxy(Min_X - 44, Min_Y + 18);textattr(0x9f);cprintf("GAME PAUSED!");
					while(scancode!=P_KEY); delay(100);
					gotoxy(Min_X - 44, Min_Y + 18);textattr(0x0f);cprintf("            ");break;
	}//switch scan_code
//----------Show-tetraminoe-for-a-while-and-clear-for-the-new-one-------------
	 Tetraminoe->show_tetraminoe();
	 delay(3);
	 Tetraminoe->clear_tetraminoe();
//--------------adjust-the-counters-and-go-down-if-possible--------------------
	 counter1++; counter2++; // to control game speed
	 if (counter1 >= 100) counter1 = 0; // for when a key should be handled

	 if (counter2 > SPEED-level*(SPEED-5)/10) { //for the game speed
		counter2 = 0;
		Tetraminoe->go_down();
		if (!Tetraminoe->check_crash()) { //if crashed into something
		   Tetraminoe->go_up(); //undo going down
		   Tetraminoe->show_tetraminoe();
		   checkline();   //check if we completed any lines and delete them
		   new_tetraminoe = 1; //it is time for a new tetraminoe
		   select = next_select;
		   next_select = random(7); //select randomly a new tetraminoe
		} else sound_effect(1);
	 }//if it is falling time
  } while (scancode != ESC);//---------------end-of-game-loop-----------------
  setvect(KEYB, oldkeybhandler);
  clrscr();
}
