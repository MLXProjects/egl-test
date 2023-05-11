#include "main.h"

#define TEX_WIDTH		1280
#define TEX_HEIGHT		720

struct mali_native_window egl_win = {
	.width = TEX_WIDTH,
	.height = TEX_HEIGHT
};

/* default EGL config variables */
static EGLint const eglconfig_attrs[] = {
	EGL_RED_SIZE, 5,
	EGL_GREEN_SIZE, 6,
	EGL_BLUE_SIZE, 5,
	EGL_ALPHA_SIZE, 0,
	EGL_BUFFER_SIZE, 16,
	EGL_STENCIL_SIZE, 0,
	EGL_DEPTH_SIZE, 0,
	EGL_SAMPLES, 4,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT|EGL_PIXMAP_BIT,
	EGL_NONE
};

static EGLint eglwin_attrs[] = {
	EGL_NONE
};

static const EGLint eglcontext_attrs[] = {
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
};

int main(int argc, char **arg){
	/* some local variables */
	EGLDisplay egl_disp;
	EGLSurface egl_surface;
	EGLint egl_major, egl_minor;
	EGLConfig config;
	EGLint num_config;
	EGLContext context;
	GLint surface_width, surface_height;
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;
	GLint ret;
	int running = 1;
	/* init EGL display & instance */
	egl_disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (egl_disp == EGL_NO_DISPLAY){
		goto main_err;
	}
	if (!eglInitialize(egl_disp, &egl_major, &egl_minor)){
		goto main_err;
	}
	/* print EGL information */
	printf("EGL Version: \"%s\"\n", eglQueryString(egl_disp, EGL_VERSION));
	printf("EGL Vendor: \"%s\"\n", eglQueryString(egl_disp, EGL_VENDOR));
	/* set default config */
	eglChooseConfig(egl_disp, eglconfig_attrs, &config, 1, &num_config);
	/* create EGL context */
	context = eglCreateContext(egl_disp, config, EGL_NO_CONTEXT, eglcontext_attrs);
	if (context == EGL_NO_CONTEXT){
		goto main_err;
	}
	/* create native screen surface */
	egl_surface = eglCreateWindowSurface(egl_disp, config,
					     &egl_win,
					     eglwin_attrs);
	if (egl_surface == EGL_NO_SURFACE){
		goto main_err;
	}
	/* get & print created surface info */
	if (!eglQuerySurface(egl_disp, egl_surface, EGL_WIDTH, &surface_width) ||
	    !eglQuerySurface(egl_disp, egl_surface, EGL_HEIGHT, &surface_height)) {
		goto main_err;
	}
	printf("Surface size: %dx%d\n", surface_width, surface_height);
	/* set current context - enables actual opengl es? */
	if (!eglMakeCurrent(egl_disp, egl_surface, egl_surface, context)) {
		goto main_err;
	}
	/* print some GLES info */
	printf("GL Vendor: \"%s\"\n", glGetString(GL_VENDOR));
	printf("GL Renderer: \"%s\"\n", glGetString(GL_RENDERER));
	printf("GL Version: \"%s\"\n", glGetString(GL_VERSION));
	/*****************EGL/GL ES INITIALIZED*****************/
	/* declare triangle shader & fragment */
	const char *vertex_shader_source =
	"attribute vec4 aPosition; \n"
	"attribute vec4 aColor; \n"
	"varying vec4 vColor; \n"
	"void main(){ \n"
	"    vColor = aColor; \n"
	"    gl_Position = aPosition; \n"
	"} \n";
	const char *fragment_shader_source =
	"precision mediump float; \n"
	"varying vec4 vColor; \n"
	"void main(){ \n"
	"    gl_FragColor = vColor; \n"
	"} \n";
	/* declare triangle vertices & colors */
	GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f,
							-0.5f, -0.5f, 0.0f,
							 0.5f, -0.5f, 0.0f };
	GLfloat vColors[] = {
				1.0f, 0.0f, 0.0f, 
				1.0f, 0.0f, 1.0f,
				0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 1.0f};
	/* create vertex shader */
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	if (!vertex_shader){
		goto main_err;
	}
	/* set source & compile vertex shader */
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	/* check if vertex shader compiled correctly */
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &ret);
	if (!ret){
		fprintf(stderr, "Error: vertex shader compilation failed!\n");
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1){
			char *log = malloc(ret+1);
			glGetShaderInfoLog(vertex_shader, ret+1, NULL, log);
			fprintf(stderr, "%s", log);
		}
		goto main_err;
	}
	/* create fragment shader */
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!fragment_shader) {
		goto main_err;
	}
	/* set source & compile fragment shader */
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	/* check if fragment shader compiled correctly  */
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &ret);
	if (!ret) {
		fprintf(stderr, "Error: fragment shader compilation failed!\n");
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &ret);
		if (ret > 1) {
			char *log = malloc(ret+1);
			glGetShaderInfoLog(fragment_shader, ret+1, NULL, log);
			fprintf(stderr, "%s", log);
		}
		goto main_err;
	}
	/* create GL program */
	program = glCreateProgram();
	if (!program){
		goto main_err;
	}
	/* attach vertex & fragment shaders to GL program */
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	/* bind attribute names in shader sources */
	glBindAttribLocation(program, 0, "aPosition");
	glBindAttribLocation(program, 1, "aColor");
	/* link GL program */
	glLinkProgram(program);
	/* check if GL program compiled correctly */
	glGetProgramiv(program, GL_LINK_STATUS, &ret);
	if (!ret){
		fprintf(stderr, "Error: program linking failed!\n");
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &ret);

		if (ret > 1){
			char *log = malloc(ret);
			glGetProgramInfoLog(program, ret, NULL, log);
			fprintf(stderr, "%s", log);
		}
		goto main_err;
	}
	/* set compiled GL program as current vertex/fragment renderer */
	glUseProgram(program);
	/* setup clear color & blank at startup */
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	/* init arrays? */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, vColors);
	glEnableVertexAttribArray(1);
	/* draw scene frame */
	float state=0.0f;
	int reverse=0, count=6;
	do {
		if (reverse) state -= 0.016f;
		else state += 0.016f;
		/* setup clear color */
		glClearColor(state, state, state, 1.0);
		/* set viewport to screen center */
		glViewport(0, 0, surface_width, surface_height);
		/* clear scene */
		glClear(GL_COLOR_BUFFER_BIT);
		/* draw triangle */
		glDrawArrays(GL_TRIANGLES, 0, 3);
		/* swap drawing buffers? */
		eglSwapBuffers(egl_disp, egl_surface);
		usleep(16*1000);
		if (state>=1.0f) reverse=1;
		else if (state<=0.0f){
			reverse=0;
			count--;
		}
	} while (count);
	/* clear screen before exit */
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	/* TODO: cleanup */
	return 0;
main_err:;
	int last_err = eglGetError();
	printf("%s (0x%#04x)\n", mlx_eglError_string(last_err), last_err);
	return 1;
}
