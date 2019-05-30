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
    int quadrant = 1;
    const int cam_width = 320;
    const int cam_height = 240;
    const int v_left_go = 51;
    const int v_right_go = 46;
    double kp = 0.0005;
    double kd = 0.0006;
    double err;
    int line_present = 1;
    int prev_error;
    int turnLeftBool = 0;
    int turnRightBool = 0;
    public:
    //Rob(){};
	int InitHardware();
	int turnAround();
	int SetMotors();
	int MeasureLine();
	int FollowLine();
	void openGate();
	void turnRight();
	void turnLeft();
		
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
	v_left = 65;
	v_right = 30;
	SetMotors();
	sleep1(2000);
	quadrant = 2;
	v_left = 51;
	v_right = 46;
}

int Robot::MeasureLine(){ //only coded for quad 2 rn
	
	float totwhite = 0;	
	float totredavg = 0;
	float totblueavg =0;
	float whiteArr[cam_width];
	float errorArray[cam_width];
	int whiteBool = 0;
	double threshold = 0;
	line_present = 1;
	
	
	if(quadrant == 2) {
	for(int i = 0; i < cam_width; i++){
		threshold += get_pixel(120,i,3);
	}
	threshold /= cam_width;
	//how to get array of white pixel? use that for totwhite
	
	//for(int countRow = 0; countRow < 240; countRow++) {
	int middleIndex = (cam_width - 1)/2;
	line_error = 0;
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
			line_error += whiteArr[countCol] * (countCol-middleIndex);
			}
			clock_gettime(CLOCK_MONOTONIC, &ts_end);
			long dt = (ts_end.tv_sec-ts_start.tv_sec) * 1000000000 + ts_end.tv_nsec-ts_start.tv_nsec;
			prev_error = line_error;
			err = ((line_error*kp) + (((line_error - prev_error) * kd)/dt));
					
		    printf("\nwhiteness: %.1f",totwhite);
		    	totredavg /= cam_width;
				totblueavg /= cam_width;
				printf("\n red: %.3f blue: %.3f",totredavg, totblueavg);
				if (totredavg - totblueavg > 130){
				quadrant = 3;
				v_left = v_left_go;
				v_right = v_right_go;
				SetMotors();
				printf("\n Next Quadrant now at quad: %d",quadrant);
				printf("\n Threshold: %f", threshold);
				return 0;
			}  
		} else if(quadrant == 3) {	
			totredavg =0;
			totblueavg =0;
			//printf("\ncurrent quadrant %d",quadrant);		
			
			double lineTurn = 0; //- is left + is right
			int line3 = 0;
			int middleIndex = (cam_width - 1)/2;
			line_error = 0;
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
				line_error += whiteArr[countCol] * (countCol-middleIndex);
				line3 += whiteArr[countCol];
				if(whiteArr[countCol] == 1){
					lineTurn += countCol - middleIndex; //to count the pos of the line - l or r ?
			    }
				
				/* make a detection for a right hand turn
				 * if(whiteArr[countCol] == 1){
					lineTurn = countCol;
				} */
			}
			
			totredavg /= cam_width;
			totblueavg /= cam_width;
			lineTurn = lineTurn/cam_width; // avg line pos 			
				
				clock_gettime(CLOCK_MONOTONIC, &ts_end);
				long dt = (ts_end.tv_sec-ts_start.tv_sec) * 1000000000 + ts_end.tv_nsec-ts_start.tv_nsec;
				prev_error = line_error;
				err = (int)((line_error*kp) + ((line_error - prev_error) * kd/dt));
				printf("\nline3: %d lineTurn %f middleIndex: %d",line3,lineTurn,middleIndex);
			if(line3 == 0) {
				turnLeftBool =1;
				printf("\nline3 is 0");
							
			} else if (lineTurn > 0) {
				turnRightBool =1;
			} else if (lineTurn < 0) {
				turnLeftBool =1;
			}						
				printf("\nwhiteness: %.1f red: %.1f blue: %.1f",totwhite);	
					
			if (totredavg - totblueavg > 130){
				quadrant - 3;
				v_left = v_left_go;
				v_right = v_right_go;
				SetMotors();
				printf("\n Next Quadrant now at quad: %d",quadrant);
			}
		return 0;
		}
	
	return 0;	
} 
int Robot::SetMotors(){
	
	 set_motors(3, v_left);
	 set_motors(5, v_right);
	 printf("\n v_left: %d v_right: %d",v_left,v_right);
	 hardware_exchange();
	 
	 return 0; 

}
int Robot::FollowLine(){
	MeasureLine();
	if(line_present == 1) {
		//err = (int)(line_error*kp);
		
		if(quadrant == 2) {
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
		} else if(turnLeftBool == 2) {
		turnRight();
		turnRightBool = 0;	
		printf("\nturning right");
		printf(" \nline error: %.1f err: %.3f",line_error,err);
		sleep1(60);
		}
		else {
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
int Robot::turnAround() {
	if (v_left > 48){
		v_left = 48 - (v_left-48);
	} else {
		v_left = 48 + (48 - v_left);
	}
	if (v_right > 48){
		v_right = 48 - (v_right-48);
	} else {
		v_right = 48 + (48 - v_right);
	}
	
	return 0;
}
void Robot::turnLeft() {
		set_motors(5, 48);
		set_motors(3, 52);
		sleep1(200);
		printf("turn over");
		set_motors(5, v_right_go);
		set_motors(3, v_left_go);
		
}
void Robot::turnRight() {
		set_motors(5, v_right_go);
		set_motors(3, v_left_go);
		sleep1(200);
		printf("turn over");
		set_motors(5, v_right_go);
		set_motors(3, v_left_go);
		
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
