#ifndef AGL_PHYSICS_COLLISIONSHAPE2D
#define AGL_PHYSICS_COLLISIONSHAPE2D

enum CollisionShapeType{

    CS_CIRCLE,
    CS_SEGMENT,
    CS_POLYGON
};

class CollisionShape2D{

    public:

        CollisionShape2D();
        ~CollisionShape2D();

    public:

        CollisionShapeType type;
};

#endif