#ifndef __test_main_h__
#define __test_main_h__

/* standard headers */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* EGL/GLES headers */
#include <EGL/egl.h>
#include <GLES2/gl2.h>

/* public functions */
extern char *mlx_eglError_string(int error);

#endif /*__test_main_h__*/