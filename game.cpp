#include<iostream>
#include<opencv2/opencv.hpp>
#include<string>
#include <zconf.h>


using namespace std;
using namespace cv;

Mat getScreenImage();

int conformGameState(Mat srcImage, Mat result, Mat closeImage, Mat nextImage);

void buildDfsMap(Mat srcImage, Mat templateImage, Mat result);

void dfs(int bx, int by, int blockSize);

void touchScreen(int x, int y);

void touchScreen();

void showZoomImage(Mat srcImage);


int blockCount = 0;
int gameMap[10][10];
int dx[4] = {0, -1, 0, 1};
int dy[4] = {-1, 0, 1, 0};
int qeue[50][2];
int qeuePosition = 0;

int n; //x 的长度
int m; //y的长度
int dfsBeginX;
int dfsBeginY;



int tempQeue[50][2];
int mapPosition[10][10][2];

int blockSize = 150;

Mat templateImage = imread("/Users/saltedfish/Desktop/123.png");
Mat nextImage = imread("/Users/saltedfish/Desktop/next.jpeg");
Mat closeImage = imread("/Users/saltedfish/Desktop/close.png");
Mat srcImage;
int main() {

    while (1) {


        srcImage = getScreenImage();
//    Mat bakImage=srcImage.clone();

        Mat result;
        int result_cols = srcImage.cols - blockSize + 1;
        int result_rows = srcImage.rows - blockSize + 1;
        result.create(result_cols, result_rows, srcImage.type());
        matchTemplate(srcImage, templateImage, result, TM_CCOEFF_NORMED);



        int gameState = conformGameState(srcImage, result, closeImage, nextImage);

        //写出文件,调试看
        imwrite("/Users/saltedfish/Desktop/debug.png", srcImage);

        if (gameState == 1) {
            buildDfsMap(srcImage, templateImage, result);
            dfs(dfsBeginX, dfsBeginY, blockCount);
            showZoomImage(srcImage);
            touchScreen();

            blockCount = 0;
            qeuePosition = 0;
            sleep(2);
        }

        if (gameState == 2) {
            touchScreen(1524, 555);
            sleep(1);
        }

        if (gameState == 3) {
            touchScreen(809, 927);
            touchScreen(1299, 555);
            sleep(1);
        }


        if (gameState == -1) {
            cout << "游戏出现异常";
            break;
        }

        cout << blockCount << endl;


    }
}


void print()//打印操作
{
    for (int i = 0; i <= qeuePosition - 1; i++) 
    {
        for (int i = 0; i < 50; i++) {
            tempQeue[i][0] = qeue[i][0];
            tempQeue[i][1] = qeue[i][1];
        }
        printf("(%d,%d)", qeue[i][0], qeue[i][1]); 
        if (i != qeuePosition - 1)printf("->"); 
    }
//    return;
}

void dfs(int x, int y, int blockSize) {
    if (blockSize == 0) {
        cout << "找到答案了";
        print();
        cout << endl;
        return;
    }

    for (int i = 0; i < 4; i++) {
        int nextX = x + dx[i];
        int nextY = y + dy[i];
        if (nextX >= 0 && nextX <= n && nextY >= 0 && nextY <= m && gameMap[nextX][nextY] == 1) {
            //下一步可走
            gameMap[nextX][nextY] = 0;
            qeue[qeuePosition][0] = nextX;
            qeue[qeuePosition][1] = nextY;
            qeuePosition++;
            dfs(nextX, nextY, blockSize - 1);

            gameMap[nextX][nextY] = 1;
            qeuePosition--;
        }
    }
}

void touchScreen(int y, int x) {
    string allcommand;
    string command = "adb shell input tap  ";
    command += to_string(x);
    command += " ";
    command += to_string(y);
    cout << "touch " << x << "," << y << endl;
    system(command.c_str());
}




//x:1080
//y:2220

void touchScreen() {

    ofstream outfile;


    outfile.open("../command.sh", ios::ate);


//    ofstream ofile ;
//    ofile=open("command.sh");
    string allCommand = "";
    for (int i = 0; i < blockCount; i++) {


        circle(srcImage, Point(mapPosition[tempQeue[i][0]][tempQeue[i][1]][1] ,
                               mapPosition[tempQeue[i][0]][tempQeue[i][1]][0] ), 40, Scalar(0, 0, 255), -1);
//        showZoomImage(srcImage);
        String command = "sendevent /dev/input/event2 3 53 ";
        command += to_string((int) mapPosition[tempQeue[i][0]][tempQeue[i][1]][1] / 1080.0 * 4096);
        command += "\nsendevent /dev/input/event2 3 54  ";
        command += to_string((int) mapPosition[tempQeue[i][0]][tempQeue[i][1]][0] / 2220.0 * 4096);
        command += "\n sendevent /dev/input/event2 1 330 1\n"
                   "sendevent /dev/input/event2 0 0 0\n"
                   "sendevent /dev/input/event2 1 330 0\n"
                   "sendevent /dev/input/event2 0 0 0\n";
        cout<<command;
        allCommand += command;
    }
    outfile << allCommand << endl;
    outfile.close();

    system("adb push ../command.sh /sdcard/command.sh");
    system("adb shell sh /sdcard/command.sh");
}

void showZoomImage(Mat srcImage) {
    int nRows = 850;
    int nCols = srcImage.cols * 850 / srcImage.rows;
    Mat dst(nRows, nCols, srcImage.type());
    resize(srcImage, dst, dst.size(), 0, 0, INTER_LINEAR);

    namedWindow("zoomImage", WINDOW_NORMAL);
    imshow("zoomImage", dst);
    waitKey(1);

}

Mat getScreenImage() {

    string screenImageAndDownLoadCommand = "adb shell screencap -p /sdcard/screen.png \n";
    screenImageAndDownLoadCommand += "adb pull /sdcard/screen.png ..";
    system(screenImageAndDownLoadCommand.c_str());

    Mat srcImage = imread("../screen.png");
    return srcImage;
}

void buildDfsMap(Mat srcImage, Mat templateImage, Mat result) {
    //开始构建地图
    //随便获取一个位置 ,这里以最匹配的为开始位置
    Point maxPoint;
    Point minPoint;

    minMaxLoc(result, NULL, NULL, &minPoint, &maxPoint);
    //以开始位置为起点,向四周扫描 确定地图的范围
    int scanBeginX = maxPoint.x;
    int scanBeginY = maxPoint.y;

    cout << blockSize << endl << blockSize << "end\n";

    int mapBeginX = srcImage.cols;
    int mapBeginY = srcImage.rows;
    int mapEndX = 0;
    int mapEndY = 0;


    //灰色点三通道的值大概是204 204 204 左右


    for (int i = -10; i < 10; i++) {
        for (int j = -10; j < 10; j++) {
            int dstPointX = scanBeginX + i * blockSize + 0.5 * blockSize;
            int dstPointY = scanBeginY + j * blockSize + 0.5 * blockSize;
            //超出范围不计
            if (dstPointX <= srcImage.cols && dstPointY <= srcImage.rows && dstPointX >= 0 && dstPointY >= 0) {

                circle(srcImage, Point(dstPointX, dstPointY), 6, Scalar(0, 0, 255), 8);
                int b = srcImage.at<Vec3b>(dstPointY, dstPointX)[0];
                int g = srcImage.at<Vec3b>(dstPointY, dstPointX)[1];
                int r = srcImage.at<Vec3b>(dstPointY, dstPointX)[2];
                if (abs(b - 204) < 5 && abs(g - 204) < 5 && abs(r - 204) < 5) {


                    circle(srcImage, Point(dstPointX, dstPointY), 6, Scalar(0, 255, 0), 8);

                    if (mapBeginX >= dstPointX) {
                        mapBeginX = dstPointX;
                    }
                    if (mapBeginY >= dstPointY) {
                        mapBeginY = dstPointY;
                    }
                    if (mapEndX <= dstPointX) {
                        mapEndX = dstPointX;
                    }
                    if (mapEndY <= dstPointY) {
                        mapEndY = dstPointY;
                    }
                }

            }
        }
    }



    //
    m = (mapEndX - mapBeginX) / blockSize;
    n = (mapEndY - mapBeginY) / blockSize;

    //背景图片的 bgr  57  41  38


    for (int i = 0; i <= m; i++) {
        for (int j = 0; j <= n; j++) {
            int dstPointX = mapBeginX + i * blockSize;
            int dstPointY = mapBeginY + j * blockSize;
            int b = srcImage.at<Vec3b>(dstPointY, dstPointX)[0];
            int g = srcImage.at<Vec3b>(dstPointY, dstPointX)[1];
            int r = srcImage.at<Vec3b>(dstPointY, dstPointX)[2];
            if (abs(b - 57) < 10 && abs(g - 41) < 10 && abs(r - 38) < 10) {
                //背景图片
                gameMap[j][i] = 0;
                continue;
            }
            if (abs(b - 204) < 10 && abs(g - 204) < 10 && abs(r - 204) < 10) {
                mapPosition[j][i][0] = dstPointY;
                mapPosition[j][i][1] = dstPointX;
                gameMap[j][i] = 1;
                blockCount++;
            } else {
                //起点
                circle(srcImage, Point(dstPointX, dstPointY), 6, Scalar(255, 0, 0), 8);
                mapPosition[j][i][0] = dstPointY;
                mapPosition[j][i][1] = dstPointX;
                gameMap[j][i] = 2;
                dfsBeginX = j;
                dfsBeginY = i;
            }
        }
    }



    //print Map
    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= m; j++) {
            cout << gameMap[i][j] << ",";
        }
        cout << endl;
    }
    imwrite("/Users/saltedfish/Desktop/debug1.png", srcImage);


}

int conformGameState(Mat srcImage, Mat result, Mat closeImage, Mat nextImage) {
    //绘画匹配区域,调试用
    int blockCount = 0;
    float threshold = 0.8;
    for (int i = 0; i < result.rows; i++) {
        for (int j = 0; j < result.cols; j++) {
            if (result.at<float>(i, j) > threshold) {
                rectangle(srcImage, Point(j, i), Point(j + blockSize, i + blockSize),
                          Scalar(0, 0, 255), 2, 8, 0);
                blockCount++;
            }
        }
    }

    if (blockCount > 2000) {
        return 1;//游戏地图状态
    } else {
        //下一关状态
        Mat nextResult;
        int nextResultCols = srcImage.cols - nextImage.cols + 1;
        int nextResultRows = srcImage.rows - nextImage.rows + 1;
        nextResult.create(nextResultCols, nextResultRows, srcImage.type());

        matchTemplate(srcImage, nextImage, nextResult, TM_CCOEFF_NORMED);
        int nextCount = 0;
        for (int i = 0; i < nextResult.rows; i++) {
            for (int j = 0; j < nextResult.cols; j++) {
                if (nextResult.at<float>(i, j) > threshold) {
                    rectangle(srcImage, Point(j, i), Point(j + blockSize, i + blockSize),
                              Scalar(0, 0, 255), 2, 8, 0);
                    nextCount++;
                }
            }
        }

        if (nextCount > 30) {
            return 2;
        } else {
//        大礼包要关闭状态
            Mat closeResult;
            int closeResultCols = srcImage.cols - closeImage.cols + 1;
            int closeResultRows = srcImage.rows - closeImage.rows + 1;
            closeResult.create(closeResultCols, closeResultRows, srcImage.type());


            matchTemplate(srcImage, closeImage, closeResult, TM_CCOEFF_NORMED);
            int closeCout;

            for (int i = 0; i < closeResult.rows; i++) {
                for (int j = 0; j < closeResult.cols; j++) {
                    if (closeResult.at<float>(i, j) > threshold) {
                        rectangle(srcImage, Point(j, i), Point(j + blockSize, i + blockSize),
                                  Scalar(0, 0, 255), 2, 8, 0);
                        closeCout++;
                    }
                }
            }
            if (closeCout > 10) {
                return 3;
            }

        }

    }


    return -1;


}
