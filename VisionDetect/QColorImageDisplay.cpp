#include "QColorImageDisplay.h"

#define ToInt(value)                (static_cast<int>(value))

const int LABEL_IMAGE_WIDTH = 400;
const int LABEL_IMAGE_HEIGHT = 400;

QColorImageDisplay::QColorImageDisplay(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_windowWidth = LABEL_IMAGE_WIDTH;
	m_windowHeight = LABEL_IMAGE_HEIGHT;

	m_imageHeight = 1;
	m_imageWidth = 1;
}

QColorImageDisplay::~QColorImageDisplay()
{
}

void QColorImageDisplay::getSize(int& nSizeX, int& nSizeY)
{
	nSizeX = LABEL_IMAGE_WIDTH;
	nSizeY = LABEL_IMAGE_HEIGHT;
}

void QColorImageDisplay::setImage(cv::Mat& matImage)
{
	m_hoImage = matImage.clone();
	m_imageWidth = m_hoImage.size().width;
	m_imageHeight = m_hoImage.size().height;

	displayImage(m_hoImage);
}

void QColorImageDisplay::A_Transform(Mat& src, Mat& dst, int dx, int dy)
{
	CV_Assert(src.depth() == CV_8U);//CV_Assert�����������еı��ʽֵΪfalse���򷵻�һ��������Ϣ��  
	const int rows = src.rows;
	const int cols = src.cols;
	dst.create(rows, cols, src.type());

	//dst.row(i).setTo(Scalar(255));
	//dst.col(j).setTo(Scalar(255));

	dst.setTo(Scalar(0, 0, 0));

	Vec3b *p;   //����һ�����3ͨ��������ָ��p  
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			p = dst.ptr<Vec3b>(i);//ָ������������p  
			int x = j - dx;
			int y = i - dy;
			if (x>0 && y>0 && x < cols&&y < rows)//ƽ�ƺ������������ԭͼ���������������  
			{
				p[i, j] = src.ptr<Vec3b>(y)[x];//ƽ�ƺ��ͼ��i,j)��Ӧ��ԭͼ��ģ�y,x)  
			}
		}
	}

}

void QColorImageDisplay::displayImage(cv::Mat& image)
{
	QRect rect = ui.label_Img->geometry();
	rect.setX(0); rect.setY(0);
	rect.setWidth(LABEL_IMAGE_WIDTH); rect.setHeight(LABEL_IMAGE_HEIGHT);

	cv::Mat mat;
	double fScaleW = rect.width()*1.0 / image.size().width;
	double fScaleH = rect.height()*1.0 / image.size().height;
	if (!image.empty())
	{
		cv::resize(image, mat, cv::Size(image.size().width*fScaleW, image.size().height*fScaleH), (0, 0), (0, 0), 3);

		//if (mat.type() == CV_8UC3)
		//{
		//	cvtColor(mat, mat, CV_BGR2RGB);
		//}
		//else if (mat.type() == CV_8UC1)
		//{
		//	cvtColor(mat, mat, CV_GRAY2RGB);
		//}

		//cv::Mat matMoved;
		//A_Transform(mat, matMoved, rect.x(), rect.y());

		QImage image = QImage((uchar*)mat.data, mat.cols, mat.rows, ToInt(mat.step), QImage::Format_RGB888);
		ui.label_Img->setPixmap(QPixmap::fromImage(image));
	}
}
