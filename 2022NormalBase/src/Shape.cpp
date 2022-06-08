#include "Shape.h"
#include <iostream>
#include <assert.h>

#include "GLSL.h"
#include "Program.h"

using namespace std;
using namespace glm;

Shape::Shape() :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0), 
   vaoID(0)
{
	min = glm::vec3(0);
	max = glm::vec3(0);
}

Shape::~Shape()
{
}

/* copy the data from the shape to this object */
void Shape::createShape(tinyobj::shape_t & shape)
{
		posBuf = shape.mesh.positions;
		norBuf = shape.mesh.normals;
		texBuf = shape.mesh.texcoords;
		eleBuf = shape.mesh.indices;
}

void Shape::measure() {
  float minX, minY, minZ;
   float maxX, maxY, maxZ;

   minX = minY = minZ = 1.1754E+38F;
   maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
		if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
		if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

		if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
		if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

		if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
		if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
   max.x = maxX;
   max.y = maxY;
   max.z = maxZ;
}



/* Note this is fairly dorky - */
void Shape::ComputeTex() {
	float u, v;

	for (size_t n = 0; n < norBuf.size()/3; n++) {
		u = norBuf[n*3+0]/2.0 + 0.5;
		v = norBuf[n*3+1]/2.0 + 0.5;
        texBuf[n*3+0] = u;
        texBuf[n*3+1] = v;
     }
}

void Shape::ComputeTanBN() {
	int idx0, idx1, idx2;
	vec3 v0, v1, v2;
	vec2 t0, t1, t2;
	vec3 e0, e1;
	vec2 texE0, texE1;
	float weight;
	vec3 Tan, biTan;

	//bootstrap for every vertex create a tangent and biTangent
	for (size_t n = 0; n < posBuf.size(); n++) {
		   	tanBuf.push_back(0);
				BNBuf.push_back(0);	
	}
		
	for (size_t n = 0; n < eleBuf.size()/3; n++) {
			idx0 = eleBuf[n*3];
			idx1 = eleBuf[n*3 +1];
			idx2 = eleBuf[n*3 +2];
			v0 = vec3(posBuf[idx0*3+0], posBuf[idx0*3+1], posBuf[idx0*3+2]);
			v1 = vec3(posBuf[idx1*3+0], posBuf[idx1*3+1], posBuf[idx1*3+2]);
			v2 = vec3(posBuf[idx2*3+0], posBuf[idx2*3+1], posBuf[idx2*3+2]);
			t0 = vec2(texBuf[idx0*2+0], texBuf[idx0*2+1]); 
			t1 = vec2(texBuf[idx1*2+0], texBuf[idx1*2+1]); 
			t2 = vec2(texBuf[idx2*2+0], texBuf[idx2*2+1]); 
			e0 = v1 - v0;
			e1 = v2 - v0;
			texE0 = t1 - t0;
			texE1 = t2 - t0;
			weight = 1.0f/ (texE0.x*texE1.y - texE0.y*texE1.x);
			Tan = (e0*texE1.y - e1*texE0.y)*weight;
			biTan = (e1*texE0.x - e0*texE1.x)*weight;
			//set the tangent and biTangent for each vertex
		   	tanBuf[idx0*3] = Tan.x;
		   	tanBuf[idx0*3 +1] = Tan.y;
		   	tanBuf[idx0*3 +2] = Tan.z;
		   	BNBuf[idx0*3] = biTan.x;
		   	BNBuf[idx0*3 +1] = biTan.y;
		   	BNBuf[idx0*3 +2] = biTan.z;
		   	tanBuf[idx1*3] = Tan.x;
		   	tanBuf[idx1*3 +1] = Tan.y;
		   	tanBuf[idx1*3 +2] = Tan.z;
		   	BNBuf[idx1*3] = biTan.x;
		   	BNBuf[idx1*3 +1] = biTan.y;
		   	BNBuf[idx1*3 +2] = biTan.z;
		   	tanBuf[idx2*3] = Tan.x;
		   	tanBuf[idx2*3 +1] = Tan.y;
		   	tanBuf[idx2*3 +2] = Tan.z;
		   	BNBuf[idx2*3] = biTan.x;
		   	BNBuf[idx2*3 +1] = biTan.y;
		   	BNBuf[idx2*3 +2] = biTan.z;
	}
}

void Shape::init(bool norMap)
{
   // Initialize the vertex array object
   glGenVertexArrays(1, &vaoID);
   glBindVertexArray(vaoID);

	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(norBuf.empty()) {
		norBufID = 0;
		cout << "warning no normals" << endl;
	} else {
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the texture array to the GPU
	if(texBuf.empty()) {
		//send in approximate constructed
		for (size_t v = 0; v < posBuf.size(); v++) {
         texBuf.push_back(0);
      	}
      	ComputeTex();

		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	} else {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}

	if (norMap) {
		ComputeTanBN();
		glGenBuffers(1, &tanBufID);
		glBindBuffer(GL_ARRAY_BUFFER, tanBufID);
		glBufferData(GL_ARRAY_BUFFER, tanBuf.size()*sizeof(float), &tanBuf[0], GL_STATIC_DRAW);
		glGenBuffers(1, &BNBufID);
		glBindBuffer(GL_ARRAY_BUFFER, BNBufID);
		glBufferData(GL_ARRAY_BUFFER, BNBuf.size()*sizeof(float), &BNBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}


void Shape::draw(const shared_ptr<Program> prog, bool norMap) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

   glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if(h_nor != -1 && norBufID != 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (texBufID != 0) {	
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if(h_tex != -1 && texBufID != 0) {
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}
	
	int h_tan = -1, h_bn = -1;
	if (norMap) {
	  h_tan = prog->getAttribute("vertTan");
	  if(h_tan != -1 && tanBufID != 0) {
		  GLSL::enableVertexAttribArray(h_tan);
		  glBindBuffer(GL_ARRAY_BUFFER, tanBufID);
		  glVertexAttribPointer(h_tan, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	  }
	  h_bn = prog->getAttribute("vertBN");
	  if(h_bn != -1 && BNBufID != 0) {
		  GLSL::enableVertexAttribArray(h_bn);
		  glBindBuffer(GL_ARRAY_BUFFER, BNBufID);
		  glVertexAttribPointer(h_bn, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	  }
	}

	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	
	// Draw
	glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);
	
	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	if(h_tan != -1) {
		GLSL::disableVertexAttribArray(h_tan);
		GLSL::disableVertexAttribArray(h_bn);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
