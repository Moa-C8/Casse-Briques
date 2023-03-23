#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
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
#define ORANGE al_map_rgb(240, 134, 94)

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
  if(fabs(V[1]) > 5 || fabs(V[0]) > 3) {ballColor=ORANGE;}
  if(fabs(V[1]) > 5 && fabs(V[0]) > 3) {ballColor=RED;}
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
  ALLEGRO_FONT *Arial20;
  ALLEGRO_FONT *Arial16;
  ALLEGRO_SAMPLE *ploc=NULL;
  ALLEGRO_SAMPLE_INSTANCE *plocInstance=NULL;
  ALLEGRO_SAMPLE *bam=NULL;
  ALLEGRO_SAMPLE_INSTANCE *bamInstance=NULL;
  ALLEGRO_SAMPLE *liv=NULL;
  ALLEGRO_SAMPLE_INSTANCE *livInstance=NULL;

  al_init();
  al_install_keyboard();
  al_install_mouse();
  al_init_primitives_addon();
  al_init_font_addon();
  al_init_ttf_addon();
  al_install_audio();
  al_init_acodec_addon();

  al_reserve_samples(2);
  // init random
  srand( time( 0 ) );
  
  display=al_create_display(SCREEN_WIDTH+SCORE_WIDTH,SCREEN_HEIGHT);al_set_window_title(display, "Casse Briques");
  queue =al_create_event_queue();
  timer =al_create_timer(1.0/60);
  Arial20 = al_load_font("source/arial.ttf", 20, 0);
  Arial16 = al_load_font("source/arial.ttf", 16, 0);
  ploc=al_load_sample("source/ploc.wav");
  bam=al_load_sample("source/bam.wav");
  liv=al_load_sample("source/loose1.wav");

  plocInstance=al_create_sample_instance(ploc);
  bamInstance=al_create_sample_instance(bam);
  livInstance=al_create_sample_instance(liv);
  al_attach_sample_instance_to_mixer(plocInstance, al_get_default_mixer());
  al_attach_sample_instance_to_mixer(bamInstance, al_get_default_mixer());
  al_attach_sample_instance_to_mixer(livInstance, al_get_default_mixer());

  //al_play_sample_instance(plocInstance);
  //al_play_sample_instance(bamInstance);

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
  int startV1 = (rand() % 10)+4;
  int startV2 = (rand() % 10)+2;

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
      float un=(float)startV1/5;
      float deux=(((float)startV2*2)/5)-1;
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
        al_play_sample_instance(livInstance);
      
      }
			//gerer les deplacement de la bar
			if(al_key_down(&keyState,ALLEGRO_KEY_LEFT) || al_key_down(&keyState,ALLEGRO_KEY_A))
			{
				if(al_key_down(&keyState, ALLEGRO_KEY_LCTRL))
					{
						x1Bar-=7;
						x2Bar-=7;
					}
				else{
						x1Bar-=3;
						x2Bar-=3;
					}
			}
			if(al_key_down(&keyState,ALLEGRO_KEY_RIGHT) || al_key_down(&keyState,ALLEGRO_KEY_D))
			{
				if(al_key_down(&keyState, ALLEGRO_KEY_LCTRL))
					{
						x1Bar+=7;
						x2Bar+=7;
					}
				else{
						x1Bar+=3;
						x2Bar+=3;
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
      if(fabs(ballVector[1])>5) {vy=0;}
      else {vy=0.2;}
      if(fabs(ballVector[0])>3) {vx=0;}
      else {vx=0.2;}

      if(((cx+BALL_SIZE)>SCREEN_WIDTH-1 && ballVector[0]>0))
        {
          ballVector[0] = -(ballVector[0] + vx);
          al_play_sample_instance(plocInstance);
        }
      if(((cx-BALL_SIZE)<1 && ballVector[0]<0) )
        {
          ballVector[0] = -(ballVector[0] - vx);
          al_play_sample_instance(plocInstance);
        }
            //haut      
      if((cy-BALL_SIZE)<1 && ballVector[1] <0)
        {
            ballVector[1] = -(ballVector[1] - vy);
            al_play_sample_instance(plocInstance);
        }

      //bar collision avec la ball
      if ((cy + ry) > 450 && (cy - ry) < 460) {
        
    		if (((cx - rx) > x1Bar && (cx + rx) < x2Bar) && ballVector[1] > 0) 
				{
        	ballVector[1] = -1 * fabs(ballVector[1] + vy);

    		} 
				else if ((cx + rx) > x1Bar && (cx - rx) < (x1Bar + x2Bar) / 2 && ballVector[0] > 0) 
				{
        	ballVector[0] = -(ballVector[0] + vx);
    		} 
				else if ((cx - rx) < x2Bar && (cx + rx) > (x1Bar + x2Bar) / 2 && ballVector[0] < 0) 
				{
        	ballVector[0] = -(ballVector[0] - vx);
          
    		}
        al_play_sample_instance(plocInstance);
        //printf("la vitesse de la balle x:%f, y:%f \n",ballVector[0],ballVector[1]);
      }

			coordMatrix = drawBricks();
			drawBar(x1Bar,x2Bar);
			drawBall(cx,cy,ballVector);		
			cy+=ballVector[1];
			cx+=ballVector[0];

			collision = checkCollision(cx,cy,ballVector,coordMatrix);
			if(collision==1) {ballVector[1] = -(ballVector[1]-vy);score+=1;al_play_sample_instance(bamInstance);}
			else if(collision==2) {
        if(ballVector < 0)
        {
          ballVector[0] = -(ballVector[0]-vx);
        }
        else{
          if(ballVector[0] == 0) {ballVector[0]=0.2;}
          ballVector[0] = -(ballVector[0]+vx);
        }
        score+=1;
        al_play_sample_instance(bamInstance);
      }

      al_draw_filled_rectangle(642,0,SCREEN_WIDTH+SCORE_WIDTH,SCREEN_HEIGHT,GREY);
      al_draw_textf(Arial20, BLACK, 650, 10, 0, "Score: %d", score);
      al_draw_textf(Arial20, BLACK, 650, 40, 0, "Lives: %d", lives);
      al_draw_textf(Arial20, BLACK, 650, 70, 0, "Vitesse x: %.2f", fabs(ballVector[0]));
      al_draw_textf(Arial20, BLACK, 650, 100, 0, "Vitesse y: %.2f", fabs(ballVector[1]));

      al_draw_line(630,360,SCREEN_WIDTH+SCORE_WIDTH,360,BLACK,10);
      al_draw_text(Arial20, BLACK, 655, 370, 0, "COMMANDS");
      al_draw_text(Arial16, BLACK, 650, 400, 0, "left: leftArrow or a");
      al_draw_text(Arial16, BLACK, 650, 420, 0, "right: rightArrow or d");
      al_draw_text(Arial16, BLACK, 650, 440, 0, "sprint: toggle ctrl");
      al_draw_text(Arial16, BLACK, 650, 460, 0, "quit:echap or classic");

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
    //al_rest(0.01);
    //running = false;
  }

  al_shutdown_primitives_addon();
  al_uninstall_mouse();
  al_uninstall_keyboard();
  al_destroy_sample(ploc);
  al_destroy_sample_instance(plocInstance);
  al_destroy_sample(bam);
  al_destroy_sample_instance(bamInstance);
  al_destroy_sample(liv);
  al_destroy_sample_instance(livInstance);
  al_destroy_font(Arial20);
  al_destroy_font(Arial16);
  al_uninstall_audio();
  al_destroy_timer(timer);
  al_destroy_event_queue(queue);
  al_destroy_display(display);
  al_shutdown_font_addon();
  
	return 0;
}