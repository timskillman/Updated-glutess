#include "tessellate.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>

/*Define small structs / classes here*/

std::vector<float> createContour(std::string vals) {
    std::vector<float> fvals;
    std::istringstream ss(vals); 
    float f;
    while (ss >> f) fvals.push_back(f);
    return fvals;
}

int main()
{
    //Create 3x contours including one that intersects the other ...
    std::vector<std::vector<float>> contours;
    contours.push_back(createContour("0 0 300 0 300 300 0 300"));
    contours.push_back(createContour("100 100 200 100 420 420 100 200"));
    contours.push_back(createContour("400 100 450 150 400 200 350 150"));
  
    //Tessellate the contours into triangles and store the results in 'tris' array ...
    std::vector<float> tris;  /* store triangles in tris array - each triangle is 9 floats (3x XYZ) */
    tessellate(tris, contours);
  
  // Setup for rendering ...
  
    uint32_t vc = tris.size() / 3;
    std::vector<GLshort> lindexes;
    for (size_t i = 0; i < tris.size(); i++) lindexes.push_back(i);
  
  // Here's a basic means of rendering the triangles and their outlines ...
  
     /*	if (vc > 1) {
		glColor4ub(255, 255, 255, 255);
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &tris.front());
		glDrawElements(GL_TRIANGLES, vc, GL_UNSIGNED_SHORT, &lindexes.front());		//Draw triangles
		glColor4ub(0, 255, 0, 255);
		glDrawElements(GL_LINES, vc, GL_UNSIGNED_SHORT, &lindexes.front());		//Draw triangle outlines
	}  */
    
    return 0;
}
