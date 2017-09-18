#include "tessellate.h"
#include "glutess.h"
#include "tess.h"
#include <stdio.h>
#include <stdlib.h>

struct Triangle_s {
	double ax, ay, bx, by, cx, cy;
    struct Triangle_s *prev;
};

struct Vertex_s {
    double pt[3];
};

struct TessContext_s {
	struct Vertex_s *latest_v;
	int pointCount;
	double ax, ay, bx, by;
    GLenum current_mode;
    int odd_even_strip;
	std::vector<float> *tripoints;
    void (*vertex_cb)(struct Vertex_s *, struct TessContext_s *);
};

void skip_vertex(struct Vertex_s *v, struct TessContext_s *ctx) {};

/******************************************************************************/

struct TessContext_s *new_tess_context(std::vector<float> *tripoints)
{
    struct TessContext_s *result = (struct TessContext_s *)malloc(sizeof (struct TessContext_s));
	result->tripoints = tripoints;
    result->latest_v = NULL;
	result->pointCount = 0;
	result->vertex_cb = &skip_vertex;
    result->odd_even_strip = 0;
	result->ax = result->ay = result->bx = result->by = 0;
    return result;
}

struct Vertex_s *new_vertex(struct TessContext_s *ctx, double x, double y)
{
	struct Vertex_s *result = (struct Vertex_s *)malloc(sizeof(struct Vertex_s));
    result->pt[0] = x;
    result->pt[1] = y;
	result->pt[2] = 0;
    return ctx->latest_v = result;
}

void new_tri(std::vector<float> *tris, float p1x, float p1y, float p2x, float p2y, float p3x, float p3y)
{
	tris->push_back(p1x); tris->push_back(p1y); tris->push_back(-0.9f);
	tris->push_back(p2x); tris->push_back(p2y); tris->push_back(-0.9f);
	tris->push_back(p3x); tris->push_back(p3y); tris->push_back(-0.9f);
}

/******************************************************************************/

void fan_vertex(struct Vertex_s *v, struct TessContext_s *ctx) {
	ctx->pointCount++;
	switch (ctx->pointCount) {
	case 1:ctx->ax = v->pt[0]; ctx->ay = v->pt[1]; return;
	case 2:ctx->bx = v->pt[0]; ctx->by = v->pt[1]; return;
	}
	new_tri(ctx->tripoints, ctx->ax, ctx->ay, ctx->bx, ctx->by, v->pt[0], v->pt[1]);
	ctx->bx = v->pt[0]; ctx->by = v->pt[1];
}

void strip_vertex(struct Vertex_s *v, struct TessContext_s *ctx)
{
	ctx->pointCount++;
	switch (ctx->pointCount) {
	case 1:ctx->bx = v->pt[0]; ctx->by = v->pt[1]; return;
	case 2:ctx->ax = v->pt[0]; ctx->ay = v->pt[1]; return;
	}
	if (ctx->odd_even_strip)
		new_tri(ctx->tripoints, ctx->ax, ctx->ay, ctx->bx, ctx->by, v->pt[0], v->pt[1]);
	else
		new_tri(ctx->tripoints, ctx->bx, ctx->by, ctx->ax, ctx->ay, v->pt[0], v->pt[1]);
	ctx->odd_even_strip = !ctx->odd_even_strip;
	ctx->bx = ctx->ax; ctx->by = ctx->ay;
	ctx->ax = v->pt[0]; ctx->ay = v->pt[1];
}

void triangle_vertex(struct Vertex_s *v, struct TessContext_s *ctx) {
	ctx->pointCount++;
	switch (ctx->pointCount) {
	case 1:ctx->ax = v->pt[0]; ctx->ay = v->pt[1]; return;
	case 2:ctx->bx = v->pt[0]; ctx->by = v->pt[1]; return;
	}
	new_tri(ctx->tripoints, ctx->ax, ctx->ay, ctx->bx, ctx->by, v->pt[0], v->pt[1]);
}

void vertex(void *vertex_data, void *poly_data)
{
	struct TessContext_s *ctx = (struct TessContext_s *)poly_data;
	ctx->vertex_cb((struct Vertex_s *)vertex_data, ctx);
}

void combine(const GLdouble newVertex[3], const void *neighborVertex_s[4], 
	const GLfloat neighborWeight[4], void **outData, void *polyData)
{
	*outData = new_vertex((struct TessContext_s *)polyData, newVertex[0], newVertex[1]);
}

void begin(GLenum which, void *poly_data)
{
	struct TessContext_s *ctx = (struct TessContext_s *)poly_data;
	ctx->pointCount = 0;
	ctx->odd_even_strip = 0;
	switch (which) {
	case GL_TRIANGLES: ctx->vertex_cb = &triangle_vertex; break;
	case GL_TRIANGLE_STRIP: ctx->vertex_cb = &strip_vertex; break;
	case GL_TRIANGLE_FAN: ctx->vertex_cb = &fan_vertex; break;
	default:
		//SDL_Log("ERROR, can't handle %d\n", (int)which);
		ctx->vertex_cb = &skip_vertex;
	}
}

void tessellate (std::vector<float> &tris, std::vector<std::vector<float>> &contours)
{
    struct Vertex_s *current_vertex;
	GLUtesselator *tess = gluNewTess();
	struct TessContext_s *ctx = new_tess_context(&tris);

	//gluTessProperty(tess, GLU_TESS_BOUNDARY_ONLY, GL_TRUE);
    gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
    gluTessCallback(tess, GLU_TESS_VERTEX_DATA,  (_GLUfuncptr) &vertex);
    gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (_GLUfuncptr) &begin);
    gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (_GLUfuncptr) &combine);

    gluTessBeginPolygon(tess, ctx);
	for (size_t c = 0; c < contours.size(); c++) {
		std::vector<float> &contour = contours[c];
		gluTessBeginContour(tess);
		for (size_t v = 0; v < contour.size(); v += 2) {
			current_vertex = new_vertex(ctx, (double)contour[v], (double)contour[v+1]);
			gluTessVertex(tess, current_vertex->pt, current_vertex);
		}
		gluTessEndContour(tess);
	}
	gluTessEndPolygon(tess);

	free(ctx);
    gluDeleteTess(tess);
}
