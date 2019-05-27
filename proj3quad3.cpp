#include <stdio.h>
#include <time.h>
#include "E101.h"
//og size 240,320
//camera has a 6:8 ratio
class Robot{
	private:
	int v_left, v_right, cam_tilt;
    int dv;
    double line_error =0;
    int quadrant;
    const int cam_width = 320;
    const int cam_height = 240;
    const int v_left_go = 52;
    const int v_right_go = 43;
    double kp = 0.05;
    int line_present = 0;
    public:
    //Rob(){};
	int InitHardware();
	void ReadSetMotors();
	int SetMotors();
	int MeasureLine();
	int FollowLine();
	
		
};

int Robot::MeasureLine(){ //only coded for quad 2 rn
	float totwhite = 0;	
	float whiteArr[cam_width];
	float errorArray[cam_width];
	int whiteBool = 0;

	//how to get array of white pixel? use that for totwhite
	
	//for(int countRow = 0; countRow < 240; countRow++) {
		for(int countCol = 0; countCol < 320; countCol++){
			totwhite = get_pixel(240/2, countCol,3);
			if(totwhite > 0.3){
				whiteBool = 0;
			} else {
			whiteBool = 1;	
			}
			whiteArr[countCol] = whiteBool;
		}
		int middleIndex = cam_width/2;
		for(int i = 0; i < 320; i++){
			errorArray[i] = whiteArr[i] - (i - middleIndex);
			printf("\nerrorArray: %f",errorArray[i]);
		}	
		for(int i = 0; i < 320; i++){
			line_error = line_error + errorArray[i];
		}		
	
	return 0;	
} 
int Robot::SetMotors(){
	
	 set_motors(3, v_left);
	 set_motors(5, v_right);
	 hardware_exchange();
	 return 0;
	 

}
int Robot::FollowLine(){
	MeasureLine();
	if(line_present == 1) {
		dv = (int)(line_error*kp);
		v_left = v_left_go + dv;
		v_right = v_right_go + dv;
		printf(" line error: %f dv: %d",line_error,dv);
		SetMotors();
	} else {
			printf(" Line missing");
			v_left = 55;
			v_right = 40;
			SetMotors();
			
	}
	return 0;
}
int Robot::InitHardware(){
	init(0);
	open_screen_stream();
	take_picture();
	update_screen();
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
