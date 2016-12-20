#ifndef GLDRAWSURFACE_H
#define GLDRAWSURFACE_H

#include <QGLWidget>
#include <QOpenGLFunctions_3_0>
#include <QMatrix4x4>
#include "gl/GL.h"

class GLDrawSurface : public QGLWidget, protected QOpenGLFunctions_3_0
{
    Q_OBJECT
public:
    explicit GLDrawSurface(QWidget *parent = 0);
    QMatrix4x4 getOrtho();
    QMatrix4x4 getScreenMatrix();
    void genBuffers(int count, GLuint* buff);
    void deleteBuffers(GLsizei _n, const GLuint* _buffers);
    void bindBuffer(GLenum _target, GLuint _buffer);
    void bufferData(GLenum _target, qopengl_GLsizeiptr _size, const void* _data, GLenum _usage);
    void vertexAttribPointer(GLuint _indx, GLint _size, GLenum _type, GLboolean _normalized, GLsizei _stride, const void* _ptr);
    void drawElements (GLenum _mode, GLsizei _count, GLenum _type, const GLvoid* _indices);
    void genTextures(int count, GLuint* buff);
    void texBuffer(GLenum _target, GLenum _internalformat, GLuint _buffer);
    void _bindTexture(GLenum _target, GLuint _texture);
    void activeTexture(GLenum _texture);
    void texParameteri(GLenum _target, GLenum _pname, GLint _param);
    const GLubyte* getString(GLenum _name);
    void getIntegerv(GLenum _pname, GLint *_params);
    void genFramebuffers(GLsizei _n, GLuint *_framebuffers);
    void bindFramebuffer(GLenum _target, GLuint _framebuffer);
    void texImage2D(GLenum _target, GLint _level, GLint _internalformat, GLsizei _width, GLsizei _height, GLint _border, GLenum _format, GLenum _type, const GLvoid *_pixels);
    void drawBuffers(GLsizei _n, const GLenum *_bufs);
    void framebufferTexture(GLenum _target, GLenum _attachment, GLuint _texture, GLint _level);
    void deleteFramebuffers(GLsizei _n, const GLuint *_framebuffers);
    GLenum checkFramebufferStatus(GLenum _target);
    void getTexImage(GLenum _target, GLint _level, GLenum _format, GLenum _type, GLvoid * _img);
    int getDrawWidth();
    int getDrawHeight();
protected:
    QMatrix4x4 ortho;
    QMatrix4x4 screen;
    int _width;
    int _height;

signals:

public slots:

};

#endif // GLDRAWSURFACE_H
