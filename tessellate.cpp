#include "tessellate.h"
#include "glutess.h"
#include "tess.h"

///// --- Structs --------------------------------------------------------------------------------------------

struct Vertex_s {
    double pt[3];
};

struct TessContext_s {
	struct Vertex_s *vertex;
	std::vector<float> *tripoints;
	std::vector<std::vector<float>> *edges;
	std::vector<float> *line;
	float z;
    void (*vertex_cb)(struct Vertex_s *, struct TessContext_s *);
};

struct TessContext_s *new_tess_context(std::vector<float> *tripoints, std::vector<std::vector<float>> *edges, std::vector<float> *line, float z)
{
    struct TessContext_s *result = (struct TessContext_s *)malloc(sizeof (struct TessContext_s));
	result->tripoints = tripoints;
	result->edges = edges;
	result->line = line;
    result->vertex = NULL;
	result->vertex_cb = NULL;
	result->z = z;
    return result;
}

struct Vertex_s *new_vertex(struct TessContext_s *ctx, double x, double y, double z)
{
	struct Vertex_s *result = (struct Vertex_s *)malloc(sizeof(struct Vertex_s));
    result->pt[0] = x;
    result->pt[1] = y;
	result->pt[2] = z;
    return ctx->vertex = result;
}

///// --- Callbacks --------------------------------------------------------------------------------------------

void cb_triangle_vertex(struct Vertex_s *v, struct TessContext_s *ctx) {
	std::vector<float> *tris = ctx->tripoints;
	tris->push_back(v->pt[0]); tris->push_back(v->pt[1]); tris->push_back(v->pt[2]);
}

void cb_line_loop(struct Vertex_s *v, struct TessContext_s *ctx)
{
	ctx->line->push_back(v->pt[0]); 
	ctx->line->push_back(v->pt[1]);
}

void cb_vertex(void *vertex_data, void *poly_data)
{
	struct TessContext_s *ctx = (struct TessContext_s *)poly_data;
	ctx->vertex_cb((struct Vertex_s *)vertex_data, ctx);
}

void cb_combine(const GLdouble newVertex[3], const void *neighborVertex_s[4], const GLfloat neighborWeight[4], void **outData, void *poly_data)
{
	struct TessContext_s *ctx = (struct TessContext_s *)poly_data;
	*outData = new_vertex(ctx, newVertex[0], newVertex[1], ctx->z);
}

void cb_begin(GLenum which, void *poly_data)
{
	struct TessContext_s *ctx = (struct TessContext_s *)poly_data;
	switch (which) {
	case GL_TRIANGLES: ctx->vertex_cb = &cb_triangle_vertex; break;
	case GL_LINE_LOOP:
		if (ctx->line->size() > 0) ctx->edges->push_back(*ctx->line);
		ctx->line->clear();
		ctx->vertex_cb = &cb_line_loop;
		break;
	}
}

void cb_edgeFlag(bool flag)
{
	bool edgeFlag = flag;
}

///// --- Main tesselate function --------------------------------------------------------------------------------------------

void tessellate (std::vector<std::vector<float>> *contours_in, std::vector<float> *tris_out, std::vector<std::vector<float>> *edges, bool getBounds, float z)
{
	if (contours_in == NULL) return;

    struct Vertex_s *current_vertex;
	GLUtesselator *tess = gluNewTess();
	std::vector<float> line; //Temp line contour
	struct TessContext_s *ctx = new_tess_context(tris_out, edges, &line, z);

	gluTessProperty(tess, GLU_TESS_BOUNDARY_ONLY, (getBounds) ? GL_TRUE : GL_FALSE);
    gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
    gluTessCallback(tess, GLU_TESS_VERTEX_DATA,  (_GLUfuncptr) &cb_vertex);
    gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (_GLUfuncptr) &cb_begin);
    gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (_GLUfuncptr) &cb_combine);
	gluTessCallback(tess, GLU_TESS_EDGE_FLAG, (_GLUfuncptr)&cb_edgeFlag); //Setting this callback produces triangles only

    gluTessBeginPolygon(tess, ctx);
	for (size_t c = 0; c < (*contours_in).size(); c++) {
		std::vector<float> &contour = (*contours_in)[c];
		gluTessBeginContour(tess);
		for (size_t v = 0; v < contour.size(); v += 2) {
			current_vertex = new_vertex(ctx, (double)contour[v], (double)contour[v+1], z);
			gluTessVertex(tess, current_vertex->pt, current_vertex);
		}
		gluTessEndContour(tess);
	}
	gluTessEndPolygon(tess);
	if (ctx->line->size() > 0) ctx->edges->push_back(*ctx->line);
	free(ctx);
    gluDeleteTess(tess);
}
