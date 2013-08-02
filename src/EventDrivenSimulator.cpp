/****************************************************************************
 * Copyright 2008 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

#include <boost/tuple/tuple.hpp>
#include <Moby/XMLTree.h>
#include <Moby/ArticulatedBody.h>
#include <Moby/RigidBody.h>
#include <Moby/DynamicBody.h>
#include <Moby/CollisionGeometry.h>
#include <Moby/CollisionDetection.h>
#include <Moby/ContactParameters.h>
#include <Moby/VariableStepIntegrator.h>
#include <Moby/ImpactToleranceException.h>
#include <Moby/EventDrivenSimulator.h>

#ifdef USE_OSG
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Quat>
#endif // USE_OSG

using std::endl;
using std::list;
using std::vector;
using std::map;
using std::make_pair;
using std::multimap;
using std::pair;
using boost::tuple;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;
using namespace Ravelin;
using namespace Moby;

/// Default constructor
EventDrivenSimulator::EventDrivenSimulator()
{
  event_callback_fn = NULL;
  event_post_impulse_callback_fn = NULL;
  post_mini_step_callback_fn = NULL;
  _simulation_violated = false;
  render_contact_points = false;
}

/// Gets the contact data between a pair of geometries (if any)
/**
 * This method looks for contact data not only between the pair of geometries, but also
 * the rigid bodies that the geometries belong to, and any articulated bodies as well.
 * The search proceeds in the following manner: <br />
 * <ol>
 *  <li>two collision geometries</li>
 *  <li>one collision geometry, one rigid body</li>
 *  <li>two rigid bodies</li>
 *  <li>one collision geometry, one articulated body</li>
 *  <li>one rigid body, one articulated body</li>
 *  <li>two articulated bodies</li>
 * </ol>
 * The search order allows for multiple granularities; for example, a collision can easily
 * be specified between two geometries of two of a robot's links (i.e., representing different
 * surfaces on the links), between two links, or between two robots.
 * \param g1 the first collision geometry
 * \param g2 the second collision geometry
 * \return a pointer to the contact data, if any, found
 */
shared_ptr<ContactParameters> EventDrivenSimulator::get_contact_parameters(CollisionGeometryPtr geom1, CollisionGeometryPtr geom2) const
{
  map<sorted_pair<BasePtr>, shared_ptr<ContactParameters> >::const_iterator iter;

  // search for the two contact geometries first
  if ((iter = contact_params.find(make_sorted_pair(geom1, geom2))) != contact_params.end())
    return iter->second;

  // get the geometries as base pointers
  BasePtr g1(geom1);
  BasePtr g2(geom2);
  
  // get the two single bodies
  assert(geom1->get_single_body());
  assert(geom2->get_single_body());
  SingleBodyPtr singlebody1 = geom1->get_single_body();
  SingleBodyPtr singlebody2 = geom2->get_single_body();
  BasePtr sb1 = singlebody1;
  BasePtr sb2 = singlebody2;

  // search for contact geometry 1 and rigid body 2
  if ((iter = contact_params.find(make_sorted_pair(g1, sb2))) != contact_params.end())
    return iter->second;

  // search for contact geometry 2 and rigid body 1
  if ((iter = contact_params.find(make_sorted_pair(g2, sb1))) != contact_params.end())
    return iter->second;

  // search for both rigid bodies
  if ((iter = contact_params.find(make_sorted_pair(sb1, sb2))) != contact_params.end())
    return iter->second;

  // get the articulated bodies, if any
  RigidBodyPtr rb1 = dynamic_pointer_cast<RigidBody>(singlebody1);
  RigidBodyPtr rb2 = dynamic_pointer_cast<RigidBody>(singlebody2);
  BasePtr ab1, ab2;
  if (rb1)
    ab1 = rb1->get_articulated_body();
  if (rb2)
    ab2 = rb2->get_articulated_body();

  // check collision geometry 2 and rigid body 2 against articulated body 1
  if (ab1)
  {
    if ((iter = contact_params.find(make_sorted_pair(g2, ab1))) != contact_params.end())
      return iter->second;
    if ((iter = contact_params.find(make_sorted_pair(sb2, ab1))) != contact_params.end())
      return iter->second;
  }

  // check collision geometry 1 and rigid body 1 against articulated body 2
  if (ab2)
  {
    if ((iter = contact_params.find(make_sorted_pair(g1, ab2))) != contact_params.end())
      return iter->second;
    if ((iter = contact_params.find(make_sorted_pair(sb1, ab2))) != contact_params.end())
      return iter->second;
  }

  // check the two articulated bodies against articulated body 2
  if (ab1 && ab2)
    if ((iter = contact_params.find(make_sorted_pair(ab1, ab2))) != contact_params.end())
      return iter->second;
  
  // still here?  no contact data found
  return shared_ptr<ContactParameters>();
}

/// Draws a ray directed from a contact point along the contact normal
void EventDrivenSimulator::visualize_contact( Event& event ) {

  #ifdef USE_OSG

  // random color for this contact visualization
  double r = (double) rand() / (double) RAND_MAX;
  double g = (double) rand() / (double) RAND_MAX;
  double b = (double) rand() / (double) RAND_MAX;
  osg::Vec4 color = osg::Vec4( r, g, b, 1.0 );

  // knobs for tweaking
  const double point_radius = 0.75;
  const double point_scale = 0.01;
  const double line_length = 5.0;
  const double line_radius = 0.1;
  const double head_radius = 0.5;
  const double head_height = 2.0;

  // the osg node this event visualization will attach to 
  osg::Group* contact_root = new osg::Group();

  // turn off lighting for this node
  osg::StateSet *contact_state = contact_root->getOrCreateStateSet();
  contact_state->setMode( GL_LIGHTING, osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF );

  // a geode for the visualization geometry
  osg::Geode* contact_geode = new osg::Geode();

  // add some hints to reduce the polygonal complexity of the visualization
  osg::TessellationHints *hints = new osg::TessellationHints();
  hints->setTessellationMode( osg::TessellationHints::USE_TARGET_NUM_FACES );
  hints->setCreateNormals( true );
  hints->setDetailRatio( 0.2 );

  // add the contact point as a sphere at the origin of the geode's frame
  osg::Sphere* point_geometry = new osg::Sphere( osg::Vec3( 0, 0, 0 ), point_radius );
  osg::ShapeDrawable* point_shape = new osg::ShapeDrawable( point_geometry, hints );
  point_shape->setColor( color );
  contact_geode->addDrawable( point_shape );

  // add the contact normal as a cylinder in the geode's frame
  osg::Cylinder* line_geometry = new osg::Cylinder( osg::Vec3( 0.0, 0.0, line_length / 2 ), line_radius, line_length );
  osg::ShapeDrawable* line_shape = new osg::ShapeDrawable( line_geometry, hints );
  line_shape->setColor( color );
  contact_geode->addDrawable( line_shape );

  // add the arrow head as a cone in the geode's frame
  osg::Cone* head_geometry = new osg::Cone( osg::Vec3( 0, 0, line_length ), head_radius, head_height );
  osg::ShapeDrawable* head_shape = new osg::ShapeDrawable( head_geometry, hints );
  head_shape->setColor( color );
  contact_geode->addDrawable( head_shape );

  // calculate the orientation based upon the direction of the normal vector.
  // Note: the default orientation of the osg model is along the z-axis
  double theta;
  Vector3d z = Vector3d( 0.0, 0.0, 1.0 );
  Vector3d axis = Vector3d::cross( event.contact_normal, z );
  if( axis.norm_inf() < NEAR_ZERO) {
    // z and normal are parallel, axis ill defined
    if( event.contact_normal[2] > 0 ) {
      // normal is z
      axis = Vector3d( 0.0, 1.0, 0.0 );
      theta = 0.0;
    } else {
      // normal is -z
      axis = Vector3d( 0.0, 1.0, 0.0 );
      theta = osg::PI;
    }
  } else {
    // axis is well defined
    axis = Vector3d::normalize(axis);
    theta = -std::acos( Vector3d::dot( event.contact_normal, z ) );
    // Note: theta calculation works but is not robust, could be rotated in opposite direction
  }
  osg::Quat q = osg::Quat( axis[0]*std::sin(theta/2), axis[1]*std::sin(theta/2), axis[2]*std::sin(theta/2), std::cos(theta/2) );

  // create the visualization transform
  osg::PositionAttitudeTransform* contact_transform = new osg::PositionAttitudeTransform();
  contact_transform->setPosition( osg::Vec3( event.contact_point[0], event.contact_point[1], event.contact_point[2] ) );
  contact_transform->setScale( osg::Vec3( point_scale, point_scale, point_scale ) );
  contact_transform->setAttitude( q );

  // add the geode to the transform
  contact_transform->addChild( contact_geode );

  // add the transform to the root
  contact_root->addChild( contact_transform );
  
  // add the root to the transient data scene graph
  add_transient_vdata( contact_root );

  // JRT : remove validator once theta 100% proven
  // -----------------------------------------
  // Rotational Validator
  // -----------------------------------------

  // Validator is a simple sphere translated along the normal
  // such that the visualization above should point at the center
  // of the validator.  If it doesn't, then the calculation of 
  // theta in the rotational code above needs correction for that case

  // knobs for tweaking
  const double validator_scale = point_scale / 3;
  const double validator_ray_length = line_length * 2.5;

  // a root for the validator
  osg::Group* validator_root = new osg::Group();

  // turn off lighting for this node
  osg::StateSet *validator_state = validator_root->getOrCreateStateSet();
  validator_state->setMode( GL_LIGHTING, osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF );

  // colocate the validator position to the contact point
  osg::PositionAttitudeTransform* validator_transform = new osg::PositionAttitudeTransform();
  validator_transform->setPosition( osg::Vec3( event.contact_point[0], event.contact_point[1], event.contact_point[2] ) );
  validator_transform->setScale( osg::Vec3( validator_scale, validator_scale, validator_scale ) );
  validator_root->addChild( validator_transform );

  // validator geometry
  osg::Sphere* validator_geometry = new osg::Sphere( osg::Vec3( 0, 0, 0 ), 1.0 );
  osg::ShapeDrawable* validator_shape = new osg::ShapeDrawable( validator_geometry, hints );
  validator_shape->setColor( color );

  // validator transform follows the normal out to a distance of validator_ray_length
  // Note: the validator is not rotated at all.  It is translated from the point along the normal
  osg::PositionAttitudeTransform* validator_end_transform = new osg::PositionAttitudeTransform();
  validator_end_transform->setPosition( osg::Vec3( event.contact_normal[0] * validator_ray_length, event.contact_normal[1] * validator_ray_length, event.contact_normal[2] * validator_ray_length ) );
  validator_transform->addChild( validator_end_transform );

  // add all validator constituents to the group
  osg::Geode* validator_geode = new osg::Geode();
  validator_transform->addChild( validator_end_transform );
  validator_end_transform->addChild( validator_geode );
  validator_geode->addDrawable( validator_shape );
  add_transient_vdata( validator_root );

  #endif // USE_OSG
}

/// Handles events
void EventDrivenSimulator::handle_events()
{
  // if the setting is enabled, draw all contact events
  if( render_contact_points ) {
    for ( std::vector<Event>::iterator it = _events.begin(); it < _events.end(); it++ ) {
      Event event = *it;
      if( event.event_type != Event::eContact ) continue;
      visualize_contact( event );
    }
  }

  // call the callback function, if any
  if (event_callback_fn)
    (*event_callback_fn)(_events, event_callback_data);

  // preprocess events
  for (unsigned i=0; i< _events.size(); i++)
    preprocess_event(_events[i]);

  // begin timeing for event handling 
  tms start;  
  times(&start);

  // compute impulses here...
  try
  {
    _impact_event_handler.process_events(_events);
  }
  catch (ImpactToleranceException e)
  {
    // process events, updating tolerances
    BOOST_FOREACH(Event* ev, e.events)
    {
      double event_v = ev->calc_event_vel();
      _event_tolerances[*ev] = std::fabs(event_v) + std::numeric_limits<double>::epsilon();  
    }
  }

  // tabulate times for event handling 
  tms stop;  
  times(&stop);
  event_utime += (double) (stop.tms_utime-start.tms_utime)/CLOCKS_PER_SEC;
  event_stime += (double) (stop.tms_stime-start.tms_stime)/CLOCKS_PER_SEC;

  // call the post-impulse application callback, if any 
  if (event_post_impulse_callback_fn)
    (*event_post_impulse_callback_fn)(_events, event_post_impulse_callback_data);
}

/// Performs necessary preprocessing on an event
void EventDrivenSimulator::preprocess_event(Event& e) 
{
  // no pre-processing for limit events currently...
  if (e.event_type == Event::eLimit)
    return;

  // no pre-processing for (none) events
  if (e.event_type == Event::eNone)
    return;

  // get the contact parameters 
  assert(e.event_type == Event::eContact);
  shared_ptr<ContactParameters> cparams = get_contact_parameters(e.contact_geom1, e.contact_geom2);
  if (cparams)
    e.set_contact_parameters(*cparams);
  else
  {
    SingleBodyPtr sb1(e.contact_geom1->get_single_body());
    SingleBodyPtr sb2(e.contact_geom2->get_single_body());
    std::cerr << "EventDrivenSimulator::preprocess_event() warning- no contact ";
    std::cerr << "data for contact" << std::endl;
    std::cerr << "  between " << e.contact_geom1->id << " (body ";
    std::cerr << sb1->id << ") and " << e.contact_geom2->id;
    std::cerr << " (body " << sb2->id << ")" << std::endl;
    std::cerr << "  ... ignoring" << std::endl;
  }
}

/// Saves the coords of all bodies
void EventDrivenSimulator::get_coords(vector<VectorNd>& q) const
{
  // resize the vector if necessary
  q.resize(_bodies.size());

  for (unsigned i=0; i< _bodies.size(); i++)
    _bodies[i]->get_generalized_coordinates(DynamicBody::eEuler, q[i]);
}

/// Saves the velocities of all bodies
void EventDrivenSimulator::get_velocities(vector<VectorNd>& qd) const
{
  // resize the vector if necessary
  qd.resize(_bodies.size());

  for (unsigned i=0; i< _bodies.size(); i++)
    _bodies[i]->get_generalized_velocity(DynamicBody::eEuler, qd[i]);
}

/// Sets the coords and velocities of all bodies
/**
 * Note that we set velocities in this manner (rather than using current
 * velocities) because our event finding methods assume constant velocities
 * over a time interval. Linearly interpolating the positions and velocities
 * (as we do) will result in warped dynamics.
 * \param q0 the coords at time 0
 * \param q1 the coords at time dt
 * \param t the mixture [0,1] to set the coordinates and velocities (t=0 is
 *        equivalent to time 0; t=1 equivalent to time dt)
 */
/*
void EventDrivenSimulator::set_coords_and_velocities(const vector<pair<VectorNd, VectorNd> >& q0, const vector<pair<VectorNd, VectorNd> >& q1, double t) const
{
  SAFESTATIC VectorNd qx, qy;

  for (unsigned i=0; i< _bodies.size(); i++)
  {
    // compute and set the generalized coordinates
    (qx = q0[i].first) *= ((double) 1.0 - t);
    (qy = q1[i].first) *= t;
    qx += qy;
    _bodies[i]->set_generalized_coordinates(DynamicBody::eEuler, qx);
    FILE_LOG(LOG_SIMULATOR) << " -- set_coords_and_velocities() setting body " << _bodies[i]->id << endl;
    FILE_LOG(LOG_SIMULATOR) << "    - state at q0: " << q0[i].first << endl;
    FILE_LOG(LOG_SIMULATOR) << "    - state at q1: " << q1[i].first << endl;
    FILE_LOG(LOG_SIMULATOR) << "    - generalized coords: " << qx << endl;

    // compute the generalized velocities using finite differencing
    (qx = q0[i].second) *= ((double) 1.0 - t);
    (qy = q1[i].second) *= t;
    qx += qy;
    FILE_LOG(LOG_SIMULATOR) << "    - velocity at q0: " << q0[i].second << endl;
    FILE_LOG(LOG_SIMULATOR) << "    - velocity at q1: " << q1[i].second << endl;
    FILE_LOG(LOG_SIMULATOR) << "    - generalized vels: " << qx << endl;
    _bodies[i]->set_generalized_velocity(DynamicBody::eEuler, qx);
  }
}
*/

/// Does a semi-implicit Euler integration
void EventDrivenSimulator::integrate_si_Euler(double step_size)
{
  VectorNd q, qd, x, dx;

  // begin timing dynamics
  tms start;  
  times(&start);

  // get the state-derivative for each dynamic body
  for (unsigned i=0; i< _bodies.size(); i++)
  {
    // integrate the body
    if (LOGGING(LOG_SIMULATOR))
    {
      Ravelin::VectorNd q;
      FILE_LOG(LOG_SIMULATOR) << "  generalized coordinates (before): " << _bodies[i]->get_generalized_coordinates(DynamicBody::eEuler, q) << std::endl;
      FILE_LOG(LOG_SIMULATOR) << "  generalized velocities (before): " << _bodies[i]->get_generalized_velocity(DynamicBody::eSpatial, q) << std::endl;
    }

    // compute the velocity 
    _bodies[i]->get_generalized_coordinates(DynamicBody::eEuler, q);
    _bodies[i]->get_generalized_velocity(DynamicBody::eSpatial, qd);
    x.resize(q.size()+qd.size());
    x.set_sub_vec(0, q);
    x.set_sub_vec(q.size(), qd);
    _bodies[i]->ode_both(x, current_time, step_size, &_bodies[i], dx);

    // update the velocity and position
    dx.segment(q.size(), dx.size()) *= step_size;
    qd += dx.segment(q.size(), dx.size());
    _bodies[i]->set_generalized_velocity(DynamicBody::eSpatial, qd);
    _bodies[i]->get_generalized_velocity(DynamicBody::eEuler, qd);
    qd *= step_size;
    q += qd; 
    _bodies[i]->set_generalized_coordinates(DynamicBody::eEuler, q);

    if (LOGGING(LOG_SIMULATOR))
    {
      Ravelin::VectorNd q;
      FILE_LOG(LOG_SIMULATOR) << "  generalized coordinates (after): " << _bodies[i]->get_generalized_coordinates(DynamicBody::eEuler, q) << std::endl;
      FILE_LOG(LOG_SIMULATOR) << "  generalized velocities (after): " << _bodies[i]->get_generalized_velocity(DynamicBody::eSpatial, q) << std::endl;
    }
  }

  // tabulate dynamics computation
  tms stop;  
  times(&stop);
  dynamics_utime += (double) (stop.tms_utime-start.tms_utime)/CLOCKS_PER_SEC;
  dynamics_stime += (double) (stop.tms_stime-start.tms_stime)/CLOCKS_PER_SEC;
}

/// Steps the simulator forward
double EventDrivenSimulator::step(double step_size)
{
  const double INF = std::numeric_limits<double>::max();

  // clear timings
  dynamics_utime = (double) 0.0;
  dynamics_stime = (double) 0.0;
  event_utime = (double) 0.0;
  event_stime = (double) 0.0;
  coldet_utime = (double) 0.0;
  coldet_stime = (double) 0.0;

  // setup the amount remaining to step
  double dt = step_size;

  // clear one-step visualization data
  #ifdef USE_OSG
  _transient_vdata->removeChildren(0, _transient_vdata->getNumChildren());
  #endif
  FILE_LOG(LOG_SIMULATOR) << "+stepping simulation from time: " << this->current_time << std::endl;

  // store the current generalized coordintes 
  get_coords(_q0);

  // integrate the systems forward by dt
  integrate_si_Euler(dt);

  // save the new phase coordinates 
  get_coords(_qf);
  get_velocities(_qdf);

  // methods below assume that coords/velocities of the bodies may be modified,
  // so we need to take precautions to save/restore them as necessary
  while (dt > (double) 0.0)
  {
    // look for events
    double t = find_and_handle_si_events(dt);
    if (t > dt)
      break; // no event.. finish up

    // events have been handled already; reduce dt and keep integrating
    dt -= t;

    // call the mini-callback
    if (post_mini_step_callback_fn)
      post_mini_step_callback_fn(this);

    // get the new velocities
    get_velocities(_qdf);

    // update the coordinates using the new velocities
    for (unsigned i=0; i< _q0.size(); i++)
    {
      _qf[i] = _qdf[i];
      _qf[i] *= dt;
      _qf[i] += _q0[i];
    }
  }

  // call the callback 
  if (post_step_callback_fn)
    post_step_callback_fn(this);
  
  return step_size;
}

/// Finds and handles first impacting event(s) in [0,dt]; returns time t in [0,dt] of first impacting event(s) and advances bodies' dynamics to time t
double EventDrivenSimulator::find_and_handle_si_events(double dt)
{
  vector<Event> cd_events, limit_events;
  typedef map<Event, double, EventCompare>::const_iterator EtolIter;

  // only for debugging purposes: verify that bodies aren't already interpenetrating
  #ifndef NDEBUG
  if (!_simulation_violated)
    check_violation();
  #endif

  // clear events 
  _events.clear();

  // begin timing for collision detection
  tms start;
  times(&start);

  FILE_LOG(LOG_SIMULATOR) << "-- checking for event in interval [" << (this->current_time) << ", " << (this->current_time+dt) << "] (dt=" << dt << ")" << std::endl;

  // make sure that dt is non-negative
  assert(dt >= (double) 0.0);

  // setup x0, x1
  if (!collision_detectors.empty())
  {
    _x0.resize(_q0.size());
    _x1.resize(_q0.size());
    for (unsigned i=0; i< _bodies.size(); i++)
    {
      _x0[i].first = _x1[i].first = _bodies[i];
      _x0[i].second = _q0[i];
      _x1[i].second = _qf[i];
    }
  }

  // call each collision detector
  BOOST_FOREACH(shared_ptr<CollisionDetection> cd, collision_detectors)
  {
    // indicate this is event driven
    cd->return_all_contacts = true;

    // do the collision detection routine
    cd_events.clear();
    cd->is_contact(dt, _x0, _x1, cd_events);

    // add to events
    _events.insert(_events.end(), cd_events.begin(), cd_events.end());
  }

  // tabulate times for collision detection 
  tms stop;  
  times(&stop);
  coldet_utime += (double) (stop.tms_utime-start.tms_utime)/CLOCKS_PER_SEC;
  coldet_stime += (double) (stop.tms_stime-start.tms_stime)/CLOCKS_PER_SEC;

  // check each articulated body for a joint limit event
  limit_events.clear();
  find_limit_events(dt, limit_events);
  _events.insert(_events.end(), limit_events.begin(), limit_events.end());

  // sort the set of events
  std::sort(_events.begin(), _events.end()); 

  // set the "real" time for the events and compute the event tolerances
  // output the events
  if (LOGGING(LOG_EVENT))
  {
    FILE_LOG(LOG_EVENT) << "Events to be processed:" << std::endl;
    for (unsigned i=0; i< _events.size(); i++)
      FILE_LOG(LOG_EVENT) << _events[i] << std::endl;
  }

  // set the "real" time for the events
  for (unsigned i=0; i< _events.size(); i++)
  {
    _events[i].t_true = current_time + _events[i].t * dt;
    EtolIter j = _event_tolerances.find(_events[i]);
    if (j != _event_tolerances.end())
      _events[i].tol = j->second;
  }

  // find and integrate body positions to the time-of-impact
  double h = find_TOI(dt);

  // handle the events
  if (h < dt)
    handle_events();

  return h;  
}

/*
/// Steps the simulator forward
double EventDrivenSimulator::step(double step_size)
{
  static vector<shared_ptr<void> > x, xplus;
  static vector<VectorNd> q0, q1, qd0, qd1, qstar;
  
  const double INF = std::numeric_limits<double>::max();

  // setup the phase sequence
  _phases.bodies = _bodies;
  _phases.intervals.resize(_bodies.size());
  for (unsigned i=0; i< _phases.intervals.size(); i++)
    _phases.intervals[i].resize(_bodies[i]->num_generalized_coordinates(DynamicBody::eSpatial));

  // clear timings
  dynamics_utime = (double) 0.0;
  dynamics_stime = (double) 0.0;
  event_utime = (double) 0.0;
  event_stime = (double) 0.0;
  coldet_utime = (double) 0.0;
  coldet_stime = (double) 0.0;

  // setup the amount remaining to step
  double dt = step_size;

  // clear one-step visualization data
  #ifdef USE_OSG
  _transient_vdata->removeChildren(0, _transient_vdata->getNumChildren());
  #endif
  FILE_LOG(LOG_SIMULATOR) << "+stepping simulation from time: " << this->current_time << std::endl;

  // methods below assume that coords/velocities of the bodies may be modified,
  // so we need to take precautions to save/restore them as necessary
  while (dt > (double) 0.0)
  {
    // store the current phase coordintes 
    get_coords_and_velocities(_q0, _qd0);

    // integrate the systems forward by dt
    integrate(dt);

    // save the new phase coordinates 
    get_coords_and_velocities(_qf, _qdf);

    // setup the current coordinate evolution
    for (unsigned i=0; i< _phases.q.size(); i++)
    {
      _phases.q[i].clear();
      _phases.q[i].push_back(_q0[i]);
      _phases.q[i].push_back(_qf[i]);
    }

    // setup the current velocity evolution
    for (unsigned i=0; i< _phases.qd.size(); i++)
    {
      _phases.qd[i].clear();
      _phases.qd[i].push_back(compute_interval(_q0[i], _qf[i], _qd0[i], _qdf[i]));
    }

    // compute segments
    compute_segments();

    // look for events at time 0; if such events occur, we'll do a
    // semi-explicit step
    double t = find_and_handle_events(dt);
    if (t > dt)
      break; // no event.. finish up
    else if (t < std::numeric_limits<double>::epsilon())
    {
      // TODO: do semi-explicit step here
    }

    // events have been handled already; reduce dt and keep integrating
    dt -= t;
    // TODO: update current time here?
    current_time += t;

    // call the mini-callback
    if (post_mini_step_callback_fn)
      post_mini_step_callback_fn(this);
  }

  // TODO: update current time here?
  // update the current time
  current_time += dt;

  // call the callback 
  if (post_step_callback_fn)
    post_step_callback_fn(this);
  
  return step_size;
}

/// Gets all transition points
void EventDrivenSimulator::PhaseSequence::get_transition_points(vector<double>& tx)
{
  // add first point
  tx.clear();
  tx.push_back(0);
  for (unsigned i=0; i< _phases.intervals.size(); i++)
    for (unsigned j=0; j< _phases.intervals[i].size(); j++)
    {
      switch (_phases.intervals[i][j].type)
      {
        case Interval::eBinary: // do nothing
          break;

        case Interval::eTernary:
          tx.push_back(_phases.intervals[i][j].tc);
          break;

        case Interval::eQuaternary:
          tx.push_back(_phases.intervals[i][j].ta);
          tx.push_back(_phases.intervals[i][j].tb);
          break;
      }
    }

  // sort points
  std::sort(tx.begin(), tx.end());

  // make them unique
  // TODO: add lambda function
  tx.erase(std::unique(tx.begin(), tx.end(), compare_double), tx.end());

  // add end point
  tx.push_back(1.0);
} 

/// Computes all segments
void EventDrivenSimulator::compute_segments(const vector<VectorNd>& q0, const vector<VectorNd>& qf, const vector<VectorNd>& qd0, const vector<VectorN>& qdf)
{
  // loop through all bodies
  for (unsigned i=0; i< _bodies.size(); i++)
  {
    // loop through all body coordinates
    for (unsigned j=0; j< q0[i].size(); j++)
    {
      // get the change for this particular coordinate
      double dq = qf[i][j] - q0[i][j];

      // now get the velocities at the endpoints
      double qd_0 = qd0[i][j];
      double qd_f = qdf[i][j];

      // setup the interval endpoints
      _phases.intervals[i][j].q0 = q0[i][j];
      _phases.intervals[i][j].qf = qf[i][j];
      _phases.intervals[i][j].qd0 = qd_0;  
      _phases.intervals[i][j].qd0 = qd_f;  

      // see whether we can use a standard interval 
      if (sign_equal(dq, qd_0, qd_f))
      {
        _phases.intervals[i][j].type = Interval::eBinary;
        continue;
      }

      // see whether we want a quaternary interval
      if (dq <= 0.0 && qd0 > 0.0 && qd1 > 0.0)
      {
        _phases.intervals[i][j].type = Interval::eQuaternary;
        _phases.intervals[i][j].calc_ternary_interval();
      }
      else if (dq >= 0.0 && qd0 < 0.0 && qd1 < 0.0)
      {
        _phases.intervals[i][j].type = Interval::eQuaternary;
        _phases.intervals[i][j].calc_ternary_interval();
      }
      // a ternary interval is fine
      else
      {
        _phases.intervals[i][j].type = Interval::eTernary;
        _phases.intervals[i][j].calc_quaternary_interval();
      }
    }
  }
}

/// Gets the position for a time in an interval
double EventDrivenSimulator::Interval::calc_position(double t)
{
  assert(t >= -NEAR_ZERO && t <= 1.0 + NEAR_ZERO); 

  // TODO: we should just be able to integrate velocity to get these values
  // position will depend on the inteval type
  switch (type)
  {
    case eBinary:
      return q0 + (qf - q0)*t;

    case eTernary:
      if (t < tc)
        return q0 + (qc - q0)*(t/tc);
      else
        return qc + (qf - qc)*(t-tc)/(1.0-tc);
    
    case eQuaternary:
      if (t < ta)
        return q0 + (qa - q0)*(t/ta);
      else if (t < tb)
        return qa + (qb - qa)*((t-ta)/(tb-ta));
      else
        return qb + (qf - qb)*(t-tb)/(1-tb);
  }
}

/// Gets the velocity for a time in an interval
double EventDrivenSimulator::Interval::calc_velocity(double t)
{
  // position will depend on the inteval type
  switch (type)
  {
    case eBinary:
    case eTernary:
      return qd0 + (qdf - qd0)*t;

    case eQuaternary:
      return k1*(t - tc)*(t - tc) + k2;
  }
}

/// Computes the necessary constants for the quaternary interval
void EventDrivenSimulator::Internval::calc_quaternary_interval()
{
  // precompute some constants
  const double q0_2 = q0*q0;
  const double qf_2 = qf*qf;
  const double qd0_2 = qd0*qd0;
  const double qdf_2 = qdf*qdf;

  // Mathematica code follows...
  tc = (3*q0 + 2*qd0 + qdf - 3*qf)/(3.*(2*q0 + qd0 + qdf - 2*qf));
  k1 = 3*(2*q0 + qd0 + qdf - 2*qf);
  k2 = (-9*q0_2 - 6*q0*qd0 - qd0_2 - 6*q0*qdf - qd0*qdf - 
        qdf_2 + 18*q0*qf + 6*qd0*qf + 6*qdf*qf - 9*qf_2)/
      (3.*(2*q0 + qd0 + qdf - 2*qf));

  // the quadratic is k1*(t - tc)^2 + k2 = 0
  // : k1*t^2 - 2*k1*t*tc + k1*tc^2 + k2

  // setup constants for quadratic formula
  const double a = k1;
  const double b = -2.0*k1*tc;
  const double c = k1*tc*tc + k2;

  // compute the discriminant
  const double disc = b*b - 4*a*c;

  // verify that the discriminant is clearly positive 
  assert(disc > NEAR_ZERO);

  // do floating point implementation for quadratic formula
  ta = (-b - sgn(b)*std::sqrt(disc))/(2*a);
  tb = c/(a*ta);

  // swap ta and tb (if necessary) so that they're ordered
  if (ta > tb)
    std::swap(ta, tb);

  // now compute qa and qb
  qa = q0 + k2*ta + (k1*ta*ta*ta)/3. - k1*ta*ta*tc + k1*ta*tc*tc;
  qb = q0 + k2*tb + (k1*tb*tb*tb)/3. - k1*tb*tb*tc + k1*tb*tc*tc;
}

/// Computes the transition point (tc) of a ternary interval
void EventDrivenSimulator::Interval::calc_ternary_interval()
{
  // compute the transition point
  tc = (2*q0 + 2*qd0 - 2*qf + qdf)/(qd0 - qdf);

  // compute the coordinate at tc
  qc = q0 + qd0*(2-tc)*0.5;
}

/// Finds and handles first impacting event(s) in [0,dt]; returns time t in [0,dt] of first impacting event(s) and advances bodies' dynamics to time t
double EventDrivenSimulator::find_and_handle_events(double h)
{
  vector<double> tx;
  vector<Event> cd_events, limit_events;
  double t0, tf;
  typedef map<Event, double, EventCompare>::const_iterator EtolIter;

  // only for debugging purposes: verify that bodies aren't already interpenetrating
  #ifndef NDEBUG
  if (!_simulation_violated)
    check_violation();
  #endif

  // clear events 
  _events.clear();

  // get the interval transition points
  _phases.get_transition_points(tx); 

  // get the position at time t=0
  _phases.get_position(0.0, _q0);

  // loop over all transition points
  for (unsigned j=1; j< tx.size(); j++)
  {
    // get the time
    double t = tx[j]; 

    // get the position at tx[j]
    _phases.get_position(t, _qf);
    
    // begin timing for collision detection
    tms start;
    times(&start);

    // compute dt
    const double dt = t - t[j-1];
    FILE_LOG(LOG_SIMULATOR) << "-- checking for event in interval [" << (this->current_time+t[j-1]) << ", " << (this->current_time+t) << "] (dt=" << dt << ")" << std::endl;

    // make sure that dt is non-negative
    assert(dt >= (double) 0.0);

    // setup x0, x1
    if (!collision_detectors.empty())
    {
      x0.resize(_q0.size());
      x1.resize(_q0.size());
      for (unsigned i=0; i< _bodies.size(); i++)
      {
        x0[i].first = x1[i].first = _bodies[i];
        x0[i].second = _q0[i];
        x1[i].second = _qf[i];
      }
    }

    // call each collision detector
    BOOST_FOREACH(shared_ptr<CollisionDetection> cd, collision_detectors)
    {
      // indicate this is event driven
      cd->return_all_contacts = true;

      // do the collision detection routine
      cd_events.clear();
      cd->is_contact(dt, x0, x1, cd_events);

      // add to events
      _events.insert(_events.end(), cd_events.begin(), cd_events.end());
    }

    // tabulate times for collision detection 
    tms stop;  
    times(&stop);
    coldet_utime += (double) (stop.tms_utime-start.tms_utime)/CLOCKS_PER_SEC;
    coldet_stime += (double) (stop.tms_stime-start.tms_stime)/CLOCKS_PER_SEC;

    // check each articulated body for a joint limit event
    limit_events.clear();
    find_limit_events(_q0, _qf, dt, limit_events);
    _events.insert(_events.end(), limit_events.begin(), limit_events.end());

    // sort the set of events
    std::sort(_events.begin(), _events.end()); 

    // set the "real" time for the events and compute the event tolerances
    // output the events
    if (LOGGING(LOG_EVENT))
    {
    FILE_LOG(LOG_EVENT) << "Events to be processed:" << std::endl;
    for (unsigned i=0; i< _events.size(); i++)
      FILE_LOG(LOG_EVENT) << _events[i] << std::endl;
    }

    // set the "real" time for the events
    for (unsigned i=0; i< _events.size(); i++)
    {
      // TODO: fix the time
      _events[i].t_true = current_time + _events[i].t * h;
      EtolIter j = _event_tolerances.find(_events[i]);
      if (j != _event_tolerances.end())
        _events[i].tol = j->second;
    }

    // find and "integrate" to the time-of-impact
    double TOI = find_TOI(dt, _q0, _qf, _intervals);

    // check for Zeno point
    if (TOI < std::numeric_limits<double>::epsilon())
    {
      // if all events are resting or separating, we have a Zeno point
      Zeno = true;
      for (unsigned i=0; i< _events.size(); i++)
        if (_events[i].is_impacting())
        {
          Zeno = false;
          break;
        }
    }
    else
      Zeno = true;

    // if it's a Zeno point, update the velocities and redetermine the event classes
    if (Zeno)
    {
      // determine bodies in the events
      vector<DynamicBodyPtr> treated_bodies;
      for (unsigned i=0; i< _events.size(); i++)
        _events[i].get_super_bodies(std::back_inserter(treated_bodies));

      // remove duplicates
      std::sort(treated_bodies.begin(), treated_bodies.end());
      treated_bodies.erase(std::unique(treated_bodies.begin(), treated_bodies.end()), treated_bodies.end());
 
      // TODO: fix this
      // set velocities for bodies in events  
      for (unsigned i=0; i< _bodies.size(); i++)
        if (std::binary_search(treated_bodies.begin(), treated_bodies.end(), _bodies[i]))
        _bodies[i]->set_generalized_velocity(DynamicBody::eEuler, q1[i].second);
    }

    // finally, handle the events
    if (TOI <= dt)
      handle_events();
  }

  // TODO: fix this
  return TOI;
}
*/

/// Finds joint limit events
void EventDrivenSimulator::find_limit_events(double dt, vector<Event>& events)
{
  // clear the vector of events
  events.clear();

  // process each articulated body, looking for joint events
  for (unsigned i=0; i< _bodies.size(); i++)
  {
    // see whether the i'th body is articulated
    ArticulatedBodyPtr ab = dynamic_pointer_cast<ArticulatedBody>(_bodies[i]);
    if (!ab)
      continue;
    
    // get limit events in [t, t+dt] (if any)
    ab->find_limit_events(_q0[i], _qf[i], dt, std::back_inserter(events));
  }
}

/// Finds the next time-of-impact out of a set of events
double EventDrivenSimulator::find_TOI(double dt)
{
  const double INF = std::numeric_limits<double>::max();

  FILE_LOG(LOG_SIMULATOR) << "EventDrivenSimulator::find_TOI() entered with dt=" << dt << endl;

  // get the iterator start
  vector<Event>::iterator citer = _events.begin();

  // setup integration performed 
  double h = (double) 0.0;

  // loop while the iterator does not point to the end -- may need several
  // iterations b/c there may be no impacting events in a group 
  while (citer != _events.end())
  {
    // set tmin
    double tmin = citer->t*dt;
    FILE_LOG(LOG_SIMULATOR) << "  -- find_TOI() while loop, current time=" << current_time << " tmin=" << tmin << endl;

    // check for exit
    if (tmin > dt)
    {
      FILE_LOG(LOG_SIMULATOR) << "    " << tmin << " > " << dt << " --> exiting now w/o events" << endl;
      FILE_LOG(LOG_SIMULATOR) << "    .... but first, integrating bodies forward by " << (dt-h) << std::endl;

      // events vector no longer valid; clear it
      _events.clear();

      // set the coordinates
      for (unsigned i=0; i< _bodies.size(); i++)
      {
        _qf[i] = _qdf[i];
        _qf[i] *= dt;
        _qf[i] += _q0[i];
        _bodies[i]->set_generalized_coordinates(DynamicBody::eEuler, _qf[i]);
      }

      // update current_time
      current_time += dt;

      return INF;
    }

    // "integrate" starting coordinates to tmin
    h += tmin;
    for (unsigned i=0; i< _q0.size(); i++)
    {
      _qf[i] = _qdf[i];
      _qf[i] *= h;
      _qf[i] += _q0[i];
      _bodies[i]->set_generalized_coordinates(DynamicBody::eEuler, _qf[i]);
    }
    FILE_LOG(LOG_SIMULATOR) << "    current time is " << current_time << endl;
    FILE_LOG(LOG_SIMULATOR) << "    tmin (time to next event): " << tmin << endl;
    FILE_LOG(LOG_SIMULATOR) << "    moving forward by " << h << endl;

    // check for impacting event
    bool impacting = citer->is_impacting();

    // find all events at the same time as the event we are examining
    for (citer++; citer != _events.end(); citer++)
    {
      // see whether we are done
      if (citer->t*dt > tmin + std::numeric_limits<double>::epsilon())
        break;

      // see whether this event is impacting (if we don't yet have an
      // impacting event)
      if (!impacting)
        impacting = citer->is_impacting(); 
    }

    // see whether we are done
    if (impacting)
    {
      // remove remainder of events
      _events.erase(citer, _events.end());

      // step positions to h (note that we we'll no longer need current value
      // of _qdf) 
      for (unsigned i=0; i< _q0.size(); i++)
      {
        _qf[i] = _qdf[i];
        _qf[i] *= h;
        _q0[i] += _qf[i];
        _bodies[i]->set_generalized_coordinates(DynamicBody::eEuler, _q0[i]);
      }

      // update current time
      current_time += h;

      return h;
    }
    else
      citer = _events.erase(citer, _events.end());
  }

  // contact map is empty, no contacts
  FILE_LOG(LOG_SIMULATOR) << "-- find_TOI(): no impacts detected; integrating forward by " << dt << endl;

  // events vector is no longer valid; clear it
  _events.clear();

  // set the coordinates (velocities are already set)
  for (unsigned i=0; i< _bodies.size(); i++)
  {
    _qf[i] = _qdf[i];
    _qf[i] *= dt;
    _q0[i] += _qf[i];
    _bodies[i]->set_generalized_coordinates(DynamicBody::eEuler, _qf[i]);
  }

  // update current_time
  current_time += dt;

  return INF;
}

/// Checks the simulator for a (contact/joint limit) violation
void EventDrivenSimulator::check_violation()
{
  BOOST_FOREACH(shared_ptr<CollisionDetection> cd, collision_detectors)
  {
    // do the collision detection routine
    if (cd->is_collision((double) 0.0))
    {
      if (!_simulation_violated)
      {
        std::cerr << "EventDrivenSimulator::is_contact() warning: detected interpenetrating geometries!" << endl;
        std::cerr << "  -- current time: " << current_time << "  NOTE: fidelity of simulation is no longer assured." << endl;
      }
      _simulation_violated = true;

      // detailed contact information
/*
      BOOST_FOREACH(const CollidingTriPair& ctp, cd->colliding_tris)
      {
        std::cerr << "    interpenetrating pair: " << endl;
        std::cerr << "      -- " << ctp.geom1->id << " (from " << ctp.geom1->get_single_body()->id << ")" << endl;
        std::cerr << "      -- " << ctp.geom2->id << " (from " << ctp.geom2->get_single_body()->id << ")" << endl;

        // get the triangles
        Triangle t1 = Triangle::transform(ctp.mesh1->get_triangle(ctp.tri1), ctp.geom1->get_transform());
        Triangle t2 = Triangle::transform(ctp.mesh2->get_triangle(ctp.tri2), ctp.geom2->get_transform());
        list<Vector3> isects;
        CompGeom::intersect_tris(t1, t2, std::back_inserter(isects));
        std::cerr << "      t1: " << t1 << std::endl;
        std::cerr << "      t2: " << t2 << std::endl;
        BOOST_FOREACH(const Vector3& point, isects)
        {
          std::cerr << "        isect: " << point << std::endl;
        }
      }
*/
      // standard contact information
      BOOST_FOREACH(sorted_pair<CollisionGeometryPtr> cg_pair, cd->colliding_pairs)
      {
        std::cerr << "    interpenetrating pair: " << endl;
        std::cerr << "      -- " << cg_pair.first->id << " (from " << cg_pair.first->get_single_body()->id << ")" << endl;
        std::cerr << "      -- " << cg_pair.second->id << " (from " << cg_pair.second->get_single_body()->id << ")" << endl;
      }
    }
  }
}

/// Implements Base::load_from_xml()
void EventDrivenSimulator::load_from_xml(shared_ptr<const XMLTree> node, map<std::string, BasePtr>& id_map)
{
  list<shared_ptr<const XMLTree> > child_nodes;
  map<std::string, BasePtr>::const_iterator id_iter;

  // verify node name b/c this is abstract class
  assert(strcasecmp(node->name.c_str(), "EventDrivenSimulator") == 0);

  // first, load all data specified to the Simulator object
  Simulator::load_from_xml(node, id_map);

  // clear list of collision detectors
  collision_detectors.clear();

  // get the collision detector, if specified
  const XMLAttrib* coldet_attrib = node->get_attrib("collision-detector-id");
  if (coldet_attrib)
  {
    // get the ID of the collision detector
    const std::string& id = coldet_attrib->get_string_value(); 

    // find a collision detector
    if ((id_iter = id_map.find(id)) == id_map.end())
    {
      std::cerr << "EventDrivenSimulator::load_from_xml() - could not find";
      std::cerr << std::endl << "  collision detector w/ID: " << id;
      std::cerr << " from offending node: " << std::endl << *node;
    }
    else
    {
      // make sure that it is castable to a collision detector before we
      // save the pointer
      shared_ptr<CollisionDetection> coldet = dynamic_pointer_cast<CollisionDetection>(id_iter->second);
      if (coldet)
      {
        collision_detectors.push_back(coldet);
        coldet->simulator = get_this();
      }
    }
  }

  // read in any CollisionDetection nodes
  child_nodes = node->find_child_nodes("CollisionDetector");
  BOOST_FOREACH(shared_ptr<const XMLTree> child_node, child_nodes)
  {
    const XMLAttrib* id_attrib = child_node->get_attrib("id");
    if (!id_attrib)
      continue;

    // get the ID of the collision detector
    const std::string& id = id_attrib->get_string_value(); 

    // find a collision detector
    if ((id_iter = id_map.find(id)) == id_map.end())
    {
      std::cerr << "EventDrivenSimulator::load_from_xml() - could not find";
      std::cerr << std::endl << "  collision detector w/ID: " << id;
      std::cerr << " from offending node: " << std::endl << *child_node;
    }
    else
    {
      // make sure that it is castable to a collision detector before we
      // save the pointer
      shared_ptr<CollisionDetection> coldet = dynamic_pointer_cast<CollisionDetection>(id_iter->second);
      if (coldet)
      {
        collision_detectors.push_back(coldet);
        coldet->simulator = get_this();
      }
    }
  }

  // read in any ContactParameters
  child_nodes = node->find_child_nodes("ContactParameters");
  if (!child_nodes.empty())
    contact_params.clear();
  for (list<shared_ptr<const XMLTree> >::const_iterator i = child_nodes.begin(); i != child_nodes.end(); i++)
  {
    boost::shared_ptr<ContactParameters> cd(new ContactParameters);
    cd->load_from_xml(*i, id_map);
    contact_params[cd->objects] = cd;
  }
}

/// Implements Base::save_to_xml()
void EventDrivenSimulator::save_to_xml(XMLTreePtr node, list<shared_ptr<const Base> >& shared_objects) const
{
  // call Simulator's save method first
  Simulator::save_to_xml(node, shared_objects);

  // reset the node's name
  node->name = "EventDrivenSimulator";

  // save the IDs of the collision detectors, if any 
  BOOST_FOREACH(shared_ptr<CollisionDetection> c, collision_detectors)
  {
    XMLTreePtr new_node(new XMLTree("CollisionDetector"));
    new_node->attribs.insert(XMLAttrib("id", c->id));
    node->add_child(new_node);
    shared_objects.push_back(c);
  }

  // save all ContactParameters
  for (map<sorted_pair<BasePtr>, shared_ptr<ContactParameters> >::const_iterator i = contact_params.begin(); i != contact_params.end(); i++)
  {
    XMLTreePtr new_node(new XMLTree("ContactParameters"));
    node->add_child(new_node);
    i->second->save_to_xml(new_node, shared_objects);
  }
}

/// Outputs this class data to the stream
/**
 * This method outputs all of the low-level details to the stream; if
 * serialization is desired, use save_to_xml() instead.
 * \sa save_to_xml()
 */
void EventDrivenSimulator::output_object_state(std::ostream& out) const
{
  // indicate the object type
  out << "EventDrivenSimulator object" << std::endl; 

  // output contact parameters
  out << "  contact parameters: " << std::endl;
  for  (map<sorted_pair<BasePtr>, shared_ptr<ContactParameters> >::const_iterator i = contact_params.begin(); i != contact_params.end(); i++)
  {
    out << "   object1: " << i->first.first << "  object2: ";
    out << i->first.second << "  parameters: " << i->second << std::endl;
  }

  // output collision detection pointers
  BOOST_FOREACH(shared_ptr<CollisionDetection> cd, collision_detectors)
    out << "  collision detector: " << cd << std::endl;

  // output event impulse callback function
   out << "  event post impulse callback fn: " << event_post_impulse_callback_fn << std::endl;

  // output event impulse callback data
   out << "  event post impulse callback data: " << event_post_impulse_callback_data << std::endl;

  // output event callback function
   out << "  event callback fn: " << event_callback_fn << std::endl;

  // output event callback data
   out << "  event callback data: " << event_callback_data << std::endl;
}

