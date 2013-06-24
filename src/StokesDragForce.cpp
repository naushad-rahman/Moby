/****************************************************************************
 * Copyright 2009 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

#include <iostream>
#include <Moby/XMLTree.h>
#include <Moby/RigidBody.h>
#include <Moby/ArticulatedBody.h>
#include <Moby/DeformableBody.h>
#include <Moby/StokesDragForce.h>

using namespace Ravelin;
using namespace Moby;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;
using std::vector;

/// Constructs a default drag coefficient of 0 
StokesDragForce::StokesDragForce()
{
  this->b = (double) 0.0;
}

/// Copy constructor
StokesDragForce::StokesDragForce(const StokesDragForce& source)
{
  operator=(source);
}

/// Adds gravity to a body
void StokesDragForce::add_force(DynamicBodyPtr body)
{
  // if the body is deformable, skip it
  if (dynamic_pointer_cast<DeformableBody>(body))
    return;

  // if the body is rigid, add drag
  RigidBodyPtr rb = dynamic_pointer_cast<RigidBody>(body);
  if (rb)
  {
    Wrenchd w;
    w.set_force(rb->velocity().get_linear() * -this->b);
    w.pose = rb->velocity().pose;
    Wrenchd wx = Pose3d::transform(w.pose, rb->get_computation_frame(), w);
    rb->add_wrench(wx);
  }
  else
  {
    // it's an articulated body, get it as such
    ArticulatedBodyPtr ab = boost::dynamic_pointer_cast<ArticulatedBody>(body);
      
    // get the vector of links
    const std::vector<RigidBodyPtr>& links = ab->get_links();
      
    // apply drag force to all links
    BOOST_FOREACH(RigidBodyPtr rb, links)
    {
      Wrenchd w;
      w.set_force(rb->velocity().get_linear() * -this->b);
      w.pose = rb->velocity().pose;
      Wrenchd wx = Pose3d::transform(w.pose, rb->get_computation_frame(), w);
      rb->add_wrench(wx);
    }
  }
}

/// Implements Base::load_from_xml()
void StokesDragForce::load_from_xml(shared_ptr<const XMLTree> node, std::map<std::string, BasePtr>& id_map)
{
  // load XML data for the parent class
  RecurrentForce::load_from_xml(node, id_map);

  // verify that the name of this node is correct
  assert(strcasecmp(node->name.c_str(), "StokesDragForce") == 0);

  // read the drag coefficient, if given
  const XMLAttrib* b_attrib = node->get_attrib("drag-b");
  if (b_attrib)
    this->b = b_attrib->get_real_value();
}

/// Implements Base::save_to_xml()
void StokesDragForce::save_to_xml(XMLTreePtr node, std::list<shared_ptr<const Base> >& shared_objects) const 
{
  // save XML data from the parent class
  RecurrentForce::save_to_xml(node, shared_objects);

  // (re)set the name of this node
  node->name = "StokesDragForce";

  // save the acceleration due to gravity 
  node->attribs.insert(XMLAttrib("drag-b", this->b));
}
 
