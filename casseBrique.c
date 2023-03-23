#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <math.h>

//#define RAND_MAX 10

// Définition des constantes
const int SCREEN_WIDTH = 640;
const int SCORE_WIDTH = 160;
const int SCREEN_HEIGHT = 480;
const int BRICK_WIDTH = 77;
const int BRICK_HEIGHT = 40;
const int BRICK_ROWS = 5;
const int BRICK_COLUMNS = 8;
const int BRICK_GAP= 2;
const int MAP_SIZE = 40;
const int BALL_SIZE = 10;
const int PADDLE_WIDTH = 80;
const int PADDLE_HEIGHT = 10;

// définir les couleurs utilisées dans le jeu
#define WHITE al_map_rgb(255, 255, 255)
#define RED al_map_rgb(255, 0, 0)
#define GREEN al_map_rgb(0, 255, 0)
#define BLUE al_map_rgb(0, 0, 255)
#define BLACK al_map_rgb(0, 0, 0)
#define GREY al_map_rgb(125, 125, 125)

int map[] ={
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1
  };

void drawBar(float x1, float x2)
{
  al_draw_filled_rectangle(0, 450, SCREEN_WIDTH-3, SCREEN_HEIGHT,BLACK);
  al_draw_filled_rounded_rectangle(x1, 450, x2, 460,3,3,WHITE);
}

void drawBall(float cx, float cy, float *V)
{
  ALLEGRO_COLOR ballColor = WHITE;
  al_draw_filled_ellipse(cx,cy,5,5,al_map_rgb(0,0,0));
  cx+=V[0];
  cy+=V[1];
  if(fabs(V[1]) > 3 || fabs(V[0]) > 3) {ballColor=RED;}
  //ballColor = fabs(V[1]) < 3 ? WHITE : RED;
  al_draw_filled_ellipse(cx,cy,5,5,ballColor);
  //printf("la vitesse de la balle x:%f, y:%f \n",V[0],V[1]);
}

int **drawBricks()
{
  int x, y;
	ALLEGRO_COLOR brickColor;
  int brickX = 5;
  int brickY = 5;
  int **coordMatrix = malloc(MAP_SIZE * sizeof(int *));
  for(int i=0;i<MAP_SIZE;i++)
  {
    coordMatrix[i] = malloc(4*sizeof(int));
  }

  for (y = 0; y < BRICK_ROWS; y++) {
    for (x = 0; x < BRICK_COLUMNS; x++) {
      int k=y*BRICK_COLUMNS+x;
      brickColor = map[k] > 0 ? WHITE : BLACK;
      al_draw_filled_rectangle(brickX, brickY, brickX + BRICK_WIDTH, brickY + BRICK_HEIGHT, brickColor);

      if(map[y * BRICK_COLUMNS + x] > 0){
        coordMatrix[k][0] = brickX;
        coordMatrix[k][1] = brickX + BRICK_WIDTH;
        coordMatrix[k][2] = brickY;
        coordMatrix[k][3] = brickY + BRICK_HEIGHT;
        //printf("%d: %d %d %d %d \n", i, brickX,brickX + BRICK_WIDTH,brickY,brickY + BRICK_HEIGHT);
      }

      brickX += BRICK_WIDTH + 2;
    }
    brickX = 5;
    brickY += BRICK_HEIGHT + 2;
  }
  /*
  for(int k=0;k<MAP_SIZE;k++)
      {
        for(int j=0;j<4;j++)
        {
          printf("%d ", coordMatrix[k][j]);
        }
        printf("\n");
      }
  */
  return coordMatrix;
}

int checkCollision(float cx, float cy, float *V, int **coordMatrix) {
  int x, y,i,hit;
  
  for(y=0; y<BRICK_ROWS;y++)
  {
    for(x=0;x<BRICK_COLUMNS;x++)
    {
      i=y*BRICK_COLUMNS+x;
      hit=0;
      if(map[i]>0)
      {
        //rectangle englobant de la balle
        float ballLeft = cx - BALL_SIZE;
        float ballRight = cx + BALL_SIZE;
        float ballTop = cy - BALL_SIZE;
        float ballBottom = cy + BALL_SIZE;
        //rectangle englobant briques
        float brickLeft = coordMatrix[i][0];
        float brickRight = coordMatrix[i][1];
        float brickTop = coordMatrix[i][2];
        float brickBottom = coordMatrix[i][3];

        if(ballLeft<brickRight && ballRight> brickLeft 
        && ballTop < brickBottom && ballBottom > brickTop)
        {
          //printf("Collision detected at brick (%d,%d) and ball (%.2f,%.2f) ", x, y, cx, cy);
          hit=1;
          map[i] = 0;

          float distX = cx - (brickLeft + BRICK_WIDTH / 2);
          float distY = cy - (brickTop + BRICK_HEIGHT / 2);

          if (fabs(distX) > fabs(distY)) {
            // la collision est horizontale
            return 2;
          } 
          else {
            // la collision est verticale
            return 1;
          }
          break;
        }
      }
    }
    if(hit==1) {break;}
  }
  return 0; 
}

int main()
{
	ALLEGRO_DISPLAY *display;
  ALLEGRO_EVENT_QUEUE *queue;
  ALLEGRO_TIMER *timer;
  ALLEGRO_FONT *font;

  al_init();
  al_install_keyboard();
  al_install_mouse();
  al_init_primitives_addon();
  al_init_font_addon();
  al_init_ttf_addon();
  // init random
  srand( time( 0 ) );
  
  display=al_create_display(SCREEN_WIDTH+SCORE_WIDTH,SCREEN_HEIGHT);al_set_window_title(display, "Casse Briques");
  queue =al_create_event_queue();
  timer =al_create_timer(1.0/120);
  font = al_load_font("source/arial.ttf", 20, 0);

  al_register_event_source(queue, al_get_keyboard_event_source()     );
  al_register_event_source(queue, al_get_display_event_source(display));
  al_register_event_source(queue, al_get_timer_event_source(timer)   );

  al_start_timer(timer);
  
  float ballVector[2]={0,0};
	float x1Bar=300, x2Bar=350;
	float diff = x2Bar-x1Bar;
	float cx=SCREEN_WIDTH/2,cy=SCREEN_WIDTH/2,rx=5,ry=5;
	int collision,score,lives=3;
  float vx=0.2,vy=0.2;
  bool running = true;
  int **coordMatrix;
  int startV1 = (rand() % 5)+1;
  int startV2 = (rand() % 5)+1;

	al_clear_to_color(al_map_rgb(0, 0, 0));

	al_draw_filled_ellipse(cx,cy,rx,ry,WHITE);
  

	while (running) 
  {
		al_draw_line(SCREEN_WIDTH,0,SCREEN_WIDTH,SCREEN_HEIGHT,GREY,5);
		ALLEGRO_EVENT event;
    al_wait_for_event(queue, &event);
    ALLEGRO_KEYBOARD_STATE keyState;
    al_get_keyboard_state(&keyState);

		if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || al_key_down(&keyState,ALLEGRO_KEY_ESCAPE)) 
    {running = false;}

		if(al_key_down(&keyState, ALLEGRO_KEY_SPACE))
    {
      float un=(float)startV1/10;
      float deux=(((float)startV2*2)/10)-0.5;
      //printf("%f,%f\n",un,deux);
      if(cx==SCREEN_WIDTH/2 && cy==SCREEN_WIDTH/2)
      {
        
        ballVector[0]=deux  ;
        ballVector[1]=un;
      }
    }

		if(event.type == ALLEGRO_EVENT_TIMER)
    {

			if((cy+ry) > SCREEN_HEIGHT+20)
      {
        int i;
        ballVector[0] = 0;
        ballVector[1] = 0;

        cx=SCREEN_WIDTH/2;
        cy=SCREEN_WIDTH/2;
        i=0;
        lives-=1;
      
      }
			//gerer les deplacement de la bar
			if(al_key_down(&keyState,ALLEGRO_KEY_LEFT))
			{
				if(al_key_down(&keyState, ALLEGRO_KEY_LCTRL))
					{
						x1Bar-=5;
						x2Bar-=5;
					}
				else{
						x1Bar-=1;
						x2Bar-=1;
					}
			}
			if(al_key_down(&keyState,ALLEGRO_KEY_RIGHT))
			{
				if(al_key_down(&keyState, ALLEGRO_KEY_LCTRL))
					{
						x1Bar+=5;
						x2Bar+=5;
					}
				else{
						x1Bar+=1;
						x2Bar+=1;
					}
			}
		
			//evite la sortie de la bar
      if(x2Bar > SCREEN_WIDTH-1)
      {
        x2Bar=SCREEN_WIDTH-1;
        x1Bar=SCREEN_WIDTH-diff;
      } 
      if(x1Bar < 0)
      {
        x1Bar=0;
        x2Bar=0+diff;
      }

      //gestion vitesse Maximal de la balle
      if(ballVector[1]>3) {vy=0;}
      else {vy=0.2;}
      if(ballVector[0]>3) {vx=0;}
      else {vx=0.2;}

      if(((cx-BALL_SIZE)<1 && ballVector[0]<0) || ((cx+BALL_SIZE)>SCREEN_WIDTH-1 && ballVector[0]>0))
        {
          ballVector[0] = -ballVector[0];
        }
            //haut      
      if((cy-BALL_SIZE)<1 && ballVector[1] <0)
        {
            ballVector[1] = -ballVector[1];
        }

      //bar collision avec la ball
      if ((cy + ry) > 450 && (cy - ry) < 460) {
    		if (((cx - rx) > x1Bar && (cx + rx) < x2Bar) && ballVector[1] > 0) 
				{
        	ballVector[1] = -1 * fabs(ballVector[1] + vy);

    		} 
				else if ((cx + rx) > x1Bar && (cx - rx) < (x1Bar + x2Bar) / 2 && ballVector[0] > 0) 
				{
        	ballVector[0] = -1 * fabs(ballVector[0] + vx);
    		} 
				else if ((cx - rx) < x2Bar && (cx + rx) > (x1Bar + x2Bar) / 2 && ballVector[0] < 0) 
				{
        	ballVector[0] = -1 * fabs(ballVector[0] + vx);
          
    		}
        //printf("la vitesse de la balle x:%f, y:%f \n",ballVector[0],ballVector[1]);
      }

			coordMatrix = drawBricks();
			drawBar(x1Bar,x2Bar);
			drawBall(cx,cy,ballVector);		
			cy+=ballVector[1];
			cx+=ballVector[0];

			collision = checkCollision(cx,cy,ballVector,coordMatrix);
			if(collision==1) {ballVector[1] = -ballVector[1];score+=1;}
			else if(collision==2) {ballVector[0] = -ballVector[0];score+=1;}

      al_draw_filled_rectangle(642,0,SCREEN_WIDTH+SCORE_WIDTH,SCREEN_HEIGHT,GREY);
      al_draw_textf(font, BLACK, 650, 10, 0, "Score: %d", score);
      al_draw_textf(font, BLACK, 650, 40, 0, "Lives: %d", lives);

      if(lives == 0)
        {
          int l=0;
          while(l<MAP_SIZE)
        {
          map[l] = 1;
          l++;
        }
          drawBricks();
          lives = 3;
          score=0;
        }
			
			//al_flip_display();
      for(int i=0;i<MAP_SIZE;i++)
      {
        free(coordMatrix[i]);
      }
      free(coordMatrix);
		}
		al_flip_display();
    //running = false;
  }

  al_shutdown_primitives_addon();
  al_uninstall_mouse();
  al_uninstall_keyboard();
  al_destroy_font(font);
  al_destroy_timer(timer);
  al_destroy_event_queue(queue);
  al_destroy_display(display);
  
	return 0;
}