#include "narrowPhase.h"
#include <assert.h>
#include <cstdlib>
#include <cstdio>

#include <coreModule/ecs/components/physicsComponent.h>

using namespace IKIGAI;
using namespace IKIGAI::PHYSICS;
using namespace IKIGAI::MATH;

void CollisionPrimitive::calculateInternals()
{
    transform = body->getTransform() * offset;
}

bool IntersectionTests::sphereAndHalfSpace(
    const CollisionSphere &sphere,
    const CollisionPlane &plane)
{
    // Find the distance from the origin
    float ballDistance =
        plane.direction.dot(sphere.getAxis(3)) - sphere.radius;

    // Check for the intersection
    return ballDistance <= plane.offset;
}

bool IntersectionTests::sphereAndSphere(
    const CollisionSphere &one,
    const CollisionSphere &two)
{
    // Find the vector between the objects
    Vector3f midline = one.getAxis(3) - two.getAxis(3);

    // See if it is large enough.
    return midline.squareMagnitude() <
        (one.radius+two.radius)*(one.radius+two.radius);
}

static inline float transformToAxis(
    const CollisionBox &box,
    const Vector3f &axis
    )
{
    return
        box.halfSize.x * abs(axis.dot(box.getAxis(0))) +
        box.halfSize.y * abs(axis.dot(box.getAxis(1))) +
        box.halfSize.z * abs(axis.dot(box.getAxis(2)));
}

static inline bool overlapOnAxis(
    const CollisionBox &one,
    const CollisionBox &two,
    const Vector3f &axis,
    const Vector3f &toCentre
    )
{
    // Project the half-size of one onto axis
    float oneProject = transformToAxis(one, axis);
    float twoProject = transformToAxis(two, axis);

    // Project this onto the axis
    float distance = abs(toCentre.dot(axis));

    // Check for overlap
    return (distance < oneProject + twoProject);
}

// This preprocessor definition is only used as a convenience
// in the boxAndBox intersection  method.
#define TEST_OVERLAP(axis) overlapOnAxis(one, two, (axis), toCentre)

bool IntersectionTests::boxAndBox(
    const CollisionBox &one,
    const CollisionBox &two
    )
{
    // Find the vector between the two centres
    Vector3 toCentre = two.getAxis(3) - one.getAxis(3);

    return (
        // Check on box one's axes first
        TEST_OVERLAP(one.getAxis(0)) &&
        TEST_OVERLAP(one.getAxis(1)) &&
        TEST_OVERLAP(one.getAxis(2)) &&

        // And on two's
        TEST_OVERLAP(two.getAxis(0)) &&
        TEST_OVERLAP(two.getAxis(1)) &&
        TEST_OVERLAP(two.getAxis(2)) &&

        // Now on the cross products
        TEST_OVERLAP(one.getAxis(0).cross(two.getAxis(0))) &&
        TEST_OVERLAP(one.getAxis(0).cross(two.getAxis(1))) &&
        TEST_OVERLAP(one.getAxis(0).cross(two.getAxis(2))) &&
        TEST_OVERLAP(one.getAxis(1).cross(two.getAxis(0))) &&
        TEST_OVERLAP(one.getAxis(1).cross(two.getAxis(1))) &&
        TEST_OVERLAP(one.getAxis(1).cross(two.getAxis(2))) &&
        TEST_OVERLAP(one.getAxis(2).cross(two.getAxis(0))) &&
        TEST_OVERLAP(one.getAxis(2).cross(two.getAxis(1))) &&
        TEST_OVERLAP(one.getAxis(2).cross(two.getAxis(2)))
    );
}
#undef TEST_OVERLAP

bool IntersectionTests::boxAndHalfSpace(
    const CollisionBox &box,
    const CollisionPlane &plane
    )
{
    // Work out the projected radius of the box onto the plane direction
    float projectedRadius = transformToAxis(box, plane.direction);

    // Work out how far the box is from the origin
    float boxDistance =
        plane.direction.dot(box.getAxis(3)) - projectedRadius;

    // Check for the intersection
    return boxDistance <= plane.offset;
}

unsigned CollisionDetector::sphereAndTruePlane(
    const CollisionSphere &sphere,
    const CollisionPlane &plane,
    CollisionData *data
    )
{
    // Make sure we have contacts
    if (data->contactsLeft <= 0) return 0;

    // Cache the sphere position
    Vector3 position = sphere.getAxis(3);

    // Find the distance from the plane
    float centreDistance = plane.direction.dot(position) - plane.offset;

    // Check if we're within radius
    if (centreDistance*centreDistance > sphere.radius*sphere.radius)
    {
        return 0;
    }

    // Check which side of the plane we're on
    Vector3 normal = plane.direction;
    float penetration = -centreDistance;
    if (centreDistance < 0)
    {
        normal *= -1;
        penetration = -penetration;
    }
    penetration += sphere.radius;

    // Create the contact - it has a normal in the plane direction.
    Contact* contact = data->contacts;
    contact->contactNormal = normal;
    contact->penetration = penetration;
    contact->contactPoint = position - plane.direction * centreDistance;
    contact->setBodyData(sphere.body, NULL,
        data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}

unsigned CollisionDetector::sphereAndHalfSpace(
    const CollisionSphere &sphere,
    const CollisionPlane &plane,
    CollisionData *data
    )
{
    // Make sure we have contacts
    if (data->contactsLeft <= 0) return 0;

    // Cache the sphere position
    Vector3 position = sphere.getAxis(3);

    // Find the distance from the plane
    float ballDistance =
        plane.direction.dot(position) -
        sphere.radius - plane.offset;

    if (ballDistance >= 0) return 0;

    // Create the contact - it has a normal in the plane direction.
    Contact* contact = data->contacts;
    contact->contactNormal = plane.direction;
    contact->penetration = -ballDistance;
    contact->contactPoint =
        position - plane.direction * (ballDistance + sphere.radius);
    contact->setBodyData(sphere.body, NULL,
        data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}

unsigned CollisionDetector::sphereAndSphere(
    const CollisionSphere &one,
    const CollisionSphere &two,
    CollisionData *data
    )
{
    // Make sure we have contacts
    if (data->contactsLeft <= 0) return 0;

    // Cache the sphere positions
    Vector3 positionOne = one.getAxis(3);
    Vector3 positionTwo = two.getAxis(3);

    // Find the vector between the objects
    Vector3 midline = positionOne - positionTwo;
    float size = midline.magnitude();

    // See if it is large enough.
    if (size <= 0.0f || size >= one.radius+two.radius)
    {
        return 0;
    }

    // We manually create the normal, because we have the
    // size to hand.
    Vector3 normal = midline * (((float)1.0)/size);

    Contact* contact = data->contacts;
    contact->contactNormal = normal;
    contact->contactPoint = positionOne + midline * (float)0.5;
    contact->penetration = (one.radius+two.radius - size);
    contact->setBodyData(one.body, two.body,
        data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}

static inline float penetrationOnAxis(
    const CollisionBox &one,
    const CollisionBox &two,
    const Vector3f &axis,
    const Vector3f &toCentre
    )
{
    // Project the half-size of one onto axis
    float oneProject = transformToAxis(one, axis);
    float twoProject = transformToAxis(two, axis);

    // Project this onto the axis
    float distance = abs(toCentre.dot(axis));

    // Return the overlap (i.e. positive indicates
    // overlap, negative indicates separation).
    return oneProject + twoProject - distance;
}


static inline bool tryAxis(
    const CollisionBox &one,
    const CollisionBox &two,
    Vector3f axis,
    const Vector3f& toCentre,
    unsigned index,

    // These values may be updated
    float& smallestPenetration,
    unsigned &smallestCase
    )
{
    // Make sure we have a normalized axis, and don't check almost parallel axes
    if (axis.squareMagnitude() < 0.0001) return true;
    axis.normalise();

    float penetration = penetrationOnAxis(one, two, axis, toCentre);

    if (penetration < 0) return false;
    if (penetration < smallestPenetration) {
        smallestPenetration = penetration;
        smallestCase = index;
    }
    return true;
}

void fillPointFaceBoxBox(
    const CollisionBox &one,
    const CollisionBox &two,
    const Vector3f &toCentre,
    CollisionData *data,
    unsigned best,
    float pen
    )
{
    // This method is called when we know that a vertex from
    // box two is in contact with box one.

    Contact* contact = data->contacts;

    // We know which axis the collision is on (i.e. best),
    // but we need to work out which of the two faces on
    // this axis.
    Vector3f normal = one.getAxis(best);
    if (one.getAxis(best).dot(toCentre) > 0)
    {
        normal = normal * -1.0f;
    }

    // Work out which vertex of box two we're colliding with.
    // Using toCentre doesn't work!
    Vector3f vertex = two.halfSize;
    if (two.getAxis(0).dot(normal) < 0) vertex.x = -vertex.x;
    if (two.getAxis(1).dot(normal) < 0) vertex.y = -vertex.y;
    if (two.getAxis(2).dot(normal) < 0) vertex.z = -vertex.z;

    // Create the contact data
    contact->contactNormal = normal;
    contact->penetration = pen;
    auto v = MATH::Vector3f{vertex.x, vertex.y, vertex.z};
    auto res = two.getTransform() * v;
    contact->contactPoint = {(float)res.x, (float)res.y, (float)res.z};
    contact->setBodyData(one.body, two.body,
        data->friction, data->restitution);
}

static inline Vector3f contactPoint(
    const Vector3f &pOne,
    const Vector3f &dOne,
    float oneSize,
    const Vector3f &pTwo,
    const Vector3f &dTwo,
    float twoSize,

    // If this is true, and the contact point is outside
    // the edge (in the case of an edge-face contact) then
    // we use one's midpoint, otherwise we use two's.
    bool useOne)
{
    Vector3f toSt, cOne, cTwo;
    float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
    float denom, mua, mub;

    smOne = dOne.squareMagnitude();
    smTwo = dTwo.squareMagnitude();
    dpOneTwo = dTwo.dot(dOne);

    toSt = pOne - pTwo;
    dpStaOne = dOne.dot(toSt);
    dpStaTwo = dTwo.dot(toSt);

    denom = smOne * smTwo - dpOneTwo * dpOneTwo;

    // Zero denominator indicates parrallel lines
    if (abs(denom) < 0.0001f) {
        return useOne?pOne:pTwo;
    }

    mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
    mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

    // If either of the edges has the nearest point out
    // of bounds, then the edges aren't crossed, we have
    // an edge-face contact. Our point is on the edge, which
    // we know from the useOne parameter.
    if (mua > oneSize ||
        mua < -oneSize ||
        mub > twoSize ||
        mub < -twoSize)
    {
        return useOne?pOne:pTwo;
    }
    else
    {
        cOne = pOne + dOne * mua;
        cTwo = pTwo + dTwo * mub;

        return cOne * 0.5 + cTwo * 0.5;
    }
}

// This preprocessor definition is only used as a convenience
// in the boxAndBox contact generation method.
#define CHECK_OVERLAP(axis, index) \
    if (!tryAxis(one, two, (axis), toCentre, (index), pen, best)) return 0;

unsigned CollisionDetector::boxAndBox(
    const CollisionBox &one,
    const CollisionBox &two,
    CollisionData *data
    )
{
    //if (!IntersectionTests::boxAndBox(one, two)) return 0;

    // Find the vector between the two centres
    Vector3f toCentre = two.getAxis(3) - one.getAxis(3);

    // We start assuming there is no contact
    float pen = std::numeric_limits<float>::max();
    unsigned best = 0xffffff;

    // Now we check each axes, returning if it gives us
    // a separating axis, and keeping track of the axis with
    // the smallest penetration otherwise.
    CHECK_OVERLAP(one.getAxis(0), 0);
    CHECK_OVERLAP(one.getAxis(1), 1);
    CHECK_OVERLAP(one.getAxis(2), 2);

    CHECK_OVERLAP(two.getAxis(0), 3);
    CHECK_OVERLAP(two.getAxis(1), 4);
    CHECK_OVERLAP(two.getAxis(2), 5);

    // Store the best axis-major, in case we run into almost
    // parallel edge collisions later
    unsigned bestSingleAxis = best;

    CHECK_OVERLAP(one.getAxis(0).cross(two.getAxis(0)), 6);
    CHECK_OVERLAP(one.getAxis(0).cross(two.getAxis(1)), 7);
    CHECK_OVERLAP(one.getAxis(0).cross(two.getAxis(2)), 8);
    CHECK_OVERLAP(one.getAxis(1).cross(two.getAxis(0)), 9);
    CHECK_OVERLAP(one.getAxis(1).cross(two.getAxis(1)), 10);
    CHECK_OVERLAP(one.getAxis(1).cross(two.getAxis(2)), 11);
    CHECK_OVERLAP(one.getAxis(2).cross(two.getAxis(0)), 12);
    CHECK_OVERLAP(one.getAxis(2).cross(two.getAxis(1)), 13);
    CHECK_OVERLAP(one.getAxis(2).cross(two.getAxis(2)), 14);

    // Make sure we've got a result.
    assert(best != 0xffffff);

    // We now know there's a collision, and we know which
    // of the axes gave the smallest penetration. We now
    // can deal with it in different ways depending on
    // the case.
    if (best < 3)
    {
        // We've got a vertex of box two on a face of box one.
        fillPointFaceBoxBox(one, two, toCentre, data, best, pen);
        data->addContacts(1);
        return 1;
    }
    else if (best < 6)
    {
        // We've got a vertex of box one on a face of box two.
        // We use the same algorithm as above, but swap around
        // one and two (and therefore also the vector between their
        // centres).
        fillPointFaceBoxBox(two, one, toCentre*-1.0f, data, best-3, pen);
        data->addContacts(1);
        return 1;
    }
    else
    {
        // We've got an edge-edge contact. Find out which axes
        best -= 6;
        unsigned oneAxisIndex = best / 3;
        unsigned twoAxisIndex = best % 3;
        Vector3f oneAxis = one.getAxis(oneAxisIndex);
        Vector3f twoAxis = two.getAxis(twoAxisIndex);
        Vector3f axis = oneAxis.cross(twoAxis);
        axis.normalise();

        // The axis should point from box one to box two.
        if (axis.dot(toCentre) > 0) axis = axis * -1.0f;
        
        Vector3f ptOnOneEdge = one.halfSize;
        Vector3f ptOnTwoEdge = two.halfSize;
        for (unsigned i = 0; i < 3; i++)
        {
            if (i == oneAxisIndex) ptOnOneEdge[i] = 0;
            else if (one.getAxis(i).dot(axis) > 0) ptOnOneEdge[i] = -ptOnOneEdge[i];

            if (i == twoAxisIndex) ptOnTwoEdge[i] = 0;
            else if (two.getAxis(i).dot(axis) < 0) ptOnTwoEdge[i] = -ptOnTwoEdge[i];
        }

        // Move them into world coordinates (they are already oriented
        // correctly, since they have been derived from the axes).
        auto res = one.transform * MATH::Vector3f{ptOnOneEdge.x, ptOnOneEdge.y, ptOnOneEdge.z};
        ptOnOneEdge = {(float)res.x, (float)res.y, (float)res.z};
        res = two.transform * MATH::Vector3f{ptOnTwoEdge.x, ptOnTwoEdge.y, ptOnTwoEdge.z};
        ptOnTwoEdge = {(float)res.x, (float)res.y, (float)res.z};

        // So we have a point and a direction for the colliding edges.
        // We need to find out point of closest approach of the two
        // line-segments.
        Vector3f vertex = contactPoint(
            ptOnOneEdge, oneAxis, one.halfSize[oneAxisIndex],
            ptOnTwoEdge, twoAxis, two.halfSize[twoAxisIndex],
            bestSingleAxis > 2
            );

        // We can fill the contact.
        Contact* contact = data->contacts;

        contact->penetration = pen;
        contact->contactNormal = axis;
        contact->contactPoint = vertex;
        contact->setBodyData(one.body, two.body,
            data->friction, data->restitution);
        data->addContacts(1);
        return 1;
    }
    return 0;
}
#undef CHECK_OVERLAP




unsigned CollisionDetector::boxAndPoint(
    const CollisionBox &box,
    const Vector3f &point,
    CollisionData *data
    )
{
    // Transform the point into box coordinates
    auto res = box.transform.transformInverse(MATH::Vector3f{point.x, point.y, point.z});
    Vector3f relPt = {(float)res.x, (float)res.y, (float)res.z};

    Vector3f normal;

    // Check each axis, looking for the axis on which the
    // penetration is least deep.
    float min_depth = box.halfSize.x - abs(relPt.x);
    if (min_depth < 0) return 0;
    normal = box.getAxis(0) * ((relPt.x < 0)?-1:1);

    float depth = box.halfSize.y - abs(relPt.y);
    if (depth < 0) return 0;
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = box.getAxis(1) * ((relPt.y < 0)?-1:1);
    }

    depth = box.halfSize.z - abs(relPt.z);
    if (depth < 0) return 0;
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = box.getAxis(2) * ((relPt.z < 0)?-1:1);
    }

    // Compile the contact
    Contact* contact = data->contacts;
    contact->contactNormal = normal;
    contact->contactPoint = point;
    contact->penetration = min_depth;

    // Note that we don't know what rigid body the point
    // belongs to, so we just use NULL. Where this is called
    // this value can be left, or filled in.
    contact->setBodyData(box.body, NULL,
        data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}

unsigned CollisionDetector::boxAndSphere(
    const CollisionBox &box,
    const CollisionSphere &sphere,
    CollisionData *data
    )
{
    // Transform the centre of the sphere into box coordinates
    Vector3f centre = sphere.getAxis(3);
    auto res = box.transform.transformInverse(MATH::Vector3f{centre.x, centre.y, centre.z});
    Vector3f relCentre = {(float)res.x, (float)res.y,(float)res.z};

    // Early out check to see if we can exclude the contact
    if (abs(relCentre.x) - sphere.radius > box.halfSize.x ||
        abs(relCentre.y) - sphere.radius > box.halfSize.y ||
        abs(relCentre.z) - sphere.radius > box.halfSize.z)
    {
        return 0;
    }

    Vector3f closestPt(0,0,0);
    float dist;

    // Clamp each coordinate to the box.
    dist = relCentre.x;
    if (dist > box.halfSize.x) dist = box.halfSize.x;
    if (dist < -box.halfSize.x) dist = -box.halfSize.x;
    closestPt.x = dist;

    dist = relCentre.y;
    if (dist > box.halfSize.y) dist = box.halfSize.y;
    if (dist < -box.halfSize.y) dist = -box.halfSize.y;
    closestPt.y = dist;

    dist = relCentre.z;
    if (dist > box.halfSize.z) dist = box.halfSize.z;
    if (dist < -box.halfSize.z) dist = -box.halfSize.z;
    closestPt.z = dist;

    // Check we're in contact
    dist = (closestPt - relCentre).squareMagnitude();
    if (dist > sphere.radius * sphere.radius) return 0;

    // Compile the contact
    res = box.transform.transform(MATH::Vector3f{closestPt.x, closestPt.y, closestPt.z});
    Vector3f closestPtWorld = {(float)res.x, (float)res.y, (float)res.z};

    Contact* contact = data->contacts;
    contact->contactNormal = (closestPtWorld - centre);
    contact->contactNormal.normalise();
    contact->contactPoint = closestPtWorld;
    contact->penetration = sphere.radius - sqrt(dist);
    contact->setBodyData(box.body, sphere.body,
        data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}

unsigned CollisionDetector::boxAndHalfSpace(
    const CollisionBox &box,
    const CollisionPlane &plane,
    CollisionData *data
    )
{
    // Make sure we have contacts
    if (data->contactsLeft <= 0) return 0;

    // Check for intersection
    if (!IntersectionTests::boxAndHalfSpace(box, plane))
    {
        return 0;
    }

    // We have an intersection, so find the intersection points. We can make
    // do with only checking vertices. If the box is resting on a plane
    // or on an edge, it will be reported as four or two contact points.

    // Go through each combination of + and - for each half-size
    static float mults[8][3] = {{1,1,1},{-1,1,1},{1,-1,1},{-1,-1,1},
                               {1,1,-1},{-1,1,-1},{1,-1,-1},{-1,-1,-1}};

    Contact* contact = data->contacts;
    unsigned contactsUsed = 0;
    for (unsigned i = 0; i < 8; i++) {

        // Calculate the position of each vertex
        Vector3 vertexPos(mults[i][0], mults[i][1], mults[i][2]);
        vertexPos.componentProductUpdate(box.halfSize);
        auto res = box.transform.transform(MATH::Vector3f{vertexPos.x, vertexPos.y, vertexPos.z});
        vertexPos = {(float)res.x, (float)res.y, (float)res.z};

        // Calculate the distance from the plane
        float vertexDistance = vertexPos.dot(plane.direction);

        // Compare this to the plane's distance
        if (vertexDistance <= plane.offset)
        {
            // Create the contact data.

            // The contact point is halfway between the vertex and the
            // plane - we multiply the direction by half the separation
            // distance and add the vertex location.
            contact->contactPoint = plane.direction;
            contact->contactPoint *= (vertexDistance-plane.offset);
            contact->contactPoint += vertexPos;
            contact->contactNormal = plane.direction;
            contact->penetration = plane.offset - vertexDistance;

            // Write the appropriate data
            contact->setBodyData(box.body, NULL,
                data->friction, data->restitution);

            // Move onto the next contact
            contact++;
            contactsUsed++;
            if (contactsUsed == (unsigned)data->contactsLeft) return contactsUsed;
        }
    }

    data->addContacts(contactsUsed);
    return contactsUsed;
}
