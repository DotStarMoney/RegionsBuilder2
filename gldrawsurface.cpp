#include "gldrawsurface.h"

GLDrawSurface::GLDrawSurface(QWidget *parent) :
    QGLWidget(parent)
{
    setAutoFillBackground(false);
}

int GLDrawSurface::getDrawWidth(){
   return _width;
}

int GLDrawSurface::getDrawHeight(){
   return _height;
}

QMatrix4x4 GLDrawSurface::getOrtho(){
    return ortho;
}

QMatrix4x4 GLDrawSurface::getScreenMatrix(){
    return screen;
}

void GLDrawSurface::getTexImage(GLenum _target, GLint _level, GLenum _format, GLenum _type, GLvoid * _img){
    glGetTexImage(_target, _level, _format, _type, _img);
}

void GLDrawSurface::getIntegerv(GLenum _pname, GLint *_params){
    glGetIntegerv(_pname, _params);
}

void GLDrawSurface::genTextures(int count, GLuint* buff){
    glGenTextures(count, buff);
}

void GLDrawSurface::genBuffers(int count, GLuint *buff){
    glGenBuffers(count, buff);
}

void GLDrawSurface::bindBuffer(GLenum _target, GLuint _buffer){
    glBindBuffer(_target, _buffer);
}

void GLDrawSurface::bufferData(GLenum _target, qopengl_GLsizeiptr _size, const void *_data, GLenum _usage){
    glBufferData(_target, _size, _data, _usage);
}

void GLDrawSurface::deleteBuffers(GLsizei _n, const GLuint *_buffers){
    glDeleteBuffers(_n, _buffers);
}

void GLDrawSurface::vertexAttribPointer(GLuint _indx, GLint _size, GLenum _type, GLboolean _normalized, GLsizei _stride, const void *_ptr){
    glVertexAttribPointer(_indx, _size, _type, _normalized, _stride, _ptr);
}

void GLDrawSurface::drawElements(GLenum _mode, GLsizei _count, GLenum _type, const GLvoid *_indices){
    glDrawElements(_mode, _count, _type, _indices);
}

void GLDrawSurface::texBuffer(GLenum _target, GLenum _internalformat, GLuint _buffer){
   // glTexBuffer(_target, _internalformat, _buffer);
}

GLenum GLDrawSurface::checkFramebufferStatus(GLenum _target){
    return glCheckFramebufferStatus(_target);
}


void GLDrawSurface::_bindTexture(GLenum _target, GLuint _texture){
    glBindTexture(_target, _texture);
}

void GLDrawSurface::activeTexture(GLenum _texture){
    glActiveTexture(_texture);
}

void GLDrawSurface::texParameteri(GLenum _target, GLenum _pname, GLint _param){
    glTexParameteri(_target,_pname,_param);
}

const GLubyte* GLDrawSurface::getString(GLenum _name){
    return glGetString(_name);
}

void GLDrawSurface::genFramebuffers(GLsizei _n, GLuint *_framebuffers){
    glGenFramebuffers(_n, _framebuffers);
}

void GLDrawSurface::bindFramebuffer(GLenum _target, GLuint _framebuffer){
    glBindFramebuffer(_target,_framebuffer);
}

void GLDrawSurface::texImage2D(GLenum _target, GLint _level, GLint _internalformat, GLsizei _width, GLsizei _height, GLint _border, GLenum _format, GLenum _type, const GLvoid *_pixels){
    glTexImage2D(_target, _level, _internalformat, _width, _height, _border, _format, _type, _pixels);
}

void GLDrawSurface::drawBuffers(GLsizei _n, const GLenum *_bufs){
    glDrawBuffers(_n, _bufs);
}

void GLDrawSurface::framebufferTexture(GLenum _target, GLenum _attachment, GLuint _texture, GLint _level){
    glFramebufferTexture2D(_target, _attachment, GL_TEXTURE_2D,  _texture, _level);
}

void GLDrawSurface::deleteFramebuffers(GLsizei _n, const GLuint *_framebuffers){
    glDeleteFramebuffers(_n, _framebuffers);
}


