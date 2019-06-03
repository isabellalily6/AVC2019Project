#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "E101.h"

//og size 240,320
//camera has a 6:8 ratio
//todo add line present thingy
class Robot{
	private:
	int v_left, v_right, cam_tilt;
    int dv;
    double line_error =0;
   int prevLineCount = 0;
   
    
    double leftLine_error = 0;
    double rightLine_error = 0;
    int quadrant = 2;
    const int cam_width = 320;
    const int cam_height = 240;
    const int v_left_go = 50;
    const int v_right_go = 46;
    double kp = 0.0005;
    double kd = 0.003;
    double err;
    double prevErr = 0;
    
    int corner = 0;
    int prev_error;
    
    int reverseBool = 0;
    int deadEndBool = 0;
    int xroadBool = 0;
    public:
    //Rob(){};
	int InitHardware();
	int reverse();
	int SetMotors();
	int MeasureLine();
	int FollowLine();
	void openGate();

	void fullTurn();
		
};
void Robot::openGate(){
	char server_addr[15] = {'1', '3', '0', '.', '1', '9', '5', '.', '6', '.', '1', '9', '6'};
	char message[24] = {'P', 'l', 'e', 'a', 's', 'e'};
	char password[24];
	int port  = 1024;
	connect_to_server(server_addr, port);
	send_to_server(message);
	receive_from_server(password);
	send_to_server(password);
	v_left = 52;
	v_right = 44;
	SetMotors();
	sleep1(2000);
	quadrant = 2;
	v_left = v_left_go;
	v_right = v_right_go;
	SetMotors();
}

int Robot::MeasureLine(){ //only coded for quad 2 rn
	
	
	float totwhite = 0;	
	float vertWhite = 0;
	float totredavg = 0;
	float totblueavg =0;
	float whiteArr[cam_width];
	float vertWhiteArr[cam_height];
	float errorArray[cam_width];
	int whiteBool = 0;
	
	double threshold = 60;
	//double prevThresh = 100;
	
		
	if(quadrant == 2) {
	
	//how to get array of white pixel? use that for totwhite
	
	//for(int countRow = 0; countRow < 240; countRow++) {
	int middleIndex = (cam_width - 1)/2;
	line_error = 0;
	int lineCount = 0;
	int vertLineCount = 0;
	
	struct timespec ts_start;
	//struct timespec deadStart;
	struct timespec ts_end;
	
	rightLine_error = 0;
	leftLine_error = 0;
	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	
	
		for(int countCol = 0; countCol < 320; countCol++){
			
			totwhite = get_pixel(240/2, countCol,3); //for err line 
			//printf("\n\n\n\ntotwhite: %.5f",totwhite);
			
			totredavg += get_pixel(cam_height/2, countCol,0); //for red sensor
			totblueavg += get_pixel(cam_height/2, countCol,2);
			
			if(totwhite > threshold){
				whiteArr[countCol] = 0; //0 is white
			} else {
			whiteArr[countCol] = 1;	//1 is black
			}
			line_error += whiteArr[countCol] * (countCol-middleIndex);
			lineCount += whiteArr[countCol];
			}
			
			clock_gettime(CLOCK_MONOTONIC, &ts_end);
			
			long dt = (ts_end.tv_sec-ts_start.tv_sec) * 1000000000 + ts_end.tv_nsec-ts_start.tv_nsec;
			
			err = ((line_error*kp) + (kd * ((line_error - prev_error)/dt)));
			
			
			printf("\nerr: %.5f preverror: %.5f",err, prevErr);
			/*
			 * 
			 * for vert loop
			 * 
			 * 
			 */
			
			for(int countRow = 0; countRow < cam_height; countRow++){
				
				vertWhite = get_pixel(countRow, cam_width/2 ,3); //for err line
				//printf("\n\n\n\nVERTtotwhite: %.5f",vertWhite);
				
				if(totwhite > threshold){
					vertWhiteArr[countRow] = 0; //0 is white
				} else {
					vertWhiteArr[countRow] = 1;	//1 is black
				}
			
			
				vertLineCount += whiteArr[countRow];
			}
			printf(" \n\nn vertLineCount: %d \n\nn\n",vertLineCount);
				
				
				
			
			
			
				totredavg /= cam_width;
				totblueavg /= cam_width;
				
			printf("\n\nLineCount: %d\n\n",lineCount);
			if(err > 1 || err < -1 ) {
				corner = 1;
			}
			if(vertLineCount > 50 && (err < 1 || err > -1) && lineCount < 130){
			corner = 0;	
			}
			
			
			if (lineCount < 20 && vertLineCount < 20 && err == 0 && prevErr < 1 && corner == 0){
				printf("\n\n\n------\n\n\n\n\n ------ 123 turn around 123  --------\n\n\n------\n\n\n\n\n\n\n\n");
				
				v_left = 38;
				v_right = v_right_go - 5;
				SetMotors();
				sleep1(1500);
				v_left = 48;
				v_right = 48;
				SetMotors();
				sleep1(1000);
			}else if((lineCount < 50  && vertLineCount > 0) || (corner == 1 && lineCount < 50 && vertLineCount < 50) ) { //0 might be too harsh for this - needs testing
					
					reverseBool = 1; //if the line is not present reverse
					return 0;
			} else if ((lineCount >= 215 && (err < 1 || err > 5.25) && (lineCount - prevLineCount) < 50 && xroadBool == 1) || lineCount > 280){
			printf("\n------------------------------------------------------------------------------------\n\n\n\n Robot is at a cross road\n\n\n\n\n")	;
			v_left = v_left_go;
			v_right = v_right_go;
			SetMotors();
			sleep1(450);
			v_left = 38;
			v_right = v_right_go - 4;
			SetMotors();
			sleep1(700);
			xroadBool = 0;
			printf("\nturned left");
			return 0;
			} else if(lineCount >= 200){
				
				printf("\n$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n\n\n lemme think\n\n\n");
				v_left = 48;
				v_right = 48;
				SetMotors();
				sleep1(400);
				xroadBool = 1;
							
			}
			
			
			
			
			
			/*if(err > 4 && vertLineCount > 50 && lineCount > 120 && lineCount < 195)	{
				printf("\n\n\n\n\n\n\n\n\n\n\n*****************************************************\n\n\n\n\n\n\n\n\n\n\n\nInching forwards ...");
			
				v_left = v_left_go + 1;
				v_right = v_right_go - 1;
				SetMotors();
				sleep1(600);
				return 0;
				
				
			} */
		    //printf("\nwhiteness: %.1f",totwhite);
			 printf("\n new pic :");
			 prevLineCount = lineCount;
			 prevErr = err;
			 prev_error = line_error;
		} 
	
	return 0;	
} 
int Robot::SetMotors(){
	
	 set_motors(3, v_left);
	 set_motors(5, v_right);
	 printf("\nSet Motors: v_left: %d v_right: %d",v_left,v_right);
	 hardware_exchange();
	 
	 return 0; 

}
int Robot::FollowLine(){
	MeasureLine();
	
		if(quadrant == 2) {
			if(reverseBool == 1){ 
					reverse();
			}			
		v_left = v_left_go + err;
		v_right = v_right_go + err;
		if(v_left > 65) {
			v_left = 65;
				
		} else if(v_left < 30){
			v_left = 30;				
		}
		if(v_right > 65){
				v_right = 65;
		} else if(v_right < 30) {
			v_right = 30;
			}
		
				
		SetMotors();
	} 
	
	return 0;
}
int Robot::reverse() {
	
	v_right = 51;
	v_left = 42;
	SetMotors();
	printf("\n\nReversing now\n\n");
	sleep1(400);
	printf("\n\nReversing over\n\n");
	reverseBool = 0;
	//sleep1(2000);
	
	return 0;
}

void Robot::fullTurn() {
		v_left = 44;
		v_right = 52;
		SetMotors();
		sleep1(2400);
		printf("\n\n\n------\n\n\n\n\n ------ turn around over --------\n\n\n------\n\n\n\n\n\n\n\n");
		v_left = v_left_go;
		v_right =v_right_go;
		SetMotors();		
}

int Robot::InitHardware(){
	init(0);
	open_screen_stream();
	take_picture();
	update_screen();
	v_left = v_left_go;
	v_right = v_right_go;
	SetMotors();
	hardware_exchange();
	
	return 0;
}
int main() {
	int courseOver = 0;
	Robot robot;
	robot.InitHardware();
	//robot.openGate();
	
	
	while(true){
		//robot.MeasureLine();
		robot.FollowLine();
		take_picture();
		update_screen();
		
		if(courseOver == 1){
				break;
		}
	}
		close_screen_stream();
		//stoph();
		return 0;
}
