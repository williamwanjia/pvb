/***************************************************************************
                          pvtcl.cpp  -  description
                             -------------------
    begin                : Mon Jun 10 2002
    copyright            : (C) 2002 by R. Lehrig
    email                : lehrig@t-online.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pvdefine.h"

#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)

#include "pvVtkTclWidget.h"
#include "opt.h"
#include <QMouseEvent>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include "tcputil.h"
#include <stdarg.h>

#include "vtkAutoInit.h"
VTK_MODULE_INIT(vtkRenderingOpenGL2); // VTK was built with vtkRenderingOpenGL2

extern OPT opt;

static int rlvsnprintf(char *text, int len, const char *format, va_list ap)
{
  int ret;

#ifdef PVWIN32
  if(format == NULL || format[0] == '\0')
  {
    text[0] = '\0';
    return 1;
  }
  ret = _vsnprintf(text, len, format, ap);
#endif
#ifdef __VMS
  static char vms_is_deprecated[rl_PRINTF_LENGTH];
  if(format == NULL || format[0] == '\0')
  {
    text[0] = '\0';
    return 1;
  }
  ret = vsprintf(vms_is_deprecated, format, ap);
  rlstrncpy(text,vms_is_deprecated,len);
#endif
#ifdef PVUNIX
  if(format == NULL || format[0] == '\0')
  {
    text[0] = '\0';
    return 1;
  }
  ret = vsnprintf(text, len, format, ap);
#endif
  return ret;
}

pvVtkTclWidget::pvVtkTclWidget(QWidget *parent, const char *name, int Id, int *sock) 
               : QVTKOpenGLWidget(parent)
{
  
  int error;
  char buf[80];
  
  vtkGenericOpenGLRenderWindow *temp0;
  
  tclcommand = NULL; 
  interp = NULL;
  if(name != NULL) setObjectName(name);

  Tcl_FindExecutable(NULL);

  interp = Tcl_CreateInterp();
  if(interp == NULL) return;
  
#ifdef PVUNIX
  tclcommand = vtkTclCommand::New();
#endif
#ifdef PVWIN32
  //tclcommand = newVtkTclCommand();
  tclcommand = vtkTclCommand::New();
  //tclcommand = new vtkTclCommand();
#endif
  
  if(tclcommand == NULL)
  {
    printf("ERROR: pvVtkTclWidget::tclcommand == NULL\n");
    Tcl_DeleteInterp(interp);
    return;
  }
  
  tclcommand->SetInterp(interp);
  if(opt.arg_debug) printf("SetInterp finished\n");
  
#ifdef PVUNIX
  Tcl_AppInit(interp);
#endif
#ifdef PVWIN32
  const char* nameofexec = Tcl_GetNameOfExecutable();
  if(opt.arg_debug) printf("nameofexec %s\n", nameofexec);
  if(opt.arg_debug) printf("TCL_VERSION %s\n", TCL_VERSION);
  
  Tcl_AppInit(interp);
  if(opt.arg_debug) printf("Tcl_AppInit finished\n");
#endif
  if(opt.arg_debug) printf("package require vtk\n");
  interpret("package require vtk");
  if(opt.arg_debug) printf("package require vtkinteraction\n");
  interpret("package require vtkinteraction");
  if(opt.arg_debug) printf("package require vtktesting\n");
  interpret("package require vtktesting");

  if(opt.arg_debug) printf("vtkGenericOpenGLRenderWindow renWin\n");
  interpret("vtkGenericOpenGLRenderWindow renWin");
  sprintf(buf,"renWin SetSize %d %d",width(),height());
  printf("cmd: %s", buf);
  interpret(buf);

  // create sandbox
  if(Tcl_DeleteCommand(interp,"eof")       == -1) printf("could not delete Tcl eof\n");
  if(Tcl_DeleteCommand(interp,"fblocked")  == -1) printf("could not delete Tcl fblocked\n");
  if(Tcl_DeleteCommand(interp,"gets")      == -1) printf("could not delete Tcl gets\n");
  if(Tcl_DeleteCommand(interp,"fcopy")     == -1) printf("could not delete Tcl fcopy\n");
  if(Tcl_DeleteCommand(interp,"close")     == -1) printf("could not delete Tcl close\n");
  if(Tcl_DeleteCommand(interp,"read")      == -1) printf("could not delete Tcl read\n");
  if(Tcl_DeleteCommand(interp,"seek")      == -1) printf("could not delete Tcl seek\n");
  if(Tcl_DeleteCommand(interp,"flush")     == -1) printf("could not delete Tcl flush\n");
  if(Tcl_DeleteCommand(interp,"fileevent") == -1) printf("could not delete Tcl fileevent\n");
  if(Tcl_DeleteCommand(interp,"puts")      == -1) printf("could not delete Tcl puts\n");
  if(Tcl_DeleteCommand(interp,"tell")      == -1) printf("could not delete Tcl tell\n");
  if(Tcl_DeleteCommand(interp,"pid")       == -1) printf("could not delete Tcl pid\n");

  error = 0;
  temp0 = (vtkGenericOpenGLRenderWindow *)(vtkTclGetPointerFromObject("renWin",(char *) "vtkGenericOpenGLRenderWindow",interp,error));
  if(!error && temp0 != NULL)
  {
    //if(opt.arg_debug)
    printf("Setting Tcl interp\n");
    SetRenderWindow(temp0);
    Tcl_ResetResult(interp);
  }
  else {
    if(opt.arg_debug) printf("Could not set renderwindow! Tcl interp\n");
  }
  
  id = Id;
  s  = sock;
}

pvVtkTclWidget::~pvVtkTclWidget()
{
  interpret("vtkCommand DeleteAllObjects");
  if(tclcommand != NULL) tclcommand->Delete();
  if(interp     != NULL) Tcl_DeleteInterp(interp);
  tclcommand = NULL; 
  interp = NULL;
//#ifndef PVUNIX // will be deleted in base class
//  RemoveRenderer(renderer);
//#endif
}

void pvVtkTclWidget::interpret(const char *format, ...)
{
  int estimated_length, ret;
  va_list ap;

  estimated_length = strlen(format) + 4096;
  char *buf = new char[estimated_length];

  va_start(ap,format);
  ret = rlvsnprintf(buf,estimated_length -1 , format, ap);
  va_end(ap);

  if(ret > 0 && tclcommand != NULL)
  {
#ifdef PVUNIX
    tclcommand->SetStringCommand(buf);
#endif
#ifdef PVWIN32
    tclcommand->SetStringCommand(buf);
    //if(tclcommand->StringCommand) { delete [] tclcommand->StringCommand; }
    //tclcommand->StringCommand = new char[strlen(buf)+1];
    //strcpy(tclcommand->StringCommand, buf);
#endif
    tclcommand->Execute(NULL, 0, NULL);
  }

  delete [] buf;

  //int res = Tcl_EvalEx(interp, format, -1, TCL_EVAL_GLOBAL);
  //printf("interpret tcl res: %d", res);
}

void pvVtkTclWidget::interpretFile(const char *filename)
{
  if(interp != NULL) Tcl_EvalFile(interp,filename);
}

void pvVtkTclWidget::updateGL()
{
  // only because of qt3->qt4 port
}

void pvVtkTclWidget::mouseMoveEvent(QMouseEvent *event)
{
  char buf[100];
  sprintf( buf, "QPlotMouseMoved(%d,%d,%d)\n",id, event->x(), event->x());
  if(opt.arg_debug) printf("%s\n",buf);
  tcp_send(s,buf,strlen(buf));
  QVTKOpenGLWidget::mouseMoveEvent(event);
}

void pvVtkTclWidget::mousePressEvent(QMouseEvent *event)
{
  char buf[100];
  sprintf( buf, "QPlotMousePressed(%d,%d,%d)\n",id, event->x(), event->y());
  if(opt.arg_debug) printf("%s\n",buf);
  tcp_send(s,buf,strlen(buf));
  QVTKOpenGLWidget::mousePressEvent(event);
}

void pvVtkTclWidget::mouseReleaseEvent(QMouseEvent *event)
{
  char buf[100];
  sprintf( buf, "QPlotMouseReleased(%d,%d,%d)\n",id, event->x(), event->y());
  if(opt.arg_debug) printf("%s\n",buf);
  if(underMouse()) tcp_send(s,buf,strlen(buf));
  QVTKOpenGLWidget::mouseReleaseEvent(event);
}

void pvVtkTclWidget::enterEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,1)\n",id);
  if(opt.arg_debug) printf("%s\n",buf);
  tcp_send(s,buf,strlen(buf));
  QVTKOpenGLWidget::enterEvent(event);
}

void pvVtkTclWidget::leaveEvent(QEvent *event)
{
  char buf[100];
  sprintf(buf, "mouseEnterLeave(%d,0)\n",id);
  if(opt.arg_debug) printf("%s\n",buf);
  tcp_send(s,buf,strlen(buf));
  QVTKOpenGLWidget::leaveEvent(event);
}
