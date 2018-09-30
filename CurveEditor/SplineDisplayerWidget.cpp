#include "SplineDisplayerWidget.h"
#include "libspline/aaCurve.h"

#if defined(__APPLE_CC__) || defined(_APPLE_CC)
#include <OpenGL/glu.h>
#else
#include <gl/glu.h>
#endif

#include <iostream>

#include <QPainter>

float const SplineDisplayerWidget::MY_PI = 3.1415926f;

    SplineDisplayerWidget::SplineDisplayerWidget(double deltaT, QWidget *parent /* = 0 */)
        : QGLWidget(parent), m_axis(0), lastPos(0, 0),
        ctrlSelected(-1), m_bIfModify(false), m_spline_data(0),
        m_deltaT(deltaT){

        m_CameraPos = aaAaa::Vector3(122, 122, 127);
        m_dScaleX = m_dScaleY = 1.0;

        setFocusPolicy(Qt::ClickFocus);
    }

    SplineDisplayerWidget::~SplineDisplayerWidget(void){

    }

    void SplineDisplayerWidget::setAE(aaAaa::aaSpline *sdata){
        m_spline_data = sdata;
        ctrlSelected = -1;
        updateGL();
        emit selectValuesChanged(0, 0);
    }


    //void SplineDisplayerWidget::paintEvent(QPaintEvent *event){
    //    QPainter painter(this);
    //    painter.setRenderHint(QPainter::Antialiasing, true);
    //    painter.setPen(QPen(Qt::black, 12, Qt::DashDotLine, Qt::RoundCap));
    //    painter.setBrush(QBrush(Qt::green, Qt::SolidPattern));
    //    painter.drawEllipse(80, 80, 400, 240);

    //}

    void SplineDisplayerWidget::drawGrid(void){
        double distance = m_CameraPos.z - Z_VALUE;
        //double halfwidth = distance;
        //double halfheight = halfwidth /** (double(this->width()) / double(this->height()))*/;

        if(distance > 300)
            distance = 80;
        else if(distance > 220)
            distance = 40;
        else if(distance > 80)
            distance = 20;
        else
            distance = 10;

        glColor3f(0, 0, 0);
        // draw axies
        glLineWidth(1.5);
        glBegin(GL_LINES);
        glVertex3f(-1000, 0, Z_VALUE);
        glVertex3f(1000, 0, Z_VALUE);
        
        glVertex3f(0, -1000, Z_VALUE);
        glVertex3f(0, 1000, Z_VALUE);
        glEnd();

        aaAaa::Vector2 lt = screen2gl(1, 0);
        aaAaa::Vector2 rb = screen2gl(m_width, m_height-1);

        double val = 0;
        glColor3f(0.5f, 0.5f, 0.5f);
        glLineWidth(0.01);
        glBegin(GL_LINES);
        val = distance * Y_FACTOR;
        while(val <= lt.y){
            glVertex3f(-1000, val, Z_VALUE);
            glVertex3f(1000, val, Z_VALUE);
            val += distance * Y_FACTOR;
            
        }
        val = -distance * Y_FACTOR;
        while(val >= rb.y){
            glVertex3f(-1000, val, Z_VALUE);
            glVertex3f(1000, val, Z_VALUE);
            val -= distance * Y_FACTOR;
        }

        val = distance;
        while(val <= rb.t){
            glVertex3f(val, -1000, Z_VALUE);
            glVertex3f(val, 1000, Z_VALUE);
            val += distance;
        }

        val = -distance;
        while(val >= lt.t){
            glVertex3f(val, -1000, Z_VALUE);
            glVertex3f(val, 1000, Z_VALUE);
            val -= distance;
        }
        glEnd();


        glColor3f(0.0f, 0.0f, 0.0f);
        

        _draw_text(lt.t, 0, Z_VALUE, "0");
        val = distance * Y_FACTOR;
        while(val <= lt.y){
            _draw_text(lt.t, val, Z_VALUE, QString::number(val / m_dScaleY, 'g', 2));
            //_draw_text(lt.t, val, Z_VALUE, QString("%1").arg(val / m_dScaleY, 3, 'g', 1, '0'));            
            val += distance * Y_FACTOR;
        }
        val = -distance * Y_FACTOR;
        while(val >= rb.y){
            _draw_text(lt.t, val, Z_VALUE, QString::number(val / m_dScaleY, 'g', 2));
            //_draw_text(lt.t, val, Z_VALUE, QString("%1").arg(val / m_dScaleY, 3, 'g', 1, '0'));
            val -= distance * Y_FACTOR;
        }

        _draw_text(0, rb.y, Z_VALUE, "0");
        val = distance;
        while(val <= rb.t){
            _draw_text(val, rb.y, Z_VALUE, QString::number(val / m_dScaleX, 'g', 2));
            val += distance;
        }

        val = -distance;
        while(val >= lt.t){
            _draw_text(val, rb.y, Z_VALUE, QString::number(val / m_dScaleX, 'g', 2));
            val -= distance;
        }        
        
    }


    void SplineDisplayerWidget::drawSpline(void){

        if(!m_spline_data)
            return ;
        aaAaa::aaSpline &m_spline_data = *(this->m_spline_data);

        aaAaa::aaCurvePtr pspline;
        if(m_spline_data.size() <= 1){
            return ;
        }else{
            pspline = aaAaa::aaCurveFactory::createCurve(m_spline_data);
            if(!pspline.get())
                return ;

        }

        if(m_spline_data.bLimited){
            glColor3f(1.0f, 0.0f, 0.0f);
            glBegin(GL_LINE_STRIP);
            glVertex3f(m_spline_data.limit_left, m_spline_data.limit_bottom * Y_FACTOR, Z_VALUE);
            glVertex3f(m_spline_data.limit_left, m_spline_data.limit_top * Y_FACTOR, Z_VALUE);
            glVertex3f(m_spline_data.limit_right, m_spline_data.limit_top * Y_FACTOR, Z_VALUE);
            glVertex3f(m_spline_data.limit_right, m_spline_data.limit_bottom * Y_FACTOR, Z_VALUE);
            glVertex3f(m_spline_data.limit_left, m_spline_data.limit_bottom * Y_FACTOR, Z_VALUE);
            glEnd();
        }


        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_POINTS);
        aaAaa::aaSpline::KnotsList::iterator cit = m_spline_data.knots.begin();
        aaAaa::aaSpline::KnotsList::iterator cend = m_spline_data.knots.end();
        for(; cit!=cend; ++cit){
            glVertex3f((*cit).t * m_dScaleX, (*cit).y * Y_FACTOR * m_dScaleY, Z_VALUE);
        }
        glEnd();

        if(ctrlSelected >= 0 && ctrlSelected < m_spline_data.knots.size()){
            glColor3f(1.0f, 1.0f, 0.0f);
            glBegin(GL_POINTS);
            glVertex3f(m_spline_data.knots[ctrlSelected].t * m_dScaleX, m_spline_data.knots[ctrlSelected].y * Y_FACTOR * m_dScaleY, Z_VALUE);
            glEnd();
        }

        glColor3f(1.0f, 0.0f, 1.0f);
        glBegin(GL_LINE_STRIP);

        aaAaa::aaSpline::KnotsList::iterator beg = m_spline_data.knots.begin();
        aaAaa::aaSpline::KnotsList::reverse_iterator rbeg = m_spline_data.knots.rbegin();

        double t = (*beg).t;
        double v = (*beg).y;
        glVertex3f(t * m_dScaleX, v * Y_FACTOR * m_dScaleY, Z_VALUE);
        t += m_deltaT;

        while(t < (*rbeg).t - m_deltaT){
            pspline->getValue(t, v);
            if(m_spline_data.bLimited){
                if(v > m_spline_data.limit_top)
                    v = m_spline_data.limit_top;
                else if(v < m_spline_data.limit_bottom)
                    v = m_spline_data.limit_bottom;
            }
            glVertex3f(t * m_dScaleX, v * Y_FACTOR * m_dScaleY, Z_VALUE);
            t += m_deltaT;
        }

        t = (*rbeg).t;
        v = (*rbeg).y;
        glVertex3f(t * m_dScaleX, v * Y_FACTOR * m_dScaleY, Z_VALUE);

        glEnd();


    }

    void SplineDisplayerWidget::initializeGL()
    {
        //resizeGL(this->geometry().width(), this->geometry().height());
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        glShadeModel(GL_FLAT);
        glEnable(GL_LINE_SMOOTH);
        //glEnable(GL_DEPTH_TEST);
        glLineWidth(1);
        glPointSize(7.0);
    }

    void SplineDisplayerWidget::paintGL()
    {
        glClear(GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT*/);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(m_CameraPos.x, m_CameraPos.y, m_CameraPos.z, m_CameraPos.x, m_CameraPos.y, m_CameraPos.z-10, 0, 1, 0);
        /*glTranslated(0.0, 0.0, -10.0);*/

    //    glViewport(20, 20, m_width-20, m_height-20);
        glViewport(0, 0, m_width, m_height);
        drawGrid();
        drawSpline();        
    }

    void SplineDisplayerWidget::resizeGL(int width, int height)
    {
        m_width = width; m_height = height;
        glViewport(0, -(width - height) / 2, width, width);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
    //    glOrtho(-0.5, +0.5, +0.5, -0.5, 4.0, 15.0);
    //    gluPerspective(90.0f, GLfloat(width)/GLfloat(height), CAMERA_D, 5000.0f);
        gluPerspective(90.0f, 1, 1, 5000.0f);
        glMatrixMode(GL_MODELVIEW);
    }

    aaAaa::Vector2 SplineDisplayerWidget::screen2gl(int x, int y){
        int hw = this->width() * 0.5f;
        int hh = this->height() * 0.5f;
        double oglx = double(x - hw) / double(hw);
        double ogly = -double(y - hh) / double(hh);
        double realx = oglx * (m_CameraPos.z - Z_VALUE);
        double realy = ogly * (m_CameraPos.z - Z_VALUE);

        return aaAaa::Vector2(realx + m_CameraPos.x, (realy + m_CameraPos.y) / Y_FACTOR);
    }

    int SplineDisplayerWidget::checkSelected(aaAaa::Vector2 point){
        if(!m_spline_data){
            return -1;
            emit selectValuesChanged(0, 0);
        }
        aaAaa::aaSpline &m_spline_data = *(this->m_spline_data);

        float factor = (m_CameraPos.z - Z_VALUE) * 0.1f * 0.5f;
        for(size_t i=0; i<m_spline_data.knots.size(); ++i){
            if(abs(point.t - m_spline_data.knots[i].t) < factor && abs(point.y - m_spline_data.knots[i].y) < factor ){
                emit selectValuesChanged(m_spline_data.knots[i].t, m_spline_data.knots[i].y);
                return i;
            }
        }
        
        emit selectValuesChanged(0, 0);
        return -1;
    }

    void SplineDisplayerWidget::mousePressEvent(QMouseEvent *event)
    {
        //if(event->buttons() & Qt::MidButton)
        lastPos = event->pos();
        
    //    if(ctrlSelected == -1){
        if(event->button() == Qt::LeftButton || event->button() == Qt::RightButton){
            aaAaa::Vector2 glpoint = screen2gl(event->x(), event->y());

            ctrlSelected = checkSelected(glpoint);

            emit selectValuesChanged(glpoint.t, glpoint.y);
//            std::cout << "glpoint: (" << glpoint.t << ", " << glpoint.y << "); ctrlSelected: " << ctrlSelected << "\n";
        }

        updateGL();
    }

    void SplineDisplayerWidget::mouseReleaseEvent(QMouseEvent *event){
        if(!m_spline_data)
            return ;
        aaAaa::aaSpline &m_spline_data = *(this->m_spline_data);

        if(event->button() == Qt::RightButton){
            aaAaa::aaSpline::KnotsList &ctrlpoints = m_spline_data.knots;
            if (ctrlSelected >= 0){
                if(ctrlSelected != 0 && ctrlSelected != ctrlpoints.size()-1){
                    ctrlpoints.erase(ctrlpoints.begin() + ctrlSelected);
                }

                aaAaa::Vector2 glpoint = screen2gl(event->x(), event->y());
                ctrlSelected = checkSelected(glpoint);

            }else{
                aaAaa::Vector2 p = screen2gl(event->x(), event->y());
                if(p.t > ctrlpoints[0].t &&
                    p.t < ctrlpoints[ctrlpoints.size()-1].t){

                    aaAaa::aaSpline::KnotsList::iterator it = ctrlpoints.begin();
                    ++it;
                    for(; it!=ctrlpoints.end(); ++it){
                        if(p.t < (*it).t)
                            break;
                    }

                    ctrlpoints.insert(it, p);

                    aaAaa::Vector2 glpoint = screen2gl(event->x(), event->y());
                    ctrlSelected = checkSelected(glpoint);
                }
            }

            updateGL();
        }
        //ctrlSelected = -1;
    }

    void SplineDisplayerWidget::mouseMoveEvent(QMouseEvent *event)
    {
        
        int dx = event->x() - lastPos.x();
        int dy = event->y() - lastPos.y();

        

        if (ctrlSelected >= 0 &&
            event->buttons() & Qt::LeftButton && m_spline_data){
                
                aaAaa::aaSpline &m_spline_data = *(this->m_spline_data);

                aaAaa::aaSpline::KnotsList &ctrlpoints = m_spline_data.knots;
                aaAaa::Vector2 npos = screen2gl(event->x(), event->y());
                ctrlpoints[ctrlSelected].y = npos.y;
                ctrlpoints[ctrlSelected].t = npos.t;
                
                if(ctrlSelected < ctrlpoints.size() - 1){
                    if(ctrlpoints[ctrlSelected].t >= ctrlpoints[ctrlSelected+1].t)
                        ctrlpoints[ctrlSelected].t = ctrlpoints[ctrlSelected+1].t - 1;
                }
                if(ctrlSelected > 0){
                    if(ctrlpoints[ctrlSelected].t <= ctrlpoints[ctrlSelected-1].t)
                        ctrlpoints[ctrlSelected].t = ctrlpoints[ctrlSelected-1].t + 1;
                }

                if(m_spline_data.bLimited){
                    if(ctrlpoints[ctrlSelected].t < m_spline_data.limit_left){
                        ctrlpoints[ctrlSelected].t = m_spline_data.limit_left;
                    }else if(ctrlpoints[ctrlSelected].t > m_spline_data.limit_right){
                        ctrlpoints[ctrlSelected].t = m_spline_data.limit_right;
                    }

                    if(ctrlpoints[ctrlSelected].y < m_spline_data.limit_bottom){
                        ctrlpoints[ctrlSelected].y = m_spline_data.limit_bottom;
                    }else if(ctrlpoints[ctrlSelected].y > m_spline_data.limit_top){
                        ctrlpoints[ctrlSelected].y = m_spline_data.limit_top;
                    }
                }

                emit selectValuesChanged(ctrlpoints[ctrlSelected].t, ctrlpoints[ctrlSelected].y);

                m_bIfModify = true;
        }else if(event->buttons() & Qt::MidButton){
            m_CameraPos.x = (m_CameraPos.x - dx*0.3f);
            m_CameraPos.y = (m_CameraPos.y + dy*0.3f);    
        }
        lastPos = event->pos();
        updateGL();
    }

    void SplineDisplayerWidget::wheelEvent(QWheelEvent *e)
    {
        if(e->delta() > 0)
            m_CameraPos.z -= 3;
        else if(e->delta() < 0)
            m_CameraPos.z += 3;
        //m_CameraPos.setZ(m_CameraPos.z() + e->delta()*0.3);
        if(m_CameraPos.z < 0)
            m_CameraPos.z = 0;

 //       emit selectValuesChanged(m_CameraPos.z, m_CameraPos.z - Z_VALUE);
        updateGL();
    }

    void SplineDisplayerWidget::setCurrentSelected(int index){
        if(!m_spline_data)
            return ;
        if(index >= 0 && index < m_spline_data->knots.size()){
            ctrlSelected = index;
            emit selectValuesChanged(m_spline_data->knots[ctrlSelected].t, m_spline_data->knots[ctrlSelected].y);
        }
        updateGL();
    }

    void SplineDisplayerWidget::_draw_text(double x, double y, double z, QString txt)
    {        
        //glDisable(GL_LIGHTING);
        //glDisable(GL_DEPTH_TEST);
        //qglColor(Qt::white);
        renderText(x, y, z, txt);
        //renderText(x, y, z, txt, QFont("Arial", 12, QFont::Bold, false));
        //glEnable(GL_DEPTH_TEST);
        //glEnable(GL_LIGHTING);
    }

