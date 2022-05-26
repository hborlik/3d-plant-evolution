/*
* SuperSphere Class template from http://www.songho.ca/opengl/gl_sphere.html
* Modified by Jack Ortega to render a super shape instead, and to support animation.
*/

#ifndef GEOMETRY_SPHERE_H
#define GEOMETRY_SPHERE_H

#include <vector>
#include <resource.h>

struct SuperShapeParams {
    float n1 = .2f;
    float n2 = 1.7f;
    float n3 = 1.7f;
    float m = 7.f;
    float a = 1.f;
    float b = 1.f;

    float q1 = 0.2f;
    float q2 = 1.7f;
    float q3 = 1.7f;
    float k = 7.f;
    float c = 1.f;
    float d = 1.f;
};

class SuperSphere
{
public:
    // ctor/dtor
    SuperSphere(float radius=1.0f, int sectorCount=36, int stackCount=18);
    SuperSphere(float radius, int sectorCount, int stackCount, const SuperShapeParams& params);
    ~SuperSphere() {}

    // getters/setters
    float getRadius() const                 { return radius; }
    int getSectorCount() const              { return sectorCount; }
    int getStackCount() const               { return stackCount; }
    void set(float radius, int sectorCount, int stackCount);
    void setRadius(float radius);
    void setSectorCount(int sectorCount);
    void setStackCount(int stackCount);

    // for vertex data
    unsigned int getVertexCount() const     { return (unsigned int)vertices.size() / 3; }
    unsigned int getNormalCount() const     { return (unsigned int)normals.size() / 3; }
    unsigned int getTexCoordCount() const   { return (unsigned int)texCoords.size() / 2; }
    unsigned int getIndexCount() const      { return (unsigned int)indices.size(); }
    unsigned int getLineIndexCount() const  { return (unsigned int)lineIndices.size(); }
    unsigned int getTriangleCount() const   { return getIndexCount() / 3; }
    unsigned int getVertexSize() const      { return (unsigned int)vertices.size() * sizeof(float); }
    unsigned int getNormalSize() const      { return (unsigned int)normals.size() * sizeof(float); }
    unsigned int getTexCoordSize() const    { return (unsigned int)texCoords.size() * sizeof(float); }
    unsigned int getIndexSize() const       { return (unsigned int)indices.size() * sizeof(unsigned int); }
    unsigned int getLineIndexSize() const   { return (unsigned int)lineIndices.size() * sizeof(unsigned int); }
    const float* getVertices() const        { return vertices.data(); }
    const float* getNormals() const         { return normals.data(); }
    const float* getTexCoords() const       { return texCoords.data(); }
    const unsigned int* getIndices() const  { return indices.data(); }
    const unsigned int* getLineIndices() const  { return lineIndices.data(); }

    const std::vector<float>& getVerticesv() const        { return vertices; }
    const std::vector<float>& getNormalsv() const         { return normals; }
    const std::vector<float>& getTexCoordsv() const       { return texCoords; }
    const std::vector<unsigned int>& getIndicesv() const  { return indices; }
    const std::vector<unsigned int>& getLineIndicesv() const  { return lineIndices; }
    const std::vector<float>& getInterleavedVerticesv() const {return interleavedVertices;}

    // for interleaved vertices: V/N/T
    unsigned int getInterleavedVertexCount() const  { return getVertexCount(); }    // # of vertices
    unsigned int getInterleavedVertexSize() const   { return (unsigned int)interleavedVertices.size() * sizeof(float); }    // # of bytes
    int getInterleavedStride() const                { return interleavedStride; }   // should be 32 bytes
    const float* getInterleavedVertices() const     { return interleavedVertices.data(); }

    // debug
    void printSelf() const;

    std::unique_ptr<ev2::Model> getModel();

    // genes
    void setGenes(float a, float b, float m, float n1, float n2, float n3, 
                  float c, float d, float k, float q1, float q2, float q3){
                  this->a = a; this->b = b; this->m = m; this->n1 = n1; this->n2 = n2; this->n3 = n3;     
                  this->c = c; this->d = d; this->k = k; this->q1 = q1; this->q2 = q2; this->q3 = q3; 
                  }

    SuperSphere crossGenes(SuperSphere parentB);

    //genes
    float n1 = .2f;
    float n2 = 1.7f;
    float n3 = 1.7f;
    float m = 7.f;
    float a = 1.f; //* //sin(time); //+ sin(f);
    float b = 1.f;

    float q1 = 0.2f;
    float q2 = 1.7f;
    float q3 = 1.7f;
    float k = 7.f;
    float c = 1.f;
    float d = 1.f;

protected:

private:
    // member functions
    void buildVerticesSmooth();
    void buildInterleavedVertices();
    void clearArrays();
    void addVertex(float x, float y, float z);
    void addNormal(float x, float y, float z);
    void addTexCoord(float s, float t);
    void addIndices(unsigned int i1, unsigned int i2, unsigned int i3);
    std::vector<float> computeFaceNormal(float x1, float y1, float z1,
                                         float x2, float y2, float z2,
                                         float x3, float y3, float z3);

    // memeber vars
    float radius;
    int sectorCount;                        // longitude, # of slices
    int stackCount;                         // latitude, # of stacks
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texCoords;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> lineIndices;

    // interleaved
    std::vector<float> interleavedVertices;
    int interleavedStride;                  // # of bytes to hop to the next vertex (should be 32 bytes)


};

#endif
