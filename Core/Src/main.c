/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx.h"
#include "bitmaps.h"
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

DAC_HandleTypeDef hdac;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

/* USER CODE BEGIN PV */

// display
struct display_config cfg;

//joystick
uint16_t Joystick[2];

//game
int round = 1, next_round = 1;
int points = 0;
int gameon = 0, menu = 1;
int menuvalue = 0 ;

//shape
static uint8_t shape[8][8];
int value_x = 12,value_y = 4;
int current_x, current_y;

//audio
extern unsigned char rawData[409898];
int musicvalue = 0;
int song, off = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM6_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

//display
void display_centre(uint8_t plansza_wyswietlana[48][38]);
void display_points();
void menu_();

//game
_Bool check_table(uint8_t table[48][38]);
void clear_table();
void add_shape();
void full_line();
_Bool game_over();

//shapes
void new_shape();
void random_shape();
void rotate();


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
if(htim->Instance == TIM2)
	{
		if(round < 10 ) {htim2.Init.Period = 899;}
		if(round > 10 ) {htim2.Init.Period = 800;}
		if(round > 20 ) {htim2.Init.Period = 700;}
		if(round > 30 ) {htim2.Init.Period = 600;}
		if(round > 40 ) {htim2.Init.Period = 500;}
		if(round > 50 ) {htim2.Init.Period = 400;}
		if(round > 60 ) {htim2.Init.Period = 300;}
		if(round > 70 ) {htim2.Init.Period = 200;}
		if(round > 80 ) {htim2.Init.Period = 100;}
		if(round > 90 ) {htim2.Init.Period = 50;}

	if(Joystick[0] < 1000 && Joystick[1]>1000 && Joystick[1]<3000 )
	{//lewo
		clear_table();
		value_x = value_x + 2;
		add_shape(plansza_wyswietlana);

		if(check_table(plansza_wyswietlana) == 0)
		{
			value_x = value_x - 2;
			clear_table();
			add_shape(plansza_wyswietlana);
		}

	}
	else if(Joystick[0] > 2500  && Joystick[1]>1000 && Joystick[1]<3000)
	{//prawo
		clear_table();
		value_x = value_x - 2;
		add_shape(plansza_wyswietlana);

		if(check_table(plansza_wyswietlana) == 0)
		{
			value_x = value_x + 2;
			clear_table();
			add_shape(plansza_wyswietlana);
		}
	}
	else if(Joystick[0]>1000 && Joystick[0]<3000  && Joystick[1] < 1000)
	{////gora
			rotate();
			add_shape(plansza_wyswietlana);
	}
	else if(Joystick[0]>1000 && Joystick[0]<3000  && Joystick[1] > 3000)
	{//dol
			value_y = value_y + 2;
			add_shape(plansza_wyswietlana);

	}
	else
	{
		add_shape(plansza_wyswietlana);
	}

if (gameon == 1){



	display_rewrite_buffer(&cfg);
	display_set_dxy(&cfg, horizontal, 0, 0);
	display_plansza(&cfg,Plansza);
	display_centre(plansza_wyswietlana);
	display_points();
	if(check_table(plansza_wyswietlana))
	     {
		 value_y=value_y+2;
		 clear_table();
	     }
	else
	{
		value_y=value_y-2;
		clear_table();
		add_shape(plansza_wyswietlana);


		full_line();

		display_rewrite_buffer(&cfg);
		display_set_dxy(&cfg, horizontal, 0, 0);
		display_plansza(&cfg,Plansza);
		display_centre(plansza_wyswietlana);
		display_points();

		for (int i = 0 ; i < 48; i++) {
		            for (int j = 0; j < 38; j++)
		             {
		                tablica_czysta[i][j] = plansza_wyswietlana[i][j];
		             }
		         }



		if(game_over()) //check czy koniec gry
		{
			 next_round = 0;
			 display_clear_buffer(&cfg);
			 char hw[] = "Koniec gry";
			 memcpy(&(cfg.buffer[2][2]), hw, strlen(hw));
			 display_rewrite_buffer(&cfg);
			 HAL_TIM_Base_Stop_IT(&htim6);

		}
		else
		{

			random_shape();
		 	value_y= 4;
		 	value_x = 12;
		 	round = round +1;
		}
  	 	HAL_TIM_Base_Stop_IT(&htim2);
	  }
	}
}
if(htim->Instance == TIM6){
		song=song+2;
		if(song == 409000)
			song=0;

		HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R, rawData[song]);
	}
}


void start() //koniec gierki
{
	cfg.spi = &hspi1;
	cfg.reset_port = RST_GPIO_Port;
	cfg.reset_pin = RST_Pin;
	cfg.dc_port = DC_GPIO_Port;
	cfg.dc_pin = DC_Pin;
	cfg.ce_port = CE_GPIO_Port;
	cfg.ce_pin = CE_Pin;

	display_setup(&cfg);
	display_clear_buffer(&cfg);
	display_rewrite_buffer(&cfg);

}


_Bool game_over()
{
	for(int i = 3 ; i < 36; i++)
	{
		if(tablica_czysta[3][i] > 0){
			gameon = 0;
			return 1;
		}
	}
	return 0;
}

void rotate()
{
	static uint8_t rotation[8][8] ={
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
			{0,0,0,0,0,0,0,0},
		};

	for(int i = 0; i < 8; i++)
	{
		for(int j=0;j<8;j++)
		{
			rotation[i][j] = shape[j][7-i];
		}
	}
	for(int i = 0; i < 8; i++)
	{
		for(int j=0;j<8;j++)
		{
		shape[i][j] = rotation[i][j];
		}
	}
}

// losuje klocek
void random_shape()
{

    int shape_random = rand() % 7;
    shape_random = shape_random + rand() % 7;
    if (shape_random > 6)
    {
    	shape_random = shape_random % 7;
    }
    switch(shape_random)
    {
    case 0:
    {
      for(int i =0; i <8; i++)
        {
            for(int j=0;j<8; j++)
            {
                shape[i][j] = kwadrat[i][j];
            }
        }
        break;
    }
    case 1:
        {

            for(int i =0; i <8; i++)
            {
                for(int j=0;j<8; j++)
                {
                    shape[i][j] = linia[i][j];
                }
            }

            break;
        }
    case 2:
        {

            for(int i =0; i <8; i++)
            {
                for(int j=0;j<8; j++)
                {
                    shape[i][j] = T[i][j];
                }
            }

            break;
        }
    case 3:
        {
            for(int i =0; i <8; i++)
            {
                for(int j=0;j<8; j++)
                {
                    shape[i][j] = L[i][j];
                }
            }

            break;
        }
    case 4:
        {
        	for(int i =0; i <8; i++)
            {
                for(int j=0;j<8; j++)
                {
                    shape[i][j] =J[i][j];
                }
            }

            break;
        }
    case 5:
        {
        	for(int i =0; i <8; i++)
            {
                for(int j=0;j<8; j++)
                {
                    shape[i][j] = zygzak[i][j];
                }
            }

            break;
        }
    case 6:
        {
            for(int i =0; i <8; i++)
            {
                for(int j=0;j<8; j++)
                {
                    shape[i][j] = zygzak2[i][j];
                }
            }
            break;
        }
    }
}

// wyswietla tablice z gra
void display_centre(uint8_t srodek[48][38])
{
	for(int wiersz = 1; wiersz < 7; wiersz++)
	{
		int tablica_wyswietlana[38];
					 int data = 0;
					 int n = 0;
					 int liczba = 0;
					 int row = wiersz*8-1;
						 for (int col = 0; col <38; col++){
					    	if(srodek[row][col] == 1)
					    		data = data + 8;
					    	row=row-1;
					    	if(srodek[row][col] == 1)
					    		data = data + 4;
					    	row=row-1;
					    	if(srodek[row][col] == 1)
					    		data = data + 2;
					    	row=row-1;
					    	if(srodek[row][col] == 1)
					    		data = data + 1;
					    	row=row-1;

					    	if(n == 0){
					    		data=data*16;
					    		n = 1 ;
					    		col=col-1;
					    	}

					    	else{
					    		n = 0;
					    		tablica_wyswietlana[liczba]=data;
					    		data = 0;
					    		liczba = liczba + 1;
								row = wiersz*8-1;

					    }
					}
					display_set_dxy(&cfg, horizontal, 14, wiersz-1);
						for(int i=0;i<38;i++){
						display_write_data(&cfg, tablica_wyswietlana[i]);
		}
	}
}

//zamienia tablice wyswietlana na tablice czysta
void clear_table()
{
    for (int i = 0 ; i < 48; i++) {
            for (int j = 0; j < 38; j++)
             {
                plansza_wyswietlana[i][j] = tablica_czysta[i][j];
             }
         }
}

//nakłada klocek na tablice wyswietlana
void add_shape()
{
	for (int i = 0 ; i < 8; i++) {
		for (int j = 0, xwar = value_x; j < 8; j++)
		 {
			plansza_wyswietlana[value_y+i][xwar] = plansza_wyswietlana[value_y+i][xwar] + shape[i][j];
			xwar = xwar +1;
		 }

   }

}

// sprawdza czy w tablicy nałożyły sie klocki (false - nałożyły sie, true- jest git)
_Bool check_table(uint8_t table[48][38])
{
	for(int i=0;i<48;i++)
	{
		for(int j=0;j<38;j++)
		{
			if(table[i][j] > 1)
				return 0;
		}
	}
	return 1;
}

void full_line()
{int licznik =0;

    for(int i=2;i<46;i++)
        {
         for(int j=0;j<38;j++)
             {
               if(plansza_wyswietlana[i][j] == 1)
                {
                    licznik = licznik +1;
                }
                if(licznik == 38)
                {
                	//zmiana punktów

                    points = points + (38*round);

                    while(i>3)
                    	{
                    		for(int k = 0; k < 38; k++)
                    		{
                                plansza_wyswietlana[i + 1][k] = plansza_wyswietlana[i][k];
                                //punkty
                    		}
                    		i = i -1;
                    	}
                     licznik =0;
                     i = 2;
                }
            }
         licznik =0;
        }
}

void display_points()
{   int punkty_wyswietlane[6] = {0,0,0,0,0,0};

    punkty_wyswietlane[5] = points%10;
    punkty_wyswietlane[4] = (points%100 - punkty_wyswietlane[5])/10;
    punkty_wyswietlane[3] = (points%1000 - (punkty_wyswietlane[4]*10)- punkty_wyswietlane[5])/100;
    punkty_wyswietlane[2] = (points%10000 - (punkty_wyswietlane[3]*100)- (punkty_wyswietlane[4]*10)- punkty_wyswietlane[5])/1000;
    punkty_wyswietlane[1] = (points%100000- (punkty_wyswietlane[2]*1000) - (punkty_wyswietlane[3]*100)- (punkty_wyswietlane[4]*10)- punkty_wyswietlane[5])/10000;
    punkty_wyswietlane[0] = points/100000;



    display_set_dxy(&cfg, horizontal,56,2);
     for(int j=0;j<6;j++){
        for(int i=0;i<3;i++){
       display_write_data(&cfg, ASCII2[punkty_wyswietlane[j]+16][i]);
      }
      display_write_data(&cfg, 0x00);
    }
}

void menu_()
{
			 display_clear_buffer(&cfg);
		 	 char hw[] = "TETRIS";
		 	 memcpy(&(cfg.buffer[0][4]), hw, strlen(hw));
		 	 if (menuvalue == 0)
		 	 {
			 	   	char hw1[] = "-> Play";
			 	    memcpy(&(cfg.buffer[2][2]), hw1, strlen(hw1));

		 	 }
		 	 else
		 	 {
			 	   	char hw1[] = "Play";
			 	    memcpy(&(cfg.buffer[2][4]), hw1, strlen(hw1));
		 	 }

		 	 if (menuvalue == 1)
		 	 {

		 		 	 char hw2[] = "-> Credits";
		 	   		memcpy(&(cfg.buffer[3][2]), hw2, strlen(hw2));
		 	 }
		 	 else
		 	 {

		 		 	 char hw2[] = "Credits";
		 	   		memcpy(&(cfg.buffer[3][4]), hw2, strlen(hw2));
		 	 }

		 	 if (menuvalue == 2)
		 	 {

		   	    	char hw3[] = "-> Restart";
		   	    	memcpy(&(cfg.buffer[4][2]), hw3, strlen(hw3));
		 	 }
		 	 else
		 	 {
		   	    	char hw3[] = "Restart";
		   	    	memcpy(&(cfg.buffer[4][4]), hw3, strlen(hw3));
		 	 }
		 	 if (menuvalue == 3)
		     {

		 		    char hw4[] = "-> Options";
		 			memcpy(&(cfg.buffer[5][2]), hw4, strlen(hw4));
		     }
		 	 else
		 	 {
		 		    char hw4[] = "Options";
		 			memcpy(&(cfg.buffer[5][4]), hw4, strlen(hw4));
		 	 }
		 	   	display_rewrite_buffer(&cfg);


		 	   if(Joystick[0]>1000 && Joystick[0]<3000  && Joystick[1] < 1000)
		 	   	{ // gora menu
		 		   	   	    menuvalue = menuvalue - 1;
		 	   				if(menuvalue < 0) menuvalue= 0;
		 	   	// gora dzwiek
		 	   			menuvalue = menuvalue - 1;
		 	   		    if(menuvalue < 0) menuvalue= 0;
		 			 	   HAL_Delay(500);
		 	   	}
		 	   	else if(Joystick[0]>1000 && Joystick[0]<3000  && Joystick[1] > 3000)
		 	   	{ //dol menu
		 	   			menuvalue = menuvalue +1;
		 	   			if(menuvalue > 3)
		 	   				{
		 	   				menuvalue = 3;
		 	   				}
		 	   		//dol dzwiek
		 	   		      musicvalue = musicvalue +1;
		 	   		     if(musicvalue > 1){
		 	   		    	musicvalue = 1;
		 	   		      }
		 		 	   HAL_Delay(500);
		 	   	}

		 	   HAL_Delay(1);
		 	   if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET)
		 	  			  {
		 	  	    if (menu == 1)
		 	  				{
		 	  			 if(menuvalue == 0)
		 	  					 	 {
		 	  							gameon = 1;
		 	  							menu = 0;
		 	  							menuvalue = 0;
		 	  							clear_table();

			 	  						HAL_Delay(100);

		 	  					  	}

		 	  		     if(menuvalue == 1)
		 	  						{
		 	  						menu = 2;
		 	  						display_rewrite_buffer(&cfg);
		 	  						display_set_dxy(&cfg, horizontal, 0, 0);
		 	  						display_plansza(&cfg,credits_bitmap);
		 	  						HAL_Delay(500);
		 	  							while(menu == 2){
		 	  								if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET){
		 	  					 		  menu = 1;
		 	  					 		  HAL_Delay(500);}
		 	  						 }
		 	  					  }
		 	  		    if(menuvalue == 2) {//restart
		 	  							menu = 0;
		 	  							menuvalue = 0;
		 	  							random_shape();
		 	  							value_y= 4;
		 	  							value_x = 12;
		 	  							round = 1;
		 	  							next_round = 1;
		 	  							points = 0;
		 	  						 for (int i = 0 ; i < 48; i++) {
		 	  							for (int j = 0; j < 38; j++){
		 	  								plansza_wyswietlana[i][j] = tablica_czysta2[i][j];
		 	  								tablica_czysta[i][j] = tablica_czysta2[i][j];}}
		 	  							gameon = 1;
		 	  						 }
		 	  			if(menuvalue == 3) {//opcje muzyki
		 	  						  menu = 3;
		 	  					      musicvalue = 0;
		 	  						  HAL_Delay(500);

		 	  					 while(menu == 3)
		 	  						 {
		 	  						display_clear_buffer(&cfg);
		 	  						if (musicvalue == 0 && off == 0){
		 	  							char hw7[] = "-> Music On";
		 	  							memcpy(&(cfg.buffer[1][0]), hw7, strlen(hw7));}

		 	  						else if(musicvalue == 1 && off == 0){
		 	  							char hw7[] = "Music On";
		 	  							memcpy(&(cfg.buffer[1][0]), hw7, strlen(hw7)); }

		 	  						else if(musicvalue == 0 && off == 1){
		 	  							char hw8[] = "-> Music Off";
		 	  							memcpy(&(cfg.buffer[1][0]), hw8, strlen(hw8));}
		 	  						else{
		 	  							char hw8[] = "Music Off";
		 	  							memcpy(&(cfg.buffer[1][0]), hw8, strlen(hw8));}

		 	  						if (musicvalue == 1){
		 	  							char hw6[] = "-> Back";
		 	  							memcpy(&(cfg.buffer[3][0]), hw6, strlen(hw6));}
		 	  						else{
		 	  							char hw6[] = "Back";
		 	  							memcpy(&(cfg.buffer[3][0]), hw6, strlen(hw6));}

		 	  						display_rewrite_buffer(&cfg);

		 	  						if(Joystick[0]>1000 && Joystick[0]<3000  && Joystick[1] < 1000)
		 	  						{
		 	  						// gora dzwiek
		 	  							musicvalue= 0;
		 	  						HAL_Delay(500);
		 	  						}
		 	  						else if(Joystick[0]>1000 && Joystick[0]<3000  && Joystick[1] > 3000)
		 	  						{
		 	  						//dol dzwiek
		 	  							musicvalue = 1;
		 	  						HAL_Delay(500);
		 	  						}

		 	  						else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET)
		 	  					 	 {
		 	  					 		if(musicvalue == 0){
		 	  					 	     if(off == 1)
		 	  					 	     {
		 	  					 	    	HAL_TIM_Base_Start_IT(&htim6);
		 	  					 	    	off = 0;
		 	  					 	     }
		 	  					 	     else
		 	  					 	     {
		 	  					 	    	 off = 1;
		 	  					 	    	HAL_TIM_Base_Stop_IT(&htim6);
		 	  					 	     }
		 	  					 	    }
		 	  					 	if (musicvalue == 1){
		 	  					 		 menu = 1;
		 	  					 		}
	 	  					 		HAL_Delay(500);

		 	  					 }

		 	  					 	 }
		 	  					}

		 	  			    }

		 	  		}

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	srand(time(NULL));
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_DAC_Init();
  MX_TIM2_Init();
  MX_TIM6_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

	HAL_ADC_Start_DMA(&hadc1, Joystick, 2);
	HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
	HAL_TIM_Base_Start_IT(&htim6);
  	start();
  	random_shape();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		if(round < next_round)
		{
		 	 HAL_TIM_Base_Start_IT(&htim2);
		 	 HAL_Delay(500);
		 	 if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET)
		 		{
		 			gameon = 0;
		 			menu = 1;
		 			current_x= value_x;
		 			current_y = value_y;
		 			while(gameon == 0)
		 			{
		 				if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET)
		 					menu_();
		 			}
		 		}


		}
		else if(round == next_round)
		{
			if(gameon == 1)
			{
				next_round = next_round +1;
			}
			else
			{
				menu_();
			}
		}
		else if(next_round == 0)
		{

			             HAL_Delay(1);
				if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_RESET)
					{             gameon = 0;
							            menu = 1;
										menuvalue = 0;
										random_shape();
										value_y= 4;
										value_x = 12;
										round = 1;
										next_round = 1;
										points = 0;
										display_clear_buffer(&cfg);
									 for (int i = 0 ; i < 48; i++) {
									            for (int j = 0; j < 38; j++)
									             {
									                plansza_wyswietlana[i][j] = tablica_czysta2[i][j];
									                tablica_czysta[i][j] = tablica_czysta2[i][j];
									             }
									         }
									 display_rewrite_buffer(&cfg);
							HAL_TIM_Base_Start_IT(&htim6);
							HAL_Delay(500);

						}
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC_Init(void)
{

  /* USER CODE BEGIN DAC_Init 0 */

  /* USER CODE END DAC_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC_Init 1 */

  /* USER CODE END DAC_Init 1 */
  /** DAC Initialization 
  */
  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT1 config 
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC_Init 2 */

  /* USER CODE END DAC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 63999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 899;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 84;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 63;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DC_Pin|RST_Pin|CE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : DC_Pin RST_Pin CE_Pin */
  GPIO_InitStruct.Pin = DC_Pin|RST_Pin|CE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
