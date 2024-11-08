#include <Physics/polygon2d.hpp>

#include <vector>
#include <algorithm>
#include <limits>

static void swap(int &a, int &b) {
    int c;
    c = a;
    a = b;
    b = c;
}

static float min(const float &a, const float &b) {
    return a < b ? a : b;
}

static bool eq(const float &a, const float &b) {
    return abs(a - b) <= 1e-8;
}

static int wrap(const int &a, const int &b) {
    return a < 0 ? a % b + b : a % b;
}

static float srand(const float &min = 0, const float &max = 1) {
    return rand() / (float) RAND_MAX * (max - min) + min;
}

static float area(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) {
    return (((b.x - a.x)*(c.y - a.y))-((c.x - a.x)*(b.y - a.y)));
}

static bool left(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) {
    return area(a, b, c) > 0;
}

static bool leftOn(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) {
    return area(a, b, c) >= 0;
}

static bool right(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) {
    return area(a, b, c) < 0;
}

static bool rightOn(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) {
    return area(a, b, c) <= 0;
}

static bool collinear(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) {
    return area(a, b, c) == 0;
}

static float sqdist(const glm::vec2 &a, const glm::vec2 &b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return dx * dx + dy * dy;
}

static glm::vec2 intersection(const glm::vec2 &p1, const glm::vec2 &p2, const glm::vec2 &q1, const glm::vec2 &q2) {
    glm::vec2 i;
    float a1, b1, c1, a2, b2, c2, det;
    a1 = p2.y - p1.y;
    b1 = p1.x - p2.x;
    c1 = a1 * p1.x + b1 * p1.y;
    a2 = q2.y - q1.y;
    b2 = q1.x - q2.x;
    c2 = a2 * q1.x + b2 * q1.y;
    det = a1 * b2 - a2*b1;
    if (!eq(det, 0)) { // lines are not parallel
        i.x = (b2 * c1 - b1 * c2) / det;
        i.y = (a1 * c2 - a2 * c1) / det;
    }
    return i;
}

glm::vec2& Polygon2D::At(int i){
    return m_points[wrap(i, m_points.size())];
}

bool Polygon2D::IsReflex(int i){

    return right(At(i - 1), At(i), At(i + 1));
}

void Polygon2D::MakeCCW(){

    int br = 0;
    // find bottom right point
    for (int i = 1; i < m_points.size(); ++i) {
        if (m_points[i].y < m_points[br].y || (m_points[i].y == m_points[br].y && m_points[i].x > m_points[br].x)) {
            br = i;
        }
    }

    // reverse poly if clockwise
    if (!left(At(br-1), At(br), At(br+1))) {
        std::reverse(m_points.begin(), m_points.end());
    }
}

void DecomposeHelper(Polygon2D &poly, std::vector<Polygon2D> &polys, std::vector<glm::vec2> &steinerPoints, std::vector<glm::vec2> &reflexVertices);

void Decompose(Polygon2D &polygon, std::vector<Polygon2D> &subPolys){

    Polygon2D poly = polygon;
    poly.MakeCCW();
    std::vector<glm::vec2> steinerPoints, reflexVertices;
    DecomposeHelper(poly, subPolys, steinerPoints, reflexVertices);
}

void DecomposeHelper(Polygon2D &poly, std::vector<Polygon2D> &polys, std::vector<glm::vec2> &steinerPoints, std::vector<glm::vec2> &reflexVertices){

    glm::vec2 upperInt, lowerInt, p, closestVert;
    float upperDist, lowerDist, d, closestDist;
    int upperIndex, lowerIndex, closestIndex;

    Polygon2D lowerPoly, upperPoly;

    for (int i = 0; i < poly.size(); ++i) {

        if (poly.IsReflex(i)) {

            reflexVertices.push_back(poly.At(i));
            upperDist = lowerDist = std::numeric_limits<float>::max();

            for (int j = 0; j < poly.size(); ++j) {

                if ( left(poly.At(i-1), poly.At(i), poly.At(j)) && rightOn(poly.At(i-1), poly.At(i), poly.At(j-1))) { // if line intersects with an edge

                    p = intersection(poly.At(i-1), poly.At(i), poly.At(j), poly.At(j-1)); // find the point of intersection

                    if (right(poly.At(i+1), poly.At(i), p)) { // make sure it's inside the poly
                        d = sqdist(poly.At(i), p);
                        if (d < lowerDist) { // keep only the closest intersection
                            lowerDist = d;
                            lowerInt = p;
                            lowerIndex = j;
                        }
                    }
                }

                if (left(poly.At(i+1), poly.At(i), poly.At(j+1)) && rightOn(poly.At(i+1), poly.At(i), poly.At(j))) {

                    p = intersection(poly.At(i+1), poly.At(i), poly.At(j), poly.At(j+1));

                    if (left(poly.At(i-1), poly.At(i), p)) {

                        d = sqdist(poly.At(i), p);

                        if (d < upperDist) {
                            upperDist = d;
                            upperInt = p;
                            upperIndex = j;
                        }
                    }
                }
            }

            // if there are no vertices to connect to, choose a point in the middle
            if (lowerIndex == (upperIndex + 1) % poly.size()) {

                p.x = (lowerInt.x + upperInt.x) / 2;
                p.y = (lowerInt.y + upperInt.y) / 2;
                steinerPoints.push_back(p);

                if (i < upperIndex) {

                    lowerPoly.m_points.insert(lowerPoly.m_points.end(), poly.m_points.begin() + i, poly.m_points.begin() + upperIndex + 1);
                    lowerPoly.m_points.push_back(p);
                    upperPoly.m_points.push_back(p);

                    if (lowerIndex != 0)
                        upperPoly.m_points.insert(upperPoly.m_points.end(), poly.m_points.begin() + lowerIndex, poly.m_points.end());
                    upperPoly.m_points.insert(upperPoly.m_points.end(), poly.m_points.begin(), poly.m_points.begin() + i + 1);

                }else {

                    if (i != 0) lowerPoly.m_points.insert(lowerPoly.m_points.end(), poly.m_points.begin() + i, poly.m_points.end());
                    lowerPoly.m_points.insert(lowerPoly.m_points.end(), poly.m_points.begin(), poly.m_points.begin() + upperIndex + 1);
                    lowerPoly.m_points.push_back(p);
                    upperPoly.m_points.push_back(p);
                    upperPoly.m_points.insert(upperPoly.m_points.end(), poly.m_points.begin() + lowerIndex, poly.m_points.begin() + i + 1);
                }

            }else {

                if (lowerIndex > upperIndex) {
                    upperIndex += poly.size();
                }
                closestDist = std::numeric_limits<float>::max();

                for (int j = lowerIndex; j <= upperIndex; ++j) {
                    if (leftOn(poly.At(i-1), poly.At(i), poly.At(j))
                            && rightOn(poly.At(i+1), poly.At(i), poly.At(j))) {
                        d = sqdist(poly.At(i), poly.At(j));
                        if (d < closestDist) {
                            closestDist = d;
                            closestVert = poly.At(j);
                            closestIndex = j % poly.size();
                        }
                    }
                }

                if (i < closestIndex) {
                    lowerPoly.m_points.insert(lowerPoly.m_points.end(), poly.m_points.begin() + i, poly.m_points.begin() + closestIndex + 1);
                    if (closestIndex != 0) upperPoly.m_points.insert(upperPoly.m_points.end(), poly.m_points.begin() + closestIndex, poly.m_points.end());
                    upperPoly.m_points.insert(upperPoly.m_points.end(), poly.m_points.begin(), poly.m_points.begin() + i + 1);
                } else {
                    if (i != 0) lowerPoly.m_points.insert(lowerPoly.m_points.end(), poly.m_points.begin() + i, poly.m_points.end());
                    lowerPoly.m_points.insert(lowerPoly.m_points.end(), poly.m_points.begin(), poly.m_points.begin() + closestIndex + 1);
                    upperPoly.m_points.insert(upperPoly.m_points.end(), poly.m_points.begin() + closestIndex, poly.m_points.begin() + i + 1);
                }
            }

            // solve smallest poly first
            if (lowerPoly.size() < upperPoly.size()) {
                Decompose(lowerPoly, polys);
                Decompose(upperPoly, polys);
            } else {
                Decompose(upperPoly, polys);
                Decompose(lowerPoly, polys);
            }
            return;
        }
    }
    polys.push_back(poly);
}