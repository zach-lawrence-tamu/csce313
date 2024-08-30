#include <iostream>
#include <string.h>

struct Point {
    int x, y;

    Point () : x(), y() {}
    Point (int _x, int _y) : x(_x), y(_y) {}
};

class Shape
{
    public:
    int vertices;
    Point** points;

    Shape (int _vertices) {
        vertices = _vertices;
        points = new Point*[vertices];
        for (int i = 0; i < vertices; i++)
	{
            points[i] = new Point();
        }
    }

    ~Shape () 
    {
	for (int i = 0; i < vertices; i++)
	{
	    delete points[i];
	}
        delete[] points;
    }

    void addPoints (/* formal parameter for unsized array called pts */ Point pts[]) {
        for (int i = 0; i < vertices; i++)
	{
	    memcpy(points[i], &(pts[i]), sizeof(Point));
        }
    }

    double* area (double* a) {
        int temp = 0;
        for (int i = 0; i < vertices - 1; i++) {
            // FIXME: there are two methods to access members of pointers
            //        use one to fix lhs and the other to fix rhs
              	    
            int lhs = points[i]->x * points[i+1]->y;
            int rhs = (*points[i+1]).x * (*points[i]).y;
	    //int lhs = points[i]->x;
	    //int rhs = (*points[i]).x;
            temp += (lhs - rhs);
            //std::cout << i << ": " << points[i]->x << (*points[i]).y << std::endl;
        }
        *a = abs(temp)/2.0;
        return &(*a);
    }
};

int main ()
{
    // FIXME: create the following points using the three different methods
    //        of defining structs:
    //          tri1 = (0, 0)
    //          tri2 = (1, 2)
    //          tri3 = (2, 0)
    Point tri1(0,0);
    Point tri2(1,2);
    Point tri3(2,0);

    // adding points to tri
    Point triPts[3] = {tri1, tri2, tri3};
    Shape tri(3);
    tri.addPoints(triPts);

    // FIXME: create the following points using your preferred struct
    //        definition:
    //          quad1 = (0, 0)
    //          quad2 = (0, 2)
    //          quad3 = (2, 2)
    //          quad4 = (2, 0)
    Point quad1(0, 0);
    Point quad2(0, 2);
    Point quad3(2, 2);
    Point quad4(2, 0);

    // adding points to quad
    Point quadPts[4] = {quad1, quad2, quad3, quad4};
    Shape quad(4);
    quad.addPoints(quadPts);

    // FIXME: print out area of tri and area of quad
    //std::cout << *tri->area() << std::endl;
    //std::cout << *quad->area() << std::endl;
    double* area = new double;
    *area = 0;
    std::cout << *tri.area(area) << std::endl;
    std::cout << *quad.area(area) << std::endl;
    delete area;
}
