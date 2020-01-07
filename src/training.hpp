
#ifndef TRAINING_HPP
#define TRAINING_HPP

#include <QObject>
#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>
#include <QFileDialog>
#include <QTime>
#include <QDebug>
#include <QImage>
#include <QString>
#include <QObject>

#include <opencv2/opencv.hpp>

#include <iostream>
#include <fstream>
#include <ostream>
#include <string>

using namespace std;
using namespace cv;
using namespace cv::ml;

static string trainImagesWin = "C:\\Users\\xuhua\\GitHub\\OpenCV-MNIST\\data\\train-images.idx3-ubyte";
static string trainImageUnix = "/home/xuhua/GitHub/OpenCV-MNIST/data/train-images.idx3-ubyte";
static string trainLabelsWin = "C:\\Users\\xuhua\\GitHub\\OpenCV-MNIST\\data\\train-labels.idx3-ubyte";
static string trainLabelsUnix = "/home/jackey/GitHub/OpenCV-MNIST/data/train-labels.idx1-ubyte";
static string testImagesWin = "C:\\Users\\xuhua\\GitHub\\OpenCV-MNIST\\data\\t10k-images.idx3-ubyte";
static string testImagesUnix = "/home/xuhua/GitHub/OpenCV-MNIST/data/t10k-images.idx3-ubyte";
static string testLabelsWin = "C:\\Users\\xuhua\\GitHub\\OpenCV-MNIST\\data\\t10k-labels.idx3-ubyte";
static string testLabelsUnix = "/home/jackey/GitHub/OpenCV-MNIST/data/t10k-labels.idx1-ubyte";
static string knnSaveWin = "C:\\Users\\xuhua\\GitHub\\OpenCV-MNIST\\data\\knn.yml";
static string knnSaveUnix = "/home/jackey/GitHub/OpenCV-MNIST/data/knn.yml";
static string svmSaveWin = "C:\\Users\\xuhua\\GitHub\\OpenCV-MNIST\\data\\svm.xml";
static string svmSaveUnix = "/home/jackey/GitHub/OpenCV-MNIST/data/svm.xml";

class Training
{
public:
    Training(){}
    //读取图像数据集
    Mat readImages(int opt)
    {
        ifstream f;
        Mat img;
        if(opt == 0){
            qDebug() << "读取训练图像数据中 ...";
#ifdef Q_OS_WIN
        f.open("train-images.idx3-ubyte",ios::binary);
#endif
#ifdef Q_OS_UNIX
        f.open(trainImageUnix,ios::binary);
#endif

        }
        else{
            qDebug() << "读取测试图像数据中 ...";
#ifdef Q_OS_WIN
        f.open("t10k-images.idx3-ubyte",ios::binary);
#endif
#ifdef Q_OS_UNIX
        f.open(testImagesUnix,ios::binary);
#endif
        }
        // check file
        if (!f.is_open()){
            qDebug() << "未找到图像数据!";
            return img;
        }
        /*
         byte 0 - 3 : Magic Number(Not to be used)
         byte 4 - 7 : Total number of images in the dataset
         byte 8 - 11 : rows of each image in the dataset
         byte 12 - 15 : cols of each image in the dataset
        */
        int magic_number = 0;
        int number_of_images = 0;
        int height = 0;
        int width = 0;

        f.read((char*)&magic_number, sizeof(magic_number));

        magic_number = reverseDigit(magic_number);
        f.read((char*)&number_of_images, sizeof(number_of_images));

        number_of_images = reverseDigit(number_of_images);
        qDebug()<<"图像数据量为: "<<number_of_images;

        f.read((char*)&height, sizeof(height));
        height = reverseDigit(height);

        f.read((char*)&width, sizeof(width));
        width = reverseDigit(width);

        Mat train_images = Mat(number_of_images, height*width, CV_8UC1);

        Mat digitImg = Mat::zeros(height, width, CV_8UC1);

        for (int i = 0; i < number_of_images; i++) {
            int index = 0;
            for (int r = 0; r<height; ++r) {
               for (int c = 0; c<width; ++c) {
                   unsigned char temp = 0;
                   f.read((char*)&temp, sizeof(temp));
                   index = r*width + c;
                   train_images.at<uchar>(i, index) = (int)temp;
                   digitImg.at<uchar>(r, c) = (int)temp;
               }
           }
        //       if (i < 100) {
        //           qDebug()<<"Reading image: "<<i<<" done";
        //            imwrite(format("/home/jackey/GitHub/OpenCV-MNIST/data/images/digit_%d.png", i), digitImg);
        //       }
        }
        train_images.convertTo(train_images, CV_32F);

        f.close();
        return train_images;
    }
    //读取标记数据集
    Mat readLabels(int opt)
    {
        ifstream f;
        Mat img;
        if(opt == 0){
            qDebug() << "读取训练标签数据中 ...";
#ifdef Q_OS_WIN
        f.open("train-labels.idx1-ubyte");
#endif
#ifdef Q_OS_UNIX
        f.open(trainLabelsUnix);
#endif
        }
        else{
            qDebug() << "读取测试标签数据中 ...";
#ifdef Q_OS_WIN
        f.open("t10k-labels.idx1-ubyte");
#endif
#ifdef Q_OS_UNIX
        f.open(testLabelsUnix);
#endif
        }
         // check file
        if (!f.is_open()){
            qDebug() << "未找到标签数据文件!";
            return img;
        }
        /*
         byte 0 - 3 : Magic Number(Not to be used)
         byte 4 - 7 : Total number of labels in the dataset
        */
        int magic_number = 0;
        int number_of_labels = 0;
        f.read((char*)&magic_number, sizeof(magic_number));
        magic_number = reverseDigit(magic_number);

        f.read((char*)&number_of_labels, sizeof(number_of_labels));
        number_of_labels = reverseDigit(number_of_labels);
        qDebug() << "标签数据量为: " << number_of_labels ;

        Mat labels = Mat(number_of_labels, 1, CV_8UC1);
        for (long int i = 0; i<number_of_labels; ++i){
            unsigned char temp = 0;
            f.read((char*)&temp, sizeof(temp));
            //printf("temp : %d\n ", temp);
            labels.at<uchar>(i, 0) = temp;
        }
        labels.convertTo(labels, CV_32S);

        f.close();
        return labels;
    }
    /*我们不使用提取特征方式，而是采用纯像素数据作为输入，分别使用KNN与SVM对数据集进行训练与测试，比较他们的正确率
      KNN是最简单的机器学习方法、主要是计算目标与模型之间的空间向量距离得到最终预测分类结果。
    */
    bool knnTrain()
    {
        qDebug()<<"KNN训练开始 ...";
        Mat train_images = readImages(0);
        if(train_images.size==0) return false;
        Mat train_labels = readLabels(0);
        if(train_labels.size ==0) return false;
        qDebug()<<"已成功读取MNIST数据集 ...";

        Ptr<ml::KNearest> knn = ml::KNearest::create();
        Ptr<ml::TrainData> tdata = ml::TrainData::create(train_images,ml::ROW_SAMPLE,train_labels);
        knn->train(tdata);
        knn->setDefaultK(5);
        knn->setIsClassifier(true);

#ifdef Q_OS_WIN
        knn->save("knn.xml");
#endif
#ifdef Q_OS_UNIX
        knn->save("knn.xml");
#endif
        qDebug()<<"KNN训练数据已成功保存";

        return true;
    }

    bool svmTrain()
    {
        qDebug()<<"SVM训练开始 ...";
        Mat train_images = readImages(0);
        if(train_images.size ==0) return false;
        Mat train_labels = readLabels(0);
        if(train_labels.size ==0) return false;
        qDebug()<<"已成功读取MNIST数据集 ...";

        Ptr<ml::SVM> svm = ml::SVM::create();
        svm->setType(SVM::C_SVC);
        svm->setKernel(SVM::LINEAR);
        svm->setDegree(5);
        Ptr<ml::TrainData> tdata = ml::TrainData::create(train_images,ml::ROW_SAMPLE,train_labels);
        svm->train(tdata);
#ifdef Q_OS_WIN
        svm->save("svm.xml");
#endif
#ifdef Q_OS_UNIX
        svm->save(svmSaveUnix);
#endif
        qDebug()<<"SVM训练数据已保存";
        return true;
    }

    float testMnistKNN()
    {
        QFile file(QString::fromStdString("knn.xml"));
        if(!file.exists()){
            qDebug()<<"未找到KNN训练结果\n退出中 ...";
            return 0;
        }
        qDebug()<<"开始KNN测试";
#ifdef Q_OS_WIN
        Ptr<ml::KNearest> knn = Algorithm::load<ml::KNearest>("knn.xml"); // KNN - 97%
#endif
#ifdef Q_OS_UNIX
        Ptr<ml::KNearest> knn = Algorithm::load<ml::KNearest>(knnSaveUnix);
#endif

        Mat train_images = readImages(1);
        Mat train_labels = readLabels(1);
        qDebug()<<"成功读取MNIST测试数据...";

        float total = train_images.rows;

        float correct = 0;
        Rect rect;
        rect.x = 0;
        rect.height = 1;
        rect.width = (28 * 28);
        for(int i = 0; i < total; i++){
            int actual = train_labels.at<int>(i);
            rect.y = i;
            Mat oneImage = train_images(rect);
            Mat result;
            float predicted = knn->predict(oneImage, result);
            int digit = static_cast<int>(predicted);
            if(digit == actual){
                correct++;
            }
        }

        qDebug()<<"已成功识别 : "<<correct;
        float rate = correct /total *100.0;
        qDebug()<<"识别准确率为: "<<rate;

        return rate;
    }

    float testMnistSVM()
    {
        QFile file(QString::fromStdString("svm.xml"));
        if(!file.exists()){
            qDebug()<<"未找到SVM训练数据\n退出";
            return 0.0;
        }
        qDebug() << "开始导入SVM文件...";
#ifdef Q_OS_WIN
        Ptr<SVM> svm1 = StatModel::load<SVM>("svm.xml");
#endif
#ifdef Q_OS_UNIX
        Ptr<SVM> svm1 = StatModel::load<SVM>(svmSaveUnix);
#endif

        qDebug()<< "成功导入SVM文件...";

        qDebug()<< "开始导入测试数据...\n";
        Mat testData = readImages(1);
        Mat tLabel = readLabels(1);
        qDebug()<< "成功导入测试数据！！！";

        float count = 0;
        for (int i = 0; i < testData.rows; i++) {
            Mat sample = testData.row(i);
            float res = svm1->predict(sample);
            res = std::abs(res - tLabel.at<unsigned int>(i, 0)) <= FLT_EPSILON ? 1.f : 0.f;
            count += res;
        }
        qDebug() << "正确的识别个数 count = " << count;
        float rate = (10000-count +0.0)/10000*100.0;
        qDebug() << "准确率为：" << rate<< "%....";

        return rate;
    }

    int reverseDigit(int num)
    {
        unsigned char c1,c2,c3,c4;
        c1=num&255;
        c2=(num>>8)&255;
        c3=(num>>16)&255;
        c4=(num>>24)&255;

        return ((int)c1<<24)+((int)c2<<16)+((int)c3<<8)+c4;
    }
};

#endif // TRAINING_HPP
