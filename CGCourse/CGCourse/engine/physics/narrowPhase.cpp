
#include "narrowPhase.h"
#include <memory.h>
//#include <assert.h>

#include "body.h"
#include "contacts.h"
#include "../ecs/components/physicsComponent.h"

using namespace KUMA;
using namespace KUMA::PHYSICS;
using namespace KUMA::MATHGL;

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
        plane.direction.dot(sphere.getAxis(3)) -
        sphere.radius;

    // Check for the intersection
    return ballDistance <= plane.offset;
}

bool IntersectionTests::sphereAndSphere(
    const CollisionSphere &one,
    const CollisionSphere &two)
{
    // Find the vector between the objects
    Vector3 midline = one.getAxis(3) - two.getAxis(3);

    // See if it is large enough.
    return midline.squareMagnitude() <
        (one.radius+two.radius)*(one.radius+two.radius);
}

static inline float transformToAxis(
    const CollisionBox &box,
    const Vector3 &axis
    )
{
    return
        box.halfSize.x * fabs(axis.dot(box.getAxis(0))) +
        box.halfSize.y * fabs(axis.dot(box.getAxis(1))) +
        box.halfSize.z * fabs(axis.dot(box.getAxis(2)));
}

/**
 * This function checks if the two boxes overlap
 * along the given axis. The final parameter toCentre
 * is used to pass in the vector between the boxes centre
 * points, to avoid having to recalculate it each time.
 */
static inline bool overlapOnAxis(
    const CollisionBox &one,
    const CollisionBox &two,
    const Vector3 &axis,
    const Vector3 &toCentre
    )
{
    // Project the half-size of one onto axis
    float oneProject = transformToAxis(one, axis);
    float twoProject = transformToAxis(two, axis);

    // Project this onto the axis
    float distance = fabs(toCentre.dot(axis));

    // Check for overlap
    return (distance < oneProject + twoProject);
}

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
        plane.direction.dot(box.getAxis(3)) -
        projectedRadius;

    // Check for the intersection
    return boxDistance <= plane.offset;
}
/*
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
    auto& contact = data->contactArray.get()[data->curContactId];
    contact.contactNormal = normal;
    contact.penetration = penetration;
    contact.contactPoint = position - plane.direction * centreDistance;
    contact.setBodyData(sphere.body, nullptr, data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}
*/
/*
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

    auto& contact = data->contactArray.get()[data->curContactId];
    contact.contactNormal = plane.direction;
    contact.penetration = -ballDistance;
    contact.contactPoint =
        position - plane.direction * (ballDistance + sphere.radius);
    contact.setBodyData(sphere.body, nullptr, data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}
*/
/*
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

    auto& contact = data->contactArray.get()[data->curContactId];
    contact.contactNormal = normal;
    contact.contactPoint = positionOne + midline * (float)0.5;
    contact.penetration = (one.radius+two.radius - size);
    contact.setBodyData(one.body, two.body, data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}
*/
static inline float penetrationOnAxis(
    const CollisionBox &one,
    const CollisionBox &two,
    const Vector3 &axis,
    const Vector3 &toCentre
    )
{
    // Project the half-size of one onto axis
    float oneProject = transformToAxis(one, axis);
    float twoProject = transformToAxis(two, axis);

    // Project this onto the axis
    float distance = fabs(toCentre.dot(axis));
    
    return oneProject + twoProject - distance;
}


static inline bool tryAxis(
    const CollisionBox &one,
    const CollisionBox &two,
    Vector3 axis,
    const Vector3& toCentre,
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
    const CollisionBox& one,
    const CollisionBox& two,
    const Vector3 &toCentre,
    CollisionData *data,
    unsigned best,
    float pen
    )
{
    // This method is called when we know that a vertex from
    // box two is in contact with box one.

    auto* contact = data->m_contacts;

    // We know which axis the collision is on (i.e. best),
    // but we need to work out which of the two faces on
    // this axis.
    Vector3 normal = one.getAxis(best);
    if (one.getAxis(best).dot(toCentre) > 0) {
        normal = normal * -1.0f;
    }

    // Work out which vertex of box two we're colliding with.
    // Using toCentre doesn't work!
    Vector3 vertex = two.halfSize;
    if (two.getAxis(0).dot(normal) < 0) vertex.x = -vertex.x;
    if (two.getAxis(1).dot(normal) < 0) vertex.y = -vertex.y;
    if (two.getAxis(2).dot(normal) < 0) vertex.z = -vertex.z;

    // Create the contact data
    contact->contactNormal = normal;
    contact->penetration = pen;
    contact->contactPoint = two.getTransform() * vertex;
    contact->setBodyData(one.body, two.body, data->m_friction, data->m_restitution);
}


static Vector3 contactPoint(
    const Vector3 &pOne,
    const Vector3 &dOne,
    float oneSize,
    const Vector3 &pTwo,
    const Vector3 &dTwo,
    float twoSize,
    bool useOne) {
    Vector3 toSt, cOne, cTwo;
    float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
    float denom, mua, mub;

    smOne = dOne.squareMagnitude();
    smTwo = dTwo.squareMagnitude();
    dpOneTwo = dTwo.dot(dOne);

    toSt = pOne - pTwo;
    dpStaOne = dOne.dot(toSt);
    dpStaTwo = dTwo.dot(toSt);

    denom = smOne * smTwo - dpOneTwo * dpOneTwo;
    
    if (fabs(denom) < 0.0001f) {
        return useOne?pOne:pTwo;
    }

    mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
    mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

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
float TransformToAxis(std::shared_ptr<ECS::PhysicsComponent> box, Vector3& axis) {
    auto& halfSize = reinterpret_cast<KUMA::PHYSICS::CollisionBox*>(box->collider.get())->halfSize;

    return halfSize.x * fabsf(axis.dot(box->collider->getAxis(0))) +
        halfSize.y *    fabsf(axis.dot(box->collider->getAxis(1))) +
        halfSize.z *    fabsf(axis.dot(box->collider->getAxis(2)));
}
float CalcPenetrationOnAxis(std::shared_ptr<ECS::PhysicsComponent> lhs, std::shared_ptr<ECS::PhysicsComponent> rhs, Vector3& axis, Vector3& toCentre) {
    float lhsProject = TransformToAxis(lhs, axis);
    float rhsProject = TransformToAxis(rhs, axis);

    float distance = fabsf(toCentre.dot(axis));

    return lhsProject + rhsProject - distance;
}

bool TryAxis(std::shared_ptr<ECS::PhysicsComponent> lhs, std::shared_ptr<ECS::PhysicsComponent> rhs, Vector3& axis, 
    Vector3& toCentre, unsigned int index, float& smallestPenetration, unsigned int& smallestCase) {
    if (axis.LengthSqrt(axis) < 0.0001) {
        return true;
    }

    float penetration = CalcPenetrationOnAxis(lhs, rhs, axis, toCentre);

    if (penetration < 0) {
        return false;
    }
    if (penetration < smallestPenetration) {
        smallestPenetration = penetration;
        smallestCase = index;
    }

    return true;
}

void FillPointFaceBoxBox(std::shared_ptr<ECS::PhysicsComponent> lhs, std::shared_ptr<ECS::PhysicsComponent> rhs, 
    Vector3& toCentre, CollisionData* data, unsigned int best, float pen) {
    Contact* contact = data->m_contacts;

    auto normal = lhs->collider->getAxis(best);

    if (normal.dot(toCentre) > 0) {
        normal = -normal;
    }

    auto vertex = reinterpret_cast<KUMA::PHYSICS::CollisionBox*>(rhs->collider.get())->halfSize;
    if (rhs->collider->getAxis(0).dot(normal) < 0) {
        vertex.x = -vertex.x;
    }
    if (rhs->collider->getAxis(1).dot(normal) < 0) {
        vertex.y = -vertex.y;
    }
    if (rhs->collider->getAxis(2).dot(normal) < 0) {
        vertex.z = -vertex.z;
    }

    contact->contactNormal = normal;
    contact->penetration = pen;
    contact->contactPoint = rhs->collider->getTransform()* vertex;
    contact->body[0] = lhs->body;
    contact->body[1] = rhs->body;
    contact->friction = data->m_friction;
    contact->restitution = data->m_restitution;
}

Vector3 CalcEdgeContactPoint(Vector3& pLhs, Vector3& dLhs, float lhsSize, Vector3& pRhs, Vector3& dRhs, float rhsSize, bool useLhs) {
    float smLhs =    dLhs.LengthSqrt(dLhs);
    float smRhs =    dRhs.LengthSqrt(dRhs);
    float dpLhsRhs = dLhs.dot(dRhs);

    auto toSt = pLhs - pRhs;
    float dpStaLhs = toSt.dot(dLhs);
    float dpStaRhs = toSt.dot(dRhs);

    float denom = smLhs * smRhs - dpLhsRhs * dpLhsRhs;

    if (fabsf(denom) < 0.0001f) {
        return useLhs ? pLhs : pRhs;
    }

    float  mua = (dpLhsRhs * dpStaRhs - smRhs * dpStaLhs) / denom;
    float  mub = (smLhs * dpStaRhs - dpLhsRhs * dpStaLhs) / denom;

    if (mua > lhsSize ||
        mua < -lhsSize ||
        mub > rhsSize ||
        mub < -rhsSize) {
        return useLhs ? pLhs : pRhs;
    }
    else {
        auto cLhs = pLhs + (dLhs * mua);
        auto cRhs = pRhs + (dRhs * mub);

        return (cLhs + cRhs) * 0.5;
    }
}
#define CHECK_OVERLAP(axis, index) \
    if (!tryAxis(one, two, (axis), toCentre, (index), pen, best)) return 0;

unsigned CollisionDetector::boxAndBox(
    const CollisionBox& one,
    const CollisionBox& two,
    CollisionData* data
) {
    //if (!IntersectionTests::boxAndBox(one, two)) return 0;

    // Find the vector between the two centres
    Vector3 toCentre = two.getAxis(3) - one.getAxis(3);

    // We start assuming there is no contact
    float pen = FLT_MAX;
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
    if (best < 3) {
        // We've got a vertex of box two on a face of box one.
        fillPointFaceBoxBox(one, two, toCentre, data, best, pen);
        data->addContacts(1);
        return 1;
    }
    else if (best < 6) {
        // We've got a vertex of box one on a face of box two.
        // We use the same algorithm as above, but swap around
        // one and two (and therefore also the vector between their
        // centres).
        fillPointFaceBoxBox(two, one, toCentre * -1.0f, data, best - 3, pen);
        data->addContacts(1);
        return 1;
    }
    else {
        // We've got an edge-edge contact. Find out which axes
        best -= 6;
        unsigned oneAxisIndex = best / 3;
        unsigned twoAxisIndex = best % 3;
        Vector3 oneAxis = one.getAxis(oneAxisIndex);
        Vector3 twoAxis = two.getAxis(twoAxisIndex);
        Vector3 axis = oneAxis.cross(twoAxis);
        axis.normalise();

        // The axis should point from box one to box two.
        if (axis.dot(toCentre) > 0) axis = axis * -1.0f;

        // We have the axes, but not the edges: each axis has 4 edges parallel
        // to it, we need to find which of the 4 for each object. We do
        // that by finding the point in the centre of the edge. We know
        // its component in the direction of the box's collision axis is zero
        // (its a mid-point) and we determine which of the extremes in each
        // of the other axes is closest.
        Vector3 ptOnOneEdge = one.halfSize;
        Vector3 ptOnTwoEdge = two.halfSize;
        for (unsigned i = 0; i < 3; i++) {
            if (i == oneAxisIndex) ptOnOneEdge[i] = 0;
            else if (one.getAxis(i).dot(axis) > 0) ptOnOneEdge[i] = -ptOnOneEdge[i];

            if (i == twoAxisIndex) ptOnTwoEdge[i] = 0;
            else if (two.getAxis(i).dot(axis) < 0) ptOnTwoEdge[i] = -ptOnTwoEdge[i];
        }

        // Move them into world coordinates (they are already oriented
        // correctly, since they have been derived from the axes).
        ptOnOneEdge = one.transform * ptOnOneEdge;
        ptOnTwoEdge = two.transform * ptOnTwoEdge;

        // So we have a point and a direction for the colliding edges.
        // We need to find out point of closest approach of the two
        // line-segments.
        Vector3 vertex = contactPoint(
            ptOnOneEdge, oneAxis, static_cast<Vector3>(one.halfSize)[oneAxisIndex],
            ptOnTwoEdge, twoAxis, static_cast<Vector3>(two.halfSize)[twoAxisIndex],
            bestSingleAxis > 2
        );

        // We can fill the contact.
        Contact* contact = data->m_contacts;

        contact->penetration = pen;
        contact->contactNormal = axis;
        contact->contactPoint = vertex;
        contact->setBodyData(one.body, two.body,
            data->m_friction, data->m_restitution);
        data->addContacts(1);
        return 1;
    }
    return 0;
}
#undef CHECK_OVERLAP

/*
unsigned CollisionDetector::boxAndPoint(
    const CollisionBox &box,
    const Vector3 &point,
    CollisionData *data
    )
{
    // Transform the point into box coordinates
    Vector3 relPt = box.transform.transformInverse(point);

    Vector3 normal;

    // Check each axis, looking for the axis on which the
    // penetration is least deep.
    float min_depth = box.halfSize.x - fabs(relPt.x);
    if (min_depth < 0) return 0;
    normal = box.getAxis(0) * ((relPt.x < 0)?-1:1);

    float depth = box.halfSize.y - fabs(relPt.y);
    if (depth < 0) return 0;
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = box.getAxis(1) * ((relPt.y < 0)?-1:1);
    }

    depth = box.halfSize.z - fabs(relPt.z);
    if (depth < 0) return 0;
    else if (depth < min_depth)
    {
        min_depth = depth;
        normal = box.getAxis(2) * ((relPt.z < 0)?-1:1);
    }

    // Compile the contact
    auto& contact = data->contactArray.get()[data->curContactId];
    contact.contactNormal = normal;
    contact.contactPoint = point;
    contact.penetration = min_depth;
    contact.setBodyData(box.body, nullptr, data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}
*/
/*
unsigned CollisionDetector::boxAndSphere(
    const CollisionBox &box,
    const CollisionSphere &sphere,
    CollisionData *data
    )
{
    // Transform the centre of the sphere into box coordinates
    Vector3 centre = sphere.getAxis(3);
    Vector3 relCentre = box.transform.transformInverse(centre);

    // Early out check to see if we can exclude the contact
    if (fabs(relCentre.x) - sphere.radius > box.halfSize.x ||
        fabs(relCentre.y) - sphere.radius > box.halfSize.y ||
        fabs(relCentre.z) - sphere.radius > box.halfSize.z)
    {
        return 0;
    }

    Vector3 closestPt(0,0,0);
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
    Vector3 closestPtWorld = box.transform.transform(closestPt);

    auto& contact = data->contactArray.get()[data->curContactId];
    contact.contactNormal = (closestPtWorld - centre);
    contact.contactNormal.normalise();
    contact.contactPoint = closestPtWorld;
    contact.penetration = sphere.radius - fabs(dist);
    contact.setBodyData(box.body, sphere.body, data->friction, data->restitution);

    data->addContacts(1);
    return 1;
}
*/

unsigned CollisionDetector::boxAndHalfSpace(
    const CollisionBox &box,
    const CollisionPlane &plane,
    CollisionData *data
    )
{
    // Make sure we have contacts
    if (data->m_contactsLeft <= 0) return 0;

    // Check for intersection
    if (!IntersectionTests::boxAndHalfSpace(box, plane))
    {
        return 0;
    }
    static float mults[8][3] = {{1,1,1},{-1,1,1},{1,-1,1},{-1,-1,1},
                               {1,1,-1},{-1,1,-1},{1,-1,-1},{-1,-1,-1}};

    auto* contact = data->m_contactArray;
    unsigned contactsUsed = 0;
    for (unsigned i = 0; i < 8; i++) {

        // Calculate the position of each vertex
        Vector3 vertexPos(mults[i][0], mults[i][1], mults[i][2]);
        vertexPos.componentProductUpdate(box.halfSize);
        vertexPos = box.transform.transform(vertexPos);

        // Calculate the distance from the plane
        float vertexDistance = vertexPos.dot(plane.direction);

        // Compare this to the plane's distance
        if (vertexDistance <= plane.offset)
        {
            contact->contactPoint = plane.direction;
            contact->contactPoint *= (vertexDistance-plane.offset);
            contact->contactPoint += vertexPos;
            contact->contactNormal = plane.direction;
            contact->penetration = plane.offset - vertexDistance;

            // Write the appropriate data
            contact->setBodyData(box.body, nullptr, data->m_friction, data->m_restitution);

            // Move onto the next contact
            //data->curContactId++;
            contact++;
            contactsUsed++;
            if (contactsUsed == (unsigned)data->m_contactsLeft) return contactsUsed;
        }
    }

    data->addContacts(contactsUsed);
    return contactsUsed;
}


unsigned int KUMA::PHYSICS::DetectCollisionObjectAndObject(std::shared_ptr<ECS::PhysicsComponent> lhs, std::shared_ptr<ECS::PhysicsComponent> rhs, CollisionData* data) {
    assert(lhs && rhs && data);
    ECS::CollisionType lhsType = lhs->collisionType;
    ECS::CollisionType rhsType = rhs->collisionType;

    if (lhsType > rhsType) {
        std::swap(lhsType, rhsType);
        std::swap(lhs, rhs);
    }

    //if (lhsType == ECS::CollisionType::SPHERE) {
    //    const auto* lhsCollider = reinterpret_cast<const CollisionSphere*>(lhs->collider.get());

        //if (rhsType == ECS::CollisionType::SPHERE) {
        //    const CollisionSphere* rhsCollider = reinterpret_cast<const CollisionSphere*>(rhs->collider.get());
        //    return CollisionDetector::sphereAndSphere(*lhsCollider, *rhsCollider, data);
        //}
        //else if (rhsType == CollisionType::AABB) {
            //const CAaboundingbox* boxCollider = reinterpret_cast<const CAaboundingbox*>(rhs->GetDefaultCollider());
            //return CollisionDetector::boxAndSphere(*boxCollider, *lhsCollider, data);
        //}
        //else if (rhsType == CollisionType::OBB) {
        //    const CollisionBox* boxCollider = reinterpret_cast<const CollisionBox*>(rhs->GetDefaultCollider());
        //    return CollisionDetector::boxAndSphere(*boxCollider, *lhsCollider, data);
        //}
    //}
    //else if (lhsType == CollisionType::AABB) {
    //    const CAaboundingbox* boxCollider1 = reinterpret_cast<const CAaboundingbox*>(lhs->GetDefaultCollider());
    //
    //    if (rhsType == CollisionType::AABB) {
    //        const CAaboundingbox* boxCollider2 = reinterpret_cast<const CAaboundingbox*>(rhs->GetDefaultCollider());
    //        return CollisionDetector::boxAndBox(*boxCollider1, *boxCollider2, data);
    //    }
    //    else if (rhsType == CollisionType::OBB) {
    //        const COrientedBoundingBox* boxCollider2 = reinterpret_cast<const COrientedBoundingBox*>(rhs->GetDefaultCollider());
    //        return CollisionDetector::boxAndBox(*boxCollider1, *boxCollider2, data);
    //    }
    //}
    if (lhsType == ECS::CollisionType::OBB) {
        //const CollisionBox* boxCollider1 = reinterpret_cast<const CollisionBox*>(lhs->collider.get());

        if (rhsType == ECS::CollisionType::OBB) {
            //const CollisionBox* boxCollider2 = reinterpret_cast<const CollisionBox*>(rhs->collider.get());
            //return CollisionDetector::boxAndBox(lhs, rhs, data);
        }
    }

    return 0;
}
