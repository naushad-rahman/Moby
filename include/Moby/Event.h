/****************************************************************************
 * Copyright 2011 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

#ifndef _MOBY_EVENT_H
#define _MOBY_EVENT_H

#include <iostream>
#include <list>
#include <Ravelin/Vector3d.h>
#include <Ravelin/Wrenchd.h>
#include <boost/shared_ptr.hpp>
#include <Moby/Constants.h>
#include <Moby/Types.h>
#include <Moby/ContactParameters.h>

namespace osg { class Node; }

namespace Moby {

class CollisionGeometry;

/// Container class for describing an event in the simulation        
class Event 
{
  public:
    enum EventType { eNone, eContact, eLimit, eConstraint };
    enum EventClass { eUndetermined, eSeparating, eResting, eImpacting };
    Event();
    Event(const Event& e) { *this = e; }
    static void determine_connected_events(const std::vector<Event>& events, std::list<std::list<Event*> >& groups);
    static void remove_nonimpacting_groups(std::list<std::list<Event*> >& groups);
    Event& operator=(const Event& e);
    double calc_event_vel() const;
    double calc_event_tol() const;
    EventClass determine_event_class() const;
    bool is_impacting() const { return determine_event_class() == eImpacting; }
    bool is_resting() const { return determine_event_class() == eResting; }
    bool is_separating() const { return determine_event_class() == eSeparating; }
    void set_contact_parameters(const ContactParameters& cparams);
    void determine_contact_tangents();
    void update_event_data(Ravelin::MatrixNd& M, Ravelin::VectorNd& q) const;
    void update_cross_event_data(const Event& e, Ravelin::MatrixNd& M) const;
    static void determine_minimal_set(std::list<Event*>& group);

    template <class OutputIterator>
    OutputIterator get_super_bodies(OutputIterator begin) const;

    /// The type of event
    EventType event_type;

    /// The time that the event occurs [0,1]
    double t;

    /// The "real" time that the event occurs [0, infinity]
    double t_true;

    /// The joint at which the limit is reached (for limit events)
    JointPtr limit_joint;

    /// The coefficient of restitution for this limit
    double limit_epsilon;

    /// The DOF at which the limit is reached (for limit events)
    unsigned limit_dof;

    /// Whether the upper/lower limit is reached (for limit events)
    bool limit_upper;

    /// Limit impulse magnitude (for limit events)
    double limit_impulse;

    /// Constraint [normal] impulse magnitude (for constraint events)
    Ravelin::VectorNd constraint_nimpulse;

    /// Constraint [friction] impulse magnitude (for constraint events)
    Ravelin::VectorNd constraint_fimpulse;

    /// The joint (for constraint events)
    JointPtr constraint_joint;

    /// The point contact (for contact events)
    Ravelin::Point3d contact_point;
    
    /// The vector pointing outward from the contact on the first body, in world coordinates (for contact events)
    Ravelin::Vector3d contact_normal;  

    /// The first tangent direction to the contact normal
    Ravelin::Vector3d contact_tan1;

    /// The second tangent direction to the contact normal
    Ravelin::Vector3d contact_tan2;

    /// Impulse that has been applied (for contact events)
    /**
     * Impulse applied to the body of the first geometry; 
     * the reverse of this force / impulse is applied to the body of the second 
     * geometry.
     */
    Ravelin::Wrenchd contact_impulse;

    /// The collision geometries involved (for contact events)
    CollisionGeometryPtr contact_geom1, contact_geom2;

    /// The coefficient of Coulomb friction (for contact events)
    double contact_mu_coulomb;

    /// The coefficient of viscous friction (for contact events)
    double contact_mu_viscous;

    /// The coefficient of restitution (for contact events)
    double contact_epsilon;

    /// The number of friction directions >= 4 (for contact events)
    unsigned contact_NK;

    osg::Node* to_visualization_data() const;

    /// Tolerance for the event (users never need to modify this)
    double tol;

    void write_vrml(const std::string& filename, double sphere_radius = 0.1, double normal_length = 1.0) const;
    bool operator<(const Event& e) const { return t < e.t; }
    void compute_event_data(Ravelin::MatrixNd& M, Ravelin::VectorNd& q) const;
    void compute_cross_event_data(const Event& e, Ravelin::MatrixNd& M) const;

  private:
    unsigned get_super_bodies(DynamicBodyPtr& sb1, DynamicBodyPtr& sb2) const;
    static void determine_convex_set(std::list<Event*>& group);
    static bool is_contact_manifold_2D(const std::list<Event*>& group);

    template <class BidirectionalIterator>
    static void insertion_sort(BidirectionalIterator begin, BidirectionalIterator end);
}; // end class

std::ostream& operator<<(std::ostream& out, const Event& e);

#include "Event.inl"

} // end namespace Moby

#endif

