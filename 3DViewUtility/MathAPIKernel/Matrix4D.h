#ifndef MATRIX4D_H
#define MATRIX4D_H


/*
ʹ��˵����
      ���о���Ĳ������������������Ҳ��������Ϊ��
      Vx  Vy  Vz  P
      x1  x2  x3 x4
      y1  y2   y3 y4
      z1  z2  z3  z4
      0   0    0    1
*/
#include<iostream>

class Vector3D;
class Position3D;
using namespace std;
class  Matrix4D
{
public:
    Matrix4D();
    explicit Matrix4D(double *values);  //һ��һ������
    Matrix4D(const Matrix4D &matrix);
    Matrix4D(const Vector3D &column1,const Vector3D &column2, const Vector3D &column3, const Position3D &pos); //3����������1��λ������

public:
    void setToIndentity();  //����Ϊ��λ����

    Vector3D column(int index);
    void setColumn(int index,const Vector3D &value);

    Vector3D row(int index);
    void setRow(int index,const Vector3D &value);

    double *Datas(); //����Ϊ��������

    Matrix4D& operator+=(const Matrix4D& other);
    Matrix4D& operator-=(const Matrix4D& other);
    Matrix4D& operator*=(const Matrix4D& other);
    Matrix4D& operator*=(float factor);
    Matrix4D& operator/=(float divisor);
    bool operator==(const Matrix4D& other) const;
    bool operator!=(const Matrix4D& other) const;

    friend Matrix4D operator+(const Matrix4D& m1, const Matrix4D& m2);
    friend Matrix4D operator-(const Matrix4D& m1, const Matrix4D& m2);
    friend Matrix4D operator*(const Matrix4D& m1, const Matrix4D& m2);

    friend Vector3D operator*(const Matrix4D& matrix, const Vector3D& vector);    //��Ϊ������Ϊ��,���Ա���matrix*vector,����Ҳ��.
    friend Position3D operator*(const Matrix4D& matrix, const Position3D& position);

    friend ostream& operator <<(ostream &os,const Matrix4D &matrix);//����<<

    static  Matrix4D getTranslateMatrix(float x, float y, float z);//��ȡƽ�ƾ���
    static  Matrix4D getTranslateMatrix(const Vector3D &vec);

    static  Matrix4D getRotateMatrix(float angle,float x,float y,float z);//��ȡ��ת����,angle�ǽǶȲ��ǻ���
    static  Matrix4D getRotateMatrix(float angle, const Vector3D &vector);

private:
    double m[4][4];
    int flagBits;
    enum {
        Identity        = 0x0001,   // ��λ����
        General         = 0x0002,   // һ�����
        Translation     = 0x0004,   // ƽ�ƾ���
        Scale           = 0x0008,   //���ž���
        Rotation        = 0x0010    // ��ת����
    };
};

#endif // MATRIX4D_H
