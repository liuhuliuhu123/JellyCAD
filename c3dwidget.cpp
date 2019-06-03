/* ====================================================
#   Copyright (C)2019 Li Jianbin All rights reserved.
#
#   Author        : Li Jianbin
#   Email         : lijianbinmail@163.com
#   File Name     : c3dwidget.cpp
#   Last Modified : 2019-04-21 15:00
#   Describe      : 3D Widget
#
# ====================================================*/

#include "c3dwidget.h"
#include "makebottle.h"
C3DWidget::C3DWidget(QWidget *parent) : QGLWidget(parent)
{
    //配置QWidget
    setBackgroundRole( QPalette::NoRole );  //无背景
    setMouseTracking( true );   //开启鼠标位置追踪

    // 创建一个立方体作测试
//    TopoDS_Shape t_topo_bottle = MakeBottle(70.0, 50.0, 30.0);
//    Handle(AIS_Shape) t_ais_bottle = new AIS_Shape(t_topo_bottle);
//    m_context->Display(t_ais_bottle, Standard_True);
//    m_view->FitAll();
}

void C3DWidget::make_cube(Standard_Real _dx, Standard_Real _dy, Standard_Real _dz)
{
    TopoDS_Shape t_topo_box = BRepPrimAPI_MakeBox(_dx, _dy, _dz).Shape();
    Handle(AIS_Shape) t_ais_box = new AIS_Shape(t_topo_box);
    m_context->Display(t_ais_box, Standard_True);
    m_view->FitAll();
}

void C3DWidget::make_cylinder(Standard_Real _R, Standard_Real _H)
{
    TopoDS_Shape t_topo_cylinder = BRepPrimAPI_MakeCylinder(_R , _H).Shape();
    Handle(AIS_Shape) t_ais_cylinder = new AIS_Shape(t_topo_cylinder);
    m_context->Display(t_ais_cylinder, Standard_True);
    m_view->FitAll();
}

void C3DWidget::make_sphere(Standard_Real _R)
{
    TopoDS_Shape t_topo_sphere = BRepPrimAPI_MakeSphere(_R).Shape();
    Handle(AIS_Shape) t_ais_sphere = new AIS_Shape(t_topo_sphere);
    m_context->Display(t_ais_sphere, Standard_True);
    m_view->FitAll();
}

void C3DWidget::make_cone(Standard_Real _R1, Standard_Real _R2, Standard_Real _H)
{
    TopoDS_Shape t_topo_cone = BRepPrimAPI_MakeCone(_R1,_R2,_H).Shape();
    Handle(AIS_Shape) t_ais_cone = new AIS_Shape(t_topo_cone);
    m_context->Display(t_ais_cone, Standard_True);
    m_view->FitAll();
}

void C3DWidget::make_torus(Standard_Real _R1, Standard_Real _R2)
{
    TopoDS_Shape t_topo_torus = BRepPrimAPI_MakeTorus(_R1 ,_R2).Shape();
    Handle(AIS_Shape) t_ais_torus = new AIS_Shape(t_topo_torus);
    m_context->Display(t_ais_torus, Standard_True);
    m_view->FitAll();
}

void C3DWidget::m_initialize_context()
{
    //若交互式上下文为空，则创建对象
    if (m_context.IsNull())
    {
        //此对象提供与X server的连接，在Windows和Mac OS中不起作用
        Handle(Aspect_DisplayConnection) m_display_donnection = new Aspect_DisplayConnection();
        //创建OpenGl图形驱动
        if (m_graphic_driver.IsNull())
        {
            m_graphic_driver = new OpenGl_GraphicDriver(m_display_donnection);
        }
        //获取QWidget的窗口系统标识符
        WId window_handle = (WId) winId();
#ifdef _WIN32
        // 创建Windows NT 窗口
        Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle) window_handle);
#else
        // 创建XLib window 窗口
        Handle(Xw_Window) wind = new Xw_Window(m_display_donnection, (Window) window_handle);
#endif
        //创建3D查看器
        m_viewer = new V3d_Viewer(m_graphic_driver);
        //创建视图
        m_view = m_viewer->CreateView();
        m_view->SetWindow(wind);
        //打开窗口
        if (!wind->IsMapped())
        {
            wind->Map();
        }
        m_context = new AIS_InteractiveContext(m_viewer);  //创建交互式上下文
        //配置查看器的光照
        m_viewer->SetDefaultLights();
        m_viewer->SetLightOn();
        //设置视图的背景颜色为灰色
        m_view->SetBackgroundColor(Quantity_NOC_GRAY60);
        m_view->MustBeResized();
        //显示直角坐标系，可以配置在窗口显示位置、文字颜色、大小、样式
        m_view->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_ZBUFFER);
        //设置显示模式
        m_context->SetDisplayMode(AIS_Shaded, Standard_True);
    }
}

void C3DWidget::paintEvent(QPaintEvent *)
{
    if (m_context.IsNull()) // 若未定义交互环境
    {
        m_initialize_context(); // 初始化交互环境
    }
    m_view->Redraw();
}

void C3DWidget::resizeEvent(QResizeEvent *)
{
    if( !m_view.IsNull() )
    {
        m_view->MustBeResized();
    }
}

void C3DWidget::mousePressEvent(QMouseEvent *event)
{
    if( ((event->buttons() & Qt::MidButton) && (QApplication::keyboardModifiers()==Qt::ShiftModifier) )  //平移方式1
        ||((event->buttons()&Qt::LeftButton)&&(event->buttons()&Qt::RightButton)))//平移方式2
    {
        m_current_mode = CurAction3d_DynamicPanning;
        m_x_max = event->pos().x(); //记录起始X位置
        m_y_max = event->pos().y(); //记录起始Y位置
    }
    else if(event->buttons() & Qt::MidButton)  //旋转
    {
        m_current_mode = CurAction3d_DynamicRotation;
        m_view->StartRotation(event->pos().x(), event->pos().y());
    }
    else if(event->buttons() & Qt::LeftButton)  //鼠标左键选择模型
    {
        // 按下Shift键点击鼠标左键实现多选
        if(qApp->keyboardModifiers() == Qt::ShiftModifier)
        {
            m_context->ShiftSelect(true);
        }
        else
        {
            m_context->Select(true);    // 单选模型
        }
    }
    else
    {
        m_current_mode = CurAction3d_Nothing;
    }
}

void C3DWidget::mouseReleaseEvent(QMouseEvent *)
{
    m_current_mode = CurAction3d_Nothing;
}

void C3DWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 鼠标移动到模型时，模型高亮显示
    m_context->MoveTo(event->x(),event->y(),m_view,true);

    switch (m_current_mode)
    {
    case CurAction3d_DynamicPanning:
        //执行平移
        m_view->Pan(event->pos().x() - m_x_max, m_y_max - event->pos().y());
        m_x_max = event->pos().x();
        m_y_max = event->pos().y();
        break;
    case CurAction3d_DynamicRotation:
        //执行旋转
        m_view->Rotation(event->pos().x(), event->pos().y());
        break;
    default:
        break;
    }
}

void C3DWidget::wheelEvent(QWheelEvent *event)
{
    m_view->StartZoomAtPoint(event->pos().x(),event->pos().y());
    m_view->ZoomAtPoint(0, 0, event->angleDelta().y(), 0); //执行缩放
}
