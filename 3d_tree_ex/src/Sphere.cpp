/*
* SuperSphere Class template from http://www.songho.ca/opengl/gl_sphere.html
* Modified by Jack Ortega to render a super shape instead, and to support animation.
*/
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
//#include <GL/gl.h>
#endif

#include <tiny_obj_loader.h>
#include <iostream>
#include <iomanip>
#include <Sphere.h>
#include <cmath>
#include <resource.h>

const int MIN_SECTOR_COUNT = 3;
const int MIN_STACK_COUNT = 2;

struct DrawObject {
    size_t start;
    size_t numTriangles;
    size_t material_id;
};

//stl lerp 
template <class _Ty> /* constexpr */ _Ty lerp(const _Ty _ArgA, const _Ty _ArgB, const _Ty _ArgT) noexcept {
    // on a line intersecting {(0.0, _ArgA), (1.0, _ArgB)}, return the Y value for X == _ArgT

    const int _Finite_mask = (int{ std::isfinite(_ArgA) } << 2) | (int{ std::isfinite(_ArgB) } << 1) | int{ std::isfinite(_ArgT) };
    if (_Finite_mask == 0b111) {
        // 99% case, put it first; this block comes from P0811R3
        if ((_ArgA <= 0 && _ArgB >= 0) || (_ArgA >= 0 && _ArgB <= 0)) {
            // exact, monotonic, bounded, determinate, and (for _ArgA == _ArgB == 0) consistent:
            return _ArgT * _ArgB + (1 - _ArgT) * _ArgA;
        }

        if (_ArgT == 1) {
            // exact
            return _ArgB;
        }

        // exact at _ArgT == 0, monotonic except near _ArgT == 1, bounded, determinate, and consistent:
        const auto _Candidate = _ArgA + _ArgT * (_ArgB - _ArgA);
        // monotonic near _ArgT == 1:
        if ((_ArgT > 1) == (_ArgB > _ArgA)) {
            if (_ArgB > _Candidate) {
                return _ArgB;
            }
        }
        else {
            if (_Candidate > _ArgB) {
                return _ArgB;
            }
        }

        return _Candidate;
    }

    if (std::isnan(_ArgA)) {
        return _ArgA;
    }

    if (std::isnan(_ArgB)) {
        return _ArgB;
    }

    if (std::isnan(_ArgT)) {
        return _ArgT;
    }

    switch (_Finite_mask) {
    case 0b000:
        // All values are infinities
        if (_ArgT >= 1) {
            return _ArgB;
        }

        return _ArgA;
    case 0b010:
    case 0b100:
    case 0b110:
        // _ArgT is an infinity; return infinity in the "direction" of _ArgA and _ArgB
        return _ArgT * (_ArgB - _ArgA);
    case 0b001:
        // Here _ArgA and _ArgB are infinities
        if (_ArgA == _ArgB) {
            // same sign, so T doesn't matter
            return _ArgA;
        }

        // Opposite signs, choose the "infinity direction" according to T if it makes sense.
        if (_ArgT <= 0) {
            return _ArgA;
        }

        if (_ArgT >= 1) {
            return _ArgB;
        }

        // Interpolating between infinities of opposite signs doesn't make sense, NaN
        if constexpr (sizeof(_Ty) == sizeof(float)) {
            return __builtin_nanf("0");
        }
        else {
            return __builtin_nan("0");
        }
    case 0b011:
        // _ArgA is an infinity but _ArgB is not
        if (_ArgT == 1) {
            return _ArgB;
        }

        if (_ArgT < 1) {
            // towards the infinity, return it
            return _ArgA;
        }

        // away from the infinity
        return -_ArgA;
    case 0b101:
        // _ArgA is finite and _ArgB is an infinity
        if (_ArgT == 0) {
            return _ArgA;
        }

        if (_ArgT > 0) {
            // toward the infinity
            return _ArgB;
        }

        return -_ArgB;
    case 0b111: // impossible; handled in fast path
    default:
        exit(1);
    }
}

float superformula_do(float t, float a, float b, float m, float n1, float n2, float n3) {
    //x'(t) = -m/(4*n1)*smt_4*cmt_4*((n3 asmt_4)^(n3 - 2))/b^2 - (n2 acmt_4)^(n2 - 2))/a^2) (acmt_4)^n2 + asmt_4)^n3)^(-(n1 + 1)/n1))
    float smt_4 = sin((m*t)/4.f);
    float cmt_4 = abs(cos((m*t)/4.f));
    float asmt_4 = abs(smt_4) / b;
    float acmt_4 = abs(cmt_4) / a;
    float x_p = -m/(4*n1)*smt_4*cmt_4*(pow(n3*asmt_4,n3 - 2)/pow(b,2) - pow(n2*acmt_4,n2 - 2)/pow(a,2))*pow(pow(acmt_4,n2) + pow(asmt_4,n3),(-(n1 + 1)/n1));
    return x_p;
}

//Supershape superformula
float superformula(float o, float a, float b, float m, float n1, float n2, float n3)
{
    double ffc = cos(m * o / 4);
    double ffs = sin(m * o / 4);
    double fa = glm::abs(ffc) / a;
    double fb = glm::abs(ffs) / b;
    double fffb = pow(fa, n2);
    double fffc = pow(fb, n3);
    return pow(fffb + fffc, 1 / n1);
}


//superparameters & pi
float pi = 3.1415926f;
float halfpi = pi / 2.0f;
int randLim = 1;



SuperSphere::SuperSphere(float radius, int sectors, int stacks) : interleavedStride(32)
{
    set(radius, sectors, stacks);
}

SuperSphere::SuperSphere(float radius, int sectorCount, int stackCount, const SuperShapeParams& params) :
    interleavedStride{32},
    n1{params.n1},
    n2{params.n2},
    n3{params.n3},
    m {params.m },
    a {params.a },
    b {params.b },
    q1{params.q1},
    q2{params.q2},
    q3{params.q3},
    k {params.k },
    c {params.c },
    d {params.d } {
    set(radius, sectorCount, stackCount);
}

static float randomFloatTo(float limit) {
    return static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/limit));
}

SuperSphere SuperSphere::crossGenes(const SuperSphere parentB){
                      SuperSphere child = SuperSphere();
                      float randomGeneWeight = randomFloatTo(1) + 1.f;
                      child.setGenes(
                                     ((this->a + parentB.a)/randomGeneWeight),
                                     ((this->b + parentB.b)/randomGeneWeight),
                                     ((this->m + parentB.m)/randomGeneWeight),
                                     ((this->n1 + parentB.n1)/randomGeneWeight),
                                     ((this->n2 + parentB.n2)/randomGeneWeight),
                                     ((this->n3 + parentB.n3)/randomGeneWeight),
                                     ((this->c + parentB.c)/randomGeneWeight),
                                     ((this->d + parentB.d)/randomGeneWeight),
                                     ((this->k + parentB.k)/randomGeneWeight),
                                     ((this->q1 + parentB.q1)/randomGeneWeight),
                                     ((this->q2 + parentB.q2)/randomGeneWeight),
                                     ((this->q3 + parentB.q3)/randomGeneWeight)
                                     );
                      return child;
                  }

void SuperSphere::set(float radius, int sectors, int stacks)
{
    this->radius = radius;
    this->sectorCount = sectors;
    if (sectors < MIN_SECTOR_COUNT)
        this->sectorCount = MIN_SECTOR_COUNT;
    this->stackCount = stacks;
    if (sectors < MIN_STACK_COUNT)
        this->sectorCount = MIN_STACK_COUNT;


    buildVerticesSmooth();
}

void SuperSphere::setRadius(float radius)
{
    if (radius != this->radius)
        set(radius, sectorCount, stackCount);
}

void SuperSphere::setSectorCount(int sectors)
{
    if (sectors != this->sectorCount)
        set(radius, sectors, stackCount);
}

void SuperSphere::setStackCount(int stacks)
{
    if (stacks != this->stackCount)
        set(radius, sectorCount, stacks);
}


void SuperSphere::printSelf() const
{
    std::cout << "===== Supershape =====\n"
        << "  Sector Count: " << sectorCount << "\n"
        << "   Stack Count: " << stackCount << "\n"
        << "Triangle Count: " << getTriangleCount() << "\n"
        << "   Index Count: " << getIndexCount() << "\n"
        << "  Vertex Count: " << getVertexCount() << "\n"
        << "  Normal Count: " << getNormalCount() << "\n"
        << "TexCoord Count: " << getTexCoordCount() << std::endl;
}


void SuperSphere::clearArrays()
{
    std::vector<float>().swap(vertices);
    std::vector<float>().swap(normals);
    std::vector<float>().swap(texCoords);
    std::vector<unsigned int>().swap(indices);
    std::vector<unsigned int>().swap(lineIndices);
}



void SuperSphere::buildVerticesSmooth()
{
    const float PI = acos(-1);

    // clear memory of prev arrays
    clearArrays();

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
    float s, t;                                     // texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;
    float uPer;
    float theta;
    float rTheta;
    float vPer;

    float phi;
    float rPhi;
    float cosphi;
    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        

        uPer = float(i) / float(stackCount);
        theta = lerp(-pi, pi, uPer);
        rTheta = 1.0f / superformula(theta, a, b, m, n1, n2, n3);


        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)
        {

            float vPer = float(j) / float(sectorCount);

            float phi = lerp(-pi/2, pi/2, vPer);
            float rPhi = 1.0f / superformula(phi, c, d, k, q1, q2, q3);
            float cosphi = cos(phi);
            x = cos(theta) * cosphi * rTheta * rPhi * radius;
            y = sin(theta) * cosphi * rTheta * rPhi * radius;
            z = sin(phi) * rPhi * radius;
            //std::cout << phi << "\n";
            //std::cout << rPhi << "\n";
            //std::cout << rTheta << "\n";
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            addVertex(x, y, z);

            // normalized vertex normal
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            addNormal(nx, ny, nz);

            // vertex tex coord between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            addTexCoord(s, t);
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    unsigned int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount + 1; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding 1st and last stacks
            if (true)
            {
                addIndices(k1, k2, k1 + 1);   // k1---k2---k1+1
            }

            if (i != (stackCount - 1))
            {
                addIndices(k1 + 1, k2, k2 + 1); // k1+1---k2---k2+1
            }
            else if ( j != sectorCount)
            {
                addIndices(k1 + 1, k2, k2 + 1); // k1+1---k2---k2+1
            }

            // vertical lines for all stacks
            lineIndices.push_back(k1);
            lineIndices.push_back(k2);
            if (i != 0)  // horizontal lines except 1st stack
            {
                lineIndices.push_back(k1);
                lineIndices.push_back(k1 + 1);
            }
        }
    }

    // generate interleaved vertex array
    buildInterleavedVertices();
}



///////////////////////////////////////////////////////////////////////////////
// generate interleaved vertices: V/N/T
// stride must be 32 bytes
///////////////////////////////////////////////////////////////////////////////
void SuperSphere::buildInterleavedVertices()
{
    std::vector<float>().swap(interleavedVertices);

    std::size_t i, j;
    std::size_t count = vertices.size();
    for (i = 0, j = 0; i < count; i += 3, j += 2)
    {
        interleavedVertices.push_back(vertices[i]);
        interleavedVertices.push_back(vertices[i + 1]);
        interleavedVertices.push_back(vertices[i + 2]);

        interleavedVertices.push_back(normals[i]);
        interleavedVertices.push_back(normals[i + 1]);
        interleavedVertices.push_back(normals[i + 2]);

        interleavedVertices.push_back(texCoords[j]);
        interleavedVertices.push_back(texCoords[j + 1]);
    }
}

///////////////////////////////////////////////////////////////////////////////
// add single vertex to array
///////////////////////////////////////////////////////////////////////////////
void SuperSphere::addVertex(float x, float y, float z)
{
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
}



///////////////////////////////////////////////////////////////////////////////
// add single normal to array
///////////////////////////////////////////////////////////////////////////////
void SuperSphere::addNormal(float nx, float ny, float nz)
{
    normals.push_back(nx);
    normals.push_back(ny);
    normals.push_back(nz);
}



///////////////////////////////////////////////////////////////////////////////
// add single texture coord to array
///////////////////////////////////////////////////////////////////////////////
void SuperSphere::addTexCoord(float s, float t)
{
    texCoords.push_back(s);
    texCoords.push_back(t);
}



///////////////////////////////////////////////////////////////////////////////
// add 3 indices to array
///////////////////////////////////////////////////////////////////////////////
void SuperSphere::addIndices(unsigned int i1, unsigned int i2, unsigned int i3)
{
    indices.push_back(i1);
    indices.push_back(i2);
    indices.push_back(i3);
}



///////////////////////////////////////////////////////////////////////////////
// return face normal of a triangle v1-v2-v3
// if a triangle has no surface (normal length = 0), then return a zero vector
///////////////////////////////////////////////////////////////////////////////
std::vector<float> SuperSphere::computeFaceNormal(float x1, float y1, float z1,  // v1
    float x2, float y2, float z2,  // v2
    float x3, float y3, float z3)  // v3
{
    const float EPSILON = 0.000001f;

    std::vector<float> normal(3, 0.0f);     // default return value (0,0,0)
    float nx, ny, nz;

    // find 2 edge vectors: v1-v2, v1-v3
    float ex1 = x2 - x1;
    float ey1 = y2 - y1;
    float ez1 = z2 - z1;
    float ex2 = x3 - x1;
    float ey2 = y3 - y1;
    float ez2 = z3 - z1;

    // cross product: e1 x e2
    nx = ey1 * ez2 - ez1 * ey2;
    ny = ez1 * ex2 - ex1 * ez2;
    nz = ex1 * ey2 - ey1 * ex2;

    // normalize only if the length is > 0
    float length = sqrtf(nx * nx + ny * ny + nz * nz);
    if (length > EPSILON)
    {
        // normalize
        float lengthInv = 1.0f / length;
        normal[0] = nx * lengthInv;
        normal[1] = ny * lengthInv;
        normal[2] = nz * lengthInv;
    }

    return normal;
}
