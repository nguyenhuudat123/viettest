#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <math.h> 
#include <stdlib.h>  
#include <algorithm> 

using namespace std;
using namespace cv;

// hàm tạp mask
Mat Fill_Poly(Mat input, vector<Point> vitri, String data, Scalar& color) {
	Mat output;
	if (data == "3") {
		output = Mat::zeros(input.size(), CV_8UC3);
		int num = (int)vitri.size();
		const Point* pt4 = &(vitri[0]);
		fillPoly(output, &pt4, &num, 1, color, 8);
	}
	else if (data == "1") {
		output = Mat::zeros(input.size(), CV_8UC1);
		int num = (int)vitri.size();
		const Point* pt4 = &(vitri[0]);
		fillPoly(output, &pt4, &num, 1, color, 8);
	}

	return output;
};

// gọi x  của Warp

struct Warp_X
{
	vector<float> value_X_Warp_Y_0;

	vector<float> value_X_Warp_Y_720;

	float Xmax_Y_0;

	float Xmin_Y_0;

	float Xmax_Y_720;

	float Xmin_Y_720;
};

// tìm Xmax Xmin
Warp_X Warp_X_min_Max(vector<Vec4f> linesWarp) {
	Warp_X X_Warp;

	for (int i = 0; i < linesWarp.size(); i++)
	{

		int X_1 = linesWarp[i][0];
		int Y_1 = linesWarp[i][1];
		int X_2 = linesWarp[i][2];
		int Y_2 = linesWarp[i][3];

		//cout <<"i = "<<i<< " "<< X_1 << " " << Y_2 << " " << X_2 << " " << Y_2 << " " << endl;
		float lineLength1 = pow(pow(Y_2 - Y_1, 2) + pow(X_2 - X_1, 2), 0.5);
		//cout << lineLength << endl;
		if (lineLength1 > 350) {
			//cout << lineLength1 << endl;
			if (X_1 != X_2) {
				//cout << " X1 != X2 " << endl;
				float a = (Y_2 - Y_1) / (X_2 - X_1);
				float b = Y_2 - a * X_1;
				// tính X tại Y = 720;
				float X_y0 = (0 - b) / a;
				//cout << X_y0 << endl;

				float X_y720 = (720 - b) / a;
				//cout << X_y720 << endl;
				X_Warp.value_X_Warp_Y_0.push_back(X_y0);
				X_Warp.value_X_Warp_Y_720.push_back(X_y720);
			}
			else if (X_1 == X_2) {

				//cout << " X1 = X2 " << endl;
				X_Warp.value_X_Warp_Y_0.push_back(X_1);
				X_Warp.value_X_Warp_Y_720.push_back(X_2);
			}


		}

	}

	sort(X_Warp.value_X_Warp_Y_0.begin(), X_Warp.value_X_Warp_Y_0.end());
	sort(X_Warp.value_X_Warp_Y_720.begin(), X_Warp.value_X_Warp_Y_720.end());

	X_Warp.Xmin_Y_0 = X_Warp.value_X_Warp_Y_0.front();
	X_Warp.Xmax_Y_0 = X_Warp.value_X_Warp_Y_0.back();

	X_Warp.Xmin_Y_720 = X_Warp.value_X_Warp_Y_720.front();
	X_Warp.Xmax_Y_720 = X_Warp.value_X_Warp_Y_720.back();

	return X_Warp;

};



// hàm tìm ra giá trị A và B của các đường

struct R_L {
	// có vạch ko 
	bool trai;
	bool phai;
	//
	vector < vector<float>> lineLeft;
	vector < vector<float>> lineRight;


	//giá trị của hàm đường thẳng

	float Average_A;

	float Average_X;

	float  B;

	;
};


// hàm phân loại các kẻ đường vạch

R_L Right_Left(vector<Vec4f> linesP) {
	R_L Line_R_L;

	Line_R_L.trai = false;
	Line_R_L.phai = false;

	int CounterLeft = 0;
	int CounterRight = 0;

	//

	for (int i = 0; i < linesP.size(); i++)
	{

		float x1 = linesP[i][0];
		float y1 = linesP[i][1];
		float x2 = linesP[i][2];
		float y2 = linesP[i][3];


		float lineLength = pow(pow(y2 - y1, 2) + pow(x2 - x1, 2), 0.5);

		if (lineLength > 15) {
			if (x1 != x2) {
				float a = (y2 - y1) / (x2 - x1);// hệ số góc

				// đường bên phải
				if (a > 0) {
					float tanTheTa = atan(abs(y1 - y2) / abs(x1 - x2));
					//cout << "tan= " << tanTheTa << endl;
					float angle = tanTheTa * 180 / CV_PI;

					// lọc loại các đường ngang và thẳng đứng
					if (abs(angle) < 85 && abs(angle) > 15) {
						Line_R_L.lineRight.resize(CounterRight + 1);
						Line_R_L.lineRight[CounterRight].resize(5);
						Line_R_L.lineRight[CounterRight][0] = x1;
						Line_R_L.lineRight[CounterRight][1] = y1;
						Line_R_L.lineRight[CounterRight][2] = x2;
						Line_R_L.lineRight[CounterRight][3] = y2;
						Line_R_L.lineRight[CounterRight][4] = -a;

						Line_R_L.phai = true;

						CounterRight++;
					}



				}

				// đường bên trái

				if (a < 0) {
					float tanTheTa = atan(abs(y1 - y2) / abs(x1 - x2));
					//cout << "tan= " << tanTheTa << endl;
					float angle = (tanTheTa) * 180 / CV_PI;

					// lọc loại các đường ngang và thẳng đứng
					if (abs(angle) < 85 && abs(angle) > 15) {
						Line_R_L.lineLeft.resize((CounterLeft)+1);
						Line_R_L.lineLeft[CounterLeft].resize(5);
						Line_R_L.lineLeft[CounterLeft][0] = x1;
						Line_R_L.lineLeft[CounterLeft][1] = y1;
						Line_R_L.lineLeft[CounterLeft][2] = x2;
						Line_R_L.lineLeft[CounterLeft][3] = y2;
						Line_R_L.lineLeft[CounterLeft][4] = -a;

						Line_R_L.trai = true;

						CounterLeft++;
					}



				}

			}
		}
	}

	return Line_R_L;

};

// hàm tìm giá trị của A và B

R_L A_B(Mat src, vector <vector<float>> line) {
	R_L A_B;

	///// lấy các hệ số góc của đường
	vector<float> A;
	for (int i = 0; i < line.size(); i++) {
		A.push_back(line[i][4]);

	}
	// 1 sắp xếp dãy hệ số góc của đường
	sort(A.begin(), A.end());

	// 2  tìm median của dãy hệ số góc của đường

	int Mid_A;
	float Median_A;
	if (A.size() % 2 == 0) {
		Mid_A = A.size() / 2;
		Median_A = (A[Mid_A - 1] + A[Mid_A]) / 2;
	}
	else {
		Mid_A = A.size() / 2;
		Median_A = A[Mid_A];
	}

	//// 3  tìm các hệ số góc của đường  thuộc tập hợp 
	vector <float> A_PhuHop;
	float Sum_A = 0;
	for (int i = 0; i < A.size(); i++) {
		if (abs(A[i] - Median_A) < abs(Median_A * 0.2)) {
			A_PhuHop.push_back(A[i]);
			Sum_A = Sum_A + A[i];

		}
	}
	//// tính giá trị trung bình từ mảng hệ số phù hợp của đường 
	A_B.Average_A = Sum_A / A_PhuHop.size();

	/////
	//////// tìm X khi Y=0 của đường phải xoay y xuống dưới

	////1 tìm các X theo a khi Y max
	vector<float> X_below;

	for (int i = 0; i < line.size(); i++) {
		float x1 = line[i][0];
		float y1 = src.rows - line[i][1];
		float a = line[i][4];
		float b = y1 - x1 * a;
		float X_low = -b / a;
		X_below.push_back(X_low);
		//cout << " X_below_Right  " << X_below << endl;
	}
	////2 sắp xếp các X  khi Y max 
	sort(X_below.begin(), X_below.end());

	////3 tìm giá trị Median của dãy X khi Y max

	int Mid_X;
	float Median_X;
	if (X_below.size() % 2 == 0) {
		Mid_X = X_below.size() / 2;
		Median_X = (X_below[Mid_X - 1] + X_below[Mid_X]) / 2;
	}
	else {
		Mid_X = X_below.size() / 2;
		Median_X = X_below[Mid_X];
	}

	//4 tìm các giá trị X phù hợp với Median X khi Y max
	vector <float> X_below_PhuHop;
	float Sum_X = 0;
	for (int i = 0; i < X_below.size(); i++) {
		if (abs(X_below[i] - Median_X) < abs(Median_X * 0.4)) {
			X_below_PhuHop.push_back(X_below[i]);
			Sum_X = Sum_X + X_below[i];
		}
	}
	//// tính giá trị trung bình của X khi Y max
	A_B.Average_X = Sum_X / X_below_PhuHop.size();
	//// B của đường phải

	A_B.B = -A_B.Average_A * A_B.Average_X;

	return A_B;
}


int  main() {
	string path = "samples/testvideo2.mp4";

	VideoCapture cap(path);
	Mat src, gray, blur, matThershold, canny, img, img1, dst, dst_canny, dst2, imgWarp, over, matline, matpoly;
	cout << " bat dau" << endl;
	while (1) {
		cap >> src;

		if (src.empty())break;

		//cout << src.size(); [1280 x 720]
		// tạo mask

		vector<Point> diem{ {480,565}, {736,565 },{956,700},{260,700} };

		Scalar White(255, 255, 255);
		img = Fill_Poly(src, diem, "3", White);
		img1 = Fill_Poly(src, diem, "1", White);

		//xử lý src

		cvtColor(src, gray, COLOR_BGR2GRAY);
		GaussianBlur(gray, blur, Size(3, 3), 0, 0);
		threshold(blur, canny, 150, 255, THRESH_BINARY);//Canny(blur, canny, 100, 200);

		Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));

		// ghép ảnh vào mask

		//bitwise_and(src, img, dst);

		bitwise_and(canny, img1, dst_canny);

		dilate(dst_canny, dst_canny, kernel);


		// lọc các đường thẳng 
		vector<Vec4f> linesP;
		HoughLinesP(dst_canny, linesP, 2, 3.1412 / 180, 100, 0.01, 200);

		// xét phía của vạch kẻ đường và vẽ vạch
		// 
		R_L line_R_L = Right_Left(linesP);

		if (line_R_L.phai == false || line_R_L.trai == false) {
			waitKey(33);
			continue;
		}

		///
		// Xét đường bên trái
		R_L line_A_B_Left = A_B(src, line_R_L.lineLeft);

		// Ptrinh đường Phải : Y =Avergage_A_Right* X + B_Right

		// tọa đội cuối của đường trái (Y=src.rows X= Average_X_Left)

		// Xét đường bên phải

		R_L line_A_B_Right = A_B(src, line_R_L.lineRight);

		// Ptrinh đường trái : Y =Avergage_A_Left* X + B_Left

		// tọa đội cuối của đường phải (Y=src.rows X= Average_X_Right)

		//// tìm giá trị X trái và phải tại Y tọa độ 560
		float y = 560;
		//	// giá trị X trái tại Y tọa độ 560
		float Value_x_Left = ((src.rows - y) - line_A_B_Left.B) / line_A_B_Left.Average_A;
		//	// giá trị X phải tại Y tọa độ 560
		float Value_x_Right = ((src.rows - y) - line_A_B_Right.B) / line_A_B_Right.Average_A;
		//

		///tô màu phần đường

		int  x1 = Value_x_Left;
		int  x2 = Value_x_Right;
		int  x3 = line_A_B_Right.Average_X;
		int  x4 = line_A_B_Left.Average_X;

		vector<Point> hinh_bao{ {x1, 560},
								{x2, 560},
								{x3, src.rows},
								{x4, src.rows} };


		Scalar Blue(237, 96, 61);

		matpoly = Fill_Poly(src, hinh_bao, "3", Blue);
		matThershold = Mat::zeros(src.size(), CV_8UC3);

		///// Vẽ 2 đường line trái phải
		matline = Mat::zeros(src.size(), CV_8UC3);
		////đường trái
		line(matline, Point(line_A_B_Left.Average_X, src.rows), Point(Value_x_Left, 560), Scalar(0, 0, 255), 4);
		//đường phải

		line(matline, Point(line_A_B_Right.Average_X, src.rows), Point(Value_x_Right, 560), Scalar(0, 0, 255), 4);


		// xử lý Warp  
		//  93 633 1127 633 -269 1485
		float w = 1280, h = 720;
		Point2f hinhthang[4] = { {93,633}, {1127,633 },{-269,720},{1485,720} };
		Point2f rec[4] = { {0.0f,0.0f}, {w,0.0f },{0.0f,h} ,{w,h} };
		Mat matrix = getPerspectiveTransform(hinhthang, rec);
		warpPerspective(matline, imgWarp, matrix, Point(w, h));

		// xử lý hình Warp

		Mat gray_Warp, blur_Warp, canny_Warp, fill_Warp, img_fill_Warp;

		cvtColor(imgWarp, gray_Warp, COLOR_BGR2GRAY);
		GaussianBlur(gray_Warp, blur_Warp, Size(3, 3), 0, 0);
		threshold(blur_Warp, canny_Warp, 20, 255, THRESH_BINARY);//Canny(blur, canny, 100, 200);



		// tìm line của Warp
		vector<Vec4f> linesWarp;
		HoughLinesP(canny_Warp, linesWarp, 1, 3.1412 / 180, 150, 100, 220);

		//
		//// tìm vị trí trung tâm Wap tại Y=630
		Warp_X X_Min_Max = Warp_X_min_Max(linesWarp);
		//cout << " Y = 0 << X min   " << X_Min_Max.Xmin_Y_0 << "  X max   " << X_Min_Max.Xmax_Y_0 << endl;
		//cout << " Y = 720 << X min   " << X_Min_Max.Xmin_Y_720 << "  X max   " << X_Min_Max.Xmax_Y_720 << endl;
		// vẽ vị trí min max của đường
		//line(matThershold, Point(X_Min_Max.Xmax_Y_0, 0), Point(X_Min_Max.Xmax_Y_0, 720), Scalar(255, 0, 255), 1);
		//line(matThershold, Point(X_Min_Max.Xmin_Y_0, 0), Point(X_Min_Max.Xmin_Y_0, 720), Scalar(255, 0, 255), 1);

		//line(matThershold, Point(X_Min_Max.Xmax_Y_720, 0), Point(X_Min_Max.Xmax_Y_720, 720), Scalar(255, 20, 0), 1);
		//line(matThershold, Point(X_Min_Max.Xmin_Y_720, 0), Point(X_Min_Max.Xmin_Y_720, 720), Scalar(255, 20, 0), 1);

		// Xác định hướng di chuyển

		int Warp_X_Center_Y_0 = (X_Min_Max.Xmax_Y_0 + X_Min_Max.Xmin_Y_0) / 2;
		//line(matThershold, Point(Warp_X_Center_Y_0, 0), Point(Warp_X_Center_Y_0, 1280), Scalar(255, 0, 0), 3);
		int Warp_X_Center_Y_720 = (X_Min_Max.Xmax_Y_720 + X_Min_Max.Xmin_Y_720) / 2;
		//line(matThershold, Point(Warp_X_Center_Y_720, 0), Point(Warp_X_Center_Y_720, 1280), Scalar(0, 0, 255), 3);

		//cout << " Y_720" << Warp_X_Center_Y_720 << " Y_ 0 " << Warp_X_Center_Y_0 << endl;

		if (Warp_X_Center_Y_720 > Warp_X_Center_Y_0 * 0.98 && Warp_X_Center_Y_0 < Warp_X_Center_Y_0 * 1.02) {
			putText(src, "DI THANG ", Point(300, 300), FONT_HERSHEY_SIMPLEX, 2, Scalar(255, 40, 0), 3);
		}
		else if (Warp_X_Center_Y_720 >= Warp_X_Center_Y_0 * 1.02 && Warp_X_Center_Y_720 < Warp_X_Center_Y_0 * 1.05) {
			putText(src, " DI SANG TRAI 1", Point(300, 300), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 40, 255), 3);
		}
		else if (Warp_X_Center_Y_720 >= Warp_X_Center_Y_0 * 1.05 && Warp_X_Center_Y_720 < Warp_X_Center_Y_0 * 1.1) {
			putText(src, " DI SANG TRAI 2", Point(300, 300), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 40, 255), 3);
		}
		else if (Warp_X_Center_Y_720 <= Warp_X_Center_Y_0 * 0.98 && Warp_X_Center_Y_720 > Warp_X_Center_Y_0 * 0.95) {

			putText(src, "DI SANG PHAI 1  ", Point(300, 300), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 40, 255), 3);
		}
		else if (Warp_X_Center_Y_720 <= Warp_X_Center_Y_0 * 0.95 && Warp_X_Center_Y_720 > Warp_X_Center_Y_0 * 0.9) {

			putText(src, "DI SANG PHAI 2 ", Point(300, 300), FONT_HERSHEY_SIMPLEX, 2, Scalar(0, 40, 255), 3);
		}

		addWeighted(src, 0.8, matline, 1, 0.0, src);
		addWeighted(src, 0.8, matpoly, 1, 0.0, src);

		//imshow("canny", dst_canny);
		imshow("warp", dst_canny);
		imshow("canny_Warp", imgWarp);
		imshow("src", src);
		imshow("matline", matline);
		imshow("matThesh", matThershold);


		char ch = (char)waitKey(33);//nhận giữ liệu từ bàn phím
		if (ch == 27) break;

	}
	cap.release();

	cv::destroyAllWindows();

	return 0;


}

