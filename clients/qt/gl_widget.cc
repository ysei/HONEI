/* vim: set sw=4 sts=4 et nofoldenable : */

/*
 * Copyright (c) 2009 Markus Geveler <apryde@gmx.de>
 * Copyright (c) 2009 Dirk Ribbrock <dirk.ribbrock@uni-dortmund.de>
 *
 * This file is part of HONEI. HONEI is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * HONEI is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <QtGui>
#include <QtOpenGL>

#include <cmath>

#include <clients/qt/gl_widget.hh>

// switch on the following if you want the scene to be drawn in wireframe
#undef WIREFRAME
#define ANTIALIAS

static const float SphereColor[3] = { 0.2f, 0.1f, 0.9f };
const float PIf = 3.14159265358979323846f;
const unsigned int NumSphereSlices = 30;
const float RotSpeed = 0.1f;
const unsigned int RotTimer = 0;	// 0 = workproc, i.e., when there are no more UI events in the queue


    GLWidget::GLWidget(QWidget * father )
: QGLWidget( QGLFormat(QGL::SampleBuffers), father),			// enables multi-sampling
      m_object(0), m_xRot(0), m_yRot(0), m_zRot(0),
      m_xTrans(0.0), m_yTrans(0.0), m_zTrans(-5.0),
      m_curr_rot(0.0), m_numFlakeRec(2)
      //m_timer()
{
    if ( ! format().sampleBuffers() )
        fprintf(stderr, "Could not get sample buffer; no polygon anti-aliasing!");

    m_backgroundcolor[0] = 0.2f;  m_backgroundcolor[1] = 1.0f;  m_backgroundcolor[2] = 0.4f;
}


GLWidget::~GLWidget()
{
    makeCurrent();
    glDeleteLists(m_object, 1);
}


QSize GLWidget::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const
{
    return QSize(400, 400);
}

void GLWidget::initializeGL()
{
    // query OpenGL
    const GLubyte * strVersion = glGetString( GL_VERSION );
    printf("\nGL version = %s\n", strVersion );
    const GLubyte * strExt = glGetString( GL_EXTENSIONS );
    fputs("GL extensions: ", stdout);
    puts( reinterpret_cast<const char *>(strExt) );
    GLint multisamplebufs;
    glGetIntegerv( GL_SAMPLE_BUFFERS, &multisamplebufs );
    GLint multisamples;
    glGetIntegerv( GL_SAMPLES, &multisamples );
    printf("multisampling = %d\nnum samples = %d\n",
            static_cast<int>(multisamplebufs), static_cast<int>(multisamples) );

    //Insert HONEI gl initialization here
#ifdef ANTIALIAS
#ifdef WIREFRAME
    // anti-aliasing of lines
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
#else /* no WIREFRAME */
    // anti-aliasing of polygons and lines
    glEnable( GL_MULTISAMPLE );
#endif
#endif

    // create object
    // now we can start the timer for the animation function timerEvent()
    startTimer(RotTimer);
}

void GLWidget::paintGL()
{
    glClearColor(0., 0., 0., 0.);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //render HONEI solver output here
    glTranslatef(-1.5f,0.0f,-6.0f);
    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 1.0, 0.0);
    glVertex3f(0.f, 1.f, 0.f);
    glVertex3f(-1.f, -1.f, 0.f);
    glVertex3f(1.f, -1.f, 0.f);
    glEnd();
}

void GLWidget::resizeGL( int w, int h )		// = width & height
{
    int side = qMin(w, h);
    glViewport((w - side) / 2, (h - side) / 2, side, side);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-1.5, +1.5, +1.5, -1.5, 1.0, 150.0);
    glFrustum(-1.0, +1.0, -1.0, +1.0, 1.0, 150.0);
}
