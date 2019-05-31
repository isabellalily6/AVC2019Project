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
    double leftLine_error = 0;
    double rightLine_error = 0;
    int quadrant = 3;
    const int cam_width = 320;
    const int cam_height = 240;
    const int v_left_go = 52;
    const int v_right_go = 44;
    double kp = 0.0003;
    double kd = 0.0007;
    double err;
    int line_present = 1;
    int prev_error;
    int turnLeftBool = 0;
    int turnRightBool = 0;
    int reverseBool = 0;
    int deadEndBool = 0;
    public:
    //Rob(){};
	int InitHardware();
	int reverse();
	int SetMotors();
	int MeasureLine();
	int FollowLine();
	void openGate();
	void turnRight();
	void turnLeft();
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
	v_left = 64;
	v_right = 30;
	SetMotors();
	sleep1(2000);
	quadrant = 3;
	v_left = v_left_go;
	v_right = v_right_go;
	SetMotors();
}

int Robot::MeasureLine(){ //only coded for quad 2 rn
	
	float totwhite = 0;	
	float totredavg = 0;
	float totblueavg =0;
	float whiteArr[cam_width];
	float errorArray[cam_width];
	int whiteBool = 0;
	double threshold = 100;
	//double prevThresh = 100;
	line_present = 1;
		
	if(quadrant == 2) {
	/*for(int i = 0; i < cam_width; i++){
		threshold += get_pixel(120,i,3);
	}
	threshold /= cam_width;
	printf("\nThreshold: %f prevThresh: %f:",threshold,prevThresh);
	if(threshold > prevThresh + 50){
		printf("setting thresh to prevthresh");
		threshold = prevThresh;
		prevThresh = 100;
	} else {
		prevThresh = threshold;
		printf("\n prevThresh: %f",prevThresh);
	} */
	
	//how to get array of white pixel? use that for totwhite
	
	//for(int countRow = 0; countRow < 240; countRow++) {
	int middleIndex = (cam_width - 1)/2;
	line_error = 0;
	int lineCount = 0;
	struct timespec ts_start;
	struct timespec ts_end;
	rightLine_error = 0;
	leftLine_error = 0;
	clock_gettime(CLOCK_MONOTONIC, &ts_start);
		for(int countCol = 0; countCol < 320; countCol++){
			
			totwhite = get_pixel(240/2, countCol,3); //for err line 
			
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
			totredavg /= cam_width;
				totblueavg /= cam_width;
				//printf("\n red: %.3f blue: %.3f",totredavg, totblueavg);
				if (totredavg - totblueavg > 130){
				quadrant = 3;
				v_left = v_left_go;
				v_right = v_right_go;
				SetMotors();
				printf("\n Next Quadrant now at quad: %d",quadrant);
				sleep1(300);
				//printf("\n Threshold: %f", threshold);
				return 0;
			}
			//printf("\n\nLineCount: %d",lineCount);
			if(lineCount < 50 ) { //0 might be too harsh for this - needs testing
					reverseBool = 1; //if the line is not present reverse
					return 0;
			}
			
				
			
			clock_gettime(CLOCK_MONOTONIC, &ts_end);
			long dt = (ts_end.tv_sec-ts_start.tv_sec) * 1000000000 + ts_end.tv_nsec-ts_start.tv_nsec;
			
			err = ((line_error*kp) + (((line_error - prev_error) * kd)/dt));
			prev_error = line_error;		
		    //printf("\nwhiteness: %.1f",totwhite);
		    	  
		} else if(quadrant == 3) {	//quad3
			totredavg =0;
			totblueavg =0;
			//printf("\ncurrent quadrant %d",quadrant);		
			
			double lineTurn = 0; //- is left + is right
			int line3 = 0;
			int middleIndex = (cam_width - 1)/2;
			line_error = 0;
			rightLine_error = 0;
			leftLine_error = 0;
			struct timespec ts_start;
			struct timespec ts_end;
			clock_gettime(CLOCK_MONOTONIC, &ts_start);
			
			for(int countCol = 0; countCol < 320; countCol++){
											
				totwhite = get_pixel(240/2, countCol,3); //for err line 
			
				totredavg += get_pixel(cam_height/2, countCol,0); //for red sensor
				totblueavg += get_pixel(cam_height/2, countCol,2);
				
				if(totwhite > threshold){
					whiteArr[countCol] = 0;
				} else {
					whiteArr[countCol] = 1;	
				}
				line_error += whiteArr[countCol];
				line3 += whiteArr[countCol];
				if(whiteArr[countCol] == 1){
					lineTurn += (countCol) - (160); //to count the pos of the line - l or r ?
			    }
				
				/* make a detection for a right hand turn
				 * if(whiteArr[countCol] == 1){
					lineTurn = countCol;
				} */
			}
			for(int countCol = 0; countCol < (cam_width/3) - 30; countCol++){
				totwhite = get_pixel(240/2, countCol,3); //for err line 
				if(totwhite > threshold){
					whiteArr[countCol] = 0;
				} else {
					whiteArr[countCol] = 1;	
				}
				leftLine_error += whiteArr[countCol];
			}
			for(int countCol = ((cam_width*2)/3) + 30; countCol < cam_width; countCol++){
				totwhite = get_pixel(240/2, countCol,3); //for err line 
				if(totwhite > threshold){
					whiteArr[countCol] = 0;
				} else {
					whiteArr[countCol] = 1;	
				}
				rightLine_error += whiteArr[countCol];
			}
			printf("\n\nRightline error: %.5f LeftlineError: %.5f",rightLine_error,leftLine_error);
			//87 seems to determine which turn
			
		
			printf("\n\nline turn before avging: %.3f",lineTurn); //debug purpose
			
			totredavg /= cam_width;
			totblueavg /= cam_width;
			//lineTurn = lineTurn/cam_width; // avg line pos - maybe this shouldnt be avged?	
			printf("\n\nline turn AFTER avging: %.3f",lineTurn); //debug purpose		
				
				clock_gettime(CLOCK_MONOTONIC, &ts_end);
				long dt = (ts_end.tv_sec-ts_start.tv_sec) * 1000000000 + ts_end.tv_nsec-ts_start.tv_nsec;
				
				err = (int)((line_error*kp) + ((line_error - prev_error) * kd/dt));
				prev_error = line_error;
				printf("\nline3: %d lineTurn %.3f middleIndex: %d",line3,lineTurn,middleIndex);
			if(line3 > 230) { //crossroad sensor
				turnLeftBool =1;
				printf("\nrobot is at a cross roads");
				sleep1(270);
				return 0;
			} else if(line3 < 10) {
				printf("\n\n\n\n Dead End\n\n\n\n");
				deadEndBool =1;
			} else if (rightLine_error - leftLine_error > 80) { // to turn based off avg line pos with wiggle room
				turnRightBool = 1;
				printf("\n\n\n\nrobot wants to turn to the right\n\n\n"); //for debug
				sleep1(95);
				
				
			} else if (leftLine_error - rightLine_error > 80) { //to turn based off avg line pos with wiggle room looks to be -30 ish based on testing
				turnLeftBool =1;
				printf("\n\n\n\nrobot wants to turn to the left\n\n\n\n");
				sleep1(95);
				
			} else{
				printf("\n\nnone of the above conditions are met - must be straight ? \n\n"); //lineavg is -15 for a straight line
				}	 //for debug					
				printf("\n%d: Whiteness: %.1f red: %.1f blue: %.1f",quadrant,totwhite,totredavg,totblueavg);	
					
			if (totredavg - totblueavg > 130){
				quadrant = 3;
				v_left = v_left_go;
				v_right = v_right_go;
				SetMotors();
				printf("\n Next Quadrant now at quad: %d",quadrant);
				sleep1(300);
			}
		return 0;
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
	if(line_present == 1) {
		//err = (int)(line_error*kp);
		
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
		printf(" \nline error: %.1f err: %.3f",line_error,err);
				
		SetMotors();
	} else if(quadrant == 3){
		//printf("\ncurrent quadrant %d",quadrant);
		if(turnLeftBool == 1) {
		turnLeft();
		turnLeftBool = 0;
		printf("\n turning left");
		printf(" \nline error: %.3f err: %.3f",line_error,err);
		sleep1(60);	
		} else if(turnRightBool == 1) {
		turnRight();
		turnRightBool = 0;	
		printf("\nturning right");
		printf(" \nline error: %.1f err: %.3f",line_error,err);
		sleep1(60);
		} else if(deadEndBool ==1) {
			fullTurn();
			deadEndBool =0;
		} else {
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
		printf(" \nline error: %.1f err: %.3f",line_error,err);
		
		
		SetMotors();
		}
	} 
	}
	return 0;
}
int Robot::reverse() {
	/*if (v_left > 48){
		v_left = 48 - (v_left_go-48);
	} else {
		v_left = 48 + (48 - v_left_go);
	}
	if (v_right > 48){
		v_right = 48 - (v_right_go-48);
	} else {
		v_right = 48 + (48 - v_right_go);
	} */
	v_right = 51;
	v_left = 42;
	SetMotors();
	printf("\n\nReversing now\n\n");
	sleep1(300);
	printf("\n\nReversing over\n\n");
	reverseBool = 0;
	//sleep1(2000);
	
	return 0;
}
void Robot::turnLeft() {
		v_left = 44;
		v_right = 44;
		SetMotors();
		sleep1(1700);
		printf("turn left over");
		v_left = v_left_go;
		v_right =v_right_go;
		SetMotors();
		
}
void Robot::turnRight() {
		v_left = 52;
		v_right = 47;
		SetMotors();
		sleep1(1700);
		printf("turn right over");
		v_left = v_left_go;
		v_right =v_right_go;
		SetMotors();		
}
void Robot::fullTurn() {
		v_left = 41;
		v_right = 45;
		SetMotors();
		sleep1(2400);
		printf("turn around over");
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
	robot.openGate();
	
	
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
