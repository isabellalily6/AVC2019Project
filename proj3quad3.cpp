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
    int quadrant;
    const int cam_width = 320;
    const int cam_height = 240;
    const int v_left_go = 51;
    const int v_right_go = 43;
    double kp = 0.0004;
    double kd = 0.0002;
    double err;
    int line_present = 1;
    int prev_error;
    public:
    //Rob(){};
	int InitHardware();
	int turnAround();
	int SetMotors();
	int MeasureLine();
	int FollowLine();
	//void turnRight();
	//void turnLeft();
	
		
};

int Robot::MeasureLine(){ //only coded for quad 2 rn
	float totwhite = 0;	
	float whiteArr[cam_width];
	float errorArray[cam_width];
	int whiteBool = 0;
	double threshold = 0;
	line_present = 1;
	
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
			
			totwhite = get_pixel(240/2, countCol,3);
			if(totwhite > threshold - 15){
				whiteArr[countCol] = 0;
			} else {
			whiteArr[countCol] = 1;	
			}
			line_error += whiteArr[countCol] * (countCol-middleIndex);
			}
			clock_gettime(CLOCK_MONOTONIC, &ts_end);
			long dt = ts_end.tv_sec-ts_start.tv_sec) * 1000000000 + ts_end.tv_nsec-ts_start.tv_nsec;
			prev_error = line_error;
			err = (int)(line_error*kp) + (int)(((line_error - prev_error) * kd)/dt);
					
		    printf("\nwhiteness: %.1f",totwhite);
	
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
		err = (int)(line_error*kp);
		
		
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
		printf(" \nline error: %.1f err: %d",line_error,err);
		
		
		SetMotors();
	} else {
			printf("%nLine missing");			
			turnAround();
			sleep1(100);
			SetMotors();
			
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
