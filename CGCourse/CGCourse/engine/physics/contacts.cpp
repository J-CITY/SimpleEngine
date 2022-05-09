
#include "contacts.h"
#include "body.h"
#include <memory.h>
#include <assert.h>
#include <vector>

using namespace KUMA;
using namespace KUMA::PHYSICS;
using namespace KUMA::MATHGL;


void Contact::setBodyData(std::shared_ptr<RigidBody> one, std::shared_ptr<RigidBody> two, float friction, float restitution) {
    this->body[0] = one;
    this->body[1] = two;
    this->friction = friction;
    this->restitution = restitution;
}

void Contact::matchAwakeState() {
    if (!body[1]) {
        return;
    }
    bool body0awake = body[0]->getAwake();
    bool body1awake = body[1]->getAwake();

    // Wake up only the sleeping one
    if (body0awake ^ body1awake) {
        if (body0awake) {
            body[1]->setAwake();
        }
        else {
            body[0]->setAwake();
        }
    }
}

void Contact::swapBodies() {
    contactNormal *= -1;
    auto temp = body[0];
    body[0] = body[1];
    body[1] = temp;
}

void Contact::calculateContactBasis() {
    Vector3 contactTangent[2];

    // Check whether the Z-axis is nearer to the X or Y axis
    if (std::fabsf(contactNormal.x) > fabsf(contactNormal.y)) {
        // Scaling factor to ensure the results are normalised
        const float s = 1.0f / sqrtf(contactNormal.z*contactNormal.z +
            contactNormal.x*contactNormal.x);

        // The new X-axis is at right angles to the world Y-axis
        contactTangent[0].x = contactNormal.z*s;
        contactTangent[0].y = 0;
        contactTangent[0].z = -contactNormal.x*s;

        // The new Y-axis is at right angles to the new X- and Z- axes
        contactTangent[1].x = contactNormal.y*contactTangent[0].x;
        contactTangent[1].y = contactNormal.z*contactTangent[0].x -
            contactNormal.x*contactTangent[0].z;
        contactTangent[1].z = -contactNormal.y*contactTangent[0].x;
    }
    else
    {
        // Scaling factor to ensure the results are normalised
        const float s = 1.0/sqrtf(contactNormal.z*contactNormal.z +
            contactNormal.y*contactNormal.y);

        // The new X-axis is at right angles to the world X-axis
        contactTangent[0].x = 0;
        contactTangent[0].y = -contactNormal.z*s;
        contactTangent[0].z = contactNormal.y*s;

        // The new Y-axis is at right angles to the new X- and Z- axes
        contactTangent[1].x = contactNormal.y*contactTangent[0].z -
            contactNormal.z*contactTangent[0].y;
        contactTangent[1].y = -contactNormal.x*contactTangent[0].z;
        contactTangent[1].z = contactNormal.x*contactTangent[0].y;
    }
    
    contactToWorld.setComponents(
        contactNormal,
        contactTangent[0],
        contactTangent[1]);
}

Vector3 Contact::calculateLocalVelocity(unsigned bodyIndex, float duration)
{
    auto thisBody = body[bodyIndex];

    // Work out the velocity of the contact point.
    Vector3 velocity =
        thisBody->getRotation().cross(relativeContactPosition[bodyIndex]);
    velocity += thisBody->getVelocity();

    // Turn the velocity into contact-coordinates.
    Vector3 contactVelocity = contactToWorld.transformTranspose(velocity);

    // Calculate the ammount of velocity that is due to forces without
    // reactions.
    Vector3 accVelocity = thisBody->getLastFrameAcceleration() * duration;

    // Calculate the velocity in contact-coordinates.
    accVelocity = contactToWorld.transformTranspose(accVelocity);

    // We ignore any component of acceleration in the contact normal
    // direction, we are only interested in planar acceleration
    accVelocity.x = 0;

    // Add the planar velocities - if there's enough friction they will
    // be removed during velocity resolution
    contactVelocity += accVelocity;

    // And return it
    return contactVelocity;
}


void Contact::calculateDesiredDeltaVelocity(float duration)
{
    const static float velocityLimit = (float)0.25f;

    // Calculate the acceleration induced velocity accumulated this frame
    float velocityFromAcc = 0;

    if (body[0]->getAwake()) {
		velocityFromAcc += (body[0]->getLastFrameAcceleration() * duration).dot(contactNormal);
    }

    if (body[1] && body[1]->getAwake()) {
        velocityFromAcc -= (body[1]->getLastFrameAcceleration() * duration).dot(contactNormal);
    }

    // If the velocity is very slow, limit the restitution
    float thisRestitution = restitution;
    if (std::fabsf(contactVelocity.x) < velocityLimit) {
        thisRestitution = (float)0.0f;
    }

    // Combine the bounce velocity with the removed
    // acceleration velocity.
    desiredDeltaVelocity =
        -contactVelocity.x
        -thisRestitution * (contactVelocity.x - velocityFromAcc);
}


void Contact::calculateInternals(float duration)
{
    // Check if the first object is NULL, and swap if it is.
    if (!body[0]) swapBodies();
    if (!body[0]) return;
    assert(body[0]);

    // Calculate an set of axis at the contact point.
    calculateContactBasis();

    // Store the relative position of the contact relative to each body
    relativeContactPosition[0] = contactPoint - body[0]->getPosition();
    if (body[1]) {
        relativeContactPosition[1] = contactPoint - body[1]->getPosition();
    }

    // Find the relative velocity of the bodies at the contact point.
    contactVelocity = calculateLocalVelocity(0, duration);
    if (body[1]) {
        contactVelocity -= calculateLocalVelocity(1, duration);
    }

    // Calculate the desired change in velocity for resolution
    calculateDesiredDeltaVelocity(duration);
}

void Contact::applyVelocityChange(Vector3 velocityChange[2],
                                  Vector3 rotationChange[2])
{
    // Get hold of the inverse mass and inverse inertia tensor, both in
    // world coordinates.
    Matrix3 inverseInertiaTensor[2];
    body[0]->getInverseInertiaTensorWorld(&inverseInertiaTensor[0]);
    if (body[1])
        body[1]->getInverseInertiaTensorWorld(&inverseInertiaTensor[1]);

    // We will calculate the impulse for each contact axis
    Vector3 impulseContact;

    if (friction == (float)0.0)
    {
        // Use the short format for frictionless contacts
        impulseContact = calculateFrictionlessImpulse(inverseInertiaTensor);
    }
    else
    {
        // Otherwise we may have impulses that aren't in the direction of the
        // contact, so we need the more complex version.
        impulseContact = calculateFrictionImpulse(inverseInertiaTensor);
    }

    // Convert impulse to world coordinates
    Vector3 impulse = contactToWorld.transform(impulseContact);

    // Split in the impulse into linear and rotational components
    Vector3 impulsiveTorque = relativeContactPosition[0].cross(impulse);
    rotationChange[0] = inverseInertiaTensor[0].transform(impulsiveTorque);
    //velocityChange[0] = Vector3::Zero;
    velocityChange[0] = impulse * body[0]->getInverseMass();

    // Apply the changes
    body[0]->addVelocity(velocityChange[0]);
    body[0]->addRotation(rotationChange[0]);

    if (body[1])
    {
        // Work out body one's linear and angular changes
        Vector3 impulsiveTorque = impulse.cross(relativeContactPosition[1]);
        rotationChange[1] = inverseInertiaTensor[1].transform(impulsiveTorque);
        //velocityChange[1] = Vector3::Zero;
        velocityChange[1]  = impulse * -body[1]->getInverseMass();

        // And apply them.
        body[1]->addVelocity(velocityChange[1]);
        body[1]->addRotation(rotationChange[1]);
    }
}

inline
Vector3 Contact::calculateFrictionlessImpulse(Matrix3 * inverseInertiaTensor)
{
    Vector3 impulseContact;

    // Build a vector that shows the change in velocity in
    // world space for a unit impulse in the direction of the contact
    // normal.
    Vector3 deltaVelWorld = relativeContactPosition[0].cross(contactNormal);
    deltaVelWorld = inverseInertiaTensor[0].transform(deltaVelWorld);
    deltaVelWorld = deltaVelWorld.cross(relativeContactPosition[0]);

    // Work out the change in velocity in contact coordiantes.
    float deltaVelocity = deltaVelWorld.dot(contactNormal);

    // Add the linear component of velocity change
    deltaVelocity += body[0]->getInverseMass();

    // Check if we need to the second body's data
    if (body[1])
    {
        // Go through the same transformation sequence again
        Vector3 deltaVelWorld = relativeContactPosition[1].cross(contactNormal);
        deltaVelWorld = inverseInertiaTensor[1].transform(deltaVelWorld);
        deltaVelWorld = deltaVelWorld.cross(relativeContactPosition[1]);

        // Add the change in velocity due to rotation
        deltaVelocity += deltaVelWorld.dot(contactNormal);

        // Add the change in velocity due to linear motion
        deltaVelocity += body[1]->getInverseMass();
    }
    impulseContact.x = desiredDeltaVelocity / deltaVelocity;
    impulseContact.y = 0;
    impulseContact.z = 0;
    return impulseContact;
}

inline
Vector3 Contact::calculateFrictionImpulse(Matrix3 * inverseInertiaTensor)
{
    Vector3 impulseContact;
    float inverseMass = body[0]->getInverseMass();

    // The equivalent of a cross product in matrices is multiplication
    // by a skew symmetric matrix - we build the matrix for converting
    // between linear and angular quantities.
    Matrix3 impulseToTorque;
    impulseToTorque.setSkewSymmetric(relativeContactPosition[0]);

    // Build the matrix to convert contact impulse to change in velocity
    // in world coordinates.
    Matrix3 deltaVelWorld = impulseToTorque;
    deltaVelWorld *= inverseInertiaTensor[0];
    deltaVelWorld *= impulseToTorque;
    deltaVelWorld *= -1;

    // Check if we need to add body two's data
    if (body[1])
    {
        // Set the cross product matrix
        impulseToTorque.setSkewSymmetric(relativeContactPosition[1]);

        // Calculate the velocity change matrix
        Matrix3 deltaVelWorld2 = impulseToTorque;
        deltaVelWorld2 *= inverseInertiaTensor[1];
        deltaVelWorld2 *= impulseToTorque;
        deltaVelWorld2 *= -1;

        // Add to the total delta velocity.
        deltaVelWorld += deltaVelWorld2;

        // Add to the inverse mass
        inverseMass += body[1]->getInverseMass();
    }

    // Do a change of basis to convert into contact coordinates.
    Matrix3 deltaVelocity = contactToWorld.transpose();
    deltaVelocity *= deltaVelWorld;
    deltaVelocity *= contactToWorld;

    // Add in the linear velocity change
    deltaVelocity.data[0] += inverseMass;
    deltaVelocity.data[4] += inverseMass;
    deltaVelocity.data[8] += inverseMass;

    // Invert to get the impulse needed per unit velocity
    Matrix3 impulseMatrix = deltaVelocity.inverse();

    // Find the target velocities to kill
    Vector3 velKill(desiredDeltaVelocity,
        -contactVelocity.y,
        -contactVelocity.z);

    // Find the impulse to kill target velocities
    impulseContact = impulseMatrix.transform(velKill);

    // Check for exceeding friction
    float planarImpulse = std::sqrt(
        impulseContact.y*impulseContact.y +
        impulseContact.z*impulseContact.z);
    if (planarImpulse > impulseContact.x * friction) {
        // We need to use dynamic friction
        impulseContact.y /= planarImpulse;
        impulseContact.z /= planarImpulse;

        impulseContact.x = deltaVelocity.data[0] +
            deltaVelocity.data[1]*friction*impulseContact.y +
            deltaVelocity.data[2]*friction*impulseContact.z;
        impulseContact.x = desiredDeltaVelocity / impulseContact.x;
        impulseContact.y *= friction * impulseContact.x;
        impulseContact.z *= friction * impulseContact.x;
    }
    return impulseContact;
}

void Contact::applyPositionChange(Vector3 linearChange[2],
                                  Vector3 angularChange[2],
                                  float penetration)
{
    const float angularLimit = (float)0.2f;
    float angularMove[2];
    float linearMove[2];

    float totalInertia = 0;
    float linearInertia[2];
    float angularInertia[2];

    // We need to work out the inertia of each object in the direction
    // of the contact normal, due to angular inertia only.
    for (unsigned i = 0; i < 2; i++) if (body[i])
    {
        Matrix3 inverseInertiaTensor;
        body[i]->getInverseInertiaTensorWorld(&inverseInertiaTensor);

        // Use the same procedure as for calculating frictionless
        // velocity change to work out the angular inertia.
        Vector3 angularInertiaWorld =
            relativeContactPosition[i].cross(contactNormal);
        angularInertiaWorld =
            inverseInertiaTensor.transform(angularInertiaWorld);
        angularInertiaWorld =
            angularInertiaWorld.cross(relativeContactPosition[i]);
        angularInertia[i] =
            angularInertiaWorld.dot(contactNormal);

        // The linear component is simply the inverse mass
        linearInertia[i] = body[i]->getInverseMass();

        // Keep track of the total inertia from all components
        totalInertia += linearInertia[i] + angularInertia[i];

        // We break the loop here so that the totalInertia value is
        // completely calculated (by both iterations) before
        // continuing.
    }

    // Loop through again calculating and applying the changes
    for (unsigned i = 0; i < 2; i++) if (body[i] && !body[i]->isStatic)
    {
        // The linear and angular movements required are in proportion to
        // the two inverse inertias.
        float sign = (i == 0)?1:-1;
        angularMove[i] =
            sign * penetration * (angularInertia[i] / totalInertia);
        linearMove[i] =
            sign * penetration * (linearInertia[i] / totalInertia);

        // To avoid angular projections that are too great (when mass is large
        // but inertia tensor is small) limit the angular move.
        Vector3 projection = relativeContactPosition[i];
        projection.addScaledVector(
            contactNormal,
            -relativeContactPosition[i].dot(contactNormal)
            );

        // Use the small angle approximation for the sine of the angle (i.e.
        // the magnitude would be sine(angularLimit) * projection.magnitude
        // but we approximate sine(angularLimit) to angularLimit).
        float maxMagnitude = angularLimit * projection.magnitude();

        if (angularMove[i] < -maxMagnitude) {
            float totalMove = angularMove[i] + linearMove[i];
            angularMove[i] = -maxMagnitude;
            linearMove[i] = totalMove - angularMove[i];
        }
        else if (angularMove[i] > maxMagnitude) {
            float totalMove = angularMove[i] + linearMove[i];
            angularMove[i] = maxMagnitude;
            linearMove[i] = totalMove - angularMove[i];
        }

        // We have the linear amount of movement required by turning
        // the rigid body (in angularMove[i]). We now need to
        // calculate the desired rotation to achieve that.
        if (angularMove[i] == 0)
        {
            // Easy case - no angular movement means no rotation.
            angularChange[i] = Vector3::Zero;
        }
        else
        {
            // Work out the direction we'd like to rotate in.
            Vector3 targetAngularDirection =
                relativeContactPosition[i].cross(contactNormal);

            Matrix3 inverseInertiaTensor;
            body[i]->getInverseInertiaTensorWorld(&inverseInertiaTensor);

            // Work out the direction we'd need to rotate to achieve that
            angularChange[i] =
                inverseInertiaTensor.transform(targetAngularDirection) *
                (angularMove[i] / angularInertia[i]);
        }

        // Velocity change is easier - it is just the linear movement
        // along the contact normal.
        linearChange[i] = contactNormal * linearMove[i];

        // Now we can start to apply the values we've calculated.
        // Apply the linear movement
        Vector3 pos;
        body[i]->getPosition(&pos);
        pos.addScaledVector(contactNormal, linearMove[i]);
        body[i]->setPosition(pos);

        // And the change in orientation
        Quaternion q;
        body[i]->getOrientation(&q);
        q.addScaledVector(angularChange[i], ((float)1.0));
        body[i]->setOrientation(q);

        // We need to calculate the derived data for any body that is
        // asleep, so that the changes are reflected in the object's
        // data. Otherwise the resolution will not change the position
        // of the object, and the next collision detection round will
        // have the same penetration.
        if (!body[i]->getAwake()) body[i]->calculateDerivedData();
    }
}





// Contact resolver implementation

ContactResolver::ContactResolver(unsigned iterations,
                                 float velocityEpsilon,
                                 float positionEpsilon)
{
    setIterations(iterations, iterations);
    setEpsilon(velocityEpsilon, positionEpsilon);
}

ContactResolver::ContactResolver(unsigned velocityIterations,
                                 unsigned positionIterations,
                                 float velocityEpsilon,
                                 float positionEpsilon)
{
    setIterations(velocityIterations);
    setEpsilon(velocityEpsilon, positionEpsilon);
}

void ContactResolver::setIterations(unsigned iterations)
{
    setIterations(iterations, iterations);
}

void ContactResolver::setIterations(unsigned velocityIterations,
                                    unsigned positionIterations)
{
    ContactResolver::velocityIterations = velocityIterations;
    ContactResolver::positionIterations = positionIterations;
}

void ContactResolver::setEpsilon(float velocityEpsilon,
                                 float positionEpsilon)
{
    ContactResolver::velocityEpsilon = velocityEpsilon;
    ContactResolver::positionEpsilon = positionEpsilon;
}

void ContactResolver::resolveContacts(std::vector<Contact>& contacts,
                                      unsigned numContacts,
                                      float duration)
{
    // Make sure we have something to do.
    if (numContacts == 0) return;
    if (!isValid()) return;
    
    // Prepare the contacts for processing
    prepareContacts(contacts, numContacts, duration);

    // Resolve the interpenetration problems with the contacts.
    adjustPositions(contacts.data(), numContacts, duration);

    // Resolve the velocity problems with the contacts.
    adjustVelocities(contacts.data(), numContacts, duration);
}

void ContactResolver::prepareContacts(std::vector<Contact>& contacts, unsigned numContacts, float duration) {
    Contact* lastContact = contacts.data() + numContacts;
    for (Contact* contact = contacts.data(); contact < lastContact; ++contact) {
        contact->calculateInternals(duration);
    }
}

void ContactResolver::adjustVelocities(Contact* c,
                                       unsigned numContacts,
                                       float duration)
{
    Vector3 velocityChange[2], rotationChange[2];
    Vector3 deltaVel;

    // iteratively handle impacts in order of severity.
    velocityIterationsUsed = 0;
    while (velocityIterationsUsed < velocityIterations)
    {
        // Find contact with maximum magnitude of probable velocity change.
        float max = velocityEpsilon;
        unsigned index = numContacts;
        for (unsigned i = 0; i < numContacts; i++)
        {
            if (c[i].desiredDeltaVelocity > max)
            {
                max = c[i].desiredDeltaVelocity;
                index = i;
            }
        }
        if (index == numContacts) break;

        // Match the awake state at the contact
        c[index].matchAwakeState();

        // Do the resolution on the contact that came out top.
        c[index].applyVelocityChange(velocityChange, rotationChange);

        // With the change in velocity of the two bodies, the update of
        // contact velocities means that some of the relative closing
        // velocities need recomputing.
        for (unsigned i = 0; i < numContacts; i++)
        {
            // Check each body in the contact
            for (unsigned b = 0; b < 2; b++) if (c[i].body[b])
            {
                // Check for a match with each body in the newly
                // resolved contact
                for (unsigned d = 0; d < 2; d++)
                {
                    if (c[i].body[b] == c[index].body[d])
                    {
                        deltaVel = velocityChange[d] +
                            rotationChange[d].cross(
                                c[i].relativeContactPosition[b]);
                        
                        c[i].contactVelocity +=
                            c[i].contactToWorld.transformTranspose(deltaVel)
                            * (b?-1:1);
                        c[i].calculateDesiredDeltaVelocity(duration);
                    }
                }
            }
        }
        velocityIterationsUsed++;
    }
}

void ContactResolver::adjustPositions(Contact* c,
                                      unsigned numContacts,
                                      float duration)
{
    unsigned i,index;
    Vector3 linearChange[2], angularChange[2];
    float max;
    Vector3 deltaPosition;

    // iteratively resolve interpenetrations in order of severity.
    positionIterationsUsed = 0;
    while (positionIterationsUsed < positionIterations)
    {
        // Find biggest penetration
        max = positionEpsilon;
        index = numContacts;
        for (i=0; i<numContacts; i++)
        {
            if (c[i].penetration > max)
            {
                max = c[i].penetration;
                index = i;
            }
        }
        if (index == numContacts) break;

        // Match the awake state at the contact
        c[index].matchAwakeState();

        // Resolve the penetration.
        c[index].applyPositionChange(
            linearChange,
            angularChange,
            max);

        // Again this action may have changed the penetration of other
        // bodies, so we update contacts.
        for (i = 0; i < numContacts; i++) {
            // Check each body in the contact
            for (unsigned b = 0; b < 2; b++) if (c[i].body[b]) {
                // Check for a match with each body in the newly
                // resolved contact
                for (unsigned d = 0; d < 2; d++) {
                    if (c[i].body[b] == c[index].body[d]) {
                        deltaPosition = linearChange[d] + angularChange[d].cross(c[i].relativeContactPosition[b]);
                        c[i].penetration += deltaPosition.dot(c[i].contactNormal) * (b?1:-1);
                    }
                }
            }
        }
        positionIterationsUsed++;
    }
}